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
#include "sys.h"

// make the C++ compile happy
int *__errno() { return &errno; }

typedef struct 
{
	char	magic[8];		/* Should be "-rom1fs-" */
	uint32	full_size;		/* Full size of the file system */
	uint32	checksum;		/* Checksum */
	char	volume_name[16];	/* Volume name (zero-terminated) */
} romdisk_hdr_t;

typedef struct {
	uint32	next_header;		/* Offset of next header */
	uint32	spec_info;		/* Spec info */
	uint32	size;			/* Data size */
	uint32	checksum;		/* File checksum */
	char	filename[16];		/* File name (zero-terminated) */
} romdisk_file_t;

static uint8 const *mounted_img = 0;

int romdisk_mount(const void *img)
{
	const romdisk_hdr_t *hdr;

	/* make sure romfs is sane */
	hdr = (romdisk_hdr_t *)img;
	if (strncmp(hdr->magic, "-rom1fs-", 8) != 0) {
		return -1;
	}

	mounted_img = (const uint8 *)img;
	return 0;
}

int romdisk_umount()
{
	mounted_img = 0;
	return 0;
}

int romdisk_find(const char *path, void **ptr, int *size)
{
	const romdisk_hdr_t *hdr;
	const romdisk_file_t *fhdr;
	const char *subdir;
	int path_len;
	int type;
	uint32 offset;

	if (mounted_img == NULL || ptr == NULL) {
		return -1;
	}

	hdr = (romdisk_hdr_t *)mounted_img;

	/* look for the directory containing the file */
	offset = sizeof(romdisk_hdr_t) + (strlen(hdr->volume_name) & ~0x0f);
	//printf("starting at offset 0x%x\n", offset);

	if (path[0] == '/')
	{
		/* skip leading '/' */
		path++;
	}

	subdir = strchr(path, '/');
	while (subdir != 0)
	{
		path_len = (subdir - path);
		//printf("looking for directory '%s' len=%d\n", path, path_len);

		while (offset != 0)
		{
			fhdr = (const romdisk_file_t *)(mounted_img + offset);
			type = READ_BE_UINT32(&fhdr->next_header) & 0x0f;
			
			//printf("comparing against %s\n", fhdr->filename);
			if (strncasecmp(fhdr->filename, path, path_len) == 0)
			{
				/* found our directory */
				offset = READ_BE_UINT32(&fhdr->spec_info);
				break;
			}

			offset = READ_BE_UINT32(&fhdr->next_header) & ~0x0f;
		}

		if (offset == 0)
		{
			/* directory not found */
			printf("directory not found\n");
			return 2;
		}

		//printf("directory found at offset 0x%x\n", offset);

		path = subdir + 1;
		subdir = strchr(path, '/'); 
	}

	/* now look for the file */
	if (strlen(path) == 0)
	{
		printf("empty file name\n");
		return 2;
	}

	while (offset != 0)
	{
		fhdr = (const romdisk_file_t *)(mounted_img + offset);
		type = READ_BE_UINT32(&fhdr->next_header) & 0x0f;
			
		//printf("looking for file %s at offset 0x%x (%s)\n", path, offset, fhdr->filename);
		if (strcasecmp(fhdr->filename, path) == 0)
		{
			/* found our file */
			*ptr = (void *)(mounted_img + offset + sizeof(romdisk_file_t) + (strlen(fhdr->filename) & ~0x0f));
			if (size != 0)
			{
				*size = READ_BE_UINT32(&fhdr->size);
			}

			break;
		}

		offset = READ_BE_UINT32(&fhdr->next_header) & ~0x0f;
	}

	if (offset == 0)
	{
		printf("file not found\n");
		return 2;
	}

	return 0;
}
