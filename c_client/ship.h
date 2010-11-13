#ifndef SHIP_H
#define SHIP_H
#include <stdlib.h>
#include "client.h"
#include "gfx.h"

struct ship_t {
	char nick[32];
	int x;
	int y;
	int a;
	bool attr[NUM_GFX_ATTR];
};

#endif
