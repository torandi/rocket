#include <stdio.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "gfx.h"
#include "common.h"

#define SCREEN_WIDTH  640 // width of the screen, in pixels
#define SCREEN_HEIGHT 480 // height of the screen, in pixels

#define PI 3.14159265

#define SHIP_SIZE 32
#define HALF_SHIP_SIZE SHIP_SIZE/2
#define FIRE_LENGTH 132 

#define NICK_FONT_SIZE 22
#define SMOOTH_ROTATION 0

#define FONT_FILE "data/Acknowledge_TT_BRK.ttf"
#define SHIP_GFX "data/ship.png"
#define SHIP_BOOST_GFX "data/ship_boost.png"



SDL_Surface *screen; 
SDL_Event    event;
SDL_Rect	screen_area;
TTF_Font * font;
SDL_Surface * ship,*ship_boost;
SDL_Color font_color={0xFF,0xFF,0xFF};

void slock();
void sulock();
int radians_to_degrees(double rad);
double degrees_to_radians(int d);
TTF_Font* loadfont(char* file, int ptsize);

/*
void main() {
	init_sdl();
	while(1) {
	draw_ship("Torandi",100,100,45,NULL,0);
	gfx_update();
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

	if (TTF_Init() == -1) {
		fprintf(stderr,"Unable to initialize SDL_ttf: %s \n", TTF_GetError());
		return 2;
	}

	font=loadfont(FONT_FILE,NICK_FONT_SIZE);

	if ((screen =
				SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 8, SDL_SWSURFACE ))
			== NULL) {

		fprintf( stderr, "Could not set SDL video mode: %s\n", SDL_GetError() );
		return 3;
	} // if (could not set mode)
	screen_area.x=0;
	screen_area.y=0;
	screen_area.w=SCREEN_WIDTH;
	screen_area.h=SCREEN_HEIGHT;

	SDL_WM_SetCaption( "Rocket - Robot Sockets", "Rocket - Robot Sockets" );
	SDL_ShowCursor( SDL_DISABLE );

	//Load ship image
	SDL_Surface * tmp=IMG_Load(SHIP_GFX);
	if(tmp==NULL) {
		    printf("Error: Failed to load ship gfx (%s)\n", IMG_GetError());
			 return 4;
	}
	ship=SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	//Load ship boost image
	tmp=IMG_Load(SHIP_BOOST_GFX);
	if(tmp==NULL) {
		    printf("Error: Failed to load ship boost gfx (%s)\n", IMG_GetError());
			 return 4;
	}
	ship_boost=SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	return 0;
}

TTF_Font* loadfont(char* file, int ptsize) {
	  TTF_Font* tmpfont;
	  tmpfont = TTF_OpenFont(file, ptsize);
		if (tmpfont == NULL){
		 printf("Unable to load font: %s %s \n", file, TTF_GetError());
		}
			  return tmpfont;
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
void draw_ship(char nick[32],int x, int y, int a,bool attr[NUM_GFX_ATTR]) {
	Uint32 color = 0xFFFFFFFF;
	SDL_Surface * cur_ship=ship;
	SDL_Rect text_rect,ship_pos;
	double angle=degrees_to_radians(a);
	
	slock();

	if(attr[GFX_ATTR_SHOOT])
		aalineColor(screen,x,y,x+FIRE_LENGTH*cos(angle),y-FIRE_LENGTH*sin(angle),color);
		

	SDL_Surface *text_surface=TTF_RenderText_Blended(font,nick,font_color);
	text_rect.x=x-(2*strlen(nick)*NICK_FONT_SIZE)/7;
	text_rect.y=y-HALF_SHIP_SIZE-NICK_FONT_SIZE;


	if(attr[GFX_ATTR_BOOST])
		cur_ship=ship_boost;

	SDL_BlitSurface(text_surface,NULL,screen,&text_rect);
	SDL_FreeSurface(text_surface);

	SDL_Surface *rotated_ship=rotozoomSurface(cur_ship,a,1.0,SMOOTH_ROTATION);
	//ship_pos.x=x-(HALF_SHIP_SIZE+(SHIP_DIAGONAL*abs(sin(angle+PI/4))));
	//ship_pos.y=y-(HALF_SHIP_SIZE+(SHIP_DIAGONAL*abs(cos(angle+PI/4))));
	ship_pos.x=x-(rotated_ship->w/2);
	ship_pos.y=y-(rotated_ship->h/2);
	SDL_BlitSurface(rotated_ship,NULL,screen,&ship_pos);

	SDL_FreeSurface(rotated_ship);

	sulock();
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
