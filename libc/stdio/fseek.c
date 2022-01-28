/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

//#include <virgil.h>
//#include <ff.h>
#include <stdio.h>

int fseek(FILE *stream, long int offset, int whence)
{
#if CONFIG_VFS_FATFS_MINIMIZE >= 3
	return -1;
#else
	FRESULT rs = 0;

	switch (whence) {
	case SEEK_SET:
		rs = f_lseek(stream, offset);
		break;
	case SEEK_CUR:
		rs = f_lseek(stream, f_tell(stream) + offset);
		break;
	case SEEK_END:
		rs = f_lseek(stream, f_size(stream) + offset);
		break;
	}

	ffeconv(rs);

	return errno ? -1 : 0;
#endif
}
