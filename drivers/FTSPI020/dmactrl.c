/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2020-July    BingYao	          Implement DMA related functions here.
 * -------------------------------------------------------------------------
 */

#include <portme_ftspi020.h>

#include "dmactrl.h"

/**
 * Initialize DMA controller
 */
void board_init_dma(void)
{		

}

/**
 * Start the DMA transfer
 */
int ftspi020_start_dma(uint32_t ctrl_id, uint32_t chan_id, uint32_t src_addr,
		       uint32_t dst_addr, uint32_t transfer_size,
		       uint32_t src_addr_width, uint32_t dst_addr_width, uint32_t maxburst,
		       uint32_t src_addr_ctrl, uint32_t dst_addr_ctrl, uint32_t priority)
{
	return 0;
}
