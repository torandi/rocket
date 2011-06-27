#ifndef _GFX_H_
#define _GFX_H_

	struct ship_t;

	#define GFX_ATTR_SHOOT 0
	#define GFX_ATTR_BOOST 1
	#define GFX_ATTR_SCAN 2
	#define NUM_GFX_ATTR 3

	#define GFX_SCAN_SIZE 400

	#define GFX_SERVER_FPS 20
	#define GFX_TARGET_FPS 100
	#define GFX_FACTOR GFX_TARGET_FPS/GFX_SERVER_FPS

	#define GFX_NUM_HIGHSCORE_ENTRIES 5

	void init_gfx(int width,int height); 
	void start_event_loop();
	void stop_gfx();
	void draw_ship(const ship_t &ship);
	void draw_highscore();
	void gfx_update();
	void gfx_clear();
	void toggle_highscore();
	double degrees_to_radians(int d);

	extern int screen_width,screen_height;
#endif
