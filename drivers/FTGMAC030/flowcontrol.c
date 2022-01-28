/**
 * (C) Copyright 2014 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "ftgmac030.h"

#if 0
/* for remote program only */
static unsigned char PauseFrame[60]=
{
	0x01, 0x80, 0xC2, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x88, 0x08, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

static volatile int enter_isr = 0;

static void flowcontrol_isr(FTGMAC030_Control *ctrl)
{
	enter_isr = 1;
}

static void flowcontrol(FTGMAC030_Control *ctrl)
{
	FTGMAC030_RXDESC *rxdes, *n_rxdes;
	int tmp;

	ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT | FTGMAC030_MACCR_ALLADDR, flowcontrol_isr);
	/* disable receive interrupts, intentionally not to rx any packets */
	mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
			   FTGMAC030_INT_RPKT_LOST);

	ctrl->set_flow_control(0xdd, 2, 5, 0, 1);

	prints("---Start to test flow control functionality.-----------------------------------\n");
	prints("---1. Receive pause frame testing.---------------------------------------------\n");
	prints("Wait for a pause frame...\n");
	while(ctrl->wait_pause_frame() != 1);
	prints("Receive a pause frame -> Passed!\n");

	prints("---2. Tramsmit pause frame (by TX_PAUSE) testing.------------------------------\n");
	ctrl->xmit_pause_frame();
	ctrl->set_flow_control(0, 2, 5, 0, 1);
	ctrl->xmit_pause_frame();

	prints("---3. Auto-send pause frame testing -(Low - High threshold) -------------------\n");
	ctrl->set_flow_control(0xabcd, 2, 5, 1, 1);

	rxdes = ctrl->rxdes_cur;
	while (rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) { 
		rxdes++; 
	}

	tmp = mac_regs->tfafcr;
	mac_regs->tfafcr &= ~(0x7 << 24);
	prints("Rx FIFO size 2 KB, low threshold is %d bytes\n", 2 * 256);
	/* In order to trigger Rx FIFO low threshold meet condition, We intentially 
	 * make next descriptor owned by software to make Rx FIFO free space less than
	 * 2 * 256 bytes.
	 * Remote program send 3 packets of 1500 bytes.
	 */
	n_rxdes = rxdes + 1;
	n_rxdes->rxdes0 |= FTGMAC030_RXDES0_RXPKT_RDY;

	/* Wait for first packet arrives */
	prints("Wait for first packets...\n");
	while (!(rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY)) { ; }

	 /* Must wait here in order to let FTGMAC030 sends pause 
	  * frame with pause time 0xabcd.
	  */
	ftgmac030_delaysecs(5);

	mac_regs->tfafcr = tmp;
	/* Return back to hardware, Rx FIFO free space hit high threshold and
	 * FTGMAC030 sends pause frame with pause time 0.
	 */
	n_rxdes->rxdes0 &= ~FTGMAC030_RXDES0_RXPKT_RDY;

	prints("---Finish testing flow control functionality.----------------------------------\n");
}

void flowcontrol_test_main(FTGMAC030_Control *ctrl)
{
	flowcontrol(ctrl);
}
