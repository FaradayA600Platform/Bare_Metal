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
#include <malloc.h>

#include "ftgmac030.h"

#define TX_TEST_PKT_SIZE	2048

static uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
static uint8_t MAC_DST[6] = {0x02,0x02,0x02,0x02,0x02,0x02};
static char send_buf[TX_TEST_PKT_SIZE];
static int enter_isr = 0;


static void transmission_isr(FTGMAC030_Control *ctrl)
{
	enter_isr = 1;
}

static void tx_wait_complete(void)
{
	uint64_t t0, t1;

	t0 = ftgmac030_get_time();
	while (!enter_isr) {
		/* wait at least 5 CONFIG_SYS_HZ */
		t1 = ftgmac030_get_time();

		if (((t1 - t0) / COUNTS_PER_SECOND)  > 5)
			break;
	}

	if (enter_isr)
		prints("tx ok\n");
	else
		prints("tx not complete\n");

}

static void send_frame(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	prints("->Send %d byte frame:\n", len);
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), len, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();
}

static void send_multideses(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	int i;
	int numDescript;
	struct multi_desc *deses = NULL;
	/* split into a small segment size is 100 byets */
	numDescript = len / 100;
	prints("->Send %d byte frame:\n", len);
	prints("Frame is divided into %d descriptors.\n", numDescript);

	deses = (struct multi_desc *)malloc(numDescript * sizeof(struct multi_desc));
	memset(deses, 0, numDescript * sizeof(struct multi_desc));
	for(i = 0; i< numDescript;i++)
	{
		deses[i].phy_tx_addr = (uint32_t)((uintptr_t)(buff + i * 100));
		deses[i].size = 100;
	}

	enter_isr = 0;
	ctrl->xmit_multi_desc(&ctrl->tx_queue[ctrl->tx_queue_num], numDescript, deses,
			      FTGMAC030_TXDES1_DEFAULT);
	free(deses);
	deses = NULL;
	tx_wait_complete();
}

static void send_diff_base(FTGMAC030_Control *ctrl, uint8_t *buff, int offset)
{
	int i;
	prints("->Offset %d frame:\n", offset);
	for(i = 60;i<64;i++)
	{
		prints("Length: %d\n", i);
		enter_isr = 0;
		ctrl->xmit(ctrl, (uint32_t)((uintptr_t)(buff + offset)), i,
			   FTGMAC030_TXDES1_DEFAULT);
		tx_wait_complete();
	}
}

static void send_without_crc(FTGMAC030_Control *ctrl, uint8_t *buff, int len)
{
	prints("->Send %d byte frame.\n", len);
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), len, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();
}

/*
 * burst_size_opt:
 * 2'b00 -> 64 bytes, 2'b01 -> 128 bytes, 2'b10 -> 256 bytes, 2'b11 -> 512 bytes
 */
static void send_diff_burst(FTGMAC030_Control *ctrl, uint8_t *buff)
{
	prints("->Send 300 byte frame.\n");
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), 300, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();

	prints("->Send 600 byte frame.\n");
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), 600, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();

	prints("->Send 900 byte frame.\n");
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), 900, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();

	prints("->Send 1200 byte frame.\n");
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), 1200, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();

	prints("->Send 1500 byte frame.\n");
	enter_isr = 0;
	ctrl->xmit(ctrl, (uint32_t)((uintptr_t)buff), 1500, FTGMAC030_TXDES1_DEFAULT);
	tx_wait_complete();

}

extern int giga_mode;
void tx_test_main(FTGMAC030_Control *ctrl)
{
	//static uint8_t * const buff = (uint8_t *)TX_BUF_ADDR;
	static uint8_t * const buff = (uint8_t *)send_buf;
	uint16_t burst_size;
	int i, speed, duplex;

	/* initial transmission buff */
	memcpy(buff, MAC_DST, 6);
	memcpy(buff + 6, MAC_SRC, 6);
	for (i = 12;i < TX_TEST_PKT_SIZE; i++) {
		buff[i] = i & 0xFF;
	}
#if 0 /* For HW debug */
	prints("Please run corresponding remote program first and press any key to continue...\n");
	ftgmac030_pause();
#endif

#if 1
	speed = (!giga_mode) ? 1 : 0;
	/* _1000 = 0, _100  = 1, _10   = 2 */
	for (; speed < 3; speed++) {

		if ('q' == uart_kbhit())
			break;
#endif
		ctrl->open(ctrl, transmission_isr);
#if 1
		/* FULL = 0, HALF = 1 */
		for (duplex = 0; duplex < 2; duplex++) {

			/* PhY usually not support 1000-Half */
			if (speed == 0 && duplex == 1)
				continue;

			prints("--- Transmission in %d/%s duplex mode.---\n",
				(speed == 2) ? 10: (speed == 1) ? 100 : 1000,
				duplex ? "Half" : "Full");

			if (ctrl->change_duplex_speed(ctrl, duplex, speed))
				continue;
#endif
#if 0
			uint16_t val;

			ctrl->phywrite(ctrl, 0xcf0, 20);
			ctrl->phywrite(ctrl, 0x9140, 0);

			val = ctrl->phyread(ctrl, 20);
			prints("0x%x\n",val);
#endif
			prints("---Test 1. Normal frame transmission. ------------------------------------------\n");
			send_frame(ctrl, buff, 60);
			send_frame(ctrl, buff, 61);
			send_frame(ctrl, buff, 62);
			send_frame(ctrl, buff, 63);
			send_frame(ctrl, buff, 100);
			send_frame(ctrl, buff, 500);
			send_frame(ctrl, buff, 1000);
			send_frame(ctrl, buff, 1500);
			send_frame(ctrl, buff, 1514);

			prints("---Test 2. Frame transmission with N descriptors (buffer size is 100 bytes). ---\n");
			send_multideses(ctrl, buff, 100);
			send_multideses(ctrl, buff, 200);
			send_multideses(ctrl, buff, 300);
			send_multideses(ctrl, buff, 400);
			send_multideses(ctrl, buff, 500);
			send_multideses(ctrl, buff, 600);
			send_multideses(ctrl, buff, 700);
			send_multideses(ctrl, buff, 800);
			send_multideses(ctrl, buff, 900);
			send_multideses(ctrl, buff, 1000);

			prints("---Test 3.Frame transmission with different transmit buffer base address. -------\n");
			
			send_diff_base(ctrl, buff, 0);
			send_diff_base(ctrl, buff, 1);
			send_diff_base(ctrl, buff, 2);
			send_diff_base(ctrl, buff, 3);
			
			prints("---Test 4. Transmit frame without CRC appened. ----------------------------------\n");
			prints("In the following test, CRC append ability will be disable.\n");
			prints("The remote will not receive any packets.\n");
			mac_regs->maccr &= ~FTGMAC030_MACCR_CRC_APD;

			send_without_crc(ctrl, buff, 60);
			send_without_crc(ctrl, buff, 250);
			send_without_crc(ctrl, buff, 1514);

			prints("CRC append ability will be enable now.\n");
			prints("Please check the remote receive packets.");
			mac_regs->maccr |= FTGMAC030_MACCR_CRC_APD;
			send_frame(ctrl, buff, 60);
			send_frame(ctrl, buff, 61);
			send_frame(ctrl, buff, 62);
			send_frame(ctrl, buff, 63);

			prints("---Test 5. Frame transmission with different burst lengths. ---------------------\n");
			prints("TX FIFO size: 0x%x\n", ((mac_regs->tfafcr >> 27) & 0x7));
			for (burst_size=0; burst_size<4; burst_size++) {

				prints("TXDMA maximum burst size: 0x%x (%d)\n",  (burst_size & 0x3),
						64 * (1 << burst_size));
				mac_regs->dblac &= ~FTGMAC030_DBLAC_TXBST_SIZE(3);
				mac_regs->dblac |= FTGMAC030_DBLAC_TXBST_SIZE(burst_size);

				send_diff_burst(ctrl, buff);
			}
#if 1
		}
	}
#endif
	prints("---Finish Transmission test.-------------------------------------------------\n");
}
