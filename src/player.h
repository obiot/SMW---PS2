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
#ifndef _PLAYER_H
#define _PLAYER_H



struct SScore{
	char	*name;
	int	score;
	//int awards[PSOUND_LAST]; //multikill etc
};



//the player class - a lot of optimization can be done here (especially
//the collision detection stuff in collision_detection_map() and think() )
class CPlayer{
	public:
		CPlayer(int padNumber, int nkl, int nkr, int nkj, gfxSprite nsprites[10], SScore *nscore, bool cpu);

		void draw();
		void move();
		void cpu_think();

		void die();

		int getKills(){return score->score;};
		char *getName(){return score->name;};
		CPlayer *getNearestPlayer();

		void spawnTextWinner();
		void spawnGameOverMario();
		

	private:
		int kl, kr, kj;		//key config

		SScore *score;
		int killsinrow;
		int killsinrowinair;

		int padInputNumber;

		float x, y;		//x, y coordinate (top left of the player rectangle)
		float velx, vely;	//velocity on x, y axis
		
		int xpressed;		//0...no -1... left 1...right

		bool inair;		//true... player is in the air, false... player is on the ground
					//in air is set in CPlayer::collision_detection_map()
		bool lockjump;		//is the player allowed to jump

		bool cl,cr,cj;		//control for computer
		bool is_cpu;

		int oldx, oldy;

		gfxSprite *sprites[10];

		int spr;
		int sprswitch;
	

		void findspawnpoint();
		void collision_detection_map();

		bool isstomping(CPlayer &o);
		friend bool coldec_obj2obj(CPlayer &o1, CPlayer &o2);
		friend void collisionhandler_p2p(CPlayer &o1, CPlayer &o2);
		friend void _collisionhandler_p2p_pushback(CPlayer &o1, CPlayer &o2);

		friend class CGameMode;
		friend class CGM_DM_Fraglimit;
		friend class CGM_DM_Timelimit;
		friend class CGM_MarioWar;
		friend class CGM_CaptureTheChicken;
		friend class CKI;
};


void collisionhandler_p2p(CPlayer &o1, CPlayer &o2);
void _collisionhandler_p2p__pushback(CPlayer &o1, CPlayer &o2);

#endif

