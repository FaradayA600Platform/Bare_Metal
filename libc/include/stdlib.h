/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <malloc.h>

/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

/* Abort execution and generate a core-dump.  */
#define abort()			while(1)

/* Return the absolute value of X.  */
#define abs(x)			((x) > 0 ? (x) : -(x))

/* converts a string to a floating-point value */
double atof(const char *__nptr);

/* converts a string to an integer */
int atoi(const char *__nptr);
long atol(const char *__nptr);
long long atoll(const char *__nptr);

/* converts a string to a floating-point value */
float strtof(const char *__nptr, char **__endptr);
double strtod(const char *__nptr, char **__endptr);
long double strtold(const char *__nptr, char **__endptr);

/* converts a string to a signed integer */
long strtol(const char *__nptr, char **__endptr, int __base);
long long strtoll(const char *__nptr, char **__endptr, int __base);

/* converts a string to an unsigned integer */
unsigned long strtoul(const char *__nptr, char **__endptr, int __base);
unsigned long long strtoull(const char *__nptr, char **__endptr, int __base);

/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647

/* Return a random integer between 0 and RAND_MAX inclusive.  */
int rand(void);

/* Reentrant interface according to POSIX.1.  */
int rand_r(unsigned int *seed);

/* Seed the random number generator with the given number.  */
void srand(unsigned int seed);

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void qsort_r(void *base, size_t nmemb, size_t size,
		   int (*compar)(const void *, const void *, void *), void *arg);

/* Returned by `div'.  */
typedef struct {
    int quot;			/* Quotient.  */
    int rem;			/* Remainder.  */
} div_t;

/* Returned by `ldiv'.  */
#ifndef __ldiv_t_defined
typedef struct {
    long int quot;		/* Quotient.  */
    long int rem;		/* Remainder.  */
} ldiv_t;
# define __ldiv_t_defined	1
#endif

/* Return the `div_t', `ldiv_t' or `lldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
div_t div(int __numer, int __denom);
ldiv_t ldiv(long int __numer, long int __denom);

/* Non-standard (i.e. non-ISO C) functions. */

/* converts an integer to a string. */
char *itoa(int __value, char *__nptr, int __base);
char *ltoa(long __value, char *__nptr, int __base);

/* converts a floating-point to a string. */
char *ftoa(double __value, char *__nptr, int __prec);

#endif	/* #ifndef _STDLIB_H */
