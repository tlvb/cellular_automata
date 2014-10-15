#include "gui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <stdbool.h>

void populate_pixelstash(SDL_Surface *s, int pixelsize, int fillsize, int c0, int c1) { /*{{{*/
	for (int y=0; y<256; ++y) {
		for (int x=0; x<8; ++x) {
			SDL_Rect r = { .x=x*pixelsize, .y=y*pixelsize, .w=fillsize, .h=fillsize };
			if ((y&(1<<(7-x))) != 0) {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, c1, c1, c1, 255));
			}
			else {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, c0, c0, c0, 255));
			}
		}
	}
} /*}}}*/
SDL_Surface *build_pixelstash(int pixelsize) { /*{{{*/
	SDL_Surface *s = SDL_CreateRGBSurface(0, 8*pixelsize, 256*pixelsize, 32, 0, 0, 0, 0);
	if (s == NULL) {
		return NULL;
	}
	SDL_LockSurface(s);
	if (pixelsize >= 4) {
		populate_pixelstash(s, pixelsize, pixelsize, 31, 127);
		populate_pixelstash(s, pixelsize, pixelsize-1, 47, 191);
	}
	else {
		populate_pixelstash(s, pixelsize, pixelsize, 47, 191);
	}
	SDL_UnlockSurface(s);
	SDL_SaveBMP(s, "/tmp/test.bmp");
	return s;
} /*}}}*/
int gui_main(world_t *wo, const ruleset_lut_t *rsl) { /*{{{*/
	// INIT
	/*{{{*/
	int ret = 0;
	SDL_VideoInit(NULL);
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
		goto gui_exit_1;
	}
	int pxsz;
	for (pxsz=1; (pxsz+1)*wo->w*8+64 < dm.w && (pxsz+1)*wo->h+64 < dm.h; ++pxsz);
	SDL_Window *win = SDL_CreateWindow(
		"cellular_automaton",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		wo->w*8*pxsz, wo->h*pxsz,
		0
	);
	if (win == NULL) { /*{{{*/
		ret = 1;
		goto gui_exit_1;
	} /*}}}*/
	SDL_Renderer *r = SDL_CreateRenderer(win, -1, 0);
	if (r == NULL) { /*{{{*/
		ret = 2;
		goto gui_exit_2;
	} /*}}}*/
	SDL_Surface *s = build_pixelstash(pxsz);
	if (s == NULL) { /*{{{*/
		ret = 3;
		goto gui_exit_3;
	} /*}}}*/
	SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
	if (t == NULL) { /*{{{*/
		ret = 4;
		goto gui_exit_4;
	} /*}}}*/
	int g = 0;
	long int i = 0;
	bool done = false;
	bool run = false;
	bool drawing = false;
	bool needsredraw = true;
	int delay = 128;
	int dx=0, dy=0, ds=0;
	Uint32 t0 = SDL_GetTicks();
	bool timesup = true;
	/*}}}*/
	// MAIN LOOP
	while (!done) {
		Uint32 t1 = SDL_GetTicks();
		if (t1 > t0 + delay) {
			t0 = SDL_GetTicks();
			timesup = true;
		}
		SDL_Event e;
		while (SDL_PollEvent(&e)) { /*{{{*/
			switch (e.type) {
				case SDL_WINDOWEVENT: /*{{{*/
					if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
						done = true;
					}
					else {
						needsredraw = true;
					}
					break;
				/*}}}*/
				case SDL_KEYDOWN: /*{{{*/
					switch (e.key.keysym.sym) {
						case SDLK_ESCAPE:
							done = true;
							break;
						case SDLK_SPACE:
							run = !run;
							needsredraw = true;
							break;
						case SDLK_DOWN:
							if (delay == 32) {
								delay = 0;
							}
							else {
								delay /= 2;
							}
							fprintf(stderr, "sleeping %dms between iterations\n", delay);
							break;
						case SDLK_UP:
							if (delay == 0) {
								delay = 32;
							}
							else {
								delay *= 2;
							}
							fprintf(stderr, "sleeping %dms between iterations\n", delay);
							break;
						default:
							fprintf(
								stderr,
								"ESC quits the program, SPACE starts/stops time.\n"
								"UP/DOWN changes delay between iterations\n"
							);
							break;
					}
					break;
				/*}}}*/
				case SDL_MOUSEBUTTONDOWN: /*{{{*/
					switch (e.button.button) {
						case SDL_BUTTON_LEFT:
							drawing = true;
							ds = 1;
							needsredraw = true;
							world_set_cell(wo, g, e.button.x/pxsz, e.button.y/pxsz, ds);
							break;
						case SDL_BUTTON_RIGHT:
							drawing = true;
							needsredraw = true;
							ds = 0;
							world_set_cell(wo, g, e.button.x/pxsz, e.button.y/pxsz, ds);
							break;
						default:
							fprintf(stderr, "LEFT button sets a cell to alive, RIGHT button sets a cell to dead.\n");
							break;
					}
					break;
				/*}}}*/
				case SDL_MOUSEBUTTONUP: /*{{{*/
					drawing = false;
					break;
				case SDL_MOUSEMOTION:
					if (drawing) {
						world_set_cell(wo, g, e.motion.x/pxsz, e.motion.y/pxsz, ds);
						needsredraw = true;
					}
					break;
				/*}}}*/
			}
		} /*}}}*/
		if ((run && timesup) || needsredraw) { /*{{{*/
			for (int x=0; x<wo->w; ++x) {
				for (int y=0; y<wo->h; ++y) {
					SDL_Rect srcr = { .x=0, .y=world_data(wo, g, x, y)*pxsz, .w=8*pxsz, .h=pxsz };
					SDL_Rect dstr = { .x=x*8*pxsz, .y=y*pxsz, .w=8*pxsz, .h=1*pxsz };
					SDL_RenderCopy(r, t, &srcr, &dstr);
				}
			}
			SDL_RenderPresent(r);
		} /*}}}*/
		if (run && timesup) { /*{{{*/
			update_world(wo, rsl, g);
			g ^= 1;
			i++;
		} /*}}}*/
		needsredraw = false;
		timesup = false;
	}
	// DONE
	printf("%ld runs\n", i);
	SDL_DestroyTexture(t);
gui_exit_4:
	SDL_FreeSurface(s);
gui_exit_3:
	SDL_DestroyRenderer(r);
gui_exit_2:
	SDL_DestroyWindow(win);
gui_exit_1:
	SDL_VideoQuit();
	return ret;
} /*}}}*/
