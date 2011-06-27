#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <string.h>

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

std::string ship_gfx, ship_boost_gfx, font_file;
bool file_exists(const char * filename);
GLuint load_texture(const char* file);
void hndl_event(unsigned char key, int x, int y);

ship_t s;
GLuint ship, ship_boost;

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
	int zero = 0;
	glutInit(&zero,NULL);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow( "Rocket - Robot Sockets");

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

	ship=load_texture(ship_gfx.c_str());
	ship_boost=load_texture(ship_boost_gfx.c_str());

	glBindTexture(GL_TEXTURE_2D,0);
	active = true;
	//Set event handling in glut and start glut mainloop
	glutKeyboardFunc(&hndl_event);
}

void loop() {
	gfx_clear();
	draw_ship(s);
	gfx_update();
}

void start_event_loop() {
	glutDisplayFunc(&loop);
	glutMainLoop();
}


/*int main() {
	init_gfx(640,480);
	s.x = 100;
	s.y = 100;
	s.a = 45;
	glutDisplayFunc(loop);
	glutMainLoop();
}*/

void draw_ship(const ship_t &s) {
	if(!active) return;




	glPushMatrix();
	glTranslatef(s.x,s.y,0.0f);

	//if(s.attr[GFX_ATTR_SHOOT])

	//if(s.attr[GFX_ATTR_SCAN]) {

	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();

	glRotatef(s.a, 0.0,0.0,1.0);
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
}

void draw_highscore() {
	/*if(show_highscore && highscore.size()>0) {
		SDL_Rect text_rect;
		SDL_Surface *text_surface=0;

		text_rect.x=5;
		text_rect.y=2;

		char buffer[64];

		for(int i=0;i<std::min((int)highscore.size(),GFX_NUM_HIGHSCORE_ENTRIES);++i) {
			score_t s=highscore[i];
			sprintf(buffer,"%i. %s: %.2f",i+1,s.nick,s.score);
			text_surface=TTF_RenderText_Blended(hs_font,buffer,font_color);
			SDL_BlitSurface(text_surface,NULL,screen,&text_rect);
			SDL_FreeSurface(text_surface);
			text_rect.y+=HS_FONT_SIZE-4;
		}
	}*/
}

void stop_gfx() {
	if(!active) return;
	active=false;
}

/** 
 * Private functions
 **/

void hndl_event(unsigned char key, int x, int y) {

}

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

