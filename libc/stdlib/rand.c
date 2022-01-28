/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <stdlib.h>

#ifdef CONFIG_LIB_FREERTOS

/* POSIX.1c requires that there is mutual exclusion for the `rand' and
   `srand' functions to prevent concurrent calls from modifying common
   data.  */
#include <FreeRTOS.h>
#include <task.h>

#define __lock()		vTaskSuspendAll()
#define __unlock()		xTaskResumeAll()

#else	/* CONFIG_LIB_FREERTOS */

#define __lock()		do { } while(0)
#define __unlock()		do { } while(0)

#endif	/* CONFIG_LIB_FREERTOS */

static unsigned int _seed = 1;

/* Seed the random number generator with the given number.  */
void srand(unsigned int seed)
{
	__lock();
	_seed = seed;
	__unlock();
}

/* Return a random integer between 0 and RAND_MAX inclusive.  */
int rand(void)
{
	int n;
	__lock();
	n = rand_r(&_seed);
	__unlock();
	return n;
}
