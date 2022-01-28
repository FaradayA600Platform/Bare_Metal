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


struct pkt_hdr {
	char da[6];
	char sa[6];
	int unused;
	uint32_t seq;
};

static void stress_test_isr(FTGMAC030_Control *ctrl)
{
	prints("%s\n", __func__);
}

void tx_stress_test_main(FTGMAC030_Control *ctrl, int speed)
{
	FTGMAC030_TX_QUEUE *txq = &ctrl->tx_queue[ctrl->tx_queue_num];
	FTGMAC030_TXDESC *txdes;
	struct pkt_hdr *hdr;
	char buf[1536];
	int i, tx_cnt;

	ctrl->open(ctrl, stress_test_isr);

	if (ctrl->change_duplex_speed(ctrl, FULL, speed)) {
		prints("Change connection to %d duplex Failed.\n",
			(speed == 2) ? 10: (speed == 1) ? 100 : 1000);
		return;
	}

	/* disable receive and transmit interrupts for pollings */
	mac_regs->ier &= ~(FTGMAC030_INT_TPKT2F | FTGMAC030_INT_TPKT_LOST |
			   FTGMAC030_INT_TPKT2E | FTGMAC030_INT_AHB_ERR |
			   FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
			   FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST);

	hdr = (struct pkt_hdr *) buf;
	for (i=0; i < 6; i++) {
		hdr->da[i] = 0x2;
		hdr->sa[i] = 0x8;
	}

	/* Auto polling tx descriptor */
	mac_regs->aptc = FTGMAC030_APTC_TX_CNT(1) | 1;
	tx_cnt = 0;
	txdes = txq->txdes_top;
	while(1) {
		/* initial txdes */
		txdes->txdes3 = (int)((uintptr_t)buf);
		txdes->txdes1 = FTGMAC030_TXDES1_DEFAULT;
		txdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
		txdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(1514) |
				 FTGMAC030_TXDES0_FTS |
				 FTGMAC030_TXDES0_LTS;

		if (txdes->txdes0 & FTGMAC030_TXDES0_EDOTR)
			break;

		txdes = (FTGMAC030_TXDESC *) ((uintptr_t)txdes->txdes2);
	}
#if 0
	t0 = ftgmac030_get_time(0);
	txdes = txq->txdes_cur;
	do {
#if 0
		ch = uart_kbhit();

		/* print capture */
		if ('p' == ch || 'q' == ch) {
			t1 = ftgmac030_get_time(0);
			prints("tx_cnt = %lld tpkt_cnt = %ld\n", tx_cnt,
				mac_regs->tpkt_cnt);

			/* Convert microseconds to seconds */
			td = (t1 - t0) * 1.0 / 1000000.0;

			rate = (tx_len * 8 * 1.0) / td / 1000000;
			
			prints("tx %lld bytes in %.2f secs, rate %.3f MBit/s\n",
			       tx_len, td, rate);

			if ('q' == ch)
				break;
		}

		//hdr->seq = __be32(++tx_cnt);
		hdr->seq = ++tx_cnt;
		tx_len += 1500;

		txdes = txq->txdes_cur;
#endif		
		/* Descriptor own by HW */
		while (txdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN){ ; }
#if 0
		txdes->txdes3 = (int) buf;
		txdes->txdes1 = FTGMAC030_TXDES1_DEFAULT;
		txdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
		txdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(1500) |
				 FTGMAC030_TXDES0_FTS |
				 FTGMAC030_TXDES0_LTS |
				 FTGMAC030_TXDES0_TXDMA_OWN;
#else
		txdes->txdes0 |= FTGMAC030_TXDES0_TXDMA_OWN;
#endif
		txdes = (FTGMAC030_TXDESC *) txdes->txdes2;
#if 0
		txq->txdes_cur = (FTGMAC030_TXDESC *) txdes->txdes2;

		/* txq->kick_off(); */

		isr = mac_regs->isr;
		mac_regs->isr = isr;
		if (isr & FTGMAC030_INT_AHB_ERR)
			prints("tx AHB error\n");

		if (isr & FTGMAC030_INT_TPKT_LOST)
			prints("tx pkt lost\n");
#endif
	} while (1);
#endif
}

void rx_stress_test_main(FTGMAC030_Control *ctrl, int speed)
{
	FTGMAC030_RXDESC *rxdes;
	int capture;
	uint64_t t0, t1, t2 ;
	unsigned long long rx_len, rx_cnt, rpkt_cnt;
	float rate, td;
	char ch;

	ctrl->open(ctrl, stress_test_isr);
#if 0
	if (ctrl->change_duplex_speed(ctrl, FULL, speed)) {
		prints("Change connection to %d/%s duplex Failed.\n",
			(speed == 2) ? 10: (speed == 1) ? 100 : 1000);
		return;
	}
#endif
	/* disable receive and transmit interrupts for pollings */
	mac_regs->ier &= ~(FTGMAC030_INT_TPKT2F | FTGMAC030_INT_TPKT_LOST |
			   FTGMAC030_INT_TPKT2E | FTGMAC030_INT_AHB_ERR |
			   FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F |
			   FTGMAC030_INT_RXBUF_UNAVA | FTGMAC030_INT_RPKT_LOST);

	rxdes = ctrl->rxdes_cur;
	rx_len = rx_cnt = capture = 0;
	do {
#if 1
check:
		ch = uart_kbhit();
		if ('q' == ch)
			break;

		if ('p' == ch || (rx_cnt && !(rx_cnt & 0xfff)))
			prints("rx_cnt = %lld rx_len = %lld\n", rx_cnt, rx_len);

		/* start capture */
		if ('c' == ch) {
			rx_len = rx_cnt = 0;
			capture = 1;
			rpkt_cnt = mac_regs->rpkt_cnt;
			prints("Start capture ... rx_cnt = %lld, rx_len = %lld, "
			       "rpkt_cnt = %llu\n", rx_cnt, rx_len, rpkt_cnt);
		}

		/* stop capture */
		if ('e' == ch) {
			t1 = ftgmac030_get_time();
			prints("rx_cnt = %lld rpkt_cnt = %lld\n", rx_cnt,
				mac_regs->rpkt_cnt - rpkt_cnt);

			/* Convert microseconds to seconds */
			td = (t1 - t0) * 1.0 / 1000000.0;

			rate = (rx_len * 8 * 1.0) / td / 1000000;
			
			prints("End capture ... rx %lld bytes in %.2f secs, "
			       "rate %.3f MBit/s\n", rx_len, td, rate);

		}
#endif
		/* Descriptor own by HW */
		t1 = ftgmac030_get_time();
		while (!(rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY)) {
			t2 = ftgmac030_get_time();
			if ((t2 - t1) > 3000000)
				goto check;
		}

		if (capture) {
			t0 = ftgmac030_get_time();
			capture = 0;
		}
#if 1
		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FRS) {
			if (!(rxdes->rxdes0 & FTGMAC030_RXDES0_LRS))
				prints("rx not LRS\n");

			rx_len += (FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4);
#if 0
			memcpy((void *)buf, (void *)rxdes->rxdes3,
			       FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4);
#endif
			rx_cnt++;
		}

		if (rxdes->rxdes0 & (FTGMAC030_RXDES0_CRC_ERR | FTGMAC030_RXDES0_RX_ERR |
				     FTGMAC030_RXDES0_RX_ODD_NB | FTGMAC030_RXDES0_FIFO_FULL))
			prints("0x%x..", rxdes->rxdes0);
#endif
		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
		rxdes = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);

	} while (1);

/*	prints("rx_cnt = %lld rx_len = %lld\n", rx_cnt, rx_len); */
}
