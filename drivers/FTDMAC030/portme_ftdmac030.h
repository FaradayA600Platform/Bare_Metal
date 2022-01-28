#ifndef PORTME_FTDMAC030_H
#define PORTME_FTDMAC030_H

#include <SoFlexible.h>

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
#include "../FTSPI020/DrvSPI020.h"

/************************NonOS A600************************/
#elif defined(CONFIG_PLATFORM_A600)

#include "SoFlexible.h"

#define printf fLib_printf

#define FLASH_NORMAL          0x00
#define FLASH_DMA_READ        0x04
#define SPI020REG_DATAPORT    SPI_FTSPI020_0_PA_BASE+0x100

/************************Without BSP************************/
#else

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#define TRUE    1
#define FALSE   0

const unsigned DMAC_FTDMAC030_pa_base[2] = {
     0x92D00000,
     0x92E00000
};

#define FREE_MEM_BASE 0x40600000

#define CONFIG_FTDMAC030_CHANNEL_COUNT 7

#define FLASH_NORMAL          0x00
#define FLASH_DMA_READ        0x04
#define SPI020REG_DATAPORT    (0xC0A00100)

const unsigned char DMAC_FTDMAC030_irq[2][3] = {
    { 51, 52, 53 },
    { 0, 0, 0 }
};

#endif

#endif
