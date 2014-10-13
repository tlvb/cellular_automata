#include "gui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>

SDL_Surface *build_pixelstash(void) { /*{{{*/
	SDL_Surface *s = SDL_CreateRGBSurface(0, 256, 256, 32, 0, 0, 0, 0);
	if (s == NULL) {
		return NULL;
	}
	SDL_LockSurface(s);
	for (int y=0; y<256; ++y) {
		for (int x=0; x<8; ++x) {
			SDL_Rect r = { .x=x, .y=y, .w=1, .h=1 };
			if ((y&(1<<(7-x))) != 0) {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, 255, 255, 255, 255));
			}
			else {
				SDL_FillRect(s, &r, SDL_MapRGBA(s->format, 0, 0, 0, 255));
			}
		}
	}
	SDL_UnlockSurface(s);
	return s;
} /*}}}*/
int gui_main(world_t *wo, const ruleset_lut_t *rsl) { /*{{{*/
	int ret = 0;
	SDL_VideoInit(NULL);
	SDL_Window *win = SDL_CreateWindow(
		"cellular_automaton",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		wo->w*8, wo->h,
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
	SDL_Surface *s = build_pixelstash();
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
	for (;;) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
				break;
			}
		}
		for (int x=0; x<wo->w; ++x) {
			for (int y=0; y<wo->h; ++y) {
				SDL_Rect srcr = { .x=0, .y=world_data(wo, g, x, y), .w=8, .h=1 };
				SDL_Rect dstr = { .x=x*8, .y=y, .w=8, .h=1 };
				SDL_RenderCopy(r, t, &srcr, &dstr);
			}
		}
		SDL_RenderPresent(r);
		update_first_pass(wo, rsl, g);
		update_second_pass(wo, rsl, g);
		g ^= 1;
	}
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
