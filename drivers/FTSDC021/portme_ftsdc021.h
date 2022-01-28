#ifndef PORTME_FTSDC021_H
#define PORTME_FTSDC021_H

#include "SoFlexible.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/************************NonOS Lite************************/
#if defined(CONFIG_BSP_LITE)
#include <common.h>
#include <platform.h>

/* Platform dependant settings */
#define FTSDC021_FPGA_BASE      0xB0200000
#define FTSDC021_FPGA_IRQ       13

/* Enable FTSDC021_EDMA to use the external DMA
 * and function ftsdc021_Start_DMA must be implemented. */
//#define FTSDC021_EDMA

#define printf	prints
#define CLOCKS_PER_SEC  TIMER_CLOCK

/************************  ZYNQ  ************************/
#elif defined(CONFIG_BSP_ZYNQ)
#include <Xil_io.h>
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xuartps_hw.h"
#include "xtime_l.h"

/* Platform dependant settings */
#define FTSDC021_FPGA_BASE      0x43C00000
#define FTSDC021_FPGA_IRQ       XPAR_FABRIC_IRQ_F2P0_INTR
#define GIC_DEVICE_ID			XPAR_SCUGIC_0_DEVICE_ID

struct cmd {
	char *name;
	char *usage;
	int32_t(*func) (int32_t argc, char *const argv[]);
};
typedef struct cmd cmd_t;
#define CMDLEN          50

#undef printf
#define printf xil_printf

#undef getchar
#define getchar inbyte

char kbhit(void);

#define CLOCKS_PER_SEC  COUNTS_PER_SECOND
#define CLOCKS_PER_MSEC  (CLOCKS_PER_SEC / 1000)

/************************  A600  ************************/
#elif defined(CONFIG_PLATFORM_A600)

#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* Platform dependant settings */
#define FTSDC021_FPGA_BASE      SDC_FTSDC021_0_PA_BASE
#define FTSDC021_FPGA_IRQ       SDC_FTSDC021_0_IRQ

struct cmd {
	char *name;
	char *usage;
	int32_t(*func) (int32_t argc, char *const argv[]);
};
typedef struct cmd cmd_t;
#define CMDLEN                  50

#define printf                  fLib_printf

#undef getchar
#define getchar                 fLib_getchar

typedef void (intr_handler_t)(void *);
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t intr_handler);

/************************Without BSP************************/
#else
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* Platform dependant settings */
#define FTSDC021_FPGA_BASE      0xB0200000
#define FTSDC021_FPGA_IRQ       13

/* Enable FTSDC021_EDMA to use the external DMA
 * and function ftsdc021_Start_DMA must be implemented. */
//#define FTSDC021_EDMA

#define TRUE    1
#define FALSE   0

/**
 * Console
 */
struct cmd {
        char *name;
        char *usage;
        int32_t(*func) (int32_t argc, char * const argv[]);
};
typedef struct cmd cmd_t;
#define CMDLEN          50

char kbhit(void);
#endif

typedef unsigned char	BOOL;

void disable_dcache(void);

void reset_dll(void);
void wait_dll_lock(void);

typedef void (intr_handler_t)(void *);
void ftsdc021_setup_interrupt(uint32_t irq, intr_handler_t intr_handler);

void set_timeout(uint32_t ms);
uint32_t is_timeout();

#ifdef FTSDC021_EDMA
void edma_init(void);
/*
 * Burst size is 128 * 4 Bytes. Because sector size is 512 bytes.
 */
int ftsdc021_Start_DMA(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size,
                       uint32_t SrcWidth, uint32_t DstWidth, uint32_t SrcSize,
                       uint32_t SrcCtrl, uint32_t DstCtrl);

int ftsdc021_wait_DMA_done(uint32_t wait_t);
#endif
#endif
