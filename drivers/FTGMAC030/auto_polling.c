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


static const uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
static const uint8_t MAC_DST[6] = {0x02,0x02,0x02,0x02,0x02,0x02};

static void autopolling_isr(FTGMAC030_Control *ctrl)
{
	/* plus 2 for zero copy */
	prints("Receive a packet which of header is %02x:%02x:%02x:%02x:%02x:%02x\n",
	       ctrl->rx_buff[0], ctrl->rx_buff[1], ctrl->rx_buff[2], 
	       ctrl->rx_buff[3], ctrl->rx_buff[4], ctrl->rx_buff[5]);

	if (0 == ftgmac030_addrcmp(ctrl->rx_buff, MAC_SRC))
		prints(" -> Passed!\n");
}

/* Prepare the packet by filling tx descriptor */
static int autopolling_prepare_packet(FTGMAC030_Control *ctrl, int len)
{
	FTGMAC030_TX_QUEUE *txq = &ctrl->tx_queue[ctrl->tx_queue_num];
	FTGMAC030_TXDESC *txdes;

	prints("-> Prepare %d bytes packet\n", len);

	/* Clear interrupt status before transmit packet */
	if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
		int sts = mac_regs->isr;
		mac_regs->isr = sts;
		prints(" -> ISR(0x%08x) before tx\n", sts);
	}
	/* send packet */

	txdes = txq->txdes_cur;

	if(txdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){
		prints("??? => tx descriptor 0x%x own by HW\n", (uintptr_t)txdes);
		return 1;
	}

	/* initial txdes */
	txdes->txdes3 = TX_BUF_ADDR;
	txdes->txdes1 = FTGMAC030_TXDES1_DEFAULT;
	/* reset txdes0 */
	txdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
	txdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(len) |
			 FTGMAC030_TXDES0_FTS |
			 FTGMAC030_TXDES0_LTS |
			 FTGMAC030_TXDES0_TXDMA_OWN;

	txq->txdes_cur = (FTGMAC030_TXDESC *) ((uintptr_t)txdes->txdes2);

	return 0;
}

/* Return receive length, only expect one packet */
static uint32_t clean_up_descriptor(FTGMAC030_Control *ctrl)
{
	FTGMAC030_RXDESC *rxdes;
	uint32_t len;

	len = 0;

	rxdes = ctrl->rxdes_cur;
	while (rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) {

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FRS) {
			if (!(rxdes->rxdes0 & FTGMAC030_RXDES0_LRS))
				prints(" -> rx not LRS\n");

			len =FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4;
		
			prints(" -> rx with length %lu\n", len);
		} else {
			prints(" -> rx not FRS\n");
		} 

		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;

		rxdes->rxdes1 = 0x0;

		ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
		/* For auto polling disable we must set descriptor own bit to software
		 * and only expect to receive one descriptor, so just leave the loop.
		 */
		if (FTGMAC030_APTC_RX_CNT(ctrl->aptc) == 0) {
			rxdes->rxdes0 |= FTGMAC030_RXDES0_RXPKT_RDY;
			break;
		}

		/* check next descriptor */
		rxdes = ctrl->rxdes_cur;
	}

	return len;
}

static void auto_polling_wait_tx_isr(void)
{
	uint32_t sts;
	uint64_t t0, t1, td, td1, td2;

	t0 = t1 = td1 = td2 = 0;
	t0 = ftgmac030_get_time();
	do {
		t1 = ftgmac030_get_time();
		td = t1 - t0;
		sts = mac_regs->isr;
		mac_regs->isr = sts;

		if (sts & FTGMAC030_INT_TPKT2F)
			td1 = td;

		if (sts & FTGMAC030_INT_TPKT2E) {
			td2 = td;
			break;
		}
	} while (1);

	if (td1)
		prints("-> Data moved to Tx FIFO(%llu us) ",
		      (td1/COUNTS_PER_SECOND));

	if (td2)
		prints("-> Tx to Ethernet OK(%llu us) -> Passed\n",
		       (td2/COUNTS_PER_SECOND));
}

static void auto_polling_wait_rx_isr(FTGMAC030_Control *ctrl)
{
	uint32_t sts;
	uint64_t t0, t1, td, td1, td2;

	t0 = t1 = td1 = td2 = 0;
	t0 = ftgmac030_get_time();
	do {
		t1 = ftgmac030_get_time();
		td = t1 - t0;
		sts = mac_regs->isr;
		mac_regs->isr = sts;

		if (sts & FTGMAC030_INT_RPKT2F)
			td1 = td;

		if (sts & FTGMAC030_INT_RPKT2B) {
			td2 = td;
			clean_up_descriptor(ctrl);
			break;
		}
	} while (1);

	if (td1)
		prints("-> Packet moved to Rx FIFO(%llu us) ", (td1/COUNTS_PER_SECOND));

	if (td2)
		prints("-> RXDMA moved packets to buffer OK(%llu us) -> Passed\n",
		       (td2/COUNTS_PER_SECOND));
}

void auto_polling_disable_run_test(FTGMAC030_Control *ctrl)
{
	FTGMAC030_RXDESC *rxdes;
	uint32_t i, sts;

	prints("---1. TX/Disable Auto Polling----------------------------------------\n");
	for (i=1; i <= 3; i++) {
		prints("[Test1] Tx by manual kickoff ");
		autopolling_prepare_packet(ctrl, (500 * i));

		sts = mac_regs->isr;
		if (sts & (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F |
				     FTGMAC030_INT_TPKT_LOST)) {
			prints(" -> Tx ISR before kickoff -> Failed\n");
		} else {
			if (ctrl->tx_queue_num == NP_TX_QUEUE)
				mac_regs->nptxpd = 0xffffffff;
			else
				mac_regs->hptxpd = 0xffffffff;

			auto_polling_wait_tx_isr();
		}
	}

	prints("---2. RX/Disable Auto Polling----------------------------------------\n");
	for (i=1; i <= 3; i++) {
		prints("[Test2] Rx by manual kickoff");
	
		rxdes = ctrl->rxdes_cur;
		/* return this descriptor to HW */
		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;

		/* Send packet to trigger remote program send back packet */
		ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 1234, FTGMAC030_TXDES1_DEFAULT);
		ftgmac030_delaysecs(1);

		sts = mac_regs->isr;
		if (sts & FTGMAC030_INT_RPKT2B)
			prints(" -> Rx ISR before kickoff -> Failed\n");
		else 
			mac_regs->rxpd = 0xffffffff;

		auto_polling_wait_rx_isr(ctrl);		
	}

	prints("---3. RX/Disable Auto Polling Multiple Rx----------------------------\n");
	/* Send packet to trigger remote program send back 10 packets.
	 * GMAC030 has 16K rx FIFO, should enough to hold these packets. 
	 */
	ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 1234, FTGMAC030_TXDES1_DEFAULT);
	ftgmac030_delaysecs(1);

	for (i=1; i <= 10; i++) {
		prints("[Test3] Multi Rx one descriptor");
	
		rxdes = ctrl->rxdes_cur;
		/* return this descriptor to HW */
		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;

		mac_regs->rxpd = 0xffffffff;

		if (mac_regs->isr & FTGMAC030_INT_RXBUF_UNAVA)
			prints(" -> Rx Buff Unavailable\n");

		if (mac_regs->isr & FTGMAC030_INT_RPKT_LOST)
			prints(" -> Rx Packete Lost\n");

		auto_polling_wait_rx_isr(ctrl);
	}
}

void auto_polling_enable_run_test(FTGMAC030_Control *ctrl, SPEED spd)
{
	uint32_t i, j;

	prints("---4. TX/Enable Auto Polling-----------------------------------------\n");
	for (i=1; i<=30; i++) {
		if (i > 15) {
			if (spd == _1000) {
				j = (i - 15) * 17;
			} else if (spd == _100) {
				j = (i - 15) * 82;
			} else if (spd == _10) {
				j = (i - 15) * 820;
			} else
				prints("-> Speed error \n");

			mac_regs->aptc = FTGMAC030_APTC_TX_CNT((i-15));
			mac_regs->aptc |= FTGMAC030_APTC_TX_TIME;
			prints("[Test4] (TXPOLL_CNT = %lu, TXPOLL_TIME_SEL = 1, wait "
			       "%lu us)\n", (i - 15), j);
		} else {
			if (spd == _1000) {
				j = i * 2;
			} else if (spd == _100) {
				j = i * 6;
			} else if (spd == _10) {
				j = i * 52;
			} else
				prints("-> Speed error \n");

			mac_regs->aptc = FTGMAC030_APTC_TX_CNT(i);
			prints("[Test4] (TXPOLL_CNT = %lu, TXPOLL_TIME_SEL = 0, wait "
			       "%lu us)\n", i, j);
		}

		autopolling_prepare_packet(ctrl, (500 + i));

		auto_polling_wait_tx_isr();

	}

	prints("---5. RX/Enable Auto Polling-----------------------------------------\n");
	for (i=1; i <= 30; i++) {
		if (i > 15) {
			if (spd == _1000) {
				j = (i - 15) * 17;
			} else if (spd == _100) {
				j = (i - 15) * 82;
			} else if (spd == _10) {
				j = (i - 15) * 820;
			} else
				prints("-> Speed error \n");

			mac_regs->aptc = FTGMAC030_APTC_RX_CNT((i-15));
			mac_regs->aptc |= FTGMAC030_APTC_RX_TIME;
			prints("[Test5] (RXPOLL_CNT = %lu, RXPOLL_TIME_SEL = 1, wait "
			       "%lu us)\n", (i - 15), j);
		} else {
			if (spd == _1000) {
				j = i * 2;
			} else if (spd == _100) {
				j = i * 6;
			} else if (spd == _10) {
				j = i * 52;
			} else
				prints("-> Speed error \n");

			mac_regs->aptc = FTGMAC030_APTC_RX_CNT(i);
			prints("[Test5] (RXPOLL_CNT = %lu, RXPOLL_TIME_SEL = 0, wait "
			       "%lu us)\n", i, j);
		}

		/* Send packet to trigger remote program send back packet */
		ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 1234, FTGMAC030_TXDES1_DEFAULT);

		auto_polling_wait_rx_isr(ctrl);

	}

	mac_regs->aptc = FTGMAC030_APTC_DEFAULT;
}

extern int giga_mode;
void auto_polling_timer_test_main(FTGMAC030_Control *ctrl)
{
	uint8_t *tx_buff = (uint8_t *)TX_BUF_ADDR;
	int i, speed, duplex;

	prints("Auto Polling Timer test.\n");
	prints("Please run remote auto polling test first.\n");
	prints("Press any key to continue...\n");
	uart_getc();

	/* initial buffer */
	memcpy(tx_buff, MAC_DST, 6);
	memcpy(tx_buff + 6, MAC_SRC, 6);
	for (i = 12; i < 1600; i++) {
		tx_buff[i] = i & 0xff;
	}

	/* _1000 = 0, _100  = 1, _10   = 2 */
	speed = (!giga_mode) ? 1 : 0;
	for (;speed < 3; speed++) {

		if ('q' == uart_kbhit())
			break;

		/* FULL = 0, HALF = 1 */
		for (duplex = 0; duplex < 2; duplex++) {

			/* PhY usually not support 1000-Half */
			if (speed == 0 && duplex == 1)
				continue;

			prints("--- Auto polling test in %d/%s duplex mode.---\n",
				(speed == 2) ? 10: (speed == 1) ? 100 : 1000,
				duplex ? "Half" : "Full");

			if (ctrl->change_duplex_speed(ctrl, duplex, speed))
				continue;
			// For disable auto polling we must set rx descriptor own
			// bit to software
			ctrl->open_aptc(ctrl, FTGMAC030_APTC_RX_CNT(0) |
					FTGMAC030_APTC_TX_CNT(0),
					autopolling_isr);

			/* disable receive and transmit interrupts for pollings */
			mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
					   FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST |
					   FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

			mac_regs->isr = (FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
					 FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST |
					 FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

			auto_polling_disable_run_test(ctrl);

			ctrl->open_aptc(ctrl, FTGMAC030_APTC_RX_CNT(1) |
					FTGMAC030_APTC_TX_CNT(1) |
					FTGMAC030_APTC_TX_TIME, autopolling_isr);

			/* disable receive and transmit interrupts for pollings */
			mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
					   FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST |
					   FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

			mac_regs->isr = (FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
					 FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST |
					 FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

			auto_polling_enable_run_test(ctrl, speed);
		}
	}
	prints("Finish Auto Polling Timer test.\n");
}
