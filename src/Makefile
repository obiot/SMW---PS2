#					     Super Mario War / PS2
#------------------------------------------------------------------
# Copyright 2005 - Olivier "evilo" Biot <evilo13@gmail.com>
# Released under GNU license
# Review SMW README & LICENSE files for further details.
#------------------------------------------------------------------
# GCC 3.2.2 / PS2SDK 1.2 / GSKIT 0.2 / SDL 0.1
#------------------------------------------------------------------
# Compilation Flags
#
#------------------------------------------------------------------

EE_BIN      = smw.elf

EE_LIBS   += -L$(PS2SDK)/ports/lib -lsdlmain -lSDL_image -lsdlmixer -lsdl -ljpg -lmc -ldebug
EE_INCS   += -I../include -I$(PS2SDK)/ports/include -I$(PS2SDK)/ports/include/zlib -I$(PS2SDK)/ports/include/SDL

EE_CXXFLAGS += -mlong32 -DUSE_ROMFS -DSYS_LITTLE_ENDIAN -DSYS_NEED_ALIGNMENT -DUSE_RWOPS

EE_CFLAGS   += -DSYS_LITTLE_ENDIAN -DSYS_NEED_ALIGNMENT 

GENROMFS_PATH = ../tools/genromfs
GENROMFS = $(GENROMFS_PATH)/genromfs
  

EE_OBJS = SFont.o \
	  eyecandy.o \
	  gamemodes.o \
	  gfx.o \
	  main.o \
	  map.o \
	  player.o \
	  music_player.o \
	  romdisk.o romfs.o romfs_io.o 

romdisk.img:
	@echo Creating romdisk..
	@$(GENROMFS) -f $@ -d romdisk

romdisk.o: romdisk.img
	@$(PS2SDK)/bin/bin2o -a 16 -n romdisk.img romdisk.o romdisk
 
  
all:    build_romfs $(EE_BIN) 
	$(EE_STRIP) $(EE_BIN)

build_romfs:
	$(MAKE) -C $(GENROMFS_PATH) all 
clean:
	rm -f -R *.elf $(EE_OBJS)  romdisk.img 
	$(MAKE) -C $(GENROMFS_PATH) clean

run:
	ps2client -h 192.168.0.100 execee host:$(EE_BIN)  
	
reset:
	ps2client -h 192.168.0.100 reset   


include Makefile.pref
include Makefile.eeglobal

