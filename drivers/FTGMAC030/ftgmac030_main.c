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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>


#include "net/net_core.h"
#include "ftgmac030.h"
#ifdef PLATFORM_A369
	#include "SoFlexible.h"
	extern void FA626_CPUCleanInvalidateDCacheAll(void);
	extern void CPU_AllCache_Disable(void);
#endif

int giga_mode;

/* 
 * a global variable for ftgmac030, 
 * initialized when user call malloc_ftgmac030_dev function
 */
static FTGMAC030_Control *ctrl = NULL;

int ftgmac030_send(struct eth_device *eth, void *packet, int length)
{
	uint64_t t0, t1;

	ctrl->xmit(ctrl, (const uint32_t)((uintptr_t)packet), length, FTGMAC030_TXDES1_DEFAULT);

	t0 = ftgmac030_get_time();
	do {
		if (mac_regs->isr & FTGMAC030_INT_TPKT2E) {
			mac_regs->isr = (FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);
			break;
		}

		/* wait at least 5 CONFIG_SYS_HZ */
		t1 = ftgmac030_get_time();
		if ((t1-t0) > (5ULL * COUNTS_PER_SECOND)) {
			prints("tx timeout\n");
			break;
		}
	} while (1);

	return 0;
}

int ftgmac030_recv(struct eth_device *eth)
{
	FTGMAC030_RXDESC *rxdes;
	uint32_t len;
	uint8_t *buf;

	len = 0;
	rxdes = ctrl->rxdes_cur;
	do {

		/* No packet received */
		if (!(rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY))
			break;

		if (rxdes->rxdes0 & FTGMAC030_RXDES0_FRS) {
			if (!(rxdes->rxdes0 & FTGMAC030_RXDES0_LRS))
				prints(" -> rx not LRS\n");

			len = FTGMAC030_RXDES0_VDBC(rxdes->rxdes0) - 4;
			buf = (uint8_t *) ((uintptr_t)rxdes->rxdes3);

			if (rxdes->rxdes0 & (0x1f << 18))
				prints(" -> rx error\n");
			else
				NetReceive(buf, len);
		} else {
			prints(" -> rx not FRS\n");
		} 

		rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
		rxdes->rxdes1 = 0x0;
		ctrl->rxdes_cur = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
	} while (0);

	return len;
}

/**
 * Override function in net/eth.c
 */
int board_eth_init(void)
{
	struct eth_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		prints("ftgmac030: %s out of memory 1\n", __func__);
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	sprintf(dev->name, "FTGMAC030");
	dev->send = ftgmac030_send;
	dev->recv = ftgmac030_recv;
	dev->iobase = FTGMAC030_REG_BASE;

	if (!ctrl) {
		ctrl = malloc_ftgmac030_control();
		if (ctrl == NULL) {
			prints("ftgmac030: %s out of memory 2\n", __func__);
			return -1;
		}
	}
	dev->priv = ctrl;

	ctrl->open(ctrl, NULL);

	/* disable receive and transmit interrupts for pollings */
	mac_regs->ier &= ~(FTGMAC030_INT_RPKT2B | FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RXBUF_UNAVA |
			   FTGMAC030_INT_RPKT_LOST | FTGMAC030_INT_TPKT2E | FTGMAC030_INT_TPKT2F);

	memcpy(dev->enetaddr, ctrl->get_mac_addr(), 6);

	eth_register(dev);

	return 0;
}

int select_tx_queue(void)
{
	int tx_queue = NP_TX_QUEUE;
#if 0
	/* tx queue selection */
	prints("Please select which tx queue to send:\n");
	prints("(0->normal, 1->high)");
	scans("%d", &tx_queue);
	prints("\n");
	if ((tx_queue > HP_TX_QUEUE) || (tx_queue < NP_TX_QUEUE)) {
		prints("error selection\n");
		return -1;
	}
#endif
	return tx_queue;
}

void ftgmac030_test_menu(void)
{
	prints("=== FTGMAC030 Test for %s Interface.==========================\n",
		(mac_regs->gisr == 2) ? "RGMII" : (mac_regs->gisr == 1) ? "RMII" : "MII or GMII" );
	prints(" 1. tx test.                  2. rx test.\n");
	prints(" 3. loopback test.            4. auto polling timer test.\n");
	prints(" 5. offload test.             6. ptp test.\n");
	prints(" 7. ipv6 test.                8. interrupt control test.\n");
	prints(" 9. address filtering test   10. multicast hash table test.\n");
	prints("11. Wake-On-Lan test.        12. flow control test.\n");
	prints("13. stress test              14. rxdes1 test.\n");
	prints("--- Other Programs. --------------------------------------------------\n");
	prints("15. command line interface. \n");
	prints("======================================================================\n");
	prints("Command>");
}

extern void tx_test_main(FTGMAC030_Control *ctrl);
extern void rx_test_main(FTGMAC030_Control *ctrl);
extern void loopback_test_main(FTGMAC030_Control *ctrl);
extern void auto_polling_timer_test_main(FTGMAC030_Control *ctrl);
extern void offload_test_main(FTGMAC030_Control *ctrl);
extern void ptp_test_main(FTGMAC030_Control *ctrl);
extern void ipv6_test_main(FTGMAC030_Control *ctrl);
extern void intr_timer_test_main(FTGMAC030_Control *ctrl);
extern void filtering_test_main(FTGMAC030_Control *ctrl);
extern void multicast_test_main(FTGMAC030_Control *ctrl);
extern void wakeup_test_main(FTGMAC030_Control *ctrl);
extern void flowcontrol_test_main(FTGMAC030_Control *ctrl);
extern void rx_stress_test_main(FTGMAC030_Control *ctrl, int speed);
extern void rxdes1_test_main(FTGMAC030_Control *ctrl);
extern void cli_test_main(FTGMAC030_Control *ctrl);


#define BUFSIZE 1024
int FTGMAC030_main(int argc, char * const argv[])
{
	int hw_version = 0x0;
	int option;
	int tx_queue = 0;
	/* for external ahb setting */
	#ifdef PLATFORM_A369
	prints("FTGMAC030_BASE = 0x%08x\n", FTGMAC030_BASE);
	CPU_AllCache_Disable();
	#ifdef EXTERNAL_DEVICE
		*((volatile int *)(0x92000028)) = 0x40000000;
		*((volatile int *)(0x92000200)) = 0x00001878; //0x1878;
		*((volatile int *)(0x92000228)) = (*((volatile int *)(0x92000228)) & ~(0xF0)) | 0x20;
		fLib_Int_Init();

		*((volatile int *)(0x94700000 + 0x420)) = *((volatile int *)(0x94700000 + 0x420)) | 0xF;
		*((volatile int *)(0x94700000 + 0x428)) = *((volatile int *)(0x94700000 + 0x428)) & (~0xF);
		*((volatile int *)(0x94700000 + 0x42C)) = *((volatile int *)(0x94700000 + 0x42C)) | 0xF; 

		#define SCLK_EN 0x230
		*((volatile int *)(SCU_FTSCU010_PA_BASE  + SCLK_EN)) = 0xE1;
	#endif
	prints("Platform: A369\n");
	#endif
	prints("sw version: build_%s, build time: %s\n", __DATE__, __TIME__);

	ctrl = malloc_ftgmac030_control();
	if (!ctrl)
		return 0;

	ftgmac030_init_platform((void *) ftgmac030_isr, (void *) ctrl);

	ftgmac030_enable_interrupts();

	hw_version = mac_regs->revr;
	prints("Hw version: version_%d_%d_r%d\n", FTGMAC030_REV_B1(hw_version),
						  FTGMAC030_REV_B2(hw_version),
						  FTGMAC030_REV_B3(hw_version));

	giga_mode = 1;
#if 0
	{
		FTGMAC030_Device *mac_dev = malloc_ftgmac030_device();
		prints("PHY addr is %d\n", mac_dev->scan_phy_addr());
		free_ftgmac030_device(mac_dev);
	}
#endif

	while (1) {
		ftgmac030_test_menu();

		scans("%d", &option);
		prints("\n");
#if 0
		if (option < 8) {
			/* Include giga speed */
			prints("Include Giga speed mode?\n");
			prints("(0->no, 1->yes)");
			scans("%d", &giga_mode);
			prints("\n");

		}
#endif
		switch(option){
			case 1:
				tx_queue = select_tx_queue();

				if ((tx_queue < NP_TX_QUEUE) ||
				    (tx_queue > HP_TX_QUEUE))
					ctrl->tx_queue_num = NP_TX_QUEUE;
				else
					ctrl->tx_queue_num = tx_queue;

				tx_test_main(ctrl);
				break;
			case 2:
				rx_test_main(ctrl);
				break;
			case 3:
				loopback_test_main(ctrl);
				break;
			case 4:
				auto_polling_timer_test_main(ctrl);
				break;
			case 5:
				offload_test_main(ctrl);
				break;
			case 6:
				ptp_test_main(ctrl);
				break;
			case 7:
				ipv6_test_main(ctrl);
				break;
			case 8:
				intr_timer_test_main(ctrl);
				break;
			case 9:
				filtering_test_main(ctrl);
				break;
			case 10:
				multicast_test_main(ctrl);
				break;
			case 11:
				wakeup_test_main(ctrl);
				break;
			case 12:
				flowcontrol_test_main(ctrl);
				break;
			case 13:
				prints("Choose speed(0=1000, 1=100, 2=10):");
				scans("%d", &option);
				prints("\n");
				rx_stress_test_main(ctrl, option);
				break;
			case 14:
				rxdes1_test_main(ctrl);
				break;
			case 15:
				cli_test_main(ctrl);
				break;
			case 16:
				goto out;
			default:
				break;
		}
	}
out:
	free_ftgmac030_control(ctrl);

	return 0;
}
