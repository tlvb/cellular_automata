#include "ca.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
ruleset_t *build_ruleset(const char *s) { /*{{{*/
	ruleset_t *rs = calloc(1, sizeof(ruleset_t));
	if (rs == NULL) {
		return NULL;
	}
	uint16_t *cr = &rs->surviveset;
	while (*s) {
		if (*s >= '0' && *s <= '8') {
			*cr |= 1<<(*s-'0');
		}
		else if (*s == '/' && cr == &rs->surviveset) {
			cr = &rs->birthset;
		}
		else {
			free(rs);
			rs = NULL;
			break;
		}
		++s;
	}
	return rs;
} /*}}}*/
void free_ruleset(ruleset_t *rs) { /*{{{*/
	if (rs != NULL) {
		free(rs);
	}
} /*}}}*/
void pp_ruleset(const ruleset_t *rs) { /*{{{*/
	fprintf(stderr, "alive cells survive if neighbourhood count matches any of:");
	for (int i=0; i<9; ++i) {
		if (rs->surviveset & (1<<i)) {
			fprintf(stderr, " %d", i);
		}
	}
	fprintf(stderr, "\ndead cells become alive if neighbourhood count matches any of:");
	for (int i=0; i<9; ++i) {
		if (rs->birthset & (1<<i)) {
			fprintf(stderr, " %d", i);
		}
	}
	fprintf(stderr, "\n");
} /*}}}*/
ruleset_lut_t *build_ruleset_lut(const ruleset_t *rs, void(*progress_reporter)(int)) { /*{{{*/
	ruleset_lut_t *rsl = malloc(sizeof(ruleset_lut_t));
	if (rsl == NULL) {
		return NULL;
	}
	rsl->lut = malloc(256*256*256*sizeof(uint8_t));
	if (rsl->lut == NULL) {
		return NULL;
	}
	for (int i=0; i<256; ++i) {
		for (int j=0; j<256; ++j) {
			for (int k=0; k<256; ++k) {
				uint8_t out = 0;
				for (int l=1; l<7; ++l) {
					int isalive = ((j&(1<<l))!=0)?1:0;
					uint16_t nsum = (
						(((i&(1<<(l+1)))!=0)?1:0) + (((i&(1<<(l+0)))!=0)?1:0) + (((i&(1<<(l-1)))!=0)?1:0) +
						(((j&(1<<(l+1)))!=0)?1:0) +                           + (((j&(1<<(l-1)))!=0)?1:0) +
						(((k&(1<<(l+1)))!=0)?1:0) + (((k&(1<<(l+0)))!=0)?1:0) + (((k&(1<<(l-1)))!=0)?1:0)
					);
					if (isalive != 0) {
						if ((rs->surviveset & 1<<nsum) != 0) {
							out |= 1<<l;
						}
					}
					else {
						if ((rs->birthset & 1<<nsum) != 0) {
							out |= 1<<l;
						}
					}
				}
				rsl->lut[i*0x010000 + j*0x000100 + k] = out;
			}
		}
		if (progress_reporter != NULL && (i&0x0f) == 0x0f) {
			progress_reporter(i);
		}
	}
	return rsl;
} /*}}}*/
void free_ruleset_lut(ruleset_lut_t *rsl) { /*{{{*/
	if (rsl != NULL) {
		if (rsl->lut != NULL) {
			free(rsl->lut);
		}
		free(rsl);
	}
} /*}}}*/
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
world_t *create_world(int w, int h) { /*{{{*/
	world_t *wo = malloc(sizeof(world_t));
	if (wo == NULL) {
		perror("create_world()");
		return NULL;
	}
	wo->w = w;
	wo->h = h;
	wo->d = calloc(2*w*h, sizeof(uint8_t));
	if (wo->d == NULL) {
		perror("create_world()");
		free(wo);
		return NULL;
	}
	return wo;
} /*}}}*/
void free_world(world_t *wo) { /*{{{*/
	if (wo != NULL) {
		if (wo->d != NULL) {
			free(wo->d);
		}
		free(wo);
	}
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
world_t *load_pbm_P4(FILE *f) {
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
}
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
void update_first_pass(world_t *wo, const ruleset_lut_t *rsl, int generation) { /*{{{*/
	// takes care of bits 6-1 of all world bytes by use of lookup table
	// AAAAAAAA    ........
	// AAAAAAAA => .BBBBBB.
	// AAAAAAAA    ........
	int next_generation = generation ^ 1;
	for (int x=0; x<wo->w; ++x) {
		// row[] is a rolling window of the current generation
		// first row wraparound special case taken care of by initialization
		uint8_t row[3] = {
			world_data(wo, generation, x, wo->h-1),
			world_data(wo, generation, x,       0),
			0
		};
		for (int y=0; y<wo->h-1; ++y) {
			row[2] = world_data(wo, generation, x, y+1);
			world_data(wo, next_generation, x, y) = lut_data(rsl, row[0], row[1], row[2]);
			row[0] = row[1]; row[1] = row[2];
		}
		// last row wraparound special case
		row[2] = world_data(wo, generation, x, 0);
		world_data(wo, next_generation, x, wo->h-1) = lut_data(rsl, row[0], row[1], row[2]);
	}
} /*}}}*/
void update_second_pass(world_t *wo, const ruleset_lut_t *rsl, int generation) { /*{{{*/
	// takes care of bits 7 and 1 of all world bytes by use of lookup table
	// and a bit more bit juggling
	// ------AA BB------    ----AABB    --------    -------- --------
	// ------AA BB------ -> ----AABB => -----CD- -> -------C D-------
	// ------AA BB------    ----AABB    --------    -------- --------
	int next_generation = generation ^ 1;
	for (int x=0; x<wo->w-1; ++x) {
		uint8_t row[3] = {
			((0x3&world_data(wo, generation, x, wo->h-1)) << 2) | (world_data(wo, generation, x+1, wo->h-1) >> 6),
			((0x3&world_data(wo, generation, x,       0)) << 2) | (world_data(wo, generation, x+1,       0) >> 6),
			0
		};
		for (int y=0; y<wo->h-1; ++y) {
			row[2] = ((0x3&world_data(wo, generation, x, y+1)) << 2) | (world_data(wo, generation, x+1, y+1) >> 6);
			uint8_t o = lut_data(rsl, row[0], row[1], row[2]);
			world_data(wo, next_generation, x, y) |= (0x4&o)>>2;
			world_data(wo, next_generation, x+1,y) |= (0x2&o)<<6;
			row[0] = row[1]; row[1] = row[2];
		}
		// last row wraparound special case
		row[2] = ((0x3&world_data(wo, generation, x, 0)) << 2) | (world_data(wo, generation, x+1, 0) >> 6);
		uint8_t o = lut_data(rsl, row[0], row[1], row[2]);
		world_data(wo, next_generation,   x, wo->h-1) |= (0x4&o)>>2;
		world_data(wo, next_generation, x+1, wo->h-1) |= (0x2&o)<<6;
	}
	// first and last column wraparound special case
	// BB------ [...] ------AA    ----AABB    --------    --------- [...] --------
	// BB------ [...] ------AA -> ----AABB => -----CD- -> D-------- [...] -------C
	// BB------ [...] ------AA    ----AABB    --------    --------- [...] --------
	uint8_t row[3] = {
		((0x3&world_data(wo, generation, wo->w-1, wo->h-1)) << 2) | (world_data(wo, generation, 0, wo->h-1) >> 6),
		((0x3&world_data(wo, generation, wo->w-1,       0)) << 2) | (world_data(wo, generation, 0,       0) >> 6),
		0
	};
	for (int y=0; y<wo->h-1; ++y) {
		row[2] = ((0x3&world_data(wo, generation, wo->w-1, y+1)) << 2) | (world_data(wo, generation, 0, y+1) >> 6);
		uint8_t o = lut_data(rsl, row[0], row[1], row[2]);
		world_data(wo, next_generation, wo->w-1, y) |= (0x4&o)>>2;
		world_data(wo, next_generation, 0,y) |= (0x2&o)<<6;
		row[0] = row[1]; row[1] = row[2];
	}
	// first and last column, last row wraparound special case
	row[2] = ((0x3&world_data(wo, generation, wo->w-1, 0)) << 2) | (world_data(wo, generation, 0, 0) >> 6);
	uint8_t o = lut_data(rsl, row[0], row[1], row[2]);
	world_data(wo, next_generation, wo->w-1, wo->h-1) |= (0x4&o)>>2;
	world_data(wo, next_generation,       0, wo->h-1) |= (0x2&o)<<6;
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
