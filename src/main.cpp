/*--------------------------------------------------------------+
| super mario war					    	|
|							    	|
| a mario war clone written using the tile based collision  	|
| detection technique explained in jnrdev #1			|
|								|
| you can read the tutorial on http://jnrdev.weed-crew.net	|
|								|
|								|
| this sourcecode is released under the GPL.			|
|								|
|								|
| the code is a bit messy and contains a few things i'm not	|
| really happy with. (menu, ...)				|
| but it works and the game is fun playing :)			|
|								|
| and some of the code in CPlayer::collision_detection_map()	|
| should be optimized. and i think there may be some minor	|
| bugs in those functions.					|
|								|
| maybe put all the game relevant stuff in a CGame class	|
| when adding a menu.						|
|								|
|								|
| start:		24.01.2003				|
| last changes:	25.04.2004					|
|								|
|					 2004 © Florian Hufsky  |
|					      fhufsky@phorus.at	|
|				http://jnrdev.weed-crew.net	|
+---------------------------------------------------------------*/


#include <string.h>
#include "global.h"				//all the global stuff
#include "music_player.h"


//now it's really time for an "engine" (aka resource manager)
//>#include "fmod.h"

#define TITLESTRING "Super Mario War v1.10"

#ifdef USE_ROMFS
#include "romfs.h"
#include "romfs_io.h"
#endif


//------ system stuff ------
Uint8			*keystates;
SDL_Surface		*screen;		//for gfx (maybe the gfx system should be improved -> resource manager)



//------ sprites (maybe this should be done in a resource manger) ------
gfxSprite		spr_player[4][PGFX_LAST];	//all player sprites (see global.h)
gfxSprite		spr_clouds[2];
gfxSprite		spr_background;
gfxSprite		spr_map;

gfxFont			font[3];

gfxSprite		menu_smw;
gfxSprite		menu_mario;
gfxSprite		menu_cpu;

gfxSprite		spr_chicken;



//------ game relevant stuff ------
CPlayer			*list_players[4];
int			list_players_cnt = 0;
bool			showscoreboard;
SScore			score[4] = { {"cursor", 0},
				     {"awd", 0}, 
				     {"huk", 0},
				     {"mouse/NUM", 0} };
				     
int			score_cnt;

CMap			map;
CEyecandyContainer 	eyecandy;

static MusicPlayer	musicPlayer;


//these things shake the screen when jumping on an enemy (ugly naming)
short			x_shake = 0;
short			y_shake = 0;
bool			y_crunch = false;
float			y_velocity;
float			y_height;		//used because SDL_Rect contains unsigned values - that's bad!
SDL_Rect		y_rect;


int			mostkills;		//for highlighting the player with the most kills

SAward awards[PSOUND_LAST] = { {"doublekill", NULL, &font[0]},
			       {"multikill", NULL, &font[0]},
			       {"killingspree", NULL, &font[0]},
			       {"ultrakill", NULL, &font[1]},
			       {"rampage", NULL, &font[1]},
			       {"monsterkill", NULL, &font[2]},
			       {"dominating", NULL, &font[2]},
			       {"godlike", NULL, &font[2]},
			       {"unstoppable", NULL, &font[2]} };

Mix_Chunk *stoprow;
Mix_Chunk *mip;
Mix_Chunk *jump;
Mix_Chunk *excellent;
Mix_Chunk *prepare;
Mix_Chunk *stage_clear;


CGameMode		*gamemodes[GAMEMODE_LAST];
int			currentgamemode = 0;


//map list in directory
struct mlentry{
	char	*file;
	mlentry	*next;
	mlentry	*prev;

	mlentry(char *name)
	{
		file = new char[strlen(name)+strlen("maps/")+1];
		sprintf(file, "maps/%s", name);
		next = NULL;
		prev = NULL;
	};

	~mlentry(){
		delete file;
	};
};

class MapList{
	public:
		MapList();
		~MapList();

		void next(){current = current->next;};
		void prev(){current = current->prev;};
		const char *current_name(){return current->file;};

	private:
		mlentry *head;
		mlentry *current;
};

MapList maplist;



//------ functions ------
void crunch_screen(){
	if(y_rect.h < 220){	//prevent too far crunching
		y_crunch = true;
		y_velocity = CRUNCHSTART;
	}
}

void rungame();
void runmenu();
void cleanup();
bool load_and_splashscreen();




gv game_values;


// ------ MAIN ------
int main(int argc, char *argv[]){
	bool loadok;

	printf("-------------------------------------------------------------------------------\n");
	printf(" %s\n", TITLESTRING);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n---------------- startup ----------------\n");

#ifdef _EE	
	gfx_init(640, 480, true);		//initialize the graphics (SDL)
#else
	gfx_init(640, 480, false);		//initialize the graphics (SDL)
#endif

	//Joystick-Init
	SDL_Joystick **joysticks;
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	int jcount = SDL_NumJoysticks();
	joysticks = new SDL_Joystick*[jcount];
	
	for(int i = 0; i < jcount; i++)
		joysticks[i] = SDL_JoystickOpen(i);
		
	SDL_JoystickEventState(SDL_ENABLE);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2 , 2048);

	Mix_AllocateChannels(16);

	keystates = SDL_GetKeyState(0);	//get key array

	//currently this only sets the title, not the icon.
	//setting the icon isn't implemented in sdl ->  i'll ask on the mailing list
#ifndef _EE
	SDL_WM_SetCaption(TITLESTRING, "smw.ico");
#endif
	SDL_ShowCursor(SDL_DISABLE);


	printf("\n---------------- loading ----------------\n");
	
	//set standard game values
	game_values.players			= 2;
	game_values.cpu				= 0;
	game_values.showfps			= false;
	game_values.sound			= true;
	game_values.gamestate			= GS_MENU;
	game_values.NUMmousecontrol 		= false;
#ifdef _EE
	game_values.fullscreen			= true;
#else
	game_values.fullscreen			= false;
#endif
	game_values.screenResize    		= 20;
	
	
	//set game modes
	gamemodes[0] = new CGM_DM_Fraglimit;
	gamemodes[1] = new CGM_DM_Timelimit;
	gamemodes[2] = new CGM_MarioWar;
	gamemodes[3] = new CGM_CaptureTheChicken;
	gamemodes[4] = new CGameMode;

	currentgamemode = 0;
	game_values.gamemode		= gamemodes[currentgamemode];


	#ifdef USE_ROMFS
	extern unsigned char romdisk_start[]; 
  	romdisk_mount(romdisk_start);
  	rfs_init();
  	#endif

	loadok = load_and_splashscreen();

	if(!loadok){
		printf("\n---------------- ERROR DURING LOADING - SHUTDOWN ----------------\n\n");
		Mix_CloseAudio();
		return 0;
	}
	printf("\n---------------- ready, steady, go! ----------------\n");
	runmenu();
	while(game_values.gamestate != GS_QUIT){
		srand(SDL_GetTicks());	//now we've got a random seed - we can't know how much time the player spends in the splash screen
		switch(game_values.gamestate){
			case GS_GAME:
				rungame();
				break;
			case GS_MENU:
				runmenu();
				break;
			case GS_QUIT:
				break;
		}
	}
	
	printf("\n---------------- shutdown ----------------\n");
	for(int i=0;i<GAMEMODE_LAST;i++)
		delete gamemodes[i];
	
	//sounds get automatically freed by fmod
	for(int i=0; i<jcount; i++)
		SDL_JoystickClose(joysticks[i]);
	delete[] joysticks;

        Mix_CloseAudio();
	return 0;
}



//-----------------------------------------------------------------------------
// THE GAME LOOP
//-----------------------------------------------------------------------------

bool coldec_obj2obj(CPlayer &o1, CPlayer &o2);

void rungame(){
	unsigned int	framestart;
	SDL_Event	event;
	int		i,j;
	float		realfps = 0, flipfps=0;
	Uint8		mouse;
	int		mousex;
	int		mousey;
	
	//the controls should be modifieable!
	if(game_values.players >= 1)
		list_players[list_players_cnt++] = new CPlayer(0, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, spr_player[0], &(score[0]), game_values.cpu >= 4);
	if(game_values.players >= 2)
		list_players[list_players_cnt++] = new CPlayer(1, SDLK_a, SDLK_d, SDLK_w, spr_player[1], &(score[1]), game_values.cpu >= 3);
	if(game_values.players >= 3)
		list_players[list_players_cnt++] = new CPlayer(2, SDLK_h, SDLK_k, SDLK_u, spr_player[2], &(score[2]), game_values.cpu >= 2);
	if(game_values.players >= 4)
		list_players[list_players_cnt++] = new CPlayer(3, SDLK_c, SDLK_v, SDLK_b, spr_player[3], &(score[3]), game_values.cpu >= 1);

	score_cnt = list_players_cnt;

	
	game_values.gamemode->init();

	showscoreboard = false;
	
	for(i =0; i < 4; i++){
		int c = rand()%2;			//cloud type (0... small cloud, 1... big cloud)
		int velx;				//speed of cloud, small clouds are slower than big ones
		if(c == 0)
			velx = ((rand()%60-30))/10;	//big clouds: -3 - +3 pixel/frame
		else
			velx = ((rand()%40-20))/10;	//small clouds: -2 - +2 pixel/frame
		
		velx = velx == 0? 1: velx;		//no static clouds please

		//add cloud to eyecandy array
		eyecandy.add(new EC_Cloud(&spr_clouds[c], (float)(rand()%640), (float)(rand()%100), (float)velx));
	}
	
	y_rect.x = 0;
	y_rect.y = 0;
	y_rect.w = 640;
	y_rect.h = 0;

	y_velocity	= 0.0f;
	y_height	= 0.0f;
	
	mostkills = 1;
	
	// stop current music
	musicPlayer.stop();
	// play ingame music
	if(game_values.sound)
	  musicPlayer.play((char *)"music/Level.mp2");

	bool quitGame = false;
	while (true){
#ifdef _EE		
		// fix some weird RPC issue
		printf("");
#endif
		framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event)){
			if(quitGame) 
			{
				// stop current music
				musicPlayer.stop();

				quitGame = false;
				cleanup();
				game_values.gamestate = GS_MENU;
				return;
			}
			switch(event.type){

				case SDL_QUIT:
					cleanup();
					game_values.gamestate = GS_QUIT;
					return;
					break;

				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE){
						cleanup();
						game_values.gamestate = GS_MENU;
						return;
					}
					else if(event.key.keysym.sym == SDLK_F1){
						game_values.showfps = !game_values.showfps;
					}
					break;

				case SDL_JOYHATMOTION:
					if(event.jhat.which == 0) {
						if (event.jhat.value & SDL_HAT_LEFT)
							keystates[SDLK_LEFT] = 1;
						else
							keystates[SDLK_LEFT] = 0;
						if (event.jhat.value & SDL_HAT_RIGHT)
							keystates[SDLK_RIGHT] = 1;
						else
							keystates[SDLK_RIGHT] = 0;
						if (event.jhat.value & SDL_HAT_UP)
							keystates[SDLK_UP] = 1;
						else
							keystates[SDLK_UP] = 0;
					} else if(event.jhat.which == 1) {
						if (event.jhat.value & SDL_HAT_LEFT)
							keystates[SDLK_a] = 1;
						else
							keystates[SDLK_a] = 0;
						if (event.jhat.value & SDL_HAT_RIGHT)
							keystates[SDLK_d] = 1;
						else
							keystates[SDLK_d] = 0;
						if (event.jhat.value & SDL_HAT_UP)
							keystates[SDLK_w] = 1;
						else
							keystates[SDLK_w] = 0;
					} else if(event.jhat.which == 2) {
						if (event.jhat.value & SDL_HAT_LEFT)
							keystates[SDLK_h] = 1;
						else
							keystates[SDLK_h] = 0;
						if (event.jhat.value & SDL_HAT_RIGHT)
							keystates[SDLK_k] = 1;
						else
							keystates[SDLK_k] = 0;
						if (event.jhat.value & SDL_HAT_UP)
							keystates[SDLK_u] = 1;
						else
							keystates[SDLK_u] = 0;
					} else if(event.jhat.which == 3) {
						if (event.jhat.value & SDL_HAT_LEFT)
							keystates[SDLK_c] = 1;
						else
							keystates[SDLK_c] = 0;
						if (event.jhat.value & SDL_HAT_RIGHT)
							keystates[SDLK_v] = 1;
						else
							keystates[SDLK_v] = 0;
						if (event.jhat.value & SDL_HAT_UP)
							keystates[SDLK_b] = 1;
						else
							keystates[SDLK_b] = 0;
					}
					break;
				 case SDL_JOYAXISMOTION:
				  	{
				    
					    if (event.jaxis.axis == 0) 
					    { 
					      /* left-right */
					      if (event.jaxis.value < -SYSJOY_RANGE)  /* left */
					      {  
					      	 if(event.jbutton.which == 0)
							keystates[SDLK_LEFT] = 1;
						 else if(event.jbutton.which == 1)
							keystates[SDLK_a] = 1;
						 else if(event.jbutton.which == 2)
							keystates[SDLK_h] = 1;
						 else if(event.jbutton.which == 3)
							keystates[SDLK_c] = 1;
					        	
					      }
					      else
					      {
					      	 keystates[SDLK_LEFT] 	= 0;
					      	 keystates[SDLK_a] 	= 0;
					      	 keystates[SDLK_h] 	= 0;
					      	 keystates[SDLK_c] 	= 0;
					      }
					      
					      if (event.jaxis.value > SYSJOY_RANGE) /* right */
					      {  
						 if(event.jbutton.which == 0)
							keystates[SDLK_RIGHT] = 1;
						 else if(event.jbutton.which == 1)
							keystates[SDLK_d] = 1;
						 else if(event.jbutton.which == 2)
							keystates[SDLK_k] = 1;
						 else if(event.jbutton.which == 3)
							keystates[SDLK_v] = 1;
					      }
					      else 
					      {
					      	 keystates[SDLK_RIGHT] 	= 0;
					      	 keystates[SDLK_d] 	= 0;
					      	 keystates[SDLK_k] 	= 0;
					      	 keystates[SDLK_v] 	= 0;
					      }
					    }
					    
					    if (event.jaxis.axis == 1) 
					    {  /* up-down */
					      if (event.jaxis.value < -SYSJOY_RANGE) /* up */
					      {  
					       	 if(event.jbutton.which == 0)
							keystates[SDLK_UP] = 1;
						 else if(event.jbutton.which == 1)
							keystates[SDLK_w] = 1;
						 else if(event.jbutton.which == 2)
							keystates[SDLK_u] = 1;
						 else if(event.jbutton.which == 3)
							keystates[SDLK_b] = 1;
					      }
					      else
					      {
					      	 keystates[SDLK_UP] 	= 0;
					      	 keystates[SDLK_w] 	= 0;
					      	 keystates[SDLK_u] 	= 0;
					      	 keystates[SDLK_b] 	= 0;
					      } 
					    
					    }
					}
				    	break; 
				case SDL_JOYBUTTONDOWN:
					if(event.jbutton.button == 4) // SELECT
						quitGame = true;
					if(event.jbutton.button == 1) { // [X]
						if(event.jbutton.which == 0)
							keystates[SDLK_UP] = 1;
						else if(event.jbutton.which == 1)
							keystates[SDLK_w] = 1;
						else if(event.jbutton.which == 2)
							keystates[SDLK_u] = 1;
						else if(event.jbutton.which == 3)
							keystates[SDLK_b] = 1;
					}
					break;
				case SDL_JOYBUTTONUP:
					if(event.jbutton.button == 1) {
						if(event.jbutton.which == 0)
							keystates[SDLK_UP] = 0;
						else if(event.jbutton.which == 1)
							keystates[SDLK_w] = 0;
						else if(event.jbutton.which == 2)
							keystates[SDLK_u] = 0;
						else if(event.jbutton.which == 3)
							keystates[SDLK_b] = 0;
					}
					break;

				default:
					break;
			}
		}

	
		if(game_values.NUMmousecontrol){
			mouse = SDL_GetMouseState(&mousex, &mousey);
			if(mouse & SDL_BUTTON(1)  && mouse & SDL_BUTTON(3))
				keystates[SDLK_KP5] = 1;
			else{
				keystates[SDLK_KP5] = 0;
				if(mouse & SDL_BUTTON(1))
					keystates[SDLK_KP1] = 1;
				else
					keystates[SDLK_KP1] = 0;
				if(mouse & SDL_BUTTON(3))
					keystates[SDLK_KP3] = 1;
				else
					keystates[SDLK_KP3] = 0;
			}
		}



		//------------- update objects -----------------------	
		for(i=0;i<list_players_cnt;i++)
			list_players[i]->move();	//move all objects before doing object-object collision detection in
										//->think(), so we test against the new position after object-map collision detection

		for(i=0;i<list_players_cnt;i++){
			for(j=i+1; j < list_players_cnt; j++){
				if(coldec_obj2obj(*(list_players[i]), *(list_players[j]))){
					collisionhandler_p2p(*(list_players[i]), *(list_players[j]));
				}
			}
		}

		//crunch the screen :)
		if(y_crunch){
			y_velocity -= SCREENCRUNCH;

			y_height += y_velocity;
			y_rect.h = (int)y_height;

			if(y_height < 0.0f){	//stop crunching the screen
				y_height = 0;
				y_rect.h = 0;
				y_velocity = 0;
				y_crunch = false;
			}
		}

		
		eyecandy.update();
		game_values.gamemode->think();
		



		//--------------- draw everything ----------------------
		if(y_crunch)
			SDL_FillRect(screen, &y_rect, 0x0);		//fill empty area with black

		spr_map.draw(0,0 + y_rect.h);
		
		

		for(i=0;i<list_players_cnt;i++)
			list_players[i]->draw();

		eyecandy.draw();

		game_values.gamemode->draw();

		
		
		//scoreboard
		for(i=0; i<list_players_cnt; i++){
			int f=0;
			if(list_players[i]->getKills() >= mostkills){	//highlight player with most kills
				mostkills = list_players[i]->getKills();
				f=1;
			}
			font[f].drawf(0, i*font[f].getHeight() + y_rect.h, "%3d %s", list_players[i]->getKills(), list_players[i]->getName());
		}
		

		//big endgame scoreboard (sorted)
		if(showscoreboard){
			int max;
			const int sx = 230;
			const int sy = 200;
			bool draw[4] = {false, false, false, false};

			font[2].drawf(sx, sy-font[2].getHeight(), "game over");

			
			for(i=0; i<score_cnt; i++){
				//find biggest score in remaining scores
				max = -1;
				for(j = 0; j < score_cnt; j++){
					if(!draw[j]){	//only if this score wasn't drawn before
						if(max == -1)
							max = j;
						else if(score[j].score >= score[max].score)
							max = j;
					}
				}
				//draw this score
				font[2].drawf(sx, sy+i*font[2].getHeight(), "%3d %s", score[max].score, score[max].name);
				draw[max] = true;	//this is the next biggest score - it doesn't belong to the remaining scores from now on
			}
		}


		if(game_values.showfps) font[0].drawf(0, 480-font[0].getHeight(), "fps: real:%.1f/%.1f, flip:%.1f, frameonly:%.1f", realfps, (float)(1000/WAITTIME), flipfps, (float)1000/(SDL_GetTicks() - framestart));
		

		

		//double buffering -> flip buffers
		SDL_Flip(screen);
		flipfps = (float)(1000/(SDL_GetTicks() - framestart));

		while((SDL_GetTicks()-framestart) < WAITTIME);	//keep framerate constant at 1000/WAITTIME fps

		realfps = (float)(1000/(SDL_GetTicks() - framestart));
	}

	//we never get here
	
	return;
}


void cleanup(){
	int i;
	//delete object list
	for(i=0;i<list_players_cnt;i++){
		delete list_players[i];
	}
	list_players_cnt = 0;

	eyecandy.clean();
}


bool coldec_obj2obj(CPlayer &o1, CPlayer &o2){
	int l1 = int(o1.x);		//left
	int r1 = int(o1.x+PW);	//right
	int t1 = int(o1.y);		//top
	int b1 = int(o1.y+PH);	//bottom

	int l2 = int(o2.x);
	int r2 = int(o2.x+PW);
	int t2 = int(o2.y);
	int b2 = int(o2.y+PH);

	//dr bobb's rect-rect collision detection
	//http://www.ddj.com/documents/s=983/ddj9513a/
	if (l1 > r2 || l2 > r1 ||
		t1 > b2 || t2 > b1) {
		return false;
	}
	else {
		return true;
	}
	return false;
}






//-----------------------------------------------------------------------------
// THE LOAD UP SEQUENCE + SPLASH SCREEN
//-----------------------------------------------------------------------------
//that's a bunch of ugly code, maybe i'll throw it out again


void _load_drawmsg(const char *f){
	static SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 640;
	r.h = font[0].getHeight();
	Uint32 col = SDL_MapRGB(screen->format, 208, 216, 248);

	SDL_FillRect(screen, &r, col);		//fill empty area with black
	font[0].draw(0,0, f);
	//SDL_Flip(screen);
	SDL_UpdateRect(screen, 0,0,640,font[0].getHeight());
}
void _load_waitforkey()
{
	
	SDL_Event event;
	while (true){
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_KEYDOWN)
				return;
			if(event.type == SDL_JOYBUTTONDOWN)
				return;
		}
		SDL_Delay(10);
	}
	
}
bool __load_gfxck(gfxSprite &g, const char *f){
	if(! g.init(f, 255,0,255) ){
		char msg[512];								//todo: welcome to buffer overflow hell :) (better -> snprintf or CString)
		sprintf(msg, "error loading colorykeysprite %s", f);
		_load_drawmsg(msg);
		return false;
	}
	else{
		_load_drawmsg(f);
		return true;
	}
}
bool __load_gfx(gfxSprite &g, const char *f){
	if(! g.init(f)){
		char msg[512];								//todo: welcome to buffer overflow hell :) (better -> snprintf or CString)
		sprintf(msg, "error loading sprite %s", f);
		_load_drawmsg(msg);
		return false;
	}
	else{
		_load_drawmsg(f);
		return true;
	}
}
bool __load_sfx(Mix_Chunk *&s, const char *f){


	printf("loading sfx %s ... ", f);
#ifdef USE_ROMFS
 	
 	SDL_RWops *context;
 	
 	char *ptr; //ptr on datastruc file 
 	int fd;
 	
 	
 	fd = rfs_open((char *)f);
	
	ptr = (char *)malloc(sizeof(char)*(rfs_size()+1));
	
	rfs_read(ptr,rfs_size());
	
	context = SDL_RWFromMem(ptr,rfs_size());
  	
 	s = Mix_LoadWAV_RW(context, 1);
#else

	s = Mix_LoadWAV(f);
#endif

	if(!s){
		char msg[512];								//todo: welcome to buffer overflow hell :) (better -> snprintf or CString)
		sprintf(msg, "error loading sound %s", f);
		_load_drawmsg(msg);
		return false;
	}
	else{
		_load_drawmsg(f);
		printf("done\n");
		return true;
	}
	
}

#define _load_gfxck(g,f)	if(!__load_gfxck(g,f)){_load_waitforkey();return false;};
#define _load_gfx(g,f)		if(!__load_gfx(g,f)){_load_waitforkey();return false;};
#define _load_sfx(s,f)		if(!__load_sfx(s,f)){_load_waitforkey();return false;};

char *convertPath(char *source) 
{
#ifndef USE_ROMFS
	static char convertedString[512];
	strcpy(convertedString, "host:"); //??
	strcat(convertedString, source);
	return convertedString;
#else
	return source;
#endif
}


bool load_and_splashscreen(){
	bool loadok = true;

	loadok &= font[0].init(convertPath("gfx/fonts/font0.bmp"));
	loadok &= font[1].init(convertPath("gfx/fonts/font1.bmp"));
	loadok &= font[2].init(convertPath("gfx/fonts/font2.bmp"));

	if(!loadok){
		_load_drawmsg("ERROR: error loading the fonts!\n");
		_load_waitforkey();
		return false;
	}

	//load basic stuff
	_load_gfxck(menu_smw, convertPath("gfx/menu_smw.bmp"));
	gfxSprite splash;
	_load_gfx(splash, convertPath("gfx/splash.bmp"));


	//draw the splash screen
	splash.draw(0,0);
	menu_smw.draw(320- ((menu_smw.getWidth())>>1), 30);	//smw logo

	const int f = 0;
	int dy = 180 - font[f].getHeight();
	int dx = 130;

	font[f].draw(dx,dy+=font[f].getHeight(), TITLESTRING);
	font[f].draw(dx,dy+=font[f].getHeight(), "http://jnrdev.weed-crew.net/smw");
	dy+=font[f].getHeight();
	font[f].draw(dx,dy+=font[f].getHeight(), "gfx: Nintendo");
	font[f].draw(dx,dy+=font[f].getHeight(), "sfx: Epic, ID Software, Florian Hufsky");
	font[f].draw(dx,dy+=font[f].getHeight(), "original idea: Samuele Poletti");
	dy+=font[f].getHeight();
	font[f].draw(dx,dy+=font[f].getHeight(), "code:");
	font[f].draw(dx,dy+=font[f].getHeight(), "  Florian Hufsky aka no_skill (main smw code)");
	font[f].draw(dx,dy+=font[f].getHeight(), "  Mario Piuk aka MPXL (game modes)");
	font[f].draw(dx,dy+=font[f].getHeight(), "  Evilo & The Froggies (PS2 Port)");
	font[f].draw(dx,dy+=font[f].getHeight(), "     http://www.psxdev.org/");
	dy+=font[f].getHeight();
	font[f].draw(dx,dy+=font[f].getHeight(), "thanks to all who supported me and to you for playing.");
	font[f].draw(dx,dy+=font[f].getHeight(), "have a lot of fun!");
	

	SDL_Flip(screen);
	
	
	//should be solved better (parse a player sprite set file, maybe +everything in a single surface + offsets)
	_load_gfxck(spr_player[0][PGFX_STANDING_R], convertPath("gfx/m1/p_standing_r.bmp"));
	_load_gfxck(spr_player[0][PGFX_STANDING_L], convertPath("gfx/m1/p_standing_l.bmp"));
	_load_gfxck(spr_player[0][PGFX_RUNNING_R], convertPath("gfx/m1/p_running_r.bmp"));
	_load_gfxck(spr_player[0][PGFX_RUNNING_L], convertPath("gfx/m1/p_running_l.bmp"));
	_load_gfxck(spr_player[0][PGFX_JUMPING_R], convertPath("gfx/m1/p_jumping_r.bmp"));
	_load_gfxck(spr_player[0][PGFX_JUMPING_L], convertPath("gfx/m1/p_jumping_l.bmp"));
	_load_gfxck(spr_player[0][PGFX_STOPPING_R], convertPath("gfx/m1/p_stopping_r.bmp"));
	_load_gfxck(spr_player[0][PGFX_STOPPING_L], convertPath("gfx/m1/p_stopping_l.bmp"));
	_load_gfxck(spr_player[0][PGFX_DEADFLYING], convertPath("gfx/m1/p_deadflying.bmp"));
	_load_gfxck(spr_player[0][PGFX_DEAD], convertPath("gfx/m1/p_dead.bmp"));

	_load_gfxck(spr_player[1][PGFX_STANDING_R], convertPath("gfx/m2/p_standing_r.bmp"));
	_load_gfxck(spr_player[1][PGFX_STANDING_L], convertPath("gfx/m2/p_standing_l.bmp"));
	_load_gfxck(spr_player[1][PGFX_RUNNING_R], convertPath("gfx/m2/p_running_r.bmp"));
	_load_gfxck(spr_player[1][PGFX_RUNNING_L], convertPath("gfx/m2/p_running_l.bmp"));
	_load_gfxck(spr_player[1][PGFX_JUMPING_R], convertPath("gfx/m2/p_jumping_r.bmp"));
	_load_gfxck(spr_player[1][PGFX_JUMPING_L], convertPath("gfx/m2/p_jumping_l.bmp"));
	_load_gfxck(spr_player[1][PGFX_STOPPING_R], convertPath("gfx/m2/p_stopping_r.bmp"));
	_load_gfxck(spr_player[1][PGFX_STOPPING_L], convertPath("gfx/m2/p_stopping_l.bmp"));
	_load_gfxck(spr_player[1][PGFX_DEADFLYING], convertPath("gfx/m2/p_deadflying.bmp"));
	_load_gfxck(spr_player[1][PGFX_DEAD], convertPath("gfx/m2/p_dead.bmp"));

	_load_gfxck(spr_player[2][PGFX_STANDING_R], convertPath("gfx/m3/p_standing_r.bmp"));
	_load_gfxck(spr_player[2][PGFX_STANDING_L], convertPath("gfx/m3/p_standing_l.bmp"));
	_load_gfxck(spr_player[2][PGFX_RUNNING_R], convertPath("gfx/m3/p_running_r.bmp"));
	_load_gfxck(spr_player[2][PGFX_RUNNING_L], convertPath("gfx/m3/p_running_l.bmp"));
	_load_gfxck(spr_player[2][PGFX_JUMPING_R], convertPath("gfx/m3/p_jumping_r.bmp"));
	_load_gfxck(spr_player[2][PGFX_JUMPING_L], convertPath("gfx/m3/p_jumping_l.bmp"));
	_load_gfxck(spr_player[2][PGFX_STOPPING_R], convertPath("gfx/m3/p_stopping_r.bmp"));
	_load_gfxck(spr_player[2][PGFX_STOPPING_L], convertPath("gfx/m3/p_stopping_l.bmp"));
	_load_gfxck(spr_player[2][PGFX_DEADFLYING], convertPath("gfx/m3/p_deadflying.bmp"));
	_load_gfxck(spr_player[2][PGFX_DEAD], convertPath("gfx/m3/p_dead.bmp"));

	_load_gfxck(spr_player[3][PGFX_STANDING_R], convertPath("gfx/m4/p_standing_r.bmp"));
	_load_gfxck(spr_player[3][PGFX_STANDING_L], convertPath("gfx/m4/p_standing_l.bmp"));
	_load_gfxck(spr_player[3][PGFX_RUNNING_R], convertPath("gfx/m4/p_running_r.bmp"));
	_load_gfxck(spr_player[3][PGFX_RUNNING_L], convertPath("gfx/m4/p_running_l.bmp"));
	_load_gfxck(spr_player[3][PGFX_JUMPING_R], convertPath("gfx/m4/p_jumping_r.bmp"));
	_load_gfxck(spr_player[3][PGFX_JUMPING_L], convertPath("gfx/m4/p_jumping_l.bmp"));
	_load_gfxck(spr_player[3][PGFX_STOPPING_R], convertPath("gfx/m4/p_stopping_r.bmp"));
	_load_gfxck(spr_player[3][PGFX_STOPPING_L], convertPath("gfx/m4/p_stopping_l.bmp"));
	_load_gfxck(spr_player[3][PGFX_DEADFLYING], convertPath("gfx/m4/p_deadflying.bmp"));
	_load_gfxck(spr_player[3][PGFX_DEAD], convertPath("gfx/m4/p_dead.bmp"));

	_load_gfxck(spr_clouds[0], convertPath("gfx/cloud1.bmp"));
	_load_gfxck(spr_clouds[1], convertPath("gfx/cloud2.bmp"));	

	_load_gfxck(spr_chicken, convertPath("gfx/chicken.bmp"));


	_load_sfx(awards[0].sound, convertPath("sfx/doublekill.wav"));
	_load_sfx(awards[1].sound, convertPath("sfx/multikill.wav"));
	_load_sfx(awards[2].sound, convertPath("sfx/killingspree.wav"));
	_load_sfx(awards[3].sound, convertPath("sfx/ultrakill.wav"));
	_load_sfx(awards[4].sound, convertPath("sfx/rampage.wav"));
	_load_sfx(awards[5].sound, convertPath("sfx/monsterkill.wav"));
	_load_sfx(awards[6].sound, convertPath("sfx/dominating.wav"));
	_load_sfx(awards[7].sound, convertPath("sfx/godlike.wav"));
	_load_sfx(awards[8].sound, convertPath("sfx/unstoppable.wav"));
	_load_sfx(stoprow, convertPath("sfx/perfect.wav"));
	_load_sfx(mip, convertPath("sfx/mip.wav"));
	_load_sfx(jump, convertPath("sfx/jump.wav"));
	_load_sfx(excellent, convertPath("sfx/excellent.wav"));
	_load_sfx(prepare, convertPath("sfx/prepare.wav"));
	//_load_sfx(stage_clear, convertPath("music/stageclear.wav"));


	_load_gfx(spr_background, convertPath("gfx/bg.bmp"));
	_load_gfx(spr_map, convertPath("gfx/bg.bmp"));

	_load_gfxck(menu_mario, convertPath("gfx/menu_mario.bmp"));
	_load_gfxck(menu_cpu, convertPath("gfx/menu_cpu.bmp"));

	_load_drawmsg("loading tileset gfx/tileset.bmp & maps/tileset.tls");
	char tileSetTLS[128];
	strcpy(tileSetTLS, convertPath("maps/tileset.tls"));
	char tileSetBMP[128];
	strcpy(tileSetBMP, convertPath("gfx/tileset.bmp"));
	map.loadTileSet(tileSetTLS, tileSetBMP);

	_load_drawmsg(maplist.current_name());
	map.loadMap(maplist.current_name());			//this must be called before initializing any players, because the player constructor needs a loaded map to see where the player can spawn
	map.predraw(spr_background, spr_map);

		
	_load_drawmsg("done - press any key to continue");
//		vid_screen_shot("/pc/menupict.pic");
	_load_waitforkey();

	return loadok;
}
















//-----------------------------------------------------------------------------
// THE NOW LESS STATIC BUT STILL WEIRD MENU
//-----------------------------------------------------------------------------

//i rewrote the whole menu code, but it's still a mess..,
//but it's now easier to add new menu items

#define BOOL2ONOFF(x) ( (x) == true ? "on" : "off" )


bool menu_startgame(){
	game_values.gamestate = GS_GAME;
	if(game_values.sound)
		Mix_PlayChannel(-1, prepare, 0);
	return true;
}
bool menu_exit(){
	game_values.gamestate = GS_QUIT;
	return true;
}


//---------------------------------------------------------------
// MAIN MENU
//---------------------------------------------------------------

class MenuItem{
	public:
		virtual bool left(){return false;};
		virtual bool right(){return false;};
		virtual bool enter(){return false;};
		virtual int draw(const int x, const int y){return y;};

		MenuItem* next(){return mnext;};
		MenuItem* prev(){return mprev;};

		void setnext(MenuItem *m){ m->mprev = this; mnext = m;};

	private:
		MenuItem* mprev;
		MenuItem* mnext;
};

class MI_Start : public MenuItem{
	public:
		bool enter(){	return menu_startgame();};
		int draw(const int x, const int y){
			font[2].draw(x, y, "start");
			return y + font[2].getHeight()*2;
		};
};

class MI_Players : public MenuItem{
	public:
		bool left(){
			if(game_values.players == 4){
				if(game_values.cpu == 0)
					game_values.players--;
				else
					game_values.cpu--;
			}
			else{
				if(game_values.players > 2)
					game_values.players--;
			}
			return false;
		};
		bool right(){
			if(game_values.players == 4){
				if(game_values.cpu < 4)
					game_values.cpu++;
			}
			else{
				if(game_values.players < 4)
					game_values.players++;
			}
			return false;
		};
		bool enter(){return menu_startgame();};

		int draw(const int x, const int y){
			font[2].draw(x, y, "players");

			int i;
			for(i=0;i<game_values.players;i++)
				spr_player[i][PGFX_STANDING_R].draw(	x + 110 + i*32, y-5);

			for(i=game_values.players; i < 4; i++)
				menu_mario.draw(						x + 110 + i*32, y-5);

			for(i=game_values.cpu; i > 0; i--){
				menu_cpu.draw(							x + 110 + (4*32 - i*32), y-5);
			}

			

			return y + font[2].getHeight();
		};

};

class MI_Map : public MenuItem{
	public:
		bool left(){
			maplist.prev();
			map.loadMap(maplist.current_name());
			map.predraw(spr_background, spr_map);
			return false;
		};
		bool right(){
			maplist.next();
			map.loadMap(maplist.current_name());
			map.predraw(spr_background, spr_map);
			return false;
		};
		bool enter(){return menu_startgame();};

		int draw(const int x, const int y){
			font[2].drawf(x, y, "map %s", maplist.current_name()+4);
			return y + font[2].getHeight();
		};
};

class MI_Mode : public MenuItem{
	public:
		bool left(){
			currentgamemode--;
			if(currentgamemode < 0)
				currentgamemode = GAMEMODE_LAST-1;
			game_values.gamemode = gamemodes[currentgamemode];
			return false;
		};
		bool right(){
			currentgamemode++;
			if(currentgamemode >= GAMEMODE_LAST)
				currentgamemode = 0;
			game_values.gamemode = gamemodes[currentgamemode];
			return false;
		};
		bool enter(){game_values.gamemode->toggleoptions();return false;};

		int draw(const int x, const int y){
			char buffer64[64];

			font[2].drawf(x, y, "mode: %s", game_values.gamemode->getMenuString(buffer64));
			return y + font[2].getHeight();
		};
};
class MI_Options : public MenuItem{
	public:
		bool enter();		//implementation below the deklaration of currentmenuhead
		int draw(const int x, const int y){
			font[2].draw(x, y, "options");
			return y + font[2].getHeight()*2;
		};
};

class MI_Exit : public MenuItem{
	public:
		bool enter(){	
			return true;
		};
		int draw(const int x, const int y){
			font[2].draw(x, y, "exit");
			return y;
		};
};


//---------------------------------------------------------------
// OPTIONS MENU
//---------------------------------------------------------------

class MI_ToggleBool : public MenuItem{
	public:
		MI_ToggleBool(char *nname, bool *nb){
			name = nname;
			b = nb;
		};

		bool left(){	*b = !(*b);		return false;	};
		bool right(){	*b = !(*b);		return false;	};
		bool enter(){	*b = !(*b);		return false;	};

		int draw(const int x, const int y){
			font[2].drawf(x, y, "%s %s", name, BOOL2ONOFF(*b));
			return y + font[2].getHeight();
		};
	
	private:
		char *name;
		bool *b;
};

class MI_Fullscreen : public MenuItem{
	public:
		bool enter(){
#ifndef _EE		// we don't change this value on the PS2
			game_values.fullscreen = !game_values.fullscreen;
			gfx_setresolution(640, 480, game_values.fullscreen);
#endif
			return false;
		};

		int draw(const int x, const int y){
			font[2].drawf(x, y, "fullscreen %s", BOOL2ONOFF(game_values.fullscreen));
			return y + font[2].getHeight();
		};
};

class MI_ScreenResize : public MenuItem{
public:
	bool enter()
	{
		game_values.screenResize += 10;
		if(game_values.screenResize == 100)
			game_values.screenResize = 0;

#ifndef _EE
		FILE *fp;

		fp = fopen("options.bin", "wb");

		if(fp!=NULL) {
			fwrite(&game_values.screenResize, sizeof(int), 1, fp);
			fclose(fp);
		}
		return false;
#else
		return false;
#endif
	};

	int draw(const int x, const int y){
		font[2].drawf(x, y, "screenresize %d", game_values.screenResize);
		return y + font[2].getHeight();
	};

};

class MI_Back : public MenuItem{
	public:
		bool enter();		//implementation below the deklaration of currentmenuhead
		int draw(const int x, const int y){
			font[2].draw(x, y, "back");
			return y + font[2].getHeight()*2;
		};
};



//---------------------------------------------------------------
// SUPER CRAZY OPTIONS/MAIN MENU SYSTEM
//---------------------------------------------------------------

MenuItem	*currentmenuhead;
MenuItem	*currentitem;


MenuItem	*head_optionsmenu;
MenuItem	*head_mainmenu;
void menu_setmenu(MenuItem *newhead){
	currentmenuhead = newhead;
	currentitem = newhead;
}


bool MI_Options	::enter(){	menu_setmenu(head_optionsmenu);		return false;}
bool MI_Back	::enter(){	menu_setmenu(head_mainmenu);		return false;}




//---------------------------------------------------------------
// RUN THE MENU
//---------------------------------------------------------------

void runmenu(){
	unsigned int	framestart;
	int delay;
	SDL_Event	event;
	

	//for drawing the menu
	int		menuy;
	const int	menux = 230;
	MenuItem	*temp;

	//main menu
	MI_Start	m_start;
	MI_Players	m_players;
	MI_Map		m_map;
	MI_Mode		m_mode;
	MI_Options	m_options;
	MI_Exit		m_exit;

	m_start		.setnext(	&m_players);
	m_players	.setnext(	&m_map);
	m_map		.setnext(	&m_mode);
	m_mode		.setnext(	&m_options);
	m_options	.setnext(	&m_exit);
	m_exit		.setnext(	&m_start);


	//options menu
	MI_Back		m_back;
	MI_Fullscreen	m_fullscreen;
	MI_ToggleBool	m_sound	("sound",	&(game_values.sound));
	MI_ToggleBool	m_fps	("show fps",	&(game_values.showfps));
	MI_ScreenResize m_resize;
	

	m_back		.setnext( &m_fullscreen);
	m_fullscreen	.setnext( &m_sound);
	m_sound		.setnext( &m_fps);
	m_fps		.setnext( &m_resize);
	m_resize	.setnext( &m_back);
	
	
	//menu heads
	head_mainmenu		= &m_start;
	head_optionsmenu	= &m_back;
	
	menu_setmenu(head_mainmenu);
	
	// playing the menu music
	if(game_values.sound)
	  musicPlayer.play((char *)"music/menu.mp2");
	
	while (true){
#ifdef _EE		
		// fix some weird RPC issue
		printf("");
#endif
		framestart = SDL_GetTicks();
		
		//printf ("waiting input....\n");
		
		//handle messages
		while(SDL_PollEvent(&event))
		{
			bool upPressed 		= false;
			bool downPressed 	= false;
			bool rightPressed 	= false;
			bool leftPressed 	= false;
			bool cancelPressed 	= false;
			bool okPressed 		= false;
			
			static bool oldLeft 	= false;
			static bool oldRight 	= false;
			static bool oldUp 	= false;
			static bool oldDown 	= false;
			
			switch(event.type){

				case SDL_QUIT:
					// stop current music
					musicPlayer.stop();

					menu_exit();
					return;
					break;

				case SDL_KEYDOWN:
					switch(event.key.keysym.sym){
						case SDLK_ESCAPE:
							cancelPressed = true;
							break;

						case SDLK_DOWN:
							downPressed = true;
							break;

						case SDLK_UP:
							upPressed = true;
							break;

						case SDLK_RETURN:
							okPressed = true;
							break;

						case SDLK_LEFT:	
							leftPressed = true;
							break;

						case SDLK_RIGHT:
							rightPressed = true;
							break;

						default:
							break;
					}
					break;
				case SDL_JOYHATMOTION:
					{
						
						if (event.jhat.value & SDL_HAT_LEFT) {
							if(oldLeft == false)
								leftPressed = true;
							oldLeft = true;
						} else
							oldLeft = false;
						if (event.jhat.value & SDL_HAT_RIGHT) {
							if(oldRight == false)
								rightPressed = true;
							oldRight = true;
						} else
							oldRight = false;
						if (event.jhat.value & SDL_HAT_UP) {
							if(oldUp == false)
								upPressed = true;
							oldUp = true;
						} else
							oldUp = false;
						if (event.jhat.value & SDL_HAT_DOWN) {
							if(oldDown == false)
								downPressed = true;
							oldDown = true;
						} else
							oldDown = false;
					}
					break;
				 
				case SDL_JOYBUTTONDOWN:
					//printf("joybutton: %d\n",event.jbutton.button);
					if(event.jbutton.button == 0)
						cancelPressed = true;
					if(event.jbutton.button == 1)
						okPressed = true;
					break;
			default:
				break;
			}
			if(rightPressed) {
				rightPressed = false;
				if( currentitem->right() )
					return;
			}
			if(leftPressed) {
				leftPressed = false;
				if( currentitem->left() )
					return;
			}
			if(downPressed) {
				downPressed = false;
				currentitem = currentitem->next();
			}
			if(upPressed) {
				upPressed = false;
				currentitem = currentitem->prev();
			}
			if(cancelPressed) {
				cancelPressed = false;
				if(currentmenuhead == head_optionsmenu){	//if we are in the options menu go back
					menu_setmenu(head_mainmenu);
				} else {	//quit
//>					menu_exit();
//>					return;
				}
			}
			if(okPressed) {
				okPressed = false;
				if( currentitem->enter() )
					return;
			}
		}

		

		//--------------- draw everything ----------------------
		spr_map.draw(0,0);
		
		menu_smw.draw(320- ((menu_smw.getWidth())>>1), 30);	//smw logo
		
		temp = currentmenuhead;
		menuy = 200;
		do{
			if(currentitem == temp){	//selected item
				font[2].draw(menux, menuy, ">");		//draw cursor here
				menuy = temp->draw(menux+25, menuy);	//another offset
			}
			else{
				menuy = temp->draw(menux, menuy);		//not selected
			}
			temp = temp->next();
		}while(temp != currentmenuhead);
		
		SDL_Flip(screen);

		//we don't need much accuracy here, so we can stick to SDL_Delay
		delay = WAITTIME - (SDL_GetTicks()-framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}
	
	// stop current music
	musicPlayer.stop();

	//we won't ever get here
	return;
}



MapList::MapList(){
	bool error = true;

#ifdef _WIN32
	WIN32_FIND_DATA	finddata;
	HANDLE			findhandle;


	if((findhandle = FindFirstFile("maps/*.map", &finddata)) == INVALID_HANDLE_VALUE)
		goto maplisterror;	//bad, i know


	head = new mlentry(finddata.cFileName);
	head->next = head;
	head->prev = head;
	current = head;
	error = false;

	while(FindNextFile(findhandle, &finddata)){
		current->next = new mlentry(finddata.cFileName);
		current->next->prev = current;
		current = current->next;
		current->next = head;
		head->prev = current;		
	}

	FindClose(findhandle);

#elif _EE
	int item;
	head = new mlentry(map_list[0]);
	head->next = head;
	head->prev = head;
	current = head;
	error = false;
	
	for (item = 1; item < MAP_LIST_ENTRIES ; item++)
	{
		current->next = new mlentry(map_list[item]);
		current->next->prev = current;
		current = current->next;
		current->next = head;
		head->prev = current;
	}
#else
	head = new mlentry("0smw.map");
	head->next = new mlentry("todo: port dir browser");
	head->prev = head->next;
	head->next->next = head;
	head->next->prev = head;
	error = false;

#endif 

#ifdef _WIN32
maplisterror:
#endif
	if(error){
		head = new mlentry("directory is empty");
		head->next = head;
		head->prev = head;
	}
	current = head;

}


MapList::~MapList(){
	mlentry *temp, *del;

	temp = head;
	while(temp->next != head){
		del = temp;
	temp = temp->next;
		delete del;
	}
	delete head;
}


