#include "bitmap.h"
#include <stdlib.h>
#include <stdio.h>

bool init_bitmap(bitmap_t *b, int w, int h) {
	if (b == NULL) {
		return false;
	}
	b->d = calloc(w*h, sizeof(uint8_t));
	if (b->d == NULL) {
		perror("init_bitmap()");
		return false;
	}
	b->w = w;
	b->h = h;
	return true;
}
void destroy_bitmap(bitmap_t *b) {
	if (b != NULL) {
		free(b->d);
	}
}
