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
#ifndef _MAP_H
#define _MAP_H

#define MAP_LIST_ENTRIES	26

static char * map_list[MAP_LIST_ENTRIES] __attribute__((aligned(64))) = {
	"0smw.map",
	"1highspeed.map",
	"2skyfight.map",
	"3blockforts.map",
	"4highabove.map",
	"5fall.map",
	"affe.map",
	"mademan_drload.map",
	"mp1_above the clouds.map",
	"mp1_blueworld.map",
	"mp1_bottle.map",
	"mp1_double-t.map",
	"mp1_friday 13..map",
	"mp1_gold-half.map",
	"mp1_hard to fly.map",
	"mp1_hello world.map",
	"mp1_highvillage.map",
	"mp1_iron.map",
	"mp1_quarterblock.map",
	"mp1_sky realm.map",
	"mp1_staircase.map",
	"mp1_synchro.map",
	"mp1_the 10th.map",
	"mp1_white ground.map",
	"scot.map",
	"Tunnel.map"
};



enum TileType{tile_nonsolid = 0, tile_solid = 1, tile_solid_on_top = 2};


class CMap{
	public:
		void loadTileSet(const char *tilesetfile, const char *tilesetbmp);
		void saveTileSet(const char *tilesetfile);
		void clearTileSet();

		void clearMap();
		void loadMap(const char *file);
		void saveMap(const char *file);

		void predraw(gfxSprite &background, gfxSprite &mapspr);

		~CMap();

		TileType map(int x, int y){			//return the tiletype at the specific position (map coordinates)
			return tileset[mapdata[x][y]];
		}

	private:
		int			mapdata[MAPWIDTH][MAPHEIGHT];		//0 to TILESTESIZE-1: tiles, TILESETSIZE: no tile

		TileType	tileset[TILESETSIZE+1];
		SDL_Surface	*tilesetsurface;
		SDL_Rect	tilebltrect;
		SDL_Rect bltrect;


		void draw(SDL_Surface *targetsurf);


		friend int editor_edit();
		friend int editor_tiles();
};



#endif


