/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <stdlib.h>
#include <math.h>

char *ftoa(double __value, char *__nptr, int __prec)
{
	__dtostr(__value, __nptr, 0xff, 12, __prec, 0);

	return __nptr;
}
