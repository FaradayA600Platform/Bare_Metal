/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2020-July    BingYao           Example for port to relevant SoC or Board.
 * -------------------------------------------------------------------------
 */
#include "portme_ftspi020.h"

void ftspi020_enable_interrupts()
{

}

void ftspi020_disable_interrupts()
{

}

int ftspi020_setup_interrupt(uint32_t irq, void (*func)(void *data), void *data)
{
	return 0;
}

void ftspi020_setup_dma(int rd_chan, int wr_chan)
{
	board_init_dma();
}

uint64_t ftspi020_get_time(void)
{
	uint64_t t = 0;

	//Implement get ticks per second function.

	return (uint64_t) t;
}

void ftspi020_enable_caches()
{

}

void ftspi020_disable_caches()
{

}

void ftspi020_dcache_flushrange(uintptr_t buf, uint32_t len)
{			

}

void ftspi020_init_platform(void)
{

}

void ftspi020_cleanup_platform(void)
{

}
