/* Super mario War
 * Copyright 2004 ® Florian Hufsky
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gfx.h"
#include "romfs_io.h"
#include <stdarg.h>		//atexit?
#include <stdlib.h>		//atexit?



extern SDL_Surface *screen;

#define GFX_BPP		16
#define GFX_FLAGS	SDL_SWSURFACE

//gfx_init
bool gfx_init(int w, int h, bool fullscreen){
	printf("init SDL\n");

#ifdef _EE	
	if( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return false;
    }
#else
	if( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO ) < 0 ) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return false;
    }
#endif
	
	


    // Clean up on exit
    atexit(SDL_Quit);
    
    if(fullscreen)
		screen = SDL_SetVideoMode(w, h, GFX_BPP, GFX_FLAGS | SDL_FULLSCREEN | SDL_DOUBLEBUF);
	else
		screen = SDL_SetVideoMode(w, h, GFX_BPP, GFX_FLAGS | SDL_DOUBLEBUF);

    if ( screen == NULL ) {
        printf("Couldn't set video mode %dx%d: %s\n", w, h, SDL_GetError());
		return false;
    }
    printf(" running @ %dx%d %dbpp (done)\n", w,h,screen->format->BitsPerPixel);	
	return true;
}

void gfx_setresolution(int w, int h, bool fullscreen){
	Uint32 flags = GFX_FLAGS;
	if(fullscreen)
		flags |= SDL_FULLSCREEN ;
	screen = SDL_SetVideoMode(w, h, GFX_BPP, flags | SDL_DOUBLEBUF);
}





//gfxSprite

gfxSprite::gfxSprite(){
	m_bltrect.x = 0;
	m_bltrect.y = 0;
	m_bltrect.w = 0;
	m_bltrect.h = 0;
	m_picture = NULL;
}

gfxSprite::~gfxSprite(){
	//free the allocated BMP surface 
	if(m_picture){
		SDL_FreeSurface(m_picture);
		m_picture = NULL;
	}
}

bool gfxSprite::init(const char *filename, Uint8 r, Uint8 g, Uint8 b)
{

	printf("loading sprite %s ... ", filename);
	

	if(m_picture != NULL){
		printf("sprite already loaded ... deleting ...");
		
		SDL_FreeSurface(m_picture);
		m_picture = NULL;
		
		printf("ok ...");
	}
	
#ifdef USE_ROMFS

	SDL_RWops *context;

	char *ptr; //ptr on datastruc file 
 	int fd;
 	
 	
 	fd = rfs_open((char *)filename);
	
	ptr = (char *)malloc(sizeof(char)*(rfs_size()+1));
	
	rfs_read(ptr,rfs_size());
	
	context = SDL_RWFromMem(ptr,rfs_size()); 
	
	// Load the BMP file into a surface
	m_picture = SDL_LoadBMP_RW(context,-1);
#else	

    	// Load the BMP file into a surface
	m_picture = SDL_LoadBMP(filename);
#endif

    if (m_picture == NULL) {
		printf("\n ERROR: Couldn't load %s: %s\n", filename, SDL_GetError());
        return false;
    }

	if( SDL_SetColorKey(m_picture, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(m_picture->format, r, g, b)) < 0){
		printf("\n ERROR: Couldn't set ColorKey + RLE for %s: %s\n", filename, SDL_GetError());
		return false;
	}

	SDL_Surface *temp;
	if( (temp = SDL_DisplayFormat(m_picture)) == NULL){
		printf("\n ERROR: couldn't convert %s to display format: %s\n", filename, SDL_GetError());
		return false;
	}
	SDL_FreeSurface(m_picture);
	m_picture = temp;


	m_bltrect.w = m_picture->w;
	m_bltrect.h = m_picture->h;

#ifdef USE_ROMFS
	free(ptr);
	ptr = NULL;
#endif
	printf("done\n");
	return true;
}

bool gfxSprite::init(const char *filename) {
	printf("loading sprite %s ... ", filename);

	if(m_picture != NULL){
		printf("sprite already loaded - deleting ...");
		
		SDL_FreeSurface(m_picture);
		m_picture = NULL;
		
		printf("ok ...");
	}
#ifdef USE_ROMFS

	SDL_RWops *context;

	char *ptr; //ptr on datastruc file 
 	int fd;
 	
 	
 	fd = rfs_open((char *)filename);
	
	ptr = (char *)malloc(sizeof(char)*(rfs_size()+1));
	
	rfs_read(ptr,rfs_size());
	
	context = SDL_RWFromMem(ptr,rfs_size()); 
	
	// Load the BMP file into a surface
	m_picture = SDL_LoadBMP_RW(context,-1);
#else	

    	// Load the BMP file into a surface
	m_picture = SDL_LoadBMP(filename);
#endif
    
    if (m_picture == NULL) {
		printf("\n ERROR: Couldn't load %s: %s\n", filename, SDL_GetError());
        return false;
    }

	SDL_Surface *temp;
	if( (temp = SDL_DisplayFormat(m_picture)) == NULL){
		printf("\n ERROR: couldn't convert %s to display format: %s\n", filename, SDL_GetError());
		return false;
	}
	SDL_FreeSurface(m_picture);
	m_picture = temp;


	m_bltrect.w = m_picture->w;
	m_bltrect.h = m_picture->h;

#ifdef USE_ROMFS
	free(ptr);
	ptr = NULL;
#endif
	
	printf("done\n");
	return true;
}



bool gfxSprite::draw(int x, int y){
	m_bltrect.x = x;
	m_bltrect.y = y;

	// Blit onto the screen surface
	if(SDL_BlitSurface(m_picture, NULL, screen, &m_bltrect) < 0){
		fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
		return false;
	}
	return true;
}




// gfxFont
bool gfxFont::init(const char *filename){
	printf("loading font %s ... ", filename);

#ifdef USE_ROMFS

	SDL_RWops *context;

	char *ptr; //ptr on datastruc file 
 	int fd;
 	
 	
 	fd = rfs_open((char *)filename);
	
	ptr = (char *)malloc(sizeof(char)*(rfs_size()+1));
	
	rfs_read(ptr,rfs_size());
	
	context = SDL_RWFromMem(ptr,rfs_size()); 
	
	SDL_Surface *fontsurf = SDL_LoadBMP_RW(context, -1);
#else	

    	SDL_Surface *fontsurf = SDL_LoadBMP(filename);
#endif
	
	if(fontsurf == NULL){
		printf("\n ERROR: couldn't load file %s: %s\n", filename, SDL_GetError());
		return false;
	}

	m_font = SFont_InitFont(fontsurf);
	if(!m_font) {
		printf("\n ERROR: an error occured while loading the font.\n");
		return false;
	}

#ifdef USE_ROMFS
	free(ptr);
	ptr = NULL;
#endif

	printf("done\n");
	return true;
}

void gfxFont::draw(int x, int y, const char *s){
	if(y >= 0)
		SFont_Write(screen, m_font, x, y, s);
}

void gfxFont::drawCentered(int y, const char *text){
	if(y >= 0)
		SFont_WriteCenter(screen, m_font, y, text);
};


void gfxFont::drawf(int x, int y, char *s, ...){
	char buffer[256];

	va_list zeiger;
	va_start(zeiger, s);
	vsprintf(buffer, s, zeiger);
	va_end(zeiger);

	draw(x,y,buffer);
}


