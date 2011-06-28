#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <FTGL/ftgl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <string.h>
#include <math.h>

#define PI 3.14159265

#include "gfx.h"
#include "client.h"
#include "ship.h"

#define FONT_FILE "data/Acknowledge_TT_BRK.ttf"
#define FONT_FILE_SHARE "/usr/share/rocket/data/Acknowledge_TT_BRK.ttf"
#define SHIP_GFX "data/ship.png"
#define SHIP_GFX_SHARE "/usr/share/rocket/data/ship.png"
#define SHIP_BOOST_GFX "data/ship_boost.png"
#define SHIP_BOOST_GFX_SHARE "/usr/share/rocket/data/ship_boost.png"

#define SHIP_SIZE 32.0f
#define FIRE_LENGTH 128.0f

#define NICK_FONT_SIZE 22.0f
#define HS_FONT_SIZE 16.0f

bool active=false;
bool show_highscore=true;
int screen_width, screen_height;

SDL_Event    event;
std::string ship_gfx, ship_boost_gfx, font_file;
bool file_exists(const char * filename);
GLuint load_texture(const char* file);
void hndl_event(unsigned char key, int x, int y);

float radians_to_degrees(double rad);
void glCircle3i(GLint x, GLint y, GLint radius);

GLuint ship, ship_boost;
FTGLTextureFont * nick_font,*hs_font;
const float text_matrix[] = 	{ 1.0f,  0.0f, 0.0f, 0.0f,
										0.0f, -1.0f, 0.0f, 0.0f,
										0.0f,  0.0f, 1.0f, 0.0f,
										0.0f,  0.0f, 0.0f, 1.0f };

void init(int w, int h) {
	glClearColor(0,0,0,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,-1,1);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void init_gfx(int width, int height) {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetVideoMode(width,height, 24, SDL_OPENGL | SDL_GL_DOUBLEBUFFER );
	SDL_WM_SetCaption("Rocket - Robot Sockets","Rocket - Robot Sockets");

	screen_width = width;
	screen_height = height;

	init(width,height);

	if(file_exists(FONT_FILE)) {
		font_file=FONT_FILE;
		ship_gfx = SHIP_GFX;
		ship_boost_gfx = SHIP_BOOST_GFX;
	} else {
		font_file=FONT_FILE_SHARE;
		ship_gfx = SHIP_GFX_SHARE;
		ship_boost_gfx = SHIP_BOOST_GFX_SHARE;
	}
	nick_font = new FTTextureFont(font_file.c_str());
	hs_font = new FTTextureFont(font_file.c_str());

	nick_font->FaceSize(NICK_FONT_SIZE);
	hs_font->FaceSize(HS_FONT_SIZE);

	ship=load_texture(ship_gfx.c_str());
	ship_boost=load_texture(ship_boost_gfx.c_str());

	glBindTexture(GL_TEXTURE_2D,0);
	active = true;
}

void draw_ship(const ship_t &s) {
	if(!active) return;

	glColor3f(1,1,1);

	glPushMatrix();
	glTranslatef(s.x,s.y,0.0f);

	glPushMatrix();
	glMultMatrixf(text_matrix);
	glTranslatef(-(2*strlen(s.nick)*NICK_FONT_SIZE)/7,SHIP_SIZE/2.0f,0.0f);
	nick_font->Render(s.nick);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();

	if(s.attr[GFX_ATTR_SCAN]) {
		glDisable(GL_TEXTURE_2D);
		glCircle3i(0.0f,0.0f,GFX_SCAN_SIZE);
		glEnable(GL_TEXTURE_2D);
	}

	glRotatef(radians_to_degrees(s.a)*-1.0f, 0.0,0.0,1.0);

	if(s.attr[GFX_ATTR_SHOOT]) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(FIRE_LENGTH,1.0f,0.0f);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}

	glTranslatef(-SHIP_SIZE/2.0f,-SHIP_SIZE/2.0f,0.0);

	if(s.attr[GFX_ATTR_BOOST]) {
		glBindTexture(GL_TEXTURE_2D,ship_boost);
	} else {
		glBindTexture(GL_TEXTURE_2D,ship);
	}

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex3f(0.0f,0.0f,0.0f);
		glTexCoord2f(0.0f,1.0f); glVertex3f(0.0f,SHIP_SIZE,0.0f);
		glTexCoord2f(1.0f,1.0f); glVertex3f(SHIP_SIZE, SHIP_SIZE,0.0f);
		glTexCoord2f(1.0f,0.0f); glVertex3f(SHIP_SIZE,0.0f,0.0f);
	glEnd();


	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	

	glPopMatrix();

}

void gfx_clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_update() {
	glFlush();
	SDL_GL_SwapBuffers();
}

void draw_highscore() {
	if(show_highscore && highscore.size()>0) {
		glPushMatrix();

		glTranslatef(5.0f, 2.0f+HS_FONT_SIZE-4, 0.0f);

		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();

		glMultMatrixf(text_matrix);

		glMatrixMode(GL_PROJECTION);

		char buffer[64];

		for(int i=0;i<std::min((int)highscore.size(),GFX_NUM_HIGHSCORE_ENTRIES);++i) {
			score_t s=highscore[i];
			sprintf(buffer,"%i. %s: %.2f",i+1,s.nick,s.score);
			hs_font->Render(buffer);
			glTranslatef(0.0f,HS_FONT_SIZE-4,0.0f);
		}
		glMatrixMode(GL_MODELVIEW);

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);

		glPopMatrix();
	}
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
		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h) {
			toggle_highscore();	
		}
	} // while (handling events)
	return 0;
}

void quit_sdl() {
	if(!active) return;
	active=false;
	SDL_Quit();
}

void toggle_highscore() {
	if(active) {
		if(!show_highscore) {
			show_highscore=true;
		} else {
			show_highscore=false;
		}
	}
}

/** 
 * Private functions
 **/


bool file_exists(const char * filename) {
	if (FILE * file = fopen(filename, "r")) {
		fclose(file);
		return true;
	}
	return false;
}

GLuint load_texture(const char* file) {
	SDL_Surface* surface = IMG_Load(file);
	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	SDL_PixelFormat *format = surface->format;
	if (format->Amask)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4,
				surface->w, surface->h, GL_RGBA,GL_UNSIGNED_BYTE, surface->pixels);
	}
	else
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	}
	SDL_FreeSurface(surface);
	return texture;
}

float radians_to_degrees(double rad) {
	return (float) (rad * (180/PI));
}

void glCircle3i(GLint x, GLint y, GLint radius) { 
	float angle; 
	glBegin(GL_LINE_LOOP); 
		for(int i = 0; i < 100; i++) { 
			angle = i*2*PI/100.0f; 
			glVertex3f(x + (cos(angle) * radius), y + (sin(angle) * radius),0.0f); 
		} 
	glEnd(); 
} 
