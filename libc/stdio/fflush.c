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

int fflush(FILE *stream)
{
#ifdef CONFIG_VFS_FATFS_READONLY
	errno = EINVAL;
	return EOF;
#else
	FRESULT rs = f_sync(stream);

	ffeconv(rs);

	return errno ? EOF : 0;
#endif
}
