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
#ifndef __PORTME_FTGMAC030_H_
#define __PORTME_FTGMAC030_H_

#include "SoFlexible.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

//Replace with correct value relevant to board
#define FTGMAC030_REG_BASE      GMAC_FTGMAC030_0_PA_BASE
#define IRQ_FTGMAC030           GMAC_FTGMAC030_IRQ0

#define NPTX_DESC_ADDR          FREE_MEM_BASE
#define HPTX_DESC_ADDR          FREE_MEM_BASE + 0x00020000
#define RX_DESC_ADDR            FREE_MEM_BASE + 0x00040000
#define TX_BUF_ADDR             FREE_MEM_BASE + 0x00080000
#define RX_BUF_ADDR             FREE_MEM_BASE + 0x08000000

//Replace with correct value relevant to board
#define COUNTS_PER_SECOND       1000000000

#define COUNTS_PER_MILLI_SECOND (COUNTS_PER_SECOND/1000)

#define COUNTS_PER_MICRO_SECOND (COUNTS_PER_SECOND/1000000)

uint32_t ftgmac030_endian_swap32(uint32_t);
uint16_t ftgmac030_endian_swap16(uint16_t);
#define htons                   ftgmac030_endian_swap16
#define htonl                   ftgmac030_endian_swap32
#define ntohs                   ftgmac030_endian_swap16
#define ntohl                   ftgmac030_endian_swap32

#define prints(fmt, args...)    fLib_printf(fmt, ## args)
#define uart_getc               fLib_kbhit
#define uart_kbhit              fLib_getchar
#define scans(fmt, args...)     fLib_scanf(fmt, ## args)
#define puts                    fLib_putstr
#define putchar                 fLib_putchar

#define min_t(x,y)              ( x < y ? x : y )
#define max_t(x,y)              ( x > y ? x : y )

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

#define outl(addr, val)         (*((volatile uint32_t *)((uintptr_t)addr)) = val)
#define inl(addr)               *((volatile uint32_t *)((uintptr_t)addr))

#define udelay                  ftgmac030_timer_udelay

typedef uint8_t unchar;
typedef uint16_t ushort;
/*typedef uint32_t ulong;*/

void ftgmac030_init_platform(void *handler, void *data);
void ftgmac030_enable_timer_interrupt(void);
void ftgmac030_start_timer(void);
void ftgmac030_enable_interrupts();
void ftgmac030_disable_interrupts();
void ftgmac030_enable_caches();
void ftgmac030_disable_caches();
void ftgmac030_dcache_flushrange(uintptr_t buf, uint32_t len);
uint64_t ftgmac030_get_time(void);
uint64_t ftgmac030_get_ms(void);
void ftgmac030_timer_udelay(uint32_t usecs);

char uart_kbhit(void);
int scan_string(char *buf);
char uart_getc(void);

#endif
