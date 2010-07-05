#ifndef _GFX_H_
	#define _GFX_H_

	typedef enum { 
		GFX_ATTR_SHOOT,
		GFX_ATTR_BOOST
	} gfx_attr_t;

	int init_sdl();
	void quit_sdl();
	/*
	 * Returns 1 if the user wants to quit
	 */
	int hndl_sdl_events();
	void draw_ship(char nick[32],int x, int y, int a,gfx_attr_t *attr,int num_attr);
	void gfx_update();
	void gfx_clear();
#endif
