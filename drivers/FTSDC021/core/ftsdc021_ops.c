/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_ops.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2010/4/2 	BingJiun-Luo	 Initial creation.
 *
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"

/**
 * MMC4.4: 
 * argument = 0x00000000, Resets the card to idle state.
 * argument = 0xF0F0F0F0, Resets the card to pre-idle state.
 * argument = 0xFFFFFFFA, Initiate alternative boot operation.
 *
 * SD Card: Argument always 0x00000000.
 */
uint32_t ftsdc021_ops_go_idle_state(uint32_t arg)
{
	uint8_t data;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_GO_IDLE_STATE;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = (arg == 0xFFFFFFFA) ? 1 : 0;
	cmd.RespType = SDHCI_CMD_NO_RESPONSE;
	cmd.InhibitDATChk = 0;
	cmd.Argu = arg;

	return ftsdc021_send_command(&cmd);
}

/**
 * SD card related operations
 */

/* CMD8 SD card */
uint32_t ftsdc021_ops_send_if_cond(SDCardInfo * card, uint32_t arg)
{
	uint32_t err;
	uint8_t test_pattern = (arg & 0xFF);
	uint8_t result_pattern;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_IF_COND;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = arg;

	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	result_pattern = card->respLo & 0xFF;

	if (result_pattern != test_pattern)
		return 1;

	return 0;
}

uint32_t ftsdc021_ops_app_cmd(SDCardInfo * card, uint32_t argu, uint8_t datChk)
{
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_APP_CMD;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = datChk;
	cmd.Argu = argu;

	return ftsdc021_send_command(&cmd);
}

/* ACMD41 */
uint32_t ftsdc021_ops_send_app_op_cond(SDCardInfo * card, uint32_t ocr, uint32_t * rocr)
{
	uint32_t i, err = 0;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SD_SEND_OP_COND;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R3R4;
	cmd.InhibitDATChk = 0;
	cmd.Argu = ocr;

	for (i = 100; i; i--) {
		/* CMD 55: Indicate to the card the next cmd is app-specific command */
		err = ftsdc021_ops_app_cmd(card, 0, 0);
		if (err)
			break;

		err = ftsdc021_send_command(&cmd);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		if (card->respLo & MMC_CARD_BUSY)
			break;

		err = 1;

		ftsdc021_delay(80);
	}

	if (rocr)
		*rocr = card->respLo;

	return err;
}

/* CMD11 R1 */
uint32_t ftsdc021_ops_send_voltage_switch(void)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_VOLTAGE_SWITCH;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = 0;

	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	return 0;
}

uint32_t ftsdc021_ops_all_send_cid(SDCardInfo * card)
{
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_ALL_CID;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R2;
	cmd.InhibitDATChk = 0;
	cmd.Argu = 0;

	if (ftsdc021_send_command(&cmd)) {
		printf("ALL SEND CID failed !\r\n");
		return 1;
	}

	card->CID_LO = card->respLo;
	card->CID_HI = card->respHi;

	return 0;
}

uint32_t ftsdc021_ops_send_rca(SDCardInfo * card)
{
	uint32_t i, err = 0;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_RELATIVE_ADDR;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = 0;

	for (i = 100; i; i--) {
		err = ftsdc021_send_command(&cmd);
		if (err)
			break;

		if ((card->respLo >> 16) & 0xffff)
			break;

		err = 1;
		ftsdc021_delay(10);
	}

	if (!err)
		card->RCA = (uint16_t) ((card->respLo >> 16) & 0xffff);

	return 0;
}

/* CMD9: Send CSD */
uint32_t ftsdc021_ops_send_csd(SDCardInfo * card)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_CSD;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R2;
	cmd.InhibitDATChk = 0;
	cmd.Argu = card->RCA << 16;

	/* CMD 9: Getting the CSD register from SD memory card */
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	card->CSD_LO = card->respLo;
	card->CSD_HI = card->respHi;

	return err;
}

uint32_t ftsdc021_ops_select_card(SDCardInfo * card)
{
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SELECT_CARD;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = card->RCA << 16;

	/* send CMD7 to enter transfer mode */
	return ftsdc021_send_command(&cmd);
}

uint32_t ftsdc021_ops_app_repo_wr_num(SDCardInfo * card, uint32_t *num)
{

	uint32_t err;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	err = ftsdc021_ops_app_cmd(card, card->RCA << 16, 1);
	if (err)
		return err;

	data.act = READ;
	data.bufAddr = (uint32_t *) &(card->SCR);
	data.blkSz = SD_WRITTEN_NUM_LENGTH;
	data.blkCnt = 1;
        cmd.data = &data;
	cmd.CmdIndex = SDHCI_SEND_NUM_WR_BLKS;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.DataPresent = 1;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	ftsdc021_transfer_data(READ, num, SD_WRITTEN_NUM_LENGTH);

	return 0;
}

uint32_t ftsdc021_ops_app_send_scr(SDCardInfo * card)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	err = ftsdc021_ops_app_cmd(card, card->RCA << 16, 1);
	if (err)
		return err;

	/* ACMD 51 */
	data.act = READ;
	data.bufAddr = (uint32_t *) &(card->SCR);
	data.blkSz = SCR_LENGTH;
	data.blkCnt = 1;
        cmd.data = &data;
	cmd.CmdIndex = SDHCI_SEND_SCR;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	return ftsdc021_transfer_data(READ, (uint32_t *) & (card->SCR), SCR_LENGTH);
}

uint32_t ftsdc021_ops_app_set_blk_cnt(SDCardInfo * card, uint32_t n_blocks)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	/* CMD 55: Indicate to the card the next cmd is app-specific command */
	cmd.CmdIndex = SDHCI_SET_BLOCK_COUNT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = n_blocks;
	if (card->context_id)
		cmd.Argu |= (card->context_id << 25);
	else if (card->reliable_write_en)
		cmd.Argu |= RELIABLE_WRITE_FLAG;
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	return 0;
}

uint32_t ftsdc021_ops_app_set_bus_width(SDCardInfo * card, uint32_t width)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	err = ftsdc021_ops_app_cmd(card, card->RCA << 16, 0);
	if (err)
		return err;

	cmd.CmdIndex = SDHCI_SET_BUS_WIDTH;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = width;
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	return 0;
}

/**
 * Switch Function returns 64 bytes status data. Caller must make sure
 * "resp" pointer has allocated enough space.
 */
uint32_t ftsdc021_ops_sd_switch(SDCardInfo * card, uint32_t mode, uint32_t group,
			      uint8_t value, uint8_t * resp)
{
	uint32_t err;
	Infinite_Test inf_mode;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	if (card->CardType != MEMORY_CARD_TYPE_SD) {
		printf("Switch Function: This is not SD Card !\r\n");
		return 1;
	}

	data.act = READ;
	data.bufAddr = (uint32_t *) resp;
	data.blkSz = 64;
	data.blkCnt = 1;
        cmd.data = &data;
	cmd.CmdIndex = SDHCI_SWITCH_FUNC;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	//Check Function
	cmd.Argu = mode << 31 | 0x00FFFFFF;
	cmd.Argu &= ~(0xF << (group * 4));
	cmd.Argu |= value << (group * 4);

	// This command does not support open-end transfer
	inf_mode = infinite_mode;
	infinite_mode = 0;
	/* Commented by MikeYeh 081201: The 31st bit of argument for CMD6 is
	 * zero to indicate "Check Function". Check function used to query if
	 * the card supported a specific function. */
	/* CMD 6 */
	err = ftsdc021_send_command(&cmd);
	if (err) {
		printf("CMD6 Failed.\r\n");
		goto out;
	}

	err = ftsdc021_transfer_data(READ, (uint32_t *) resp, 64);
out:
	infinite_mode = inf_mode;
	return err;
}

uint32_t ftsdc021_ops_send_tune_block(SDCardInfo * card, uint32_t * buf)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act = READ;
	data.bufAddr = buf;
	data.blkSz = 64;
	if (card->bus_width == 8)
		data.blkSz = 128;
	data.blkCnt = 1;
        cmd.data = &data;
	if (card->CardType == MEMORY_eMMC)
		cmd.CmdIndex = MMC_SEND_TUNE_BLOCK;
	else
		cmd.CmdIndex = SDHCI_SEND_TUNE_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_NO_RESPONSE;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	if (ftsdc021_send_command(&cmd)) {
		printf(" Execute tuning failed..\r\n");
		return 1;
	}
	/* Do not require to read data */

	if (buf == NULL)
		return 0;
	else
		return ftsdc021_transfer_data(READ, buf, data.blkSz);
}

/* CMD13: Return Card status inside R1 response */
uint32_t ftsdc021_ops_send_card_status(SDCardInfo * card)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_SEND_STATUS;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = card->RCA << 16;
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	return 0;
}

/* CMD16: Set block length */
uint32_t ftsdc021_ops_set_blocklen(SDCardInfo * card, uint32_t block_len)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	// The following insure the state is in Secure Mode.
	/* CMD 16 */
	cmd.CmdIndex = SDHCI_SET_BLOCKLEN;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = block_len;
	err = ftsdc021_send_command(&cmd);
	if (err || (card->respLo & SD_STATUS_ERROR_BITS)) {
		printf("Any error was happened when user called command\r\n");
		return 1;
	}

	return 0;
}

/* ACMD13: Return 512 bits of SD Status */
uint32_t ftsdc021_ops_send_sd_status(SDCardInfo * card)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	printf("ftsdc021_ops_send_sd_status\r\n");
	// The following insure the state is in Secure Mode.
	/* CMD 16 */
	err = ftsdc021_ops_set_blocklen(card, 0x40);
	if (err || (card->respLo & SD_STATUS_ERROR_BITS)) {
		printf("Any error was happened when user called command\r\n");
		return 1;
	}

	/* CMD 55 */
	err = ftsdc021_ops_app_cmd(card, card->RCA << 16, 0);
	if (err || (card->respLo & SD_STATUS_ERROR_BITS)) {
		printf("Error happened when user called command\r\n");
		return 1;
	}

	/* ACMD 13 */
	data.act = READ;
	data.bufAddr = (uint32_t *) &(card->sd_status);
	data.blkSz = sizeof(SDStatus);
	data.blkCnt = 1;
        cmd.data = &data;
	cmd.CmdIndex = SDHCI_SD_STATUS;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	err = ftsdc021_send_command(&cmd);
	if (err || (card->respLo & SD_STATUS_ERROR_BITS)) {
		printf("Error happened when user called command\r\n");
		return 1;
	}

	return ftsdc021_transfer_data(READ, (uint32_t *)&(card->sd_status),
				      sizeof(SDStatus));

}
