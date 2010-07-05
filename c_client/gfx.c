#include <stdio.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "gfx.h"

#define SCREEN_WIDTH  640 // width of the screen, in pixels
#define SCREEN_HEIGHT 480 // height of the screen, in pixels

#define SHIP_SIZE 14 
#define HALF_SHIP_SIZE SHIP_SIZE/2

#define PI 3.14159265

SDL_Surface *screen; 
SDL_Event    event;
SDL_Rect	screen_area;

void slock();
void sulock();
int radians_to_degrees(double rad);
double degrees_to_radians(int d);

/*
void main() {
	init_sdl();
	while(1) {
	draw_ship(100,100,45);
		if(hndl_sdl_events()!=0) {
			break;
			exit(0);
		}
		sleep(100);
	}
}*/

int init_sdl() {
	if (SDL_Init( SDL_INIT_VIDEO ) != 0) {

		fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
		return 1;

	} // if (could not init video)

	if ((screen =
				SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 8, SDL_SWSURFACE ))
			== NULL) {

		fprintf( stderr, "Could not set SDL video mode: %s\n", SDL_GetError() );
		return 2;
	
	SDL_WM_SetCaption( "Rocket - Robot Sockets", "Rocket - Robot Sockets" );
	SDL_ShowCursor( SDL_DISABLE );

	} // if (could not set mode)
	screen_area.x=0;
	screen_area.y=0;
	screen_area.w=SCREEN_WIDTH;
	screen_area.h=SCREEN_HEIGHT;

	return 0;
}

/*
 * Returns 1 if the user wants to quit
 */
int hndl_sdl_events() {
    // Handle events (keyboard input, mainly)
	while (SDL_PollEvent( &event )) {

		if (event.type == SDL_QUIT ||
				(event.type == SDL_KEYDOWN &&
				 event.key.keysym.sym == SDLK_ESCAPE)) {
			quit_sdl();
			return 1;	
		}

	} // while (handling events)
	return 0;
}

/**
 * Draws a ship at (x,y) directed at angle a (in degrees). 
 */
void draw_ship(char nick[32],int x, int y, int a,gfx_attr_t *attr,int num_attr) {
	Uint32 color = 0xFFFFFFFF;
	double angle = degrees_to_radians(a);
	
	slock();

	aatrigonColor(screen, x+SHIP_SIZE*cos(angle),y+SHIP_SIZE*sin(angle),x+SHIP_SIZE*cos(angle+2*PI/3),y+SHIP_SIZE*sin(angle+2*PI/3),x+SHIP_SIZE*cos(angle+4*PI/3),y+SHIP_SIZE*sin(angle+4*PI/3),color);

	sulock();
	//Update area
	//SDL_UpdateRect(screen,x-HALF_SHIP_SIZE,y-HALF_SHIP_SIZE,SHIP_SIZE,SHIP_SIZE);
}

void gfx_update() {
	SDL_UpdateRect(screen,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
}

void gfx_clear() {
	SDL_FillRect(screen,&screen_area,0x00);
}

void quit_sdl() {
	SDL_Quit();
}

//internal functions

int radians_to_degrees(double rad) {
	return (int) (rad * (180/PI));
}

double degrees_to_radians(int d) {
	return d*(PI/180.0);
}

void slock()
{
	if ( SDL_MUSTLOCK(screen) )
	{
		if ( SDL_LockSurface(screen) < 0 )
		{
			return;
		}
	}
}

void sulock()
{
	if ( SDL_MUSTLOCK(screen) )
	{
		SDL_UnlockSurface(screen);
	}
}
