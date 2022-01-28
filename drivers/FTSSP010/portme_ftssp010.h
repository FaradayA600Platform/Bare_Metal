/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  portme.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2015   	Bing-Yao	 Porting file for different platform        
 * -------------------------------------------------------------------------
 */

/************************NonOS Lite************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <autoconf.h>

#if defined(CONFIG_BSP_LITE)
#include "common.h"

#define FTSSP010_REG_BASE_M	0x43D00000
#define FTSSP010_REG_BASE_S	0x43E00000
#define FTSSP010_IRQ		22

#define printf prints

/************************   ZYNQ   ************************/
#elif defined(CONFIG_BSP_ZYNQ)
#include <Xil_io.h>
#include "xscugic.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xuartps_hw.h"

#define FTSSP010_REG_BASE_M	0x43D00000
#define FTSSP010_REG_BASE_S	0x43E00000
#define FTSSP010_IRQ		XPAR_FABRIC_IRQ_F2P0_INTR
#define GIC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

struct cmd {
	char *name;
	char *usage;
	int32_t(*func) (int32_t argc, char *const argv[]);
};
typedef struct cmd cmd_t;
#define CMDLEN          50

#undef printf
#define printf xil_printf

char kbhit(void);

/************************   A600   ************************/
#elif defined(CONFIG_PLATFORM_A600)
#include "SoFlexible.h"

#define FTSSP010_REG_BASE_M	SSP_FTSSP010_0_PA_BASE
#define FTSSP010_REG_BASE_S	SSP_FTSSP010_1_PA_BASE
#define FTSSP010_IRQ		SSP_FTSSP010_0_IRQ

struct cmd {
	char *name;
	char *usage;
	int32_t(*func) (int32_t argc, char *const argv[]);
};
typedef struct cmd cmd_t;

#define getchar fLib_getchar

#define printf fLib_printf

/************************Without BSP************************/
#else
#define FTSSP010_REG_BASE_M	0x43D00000
#define FTSSP010_REG_BASE_S	0x43E00000
#define FTSSP010_IRQ		22

struct cmd {
	char *name;
	char *usage;
	int32_t(*func) (int32_t argc, char *const argv[]);
};
typedef struct cmd cmd_t;
#define CMDLEN          50

char kbhit(void);
#endif

#ifdef outl
#undef outl
#undef inl
#endif
#define outl(val, addr)		(*(volatile unsigned int *)(addr) = (val))
#define inl(addr)		(*(volatile unsigned int *)(addr))

typedef void (interrupt_handler_t)(void *);
int ftssp010_setup_interrupt(unsigned int irq, interrupt_handler_t intr_handler);
void ftssp010_platform_init(void);
