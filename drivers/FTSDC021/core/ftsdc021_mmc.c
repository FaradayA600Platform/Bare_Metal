/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_mmc.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2019/12	Sanjin Liu	 Original FTSDC021 initial code for MMC/eMMC
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"

void ftsdc021_mmc_show_cid(void)
{
	uint64_t cid_lo, cid_hi;

	cid_lo = cardInfo.CID_LO;
	cid_hi = cardInfo.CID_HI;
	/*
	 * 8 bits CRC are stripped in the response. If you reference the Linux
         * driver, the 8 bits CRC are added in Linux SDHCI driver.
         * Do not confuse that.
	 */
	printf("**************** CID register ****************\r\n");
	printf("Manufacturer ID: 0x%02x\r\n", cid_hi >> 48);
	printf("OEM / App ID: 0x%04x\r\n", (cid_hi >> 32) & 0xFFFF);
	printf("Product Name: 0x%02x\r\n", (((cid_hi) & 0xFFFFFFFF) << 16) |
		    (cid_lo >> 48) & 0xFFFF);
	printf("Product Revision: %d.%d\r\n", (cid_lo >> 44) & 0xF,
		    (cid_lo >> 40) & 0xF);
	printf("Product Serial No.: %u\r\n", (cid_lo >> 8) & 0xFFFFFFFF);
	printf("Manufacturer Date: %d - %0.2d\r\n",
		    ((cid_lo & 0xF) + 1997), (cid_lo & 0xF0) >> 4);
}

void ftsdc021_mmc_decode_csd(void)
{
	uint64_t csd_lo, csd_hi;
	MMC_CSD *csd = &cardInfo.CSD_MMC;

	csd_lo = cardInfo.CSD_LO;
	csd_hi = cardInfo.CSD_HI;
	csd->CSD_STRUCTURE = (csd_hi >> 54) & 0x3;
	csd->SPEC_VERS = (csd_hi >> 50) & 0xF;
	csd->Reserved1 = (csd_hi >> 48) & 0x3;
	csd->TAAC = (csd_hi >> 40) & 0xFF;
	csd->NSAC = (csd_hi >> 32) & 0xFF;
	csd->TRAN_SPEED = (csd_hi >> 24) & 0xFF;
	csd->CCC = (csd_hi >> 12) & 0xFFF;
	csd->READ_BL_LEN = (csd_hi >> 8) & 0xF;
	csd->READ_BL_PARTIAL = (csd_hi >> 7) & 0x1;
	csd->WRITE_BLK_MISALIGN = (csd_hi >> 6) & 0x1;
	csd->READ_BLK_MISALIGN = (csd_hi >> 5) & 0x1;
	csd->DSR_IMP = (csd_hi >> 4) & 0x1;
	csd->Reserved2 = (csd_hi >> 2) & 0x3;
	csd->C_SIZE = (((csd_hi & 0x3) << 10) | (csd_lo >> 54) & 0x3FF);
	csd->VDD_R_CURR_MIN = (csd_lo >> 51) & 0x7;
	csd->VDD_R_CURR_MAX = (csd_lo >> 48) & 0x7;
	csd->VDD_W_CURR_MIN = (csd_lo >> 45) & 0x7;
	csd->VDD_W_CURR_MAX = (csd_lo >> 42) & 0x7;
	csd->C_SIZE_MULT = (csd_lo >> 39) & 0x7;
	csd->ERASE_GRP_SIZE = (csd_lo >> 34) & 0x1F;
	csd->ERASE_GRP_MULT = (csd_lo >> 29) & 0x1F;
	csd->WP_GRP_SIZE = (csd_lo >> 24) & 0x1F;
	csd->WP_GRP_ENABLE = (csd_lo >> 23) & 0x1;
	csd->DEFAULT_ECC = (csd_lo >> 21) & 0x3;
	csd->R2W_FACTOR = (csd_lo >> 18) & 0x7;
	csd->WRITE_BL_LEN = (csd_lo >> 14) & 0xF;
	csd->WRITE_BL_PARTIAL = (csd_lo >> 13) & 0x1;
	csd->Reserved3 = (csd_lo >> 9) & 0xF;
	csd->CONTENT_PROT_APP = (csd_lo >> 8) & 0x1;
	csd->FILE_FORMAT_GRP = (csd_lo >> 7) & 0x1;
	csd->COPY = (csd_lo >> 6) & 0x1;
	csd->PERM_WRITE_PROTECT = (csd_lo >> 5) & 0x1;
	csd->TMP_WRITE_PROTECT = (csd_lo >> 4) & 0x1;
	csd->FILE_FORMAT = (csd_lo >> 2) & 0x3;
	csd->ECC = (csd_lo) & 0x3;

	printf("**************** CSD register ****************\r\n");
	printf("C_SIZE %d, CSIZE_MULT %d, RD_BL_LEN %d.\r\n",
		    cardInfo.CSD_MMC.C_SIZE, cardInfo.CSD_MMC.C_SIZE_MULT,
		    cardInfo.CSD_MMC.READ_BL_LEN);
	printf("**********************************************\r\n");
}

uint32_t ftsdc021_mmc_read_ext_csd(void)
{
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card failed\r\n");
		goto fail;
	}

	printf("**************** Extended CSD register ***************\r\n");
	if (cardInfo.EXT_CSD_MMC.CARDTYPE & 0xE)
		cardInfo.max_dtr = 52000000;
	else if (cardInfo.EXT_CSD_MMC.CARDTYPE & 1)
		cardInfo.max_dtr = 26000000;
	else
		printf(" WARN## ...  Unknown Max Speed at EXT-CSD.\r\n");

	printf("Ext-CSD:  Max Speed %d Hz.\r\n", cardInfo.max_dtr);

	if (cardInfo.CardType == MEMORY_eMMC) {
		printf(" Size of MMC_EXT_CSD = %d\r\n",
			    sizeof(cardInfo.EXT_CSD_MMC));
		printf(" HPI_FEATURES[503]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.HPI_FEATURES);
		printf(" BKOPS_SUPPORT[502]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.BKOPS_SUPPORT);
		printf(" MAX_PACKED_READS[501]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.MAX_PACKED_READS);
		printf(" MAX_PACKED_WRITES[500]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.MAX_PACKED_WRITES);
		printf(" DATA_TAG_SUPPORT[499]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.DATA_TAG_SUPPORT);
		printf(" CMDQ_SUPPORT[308]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.CMDQ_SUPPORT);
		printf(" CMDQ_DEPTH[307]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.CMDQ_DEPTH);
		printf(" POWER_OFF_LONG_TIME[247]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.POWER_OFF_LONG_TIME);
		printf(" BOOT_INFO[228]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.BOOT_INFO);
		/* Some eMMC occupy byte 227 as 2nd byte for BOOT_SIZE_MULTI */
		printf(" BOOT_SIZE_MULT[226]:0x%x.\r\n",
			    (cardInfo.EXT_CSD_MMC.Reserved6 << 8 |
			    cardInfo.EXT_CSD_MMC.BOOT_SIZE_MULTI));
		printf(" SLEEP_NOTIFICATION_TIME[216]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.SLEEP_NOTIFICATION_TIME);
		printf(" SEC_COUNT[215-212]: 0x%08x.\r\n",
			    cardInfo.EXT_CSD_MMC.SEC_COUNT);
		printf(" HS_TIMING[185]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.HS_TIMING);
		printf(" BUS_WIDTH[183]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.BUS_WIDTH);
		printf(" PARTITION_CONF[179]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.PARTITION_CONF);
		printf(" BOOT_CONFIG_PROT[178]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.BOOT_CONFIG_PROT);
		printf(" BOOT_BUS_WIDTH[177]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.BOOT_BUS_WIDTH);
		printf(" ERASE_GROUP_DEF[175]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.ERASE_GROUP_DEF);
		printf(" BOOT_BUS_WP[173]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.BOOT_WP);
		printf(" USER_WP[171]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.USER_WP);
		printf(" BKOPS_EN[163]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.BKOPS_EN);
		printf(" RST_n_FUNCTION[162]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.RST_n_FUNCTION);
		printf(" HPI_MGMT[161]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.HPI_MGMT);
		printf(" DATA_SECTOR_SIZE[61]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.DATA_SECTOR_SIZE);
		printf(" CLASS_6_CTRL[59]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.CLASS_6_CTRL);
		printf(" DYNCAP_NEEDED[58]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.DYNCAP_NEEDED);
		printf(" EXCEPTION_EVENTS_STATUS[55]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_STATUS[1]);
		printf(" EXCEPTION_EVENTS_STATUS[54]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_STATUS[0]);
		printf(" PACKED_COMMAND_STATUS[36]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.PACKED_COMMAND_STATUS);
		printf(" PACKED_FAILURE_INDEX[35]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.PACKED_FAILURE_INDEX);
		printf(" POWER_OFF_NOTIFICATION[34]:0x%x\r\n",
			    cardInfo.EXT_CSD_MMC.POWER_OFF_NOTIFICATION);
		printf(" TMP_WRITE_PROTECT: 0x%x.\r\n",
			    cardInfo.CSD_MMC.TMP_WRITE_PROTECT);
		printf(" WP_GRP_SIZE: 0x%x.\r\n",
			    cardInfo.CSD_MMC.WP_GRP_SIZE);
		printf(" WP_GRP_ENABLE: 0x%x.\r\n",
			    cardInfo.CSD_MMC.WP_GRP_ENABLE);
		printf(" PARTITION_SETTING_COMPLETED[155]: 0x%x.\r\n",
			    cardInfo.EXT_CSD_MMC.PARTITION_SETTING_COMPLETED);

		cardInfo.numOfBootBlocks =
			(cardInfo.EXT_CSD_MMC.Reserved6 << 8 |
			cardInfo.EXT_CSD_MMC.BOOT_SIZE_MULTI) * 128 * 1024;
		cardInfo.numOfBootBlocks >>= 9;

		if (cardInfo.EXT_CSD_MMC.HPI_MGMT) {
			cardInfo.hpi_en = 1;
			if (cardInfo.EXT_CSD_MMC.HPI_FEATURES &
			    EXT_CSD_HPI_IMPLEMENTATION)
				cardInfo.hpi_cmd = SDHCI_STOP_TRANS;
			else
				cardInfo.hpi_cmd = SDHCI_SEND_STATUS;
		}

		if (cardInfo.EXT_CSD_MMC.BKOPS_EN)
			cardInfo.bkops_en = 1;
	}

	printf("**********************************************\r\n");

fail:
	/* The block number of MMC card, which capacity is more than 2GB, shall
	* be fetched from Ext-CSD. */
	if (cardInfo.CSD_MMC.CSD_STRUCTURE == 3) {
		printf(" MMC EXT CSD Version 1.%d\r\n",
			    cardInfo.EXT_CSD_MMC.EXT_CSD_REV);
		printf(" MMC CSD Version 1.%d\r\n",
			    cardInfo.EXT_CSD_MMC.CSD_STRUCTURE);
	} else {
		printf("MMC CSD Version 1.%d\r\n",
			    cardInfo.CSD_MMC.CSD_STRUCTURE);
	}

	if ((cardInfo.EXT_CSD_MMC.PARTITION_CONF & 7) == 3) {
		cardInfo.numOfBlocks = cardInfo.EXT_CSD_MMC.RPMB_SIZE_MULT *
				       (128 * 1024 / 256);
		cardInfo.block_addr = 0;
		cardInfo.RPMB_access = 1;
	} else if (cardInfo.CSD_MMC.C_SIZE == 0xFFF) {
		cardInfo.numOfBlocks = cardInfo.EXT_CSD_MMC.SEC_COUNT;
		cardInfo.block_addr = 1;
		cardInfo.RPMB_access = 0;
	}
	else {
		cardInfo.numOfBlocks = (cardInfo.CSD_MMC.C_SIZE + 1) <<
				       (cardInfo.CSD_MMC.C_SIZE_MULT + 2);
		/* Change to 512 bytes unit */
		cardInfo.numOfBlocks = cardInfo.numOfBlocks <<
				       (cardInfo.CSD_MMC.READ_BL_LEN - 9);
		cardInfo.block_addr = 0;
		cardInfo.RPMB_access = 0;
	}

	/* Index start from zero */
	cardInfo.numOfBlocks -= 1;

	printf("Max blocks: %d\r\n", cardInfo.numOfBlocks);
	return 0;
}

uint32_t ftsdc021_mmc_set_bus_speed(SDCardInfo *card, uint8_t speed)
{
	if (card->CSD_MMC.SPEC_VERS < 4 && speed != 0) {
		printf(" MMC: Change speed isn't allowed for version less"
			    " than MMC4.1.\r\n");
		printf(" MMC: Force the bus speed to be default speed.\r\n");
		return 1;
	}

	//Check card if support hs200?
	if (speed == 3) {
		printf(" ERR## ... HS400 is not supported.\r\n");
		return 1;
	}

	if (speed == 2 && (card->EXT_CSD_MMC.CARDTYPE & 0x10) == 0) {
		printf(" ERR## ... The device doesn't support HS200.\r\n");
		return 1;
	}

	if (card->CSD_MMC.SPEC_VERS >= 4) {
		if (ftsdc021_mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_HS_TIMING, speed, 1)) {
			printf(" ERR## ... MMC: Problem switching card"
				    " into %d mode!\r\n", speed);
			return 1;
		}
		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) {
			printf(" ERR## ... Card can't return to transfer"
				    " state\r\n");
			return 1;
		}
		printf(" MMC: Card is in transfer state.\r\n");
	}
	return 0;
}

uint32_t ftsdc021_init_mmc(uint32_t ocr)
{
	uint32_t rocr, err = 0;

	err = ftsdc021_ops_go_idle_state(0);
	if (err) {
		printf("ERR## ... Go idle state(CMD0) !\n'");
		return err;
	}

	/* For some eMMC device, 1.8V signal voltage must be switched first. */
	if (cardInfo.CardType == MEMORY_eMMC) {
		err = ftsdc021_set_1v8_signal_voltage();
		if (err) {
			printf("ERR## ... 1.8v Signal Enable Failed.\r\n");
			return err;
		}
	}

	/* Bit 30 indicate Host support high capacity */
	ocr |= (1 << 30);
	err = ftsdc021_mmc_send_op_cond(&cardInfo, ocr, &rocr);
	if (err) {
		printf("ERR## ... Send OP COND(CMD1) !\r\n'");
		return err;
	}
	cardInfo.OCR = rocr;

	err = ftsdc021_ops_all_send_cid(&cardInfo);
	if (err) {
		printf("ERR## ... Send CID(CMD2) !\r\n'");
		return err;
	}
	ftsdc021_mmc_show_cid();

	cardInfo.RCA = 1;
	err = ftsdc021_mmc_set_rca(&cardInfo);
	if (err) {
		printf("ERR## ... Set RCA(CMD3) !\r\n'");
		return err;
	}

	err = ftsdc021_ops_send_csd(&cardInfo);
	if (err) {
		printf("ERR## ... Send CSD(CMD9) !\r\n");
		return err;
	}
	ftsdc021_mmc_decode_csd();

	err = ftsdc021_ops_send_card_status(&cardInfo);
	if (err) {
		printf("ERR## ... Get Card Status) !\r\n");
		return err;
	}

	/* The following checks whether the card state is in standby or not */
	if (((cardInfo.respLo >> 9) & 0xF) == CUR_STATE_STBY) {
		err = ftsdc021_ops_select_card(&cardInfo);
		if (err) {
			printf("ERR## ... Select Card(CMD7) !\r\n");
			return err;
		}
		cardInfo.card_init = TRUE;
	} else {
		printf("MMC init failed ... State is not stand-by\r\n");
		return 1;
	}

	err = ftsdc021_mmc_read_ext_csd();
	if (err) {
		printf("ERR## ... Get EXT-CSD) !\r\n");
		return err;
	}

	return err;
}

uint32_t ftsdc021_attach_mmc(void)
{
	uint32_t err, ocr, rocr;
	/*
	 * MMC CMD1
	 */
	err = ftsdc021_mmc_send_op_cond(&cardInfo, 0, &ocr);
	if (err)
		return err;

#if defined(EMBEDDED_MMC)
	cardInfo.CardType = MEMORY_eMMC;
#else
	cardInfo.CardType = MEMORY_CARD_TYPE_MMC;
#endif

	rocr = ftsdc021_select_voltage(ocr);
	if (!rocr)
		return 1;

	/*
	 * Detect and init the card.
	 */
	err = ftsdc021_init_mmc(rocr);

	return err;

}

/*For eMMC partition setting */
uint32_t ftsdc021_mmc_set_partition_access(uint8_t partition)
{
	uint8_t val;

	if ((cardInfo.EXT_CSD_MMC.PARTITIONING_SUPPORT & 0x1) == 0) {
		printf(" ERR## ... Partition is not supported.\r\n");
		return 1;
	}

	cardInfo.EXT_CSD_MMC.PARTITION_CONF &= ~0x7;
	val = cardInfo.EXT_CSD_MMC.PARTITION_CONF | partition;
	
	if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_PARTITION_CONF, val, 1)) {
		printf(" ERR## ... MMC: Set Partition Config failed.\r\n");
		return 1;
	}	
	
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "failed\r\n");
		return 1;
	}

	if ( (cardInfo.EXT_CSD_MMC.PARTITION_CONF & 7) != partition) {
		printf(" ERR## ... Value not match (%d, %d).\r\n",
			    cardInfo.EXT_CSD_MMC.PARTITION_CONF, val);
		return 1;	
	}

	return 0;
}

/* Setup eMMC boot partition */
uint32_t ftsdc021_mmc_set_bootmode(uint8_t partition)
{
	uint8_t val;

	if ((cardInfo.EXT_CSD_MMC.PARTITIONING_SUPPORT & 0x1) == 0) {
		printf(" ERR## ... Partition is not supported.\r\n");
		return 1;
	}

	cardInfo.EXT_CSD_MMC.PARTITION_CONF &= ~(0x7 << 3);
	val = cardInfo.EXT_CSD_MMC.PARTITION_CONF | (partition << 3);
	
	if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_PARTITION_CONF, val, 1)) {
		printf(" ERR## ... MMC: Set Partition Config failed.\r\n");
		return 1;
	}	
	
	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "failed\r\n");
		return 1;
	}

	if ( ((cardInfo.EXT_CSD_MMC.PARTITION_CONF >> 3) & 7) != partition) {
		printf(" ERR## ... Value not match (%d, %d).\r\n",
			    cardInfo.EXT_CSD_MMC.PARTITION_CONF, val);
		return 1;	
	}

	return 0;
}

/* If sending eMMC boot acknowledge is needed, set ack to 1*/
uint32_t ftsdc021_mmc_set_bootack(uint8_t ack)
{
	uint8_t val;

	if (cardInfo.CardType != MEMORY_eMMC) {
		printf(" ERR## ... The inserted card isn't embedded "
			    "MMC.\r\n");
		return 1;
	}

	cardInfo.EXT_CSD_MMC.PARTITION_CONF &= ~(0x1 << 6);
	val = cardInfo.EXT_CSD_MMC.PARTITION_CONF | (ack << 6);

	if (ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_PARTITION_CONF, val, 1)) {
		printf(" ERR## ... MMC: Set boot ack Config failed.\r\n");
		return 1;
	}

	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card is "
			    "failed\r\n");
		return 1;
	}

	if ( ((cardInfo.EXT_CSD_MMC.PARTITION_CONF >> 6) & 1) != ack) {
		printf(" ERR## ... Value not match (%d, %d).\r\n",
			    cardInfo.EXT_CSD_MMC.PARTITION_CONF, val);
		return 1;
	}

	return 0;
}

/**
 * Simple version of transfer data when entering Boot Mode.
 * Never has Transfer Complete Interrupt, HW always assume Infinite Transfer.
 * Not Support ADMA for Boot Mode.
 * The unit of length is bytes.
 */
uint32_t ftsdc021_read_boot_data(uint32_t *buffer, uint32_t length)
{
	uint32_t trans_sz, len, wait_t;
	clock_t t0;

	cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;

	if (cardInfo.FlowSet.UseDMA == PIO) {
		wait_t = timeout_ms;
		trans_sz = cardInfo.fifo_depth;

		while (length) {
			t0 = clock();
			while (!(gpReg->IntrSts &
				 SDHCI_INTR_STS_BUFF_READ_READY)) {
				if (clock() - t0 > wait_t) {
					printf(" ERR## ... Wait Buffer "
						    "Read Ready timeout "
						    "(%d).\r\n", length);
					goto out;
				}
			}

			/* Clear Interrupt status */
			gpReg->IntrSts = SDHCI_INTR_STS_BUFF_READ_READY;

			if (!length)
				break;

			len = (length < trans_sz) ? length : trans_sz;
			length -= len;

			while (len) {
				*buffer = *((volatile uint32_t *)&gpReg->BufData);
				len -= 4;
				buffer++;
			}
		}

	} else if (cardInfo.FlowSet.UseDMA == SDMA) {
		uint32_t next_addr;

		trans_sz = sdma_bound_mask + 1;
		wait_t = timeout_ms * (trans_sz >> 9);

		do {
			t0 = clock();
			/*
			 * Make sure SDMA finish before we lacth the next
			 * address
			 */
			while (cardInfo.cmplMask) {
				if (cardInfo.ErrorSts)
					goto out;
				if (clock() - t0 > wait_t) {
					printf(" ERR## ... Wait SDMA "
						    "interrupt timeout (%d).\r\n",
						    length);
					goto out;
				}
			}

			next_addr = gpReg->SdmaAddr;
			/* Transfered bytes count */
			len = next_addr - (uint32_t)((uintptr_t)buffer);
			/*
			 * Minus the total desired bytes count. SDMA stops at
			 * boundary but it might already exceed our intended
			 * bytes
			 */
			if ((int32_t) (length - len) < 0)
				length = 0;
			else
				length -= len;

			if (!length)
				break;

			/* Boundary Checking */
			if (next_addr & sdma_bound_mask) {
				printf(" ERR## ... SDMA interrupt not at "
					    "%d boundary, addr=0x%08x.\r\n",
					    (sdma_bound_mask + 1), next_addr);
				return 1;
			} else {
				if (dbg_print > 1)
					printf(" SDMA interrupt at "
						    "addr=0x%08x.\r\n",
						    next_addr);
			}

			cardInfo.cmplMask = WAIT_DMA_INTR;
			buffer = (uint32_t *)((uintptr_t)next_addr);
			gpReg->SdmaAddr = (uint32_t)((uintptr_t)buffer);

		} while (1);

	} else {
		printf(" ERR## ... Boot Mode does not supprt ADMA.\r\n");
	}

out:
	if (cardInfo.ErrorSts) {
		ftsdc021_ErrorRecovery();
		return 1;
	}
	return length;
}

uint32_t ftsdc021_mmc_boot(uint8_t boot_mode, uint8_t ack, uint8_t boot_spd,
			 uint8_t boot_bw, uint32_t *boot_addr, uint32_t boot_blocks)
{
	/* Make sure we are in Normal Mode */
	gpReg->VendorReg1 &= ~0x3;

	/* Be careful, this code assume EXT_CSD already read before scan cards
	 * in boot mode. Do scan cards in common way first.
	 * Otherwise: - Must know the number of bytes at boot partition.
	 * 	      - Whether support Alternative Boot Mode.
	 */
#if 1 /* Does not work with Samsung eMMC KLMXGXXEHX */
	/* CMD0 with argument 0xF0F0F0F0 */
	if (ftsdc021_ops_go_idle_state(0xF0F0F0F0)) {
		printf(" ERR## ... Go to pre-idle state failed.\r\n");
		return 1;
	}
#endif

	gpReg->HCReg &= ~(SDHCI_HC_BUS_WIDTH_8BIT | SDHCI_HC_BUS_WIDTH_4BIT);
	switch(boot_bw) {
	case 0: /* 1 bit */
		break;
	case 1: /* 4 bits */
		gpReg->HCReg |= SDHCI_HC_BUS_WIDTH_4BIT;
		break;
	case 2: /* 8 bits */
		gpReg->HCReg |= SDHCI_HC_BUS_WIDTH_8BIT;
		break;
	default:
		printf(" ERR## ... Unknown Boot Bus Width.\r\n");
		return 1;
	}
		
	switch (boot_spd) {
	case 0:
		gpReg->HCReg &= ~(uint8_t) 0x4;
		ftsdc021_SetSDClock(25000000);
		break;
	case 1:
		gpReg->HCReg |= 0x4;
		ftsdc021_SetSDClock(50000000);
		break;
	default:
		printf(" ERR## ... Unknown Boot Bus speed.\r\n");
		return 1;
	}


	/* Prepare to receive data from Slave, use infinite transfer */
	ftsdc021_set_transfer_mode(0, 0, SDHCI_TXMODE_READ_DIRECTION, 1);
	if (ftsdc021_prepare_data(boot_blocks, 0x200, boot_addr, READ)) {
		printf(" ERR## ... Boot Mode: Prepare Data failed.\r\n");
		return 1;
	}

	/* Boot Ack enabled */
	if (ack) 
		gpReg->VendorReg1 |= MMC_BOOT_ACK;
	else
		gpReg->VendorReg1 &= ~MMC_BOOT_ACK;

	
	/* Assume boot_mode == 2 is Alternative Boot Mode, send CMD0 with 
	 * arg = 0xFFffFFfa. Must check if device also support.
	 */
	if (boot_mode == 2) {
		gpReg->VendorReg1 |= MMC_ALTERNATIVE_BOOT_MODE;
		/* CMD0 with argument 0xFFFFFFFA */
		if (ftsdc021_ops_go_idle_state(0xFFFFFFFA)) {
			printf(" ERR## ... Initiate alternative boot "
				    "operation failed.\r\n");
			return 1;
		}
	} else {	
		gpReg->VendorReg1 |= MMC_BOOT_MODE;
	}

	if (ftsdc021_read_boot_data((uint32_t *) boot_addr, (boot_blocks << 9))) {
		printf(" ERR## ... Boot Mode: Read data failed.\r\n");
		return 1;
	}

	/*Boot Mode done, enter normal storage mode. */
	gpReg->VendorReg1 &= ~(MMC_BOOT_ACK | 0x3);
	/* Reset the controller */
	gpReg->SoftRst = SDHCI_SOFTRST_ALL;

	return 0;
}

/* Send eMMC HPI command */
uint32_t ftsdc021_mmc_hpi(void)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	if (!cardInfo.hpi_en) {
		printf(" MMC: HPI is not enabled.\r\n");
                return 1;
        }

	//Check device state
	if (ftsdc021_ops_send_card_status(&cardInfo)) {
		printf("ERR## Get Card Status Failed for sending HPI "
			    "command!\r\n");
		return 1;
	}

        switch (SD_STATUS_CURRENT_STATE(cardInfo.respLo)) {
		case CUR_STATE_IDLE:
		case CUR_STATE_READY:
		case CUR_STATE_STBY:
		case CUR_STATE_TRAN:
			/*
			 * In idle and transfer states, HPI is not needed and
			 * the caller can issue the next intended command
			 * immediately
			 */
			err = 0;
			break;
		case CUR_STATE_PRG:
			printf("Send HPI command ... ");
			ftsdc021_mmc_send_hpi(&cardInfo);
			printf("done!\r\n");
			break;
		default:
			/* In all other states, it's illegal to issue HPI */
			printf("HPI cannot be sent. Card state=%d\r\n",
				    SD_STATUS_CURRENT_STATE(cardInfo.respLo));
			err = 1;
        }
        return err;
}

/* For eMMC BKOPS function */
uint32_t ftsdc021_mmc_start_bkops(void)
{
	uint32_t err;

	/* If BKOPS is not enabled, do nothing */
        if (!cardInfo.bkops_en) {
		printf(" MMC: BKOPS_START is not enabled\r\n");
		return 1;
	}

        if (!cardInfo.EXT_CSD_MMC.BKOPS_STATUS) {
		printf(" MMC: BKOPS_START is not needed, status=%x\r\n",
			    cardInfo.EXT_CSD_MMC.BKOPS_STATUS);
		return 1;
	} else {
		printf(" MMC: BKOPS_START is needed, status=%x\r\n",
			    cardInfo.EXT_CSD_MMC.BKOPS_STATUS);
	}

	err = ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL,
				      EXT_CSD_BKOPS_START, 1, 0);
	if (err) {
		printf(" ERR## ... MMC: Starting BKOPS is failed\r\n");
		return err;
	} else {
		printf(" MMC: BKOPS_START is done, wait card state.\r\n");
		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 50000)) {
			printf(" ERR## ... Card can't return to transfer "
				    "state\r\n");
			return 1;
		} else {
			printf(" MMC: Card is in transfer state.\r\n");
		}
	}

	return 0;
}

/* Check response R1 exception for eMMC*/
uint32_t ftsdc021_mmc_r1_exception(void)
{
        if (cardInfo.CardType != MEMORY_eMMC || !cardInfo.card_init) {
		return 0;
	}

	if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000))
		return 1;

	if (ftsdc021_mmc_send_ext_csd(&cardInfo)) {
		printf(" ERR## ... MMC: Get EXT-CSD from card failed\r\n");
		return 1;
	}
	printf(" EXCEPTION_EVENT_STATUS=%x\r\n",
		    cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_STATUS[0]);
	//check bkops
	if (cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_STATUS[0] &
	    EXT_CSD_EXP_URGENT_BKOPS) {
		ftsdc021_mmc_start_bkops();
	}
	//check packed command
	if (cardInfo.EXT_CSD_MMC.EXCEPTION_EVENTS_STATUS[0] &
	    EXT_CSD_EXP_PACKED_FAILURE) {
		printf(" PACKED_COMMAND_STATUS=%x\r\n",
			    cardInfo.EXT_CSD_MMC.PACKED_COMMAND_STATUS);
		return 1;
	}
	return 0;
}

uint32_t ftsdc021_mmc_send_vendor_cmd(uint32_t argument)
{
	uint16_t val;
	clock_t t0;
	
	printf(" Send Vendor command ... "); 
	cardInfo.ErrorSts = 0;
	cardInfo.cmplMask = (WAIT_CMD_COMPLETE | WAIT_TRANS_COMPLETE);

	gpReg->CmdArgu = argument;

	val = ((MMC_VENDOR_CMD & 0x3f) << 8) |
	      ((SDHCI_CMD_TYPE_NORMAL & 0x3) << 6) |
	      ((SDHCI_CMD_RTYPE_R1BR5B & 0x1F));
	gpReg->CmdReg = val;

	t0 = clock();
	while (cardInfo.cmplMask)  {
		/* 1 secs */
		if ((clock() - t0) > 1000) {
			printf(" ERR## ... Vendor Command: Wait for INTR "
				    "timeout.\r\n");
			return ERR_CMD_TIMEOUT;
		}
	}
	// Reset the Data and Cmd line due to the None-auto CMD12.
	gpReg->SoftRst |= (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT);
	while (gpReg->SoftRst & (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT));
	
	printf("Done.\r\n");

	return cardInfo.ErrorSts;
}

/* Generate packed commands header for eMMC*/
uint32_t ftsdc021_mmc_gen_packed_hdr(Transfer_Act act, uint32_t startAddr,
			       uint32_t blkcnt, uint32_t * buf)
{
	uint32_t hdr_sz, entries, addr, max_entries;

	hdr_sz = cardInfo.EXT_CSD_MMC.DATA_SECTOR_SIZE ? 4096 : 512;
	memset(buf, 0, hdr_sz);
	entries = 0;
	addr = startAddr;
	max_entries = act == READ ? 
		      cardInfo.EXT_CSD_MMC.MAX_PACKED_READS :
		      cardInfo.EXT_CSD_MMC.MAX_PACKED_WRITES;
	while (blkcnt > 0) {
		entries++;
		if (blkcnt >= entries && entries < max_entries) {
			buf[2 * entries] = entries;
			blkcnt -= entries;
		}
		else {
			buf[2 * entries] = blkcnt;
			blkcnt= 0;
		}
		/* Set reliable write flag when it is enabled. */
		if (cardInfo.reliable_write_en && act == WRITE)
			buf[2 * entries] |= RELIABLE_WRITE_FLAG;

		buf[2 * entries + 1] = addr;
		addr += entries;
	}
	buf[0] = (entries << 16) | ((act == READ ? 1 : 2) << 8) | 1;
}
