/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_sdio_ops.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2019/12/31 	Sanjin Liu	 Initial creation.
 *
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"
#include "ftsdc021_sdio.h"

/* SDIO CMD5 */
uint32_t ftsdc021_sdio_send_io_op_cond(SDCardInfo * card, uint32_t ocr,
				     uint32_t * rocr)
{
	uint32_t i, err = 0;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_IO_SEND_OP_COND;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R3R4;
	cmd.InhibitDATChk = 0;
	cmd.Argu = ocr;
	for (i = 100; i; i--) {
		/* CMD5 Response type R4 */
		err = ftsdc021_send_command(&cmd);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		if (card->respLo & MMC_CARD_BUSY)
			break;

		err = 1;

		ftsdc021_delay(10);
	}

	if (rocr)
		*rocr = card->respLo;

	return err;
}

uint32_t ftsdc021_sdio_io_rw_direct(SDCardInfo * card, uint32_t write, uint8_t fn,
				  uint32_t addr, uint8_t in, uint8_t * out)
{
	uint32_t err;
	struct mmc_cmd cmd = {0};

	cmd.CmdIndex = SDHCI_IO_RW_DIRECT;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = write ? SD_CMD52_RW_in_W : SD_CMD52_RW_in_R;
	cmd.Argu |= SD_CMD52_FUNC(fn);
	cmd.Argu |= (write && out) ? SD_CMD52_RAW : SD_CMD52_no_RAW;
	cmd.Argu |= SD_CMD52_Reg_Addr(addr);
	cmd.Argu |= in;

	/* CMD 52 Response type R5 */
	err = ftsdc021_send_command(&cmd);
	if (err)
		return err;

	if (card->respLo & (R5_ERROR | R5_FUNCTION_NUMBER | R5_OUT_OF_RANGE)) {
		printf("SDIO: IO_RW_DIRECT response indicate error "
			    "0x%x.\r\n", card->respLo);
		return 1;
	}

	if (out)
		*out = card->respLo & 0xFF;

	return 0;
}

uint32_t ftsdc021_sdio_io_rw_extended(SDCardInfo *card, uint32_t write, uint8_t fn,
				    uint32_t addr, uint32_t incr_addr, uint32_t *buf,
				    uint32_t blocks, uint32_t blksz)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act =  write ? WRITE : READ;
	data.bufAddr = buf;
	data.blkSz = blksz;
	data.blkCnt = blocks;
        cmd.data = &data;
	cmd.CmdIndex = SDHCI_IO_RW_EXTENDED;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = write ? SD_CMD53_RW_in_W : SD_CMD53_RW_in_R;
	cmd.Argu |= SD_CMD53_FUNC(fn);
	cmd.Argu |= incr_addr ? SD_CMD53_OP_inc : SD_CMD53_OP_fix;
	cmd.Argu |= SD_CMD53_Reg_Addr(addr);
	if (blocks == 1 && blksz <= 512) {
		/* byte mode */
		cmd.Argu |= (blksz == 512) ? SD_CMD53_Byte_Mode : blksz;
	} else {
		/* block mode */
		cmd.Argu |= SD_CMD53_Block_Mode | blocks;
	}

	/* CMD 53 */
	if (ftsdc021_send_command(&cmd)) {
		printf("SDIO: IO_RW_EXTENDED failed\r\n");
		return 1;
	}

	if (ftsdc021_transfer_data(data.act, buf, (blocks * blksz))) {
		return 1;
	}

	if (card->respLo & (R5_ERROR | R5_FUNCTION_NUMBER | R5_OUT_OF_RANGE)) {
		printf("SDIO: IO_RW_EXTENDED response error 0x%x.\r\n",
			    card->respLo);
		return 1;
	}

	return 0;
}
