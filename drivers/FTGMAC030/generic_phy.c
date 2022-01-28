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

uint32_t generic_phy_processing(FTGMAC030_Control *ctrl)
{
	int maccr;
#if 1
	int status, adv;
	int lpa;
	int lpagb = 0;
	int common_adv;
	int common_adv_gb = 0;
	int retry = 32;

read_bmsr:
	/* Do a fake read */
	status = ctrl->phyread(ctrl, MII_BMSR);

	/* Read link and autonegotiation status */
	status = ctrl->phyread(ctrl, MII_BMSR);

	if ((status & BMSR_LSTATUS) == 0) {
		if (retry > 0) {
			retry--;
			udelay(100000);
			goto read_bmsr;
		}
		prints("??? Link status => FAIL\n");
		return 1;
	}

	lpa = ctrl->phyread(ctrl, MII_LPA);

	maccr = mac_regs->maccr;
	maccr &= ~(FTGMAC030_MACCR_SPEED_MASK | FTGMAC030_MACCR_FULLDUP);

	if (lpa & LPA_LPACK) {
		if (status & BMSR_ESTATEN) {
			status = ctrl->phyread(ctrl, MII_ESTATUS);
			if (status & (ESTATUS_1000_TFULL | ESTATUS_1000_THALF)) {

				lpagb = ctrl->phyread(ctrl, MII_STAT1000);

				adv = ctrl->phyread(ctrl, MII_CTRL1000);

				common_adv_gb = lpagb & (adv << 2);
			}
		}

		lpa = ctrl->phyread(ctrl, MII_LPA);

		adv = ctrl->phyread(ctrl, MII_ADVERTISE);

		common_adv = lpa & adv;

		if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)) {
			maccr |= FTGMAC030_MACCR_SPEED_1000;

			if (common_adv_gb & LPA_1000FULL)
				maccr |= FTGMAC030_MACCR_FULLDUP;
		} else if (common_adv & (LPA_100FULL | LPA_100HALF)) {
			maccr |= FTGMAC030_MACCR_SPEED_100;

			if (common_adv & LPA_100FULL)
				maccr |= FTGMAC030_MACCR_FULLDUP;
		} else {
			maccr |= FTGMAC030_MACCR_SPEED_10;
			if (common_adv & LPA_10FULL)
				maccr |= FTGMAC030_MACCR_FULLDUP;
		}
	} else {
		int bmcr = ctrl->phyread(ctrl, MII_BMCR);

		if (bmcr & BMCR_FULLDPLX)
			maccr |= FTGMAC030_MACCR_FULLDUP;

		if (bmcr & BMCR_SPEED1000)
			maccr |= FTGMAC030_MACCR_SPEED_1000;
		else if (bmcr & BMCR_SPEED100)
			maccr |= FTGMAC030_MACCR_SPEED_100;
		else
			maccr |= FTGMAC030_MACCR_SPEED_10;
	}
#else
	maccr = mac_regs->maccr;
	maccr &= ~(FTGMAC030_MACCR_SPEED_MASK);
	maccr |= (FTGMAC030_MACCR_FULLDUP | FTGMAC030_MACCR_SPEED_1000);
#endif
	mac_regs->maccr = maccr;

	return 0;
}

void generic_phy_auto_neo(FTGMAC030_Control *ctrl)
{
	/* kick off phy auto negotiation */
	uint16_t tmp;
	int i, retry = 32768;
	tmp = ctrl->phyread(ctrl, MII_BMCR);
	tmp |= (BMCR_ANENABLE | BMCR_ANRESTART);

	/* Don't isolate the PHY if we're negotiating */
	tmp &= ~BMCR_ISOLATE;

	ctrl->phywrite(ctrl, tmp, MII_BMCR);

	do {
		for (i = 0; i < 100; i++){};
		tmp = ctrl->phyread(ctrl, MII_BMSR);

		if (tmp & BMSR_ANEGCOMPLETE)
			break;
	} while (retry-- > 0) ;
}

void generic_phy_info(FTGMAC030_Control *ctrl)
{
	prints("PHY: generic\n");
}

void generic_clear_phy_int(FTGMAC030_Control *ctrl)
{

}

void generic_set_phy_int(FTGMAC030_Control *ctrl)
{

}

int generic_change_duplex_speed(FTGMAC030_Control *ctrl, DUPLEX duplex, SPEED speed)
{
	int adv;
	int lpa;
	uint16_t ret = 1;

	mac_regs->ier &= ~FTGMAC030_INT_PHYSTS_CHG;

	if (speed == _1000) {
		lpa = ctrl->phyread(ctrl, MII_STAT1000);

		adv = ctrl->phyread(ctrl, MII_CTRL1000);

		adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

		if (FULL == duplex)
			adv |= ADVERTISE_1000FULL;
		else
			adv |= ADVERTISE_1000HALF;

		if (!(lpa & (adv << 2))) {
			prints("partner doesnot support 1000 Full/Half duplex.\n");
			goto error;
		}

		ctrl->phywrite(ctrl, adv, MII_CTRL1000);
	} else {
		adv = ctrl->phyread(ctrl, MII_CTRL1000); //R9_MSCR
		adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF); //clear [8]_1000baseT, [9]_1000baseT_FDX
		ctrl->phywrite(ctrl, adv, MII_CTRL1000); //R9_MSCR

		lpa = ctrl->phyread(ctrl, MII_LPA);

		adv = ctrl->phyread(ctrl, MII_ADVERTISE);

		adv &= ~ADVERTISE_ALL;

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
		} else {
			prints("Argumnent value speed error!\n");
			goto error;
		}

		if (!(lpa & adv)) {
			prints("partner doesnot support configure "\
			       "speed duplex. adv %04x lpa %04x.\n", adv ,lpa);
			goto error;
		}
	
		ctrl->phywrite(ctrl, adv, MII_ADVERTISE);
	}

	/* Restart auto negotiation */
	generic_phy_auto_neo(ctrl);

	generic_phy_processing(ctrl);
	ret = 0;

error:
	mac_regs->ier |= FTGMAC030_INT_PHYSTS_CHG;

	return ret;
}

uint32_t generic_is_link_up(FTGMAC030_Control *ctrl)
{
	int status;

	/* Do a fake read */
	status = ctrl->phyread(ctrl, MII_BMSR);

	/* Read link and autonegotiation status */
	status = ctrl->phyread(ctrl, MII_BMSR);

	return (status & BMSR_LSTATUS);
}

void generic_show_link_status(FTGMAC030_Control *ctrl)
{
	int status;
	int adv;
	int lpa;
	int lpagb = 0;
	int common_adv;
	int common_adv_gb = 0;
	int retry = 256;


read_bmsr:
	/* Do a fake read */
	status = ctrl->phyread(ctrl, MII_BMSR);

	/* Read link and autonegotiation status */
	status = ctrl->phyread(ctrl, MII_BMSR);

	if ((status & BMSR_LSTATUS) == 0) {

		if (retry > 0) {
			retry--;
			udelay(1000);
			goto read_bmsr;
		}
		prints("??? Link status => FAIL\n");
		return;
	}

	lpa = ctrl->phyread(ctrl, MII_LPA);

	if (lpa & LPA_LPACK) {
		if (status & BMSR_ESTATEN) {
			status = ctrl->phyread(ctrl, MII_ESTATUS);
			if (status & (ESTATUS_1000_TFULL | ESTATUS_1000_THALF)) {

				lpagb = ctrl->phyread(ctrl, MII_STAT1000);

				adv = ctrl->phyread(ctrl, MII_CTRL1000);

				common_adv_gb = lpagb & adv << 2;
			}
		}

		lpa = ctrl->phyread(ctrl, MII_LPA);

		adv = ctrl->phyread(ctrl, MII_ADVERTISE);

		common_adv = lpa & adv;

		if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)) {
			prints("SPEED-LPA: 1000Mbps ");
			if (common_adv_gb & LPA_1000FULL)
				prints("Full-duplex\n");
			else
				prints("Half-duplex\n");
		} else if (common_adv & (LPA_100FULL | LPA_100HALF)) {
			prints("SPEED-LPA: 100Mbps ");
			if (common_adv & LPA_100FULL)
				prints("Full-duplex\n");
			else
				prints("Half-duplex\n");
		} else {
			prints("SPEED-LPA: 10Mbps ");
			if (common_adv & LPA_10FULL)
				prints("Full-duplex\n");
			else
				prints("Half-duplex\n");
		}
	} else {
		int bmcr = ctrl->phyread(ctrl, MII_BMCR);

		if (bmcr & BMCR_SPEED1000)
			prints("SPEED: 1000Mbps ");
		else if (bmcr & BMCR_SPEED100)
			prints("SPEED: 100Mbps ");
		else
			prints("SPEED: 10Mbps ");

		if (bmcr & BMCR_FULLDPLX)
			prints("FULL-duplex\n");
		else 
			prints("Half-duplex\n");
	}
}
