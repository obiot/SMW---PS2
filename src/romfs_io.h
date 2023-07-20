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
#ifndef _ROMFS_IO_
#define _ROMFS_IO_

#ifdef __cplusplus
extern "C" {
#endif

extern int sz;                  //size      of  file from romfs
extern int offset;              //offset  ... 
extern unsigned char *dataptr;  //datptr ... 


extern unsigned char rd_start[];

void rfs_init();

int rfs_open(char* path);

void rfs_read(void *ptr, unsigned int len);

void rfs_close();

int rfs_size();

void rfs_seek(int off);

#ifdef __cplusplus
}
#endif


#endif

