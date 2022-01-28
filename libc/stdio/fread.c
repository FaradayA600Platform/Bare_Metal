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
#include <types.h>

size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
	size_t rd;
	FRESULT rs;

	rs = f_read(stream, ptr, size * nitems, (UINT *)&rd);

	ffeconv(rs);

	return rd;
}
