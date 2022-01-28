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

static uint8_t MAC_DST[6] = {0x08,0x08,0x08,0x08,0x08,0x08};

static volatile uint32_t enter_isr = 0;
static volatile uint8_t g_rx_len = 0; /* a global rx length variable */
static volatile uint8_t *g_tx_buff; /* a global tx buffer pointer */
static volatile uint8_t g_rx_buff[RX_BUF_SIZE]; /* a global rx buffer pointer */

static void loopback_isr(FTGMAC030_Control *dev)
{
	if (0 == ftgmac030_addrcmp((const uint8_t *)MAC_DST, dev->rx_buff)) {
		enter_isr = 1;
		memcpy((void *)g_rx_buff, dev->rx_buff, dev->rx_len);
		g_rx_len = dev->rx_len - 4;
	}
}

/* initialize global buffer using static address (TX_BUF_ADDR) */
static void init_g_buffer(void)
{
	unsigned int i;
	int pattern_size = RX_BUF_SIZE;
	g_tx_buff = (uint8_t *)TX_BUF_ADDR;

	/* ensure 16-byte alignment */
	if (((uintptr_t)g_tx_buff&0x0000000F) != 0) {
		g_tx_buff = (uint8_t*)((uintptr_t)g_tx_buff&0xFFFFFFF0);
		g_tx_buff = (uint8_t *) g_tx_buff + 0x10;
	}
	
	/* initialize head mac address */
	for (i = 0; i < 6; i++) {
		g_tx_buff[i] = MAC_DST[i];
	}

	/* intialize global buffer with increasing values */
	for (; i < pattern_size; i++) {
		g_tx_buff[i] = i & 0xff;
	}
}

static void wait_packet(FTGMAC030_Control *dev, uint32_t len)
{
	int i;
	uint64_t t0, t1;
	prints("Wait for packet %lu bytes:", len);

	t0 = ftgmac030_get_time();
	while (!enter_isr) {
		/* wait at least 5 CONFIG_SYS_HZ */
		t1 = ftgmac030_get_time();
		if ((t1-t0) > (5ULL * COUNTS_PER_SECOND))
			break;
	}

	if (!enter_isr) {
		prints(" -> wait packet timeout !\n");
		return;
	}

	enter_isr = 0;

	prints("Receive Length %d\n", g_rx_len);

	if(len != g_rx_len){
		prints(" -> Failed!\n");
		prints("Press any key to continue...\n");
		uart_getc();
	}
	else{
		/* compare received buffer */
		for(i = 12; i < g_rx_len; i++){
			if(g_tx_buff[i] != g_rx_buff[i]){
				prints(" -> Failed!\n");
				prints("Error data %02x, should %02x at offset %d\n",
				       g_rx_buff[i], g_tx_buff[i], i);
				return ;
			}
		}
		prints(" -> Passed!\n");
	}
}

void loopback_test_main(FTGMAC030_Control *ctrl)
{
	int transmit_length = 0;
	int start_transmit_length = 64;
	int end_transmit_length = 72;

	prints("---Enter Loopback Test.-----------------------------------------\n");
	/* initialize global buffer */
	init_g_buffer();

	ctrl->open(ctrl, loopback_isr);
	
	prints("--- Internal loopback test ---\n");
	mac_regs->maccr |= FTGMAC030_MACCR_LOOPBACK;

	transmit_length = start_transmit_length;
	while(transmit_length <= end_transmit_length){
		memset((void *)ctrl->rx_buff, 0, transmit_length);
		ctrl->xmit(ctrl, (const uint32_t)((uintptr_t)g_tx_buff),
			   transmit_length, FTGMAC030_TXDES1_DEFAULT);
		wait_packet(ctrl, transmit_length);
		transmit_length++;
	}

	mac_regs->maccr &= ~FTGMAC030_MACCR_LOOPBACK;
	prints("--- External loopback test ---\n");
	transmit_length = start_transmit_length;
	while(transmit_length <= end_transmit_length) {
		memset((void *)ctrl->rx_buff, 0, transmit_length);
		ctrl->xmit(ctrl, (const uint32_t)((uintptr_t)g_tx_buff),
			   transmit_length, FTGMAC030_TXDES1_DEFAULT);
		ftgmac030_delaysecs(1);
		wait_packet(ctrl, transmit_length);
		transmit_length++;
	}

	prints("Finish Loopback Test.\n");
}
