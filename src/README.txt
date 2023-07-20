
------------------------------------------------------------------------------
 Super Mario War 1.01
 --------------------------------------
								2004 © Florian Hufsky
								http://jnrdev.weed-crew.net

								code: Florian Hufsky
								idea: Samuele Poletti
								gfx:  Nintendo
								sfx:  Epic, ID Software, F.H.

								the code is released under the GPL.

								last changes: 03.5.2004

								have a lot of fun :)
------------------------------------------------------------------------------



------------------------------------------------------------------------------
 !!!! IMPORTANT !!!!
------------------------------------------------------------------------------

ALL GRAPHICS IN THIS GAME ARE © NINTENDO
MOST OF THE SOUNDS ARE © EPIC

------------------------------------------------------------------------------

Super Mario War Homepage:

  http://jnrdev.weed-crew.net/smw


jnrdev - the 2d jump'n'run tutorial series:

  http://jnrdev.weed-crew.net

------------------------------------------------------------------------------


------------------------------------------------------------------------------
 general:
------------------------------------------------------------------------------

the goal of the game is to stomp the other marios as often as possible.

controls:
	          left  right  jump
	player 1: cursor keys
	player 2: a     d      w
	player 3: h     k      u
	player 4: lmb   rmb    lmb+rmb / NUM1 NUM5 NUM 3 (if mousecontrol is off)

	menu: cursor keys + enter
		mode selection: press enter to increase the gamemode value
		player selection: press cursor_right more than 4 times to switch on computer marios ("CPU")
		
	


a map editor is also included, but all maps are saved to ZZleveleditor.map.
if you want to keep a map you'll have to rename it.

leveleditor controls:
	space: switch edit / tile mode
	ESC:   save and quit
	
	edit mode:
		mouse left: place tile
		mouse right: erase tile
	tile mode:
		mouse left: choose tile
		mouse right: change tile type (red box... solid, blue box... solid on top, nothing: non solid)


i hope you enjoy the game!

------------------------------------------------------------------------------



------------------------------------------------------------------------------
 notes:
------------------------------------------------------------------------------

i know that some things in the code are very ugly and might contain some bugs,
but i want to get the game done and continue with jnrdev.

when writing the jnrdev tutorials dealing with those parts of the code i'll
revisit it. i hope to have better solutions until then.
(those parts are mainly player-player collision detection and some of the code
for appearing on both sides of the screen (moving over the edge of the screen)
).


known bugs:


top tile row is bad
 -> i know - don't place tiles there, maybe i'll fix it somewhen

collision detection is not 100% accurate when standing
 -> don't worry, this is supposed to be like that.
	the player sprite is smaller then the collision detection area,
	so it just looks wrong.


you can get the whole source code at http://jnrdev.weed-crew.net/smw
take a look at how the thing works.txt and dev-diary.txt for further
source related informations.

for linux related info take a look at linux.txt

------------------------------------------------------------------------------



------------------------------------------------------------------------------
 THANKS TO:
------------------------------------------------------------------------------

first of all i want to thank everyone who gave me feedback - this game won't
ever got so far without you. it really keeps you motivated when you know that
there are some people who like your game and look forward to the next version.
THANK YOU!

other thanks in no particular order:
no_shorty           for the first three maps
Samuele Poletti     for the original mario war
redfalcon,
no_human,
dschingis,
matsche,
aeroflare,
no_shorty           for tips and finding bugs
Tymoe               for playing smw and submitting some screenshots
Nintendo            for not sueing me after one week and creating
                    some of the coolest games ever
David Olofson       for the very interesting conversations and tips
scoti               for the map scot
affeOHNEwaffe       for the map affe and the bowser skin
mademan             for his skin and map
mario piuk          for the new gamemodes the ki and some bugfixes
yvoo                for following ideas: pushback speed depending on player
                                         speed, stomping+jumpkey->jump higher
kaddar              for the additional sprite tiles and some other tiles

and everyone i forgot

THANKS!

------------------------------------------------------------------------------



2004 © Florian Hufsky
http://jnrdev.weed-crew.net
http://jnrdev.weed-crew.net/smw
