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

static uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};

static volatile uint32_t enter_isr = 0;

static int wait_packet(FTGMAC030_Control *ctrl, uint32_t len)
{
	int i;
	uint8_t *buff;
	uint64_t t0, t1;

	prints("Wait for packet %lu bytes: ", len);

	t0 = ftgmac030_get_time();
	while (!enter_isr) {
		/* wait at least 10 seconds */
		t1 = ftgmac030_get_time();
		if ((t1 - t0) > (10ULL * COUNTS_PER_SECOND))
			break;
	}

	if (!enter_isr) {
		prints("Wait packet timeout!\n");
		return 1;
	}

	enter_isr = 0;

	prints("Receive Length %lu\n", (ctrl->rx_len - 4));

	if(len != (ctrl->rx_len - 4)){
		prints(" -> Failed!\n");
		prints("rxdes0 = 0x%08lx, rxdes1 = 0x%08lx\n",
			ctrl->rxdes0, ctrl->rxdes1);
		return 1;
	}
	else{
		buff = ctrl->rx_buff;
		for (i = 12; i < len; i++) {
			if (buff[i] != (uint8_t) i) {
				prints(" --> [Failed]:\n");
				prints("      because of data error at offset %d,"
				       " error data: 0x%02x, correct data: 0x%02x\n",
				       i, buff[i], (i & 0xFF));
				return 1;
			}
		} 

		prints(" -> Passed!\n");
	}

	return 0;
}

static int wait_ftl_packet(FTGMAC030_Control *ctrl, uint32_t len)
{
	uint64_t t0, t1;

	prints("\rWait for FTL packet %lu bytes: ", len);

	t0 = ftgmac030_get_time();
	while (!enter_isr) {
		/* wait at least 10 CONFIG_SYS_HZ */
		t1 = ftgmac030_get_time();
		if ((t1 - t0) > (10ULL * COUNTS_PER_SECOND))
			break;
	}

	if (!enter_isr) {
		prints("Wait packet timeout!\n");
		return 1;
	}

	enter_isr = 0;

	if (ctrl->rx_err_happen && (ctrl->rxdes0 & FTGMAC030_RXDES0_FTL)) {
		prints(" -> Passed!\n");
	} else {
		prints(" -> Failed! (%lu, 0x%lx)\n", ctrl->rx_err_happen, ctrl->rxdes0);
		return 1;
	}

	return 0;
}

static void reception_isr(FTGMAC030_Control *ctrl)
{
	if(0 == ftgmac030_addrcmp(MAC_SRC, ctrl->rx_buff) || ctrl->rx_err_happen){
		enter_isr = 1;
	}
}

extern int giga_mode;
void rx_test_main(FTGMAC030_Control *ctrl)
{
	int speed, duplex, burst_size;
	int ret = 0;

	prints("---Enter Reception test.--------------------------------------------\n");
	prints("---Please run remote reception test.--------------------------------\n");
	/* _1000 = 0, _100  = 1, _10   = 2 */
	speed = 0;//(!giga_mode) ? 1 : 0;
	for (;speed < 3; speed++) {

		if ('q' == uart_kbhit() || ret)
			break;
		ctrl->open_maccr(ctrl, FTGMAC030_MACCR_DEFAULT, reception_isr);

		/* FULL = 0, HALF = 1 */
		for (duplex = 0; duplex < 2; duplex++) {

			if (ret)
				break;

			/* PhY usually not support 1000-Half */
			if (speed == 0 && duplex == 1)
				continue;

			prints("--- Reception in %d/%s duplex mode.---\n",
				(speed == 2) ? 10: (speed == 1) ? 100 : 1000,
				duplex ? "Half" : "Full");

			if (ctrl->change_duplex_speed(ctrl, duplex, speed))
				continue;

			prints("---1. Normal frame Reception. -----------------------\n");
			ret = wait_packet(ctrl, 60);
			ret = wait_packet(ctrl, 61);
			ret = wait_packet(ctrl, 62);
			ret = wait_packet(ctrl, 63);
			ret = wait_packet(ctrl, 64);
			ret = wait_packet(ctrl, 100);
			ret = wait_packet(ctrl, 500);
			ret = wait_packet(ctrl, 1000);
			ret = wait_packet(ctrl, 1500);
			ret = wait_packet(ctrl, 1514);

			prints("---2. Frame reception with N descriptors (buffer size is "
			       "100 bytes).---\n");
			/* set rx buffer size to 100 bytes */
			mac_regs->rbsr = 100;

			ret = wait_packet(ctrl, 100);
			ret = wait_packet(ctrl, 200);
			ret = wait_packet(ctrl, 300);
			ret = wait_packet(ctrl, 400);
			ret = wait_packet(ctrl, 500);
			ret = wait_packet(ctrl, 600);
			ret = wait_packet(ctrl, 700);
			ret = wait_packet(ctrl, 800);
			ret = wait_packet(ctrl, 900);
			ret = wait_packet(ctrl, 1000);

			mac_regs->rbsr = RX_BUF_SIZE;

			prints("---3.Test Jumbo frame (bit 13 in 0x50h = 0x%08x)------\n",
				mac_regs->maccr);
			mac_regs->maccr &= ~FTGMAC030_MACCR_JUMBO_LF;
			ret = wait_ftl_packet(ctrl, 1600);
			ret = wait_ftl_packet(ctrl, 1700);
			ret = wait_ftl_packet(ctrl, 1800);
			ret = wait_ftl_packet(ctrl, 1900);
			mac_regs->maccr |= FTGMAC030_MACCR_JUMBO_LF;
			prints("enable JUMBO_LF (bit 13 in MACCR, 0x50h = 0x%08x) ----\n",
				mac_regs->maccr);
			ret = wait_packet(ctrl, 1600);
			ret = wait_packet(ctrl, 1700);
			ret = wait_packet(ctrl, 1800);
			ret = wait_packet(ctrl, 1900);

			prints("---Test 4. Frame rx with different burst lengths. ---\n");
			for (burst_size=0; burst_size<4; burst_size++) {
				prints("RXDMA max burst size: %d bytes\n", burst_size);
				mac_regs->dblac &= ~FTGMAC030_DBLAC_RXBST_SIZE(3);
				mac_regs->dblac |= FTGMAC030_DBLAC_RXBST_SIZE(burst_size);

				ret = wait_packet(ctrl, 300);
				ret = wait_packet(ctrl, 600);
				ret = wait_packet(ctrl, 900);
			}
		}
	}
	prints("---Finish Transmission test.\n---------------------------------------\n");
}
