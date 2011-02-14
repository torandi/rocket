#ifndef SHIP_H
#define SHIP_H
#include <stdlib.h>
#include "client.h"
#include "gfx.h"

struct ship_t {
	char nick[32];
	int x;
	int y;
	double a; //angle
	float s; //speed
	bool attr[NUM_GFX_ATTR];
	
	int _x;
	int _y;
};

#endif
