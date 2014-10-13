#ifndef __IO_H__
#define __IO_H__

#include "ca.h"

void pp_ruleset(const ruleset_t *rs);
void pp_8(uint8_t b);
int pbm_8(FILE *f, uint8_t b);
void pp_lut(const ruleset_lut_t *rsl, int i, int j, int k);
world_t *load_seed(const char *seedfn);
void pp_world(const world_t *wo, int generation);
void write_pbm_world(const char *fn, const world_t *wo, int generation);

#endif
