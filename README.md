# Super Mario War / PS2 
a Super Mario Death match game for the PlayStation 2™

![smw_1 10b2_ingame](https://github.com/obiot/SMW-PS2/assets/4033090/dee9334e-b595-43d9-8c8d-157f9b45f503)

Last update: February 23, 2006

# Copyright

Original Game by Florian Hufsky and Michael Schaffer (see [here](/src/README.txt)).

Playstation 2 version by Olivier "evilo" Biot and The Froggies.

#History
-------

- February 23th, 2006 - Super Mario War v1.10r6
As promised, a little update to add a great feature : Multitap 
support thanks to the work of Drakonite on the SDL library
(as well as others little fixes to the driver, but that should 
be however not visible).
- February 15th, 2006 - Super Mario War v1.10b5
It's still a wip version, and still need some rework, but since 
it's already playable and fun, I decided to release a first version.
The PS2 version is a based on an older version of smw (1.10), but 
should be updated in the future to be inline with the official version.


# Compiling:
----------

To get your sources compiled, checkout the latest PS2SDK source tree. 
You will need the last SDL and SDL_mixer version from the repository.
(All of these are available at www.ps2dev.org)
Check also ps2_lib_patch.txt for specific details/issues.

just type "make all" and you are done !


# Credits:
--------
Florian Hufsky and Michael Schaffer for their great game !
Gawd for the SDL library port on the ps2.
Drakonite for adding mtap support in the SDL driver.
To all the froggies, and even Shazz :)