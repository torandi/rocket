#ifndef _GFX_H_
	#define _GFX_H_

	#include "common.h"

	#define GFX_ATTR_SHOOT 0
	#define GFX_ATTR_BOOST 1
	#define NUM_GFX_ATTR 2

	int init_sdl();
	void quit_sdl();
	/*
	 * Returns 1 if the user wants to quit
	 */
	int hndl_sdl_events();
	void draw_ship(char nick[32],int x, int y, int a,bool attr[NUM_GFX_ATTR]);
	void gfx_update();
	void gfx_clear();
#endif
