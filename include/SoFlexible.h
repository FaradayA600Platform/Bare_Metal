#ifndef SOFLEXIBLE_H_
#define SOFLEXIBLE_H_

#include "autoconf.h"

#include "types.h"
#include "serial.h"
#include "sizes.h"
#include "board.h"
#include "platform.h"
#include "cpu.h"

#include "interrupt.h"
#include "spec_macro.h"
#include "io.h"
#include "tool/bits.h"
#include "tool/utility.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef CONFIG_JIFFIES 
#include "jiffies.h"
#endif

//#define printf      fLib_printf


#ifdef COMP_GCC
#	define __rt_lib_init(arg...)
#endif

#endif /*SOFLEXIBLE_H_*/
