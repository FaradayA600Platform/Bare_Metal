#ifndef _TIME_H
#define _TIME_H

/* Get CONFIG_SYS_HZ */
#include <config.h>
/* Get size_t, clock_t, time_t */
#include <sys/types.h>

/* This defines CLOCKS_PER_SEC, which is the number of processor clock
   ticks per second.  */
#define CLOCKS_PER_SEC	CONFIG_SYS_HZ
#define CLK_TCK			CLOCKS_PER_SEC

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval {
	time_t tv_sec;		/* Seconds.  */
	suseconds_t tv_usec;	/* Microseconds.  */
};

/* Used by other time functions.  */
struct tm {
	int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
	int tm_min;			/* Minutes.	[0-59] */
	int tm_hour;		/* Hours.	[0-23] */
	int tm_mday;		/* Day.		[1-31] */
	int tm_mon;			/* Month.	[0-11] */
	int tm_year;		/* Year	- 1900.  */
	int tm_wday;		/* Day of week.	[0-6] */
	int tm_yday;		/* Days in year.[0-365]	*/
	int tm_isdst;		/* DST.		[-1/0/1]*/
};

/* Time used by the program so far (user time + system time).
   The result / CLOCKS_PER_SECOND is program time in seconds.  */
clock_t clock(void);

/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
time_t time(time_t *__timer);

#if 0

/* Return the `time_t' representation of TP and normalize TP.  */
extern time_t mktime (struct tm *__tp);

/* Set the system time to *WHEN.
   This call is restricted to the superuser.  */
extern int stime (const time_t *__when);

#endif	/* #if 0 */

#endif
