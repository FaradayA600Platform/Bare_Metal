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

static unsigned int clean_up_descriptor(FTGMAC030_Control *ctrl, uint32_t len)
{
	FTGMAC030_RXDESC *rxdes;
	int rx_packet_cnt;

	rx_packet_cnt = 0;

	rxdes = ctrl->rxdes_cur;
	while (rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) {

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FRS) {
			if (!(rxdes->rxdes0 & FTGMAC030_RXDES0_LRS))
				prints(" -> rx not LRS\n");

			if (len == (FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4))
				rx_packet_cnt++;
		
			prints(" -> rx with length %d\n",
			       (FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4));
		} else {
			prints(" -> rx not FRS\n");
		} 

		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
		rxdes->rxdes1 = 0x0;

		/* check next descriptor */
		ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
		rxdes = ctrl->rxdes_cur;
	}

	return rx_packet_cnt;
}

/**
 * Return microseonds wait time.
 */
static int wait_packet_cycl(FTGMAC030_Control *ctrl, uint32_t len,
			    unsigned int wait_us)
{
	uint64_t t0, t1;
	int sts, ret;

	t0 = ftgmac030_get_time();
	do {
		sts = mac_regs->isr;
		t1 = ftgmac030_get_time();
		ret = (t1 - t0) / COUNTS_PER_MICRO_SECOND;
		mac_regs->isr = sts;

		if (sts & FTGMAC030_INT_RPKT2B)
			break;

		/* we already wait 0x1000 us more than specified timeout,
		 * just leave this loop.
		 */
		if (ret > (wait_us + 0x1000))
			break;

	} while(1);

	if (sts & FTGMAC030_INT_RPKT2B)
		clean_up_descriptor(ctrl, len);

	return ret;
}

/**
 * Return number of receive packets 
 */
static int wait_packets_threshold(FTGMAC030_Control *ctrl,
				  uint32_t len, int num)
{
	uint64_t t0, t1;
	int sts, ret;

	t0 = ftgmac030_get_time();
	do {
		sts = mac_regs->isr;
		t1 = ftgmac030_get_time();
		ret = (t1 - t0) / COUNTS_PER_MICRO_SECOND;
		mac_regs->isr = sts;

		if (t1 - t0 > 0xffffffff)
			break;

		if (sts & FTGMAC030_INT_RPKT2B)
			break;
	} while(1);
	
	if (sts & FTGMAC030_INT_RPKT2B)
		 ret = clean_up_descriptor(ctrl, len);

	return ret;
}

static void interrupttimer_isr(FTGMAC030_Control *ctrl)
{
	prints("Should not receive interrupt\n");
}

/* return the tick time as soon as the packet is sent */
static uint64_t interrupt_send(FTGMAC030_Control *ctrl, int len, int idx)
{
	FTGMAC030_TX_QUEUE *txq = &ctrl->tx_queue[ctrl->tx_queue_num];
	FTGMAC030_TXDESC *txdes;
	uint64_t t0;

	prints(" -> [%d] Send %d bytes packet:\n", idx, len);

	/* Clear interrupt status before transmit packet */
	if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
		prints(" -> ISR before tx\n");
	}
	/* send packet */

	txdes = txq->txdes_cur;

	if(txdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){
		prints("??? => tx descriptor 0x%x own by HW\n", (uintptr_t)txdes);
		t0 = ftgmac030_get_time();
		return t0;
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

	if (ctrl->tx_queue_num == NP_TX_QUEUE)
		mac_regs->nptxpd = 0xffffffff;
	else
		mac_regs->hptxpd = 0xffffffff;

	t0 = ftgmac030_get_time();
	return t0;
}

static int intr_timer_run_test(FTGMAC030_Control *ctrl, SPEED spd)
{
	int i, ret, thr_cnt, cyl_cnt, unit;
	uint64_t t0, t1;

	prints("---1. TXITC Test --------------------------------------------------\n");
	prints("case 1: (TXITC_CYL = 0, TXITC_THR = 0)\n");
	mac_regs->txitc = 0;
	interrupt_send(ctrl, 100, 1);
	udelay(100);
	/* check intr status */
	if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
		prints(" -> Passed\n");
		mac_regs->isr = (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F |
			 	 FTGMAC030_INT_TPKT_LOST);
	} else
		prints(" -> Failed\n");

	prints("case1: (TXITC_CYL = 0, TXITC_THR = 1)\n");
	mac_regs->txitc = FTGMAC030_TXITC_THR(1);
	interrupt_send(ctrl, 100, 1);
	udelay(100);
	/* check intr status */
	if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
		prints(" -> Passed\n");
		mac_regs->isr = (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F |
				 FTGMAC030_INT_TPKT_LOST);
	} else
		prints(" -> Failed\n");

	ret = 0;
	for (thr_cnt=2; thr_cnt < 8; thr_cnt+=2) {

		for (unit=0; unit < 3; unit++) {
			int j;

			mac_regs->txitc = FTGMAC030_TXITC_THR(thr_cnt) | FTGMAC030_TXITC_THR_UNIT(unit);
			prints("case 3: (TXITC_CYL = 0, TXITC_THR = %d, TXITC_THR_UNIT = %d, txitc = 0x%x)\n",
				thr_cnt, unit, mac_regs->txitc);

			j = (1 << (unit * 2)) * thr_cnt;
			ret = 0;

			for (i = 1; i <= j; i++) {
				interrupt_send(ctrl, 100 + i, i);
				ftgmac030_delaysecs(1);
				/* Should not have interrupt before the number of
				 * transmitted packes equal to thr_cnt * unit
				 */
				if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
					mac_regs->isr = (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F |
							 FTGMAC030_INT_TPKT_LOST);

					if (!ret && (i != j)) {
						ret = 1;
						prints(" -> Failed\n");
					}
				}

			}

			if (!ret)
				prints(" -> Passed\n");
		}
	}

	/* When set, the TX cycle times are:
	 *	1000 Mbps mode -> 16.384 us 
	 *	100 Mbps mode -> 81.92 us
	 *	10 Mbps mode ->819.2 us
	 *  When cleared, the TX cycle times are:
	 *	1000 Mbps mode -> 1.024 us
	 *	100 Mbps mode -> 5.12 us
	 *	10 Mbps mode -> 51.2 us
	 *
	 * Maximum cycles can be set is 0xff:
	 * 	1. 255 * 16.384 us = 4177.92 us
	 * 	2. 255 * 81.92 us = 20889.6 us
	 * 	3. 255 * 819.2 us = 208896 us
	 * Need to adjust CONFIG_SYS_HZ to 1000000 to get microseconds resoultion.
	 */
	for(cyl_cnt = 1; cyl_cnt <= 4; cyl_cnt++){

		i = (cyl_cnt * 63) + 3;
		mac_regs->txitc = (FTGMAC030_TXITC_CYL(i) | FTGMAC030_TXITC_TIME_SEL |
				   FTGMAC030_TXITC_THR(7));
		prints("case 4: (TXITC_CYL = %d, TXITC_THR = 7, txitc = 0x%08x)\n",
			i, mac_regs->txitc);

		if (spd == _1000)
			i = (i * 16) + (cyl_cnt * 25);
		else if (spd == _100)
			i = i * 82;
		else if (spd == _10)
			i = i * 820;
		else
			prints("-> Speed error \n");

		t0 = interrupt_send(ctrl, 100, 1);

		do {
			t1 = ftgmac030_get_time();
			if (mac_regs->isr & FTGMAC030_INT_TPKT2E)
				break;

			if ((t1 - t0) > (i + 1000))
				break;
		} while (1);

		prints("ISR at %llu us, must be at about %d us\n",
		       (t1 - t0) / COUNTS_PER_MICRO_SECOND, i);

		mac_regs->isr = (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F |
				FTGMAC030_INT_TPKT_LOST);
	}

	mac_regs->txitc = FTGMAC030_TXITC_DEFAULT;

	prints("---2. RXITC Test --------------------------------------------------\n");
	prints("case 1: (RXITC_RST = 0, RXITC_CYL = 0, RXITC_THR = 0)\n");
	mac_regs->rxitc = 0;
	ret = wait_packets_threshold(ctrl, 600, 1);
	if (!ret)
		prints(" wait time out -> Failed\n");
	else
		prints(" -> Passed\n");
		
	/* set maximum to rxint_cnt and check one INT to server two or more packets */
	prints("case2: (RXITC_RST = 0,RXITC_CYL = 0, RXITC_THR = 1)\n");
	mac_regs->rxitc = FTGMAC030_RXITC_THR(1);
	ret = wait_packets_threshold(ctrl, 600, 1);
	if (!ret)
		prints(" wait time out -> Failed\n");
	else
		prints(" -> Passed\n");


	for (thr_cnt=2; thr_cnt < 8; thr_cnt+=2) {

		for (unit=0; unit < 3; unit++) {
			int j;

			mac_regs->rxitc = FTGMAC030_RXITC_THR(thr_cnt) | FTGMAC030_RXITC_THR_UNIT(unit);
			prints("case 3: (RXITC_RST = 0, RXITC_CYL = 0, RXITC_THR = %d, RXITC_THR_UNIT = %d, "
			       "rxitc = 0x%08x)\n", thr_cnt, unit, mac_regs->rxitc);

			j = (1 << (unit * 2)) * thr_cnt;

			ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 100, FTGMAC030_TXDES1_DEFAULT);

			ret = wait_packets_threshold(ctrl, 600, j);
			if (ret != j)
				prints(" -> Failed(rx %d packets) !\n", ret);
			else
				prints(" -> Passed\n");
		}
	}

	ftgmac030_delaysecs(1);

	for(cyl_cnt = 1; cyl_cnt <= 4; cyl_cnt++){

		i = (cyl_cnt * 63) + 3;
		mac_regs->rxitc = (FTGMAC030_RXITC_CYL(i) | FTGMAC030_RXITC_TIME_SEL |
				   FTGMAC030_RXITC_THR(7));
		prints("case 4: (RXITC_RST = 0, RXITC_CYL = %d, RXITC_THR = 7, rxitc = 0x%08x)\n",
			i, mac_regs->rxitc);

		ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 100, FTGMAC030_TXDES1_DEFAULT);

		if (spd == _1000) {
			i = (i * 16) + (cyl_cnt * 25);
			udelay(100);
		} else if (spd == _100) {
			i = i * 82;
			udelay(500);
		} else if (spd == _10) {
			i = i * 819;
			udelay(1000);
		} else
			prints("-> Speed error \n");

		ret = wait_packet_cycl(ctrl, 600, i);

		prints(" -> ISR must after %d us, happen at %d us!\n", i, ret);
	
	}

	for(cyl_cnt = 1; cyl_cnt <= 64; cyl_cnt*=4){

		mac_regs->rxitc = (FTGMAC030_RXITC_CYL(255) | FTGMAC030_RXITC_TIME_SEL |
				   FTGMAC030_RXITC_THR(7) | FTGMAC030_RXITC_RST(cyl_cnt));

		prints("case 5: (RXITC_RST = %d, RXITC_CYL = 255, RXITC_THR = 7, rxitc = 0x%08x)\n",
			cyl_cnt, mac_regs->rxitc);

		ctrl->xmit(ctrl, (uint32_t)TX_BUF_ADDR, 100, FTGMAC030_TXDES1_DEFAULT);

		if (spd == _1000) {
			i = cyl_cnt * 17;
			udelay(100);
		} else if (spd == _100) {
			i = cyl_cnt * 82;
			udelay(500);
		} else if (spd == _10) {
			i = cyl_cnt * 820;
			udelay(1000);
		} else
			prints("-> Speed error \n");

		ret = wait_packet_cycl(ctrl, 600, i);

		prints(" -> ISR must after %d us, happen at %d us!\n", i, ret);
	
	}

	mac_regs->rxitc = FTGMAC030_RXITC_DEFAULT;

	return ret;
}

void intr_timer_test_main(FTGMAC030_Control *ctrl)
{
	uint8_t *tx_buff = (uint8_t *)TX_BUF_ADDR;
	uint32_t i, speed, duplex;

	prints("Intterrupt Timer test.\n");
	prints("Please run remote auto polling test first.\n");
	prints("Press any key to continue...\n");
	uart_getc();

	/* initial buffer */
	memcpy(tx_buff, MAC_DST, 6);
	memcpy(tx_buff + 6, MAC_SRC, 6);
	for (i = 12; i < 1024; i++) {
		tx_buff[i] = i & 0xff;
	}
	
	/* _1000 = 0, _100  = 1, _10   = 2 */
	for (speed = 0; speed < 3; speed++) {

		if ('q' == uart_kbhit())
			break;

		ctrl->open(ctrl, interrupttimer_isr);

		/* disable receive and transmit interrupts for pollings */
		mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
				   FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

		mac_regs->isr = (FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
				 FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

		/* FULL = 0, HALF = 1 */
		for (duplex = 0; duplex < 2; duplex++) {

			/* PhY usually not support 1000-Half */
			if (speed == 0 && duplex == 1)
				continue;

			prints("--- Interrupt test in %d/%s duplex mode.---\n",
				(speed == 2) ? 10: (speed == 1) ? 100 : 1000,
				duplex ? "Half" : "Full");

			if (ctrl->change_duplex_speed(ctrl, duplex, speed))
				continue;

			intr_timer_run_test(ctrl, speed);
		}
	}
}
