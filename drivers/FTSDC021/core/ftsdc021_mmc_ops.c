/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_mmc_ops.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2019/12/12 	Sanjin Liu	 Initial creation.
 *
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"

/* Send CMD12/CMD13 with HPI flag */
uint32_t ftsdc021_mmc_send_hpi(SDCardInfo *card)
{
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = card->hpi_cmd;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.Argu = card->RCA << 16 | 1;

	if (card->hpi_cmd == SDHCI_STOP_TRANS) {
		cmd.InhibitDATChk = 1;
		cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	}
	else {
		cmd.InhibitDATChk = 0;
		cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	}

	return ftsdc021_send_command(&cmd);
}

/* MMC CMD1 */
uint32_t ftsdc021_mmc_send_op_cond(SDCardInfo *card, uint32_t ocr, uint32_t *rocr)
{
	uint32_t i, err = 0;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = MMC_SEND_OP_COND;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R3R4;
	cmd.InhibitDATChk = 0;
	cmd.Argu = ocr;

	for (i = 100; i; i--) {
		err = ftsdc021_send_command(&cmd);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		/* otherwise wait until reset completes */
		if (card->respLo & MMC_CARD_BUSY)
			break;

		err = 1;

		ftsdc021_delay(10);
	}

	if (rocr)
		*rocr = card->respLo;

	return err;
}

/* MMC CMD3: assigns relative address to the Device */
uint32_t ftsdc021_mmc_set_rca(SDCardInfo *card)
{
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_RELATIVE_ADDR;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = card->RCA << 16;

	return ftsdc021_send_command(&cmd);
}

/* MMC CMD6: Switches the mode of operation of the selected Device or modifies
 * the EXT_CSD registers. */
uint32_t ftsdc021_mmc_switch(SDCardInfo *card, uint8_t set, uint8_t index, uint8_t value,
			   uint32_t use_busy)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = MMC_SWITCH;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	if (use_busy) {
		cmd.InhibitDATChk = 1;
		cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	} else {
		cmd.InhibitDATChk = 0;
		cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	}
	cmd.Argu = (EXT_CSD_Write_byte << 24) | (index << 16) | (value << 8) | set;

	err = ftsdc021_send_command(&cmd);

	if (card->respLo & R1_SWITCH_ERROR) {
		printf("MMC Switch: response indicate switch error "
			    "0x%x.\r\n", card->respLo);
		return 1;
	}

	return err;
}

/* MMC CMD8: Device sends its EXT_CSD register as a block of data. */
uint32_t ftsdc021_mmc_send_ext_csd(SDCardInfo *card)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};
	int ret, inf_mode;

	if (card->cqe_en)
		cqhci_off(1);

	memset(&card->EXT_CSD_MMC, 0, EXT_CSD_LENGTH);
	data.act = READ;
	data.bufAddr = (uint32_t *) &(card->EXT_CSD_MMC);
	data.blkSz = EXT_CSD_LENGTH;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = MMC_SEND_EXT_CSD;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	// This command does not support open-end transfer
	inf_mode = infinite_mode;
	infinite_mode = 0;
	ret = ftsdc021_send_command(&cmd);
	if (ret) {
		printf("Getting the Ext-CSD failed\r\n");
		goto out;
	}

	ret = ftsdc021_transfer_data(READ, (uint32_t *) & (card->EXT_CSD_MMC),
				     EXT_CSD_LENGTH);
out:
	infinite_mode = inf_mode;
	if (card->cqe_en)
		cqhci_on();

	return ret;
}

/* CMD28 */
uint32_t ftsdc021_mmc_set_write_prot(SDCardInfo *card, uint32_t address)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SET_WRITE_PROT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	cmd.InhibitDATChk = 1;
	cmd.Argu = address;
	err = ftsdc021_send_command(&cmd);
	if (err) {
		printf("Set write protection bits failed\r\n");
		return 1;
	}

	return 0;
}

/* CMD29 */
uint32_t ftsdc021_mmc_clear_write_prot(SDCardInfo *card, uint32_t address)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_CLR_WRITE_PROT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	cmd.InhibitDATChk = 1;
	cmd.Argu = address;
	err = ftsdc021_send_command(&cmd);
	if (err) {
		printf("Clear write protection bits failed\r\n");
		return 1;
	}

	return 0;
}

/* CMD30 */
uint32_t ftsdc021_mmc_send_write_prot(SDCardInfo *card, uint32_t *WP_bits,
				    uint32_t address)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act = READ;
	data.bufAddr = WP_bits;
	data.blkSz = 4;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_SEND_WRITE_PROT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = address;

	/* CMD 30 */
	err = ftsdc021_send_command(&cmd);
	if (err) {
		printf("Getting write protection bits failed\r\n");
		return 1;
	}

	return ftsdc021_transfer_data(READ, WP_bits, 4);
}

/* CMD31 */
uint32_t ftsdc021_mmc_send_write_prot_type(SDCardInfo *card, uint32_t *WP_bits,
					 uint32_t address)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act = READ;
	data.bufAddr = WP_bits;
	data.blkSz = 8;
	data.blkCnt = 1;
	cmd.data = &data;
	cmd.CmdIndex = SDHCI_SEND_WRITE_PROT_TYPE;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = address;

	/* CMD 30 */
	err = ftsdc021_send_command(&cmd);
	if (err) {
		printf("Getting write protection bits failed\r\n");
		return 1;
	}

	return ftsdc021_transfer_data(READ, WP_bits, 8);
}

/* CMD49: Send 512 bits Real Time Clock Information */
uint32_t ftsdc021_mmc_set_time(SDCardInfo *card, uint64_t time, uint8_t type)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};
	MMC_RTC_INFO mmc_rtc;
	uint32_t ret;

	memset(&mmc_rtc, 0, 512);
	mmc_rtc.Version = 1;
	mmc_rtc.RTC_INFO_TYPE = type;
	mmc_rtc.SECONDS_PASSED = time;

	cmd.CmdIndex = MMC_SET_TIME;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;

        data.act = WRITE;
        data.bufAddr = (uint32_t *)&mmc_rtc;
        data.blkSz = 512;
        data.blkCnt = 1;
        cmd.data = &data;

	ret = ftsdc021_send_command(&cmd);
	if (ret) {
		printf("RTC_SET_TIME:ftsdc021_send_command-> %d\r\n", ret);
		return ret;
	}
	ret = ftsdc021_transfer_data(WRITE, (uint32_t *)&mmc_rtc, 512);
	if (ret)
		printf("RTC_SET_TIME:ftsdc021_transfer_data-> %d\r\n", ret);

	return ret;
}
