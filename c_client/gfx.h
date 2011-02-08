#ifndef _GFX_H_
	#define _GFX_H_

	#define GFX_ATTR_SHOOT 0
	#define GFX_ATTR_BOOST 1
	#define NUM_GFX_ATTR 2

	#define GFX_SERVER_FPS 50
	#define GFX_TARGET_FPS 100
	#define GFX_FACTOR GFX_TARGET_FPS/GFX_SERVER_FPS

	int init_sdl(int width,int height);
	void quit_sdl();
	/*
	 * Returns 1 if the user wants to quit
	 */
	int hndl_sdl_events();
	void draw_ship(char nick[32],int x, int y, int a,bool attr[NUM_GFX_ATTR]);
	void draw_highscore();
	void gfx_update();
	void gfx_clear();
	void toggle_highscore();
	double degrees_to_radians(int d);
#endif
