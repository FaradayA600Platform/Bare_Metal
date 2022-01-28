#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(CONFIG_PLATFORM_A369)
	#include "a369/platform.h"
	#include "a369/spec.h"
#elif defined(CONFIG_PLATFORM_A380)
	#include "a380/platform.h"
	#include "a380/spec.h"
#elif defined(CONFIG_PLATFORM_A380_ZYNQ)
	#include "a380_zynq/platform.h"
	#include "a380_zynq/spec.h"
#elif defined(CONFIG_PLATFORM_A360)
	#include "a360/platform.h"
	#include "a360/spec.h"
#elif defined(CONFIG_PLATFORM_FH36)
	#include "fh36/platform.h"
	#include "fh36/spec.h"
#elif defined(CONFIG_PLATFORM_LTE)
	#include "lte/platform.h"
	#include "lte/spec.h"
#elif defined(CONFIG_PLATFORM_GM8220)
	#include "gm8220/platform.h"
	#include "gm8220/spec.h"
#elif defined(CONFIG_PLATFORM_A500)
	#include "a500/platform.h"
	#include "a500/spec.h"
#elif defined(CONFIG_PLATFORM_A600)
	#include "a600/platform.h"
	#include "a600/spec.h"	
#else
	#error "please assign plaform"
#endif

#endif
