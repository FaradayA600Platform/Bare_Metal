/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

//#include <virgil.h>
#include <types.h>
//#include <ff.h>
#include <stdio.h>

size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream)
{
#ifdef CONFIG_VFS_FATFS_READONLY
	errno = EINVAL;
	return 0;
#else
	size_t wr;
	FRESULT rs;

	rs = f_write(stream, ptr, size * nitems, (UINT *)&wr);

	ffeconv(rs);

	return wr;
#endif
}
