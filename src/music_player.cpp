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

#include "romfs.h"
#include "romfs_io.h"
#include "music_player.h"

MusicPlayer::MusicPlayer()
	: _playing(false), _music(NULL) {
}

void MusicPlayer::play(const char *musicPath) 
{
	int fd;
	char *dummy;
	if (!_playing)
	{
		
		// playing the menu music
		fd	= rfs_open((char *)musicPath);
		dummy 	= (char *)malloc(rfs_size());
		
		rfs_read(dummy, rfs_size());
		
		SDL_RWops *rw 	= SDL_RWFromMem(dummy, rfs_size());
		_music 		= Mix_LoadMUS_RW((SDL_RWops *)rw);
		
		
		// free temporary ressource
		free(dummy);
		dummy = NULL;
		
		if (_music != NULL)
		{
			Mix_PlayMusic(_music, -1);
		}
		
		_playing = true;
	}
}

void MusicPlayer::stop() 
{
	if (_playing) 
	{
		Mix_HaltMusic();
		if (_music != NULL) 
		{
			Mix_FreeMusic(_music);
			_music = 0;
		}
		_playing = false;
	}
}

