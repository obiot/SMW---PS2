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
#include "global.h"
#include "romfs_io.h"
#include "sys.h"

CMap::~CMap(){
	if(tilesetsurface != NULL){
		SDL_FreeSurface(tilesetsurface);
		tilesetsurface = NULL;
	}
}


void CMap::loadTileSet(const char *tilesetfile, const char *tilesetbmp){
	
	int i;

	clearTileSet();

	//1. load tileset file
	printf("loading tile set from %s ... ", tilesetfile);

#ifdef USE_ROMFS

	int  tsf;	
	
	char * ptr;
 	
 	tsf = rfs_open((char *)tilesetfile);
 	
 	if(tsf == -1){
		printf("\n ERROR: couldn't open tileset file.\n");
		return;
	}
	
	for(i = 0; i < TILESETSIZE; i++){
		rfs_read(&(tileset[i]), sizeof(TileType));
	}
	
#else	
	FILE *tsf;
	
	tsf = fopen(tilesetfile, "rb");
	if(tsf == NULL){
		printf("\n ERROR: couldn't open tileset file.\n");
		return;
	}
	
	for(i = 0; i < TILESETSIZE; i++){
		fread(&(tileset[i]), sizeof(TileType), 1, tsf);
	}
	
	fclose(tsf);
#endif
	
	tileset[TILESETSIZE] = tile_nonsolid;	//this is the no tile selected tile

	printf("done\n");


	//2. load tileset graphics
	printf("loading tile set suface from %s ... ", tilesetbmp);

	if(tilesetsurface != NULL){
		SDL_FreeSurface(tilesetsurface);
		tilesetsurface = NULL;
	}

#ifdef USE_ROMFS

	SDL_RWops *context;
 	int fd;
 	
 	
 	fd = rfs_open((char *)tilesetbmp);
	
	ptr = (char *)malloc(sizeof(char)*(rfs_size()+1));
	
	rfs_read(ptr,rfs_size());
	
	context = SDL_RWFromMem(ptr,rfs_size()); 
	
	// Load the BMP file into a surface
	tilesetsurface = SDL_LoadBMP_RW(context,-1);
#else	

    	// Load the BMP file into a surface
	tilesetsurface = SDL_LoadBMP(tilesetbmp);
#endif
	

    if (tilesetsurface == NULL){
		printf("\n ERROR: Couldn't load %s: %s\n", tilesetbmp, SDL_GetError());
        return;
    }

	if( SDL_SetColorKey(tilesetsurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tilesetsurface->format, 255, 0, 255)) < 0){
		printf("\n ERROR: Couldn't set ColorKey + RLE: %s\n", SDL_GetError());
		return;
	}

	SDL_Surface *temp;
	if( (temp = SDL_DisplayFormat(tilesetsurface)) == NULL){
		printf("\n ERROR: couldn't convert to display format: %s\n", SDL_GetError());
		return;
	}
	SDL_FreeSurface(tilesetsurface);
	tilesetsurface = temp;

	printf("done\n");
}




void CMap::saveTileSet(const char *tilesetfile){
	FILE *mapfile;
	int i;

	printf("saving tileset %s ... ", tilesetfile);

	mapfile = fopen(tilesetfile, "wb");
	if(mapfile == NULL){
		printf("\n ERROR: couldn't open tileset file\n");
		return;
	}

	//save tileset
	for(i = 0; i < TILESETSIZE; i++){
		fwrite(&(tileset[i]), sizeof(TileType), 1, mapfile);
	}
	

	fclose(mapfile);

	printf("done\n");
}


void CMap::clearTileSet(){
	int i;
	for(i=0; i<TILESETSIZE; i++){
		tileset[i] = tile_nonsolid;
	}
	tilebltrect.w = TILESIZE;
	tilebltrect.h = TILESIZE;
}

void CMap::clearMap(){
	int i, j;

	for(j = 0; j < MAPHEIGHT; j++){
		for(i = 0; i < MAPWIDTH; i++){
			//reset tile
			mapdata[i][j] = TILESETSIZE;	//no tile selected
		}
	}
	bltrect.w = TILESIZE;
	bltrect.h = TILESIZE;
}


void CMap::loadMap(const char *file){
	
	int i, j;
	
	printf("loading map %s ... ", file);

	//clear map
	clearMap();

#ifdef USE_ROMFS

	int  mapfile;	
	
	int * ptr;
 	
 	mapfile = rfs_open((char *)file);
 	
 	if(mapfile == -1){
		printf("\n ERROR: couldn't open map\n");
		return;
	}
	
	ptr = (int *)dataptr;
	
	//2. load map data
	for(j = 0; j < MAPHEIGHT; j++){
		for(i = 0; i < MAPWIDTH; i++)
		{
			mapdata[i][j] = READ_BE_UINT32((void *)ptr);
			ptr++;
			rfs_read(&(mapdata[i][j]), sizeof(int));
		}
	}
	
#else	
	FILE *mapfile;	
	
	mapfile = fopen(file, "rb");
	if(mapfile == NULL){
		printf("\n ERROR: couldn't open map\n");
		return;
	}
	
	//2. load map data
	for(j = 0; j < MAPHEIGHT; j++){
		for(i = 0; i < MAPWIDTH; i++){
			fread(&(mapdata[i][j]), sizeof(int), 1, mapfile);
		}
	}

	fclose(mapfile);
	
#endif
	printf("done\n");
}




void CMap::saveMap(const char *file){
	FILE *mapfile;
	int i, j;

	printf("saving map %s ... ", file);

	mapfile = fopen(file, "wb");
	if(mapfile == NULL){
		printf("\n ERROR: couldn't save map\n");
		return;
	}
	
	//save map
	for(j = 0; j < MAPHEIGHT; j++){
		for(i = 0; i < MAPWIDTH; i++){
			//write index in tileset array
			fwrite(&(mapdata[i][j]), sizeof(int), 1, mapfile);
		}
	}

	fclose(mapfile);

	printf("done\n");
}




void CMap::draw(SDL_Surface *targetsurf){
	int i, j, ts;
	

	//draw left to right full vertical
	bltrect.x = 0;
	for(i = 0; i < MAPWIDTH; i++){
		bltrect.y = -TILESIZE;	//this is okay, see

		for(j = 0; j < MAPHEIGHT; j++){
			bltrect.y += TILESIZE;	//				here

			ts = mapdata[i][j];
			if(ts == TILESETSIZE)
				continue;

			
			tilebltrect.x = (ts%12)*TILESIZE;
			tilebltrect.y = (ts/12)*TILESIZE;
			

			SDL_BlitSurface(tilesetsurface, &tilebltrect, targetsurf, &bltrect);
		}

		bltrect.x += TILESIZE;
	}

	bltrect.x = 0; bltrect.y = 0; bltrect.w = 640, bltrect.h = 480;
}


void CMap::predraw(gfxSprite &background, gfxSprite &mapspr){
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 640;
	r.h = 480;

	printf("predrawing map... ");

	SDL_BlitSurface(background.getSurface(), NULL, mapspr.getSurface(), &r);
	
	draw(mapspr.getSurface());

	printf("done!\n");
}


