#include "io.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

void pp_8(uint8_t b) { /*{{{*/
	for (int i=7; i>=0; --i) {
		if ((b & (1<<i)) != 0) {
			putchar('#');
		}
		else {
			putchar('.');
		}
	}
} /*}}}*/
int pbm_8(FILE *f, uint8_t b) { /*{{{*/
	for (int i=7; i>=0; --i) {
		if ((b & (1<<i)) != 0) {
			if (fputc('1', f) == EOF) {
				return EOF;
			}
		}
		else {
			if(putc('0', f) == EOF) {
				return EOF;
			}
		}
		if (i > 0) {
			if (putc(' ', f) == EOF) {
				return EOF;
			}
		}
	}
	return 0;
} /*}}}*/
void pp_lut(const ruleset_lut_t *rsl, int i, int j, int k) { /*{{{*/
	pp_8(i); fprintf(stderr, "\n");
	pp_8(j); fprintf(stderr, " -> "); pp_8(lut_data(rsl, i, j, k)); fprintf(stderr, "\n");
	pp_8(k); fprintf(stderr, "\n");
} /*}}}*/
void ignorewhitespace(FILE *f) { /*{{{*/
	int c = 0;
	do {
		c = fgetc(f);
	} while (isspace(c));
	ungetc(c, f);
} /*}}}*/
void ignorecomment(FILE *f) { /*{{{*/
	int t = fgetc(f);
	if (t == '#') {
		while (fgetc(f) != '\n');
	}
	else {
		ungetc(t, f);
	}
	ignorewhitespace(f);
} /*}}}*/
world_t *load_simple_text(FILE *f) { /*{{{*/
		// simple text format
		int width = 0;
		int height = 0;
		int xo = 0;
		int yo = 0;
		fscanf(f, "%d %d %d %d", &width, &height, &xo, &yo);
		world_t *wo = create_world((width+7)/8, height);
		if (wo == NULL) {
			return NULL;
		}
		int x=0;
		int y=0;
		for (;;) {
			switch(fgetc(f)) {
				case EOF:
					return wo;
					break;
				case '\n':
					x=0; ++y;
					break;
				case '#':
					if ((xo+x)>>8 >= wo->w || yo+y >= wo->h) {
						fprintf(stderr, "(%d,%d) is out of bounds (%d,%d)\n", (xo+x)>>8, yo+y, wo->w, wo->h);
						free_world(wo);
						wo = NULL;
						return NULL;
					}
					world_data(wo, 0, (xo+x)>>3, yo+y) |= (1<<(7-((xo+x)&0x7)));
				default:
					++x;
					break;
			}
		}
} /*}}}*/
world_t *load_pbm_P4(FILE *f) { /*{{{*/
	ignorewhitespace(f);
	ignorecomment(f);
	int width, height;
	if (fscanf(f, "%d %d", &width, &height) != 2) {
		return NULL;
	}
	ignorewhitespace(f);
	ignorecomment(f);
	world_t *wo = create_world((width+7)/8, height);
	for (int y=0; y<height; ++y) {
		for (int x=0; x<(width+7)/8; ++x) {
			int d = fgetc(f);
			if (d == EOF) {
				free_world(wo);
				return NULL;
			}
			world_data(wo, 0, x, y) = d;
		}
	}
	return wo;
} /*}}}*/
world_t *load_seed(const char *seedfn) { /*{{{*/
	world_t *wo = NULL;
	FILE *f = fopen(seedfn, "r");
	if (f == NULL) {
		perror("load_seed()");
		return NULL;
	}
	char fmt[4] = "nul";
	bool binary = false;
	fscanf(f, "%3s", fmt);
	fprintf(stderr, "format \"%s\"\n", fmt);
	if (strcmp("txt", fmt) == 0) { /*{{{*/
		wo = load_simple_text(f);
	}
	else if (strcmp("P4", fmt) == 0) {
		wo = load_pbm_P4(f);
	}
	else {
		// currently not supported
	} /*}}}*/
	fclose(f);
	return wo;
} /*}}}*/
void pp_world(const world_t *wo, int generation) { /*{{{*/
	for (int y=0; y<wo->h; ++y) {
		for (int x=0; x<wo->w; ++x) {
			pp_8(world_data(wo, generation, x, y));
		}
		printf("\n");
	}
} /*}}}*/
void write_pbm_world(const char *fn, const world_t *wo, int generation) { /*{{{*/
	FILE *f = fopen(fn, "w");
	if (f == NULL) {
		perror("write_pbm_world");
		return;
	}
	fprintf(f, "P4\n%d %d\n", wo->w*8, wo->h);
	for (int y=0; y<wo->h; ++y) {
		for (int x=0; x<wo->w; ++x) {
			if (fputc(world_data(wo, generation, x, y), f) == EOF) {
				goto pbm_error_0;
			}
		}
	}
	fclose(f);
	return;
pbm_error_1:
	perror("write_pbm_world");
pbm_error_0:
	fclose(f);
	return;
} /*}}}*/

