/* ROMFS
 * Copyright (C) 2005 Gawd
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

#include <stdio.h>
#include <string.h>
#include "romfs_io.h"

// handle romdisk 
#include "romfs.h"




int sz;                  //size      of  file from romfs
int offset;              //offset  ... 
unsigned char *dataptr;  //datptr ... 

void rfs_init()
{
	sz = 0;
	offset = 0;
	dataptr = 0;
}

int rfs_open(char* path)
{
	rfs_init();
	
        if (romdisk_find(path, (void **)&dataptr, &sz) == 0) {
		return 0;
	}
	else
	{
		printf("failed to open %s\n", path);
		return 1;
	}
}

void rfs_read(void *ptr, unsigned int  len) {
	memcpy(ptr, dataptr + offset, len);
	offset += len;
}

void rfs_close() {	
        rfs_init();
}

int rfs_size() {
	return sz;
}

void rfs_seek(int off) {
	offset = off;
}
 
