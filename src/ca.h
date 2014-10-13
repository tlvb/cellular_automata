#ifndef __CA_H__
#define __CA_H__

#include <stdint.h>
#include <stdio.h>

typedef struct {
	uint16_t surviveset;
	uint16_t birthset;
} ruleset_t;

typedef struct {
	uint8_t *lut;
} ruleset_lut_t;

typedef struct {
	int w;
	int h;
	uint8_t *d;
} world_t;

#define lut_data(LUT_, I_, J_, K_) ((LUT_)->lut[((I_)<<16) + ((J_)<<8) + (K_)])
#define world_data(WW_, GG_, XX_, YY_) ((WW_)->d[(GG_)*(WW_)->w*(WW_)->h + (YY_)*(WW_)->w + (XX_)])

ruleset_t *build_ruleset(const char *s);
void free_ruleset(ruleset_t *rs);
ruleset_lut_t *build_ruleset_lut(const ruleset_t *rs, void(*progress_reporter)(int));
void free_ruleset_lut(ruleset_lut_t *rsl);
world_t *create_world(int w, int h);
void free_world(world_t *wo);
void update_first_pass(world_t *wo, const ruleset_lut_t *rsl, int generation);
void update_second_pass(world_t *wo, const ruleset_lut_t *rsl, int generation);

#endif
