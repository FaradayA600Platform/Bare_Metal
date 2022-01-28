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

#define RTL8211_REG_PHYCR 0x10
#define RTL8211_REG_PHYSR 0x11
#define RTL8211_REG_INER  0x12
#define RTL8211_REG_INSR  0x13

/*
 * PHY Specific Control Register (0x10)
 */
#define RTL8211_PHYCR_JABBER     (1 << 0)
#define RTL8211_PHYCR_CLK125     (1 << 4)
#define RTL8211_PHYCR_MDI        (1 << 5)
#define RTL8211_PHYCR_CROSSOVER  (1 << 6)
#define RTL8211_PHYCR_FORCE_LINK (1 << 10)
#define RTL8211_PHYCR_ASSERT_CRS (1 << 11)
#define RTL8211_PHYCR_FPR(x)     ((x & 7) << 12)
#define RTL8211_PHYCR_RXC        (1 << 15)

/*
 * PHY Specific Status Register (0x11)
 */
#define RTL8211_PHYSR_JABBER(x)   (x & 1)
#define RTL8211_PHYSR_PRE_LINK(x) ((x >> 1) & 1)
#define RTL8211_PHYSR_MDI(x)      ((x >> 6) & 1)
#define RTL8211_PHYSR_LINK        (1 << 10)
#define RTL8211_PHYSR_DUPLEX(x)   ((x >> 13) & 1)
#define RTL8211_PHYSR_SPEED(x)    ((x >> 14) & 3)

/*
 * Interrupt Enable Register (0x12)
 * Interrupt Status Register (0x13)
 */
#define RTL8211_INSR_JABBER          (1 << 0)
#define RTL8211_INSR_FALSECARRIER    (1 << 8)
#define RTL8211_INSR_SYMBOL_ERR      (1 << 9)
#define RTL8211_INSR_LINK_CHG        (1 << 10)
#define RTL8211_INSR_AUTO_NEO        (1 << 11)
#define RTL8211_INSR_PAGE_RX         (1 << 12)
#define RTL8211_INSR_AUTO_NEO_ERR    (1 << 15)

uint32_t rtl8211_phy_processing(FTGMAC030_Control *ctrl)
{
	uint16_t value = 0x0;
	uint32_t maccr = 0x0;

	/* read status of phy */
	value = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);

	if (!(value & RTL8211_PHYSR_LINK)) {
		prints("??? Link status => FAIL\n");
		return 1;
	}

	value = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);
	maccr = mac_regs->maccr;

	if (0 == RTL8211_PHYSR_DUPLEX(value)) {
		/* half duplex */
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
	} else {
		/* full duplex */
		maccr |= FTGMAC030_MACCR_FULLDUP;
	}

	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if (0x2 == RTL8211_PHYSR_SPEED(value)) {
		maccr |= FTGMAC030_MACCR_SPEED_1000;
	} else {
		if (0x1 == RTL8211_PHYSR_SPEED(value)) {
			maccr |= FTGMAC030_MACCR_SPEED_100;
		} else if (0x0 == RTL8211_PHYSR_SPEED(value)) {
			maccr |= FTGMAC030_MACCR_SPEED_10;
		}
	}

	mac_regs->maccr = maccr;
	return 0;
}

void rtl8211_phy_auto_neo(FTGMAC030_Control *ctrl)
{
	/* kick off phy auto negotiation */
	uint16_t tmp16;
	int i;
	tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	tmp16 |= (MII_BMCR_RESTART_AUTO_NEG | MII_BMCR_AUTO_NEG_EN);
	ctrl->phywrite(ctrl, tmp16, MII_REG_BMCR);

	do {
		for (i = 0; i < 100; i++){};
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	} while ((tmp16 & MII_BMCR_RESTART_AUTO_NEG) > 0) ;

	/* wait link status */
	do {
		for (i = 0; i < 100; i++) {};
		tmp16 = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);
	} while ((tmp16 & RTL8211_PHYSR_LINK) == 0) ;
}

void rtl8211_phy_info(FTGMAC030_Control *ctrl)
{
	const uint16_t phycr = ctrl->phyread(ctrl, RTL8211_REG_PHYCR);
	prints("PHY: Realtek RTL8211\n");
	prints("Connection type: ");
	if (phycr & RTL8211_PHYCR_ASSERT_CRS) {
		prints("GMII\n");
	}
	else {
		prints("RGMII\n");
	}
}

void rtl8211_clear_phy_int(FTGMAC030_Control *ctrl)
{
	/* read RTL8211_REG_INSR (0x13) to clear interrupt of phy */
	ctrl->phyread(ctrl, RTL8211_REG_INSR);
}

void rtl8211_set_phy_int(FTGMAC030_Control *ctrl)
{
	ctrl->phywrite(ctrl, 0x0, RTL8211_REG_INER);
	ctrl->phywrite(ctrl, RTL8211_INSR_LINK_CHG, RTL8211_REG_INER);
}

int rtl8211_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	uint32_t maccr = 0x0;
	/*
	 * Change advitisement to chage speed and duplex
	 * 10/100 in register 4, 1000 in register 9
	 */
	uint16_t tmp16 = 0x0;
	uint16_t tmp16_2 = 0x0;
	uint16_t physr = 0x0;
	uint16_t ret = 1;

	mac_regs->ier &= ~FTGMAC030_INT_PHYSTS_CHG;

	/* read 1000Mbps ability */
	tmp16 = ctrl->phyread(ctrl, MII_REG_1000_CR);

	if (_1000 == speed) {

		/* If do not enable 1000 caps, turn them on */
		if (!(tmp16 & (MII_1000_CR_HD | MII_1000_CR_FD))) {
			tmp16 |= (MII_1000_CR_HD | MII_1000_CR_FD);
			ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);
		}

		/* If register 6.1 is not 1, renegoation required to check
		   partner 1000Mbps caps */
		tmp16_2 = ctrl->phyread(ctrl, MII_REG_ANER);
		if (!(tmp16_2 & MII_ANER_PAGE_RX)) {
			/* Restart auto negotiation */
			ctrl->phy_auto_neo(ctrl);
			prints("Wait 5 seconds for changing speed and mode ...\n");
			ftgmac030_delaysecs(5);
		}

		tmp16_2 = ctrl->phyread(ctrl, MII_REG_1000_SR);
		if (FULL == duplex) {
			/* check partner's ability */
			if (tmp16_2 & MII_1000_SR_FD) {
				tmp16 |= MII_1000_CR_FD;
			} else {
				prints("partner doesnot support 1000/Full duplex.\n");
				goto error;
			}
		}
		else if (HALF == duplex) {
			/* check partner's ability */
			if (tmp16_2 & MII_1000_SR_HD) {
				tmp16 |= MII_1000_CR_HD;
			} else {
				prints("partner doesnot support 1000/Half duplex.\n");
				goto error;
			}
		} else {
			prints("Argument value duplex error !\n");
			goto error;
		}

		ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);

	} else {

		tmp16 &= ~(MII_1000_CR_HD | MII_1000_CR_FD);
		/* turn off 1000Mbps capability, or can not change to 100/10 */
		ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);

		tmp16_2 = ctrl->phyread(ctrl, MII_REG_ANLPAR);
		tmp16 = ctrl->phyread(ctrl, MII_REG_ANAR);
		physr = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);
		tmp16 &= ~(MII_ANAR_10_HD | MII_ANAR_10_FD | MII_ANAR_100_HD | MII_ANAR_100_FD);

		if (_100 == speed) {
			if (FULL == duplex) {
				if (tmp16_2 & MII_ANLPAR_100_FD) {
					tmp16 |= MII_ANAR_100_FD;
				} else {
					prints("partner doesnot support 100/Full duplex.\n");
					goto error;
				}
			} else if (HALF == duplex) {
				if (tmp16_2 & MII_ANLPAR_100_HD) {
					tmp16 |= MII_ANAR_100_HD;
				} else {
					prints("partner doesnot support 100/Half duplex.\n");
					goto error;
				}
			} else {
				prints("Argument value duplex errror!\n");
				goto error;
			}

		} else if (_10 == speed) {
			if (FULL == duplex) {
				if (tmp16_2 & MII_ANLPAR_10_FD) {
					tmp16 |= MII_ANAR_10_FD;
				} else {
					prints("partner doesnot support 10/Full duplex.\n");
					goto error;
				}
			} else if (HALF == duplex) {
				if (tmp16_2 & MII_ANLPAR_10_HD) {
					tmp16 |= MII_ANAR_10_HD;
				} else {
					prints("partner doesnot support 10/Half duplex.\n");
					goto error;
				}
			} else {
				prints("Argument value duplex error!\n");
				goto error;
			}
		} else {
			prints("Argumnent value speed error!\n");
			goto error;
		}

		ctrl->phywrite(ctrl, tmp16, MII_REG_ANAR);
	}

	/* Restart auto negotiation */
	ctrl->phy_auto_neo(ctrl);

	/* read M1111_REG_ISR (19) to clear interrupt of phy and check the status */
	while (1) {
		tmp16 = ctrl->phyread(ctrl, RTL8211_REG_INSR);
		if (tmp16 & RTL8211_INSR_LINK_CHG) {
			if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
				mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
				prints("[ISR:0] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
			}
			break;
		}

		if (tmp16 & RTL8211_INSR_AUTO_NEO) {
			prints("??? Auto negotiation error\n");
			break;
		}
	}

	if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
		tmp16 = ctrl->phyread(ctrl, RTL8211_REG_INSR);
		mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
		prints("[ISR:1] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
		while (tmp16 & RTL8211_INSR_LINK_CHG) {;}
	}

	/* read status of phy */
	physr = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);
	if (!(physr & RTL8211_PHYSR_LINK)) {
		prints("??? Link status => FAIL\n");
		goto error;
	}

	maccr = mac_regs->maccr;
	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if ((_1000 == speed) && (0x2 == RTL8211_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_1000;
		prints("SPEED: 1000Mbps ");
	} else if ((_100 == speed) && (0x1 == RTL8211_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_100;
		prints("SPEED: 100Mbps ");
	} else if ((_10 == speed) && (0x0 == RTL8211_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_10;
		prints("SPEED: 10Mbps ");
	} else {
		prints("Configured speed error!\n");
		goto error;
	}

	if ((FULL == duplex) && (1 == RTL8211_PHYSR_DUPLEX(physr))) {
		maccr |= FTGMAC030_MACCR_FULLDUP;
		prints("Full-duplex\n");
	} else if ((HALF == duplex) && (0 == RTL8211_PHYSR_DUPLEX(physr))) {
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
		prints("Half-duplex\n");
	} else {
		prints("Configured duplex mode error!\n");
		goto error;
	}

	mac_regs->maccr = maccr;
	ret = 0;

error:
	mac_regs->ier |= FTGMAC030_INT_PHYSTS_CHG;

	return ret;
}

void rtl8211_disable_green_ethernet(FTGMAC030_Control *ctrl)
{
	ctrl->phywrite(ctrl, 0x0003, 31);
	ctrl->phywrite(ctrl, 0x3246, 25);
	ctrl->phywrite(ctrl, 0x0000, 31);
}

uint32_t rtl8211_is_link_up(FTGMAC030_Control *ctrl)
{
	return (ctrl->phyread(ctrl, RTL8211_REG_PHYSR) & RTL8211_PHYSR_LINK);
}

void rtl8211_show_link_status(FTGMAC030_Control *ctrl)
{
	uint16_t tmp16 = ctrl->phyread(ctrl, RTL8211_REG_PHYSR);
	if (tmp16 & RTL8211_PHYSR_LINK) {
		if (RTL8211_PHYSR_SPEED(tmp16) == 2) {
			prints("SPEED: 1000Mbps\n");
		} else if (RTL8211_PHYSR_SPEED(tmp16) == 1) {
			prints("SPEED: 100Mbps\n");
		} else if (RTL8211_PHYSR_SPEED(tmp16) == 0) {
			prints("SPEED: 10Mbps\n");
		} else {
			prints("SPEED: Reserved\n");
		}

		if (RTL8211_PHYSR_DUPLEX(tmp16)) {
			prints("FULL-duplex\n");
		} else {
			prints("Half-duplex\n");
		}
	} else {
		prints("link not established\n");
	}
}

/* Realtek RTL8201 PHY */
/*
 * Interrupt Status Register 30
 */
#define RTL8201_REG_INSR         30
#define RTL8201_INSR_LINK_CHG   (1 << 11)

#define RTL8201_REG_PAGE_SEL     31

/* Page 7 Interrupt Enable Register 19 */
#define RTL8201_REG_INER         19
#define RTL8201_INER_LINK_CHG   (1 << 13)

/* Page 7 RMII Mode setting Register 16 */
#define RTL8201_REG_RMII_SET     16
#define RTL8201_RMII_TX_TIME(x) ((x & 0xf) << 8)
#define RTL8201_RMII_RX_TIME(x) ((x & 0xf) << 4)
#define RTL8201_RMII_MODE       (1 << 3)

uint32_t rtl8201_phy_processing(FTGMAC030_Control *ctrl)
{
	uint16_t val = 0x0;
	uint32_t maccr = 0x0;

	/* For current link status, must read this register twice */
	val = ctrl->phyread(ctrl, MII_REG_BMSR);
	val = ctrl->phyread(ctrl, MII_REG_BMSR);

	if (!(val & (MII_BMSR_LINK_STATUS | MII_BMSR_AUTO_NEG_COMPLETE))) {
		prints("??? Link status => FAIL\n");
		return 1;
	}

	val = ctrl->phyread(ctrl, MII_REG_BMCR);
	maccr = mac_regs->maccr;

	if (val & MII_BMCR_DUPLEX) {
		maccr |= FTGMAC030_MACCR_FULLDUP;
	} else {
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
	}

	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if (val & MII_BMCR_SPEED_L) {
		maccr |= FTGMAC030_MACCR_SPEED_100;
	} else {
		maccr |= FTGMAC030_MACCR_SPEED_10;
	}

	mac_regs->maccr = maccr;

	/* Set RMII mode */
	/* 0:MII or GMII, 1:RMII, 2: RGMII */
	ctrl->phywrite(ctrl, 7, RTL8201_REG_PAGE_SEL);
	val = ctrl->phyread(ctrl, RTL8201_REG_RMII_SET);

	if (!(mac_regs->gisr & 3))
		val &= ~RTL8201_RMII_MODE;
	else
		val |= RTL8201_RMII_MODE;

	/* Adjust RMII TX and RX Interface Timing
	 * RMII tx_interface_timing OK values are 5, 6 and 7.
	 * RMII rx_interface_timing OK values are 2-5 and 7
	 */
	val |= RTL8201_RMII_TX_TIME(0xf);
	//val |= RTL8201_RMII_TX_TIME(6);
	val |= RTL8201_RMII_RX_TIME(0xf);
	//val |= RTL8201_RMII_RX_TIME(3);

	ctrl->phywrite(ctrl, val, RTL8201_REG_RMII_SET);

	return 0;
}

void rtl8201_phy_auto_neo(FTGMAC030_Control *ctrl)
{
	/* kick off phy auto negotiation */
	uint16_t tmp16;
	int i;
	tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	tmp16 |= (MII_BMCR_RESTART_AUTO_NEG | MII_BMCR_AUTO_NEG_EN);
	ctrl->phywrite(ctrl, tmp16, MII_REG_BMCR);

	do {
		for (i = 0; i < 100; i++){};
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	} while (tmp16 & MII_BMCR_RESTART_AUTO_NEG);

	do {
		/* read twice for current link status */
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMSR);
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMSR);

		if (tmp16 & (MII_BMSR_AUTO_NEG_COMPLETE | MII_BMSR_LINK_STATUS))
			break;

		for (i = 0; i < 100; i++){};
	} while (1);

}

void rtl8201_phy_info(FTGMAC030_Control *ctrl)
{
	uint16_t val;

	prints("PHY: Realtek RTL8201\n");
	prints("Connection type: ");

	ctrl->phywrite(ctrl, 7, RTL8201_REG_PAGE_SEL);
	val = ctrl->phyread(ctrl, RTL8201_REG_RMII_SET);
	if (val & RTL8201_RMII_MODE) {
		prints("RMII\n");
	}
	else {
		prints("MII\n");
	}
}

void rtl8201_clear_phy_int(FTGMAC030_Control *ctrl)
{
	/* read register 30 to clear interrupt of phy */
	ctrl->phyread(ctrl, RTL8201_REG_INSR);
}

void rtl8201_set_phy_int(FTGMAC030_Control *ctrl)
{
	ctrl->phywrite(ctrl, 7, RTL8201_REG_PAGE_SEL);
	ctrl->phywrite(ctrl, 0x0, RTL8201_REG_INER);
	ctrl->phywrite(ctrl, RTL8201_INER_LINK_CHG, RTL8201_REG_INER);
}

int rtl8201_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	uint32_t maccr = 0x0;
	/*
	 * Change advitisement to chage speed and duplex
	 * 10/100 in register 4, 1000 in register 9
	 */
	uint16_t tmp16 = 0x0;
	uint16_t tmp16_2 = 0x0;
	uint16_t physr = 0x0;
	uint16_t ret = 1;

	mac_regs->ier &= ~FTGMAC030_INT_PHYSTS_CHG;

	tmp16_2 = ctrl->phyread(ctrl, MII_REG_ANLPAR);
	tmp16 = ctrl->phyread(ctrl, MII_REG_ANAR);
	tmp16 &= ~(MII_ANAR_10_HD | MII_ANAR_10_FD | MII_ANAR_100_HD | MII_ANAR_100_FD);

	if (_100 == speed) {
		if (FULL == duplex) {
			if (tmp16_2 & MII_ANLPAR_100_FD) {
				tmp16 |= MII_ANAR_100_FD;
			} else {
				prints("partner doesnot support 100/Full duplex.\n");
				goto error;
			}
		} else if (HALF == duplex) {
			if (tmp16_2 & MII_ANLPAR_100_HD) {
				tmp16 |= MII_ANAR_100_HD;
			} else {
				prints("partner doesnot support 100/Half duplex.\n");
				goto error;
			}
		} else {
			prints("Argument value duplex errror!\n");
			goto error;
		}

	} else if (_10 == speed) {
		if (FULL == duplex) {
			if (tmp16_2 & MII_ANLPAR_10_FD) {
				tmp16 |= MII_ANAR_10_FD;
			} else {
				prints("partner doesnot support 10/Full duplex.\n");
				goto error;
			}
		} else if (HALF == duplex) {
			if (tmp16_2 & MII_ANLPAR_10_HD) {
				tmp16 |= MII_ANAR_10_HD;
			} else {
				prints("partner doesnot support 10/Half duplex.\n");
				goto error;
			}
		} else {
			prints("Argument value duplex error!\n");
			goto error;
		}
	} else {
		prints("Argumnent value speed error!\n");
		goto error;
	}

	ctrl->phywrite(ctrl, tmp16, MII_REG_ANAR);

	/* Restart auto negotiation */
	ctrl->phy_auto_neo(ctrl);

	physr = ctrl->phyread(ctrl, MII_REG_BMCR);
	maccr = mac_regs->maccr;
	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if ((_100 == speed) && (physr & MII_BMCR_SPEED_L)) {
		maccr |= FTGMAC030_MACCR_SPEED_100;
		prints("SPEED: 100Mbps ");
	} else if ((_10 == speed) && !(physr & MII_BMCR_SPEED_L)) {
		maccr |= FTGMAC030_MACCR_SPEED_10;
		prints("SPEED: 10Mbps ");
	} else {
		prints("Configured speed error!\n");
		goto error;
	}

	if ((FULL == duplex) && (physr & MII_BMCR_DUPLEX)) {
		maccr |= FTGMAC030_MACCR_FULLDUP;
		prints("Full-duplex\n");
	} else if ((HALF == duplex) && !(physr & MII_BMCR_DUPLEX)) {
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
		prints("Half-duplex\n");
	} else {
		prints("Configured duplex mode error!\n");
		goto error;
	}

	mac_regs->maccr = maccr;
	ret = 0;

error:
	mac_regs->ier |= FTGMAC030_INT_PHYSTS_CHG;

	return ret;
}

uint32_t rtl8201_is_link_up(FTGMAC030_Control *ctrl)
{
	return (ctrl->phyread(ctrl, MII_REG_BMSR) & MII_BMSR_LINK_STATUS);
}

void rtl8201_show_link_status(FTGMAC030_Control *ctrl)
{
	uint16_t tmp16;

	/* For current link status, must read this register twice */
	tmp16 = ctrl->phyread(ctrl, MII_REG_BMSR);
	tmp16 = ctrl->phyread(ctrl, MII_REG_BMSR);

	if (tmp16 & (MII_BMSR_LINK_STATUS | MII_BMSR_AUTO_NEG_COMPLETE)) {
	
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);

		if (tmp16 & MII_BMCR_SPEED_L) {
			prints("SPEED: 100Mbps\n");
		} else {
			prints("SPEED: 10Mbps\n");
		}

		if (tmp16 & MII_BMCR_DUPLEX) {
			prints("FULL-duplex\n");
		} else {
			prints("Half-duplex\n");
		}

	} else {
		prints("link not established\n");
	}
}
