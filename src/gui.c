#include "gui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <stdbool.h>

SDL_Surface *build_pixelstash(int pixelsize) { /*{{{*/
	SDL_Surface *s = SDL_CreateRGBSurface(0, 8*pixelsize, 256*pixelsize, 32, 0, 0, 0, 0);
	if (s == NULL) {
		return NULL;
	}
	SDL_LockSurface(s);
	for (int y=0; y<256; ++y) {
		for (int x=0; x<8; ++x) {
			SDL_Rect r = { .x=x*pixelsize, .y=y*pixelsize, .w=pixelsize, .h=pixelsize };
			if ((y&(1<<(7-x))) != 0) {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, 255, 255, 255, 255));
			}
			else {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, 0, 0, 0, 255));
			}
		}
	}
	SDL_UnlockSurface(s);
	SDL_SaveBMP(s, "/tmp/test.bmp");
	return s;
} /*}}}*/
int gui_main(world_t *wo, const ruleset_lut_t *rsl) { /*{{{*/
	int ret = 0;
	SDL_VideoInit(NULL);
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
		goto gui_exit_1;
	}
	int pxsz;
	for (pxsz=1; (pxsz+1)*wo->w*8+64 < dm.w && (pxsz+1)*wo->h+64 < dm.h; ++pxsz);
	printf("%d\n", pxsz);
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
	bool drawchange = false;
	int dx=0, dy=0, ds=0;
	while (!done) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) { /*{{{*/
			switch (e.type) {
				case SDL_WINDOWEVENT:
					if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
						done = true;
					}
					break;
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {
						case SDLK_ESCAPE:
							done = true;
							break;
						case SDLK_SPACE:
							run = !run;
							break;
						default:
							fprintf(stderr, "ESC quits the program, SPACE starts/stops time.\n");
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					switch (e.button.button) {
						case SDL_BUTTON_LEFT:
							drawing = true;
							dx = e.button.x;
							dy = e.button.y;
							ds = 1;
							drawchange = true;
							break;
						case SDL_BUTTON_RIGHT:
							drawing = true;
							dx = e.button.x;
							dy = e.button.y;
							drawchange = true;
							ds = 0;
							break;
						default:
							fprintf(stderr, "LEFT button sets a cell to alive, RIGHT button sets a cell to dead.\n");
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					drawing = false;
					break;
				case SDL_MOUSEMOTION:
					dx = e.motion.x;
					dy = e.motion.y;
					drawchange = true;
					break;
			}
		} /*}}}*/
		if (drawing && drawchange) {
			world_set_cell(wo, g, dx/pxsz, dy/pxsz, ds);
		}
		if (run || drawchange) {
			for (int x=0; x<wo->w; ++x) {
				for (int y=0; y<wo->h; ++y) {
					SDL_Rect srcr = { .x=0, .y=world_data(wo, g, x, y)*pxsz, .w=8*pxsz, .h=pxsz };
					SDL_Rect dstr = { .x=x*8*pxsz, .y=y*pxsz, .w=8*pxsz, .h=1*pxsz };
					SDL_RenderCopy(r, t, &srcr, &dstr);
				}
			}
			SDL_RenderPresent(r);
		}
		if (run) {
			update_world(wo, rsl, g);
			g ^= 1;
			i++;
		}
		drawchange = false;
	}
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
