/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <stdlib.h>

long atol(const char *nptr)
{
	return (long)strtol(nptr, NULL, 10);
}