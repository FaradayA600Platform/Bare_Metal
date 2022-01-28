/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <stdlib.h>

static char buf[12] = { 0 };
static const char *hex = "0123456789ABCDEF";

char *ltoa(long value, char *nptr, int base)
{
	unsigned int ret = 0;
	int i, len = 0, off = 0;
	char tmp[12];

	if (nptr == NULL)
		nptr = buf;

	if (value == 0) {
		tmp[len++] = '0';
	} else {
		if (value < 0) {
			off = 1;
			nptr[0] = '-';
			value = -1 * value;
		}
		for (; value > 0; value /= base) {
			ret = value % base;
			tmp[len++] = hex[ret];
		}
	}
	tmp[len] = 0;

	for (i = 0; i < len; ++i)
		nptr[i + off] = tmp[len - 1 - i];
	nptr[len + off] = 0;

	return nptr;
}
