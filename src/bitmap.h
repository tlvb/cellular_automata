#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	int w;
	int h;
	uint8_t *d;
} bitmap_t;

bool init_bitmap(bitmap_t *b, int w, int h);
void destroy_bitmap(bitmap_t *b);

#endif
