#include "ca.h"
#include "bitmap.h"
#include <stdlib.h>

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
world_t *create_world(int w, int h) { /*{{{*/
	world_t *wo = malloc(sizeof(world_t));
	if (wo == NULL) {
		perror("create_world()");
		return NULL;
	}
	wo->w = w;
	wo->h = h;
	if (!init_bitmap(wo->g, w, h) || !init_bitmap(wo->g+1, w, h)) {
		free_world(wo);
		return NULL;
	}
	return wo;
} /*}}}*/
void free_world(world_t *wo) { /*{{{*/
	if (wo != NULL) {
		if (wo->g[0].d != NULL) { destroy_bitmap(wo->g); }
		if (wo->g[1].d != NULL) { destroy_bitmap(wo->g+1); }
		free(wo);
	}
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
void update_world(world_t *wo, const ruleset_lut_t *rsl, int generation) { /*{{{*/
	update_first_pass(wo, rsl, generation);
	update_second_pass(wo, rsl, generation);
} /*}}}*/
void world_set_cell(world_t *wo, int generation, int x, int y, int state) { /*{{{*/
	uint8_t current = world_data(wo, generation, x>>3, y);
	uint8_t b = 1<<(7-(x&7));
	if (state) {
		current |= b;
	}
	else {
		current &= ~b;
	}
	world_data(wo, generation, x>>3, y) = current;
} /*}}}*/
