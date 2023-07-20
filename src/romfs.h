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

#ifndef __ROMFS_H__
#define __ROMFS_H__

#ifdef __cplusplus
extern "C" {
#endif

int romdisk_mount(const void *img);
int romdisk_umount();
int romdisk_find(const char *path, void **ptr, int *size);




#ifdef __cplusplus
}
#endif


#endif
