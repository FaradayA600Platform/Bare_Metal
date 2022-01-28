/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_sd.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2019/12	Sanjin Liu	 Original FTSDC021 initial code for SD card
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"

void ftsdc021_sd_show_cid(void)
{
	uint64_t cid_lo, cid_hi;

	cid_lo = cardInfo.CID_LO;
	cid_hi = cardInfo.CID_HI;

	printf("**************** CID register ****************\r\n");
	printf("Manufacturer ID: 0x%02x\r\n", cid_hi >> 48);
	printf("OEM / App ID: 0x%04x\r\n", (cid_hi >> 32) & 0xFFFF);
	printf("Product Name: 0x%02x\r\n", ((cid_hi & 0xFFFFFFFF) << 8) |
		    (cid_lo >> 56) & 0xFF);
	printf("Product Revision: %d.%d\r\n", (cid_lo >> 52) & 0xF,
		    (cid_lo >> 48) & 0xF);
	printf("Product Serial No.: %u\r\n", (cid_lo >> 16) & 0xFFFFFFFF);
	printf("Reserved:0x%x\r\n", (cid_lo & 0xF000) >> 12);
	printf("Manufacturer Date: %d - %0.2d\r\n",
		    ((cid_lo & 0xFF0) >> 4) + 2000, (cid_lo & 0xF));
}

static const uint32_t tran_exp[] = {
	10000, 100000, 1000000, 10000000,
	0, 0, 0, 0
};

static const uint8_t tran_mant[] = {
	0, 10, 12, 13, 15, 20, 25, 30,
	35, 40, 45, 50, 55, 60, 70, 80,
};

void ftsdc021_sd_decode_csd(void)
{
	uint32_t e, m;
	uint64_t csd_lo, csd_hi;
	CSD_v1 *csd_v1 = &cardInfo.CSD_ver_1;
	CSD_v2 *csd_v2 = &cardInfo.CSD_ver_2;

	csd_lo = cardInfo.CSD_LO;
	csd_hi = cardInfo.CSD_HI;

	printf("**************** CSD register ****************\r\n");
	if ((csd_hi >> 54) == 0) {
		csd_v1->CSD_STRUCTURE = (csd_hi >> 54) & 0x3;
		csd_v1->Reserved1 = (csd_hi >> 48) & 0x3F;
		csd_v1->TAAC = (csd_hi >> 40) & 0xFF;
		csd_v1->NSAC = (csd_hi >> 32) & 0xFF;
		csd_v1->TRAN_SPEED = (csd_hi >> 24) & 0xFF;
		csd_v1->CCC = (csd_hi >> 12) & 0xFFF;
		csd_v1->READ_BL_LEN = (csd_hi >> 8) & 0xF;
		csd_v1->READ_BL_PARTIAL = (csd_hi >> 7) & 0x1;
		csd_v1->WRITE_BLK_MISALIGN = (csd_hi >> 6) & 0x1;
		csd_v1->READ_BLK_MISALIGN = (csd_hi >> 5) & 0x1;
		csd_v1->DSR_IMP = (csd_hi >> 4) & 0x1;
		csd_v1->Reserved2 = (csd_hi >> 2) & 0x3;
		csd_v1->C_SIZE = (((csd_hi & 0x3) << 10) | ((csd_lo >> 54) & 0x3FF));
		csd_v1->VDD_R_CURR_MIN = (csd_lo >> 51) & 0x7;
		csd_v1->VDD_R_CURR_MAX = (csd_lo >> 48) & 0x7;
		csd_v1->VDD_W_CURR_MIN = (csd_lo >> 45) & 0x7;
		csd_v1->VDD_W_CURR_MAX = (csd_lo >> 42) & 0x7;
		csd_v1->C_SIZE_MULT = (csd_lo >> 39) & 0x7;
		csd_v1->ERASE_BLK_EN = (csd_lo >> 38) & 0x1;
		csd_v1->SECTOR_SIZE = (csd_lo >> 31) & 0x7F;
		csd_v1->WP_GRP_SIZE = (csd_lo >> 24) & 0x7F;
		csd_v1->WP_GRP_ENABLE = (csd_lo >> 23) & 0x1;
		csd_v1->Reserved3 = (csd_lo >> 21) & 0x3;
		csd_v1->R2W_FACTOR = (csd_lo >> 18) & 0x7;
		csd_v1->WRITE_BL_LEN = (csd_lo >> 14) & 0xF;
		csd_v1->WRITE_BL_PARTIAL = (csd_lo >> 13) & 0x1;
		csd_v1->Reserved4 = (csd_lo >> 8) & 0x1F;
		csd_v1->FILE_FORMAT_GRP = (csd_lo >> 7) & 0x1;
		csd_v1->COPY = (csd_lo >> 6) & 0x1;
		csd_v1->PERM_WRITE_PROTECT = (csd_lo >> 5) & 0x1;
		csd_v1->TMP_WRITE_PROTECT = (csd_lo >> 4) & 0x1;
		csd_v1->FILE_FORMAT = (csd_lo >> 2) & 0x3;
		csd_v1->Reserver5 = (csd_lo) & 0x3;

		cardInfo.numOfBlocks = (cardInfo.CSD_ver_1.C_SIZE + 1) <<
                                       (cardInfo.CSD_ver_1.C_SIZE_MULT + 2);

		printf(" CSD Ver 1.0 Block num: %d,"
			    " Read Block Length: %d bytes,"
			    " Write " "Block Length: %d bytes,"
			    " Capacity: 0x%llx Bytes.\r\n", cardInfo.numOfBlocks,
			    (1 << rd_bl_len), (1 << wr_bl_len),
			    (uint64_t) (cardInfo.numOfBlocks * (1 << rd_bl_len)));

		e = cardInfo.CSD_ver_1.TRAN_SPEED & 7;
		m = (cardInfo.CSD_ver_1.TRAN_SPEED >> 3) & 0xF;

		cardInfo.block_addr = 0;
	} else if ((csd_hi >> 54) == 1) {
		csd_v2->CSD_STRUCTURE = (csd_hi >> 54) & 0x3;
		csd_v2->Reserved1 = (csd_hi >> 48) & 0x3F;
		csd_v2->TAAC = (csd_hi >> 40) & 0xFF;
		csd_v2->NSAC = (csd_hi >> 32) & 0xFF;
		csd_v2->TRAN_SPEED = (csd_hi >> 24) & 0xFF;
		csd_v2->CCC = (csd_hi >> 12) & 0xFFF;
		csd_v2->READ_BL_LEN = (csd_hi >> 8) & 0xF;
		csd_v2->READ_BL_PARTIAL = (csd_hi >> 7) & 0x1;
		csd_v2->WRITE_BLK_MISALIGN = (csd_hi >> 6) & 0x1;
		csd_v2->READ_BLK_MISALIGN = (csd_hi >> 5) & 0x1;
		csd_v2->DSR_IMP = (csd_hi >> 4) & 0x1;
		csd_v2->Reserved2 = (((csd_hi >> 2) & 0x3) << 2) | ((csd_lo >> 62) & 0x3);
		csd_v2->C_SIZE = ((csd_lo >> 40) & 0x3FFFFF);
		csd_v2->Reserved3 = (csd_lo >> 39) & 0x1;
		csd_v2->ERASE_BLK_EN = (csd_lo >> 38) & 0x1;
		csd_v2->SECTOR_SIZE = (csd_lo >> 31) & 0x7F;
		csd_v2->WP_GRP_SIZE = (csd_lo >> 24) & 0x7F;
		csd_v2->WP_GRP_ENABLE = (csd_lo >> 23) & 0x1;
		csd_v2->Reserved4 = (csd_lo >> 21) & 0x3;
		csd_v2->R2W_FACTOR = (csd_lo >> 18) & 0x7;
		csd_v2->WRITE_BL_LEN = (csd_lo >> 14) & 0xF;
		csd_v2->WRITE_BL_PARTIAL = (csd_lo >> 13) & 0x1;
		csd_v2->Reserved5 = (csd_lo >> 8) & 0x1F;
		csd_v2->FILE_FORMAT_GRP = (csd_lo >> 7) & 0x1;
		csd_v2->COPY = (csd_lo >> 6) & 0x1;
		csd_v2->PERM_WRITE_PROTECT = (csd_lo >> 5) & 0x1;
		csd_v2->TMP_WRITE_PROTECT = (csd_lo >> 4) & 0x1;
		csd_v2->FILE_FORMAT = (csd_lo >> 2) & 0x3;
		csd_v2->Reserver6 = (csd_lo) & 0x3;

		cardInfo.numOfBlocks = (cardInfo.CSD_ver_2.C_SIZE + 1) << 10;
		printf(" CSD Ver 2.0, Block num: %d,"
			    " Block Length: %d bytes,"
			    " Capacity: 0x%llx Bytes.\r\n", cardInfo.numOfBlocks,
			    (1 << rd_bl_len), (uint64_t)(cardInfo.numOfBlocks *
			    (1 << rd_bl_len)));

		e = cardInfo.CSD_ver_2.TRAN_SPEED & 7;
		m = (cardInfo.CSD_ver_2.TRAN_SPEED >> 3) & 0xF;

		cardInfo.block_addr = 1;
	} else {
		cardInfo.numOfBlocks = 0;
		printf(" Reserve.\r\n");
	}

	cardInfo.max_dtr = tran_exp[e] * tran_mant[m];
	printf(" Current Max Transfer Speed %d Hz.\r\n", cardInfo.max_dtr);
	/* Index start from zero */
	cardInfo.numOfBlocks -= 1;

	printf("**********************************************\r\n");
}

uint32_t ftsdc021_sd_read_scr(void)
{
	memset(&cardInfo.SCR, 0, sizeof(SD_SCR));
	if (ftsdc021_ops_app_send_scr(&cardInfo)) {
		printf(" ERR## ... SD: Get SCR from card is failed\r\n");
		return 1;
	}

	printf("**************** SCR register ****************\r\n");
	printf("SCR Structure: %d\r\n", cardInfo.SCR.SCR_STRUCTURE);
	printf("SCR Memory Card - Spec. Version: %d\r\n",
		    cardInfo.SCR.SD_SPEC);
	printf("Data status after erase: %d\r\n",
		    cardInfo.SCR.DATA_STAT_AFTER_ERASE);
	printf("SD Security Support: %d\r\n", cardInfo.SCR.SD_SECURITY);
	printf("DAT Bus widths supported: %d\r\n",
		    cardInfo.SCR.SD_BUS_WIDTHS);
	printf("CMD23 supported: %d\r\n", cardInfo.SCR.CMD23_SUPPORT);
	printf("CMD20 supported: %d\r\n", cardInfo.SCR.CMD20_SUPPORT);

	printf("**********************************************\r\n");

	return 0;
} 

uint32_t ftsdc021_init_sd(uint32_t ocr)
{
	uint32_t rocr, err = 0;

	if (ftsdc021_uhs_mode_support())
		ocr |= (1 << 24);

reinit:
	if (ftsdc021_ops_go_idle_state(0)) {
		return 1;
	}

	/* CMD8 of SD card, F8 is only referenced by SD and SDIO flow */
	err = ftsdc021_ops_send_if_cond(&cardInfo, ((host.ocr_avail &
					0xFF8000) != 0) << 8 | 0xAA);
	if (!err)
		ocr |= (1 << 30);

	/* ACMD41 */
	err = ftsdc021_ops_send_app_op_cond(&cardInfo, ocr, &rocr);
	if (err) {
		printf("ERR## ... ACMD41!\r\n'");
		return err;
	}

	if (rocr & (1 << 30))
		printf("Found SD Memory Card (SDHC or SDXC).\r\n");
	else
		printf("Found SD Memory Card Version 1.X.\r\n");

	cardInfo.OCR = rocr;

	/* Accept IO Signal Voltage switch to 1.8v */
	if ((cardInfo.OCR & (1 << 24)) && cardInfo.signal_vol == 1) {
		err = ftsdc021_signal_voltage_switch();
		if (err) {
			printf("ERR## ... 1.8v Signal Enable Failed.\r\n");
			ocr &= ~(1 << 24);
			goto reinit;
		}
		printf("Switching to S18V IO level success.\r\n");
	} else {
		printf("SD/SDIO Card does not support S18V.\r\n");
	}

	err = ftsdc021_ops_all_send_cid(&cardInfo);
	if (err) {
		printf("ERR## ... Send CID(CMD2)!\r\n");
		return err;
	}
	ftsdc021_sd_show_cid();

	err = ftsdc021_ops_send_rca(&cardInfo);
	if (err) {
		printf("ERR## ... Send RCA(CMD3)!\r\n");
		return err;
	}

	err = ftsdc021_ops_send_csd(&cardInfo);
	if (err) {
		printf("ERR## ... Send CSD(CMD9) !\r\n");
		return err;
        }
        ftsdc021_sd_decode_csd();

	err = ftsdc021_ops_select_card(&cardInfo);
	if (err) {
		printf("ERR## ... Select Card(CMD7) !\r\n");
		return err;
	}

	err = ftsdc021_sd_read_scr();
	if (err) {
		printf("ERR## ... Send SCR!\r\n");
		return err;
	}

	err = ftsdc021_ops_send_card_status(&cardInfo);
	if (err) {
		printf("ERR## ... Get Card Status!\r\n");
		return err;
	}

	printf("Init done: Card Status --> %s\r\n",
		    SDC_ShowCardState((cardInfo.respLo >> 9) & 0xF));

	return err;
}

uint32_t ftsdc021_attach_sd(void)
{
	uint32_t err, ocr, rocr;
	/*
	 * SD ACMD41
	 */
	err = ftsdc021_ops_send_app_op_cond(&cardInfo, 0, &ocr);
	if (err)
		return err;

	cardInfo.CardType = MEMORY_CARD_TYPE_SD;

	rocr = ftsdc021_select_voltage(ocr);
        if (!rocr)
		return 1;

	/*
	 * Detect and init the card.
	 */
	err = ftsdc021_init_sd(rocr);

	return err;
}
