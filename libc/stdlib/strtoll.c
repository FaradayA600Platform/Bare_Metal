/*
 * Copyright (C) 2004-2006 Manuel Novoa III <mjn3@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif

#include <stdlib.h>
#include <assert.h>
#include "../include/limits.h"
#include <ctype.h>

#ifndef ERANGE
#define ERANGE	34	/* Math result not representable.  */
#endif

#define SET_ERRNO(X)            ((void)(X))	/* keep side effects */

static unsigned long long __strtoull(const char *str, char **endptr, int base, int sflag)
{
	unsigned long long number;
#if _STRTO_ENDPTR
	const char *fail_char;
#define SET_FAIL(X) fail_char = (X)
#else
#define SET_FAIL(X) ((void)(X)) /* Keep side effects. */
#endif
	unsigned int n1;
	unsigned char negative, digit;

	assert(((unsigned int)sflag) <= 1);

	SET_FAIL(str);

	while (isspace(*str)) {		/* Skip leading whitespace. */
		++str;
	}

	/* Handle optional sign. */
	negative = 0;
	switch (*str) {
	case '-':
		negative = 1;	/* Fall through to increment str. */
	case '+':
		++str;
	}

	if (!(base & ~0x10)) {		/* Either dynamic (base = 0) or base 16. */
		base += 10;				/* Default is 10 (26). */
		if (*str == '0') {
			SET_FAIL(++str);
			base -= 2;			/* Now base is 8 or 16 (24). */
			if ((0x20 | (*str)) == 'x') { /* WARNING: assumes ascii. */
				++str;
				base += base;	/* Base is 16 (16 or 48). */
			}
		}

		if (base > 16) {		/* Adjust in case base wasn't dynamic. */
			base = 16;
		}
	}

	number = 0;

	if (((unsigned)(base - 2)) < 35) { /* Legal base. */
		do {
			digit = ((unsigned char)(*str - '0') <= 9)
					? /* 0..9 */ (*str - '0')
					: /* else */ (((unsigned char)(0x20 | *str) >= 'a') /* WARNING: assumes ascii. */
								  ? /* >= A/a */ ((unsigned char)(0x20 | *str) - ('a' - 10))
								  : /* else   */ 40 /* bad value */);

			if (digit >= base) {
				break;
			}

			SET_FAIL(++str);

#if 1
			/* Optional, but speeds things up in the usual case. */
			if (number <= (ULLONG_MAX >> 6)) {
				number = number * base + digit;
			} else
#endif
			{
				n1 = ((unsigned char) number) * base + digit;
				number = (number >> CHAR_BIT) * base;

				if (number + (n1 >> CHAR_BIT) <= (ULLONG_MAX >> CHAR_BIT)) {
					number = (number << CHAR_BIT) + n1;
				} else {		/* Overflow. */
					number = ULLONG_MAX;
					negative &= sflag;
					SET_ERRNO(ERANGE);
				}
			}

		} while (1);
	}

#if _STRTO_ENDPTR
	if (endptr) {
		*endptr = (char *) fail_char;
	}
#endif

	{
		unsigned long long tmp = ((negative)
								  ? ((unsigned long long)(-(1 + LLONG_MIN))) + 1
								  : LLONG_MAX);
		if (sflag && (number > tmp)) {
			number = tmp;
			SET_ERRNO(ERANGE);
		}
	}

	return negative ? (unsigned long long)(-((long long)number)) : number;
}

long long strtoll(const char *__nptr, char **__endptr, int __base)
{
	return (long long)__strtoull(__nptr, __endptr, __base, 1);
}

unsigned long long strtoull(const char *__nptr, char **__endptr, int __base)
{
	return __strtoull(__nptr, __endptr, __base, 0);
}
