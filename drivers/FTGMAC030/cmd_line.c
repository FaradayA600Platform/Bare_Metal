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

#include <stdlib.h>

#include "ftgmac030.h"

#define BUFFER_SIZE 1024
static const uint8_t MAC_SRC[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
static const uint8_t MAC_DST[6] = {0x02,0x02,0x02,0x02,0x02,0x02};
static int enter_isr = 0;
static int is_display = 0;
static int ptp_test = 0;


static int to_argv(char *command, char **argv)
{
	uint32_t argc = 0;
	argv[argc] = strtok(command, " ");
	argc++;
	while((argv[argc] = strtok(NULL, " ")) != NULL){
		argc++;
	}
	return argc;
}
#if 0
static uint16_t dec2int(char *hex)
{
	uint32_t index = 0;
	uint16_t tmp = 0;
	uint16_t ret = 0;
	while(hex[index]){
		/* 0~9 */
		if(hex[index] >= '0' && hex[index] <= '9'){
			tmp = hex[index] - '0';
		}
		else{
			return -1;
		}
		index++;
		ret = (ret * 10) + tmp;
	}
	return ret;
}

static uint16_t hex2int(char *hex)
{
	uint32_t index = 0;
	uint16_t tmp = 0;
	uint16_t ret = 0;
	while(hex[index]){
		/* 0~9 */
		if(hex[index] >= '0' && hex[index] <= '9'){
			tmp = hex[index] - '0';
		}
		else if(hex[index] >= 'a' && hex[index] <= 'f'){/* a~f */
			tmp = hex[index] - 'a' + 10;
		}
		else if(hex[index] >= 'A' && hex[index] <= 'F'){/* A~F*/
			tmp = hex[index] - 'A' + 10;
		}
		else{
			return -1;
		}
		index++;
		ret = (ret << 4) + tmp;
	}
	return ret;
}
#endif

static void cli_isr(FTGMAC030_Control *ctrl)
{
	int i = 0;

	if (is_display) {
		prints("rx a packet, len: %lu\n", ctrl->rx_len);
		/*prints("rx packet crc = 0x%08x\n", ftgmac030_crc32(ctrl->rx_buff, ctrl->rx_len-4));*/
		prints("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");
		prints("-----------------------------------------------\n");
		for (i = 0;i < ctrl->rx_len; i++) {
			prints("%02x ", ctrl->rx_buff[i]);
			if (((i + 1) % 16) == 0) {
				prints("\r\n");
			}
		}

		if (((i + 1) % 16) != 0) {
			prints("\r\n");
		}

		prints("receive %lu pkts\n", ctrl->rx_packet_cnt);
	} else if (ptp_test) {
		//Disable PTP to lock the timer value.
		ctrl->maccr &= ~FTGMAC030_MACCR_PTP_EN;

		prints(" PTP time is %u.%09u (ISR)\n",
			mac_regs->ptp_sec_tmr,  mac_regs->ptp_nsec_tmr);
	}

	/* check packet's correctness */
/*	prints("receive packet length = %d\n", ctrl->rx_len);
	for (i = 12; i < (ctrl->rx_len - 4); i++) {
		if (ctrl->rx_buff[i] != (i & 0xff)) {
			prints(" --> [Failed]:\n");
			prints(" error at offset %d, error data: 0x%02x, correct data: "
			       "0x%02x\n", i, ctrl->rx_buff[i], i&0xFF);
		}
	}

	prints("packet's crc received   = 0x%08x\n",
		ctrl->rx_buff[i] | 
		(ctrl->rx_buff[i+1] << 8)| 
		(ctrl->rx_buff[i+2] << 16)|
		(ctrl->rx_buff[i+3] << 24));
	prints("packet's crc calculated = 0x%08x\n",
		ftgmac030_crc32(ctrl->rx_buff, ctrl->rx_len-4));
*/
	enter_isr = 1;
}

/* IEEE PHY Specific definitions */
#define PHY_R0_CTRL_REG		0
#define PHY_R3_PHY_IDENT_REG	3

#define PHY_R0_RESET         0x8000
#define PHY_R0_LOOPBACK      0x4000
#define PHY_R0_ANEG_ENABLE   0x1000
#define PHY_R0_DFT_SPD_MASK  0x2040
#define PHY_R0_DFT_SPD_10    0x0000
#define PHY_R0_DFT_SPD_100   0x2000
#define PHY_R0_DFT_SPD_1000  0x0040
#define PHY_R0_DFT_SPD_2500  0x0040
#define PHY_R0_ISOLATE       0x0400

/* Marvel PHY 88E1111 Specific definitions */
#define PHY_R20_EXTND_CTRL_REG	20
#define PHY_R27_EXTND_STS_REG	27

#define PHY_R20_DFT_SPD_10    	0x20
#define PHY_R20_DFT_SPD_100   	0x50
#define PHY_R20_DFT_SPD_1000  	0x60
#define PHY_R20_RX_DLY		0x80

#define PHY_R27_MAC_CONFIG_GMII      0x000F
#define PHY_R27_MAC_CONFIG_MII       0x000F
#define PHY_R27_MAC_CONFIG_RGMII     0x000B
#define PHY_R27_MAC_CONFIG_SGMII     0x0004

/* Marvel PHY 88E1116R Specific definitions */
#define PHY_R22_PAGE_ADDR_REG	22
#define PHY_PG2_R21_CTRL_REG	21

#define PHY_REG21_10      0x0030
#define PHY_REG21_100     0x2030
#define PHY_REG21_1000    0x0070

/* Marvel PHY flags */
#define MARVEL_PHY_88E1111_MODEL	0xC0
#define MARVEL_PHY_88E1116R_MODEL	0x240
#define PHY_MODEL_NUM_MASK		0x3F0

#define XAE_PHY_TYPE_MII		0
#define XAE_PHY_TYPE_GMII		1
#define XAE_PHY_TYPE_RGMII_1_3		2
#define XAE_PHY_TYPE_RGMII_2_0		3

extern void ftgmac030_dump_reg(FTGMAC030_Control *ctrl);
void cli_test_main(FTGMAC030_Control *ctrl)
{
	char command[BUFFER_SIZE] = {0};
	char *command_vector[100] = {0};
	int command_cnt = 0, reg, val;
	char *buff = (char *)TX_BUF_ADDR;
	uint32_t maccr = 0x0;
	ctrl->open(ctrl, cli_isr);
	/* initial phy steps from linux driver */
	/* Enable Fiber/Copper auto selection */
	/*tmp16 = phyread16(MII_M1111_PHY_EXT_SR);
	tmp16 &= ~MII_M1111_HWCFG_FIBER_COPPER_AUTO;
	phywrite16(tmp16, MII_M1111_PHY_EXT_SR);

	tmp16 = phyread16(FTPHY_REG_BMCR);
	tmp16 |= FTPHY_BMCR_RESET;
	phywrite16(tmp16, FTPHY_REG_BMCR);

	phywrite16(FTPHY_BMCR_RESET, FTPHY_REG_BMCR);
	phywrite16(0x1f, 0x1d);
	phywrite16(0x200c, 0x1e);
	phywrite16(0x5, 0x1d);
	phywrite16(0x0, 0x1e);
	phywrite16(0x100, 0x1e);
	phywrite16(MII_M1011_PHY_SCR_AUTO_CROSS, MII_M1011_PHY_SCR);
	phywrite16(MII_M1111_PHY_LED_DIRECT, MII_M1111_PHY_LED_CONTROL);*/
	/* implementation of phy shell */

	/* initial phy's interrupt */

	mac_regs->maccr = mac_regs->maccr & ~(FTGMAC030_MACCR_RXDMA | FTGMAC030_MACCR_RXMAC |
					      FTGMAC030_MACCR_ALLADDR);
	maccr = 0x0;

	prints("\r");
	while(1) {
		memset(command, 0, BUFFER_SIZE);
out:
		prints("\nPHY ctrl>>");
		scans(command);
		command_cnt = to_argv(command, command_vector);
		if (0 == strcmp(command_vector[0], "exit")) {
			break;
		}
		else if (0 == strcmp(command_vector[0], "phyr")) {
			if (command_cnt != 2) {
				prints("phyr offset(decimal)\n");
				continue;
			}
			prints("offset %s: 0x%04x\n", command_vector[1],
			       ctrl->phyread(ctrl, strtoul(command_vector[1], NULL, 10)));
		}
		else if (0 == strcmp(command_vector[0], "phyw")) {
			if (command_cnt != 3) {
				prints("phyw offset(decimal) value(hex)\n");
				continue;
			}
			ctrl->phywrite(ctrl, strtoul(command_vector[2], NULL, 16),
				       strtoul(command_vector[1], 0, 10));
		}
		else if (0 == strcmp(command_vector[0], "mmdr")) {
			if (command_cnt != 3) {
				prints("mmdr <ctrl_addr (decimal)> <reg_addr (decimal)>\n");
				continue;
			}
			prints("mmd %s.%s: 0x%04x\n", command_vector[1], command_vector[2],
			       ctrl->mmdread(ctrl, strtoul(command_vector[1], NULL, 10),
			       strtoul(command_vector[2], NULL, 10)));
		}
		else if (0 == strcmp(command_vector[0], "mmdw")) {
			if (command_cnt != 4) {
				prints("mmdw <ctrl_addr (decimal)> <reg_addr (decimal)>"
				       " <value (hex)>\n");
				continue;
			}
			ctrl->mmdwrite(ctrl, strtoul(command_vector[3], NULL, 16), strtoul(command_vector[1], NULL, 10),
					strtoul(command_vector[2], NULL, 10));
		}
		else if (0 == strcmp(command_vector[0], "ptp")) {
			char *tmp;

			if (command_cnt != 2) {
				prints("cl | clc | rd | rdc\n");
				continue;
			}

			tmp = command_vector[1];
			if (0 == strcmp(command_vector[1], "on")) {
				prints(" PTP time is enabled\n");
				mac_regs->maccr |= FTGMAC030_MACCR_PTP_EN;
				ptp_test = 1;
				mac_regs->ptp_sec_tmr = 0;
				mac_regs->ptp_nsec_tmr = 0;
				mac_regs->ier |= FTGMAC030_INT_TSU_SEC_INC;
			}
			else if (0 == strcmp(command_vector[1], "off")) {
				prints(" PTP time is disabled\n");
				ptp_test = 0;
				mac_regs->ptp_sec_tmr = 0;
				mac_regs->ptp_nsec_tmr = 0;
				mac_regs->maccr &= ~FTGMAC030_MACCR_PTP_EN;
				prints(" PTP time is %u.%09u\n",
					mac_regs->ptp_sec_tmr,  mac_regs->ptp_nsec_tmr);
			} else if (tmp[0] == 'c' && tmp[1] == 'l') {
				mac_regs->ptp_nsec_tmr = 0;
				mac_regs->ptp_sec_tmr = 0;

				if (tmp[2] == 'c')
					goto rd_ptp;
			} else if (0 == strcmp(tmp, "rd")) {
				prints("s: %u, ns: %09u.\n", mac_regs->ptp_sec_tmr,
				       mac_regs->ptp_nsec_tmr);
			} else if (0 == strcmp(tmp, "rdc")) {
rd_ptp:
				while (1) {
					if ('q' == uart_kbhit())
						break;
					prints("s: %u, ns: %09u.\r", mac_regs->ptp_sec_tmr,
					       mac_regs->ptp_nsec_tmr);
				}
			}
		}
		else if (0 == strcmp(command_vector[0], "macr")) {
			if (command_cnt != 2) {
				prints("macr offset(hex)\n");
				continue;
			}
			reg = (uintptr_t) mac_regs + strtoul(command_vector[1], 0, 16);
			prints("offset 0x%x: 0x%08lx\n", reg, ctrl->macread(reg));
		}
		else if (0 == strcmp(command_vector[0], "macw")) {
			if (command_cnt != 3) {
				prints("macw offset(hex) value(hex)\n");
				continue;
			}
			reg = (uintptr_t) mac_regs + strtoul(command_vector[1], 0, 16);
			val = strtoul(command_vector[2], 0, 16);
			ctrl->macwrite(val, reg);
			prints("offset 0x%x: 0x%08lx\n", reg, ctrl->macread(reg));
		}
		else if (0 == strcmp(command_vector[0], "phyaddr")) {
			prints("phy address is 0x%04x\n", ctrl->phyad);
		}
		else if (0 == strcmp(command_vector[0], "send_p")) {
			if (command_cnt != 1) {
				prints("send_p\n");
				continue;
			}
			ctrl->set_flow_control(0xdd, 0x0a, 0x04, 1, 1);
			ctrl->xmit_pause_frame();
		}
		else if (0 == strcmp(command_vector[0], "loopback")) {
			maccr = 0x0;
			maccr = mac_regs->maccr;
			if (command_cnt != 2) {
				prints("loopback on/off\n");
				continue;
			}
			if (0 == strcmp(command_vector[1], "on")) {
				maccr |= FTGMAC030_MACCR_LOOPBACK;
				mac_regs->maccr = maccr;
			}
			else if (0 == strcmp(command_vector[1], "off")) {
				maccr &= ~FTGMAC030_MACCR_LOOPBACK;
				mac_regs->maccr = maccr;
			}
			else {
				prints("loopback on/off\n");
				continue;
			}
		}
		else if (0 == strcmp(command_vector[0], "bcst")) {
			FTGMAC030_RXDESC *rxdes;

			mac_regs->ier &= ~(FTGMAC030_INT_RPKT2F | FTGMAC030_INT_RPKT2B |
					   FTGMAC030_INT_RXBUF_UNAVA);

			maccr = mac_regs->maccr;
			maccr &= ~(FTGMAC030_MACCR_ALLADDR | FTGMAC030_MACCR_MULTIPKT |
				   FTGMAC030_MACCR_HT_EN);
			maccr |= (FTGMAC030_MACCR_RXDMA | FTGMAC030_MACCR_RXMAC |
				  FTGMAC030_MACCR_BROADPKT);
			mac_regs->maccr = maccr;

			prints("rx broadcast cnt %d\n", mac_regs->bropkt_cnt);
			rxdes = ctrl->rxdes_cur;
			while (1) {
				if ('q' == uart_kbhit())
					break;

				if ('p' == uart_kbhit())
					prints("rx broadcast cnt %d\n", mac_regs->bropkt_cnt);

				if (rxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) {
					rxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
					rxdes = (FTGMAC030_RXDESC *)((uintptr_t)rxdes->rxdes2);
				}
			};
			prints("rx broadcast cnt %d\n", mac_regs->bropkt_cnt);

		} else if (0 == strcmp(command_vector[0], "rx")) {
			int rcv_all, rcv_num = 0;
			char ch;

			maccr = mac_regs->maccr;
			if (command_cnt != 2) {
				prints("rx <num>\n");
				continue;
			}
			maccr &= ~(FTGMAC030_MACCR_MULTIPKT | FTGMAC030_MACCR_HT_EN);
			maccr |= (FTGMAC030_MACCR_RXDMA | FTGMAC030_MACCR_RXMAC);
#if 1
			prints("Receive All ? (0->No, 1->Yes)");
			scanf("%d", &rcv_all);
			prints("\n");

			if (rcv_all)
				maccr |= FTGMAC030_MACCR_ALLADDR;
			else
				maccr &= ~FTGMAC030_MACCR_ALLADDR;
#endif
			mac_regs->maccr = maccr;
			rcv_num = strtoul(command_vector[1], NULL, 10);
			while (rcv_num > 0) {

				prints("wait... ramaining %d packets\n", rcv_num);
				/* wait fo packet */
				while (!enter_isr) {
					ch = uart_kbhit();
					if (ch == 'q')
						goto out;

					if (ch == 'p')
						ftgmac030_dump_reg(ctrl);
				}
				enter_isr = 0;
				rcv_num--;
			}
			/*if (0 == strcmp(command_vector[1], "on")) {
				maccr |= (FTGMAC030_MACCR_RXDMA | FTGMAC030_MACCR_RXMAC | FTGMAC030_MACCR_ALLADDR);
				ctrl->macwrite(maccr, ctrl->base + FTGMAC030_REG_MACCR);
			}
			else if (0 == strcmp(command_vector[1], "off")) {
				maccr &= ~(FTGMAC030_MACCR_RXDMA | FTGMAC030_MACCR_RXMAC | FTGMAC030_MACCR_ALLADDR);
				ctrl->macwrite(maccr, ctrl->base + FTGMAC030_REG_MACCR);
			}
			else {
				prints("rx <num>\n");
				continue;
			}*/
		}
		else if (0 == strcmp(command_vector[0], "tx")) {
			int start_len = 0, num = 0;
			int i = 0;
			if (command_cnt != 3) {
				prints("tx <start_len> <num>\n");
				continue;
			}
			memcpy(buff, MAC_DST, 6);
			memcpy(buff + 6, MAC_SRC, 6);
			start_len = strtoul(command_vector[1], NULL, 10);
			num = strtoul(command_vector[2], NULL, 10);
			for (i = 12;i < (start_len + num); i++) {
				buff[i] = (i) & 0xFF;
			}
			
			for (i = 0; i < num; i++) {
				ctrl->xmit(ctrl, (const uint32_t)((uintptr_t)buff),
					   start_len, FTGMAC030_TXDES1_DEFAULT);
				prints("tx %d length %d finished (crc calculated = "
				       "0x%08x)!\n", i, start_len,
				       ftgmac030_crc32(buff, start_len));
				udelay(1000);

				if (start_len < 1510)
					start_len++;
			}
		}
		else if (0 == strcmp(command_vector[0], "10h")) {
			ctrl->half_dup_10(ctrl);
		}
		else if (0 == strcmp(command_vector[0], "10f")) {
			ctrl->full_dup_10(ctrl);
		}
		else if (0 == strcmp(command_vector[0], "100h")) {
			ctrl->half_dup_100(ctrl);
		}
		else if (0 == strcmp(command_vector[0], "100f")) {
			ctrl->full_dup_100(ctrl);
		}
		else if (0 == strcmp(command_vector[0], "1000f")) {
			ctrl->full_dup_1000(ctrl);
		}
		else if (0 == strcmp(command_vector[0], "showlink")) {
			ctrl->show_link_status(ctrl);
		}
		else if (!strcmp(command_vector[0], "phy_loopback")) {
			if (command_cnt != 3) {
				prints("phy_loopback on/off\n");
				continue;
			}
			if (0 == strcmp(command_vector[1], "on")) {
#if 0
{
				uint16_t PhyReg0;
				int Speed, ExternalLoopback = 1;
				uint8_t PhyType = XAE_PHY_TYPE_RGMII_2_0;
				uint16_t PhyModel;
				uint16_t PhyReg20;	/* Extended PHY specific Register (Reg 20)
						   of Marvell 88E1111 PHY */
				uint16_t PhyReg21;	/* Control Register MAC (Reg 21)
						   of Marvell 88E1116R PHY */

				Speed = dec2int(command_vector[2]);

				 mac_regs->maccr &= ~(FTGMAC030_MACCR_SPEED_MASK);

				switch (Speed) {
				case 10:
					PhyReg0 |= PHY_R0_DFT_SPD_10;
					PhyReg20 |= PHY_R20_DFT_SPD_10;
					PhyReg21 |= PHY_REG21_10;
					mac_regs->maccr |= (FTGMAC030_MACCR_FULLDUP | FTGMAC030_MACCR_SPEED_10);
					break;
				case 100:
					PhyReg0 |= PHY_R0_DFT_SPD_100;
					PhyReg20 |= PHY_R20_DFT_SPD_100;
					PhyReg21 |= PHY_REG21_100;
					mac_regs->maccr |= (FTGMAC030_MACCR_FULLDUP | FTGMAC030_MACCR_SPEED_100);
					break;
				case 1000:
					PhyReg0 |= PHY_R0_DFT_SPD_1000;
					PhyReg20 |= PHY_R20_DFT_SPD_1000;
					PhyReg21 |= PHY_REG21_1000;
					mac_regs->maccr |= (FTGMAC030_MACCR_FULLDUP | FTGMAC030_MACCR_SPEED_1000);
					break;
				default:
					prints("Speed error\n");
					continue;
				}

				PhyModel = hex2int(command_vector[3]);
				/* RGMII mode Phy specific registers initialization */
				if ((PhyType == XAE_PHY_TYPE_RGMII_2_0) ||
						(PhyType == XAE_PHY_TYPE_RGMII_1_3)) {
					if (PhyModel == MARVEL_PHY_88E1111_MODEL) {
						PhyReg20 |= PHY_R20_RX_DLY;
						/*
						 * Adding Rx delay. Configuring loopback speed.
						 */
						ctrl->phywrite(ctrl, PhyReg20, PHY_R20_EXTND_CTRL_REG);

					} else if (PhyModel == MARVEL_PHY_88E1116R_MODEL) {
						/*
						 * Switching to PAGE2
						 */
						ctrl->phywrite(ctrl, 2, PHY_R22_PAGE_ADDR_REG);

						/*
						 * Adding Tx and Rx delay. Configuring loopback speed.
						 */
						ctrl->phywrite(ctrl, PhyReg21, PHY_PG2_R21_CTRL_REG);

						/*
						 * Switching to PAGE0
						 */
						ctrl->phywrite(ctrl, 0, PHY_R22_PAGE_ADDR_REG);
			}
			PhyReg0 &= (~PHY_R0_ANEG_ENABLE);

			/* Configure interface modes */
			if (PhyModel == MARVEL_PHY_88E1111_MODEL) {
				if ((PhyType == XAE_PHY_TYPE_RGMII_2_0) ||
						(PhyType == XAE_PHY_TYPE_RGMII_1_3))  {
					ctrl->phywrite(ctrl, PHY_R27_MAC_CONFIG_RGMII, PHY_R27_EXTND_STS_REG);
				} else if ((PhyType == XAE_PHY_TYPE_GMII) ||
						(PhyType == XAE_PHY_TYPE_MII)) {
					ctrl->phywrite(ctrl, PHY_R27_MAC_CONFIG_GMII, PHY_R27_EXTND_STS_REG);
				}
			}

			/* Set the speed and put the PHY in reset, then put the PHY in loopback */
			ctrl->phywrite(ctrl, PhyReg0 | PHY_R0_RESET, PHY_R0_CTRL_REG);

			udelay(4000);

			PhyReg0 = ctrl->phyread(ctrl, PHY_R0_CTRL_REG);

			ExternalLoopback = 1;
			if (!ExternalLoopback) {
				ctrl->phywrite(ctrl, PhyReg0 | PHY_R0_LOOPBACK, PHY_R0_CTRL_REG);
			}
		}
}
#else
				ctrl->phywrite(ctrl, 0x0000, 18);
				ctrl->phywrite(ctrl, 0x1b00, 9);
				ctrl->phywrite(ctrl, 0x9140, 0);
				ctrl->phywrite(ctrl, 0x0007, 29);
				ctrl->phywrite(ctrl, 0x0808, 30);
				ctrl->phywrite(ctrl, 0x0010, 29);
				ctrl->phywrite(ctrl, 0x0042, 30);
				ctrl->phywrite(ctrl, 0x0012, 29);
				ctrl->phywrite(ctrl, 0x8901, 30);
#endif
			}
			else if (0 == strcmp(command_vector[1], "off")) {
				ctrl->phywrite(ctrl, 0x0300, 9);
				ctrl->phywrite(ctrl, 0x9140, 0);
				ctrl->phywrite(ctrl, 0x0007, 29);
				ctrl->phywrite(ctrl, 0x0800, 30);
				ctrl->phywrite(ctrl, 0x0010, 29);
				ctrl->phywrite(ctrl, 0x0040, 30);
				ctrl->phywrite(ctrl, 0x0012, 29);
				ctrl->phywrite(ctrl, 0x8900, 30);
				ctrl->phywrite(ctrl, 0x0400, 18); /* link status changed */
			}
			else {
				prints("phy_loopback on/off\n");
				continue;
			}
		}
		else if (!strcmp(command_vector[0], "pr")) {
			ftgmac030_dump_reg(ctrl);
		}
		else if (!strcmp(command_vector[0], "display")) {
			if (command_cnt != 2) {
				prints("display on/off\n");
				continue;
			}
			if (0 == strcmp(command_vector[1], "on")) {
				is_display = 1;
			}
			else if (0 == strcmp(command_vector[1], "off")) {
				is_display = 0;
			}
			else {
				prints("display on/off\n");
				continue;
			}
		}
		else {
			prints("PHY commands:\n");
			prints("phyr <offset(decimal)>\n");
			prints("phyw <offset(decimal)> <value(hex)>\n");
			prints("phyaddr \n");
			prints("phy_loopback <on/off>\n");
			prints("MAC commands:\n");
			prints("macr <offset(hex)>\n");
			prints("macw <offset(hex)> <value(hex)>\n");
			prints("send_p\n");
			prints("loopback <on/off>\n");
			prints("tx <start_len> <num>\n");
			prints("bcst\n");
			prints("rx <num>\n");
			prints("pr\n");
			/*prints("rx on/off\n");*/
			prints("10h\n");
			prints("10f\n");
			prints("100h\n");
			prints("100f\n");
			prints("1000f\n");
			prints("showlink\n");
			prints("display <on/off>\n");
			prints("exit\n");
		}
	}
}
