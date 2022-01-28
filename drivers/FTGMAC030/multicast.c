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

static uint8_t multicast_addrs[13][6] = { //default patterns
	/*{0x41, 0x12, 0x76, 0x45, 0xba, 0x89},*/
	{0x01, 0x02, 0x03, 0x04, 0x00, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x01, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x02, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x03, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x04, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x06, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x07, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x08, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x09, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x0A, 0x06},
	{0x01, 0x02, 0x03, 0x04, 0x0B, 0x06},
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x08} //terminated frame
};

#define EXCEPTADDR_NUM 2
static uint8_t except_addrs[EXCEPTADDR_NUM][6] = {
	{0x33, 0x33, 0x00, 0x00, 0x00, 0x01},
	{0x01, 0x00, 0x5e, 0x00, 0x00, 0xfc},
};

static volatile uint32_t enter_isr = 0;
static volatile uint32_t terminate_flag = 0;
static volatile uint8_t *g_mac_addr = NULL;

static void wait_multicast_packet(uint8_t *mac_addr)
{
	g_mac_addr = mac_addr;
	prints("Wait for a frame which MAC addr is: ");
	prints("%02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1],
	       mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	while (0 == enter_isr);
	enter_isr = 0;
}

static int check_except_addrs(FTGMAC030_Control *ctrl)
{
	int i;
	for (i = 0; i < EXCEPTADDR_NUM; i ++) {
		if (0 == ftgmac030_addrcmp(ctrl->rx_buff, except_addrs[i])) {
			return 1;
		}
	}
	return 0;
}

static void set_multicast_register(FTGMAC030_Control *ctrl)
{
	/*
	 * Set Multicast Registers.
	 * The first six addresses index are set.
	 */
	int i;
	for (i = 0; i < 6; i++) {
		ctrl->set_multicast_address(ctrl, multicast_addrs[i]);
	}
}

/*
 * There are only two kinds of packet can trigger isr:
 * - destination address matches MAC_ADR (08h ~ 0Fh)
 * - multicast address and exists in hash table
 */
static void multicast_isr(FTGMAC030_Control *ctrl)
{
	if (0 == ftgmac030_addrcmp(ctrl->rx_buff, ctrl->get_mac_addr())) {
		//If entering this state, this means receive a normal packet
		//which terminates this test
		terminate_flag = 1;
	}
	else {
		prints("Receive Length %lu\n", ctrl->rx_len - 4);
		prints("Receive a packet which MAC addr is: ");
		prints("%02x:%02x:%02x:%02x:%02x:%02x\n", 
			ctrl->rx_buff[0], ctrl->rx_buff[1], ctrl->rx_buff[2], 
			ctrl->rx_buff[3], ctrl->rx_buff[4], ctrl->rx_buff[5]);

		///g_mac_addr may not be initialized, this will cause a system crash 
		if (g_mac_addr) {
			if (check_except_addrs(ctrl)) {
				prints("An except address!\n");
			}
			else {
				enter_isr = 1;
				if (0 == ftgmac030_addrcmp(ctrl->rx_buff,
							   (uint8_t *)g_mac_addr)) {
					prints(" -> Passed!\n");
				}
				else {
					//Not a normal packet ether multicast packet,
					//this is an error condition
					prints(" -> Failed!\n");
				}
			}
		}
	}
}

void multicast_test_main(FTGMAC030_Control *ctrl)
{
	prints("Multicast Hash Table Test\n");
	prints("Speed: 100Mbps\n");
	prints("---1. Enable HT_MULTI_EN-----------------------------------------\n");
	ctrl->open(ctrl, multicast_isr);
	mac_regs->maccr |= FTGMAC030_MACCR_HT_EN;
	ctrl->change_duplex_speed(ctrl, FULL, _100);
	set_multicast_register(ctrl);
	wait_multicast_packet(multicast_addrs[0]);
	wait_multicast_packet(multicast_addrs[1]);
	wait_multicast_packet(multicast_addrs[2]);
	wait_multicast_packet(multicast_addrs[3]);
	wait_multicast_packet(multicast_addrs[4]);
	wait_multicast_packet(multicast_addrs[5]);

	prints("---2. Disable HT_MULTI_EN----------------------------------------\n");
	g_mac_addr = NULL;
	mac_regs->maccr &= ~FTGMAC030_MACCR_HT_EN;
	prints("Wait for a terminated frame.\n");

	while (0 == terminate_flag);
	terminate_flag = 0;

	prints("Speed: 1000Mbps\n");
	prints("---3. Enable HT_MULTI_EN-----------------------------------------\n");
	mac_regs->maccr |= FTGMAC030_MACCR_HT_EN;
	ctrl->change_duplex_speed(ctrl, FULL, _1000);
	set_multicast_register(ctrl);
	wait_multicast_packet(multicast_addrs[0]);
	wait_multicast_packet(multicast_addrs[1]);
	wait_multicast_packet(multicast_addrs[2]);
	wait_multicast_packet(multicast_addrs[3]);
	wait_multicast_packet(multicast_addrs[4]);
	wait_multicast_packet(multicast_addrs[5]);

	prints("---4. Disable HT_MULTI_EN----------------------------------------\n");
	g_mac_addr = NULL;
	mac_regs->maccr &= ~FTGMAC030_MACCR_HT_EN;
	prints("Wait for a terminated frame.\n");

	while (0 == terminate_flag);
	terminate_flag = 0;

}
