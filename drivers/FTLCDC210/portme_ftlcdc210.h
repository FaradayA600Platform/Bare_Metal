#ifndef PORTME_FTLCDC210_H
#define PORTME_FTLCDC210_H

#define CONFIG_BSP_AARCH64 1
#define printf fLib_printf
//#define getchar fLib_getch

/************************NonOS Lite************************/
#if defined(CONFIG_BSP_LITE)
#include <common.h>
#include <platform.h>


#ifdef CONFIG_PLATFORM_A320
#define OLD_VERSION
#endif

/* Platform dependant settings */
#define DMA020_BASE     0x90400000
#define IRQ_DMA020      21

/* Max AHB Master number can be selected */
#define AHB_MASTER_NUM  1

#define printf    prints
#define clock     get_ticks
#define CLOCKS_PER_SEC  TIMER_CLOCK

#define SRC_BUF         0x1000000
#define DST_BUF         0x2000000

#define ftdmac020_enable_interrupt  irq_set_enable
#define ftdmac020_disable_interrupt irq_set_disable

/************************NonOS A380************************/
#elif defined(CONFIG_BSP_A380)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "SoFlexible.h"



/************************NonOS AARCH64************************/
#elif defined(CONFIG_BSP_AARCH64)


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <math.h>

#define TRUE    1
#define FALSE   0

char ftlcdc210_get_char(void);
extern int fLib_ConnectInt(unsigned int IntNum,void* Handler);
extern int fLib_CloseInt(unsigned int IntNum);
extern void UnmaskIRQ(unsigned int IRQ);
extern void SetIRQmode(unsigned int IRQ,unsigned int edge);
extern void EnableIRQ(void);
extern void DisableIRQ(void);



/************************Without BSP************************/
#else


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#define TRUE    1
#define FALSE   0

#endif

#endif
