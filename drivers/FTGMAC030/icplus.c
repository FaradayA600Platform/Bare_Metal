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

/*
 * IC+ PHY definitions
 */
#define ICPLUS_REG_CTRL                0
#define ICPLUS_CTRL_DUPLEX            (1 << 8) /* 1 = full, 0 = half */
#define ICPLUS_CTRL_RESTART_AUTO_NEG  (1 << 9)
#define ICPLUS_CTRL_AUTO_NEG_EN       (1 << 12)
#define ICPLUS_CTRL_SPEED             (1 << 13) /* 1 = 100Mbps, 0 = 10Mbps */

#define ICPLUS_REG_STS                 1
#define ICPLUS_STS_LINK_EST           (1 << 2)
#define ICPLUS_STS_AUTO_NEG_COMPLETE  (1 << 5)

#define ICPLUS_REG_ANAR                4
#define ICPLUS_ANAR_10_HD             (1 << 5)
#define ICPLUS_ANAR_10_FD             (1 << 6)
#define ICPLUS_ANAR_100_HD            (1 << 7)
#define ICPLUS_ANAR_100_FD            (1 << 8)

#define ICPLUS_REG_ANLPAR              5
#define ICPLUS_ANLPAR_10_HD           (1 << 5)
#define ICPLUS_ANLPAR_10_FD           (1 << 6)
#define ICPLUS_ANLPAR_100_HD          (1 << 7)
#define ICPLUS_ANLPAR_100_FD          (1 << 8)

#define ICPLUS_REG_PHYCR               0x10
#define ICPLUS_PHYCR_RMII_V10         (1 << 13)

#define ICPLUS_REG_ISR                 0x11
#define ICPLUS_ISR_LINK_MASK          (1 << 8)
#define ICPLUS_ISR_LINK_CHG           (1 << 0)

#define ICPLUS_REG_PHYSR               0x12
#define ICPLUS_PHYSR_LINK(x)          ((x >> 10) & 1)
#define ICPLUS_PHYSR_DUPLEX(x)        ((x >> 13) & 1)
#define ICPLUS_PHYSR_SPEED(x)         ((x >> 14) & 3)

uint32_t icplus_phy_processing(FTGMAC030_Control *ctrl)
{
	uint16_t val = 0x0;
	uint32_t maccr = 0x0;

	/* read status of phy */
	val = ctrl->phyread(ctrl, ICPLUS_REG_PHYSR);
	if (0 == ICPLUS_PHYSR_LINK(val)) {
		prints("??? Link status => FAIL\n");
		return 1;
	}

	if (!ICPLUS_PHYSR_SPEED(val)) {
		prints("??? Speed not resolved => FAIL\n");
		return 1;
	}

	if (!ICPLUS_PHYSR_DUPLEX(val)) {
		prints("??? Duplex not resolved => FAIL\n");
		return 1;
	}

	val = ctrl->phyread(ctrl, ICPLUS_REG_CTRL);
	maccr = mac_regs->maccr;

	if (val & ICPLUS_CTRL_DUPLEX) {
		/* full duplex */
		maccr |= FTGMAC030_MACCR_FULLDUP;
	} else {
		/* half duplex */
		maccr &= ~FTGMAC030_MACCR_FULLDUP;
	}

	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if (val & ICPLUS_CTRL_SPEED) {
		/* 100 */
		maccr |= FTGMAC030_MACCR_SPEED_100;
	} else {
		/* 10 */
		maccr |= FTGMAC030_MACCR_SPEED_10;
	}

	mac_regs->maccr = maccr;

	/* Set RMII version 1.0 */
	val = ctrl->phyread(ctrl, ICPLUS_REG_PHYCR);
	ctrl->phywrite(ctrl, (ICPLUS_PHYCR_RMII_V10 | val), ICPLUS_REG_PHYCR);

	return 0;
}

void icplus_phy_auto_neo(FTGMAC030_Control *ctrl)
{
	/* kick off phy auto negotiation */
	uint16_t tmp16;
	int i;
	tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_CTRL);
	tmp16 |= (ICPLUS_CTRL_RESTART_AUTO_NEG | ICPLUS_CTRL_AUTO_NEG_EN);
	ctrl->phywrite(ctrl, tmp16, ICPLUS_REG_CTRL);

	do {
		for (i = 0; i < 100; i++){};
		tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_CTRL);
	} while ((tmp16 & ICPLUS_CTRL_RESTART_AUTO_NEG) > 0) ;

	/* wait link status */
	do {
		for (i = 0; i < 100; i++) {};
		tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_PHYSR);
	} while (ICPLUS_PHYSR_LINK(tmp16) == 0);
}

void icplus_phy_info(FTGMAC030_Control *ctrl)
{
	prints("PHY: IC+\n");
}

void icplus_clear_phy_int(FTGMAC030_Control *ctrl)
{
	/* read to clear interrupt of phy */
	ctrl->phyread(ctrl, ICPLUS_REG_ISR);
}

void icplus_set_phy_int(FTGMAC030_Control *ctrl)
{
	ctrl->phywrite(ctrl, 0x0, ICPLUS_REG_ISR);
	ctrl->phywrite(ctrl, (0x8000 | ICPLUS_ISR_LINK_MASK), ICPLUS_REG_ISR);
}

int icplus_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	uint32_t maccr = 0x0;
	/*
	 * Change advitisement to chage speed and duplex
	 * 10/100 in register 4, 1000 in register 9
	 */
	uint16_t tmp16 = 0x0;
	uint16_t tmp16_2 = 0x0;
	uint16_t ret = 1;

	mac_regs->ier &= ~FTGMAC030_INT_PHYSTS_CHG;

	tmp16_2 = ctrl->phyread(ctrl, ICPLUS_REG_ANLPAR);
	tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_ANAR);
	tmp16 &= ~(ICPLUS_ANAR_10_HD | ICPLUS_ANAR_10_FD | ICPLUS_ANAR_100_HD | ICPLUS_ANAR_100_FD);

	if (_100 == speed) {
		if (FULL == duplex) {
			if (tmp16_2 & ICPLUS_ANLPAR_100_FD) {
				tmp16 |= ICPLUS_ANAR_100_FD;
			} else {
				prints("partner doesnot support 100/Full duplex.\n");
				goto error;
			}
		} else if (HALF == duplex) {
			if (tmp16_2 & ICPLUS_ANLPAR_100_HD) {
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
			if (tmp16_2 & ICPLUS_ANLPAR_10_FD) {
				tmp16 |= ICPLUS_ANAR_10_FD;
			} else {
				prints("partner doesnot support 10/Full duplex.\n");
				goto error;
			}
		} else if (HALF == duplex) {
			if (tmp16_2 & ICPLUS_ANLPAR_10_HD) {
				tmp16 |= ICPLUS_ANAR_10_HD;
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

	/* read M1111_REG_ISR (19) to clear interrupt of phy and check the status */
	while (1) {
		tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_ISR);
		if (tmp16 & ICPLUS_ISR_LINK_CHG) {
			if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
				mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
				prints("[ISR:0] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
			}
			break;
		}
	}

	if (mac_regs->isr & FTGMAC030_INT_PHYSTS_CHG) {
		tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_ISR);
		mac_regs->isr = FTGMAC030_INT_PHYSTS_CHG;
		prints("[ISR:1] PHYSTS_CHG cleared 0x%x\n", mac_regs->isr);
		while (tmp16 & ICPLUS_ISR_LINK_CHG) {;}
	}

	/* read status of phy */
	tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_PHYSR);
	if (0 == ICPLUS_PHYSR_LINK(tmp16)) {
		prints("??? Link status => FAIL\n");
		goto error;
	}

	if (!ICPLUS_PHYSR_SPEED(tmp16)) {
		prints("??? Speed not resolved => FAIL\n");
		goto error;
	}

	if (!ICPLUS_PHYSR_DUPLEX(tmp16)) {
		prints("??? Duplex not resolved => FAIL\n");
		goto error;
	}

	tmp16 = ctrl->phyread(ctrl, ICPLUS_REG_CTRL);
	maccr = mac_regs->maccr;
	maccr &= ~FTGMAC030_MACCR_SPEED_MASK;
	if ((_100 == speed) && (tmp16 & ICPLUS_CTRL_SPEED)) {
		maccr |= FTGMAC030_MACCR_SPEED_100;
		prints("SPEED: 100Mbps ");
	} else if ((_10 == speed) && !(tmp16 & ICPLUS_CTRL_SPEED)) {
		maccr |= FTGMAC030_MACCR_SPEED_10;
		prints("SPEED: 10Mbps ");
	} else {
		prints("Configured speed error!\n");
		goto error;
	}

	if ((FULL == duplex) && (tmp16 & ICPLUS_CTRL_DUPLEX)) {
		maccr |= FTGMAC030_MACCR_FULLDUP;
		prints("Full-duplex\n");
	} else if ((HALF == duplex) && !(tmp16 & ICPLUS_CTRL_DUPLEX)) {
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

uint32_t icplus_is_link_up(FTGMAC030_Control *ctrl)
{
	uint16_t val;

	val = ctrl->phyread(ctrl, ICPLUS_REG_STS);

	return (val & ICPLUS_STS_LINK_EST);
}

void icplus_show_link_status(FTGMAC030_Control *ctrl)
{
	uint16_t val;

	val = ctrl->phyread(ctrl, ICPLUS_REG_STS);

	if (val & ICPLUS_STS_LINK_EST) {
	
		val = ctrl->phyread(ctrl, ICPLUS_REG_CTRL);

		if (val & ICPLUS_CTRL_SPEED) {
			prints("SPEED: 100Mbps\n");
		} else {
			prints("SPEED: 10Mbps\n");
		}

		if (val & ICPLUS_CTRL_DUPLEX) {
			prints("FULL-duplex\n");
		} else {
			prints("Half-duplex\n");
		}
	} else {
		prints("link not established\n");
	}
}

