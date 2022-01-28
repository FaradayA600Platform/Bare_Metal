/*
 * Copyright (C) 2004-2006 Manuel Novoa III <mjn3@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "../include/ctype.h"

#ifndef ERANGE
#define ERANGE	34	/* Math result not representable.  */
#endif

#define SET_ERRNO(X)            ((void)(X))	/* keep side effects */

unsigned long __strtoul(const char *str, char **endptr, int base, int sflag)
{
	unsigned long number, cutoff;
#if _STRTO_ENDPTR
	const char *fail_char;
#define SET_FAIL(X) fail_char = (X)
#else
#define SET_FAIL(X) ((void)(X)) /* Keep side effects. */
#endif
	unsigned char negative, digit, cutoff_digit;

//	assert(((unsigned int)sflag) <= 1);

	SET_FAIL(str);

	while (isspace(*str)) { /* Skip leading whitespace. */
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
		cutoff_digit = ULONG_MAX % base;
		cutoff = ULONG_MAX / base;
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

			if ((number > cutoff)
				|| ((number == cutoff) && (digit > cutoff_digit))) {
				number = ULONG_MAX;
				negative &= sflag;
				SET_ERRNO(ERANGE);
			} else {
				number = number * base + digit;
			}
		} while (1);
	}

#if _STRTO_ENDPTR
	if (endptr) {
		*endptr = (char *) fail_char;
	}
#endif

	{
		unsigned long tmp = (negative
							 ? ((unsigned long)(-(1 + LONG_MIN))) + 1
							 : LONG_MAX);
		if (sflag && (number > tmp)) {
			number = tmp;
			SET_ERRNO(ERANGE);
		}
	}

	return negative ? (unsigned long)(-((long)number)) : number;
}

long strtol(const char *__nptr, char **__endptr, int __base)
{
	return (long)__strtoul(__nptr, __endptr, __base, 1);
}

unsigned long strtoul(const char *__nptr, char **__endptr, int __base)
{
	return __strtoul(__nptr, __endptr, __base, 0);
}
