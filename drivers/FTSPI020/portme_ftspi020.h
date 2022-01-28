/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  portme_ftspi020.h
 * DEPARTMENT :CTD/SD
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2020-July    BingYao           Example for port to relevant SoC or Board.
 * -------------------------------------------------------------------------
 */
#ifndef __PORTME_FTSPI020_H_
#define __PORTME_FTSPI020_H_

#include "SoFlexible.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <dmactrl.h>
#include <ftspi020.h>

#ifdef CONFIG_PLATFORM_A600

/*
 * How many CEs available on system?
 */
#define FTSPI020_MAXIMUM_CTRL       1
#define FTSPI020_MAX_CE_PER_CTRL    4
#define FTSPI020_TOTAL_CE           (FTSPI020_MAXIMUM_CTRL * FTSPI020_MAX_CE_PER_CTRL)

#undef FTSPI020_SUPPORT_DTR
#define FTSPI020_SUPPORT_QUAD
#define FTSPI020_USE_INTERRUPT
#define FTSPI020_USE_DMA

/* Replace with the correct register base address  */
#define FTSPI020_REG_BASE_0         SPI_FTSPI020_0_PA_BASE
#define FTSPI020_XIP_PORT_0         0x10000000
#define FTDMAC_REG_BASE             DMA_FTDMAC030_0_PA_BASE

/* Replace with the correct IRQ number  */
#define FTSPI020_IRQ_0              SPI_FTSPI020_IRQ

#define FTSPI020_RW_BUFFER_ADDR     FREE_MEM_BASE + 0x1000
#define FTSPI020_DMA_ADDR           (FTSPI020_RW_BUFFER_ADDR + (32 * 1024 * 1024))
#define FTSPI020_DMA_LLD_STRUCT_ADDR    (FTSPI020_DMA_ADDR + (32 * 1024 * 1024))

/* Fill the ticks per ms and us relevant to user board */
#define COUNTS_PER_MSECOND          1000
#define COUNTS_PER_USECOND          1000000

#define min_t(x,y)                  ( x < y ? x : y )
#define max_t(x,y)                  ( x > y ? x : y )

#define prints(fmt, args...)        fLib_printf(fmt, ## args)
#define uart_getc                   fLib_kbhit
#define uart_kbhit                  fLib_getchar
#define scan_string                 fLib_scanf

#ifdef FTSPI020_USE_DMA

#define FTSPI020_DMA_WR_CHNL        0
#define FTSPI020_DMA_RD_CHNL        0

// Select from 0 to 3 in FTSPI020_DMA_PRIORITY
#define FTSPI020_DMA_PRIORITY       3

extern uint32_t g_uint32_t_burst_sz;

int ftspi020_start_dma(uint32_t ctrl_id, uint32_t chan_id, uint32_t src_addr,
		       uint32_t dst_addr, uint32_t transfer_size,
		       uint32_t src_addr_width, uint32_t dst_addr_width, uint32_t maxburst,
		       uint32_t src_addr_ctrl, uint32_t dst_addr_ctrl, uint32_t priority);
#endif

#else
/*
 * How many CEs available on system?
 */
#define FTSPI020_MAXIMUM_CTRL	2
#define FTSPI020_MAX_CE_PER_CTRL    4
#define FTSPI020_TOTAL_CE           (FTSPI020_MAXIMUM_CTRL * FTSPI020_MAX_CE_PER_CTRL)

#undef FTSPI020_SUPPORT_DTR
#define FTSPI020_SUPPORT_QUAD
#define FTSPI020_USE_INTERRUPT
#define FTSPI020_USE_DMA


/* Replace with the correct register base address  */
#define FTSPI020_REG_BASE_0 0x43C00000
#define FTSPI020_REG_BASE_1 0x43C10000
#define FTSPI020_XIP_PORT_0 0x50000000
#define FTSPI020_XIP_PORT_1 0x60000000
#define FTDMAC_REG_BASE	0x43D00000

/* Replace with the correct IRQ number  */
#define FTSPI020_IRQ_0	0
#define FTSPI020_IRQ_1	1
#define FTDMAC030_IRQ_0	3
#define FTDMAC030_IRQ_1	4
#define FTDMAC030_IRQ_2	5

#define FTSPI020_RW_BUFFER_ADDR		0x10000000
#define FTSPI020_DMA_ADDR		(FTSPI020_RW_BUFFER_ADDR + (32 * 1024 * 1024))
#define FTSPI020_DMA_LLD_STRUCT_ADDR	(FTSPI020_DMA_ADDR + (32 * 1024 * 1024))

#define FTSPI020_REQ_SEL_DEV0	0
#define FTSPI020_REQ_SEL_DEV1	1

/* Fill the ticks per ms and us relevant to user board */
#define COUNTS_PER_MSECOND	1000
#define COUNTS_PER_USECOND	1000000

#define min_t(x,y)	( x < y ? x : y )
#define max_t(x,y)	( x > y ? x : y )

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define prints(fmt, args...)	printf(fmt, ## args)

#define scans(fmt, args...)	scanf(fmt, ## args)

#ifdef FTSPI020_USE_DMA

#define FTSPI020_DMA_WR_CHNL	0
#define FTSPI020_DMA_RD_CHNL	0

// Select from 0 to 3 in FTSPI020_DMA_PRIORITY
#define FTSPI020_DMA_PRIORITY	3

extern uint32_t g_uint32_t_burst_sz;

int ftspi020_start_dma(uint32_t ctrl_id, uint32_t chan_id, uint32_t src_addr,
		       uint32_t dst_addr, uint32_t transfer_size,
		       uint32_t src_addr_width, uint32_t dst_addr_width, uint32_t maxburst,
		       uint32_t src_addr_ctrl, uint32_t dst_addr_ctrl, uint32_t priority);
#endif

void ftspi020_enable_interrupts();
void ftspi020_disable_interrupts();
void ftspi020_enable_caches();
void ftspi020_disable_caches();
void ftspi020_dcache_flushrange(uintptr_t buf, uint32_t len);

char uart_kbhit(void);
int scan_string(char *buf);
char uart_getc(void);
int ftspi020_setup_interrupt(uint32_t irq, void (*func)(void *data), void *data);
void ftspi020_setup_dma(int rd_chan, int wr_chan);
uint64_t ftspi020_get_time(void);
void ftspi020_init_platform(void);
void ftspi020_cleanup_platform(void);

#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)
#define BIT6 (1 << 6)
#define BIT7 (1 << 7)
#define BIT8 (1 << 8)
#define BIT9 (1 << 9)
#define BIT10 (1 << 10)
#define BIT11 (1 << 11)
#define BIT12 (1 << 12)
#define BIT13 (1 << 13)
#define BIT14 (1 << 14)
#define BIT15 (1 << 15)
#define BIT16 (1 << 16)
#define BIT17 (1 << 17)
#define BIT18 (1 << 18)
#define BIT19 (1 << 19)
#define BIT20 (1 << 20)
#define BIT21 (1 << 21)
#define BIT22 (1 << 22)
#define BIT23 (1 << 23)
#define BIT24 (1 << 24)
#define BIT25 (1 << 25)
#define BIT26 (1 << 26)
#define BIT27 (1 << 27)
#define BIT28 (1 << 28)
#define BIT29 (1 << 29)
#define BIT30 (1 << 30)
#define BIT31 (1 << 31)
#endif

#endif
