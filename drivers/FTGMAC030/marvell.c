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

#define M1111_REG_SSR  17
#define M1111_REG_IER  18
#define M1111_REG_ISR  19
#define M1111_REG_ESCR 20
#define M1111_REG_ESSR 27

/*
 * PHY specific status register (17)
 */
#define M1111_PHYSR_DUPLEX(x)  ((x >> 13) & 1)
#define M1111_PHYSR_SPEED(x)   ((x >> 14) & 3)
#define M1111_SSR_PAGE_RX      (1 << 12)
#define M1111_SSR_RESOLVED     (1 << 11)
#define M1111_SSR_LINK         (1 << 10)
#define M1111_SSR_CABLE_LEN(x) ((x & (0x7 << 7)) >> 7) /* read only */
#define M1111_SSR_MDI_STATUS   (1 << 6)
#define M1111_SSR_DOWNSHIFT    (1 << 5)
#define M1111_SSR_ENERGY       (1 << 4)
#define M1111_SSR_TX_PAUSE     (1 << 3)
#define M1111_SSR_RX_PAUSE     (1 << 2)
#define M1111_SSR_POLARITY     (1 << 1)
#define M1111_SSR_JABBER       (1 << 0)

/*
 * Interrupt enable register and status (18 and 19)
 */
#define M1111_INT_JABBER          (1 << 0)
#define M1111_INT_POLARITY_CHANGE (1 << 1)
#define M1111_INT_DTE_DETECT      (1 << 2)
#define M1111_INT_ENERGY_DETECT   (1 << 4)
#define M1111_INT_DOWNSHIT        (1 << 5)
#define M1111_INT_MDI_CROSS_CHG   (1 << 6)
#define M1111_INT_FIFO_OVER_UND   (1 << 7)
#define M1111_INT_FALSECARRIER    (1 << 8)
#define M1111_INT_SYMBOL_ERR      (1 << 9)
#define M1111_INT_LINK_CHG        (1 << 10)
#define M1111_INT_AUTO_NEO        (1 << 11)
#define M1111_INT_PAGE_RX         (1 << 12)
#define M1111_INT_DUP_CHG         (1 << 13)
#define M1111_INT_SPEED_CHG       (1 << 14)
#define M1111_INT_AUTO_NEO_ERR    (1 << 15)

/*
 * Extended PHY Specific Control Register (20)
 */
#define M1111_ESCR_TX_DISABLE (1 << 0)
#define M1111_ESCR_TX_TIME    (1 << 1)
#define M1111_ESCR_RX_TIME    (1 << 7)

/*
 * Extended phy specific status register (27)
 */
#define M1111_ESSR_HWCFG(x) (x & 0xf)


/*
 * HWCFG values
 */
#define M1111_GMII_COPPER  0xf
#define M1111_RGMII_COPPER 0xb
#define M1111_GMII_FIBER   0x7
#define M1111_RGMII_FIBER  0x6
#define M1111_GMII_SGMII   0xe
#define M1111_RESERVED     0xa

uint32_t marvell_phy_processing(FTGMAC030_Control *ctrl)
{
	uint16_t value = 0x0;
	uint32_t maccr = 0x0;

#if 1
	if ((mac_regs->gisr & 3) == 2) {
		value = ctrl->phyread(ctrl, M1111_REG_ESCR);
		if (!(value & M1111_ESCR_RX_TIME)) {
		
			value |= M1111_ESCR_RX_TIME;
			ctrl->phywrite(ctrl, value, M1111_REG_ESCR);

			/* Reset PHY */
			value = ctrl->phyread(ctrl, MII_REG_BMCR);
			value |= MII_BMCR_RESET;
			ctrl->phywrite(ctrl, value, MII_REG_BMCR);

			/* Wait reset done */
			do {
				value = ctrl->phyread(ctrl, MII_REG_BMCR);
				if (!(value & MII_BMCR_RESET))
					break;

				ftgmac030_delaysecs(1);
			} while (1) ;

			/* Wait PHY Link up */
			do {
				value = ctrl->phyread(ctrl, M1111_REG_SSR);
				if (value & M1111_SSR_LINK)
					break;

				ftgmac030_delaysecs(1);
			} while (1) ;
		
			value = ctrl->phyread(ctrl, M1111_REG_ISR);
			if (value & M1111_INT_LINK_CHG) {
				if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
					mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
					prints("[ISR:0] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
				}
			}
		}
	}
#endif

	/* read status of phy */
	value = ctrl->phyread(ctrl, M1111_REG_SSR);

	if ((value & M1111_SSR_LINK) == 0){
		prints("??? Link status => FAIL\n");
		return (0);
	}

	maccr = mac_regs->maccr;

	if (0 == M1111_PHYSR_DUPLEX(value)) {
		/* half duplex */
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
	} else {
		/* full duplex */
		maccr |= FTGMAC030_MACCR_FULLDUP;
	}

	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if (0x2 == M1111_PHYSR_SPEED(value)) {
		/* 1000 */
		maccr |= FTGMAC030_MACCR_SPEED_1000;
	} else {
		if (0x1 == M1111_PHYSR_SPEED(value)) {
			/* 100 */
			maccr |= FTGMAC030_MACCR_SPEED_100;
		} else if (0x0 == M1111_PHYSR_SPEED(value)) {
			/* 10 */
			maccr |= FTGMAC030_MACCR_SPEED_10;
		}
	}

	mac_regs->maccr = maccr;
	return 0;
}

/*
 * Change advitisement to chage speed and duplex
 * 10/100 in register 4, 1000 in register 9
 */
int marvell_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	uint32_t maccr = 0x0;
	uint16_t tmp16 = 0x0;
	uint16_t tmp16_2 = 0x0;
	uint16_t physr = 0x0;
	int ret = 1;

	maccr = mac_regs->maccr & FTGMAC030_MACCR_SPEED_MASK;

	if ((duplex == FULL) && (mac_regs->maccr & FTGMAC030_MACCR_FULLDUP)) {
		if (((_1000 == speed) && (maccr == FTGMAC030_MACCR_SPEED_1000)) ||
		    ((_100 == speed) && (maccr == FTGMAC030_MACCR_SPEED_100)) ||
		    ((_10 == speed) && (maccr == FTGMAC030_MACCR_SPEED_10))) {
			prints("No change for speed and mode ...\n");
			return 0;
		}
	}

	mac_regs->ier &= ~FTGMAC030_INT_PHYSTS_CHG;

	/* read 1000Mbps ability */
	tmp16 = ctrl->phyread(ctrl, MII_REG_1000_CR);
	tmp16 &= ~(MII_1000_CR_HD | MII_1000_CR_FD);

	if (_1000 == speed) {
		/* Enable 1000 caps */
		ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);
		if (FULL == duplex) {
			tmp16 = MII_1000_CR_FD;
		} else if (HALF == duplex) {
			tmp16 = MII_1000_CR_HD;
		} else {
			prints("Duplex value error.\n");
			goto error;
		}
#if 1
		/* Restart auto negotiation */
		ctrl->phy_auto_neo(ctrl);
		ftgmac030_delaysecs(5);

#else
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
		} else if (HALF == duplex) {
			/* check partner's ability */
			if (tmp16_2 & MII_1000_SR_HD) {
				tmp16 |= MII_1000_CR_HD;
			} else {
				prints("partner doesnot support 1000/Half duplex.\n");
				goto error;
			}
		} else {
			prints("partner doesnot support 1000/Half and 1000/Full duplex.\n");
			goto error;
		}
		ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);
#endif
	} else {

		/* turn off 1000Mbps capability, or can not change to 100/10 */
		ctrl->phywrite(ctrl, tmp16, MII_REG_1000_CR);

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
				prints("partner doesnot support 100/Half and 100/Full duplex.\n");
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
				prints("partner doesnot support 10/Half and 10/Full duplex.\n");
				goto error;
			}
		} else {
			prints("speed parameter error!\n");
			goto error;
		}
		ctrl->phywrite(ctrl, tmp16, MII_REG_ANAR);

		/* Restart auto negotiation */
		ctrl->phy_auto_neo(ctrl);
	}

	/* read M1111_REG_ISR (19) to clear interrupt of phy and check the status */
	while (1) {
		tmp16 = ctrl->phyread(ctrl, M1111_REG_ISR);
		if (tmp16 & M1111_INT_LINK_CHG) {
			if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
				mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
				prints("[ISR:0] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
			}
			break;
		}

		if (tmp16 & M1111_INT_AUTO_NEO) {
			prints("??? Auto negotiation error\n");
			break;
		}
	}

	if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
		tmp16 = ctrl->phyread(ctrl, M1111_REG_ISR);
		mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
		prints("[ISR:1] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
		while (tmp16 & M1111_INT_LINK_CHG) {;}
	}

	/* For speed 1000 check register 6.1 if valid page received
	   after renegotiation */
	if (_1000 == speed) {
		tmp16_2 = ctrl->phyread(ctrl, MII_REG_ANER);

		if (tmp16_2 & MII_ANER_PAGE_RX) {
			tmp16 = ctrl->phyread(ctrl, MII_REG_1000_SR);
			if((FULL == duplex) &&
			   !(tmp16 & MII_1000_SR_FD))
				prints("partner doesnot support 1000/Full duplex "\
				       "(0x%x).\n", tmp16);
			
			if ((HALF == duplex) &&
			    !(tmp16 & MII_1000_SR_HD))
				prints("partner doesnot support 1000/Half duplex "\
				       "(0x%x).\n", tmp16);
		} else
			prints("Valid page not received ... \n");
	}

	/* read status of phy */
	physr = ctrl->phyread(ctrl, M1111_REG_SSR);
	if ((physr & M1111_SSR_LINK) == 0){
		prints("??? Link status => FAIL\n");
		goto error;
	}

	maccr = mac_regs->maccr;
	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if ((_1000 == speed) && (0x2 == M1111_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_1000;
		prints("SPEED: 1000Mbps ");
	} else if ((_100 == speed) && (0x1 == M1111_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_100;
		prints("SPEED: 100Mbps ");
	} else if ((_10 == speed) && (0x0 == M1111_PHYSR_SPEED(physr))) {
		maccr |= FTGMAC030_MACCR_SPEED_10;
		prints("SPEED: 10Mbps ");
	} else {
		prints("Configured speed error!\n");
		goto error;
	}

	if ((FULL == duplex) && (1 == M1111_PHYSR_DUPLEX(physr))) {
		maccr |= FTGMAC030_MACCR_FULLDUP;
		prints("Full-duplex\n");
	} else if ((HALF == duplex) && (0 == M1111_PHYSR_DUPLEX(physr))) {
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

void marvell_phy_auto_neo(FTGMAC030_Control *ctrl)
{
	/* kick off phy auto negotiation */
	uint16_t tmp16;
	int i;
	tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	tmp16 |= (MII_BMCR_RESTART_AUTO_NEG | MII_BMCR_AUTO_NEG_EN);
	ctrl->phywrite(ctrl, tmp16, MII_REG_BMCR);

	do {
		for (i = 0; i < 100000; i++){};
		tmp16 = ctrl->phyread(ctrl, MII_REG_BMCR);
	} while ((tmp16 & MII_BMCR_RESTART_AUTO_NEG) > 0) ;

	/* wait link status */
	do {
		for (i = 0; i < 100000; i++) {};
		tmp16 = ctrl->phyread(ctrl, M1111_REG_SSR);
	} while ((tmp16 & M1111_SSR_LINK) == 0) ;
}

void marvell_phy_info(FTGMAC030_Control *ctrl)
{
	const uint16_t essr = ctrl->phyread(ctrl, M1111_REG_ESSR);
	uint16_t hwcfg = M1111_ESSR_HWCFG(essr);

	prints("PHY: Marvell\n");
	prints("Connection type: ");
	switch (hwcfg) {
	case M1111_GMII_COPPER:
		prints("GMII -> Copper\n");
		break;
	case M1111_RGMII_COPPER:
		prints("RGMII -> Copper\n");
		break;
	default:
		prints("Unknown, see datasheet\n");
		prints("HWCFG = 0x%x\n", hwcfg);
		break;
	}
}

void marvell_clear_phy_int(FTGMAC030_Control *ctrl)
{
	/* read M1111_REG_ISR (19) to clear interrupt of phy */
	ctrl->phyread(ctrl, M1111_REG_ISR);
}

void marvell_set_phy_int(FTGMAC030_Control *ctrl)
{
	ctrl->phywrite(ctrl, 0x0, M1111_REG_IER);
	ctrl->phywrite(ctrl, M1111_INT_LINK_CHG, M1111_REG_IER);
}

uint32_t marvell_is_link_up(FTGMAC030_Control *ctrl)
{
	return (ctrl->phyread(ctrl, M1111_REG_SSR) & M1111_SSR_LINK);
}

void marvell_show_link_status(FTGMAC030_Control *ctrl)
{
	uint16_t tmp16 = ctrl->phyread(ctrl, M1111_REG_SSR);
	if (tmp16 & M1111_SSR_LINK) {
		if (M1111_PHYSR_SPEED(tmp16) == 2) {
			prints("SPEED: 1000Mbps\n");
		} else if (M1111_PHYSR_SPEED(tmp16) == 1) {
			prints("SPEED: 100Mbps\n");
		} else if (M1111_PHYSR_SPEED(tmp16) == 0) {
			prints("SPEED: 10Mbps\n");
		} else {
			prints("SPEED: Reserved\n");
		}

		if (M1111_PHYSR_DUPLEX(tmp16)) {
			prints("FULL-duplex\n");
		} else {
			prints("Half-duplex\n");
		}
	} else {
		prints("link not established\n");
	}
}

void m88e1116r_phy_info(FTGMAC030_Control *ctrl)
{

	prints("PHY: Marvell m88e1116r\n");
}

#define MII_M1011_PHY_SCR               0x10
#define MII_M1011_PHY_SCR_AUTO_CROSS    0x0060

#define MII_M1116R_MSCR_REG_MAC      21
#define MII_88E1121_PHY_MSCR_RX_DELAY	(1 << 5)
#define MII_88E1121_PHY_MSCR_TX_DELAY	(1 << 4)
#define MII_MARVELL_PHY_PAGE		22

uint32_t m88e1116r_config_init(FTGMAC030_Control *ctrl)
{
	int temp;

	temp = ctrl->phyread(ctrl, MII_REG_BMCR);
	temp |= MII_BMCR_RESET;
	ctrl->phywrite(ctrl, temp, MII_REG_BMCR);

	udelay(500000);
#if 0
	ctrl->phywrite(ctrl, 0, MII_MARVELL_PHY_PAGE);

	temp = ctrl->phyread(ctrl, MII_M1011_PHY_SCR);
	temp |= (7 << 12);      /* max number of gigabit attempts */
	temp |= (1 << 11);      /* enable downshift */
	temp |= MII_M1011_PHY_SCR_AUTO_CROSS;
	ctrl->phywrite(ctrl, temp, MII_M1011_PHY_SCR);
#endif

	ctrl->phywrite(ctrl, 2, MII_MARVELL_PHY_PAGE);

	//Set Tx/Rx Delay for 88E1116R PHY
	temp = ctrl->phyread(ctrl, MII_M1116R_MSCR_REG_MAC);
	temp |= MII_88E1121_PHY_MSCR_RX_DELAY;
	//temp |= MII_88E1121_PHY_MSCR_TX_DELAY;
	ctrl->phywrite(ctrl, temp, MII_M1116R_MSCR_REG_MAC);

	ctrl->phywrite(ctrl, 0, MII_MARVELL_PHY_PAGE);

	temp = ctrl->phyread(ctrl, MII_REG_BMCR);
	temp |= MII_BMCR_RESET;
	ctrl->phywrite(ctrl, temp, MII_REG_BMCR);

	udelay(500000);

	generic_phy_processing(ctrl);

	return 0;
}

static int genphy_config_advert(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	int oldadv, adv, bmsr;
	int changed = 0;
#if 0
	/* Only allow advertising what this PHY supports */
	phydev->advertising &= phydev->supported;
	advertise = phydev->advertising;
#endif
	/* Setup standard advertisement */
	adv = ctrl->phyread(ctrl, MII_ADVERTISE);

	oldadv = adv;
	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
	ADVERTISE_PAUSE_ASYM);
	if (speed == _100) {
		if (FULL == duplex)
			adv |= ADVERTISE_100FULL;
		else
			adv |= ADVERTISE_100HALF;
	} else if (speed == _10) {
		if (FULL == duplex)
			adv |= ADVERTISE_10FULL;
		else
			adv |= ADVERTISE_10HALF;
	}

	if (adv != oldadv) {
		ctrl->phywrite(ctrl, adv, MII_ADVERTISE);
		changed = 1;
	}

	bmsr = ctrl->phyread(ctrl, MII_BMSR);

	/* Per 802.3-2008, Section 22.2.4.2.16 Extended status all
	* 1000Mbits/sec capable PHYs shall have the BMSR_ESTATEN bit set to a
	* logical 1.
	*/
	if (!(bmsr & BMSR_ESTATEN))
		return changed;

	/* Configure gigabit if it's supported */
	adv = ctrl->phyread(ctrl, MII_CTRL1000);

	oldadv = adv;
	adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

	if (speed == _1000) {
		if (FULL == duplex)
			adv |= ADVERTISE_1000FULL;
		else
			adv |= ADVERTISE_1000HALF;

		if (adv != oldadv)
			changed = 1;
	}

	ctrl->phywrite(ctrl, adv, MII_CTRL1000);

	return changed;
}

int genphy_setup_forced(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	int ctl = 0;

	if (_1000 == speed)
		ctl |= BMCR_SPEED1000;
	else if (_100 == speed)
		ctl |= BMCR_SPEED100;

	if (FULL == duplex)
		ctl |= BMCR_FULLDPLX;

	ctrl->phywrite(ctrl, ctl, MII_BMCR);

	return 0;
}

void m88e1116r_config_aneg(FTGMAC030_Control *ctrl)
{
        int ctl, retval;

        ctl = ctrl->phyread(ctrl, MII_BMCR);
        ctl |= BMCR_ANENABLE | BMCR_ANRESTART;
        
        /* Don't isolate the PHY if we're negotiating */
        ctl &= ~BMCR_ISOLATE;

        ctrl->phywrite(ctrl, ctl, MII_BMCR);

	do {	
        	retval = ctrl->phyread(ctrl, MII_BMSR);

	} while(!(retval & BMSR_ANEGCOMPLETE));
}

int m88e1116r_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	int result;
#if 0
	if (AUTONEG_ENABLE != phydev->autoneg)
		return genphy_setup_forced(ctrl, duplex, speed);
#endif

	result = genphy_config_advert(ctrl, duplex, speed);

	if (result == 0) {
		/* Advertisement hasn't changed, but maybe aneg was never on to
		* begin with?  Or maybe phy was isolated?
		*/ 
		int ctl = ctrl->phyread(ctrl, MII_BMCR);

		if (ctl < 0)
			return ctl;

		if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
			result = 1; /* do restart aneg */
	}

	/* Only restart aneg if we are advertising something different
	* than we were before.
	*/     
	if (result > 0)
		m88e1116r_config_aneg(ctrl);

	generic_phy_processing(ctrl);
		
	return 0;
}

