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
 
#ifndef __MOD_PLAYER_H__
#define __MOD_PLAYER_H__

#include <SDL_Mixer.h>

struct MusicPlayer {

	MusicPlayer();
	void play(const char *musicPath);
	void stop();

	private:
	bool _playing;
	Mix_Music *_music;
};

#endif // __MOD_PLAYER_H__
