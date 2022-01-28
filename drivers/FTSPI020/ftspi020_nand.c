/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_nand.c
 * DEPARTMENT : CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE         AUTHOR      DESCRIPTION
 * 2019/08/16   BingYao     SPI nand flash driver
 * -------------------------------------------------------------------------
 */
#include "ftspi020.h"
#include "ftspi020_cntr.h"

static int nand_check_status_till_ready(struct spi_flash *flash,
					uint32_t wait_ms);

struct spinand_flash_params {
	uint32_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint32_t block_size;
	const char *name;
	uint8_t *rd_cmds;
	uint32_t max_rd_cmds;
	uint8_t *wr_cmds;
	uint32_t max_wr_cmds;
	uint8_t *er_cmds;
	uint32_t max_er_cmds;
};

#define NR_OF_PAGES_PER_BLOCK    64
#define BITS_PER_ENT     8

#define BASIC_RD_CMDS	SPINAND_READ_FROM_CACHE, SPINAND_READ_FAST_FROM_CACHE

static uint8_t common_rd_cmds[] = {
	BASIC_RD_CMDS, SPINAND_READ_FROM_CACHE_DUAL, SPINAND_READ_FROM_CACHE_DUAL_IO,
	SPINAND_READ_FROM_CACHE_QUAD, SPINAND_READ_FROM_CACHE_QUAD_IO};

static uint8_t micron_rd_cmds[] = {
	SPINAND_READ_FROM_CACHE_QUAD};
	//BASIC_RD_CMDS, SPINAND_READ_FROM_CACHE_DUAL, SPINAND_READ_FROM_CACHE_QUAD};

#define BASIC_WR_CMDS	SPINAND_PAGE_LOAD

static uint8_t common_wr_cmds[] = {
	BASIC_WR_CMDS, SPINAND_PAGE_LOAD_QUAD};

static uint8_t micron_wr_cmds[] = {
	BASIC_WR_CMDS};

static uint8_t common_er_cmds[] = {
	SPINAND_ERASE_BLOCK};

/**
 * 1 page = (2K + 64) bytes
 *               1 block = (2K + 64) bytes x 64 pages
 *			 = (128K + 4K) bytes
 * A5U12A21ASC : 1 device = 512 blocks x 64 pages
 *			  = 32768 pages
 * GD5F1GQ4U : 1 device = 1024 blocks x 64 pages
 *			  = 65536 pages
 *
 */
static const struct spinand_flash_params nand_spi_flash_table[] = {
         {0x21AAEF, 2048, 65536, (2048 * 64), "W25N01GV",
          common_rd_cmds, sizeof(common_rd_cmds),
          common_wr_cmds, sizeof(common_wr_cmds),
          common_er_cmds, sizeof(common_er_cmds)},
         {0x2C122C, 2048, 65536, (2048 * 64), "MT29F1G01A",
          micron_rd_cmds, sizeof(micron_rd_cmds),
          micron_wr_cmds, sizeof(micron_wr_cmds),
          common_er_cmds, sizeof(common_er_cmds)},
         {0x48B1C8, 2048, 65536, (2048 * 64), "GD5F1GQ4xFxxG",
          common_rd_cmds, sizeof(common_rd_cmds),
          common_wr_cmds, sizeof(common_wr_cmds),
          common_er_cmds, sizeof(common_er_cmds)},
         {0x00D1C8, 2048, 65536, (2048 * 64), "GD5F1GQ4UB"},
	     {0x00D2C8, 2048, 131072, (2048 * 64), "GD5F2GQ4UB"},
	     {0x00F1C8, 2048, 65536, (2048 * 64), "GD5F1GQ4UA"},
	     {0x00F2C8, 2048, 131072, (2048 * 64), "GD5F2GQ4UA"},
	     {0x7F21C8, 2048, 65536, (2048 * 64), "F50L1G41A"},
#if 0
	     {0x7F20, 2048, 32768, (2048 * 64), "EN25LN512"},
	     {0x0041, 2048, 65536, (2048 * 64), "HYF1GQ4UA"},
	     {0x0051, 2048, 65536, (2048 * 64), "HYF1GQ4UA"},
	     {0x0042, 2048, 131072, (2048 * 64), "HYF2GQ4UA"},
#endif
};

static int spinand_set_block_protect(struct spi_flash * flash, uint8_t bp);
static int spinand_get_feature(struct spi_flash * flash, int fea_addr,
			       uint8_t *status);

uint32_t spinand_wait_ms = 1;

static char *spinand_action_get_string(struct spi_flash *flash,
				       uint32_t act, uint32_t type)
{
	uint8_t cmd;

	if (act == WRITE) {
		cmd = flash->wr_cmds[type];

		switch(cmd) {
		case SPINAND_PAGE_LOAD:
			return "Page Load";
		case SPINAND_PAGE_LOAD_QUAD:
			return "Page Load Quad";
		case SPINAND_PROGRAM_EXECUTE:
			return "Program Execute";
		default:
			return "Unknown Write Type";
		}
	} else if (act == READ) {
		cmd = flash->rd_cmds[type];

		switch(cmd) {
		case SPINAND_READ_FROM_CACHE:
			return "Read From Cache";
		case SPINAND_READ_FAST_FROM_CACHE:
			return "Read Fast From Cache";
		case SPINAND_READ_FROM_CACHE_DUAL:
			return "Read Dual From Cache";
		case SPINAND_READ_FROM_CACHE_DUAL_IO:
			return "Read Dual IO From Cache";
		case SPINAND_READ_FROM_CACHE_QUAD:
			return "Read Quad  From Cache";
		case SPINAND_READ_FROM_CACHE_QUAD_IO:
			return "Read Quad IO  IO From Cache";
		case SPINAND_READ_TO_CACHE:
			return "Read To Cache";
		default:
			return "Unknown Read Type";
		}
	} else if (act == ERASE) {
		cmd = flash->er_cmds[type];

		switch(cmd) {
		case SPINAND_ERASE_BLOCK:
			return "Erase Block 64KB";
		default:
			return "Unknown Erase Type";
		}
	}

	return "Unknown Action";
}

static int spinand_get_feature(struct spi_flash * flash, int fea_addr,
			       uint8_t *status)
{
	uint8_t rd_sts_cmd[2];
	uint8_t *sts = (uint8_t *) FTSPI020_DMA_ADDR;

	rd_sts_cmd[0] = SPINAND_GET_FEATURE;
	rd_sts_cmd[1] = fea_addr;
	if (spi_flash_cmd_read(flash, rd_sts_cmd, sts, 1))
		return 1;

	*status = *sts;

	return 0;
}

/**
 * ECC enable at Feature Register bit 4.
 * Make sure BUF mode at bit 3 is 0 for normal READ.
 * BUF=1 is Continuous READ mode
 */
static int spinand_set_fea_en(struct spi_flash * flash, uint8_t fea,
			      uint8_t en)
{
	uint8_t cmd[2];
	uint8_t cfg;

	if (spinand_get_feature(flash, SPINAND_FEA_CFG, &cfg)) {
		prints("%s: 1-Failed to read OTP register\n", flash->name);
		return 1;
	}
	prints("Feature register 0x%x \n", cfg);

	if (en && (cfg & fea)) {
		prints("%s already enabled(0x%x) \n",
		(fea & SPINAND_CFG_QE_EN) ? "QE" :
		(fea & SPINAND_CFG_BUF_MODE) ? "BUF" :
		(fea & SPINAND_CFG_ECC_EN) ? "ECC" :
		(fea & SPINAND_CFG_ENABLE) ? "OTP" : "UnKnown", fea);
		return 0;
	}

	if (!en && !(cfg & fea)) {
		prints("%s already disabled(0x%x) \n",
		(fea & SPINAND_CFG_QE_EN) ? "QE" :
		(fea & SPINAND_CFG_BUF_MODE) ? "BUF" :
		(fea & SPINAND_CFG_ECC_EN) ? "ECC" :
		(fea & SPINAND_CFG_ENABLE) ? "OTP" : "UnKnown", fea);
		return 0;
	}

	if (en)
		cfg |= fea;
	else
		cfg &= ~fea;

	prints("%s: %s %s (Fea val 0x%x) ", flash->name,
		 en ? "Enable":"Disable",
		(fea & SPINAND_CFG_QE_EN) ? "QE" :
		(fea & SPINAND_CFG_BUF_MODE) ? "BUF" :
		(fea & SPINAND_CFG_ECC_EN) ? "ECC" :
		(fea & SPINAND_CFG_ENABLE) ? "OTP" : "UnKnown",
				cfg);

	cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	cmd[0] = SPINAND_SET_FEATURE;
	cmd[1] = SPINAND_FEA_CFG;
	if (spi_flash_cmd_write(flash, cmd, (const void *) &cfg, 1)) {
		prints("failed.\n");
		return 1;
	}

	/* Read it back to confirm the value */
	cfg = en ? 0 : 0xf;
	if (spinand_get_feature(flash, SPINAND_FEA_CFG, &cfg)) {
		prints("2-Failed to read OTP register\n");
		return 1;
	}

	if ((en && !(cfg & fea)) ||
	    (!en && (cfg & fea))) {
		prints("failed.\n");
		return 1;
	}

	prints("success.\n");

	cmd[0] = SPINAND_WRITE_DISABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	return 0;

}

/**
 * Default value BP is 3'b111 means all locked
 */
static int spinand_set_block_protect(struct spi_flash * flash, uint8_t bp)
{
	uint8_t cmd[2];
	uint8_t bl_lock;

	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &bl_lock)) {
		prints("%s: 1-Failed to read Block lock register\n",
		       flash->name);
		return 1;
	}

	if (g_debug > 2)
		prints("Block Protect register 0x%x \n", bl_lock);

	bp &= 0xF;
	bl_lock &= ~(SPINAND_BL_SRP1 | SPINAND_BL_BP0TO2 | SPINAND_BL_SRP0);
	bl_lock |= (bp << 3);
	prints("%s: Set Block Protect register 0x%x: ",
		flash->name, bl_lock);

	cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	cmd[0] = SPINAND_SET_FEATURE;
	cmd[1] = SPINAND_FEA_PROTECTION;
	if (spi_flash_cmd_write(flash, cmd, (const void *) &bl_lock, 1)) {
		prints("failed.\n");
		return 1;
	}

	/* Read it back to confirm the value */
	bl_lock = 0xf;
	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &bl_lock)) {
		prints("failed to read Block lock register\n");
		return 1;
	}

	if (((bl_lock >> 3) & 0xf) == bp)
		prints(" success\n");
	else
		prints("read back value wrong.(0x%x)\n", bl_lock);

	cmd[0] = SPINAND_WRITE_DISABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	return 0;

}

static int nand_check_status_till_ready(struct spi_flash *flash,
				        uint32_t wait_ms)
{
	uint64_t t0, t1;
	uint8_t status;

	FTSPI020_busy_location(flash->ctrl_id, BUSY_BIT0);
	spinand_wait_ms = wait_ms;
	t0 = ftspi020_get_time();
	do {
		if (spinand_get_feature(flash, SPINAND_FEA_STATUS,
					&status)) {
			prints("%s: Failed to check status by SW\n",
			       flash->name);
			return 1;
		}

		t1 = ftspi020_get_time();
		if (((t1 - t0) / COUNTS_PER_MSECOND) > 1000) {
			prints("%s: Timeout to check status by SW\n",
					flash->name);
			return 1;
		}
	} while (status & SPINAND_STS_BUSY);

	return 0;
}

static void spinand_mark_bad_block(struct spi_flash * flash,
				   uint32_t blk_number)
{
	uint16_t blk_idx, blk_shift;

	blk_idx = blk_number / 8;
	blk_shift = blk_number % 8;
	flash->bad_block_table[blk_idx] |= (1 << blk_shift);

	prints("%s: Bad block %lu bbt index %d pos %d.\n", flash->name,
		blk_number, blk_idx, blk_shift);
}


static int nand_is_bad_block(struct spi_flash * flash, uint32_t offset)
{
	uint32_t blk;
	uint16_t blk_idx, blk_shift;

	blk = offset / flash->erase_block_size;
	blk_idx = blk / 8;
	blk_shift = blk % 8;

	return (flash->bad_block_table[blk_idx] & (1 << blk_shift));
}

/**
 * Identifying Initial invalid Blocks by check the
 * data page 0 and page 1 from block 0 to the last block.
 *
 * offset should point to the first page of a block, but
 * not mandatory.
 *
 * return 1 if offset is located at bad block
 */
static int nand_check_bad_block(struct spi_flash * flash, uint32_t offset)
{
	int row_addr, blk, ret, i, len, j;
	char *write_buf, *read_buf;
	//char buf;

	row_addr = offset / flash->page_size;
	blk = offset / flash->erase_block_size;

	prints("%s: Scan block %d off %lu ...\n",
			flash->name, blk, offset);

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	len = flash->page_size;

	for (i = 0; i < 2; i++) {

		for (j = 0; j < len; j++)
			*(write_buf + j) = (rand() & 0xFF);

		offset += (i * len);
		flash->write(flash, 0, offset, len, write_buf);

		memset(read_buf, 0, len);
		flash->read(flash, 0, offset, len, read_buf);

		ret = FTSPI020_compare(write_buf, read_buf, len);
		if (ret) {
			blk = offset / flash->erase_block_size;
			goto mark_bad_block;
		}

		//Check the next page content.
		memset(read_buf, 0, len);
		flash->read(flash, 0, (offset + len), len, read_buf);

		//The next page read content should not be the same
		//with previous page.
		ret = FTSPI020_compare(write_buf, read_buf, len);
		if (ret == 0) {
			prints("Off %lx has the same value with off %lx\n",
					(offset + len), offset);
			blk = offset / flash->erase_block_size;
			goto mark_bad_block;
		}

		// Check if next page content all 0xFF
		ret = FTSPI020_compare(NULL, read_buf, len);
		if (ret) {
			blk = offset / flash->erase_block_size;
			goto mark_bad_block;
		}


	}
#if 0
	buf = 0;
	//Read the first page of block
	row_addr = blk * NR_OF_PAGES_PER_BLOCK;
	if (flash->read_spare(flash, row_addr, 2048, 1, &buf))
		prints("%s: Scan bad blocks, read spare failed at " \
		       "addr 0x%x.\n", flash->name, row_addr);

	if (buf != 0xff)
		goto mark_bad_block;

	buf = 0;
	if (flash->read_spare(flash, (row_addr + 1), 2048, 1, &buf))
		prints("%s: Scan bad blocks, read spare failed " \
		       "at addr 0x%x.\n", flash->name,
		       (row_addr + 1));

	if (buf != 0xff)
		goto mark_bad_block;
#endif
	return 0;

mark_bad_block:
	spinand_mark_bad_block(flash, blk);

	return 1;
}

/**
 * Do not erase or program factory-marked bad blocks.
 *
 * Do not recommend use this function, take very long time
 * to scan the whole chip for bad blocks.
 */
static int nand_scan_bad_blocks(struct spi_flash * flash)
{
	int offset;

	//for (offset = 0; offset < flash->size; offset +=
	for (offset = 0; offset < (flash->size / 2); offset +=
	     flash->erase_block_size) {

		nand_check_bad_block(flash, offset);
	}

	return 0;
}

static int spi_xfer_spinand(struct spi_flash *flash, unsigned int len,
			    const void *dout, void *din, unsigned int *flags)
{

	struct ftspi020_cmd spi_cmd;
	uint8_t *data_out = (uint8_t *) dout;

	memset(&spi_cmd, 0, sizeof(struct ftspi020_cmd));

	if (*flags & SPI_XFER_CMD_STATE) {
		spi_cmd.start_ce = flash->ce;
		spi_cmd.ins_code = *data_out;

		switch (spi_cmd.ins_code) {
		case CMD_RESET:	/* Reset the Flash */
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_WRITE_ENABLE:
		case SPINAND_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_ERASE_BLOCK:
			spi_cmd.spi_addr = (*(data_out + 3) << 16) |
			   (*(data_out + 2) << 8) |
			   (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_SET_FEATURE:
			spi_cmd.spi_addr = *(data_out + 1);
			spi_cmd.addr_len = addr_1byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;
			break;
		case SPINAND_GET_FEATURE:
			spi_cmd.spi_addr = *(data_out + 1);
			spi_cmd.addr_len = addr_1byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.data_cnt = len;
			spi_cmd.read_status = 1;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.spi_type = 1;
			break;
		case SPINAND_RANDOM_WRITE_PAGE:
		case SPINAND_QUAD_RANDOM_WRITE_PAGE:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) |
					    *(data_out + 1);
			spi_cmd.addr_len = addr_2byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			if (spi_cmd.ins_code == SPINAND_RANDOM_WRITE_PAGE)
				spi_cmd.spi_mode = spi_operate_serial_mode;
			else
				spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case SPINAND_PROGRAM_EXECUTE:
			spi_cmd.spi_addr = (*(data_out + 3) << 16) |
					   (*(data_out + 2) << 8) |
					   (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.spi_type = 1;
			break;
		case SPINAND_PAGE_LOAD:
		case SPINAND_PAGE_LOAD_QUAD:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) |
					    *(data_out + 1);
			spi_cmd.addr_len = addr_2byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			if (spi_cmd.ins_code == SPINAND_PAGE_LOAD)
				spi_cmd.spi_mode = spi_operate_serial_mode;
			else
				spi_cmd.spi_mode = spi_operate_quad_mode;
			spi_cmd.spi_type = 1;
			break;
		case SPINAND_READ_TO_CACHE:
			spi_cmd.spi_addr =(*(data_out + 3) << 16) |
			   (*(data_out + 2) << 8) |
			   (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write; /* no return data */
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.spi_type = 1;
			break;
		case SPINAND_READ_FROM_CACHE:
		case SPINAND_READ_FAST_FROM_CACHE:
		case SPINAND_READ_FROM_CACHE_DUAL:
		case SPINAND_READ_FROM_CACHE_QUAD:
		case SPINAND_READ_FROM_CACHE_DUAL_IO:
		case SPINAND_READ_FROM_CACHE_QUAD_IO:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) |
					    *(data_out + 1);
			//Gigadevice use 3 bytes column address.
			//Instead of dummy cycles after address.
			if (flash->code == 0x48B1C8) {
				spi_cmd.addr_len = addr_3byte;
			} else {
				spi_cmd.addr_len = addr_2byte;
			}

			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			if (spi_cmd.ins_code == SPINAND_READ_FROM_CACHE) {
				spi_cmd.spi_mode = spi_operate_serial_mode;
				//Gigadevice 03H uses no dummy cycles after column address
				if (flash->code == 0x48B1C8) {
					spi_cmd.dum_2nd_cyc = 0;
				}
			} else if (spi_cmd.ins_code == SPINAND_READ_FAST_FROM_CACHE) {
				spi_cmd.spi_mode = spi_operate_serial_mode;
			} else if (spi_cmd.ins_code ==
				 SPINAND_READ_FROM_CACHE_DUAL) {
				spi_cmd.spi_mode = spi_operate_dual_mode;
			} else if (spi_cmd.ins_code ==
				 SPINAND_READ_FROM_CACHE_QUAD) {
				spi_cmd.spi_mode = spi_operate_quad_mode;
			} else if (spi_cmd.ins_code ==
				 SPINAND_READ_FROM_CACHE_DUAL_IO) {
				spi_cmd.spi_mode = spi_operate_dualio_mode;
				spi_cmd.dum_2nd_cyc = 4;

				if (flash->code == 0x48B1C8) {
					spi_cmd.addr_len = addr_2byte;
				}
			} else if (spi_cmd.ins_code ==
					SPINAND_READ_FROM_CACHE_QUAD_IO) {
				spi_cmd.spi_mode = spi_operate_quadio_mode;
				spi_cmd.dum_2nd_cyc = 4;

				if (flash->code == 0x48B1C8) {
					spi_cmd.addr_len = addr_2byte;
					spi_cmd.dum_2nd_cyc = 2;
				}
			}
			spi_cmd.spi_type = 1;
			break;
		default:
			prints("%s: Wrong Command Code 0x%x.\n", flash->name,
			       spi_cmd.ins_code);
			return 1;
		}
		FTSPI020_issue_cmd(flash->ctrl_id, &spi_cmd);
	} else if (*flags & SPI_XFER_DATA_STATE) {
		FTSPI020_data_access(flash->ctrl_id, (uint8_t *) dout, (uint8_t *) din, len);

	}

	if (*flags & SPI_XFER_CHECK_CMD_COMPLETE) {
		if (FTSPI020_wait_cmd_complete(flash->ctrl_id, spinand_wait_ms))
			return 1;
	}

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache) -> row address
 *  2) 0Fh (GET FEATURE command to read the status)
 *  3) 0Bh or 03h (READ FROM CACHE x1) / 3Bh (x2) / 6Bh (x4) -> column address
 */
static int nand_read_spare(struct spi_flash *flash,
			   uint32_t row_addr, uint32_t col_addr,
			   uint32_t total_len, char *buf)
{
	uint8_t rd_cmd[4];

	if (nand_check_status_till_ready(flash, 100))
		return 1;

	if (col_addr < 2048) {
		prints("%s: Spare area start at byte 2048.\n", flash->name);
		col_addr = 2048;
	}

	if (total_len > 64) {
		prints("%s: Spare area has only 64 bytes.\n", flash->name);
		total_len = 64;
	}

	if ((col_addr + total_len) > 2112) {
		prints("%s: Spare area read start plus length exceeds " \
		       "2010 bytes.\n", flash->name);
		total_len = 2112 - col_addr;
	}

	rd_cmd[0] = SPINAND_READ_TO_CACHE;
	rd_cmd[1] = row_addr & 0xFF;
	rd_cmd[2] = ((row_addr & 0xFF00) >> 8);
	rd_cmd[3] = 0;
	if (spi_flash_cmd(flash, rd_cmd, NULL, 0)) {
		prints("%s: %s: READ(SPARE) TO CACHE @ row %lu failed\n",
		        flash->name,
			(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (nand_check_status_till_ready(flash, 1)) {
		prints("%s: %s: Wait READ(SPARE) TO CACHE ready @ " \
			"row %lu failed\n", flash->name,
			(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	rd_cmd[0] = SPINAND_READ_FROM_CACHE;
	rd_cmd[1] = col_addr & 0xFF;
	rd_cmd[2] = ((col_addr & 0xFF00) >> 8);
	if (spi_flash_cmd_read(flash, rd_cmd, buf, total_len)) {
		prints("%s: %s: READ(SPARE) FROM CACHE %lu bytes @ " \
			"col %lu failed\n", flash->name,
			(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", total_len,
			col_addr);
		return 1;
	}

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 06h (WRITE ENABLE)
 *  2) 02h (PROGRAM LOAD x1) / 32h (x4)
 *  3) 10h (PROGRAM EXECUTE)
 *  4) 0Fh (GET FEATURE command to read the status)
 */
static int nand_write_spare(struct spi_flash *flash,
			    uint32_t row_addr, uint32_t col_addr,
			    uint32_t total_len, void *buf)
{
	uint8_t sts;
	uint8_t wr_cmd[4];

	if (nand_check_status_till_ready(flash, 100))
		return 1;

	if (col_addr < 2048) {
		prints("%s: Spare area start at byte 2048.\n", flash->name);
		col_addr = 2048;
	}

	if (total_len > 64) {
		prints("%s: Spare area has only 64 bytes.\n", flash->name);
		total_len = 64;
	}

	if ((col_addr + total_len) > 2112) {
		prints("%s: Spare area read start plus length exceeds " \
		       "2010 bytes.\n", flash->name);
		total_len = 2112 - col_addr;
	}

	wr_cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
		return 1;

	wr_cmd[0] = SPINAND_PAGE_LOAD;
	wr_cmd[1] = col_addr & 0xFF;
	wr_cmd[2] = ((col_addr & 0xFF00) >> 8);
	if (spi_flash_cmd_write(flash, wr_cmd, buf, total_len)) {
		prints("%s: %s: PAGE LOAD(SPARE) %lu bytes @ col %lu" \
		       "failed\n", flash->name,
		       (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		       total_len, col_addr);
		return 1;
	}

	wr_cmd[0] = SPINAND_PROGRAM_EXECUTE;
	wr_cmd[1] = row_addr & 0xFF;
	wr_cmd[2] = ((row_addr & 0xFF00) >> 8);
	wr_cmd[3] = 0;
	if (spi_flash_cmd(flash, wr_cmd, NULL, 0)) {
		prints("%s: %s: PROGRAM EXECUTE(SPARE) @ row %lu " \
		       "failed\n", flash->name,
			(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (nand_check_status_till_ready(flash, 1)) {
		prints("%s: %s: Wait PROGRAM EXECUTE(SPARE) ready @ " \
		       "row %lu failed\n", flash->name,
		       (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
		prints("%s: Failed to check page %lu program(spare) status.\n",
			flash->name, row_addr);
		return 1;
	}

	if (sts & SPINAND_STS_PROGRAM_FAIL)
		prints("%s: Page %lu check status program(spare) failed.\n",
			flash->name, row_addr);

	return 0;
}
/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache) -> row address
 *  2) 0Fh (GET FEATURE command to read the status)
 *  3) 0Bh or 03h (READ FROM CACHE x1) / 3Bh (x2) / 6Bh (x4) -> column address
 */
static int nand_read(struct spi_flash *flash, uint8_t type,
		     uint32_t offset, uint32_t total_len, void *buf)
{
	uint8_t cmd_code, sts;
	uint8_t rd_cmd[4];
	uint32_t row_addr, col_addr, len;

	if (type > flash->max_rd_type) {
		prints("%s: Read type exceed max\n", flash->name);
		return 2;
	}

	cmd_code = flash->rd_cmds[type];

	prints("%s: %s: %s(%02xh) %lu bytes @ %lu\n", flash->name,
		(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		spinand_action_get_string(flash, READ, type),
		cmd_code, total_len, offset);

	do {
		row_addr = offset / flash->page_size;
		col_addr = offset % flash->page_size;
		len = total_len;

		/* exceed one page size */
		if (col_addr + len > flash->page_size)
			len = flash->page_size - col_addr;

		rd_cmd[0] = SPINAND_READ_TO_CACHE;
		rd_cmd[1] = row_addr & 0xFF;
		rd_cmd[2] = ((row_addr & 0xFF00) >> 8);
		rd_cmd[3] = 0;
		if (spi_flash_cmd(flash, rd_cmd, NULL, 0)) {
			prints("%s: %s: READ TO CACHE @ row %lu failed\n",
				flash->name,
			        (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				row_addr);
			return 1;
		}

		if (nand_check_status_till_ready(flash, 1)) {
			prints("%s: %s: Wait READ TO CACHE ready @ row %lu"\
			       "failed\n", flash->name,
			       (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA", row_addr);
			return 1;
		}

		rd_cmd[0] = cmd_code;
		rd_cmd[1] = col_addr & 0xFF;
		rd_cmd[2] = ((col_addr & 0xFF00) >> 8);
		if (spi_flash_cmd_read(flash, rd_cmd, buf, len)) {
			prints("%s: %s: %s %lu bytes @ col %lu failed\n",
				flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				spinand_action_get_string(flash, READ, type), len, col_addr);
			return 1;
		}

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check read row %lu col %lu" \
			       "ECC.\n", flash->name, row_addr, col_addr);
			return 1;
		}

		if ((sts & SPINAND_STS_ECC_STS0_1) == 0x20) {
			prints("%s: Read row %lu col %lu len %lu ECC failed.\n",
				flash->name, row_addr, col_addr, len);
			return 3;
		}

		total_len -= len;
		buf += len;
		offset += len;

	} while (total_len);

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 06h (WRITE ENABLE)
 *  2) 02h (PROGRAM LOAD x1) / 32h (x4)
 *  3) 10h (PROGRAM EXECUTE)
 *  4) 0Fh (GET FEATURE command to read the status)
 */
static int nand_write(struct spi_flash *flash, uint8_t type,
		      uint32_t offset, uint32_t total_len, void *buf)
{
	uint8_t cmd_code, sts;
	uint8_t wr_cmd[4];
	uint32_t row_addr, col_addr, blk, blk_idx, blk_shift;
	uint32_t len;

	if (type > flash->max_wr_type) {
		prints("%s: Write type exceed max\n", flash->name);
		return 1;
	}

	cmd_code = flash->wr_cmds[type];

	prints("%s: %s: %s(%02xh) %lu bytes @ %lu\n", flash->name,
		(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		spinand_action_get_string(flash, WRITE, type),
		cmd_code, total_len, offset);

	do {
		row_addr = offset / flash->page_size;
		col_addr = offset % flash->page_size;
		len = total_len;

		/* exceed one page size */
		if (col_addr + len > flash->page_size)
			len = flash->page_size - col_addr;

		/* Do not program factory-marked bad blocks. */
		blk = row_addr / NR_OF_PAGES_PER_BLOCK;
		blk_idx = blk / 8;
		blk_shift = blk % 8;
		if (flash->bad_block_table[blk_idx] & (1 << blk_shift)) {
			prints("%s: %s: Skip bad block %lu row addr 0x%lx ",
				flash->name,
				spinand_action_get_string(flash, WRITE, type), blk,
				row_addr);
			return 3;
		}

		wr_cmd[0] = SPINAND_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
			return 1;

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check page %lu program " \
			       "status.\n", flash->name, row_addr);
			return 1;
		}

		if ((sts & SPINAND_STS_WE_LATCH) == 0){
			prints("%s: page %lu program FAILED, WEL != 1\n", flash->name, row_addr);
			return 1;
		}

		wr_cmd[0] = cmd_code;
		wr_cmd[1] = col_addr & 0xFF;
		wr_cmd[2] = ((col_addr & 0xFF00) >> 8);
		if (spi_flash_cmd_write(flash, wr_cmd, buf, len)) {
			prints("%s: %s: %s %lu bytes @ col %lu failed\n",
				flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				spinand_action_get_string(flash, WRITE, type), len, col_addr);
			return 1;
		}

		if (nand_check_status_till_ready(flash, 1)) {
			prints("%s: %s: Wait PROGRAM LOAD DATA ready @ " \
			       "row %lu failed\n", flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				row_addr);
			return 1;
		}

		wr_cmd[0] = SPINAND_PROGRAM_EXECUTE;
		wr_cmd[1] = row_addr & 0xFF;
		wr_cmd[2] = ((row_addr & 0xFF00) >> 8);
		wr_cmd[3] = 0;
		if (spi_flash_cmd(flash, wr_cmd, NULL, 0)) {
			prints("%s: %s: PROGRAM EXECUTE @ row %lu failed\n",
				flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				row_addr);
			return 1;
		}

		if (nand_check_status_till_ready(flash, 1)) {
			prints("%s: %s: Wait PROGRAM EXECUTE ready @ " \
			       "row %lu failed\n", flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				row_addr);
			return 1;
		}

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check page %lu program " \
			       "status.\n", flash->name, row_addr);
			return 1;
		}

		if (sts & SPINAND_STS_PROGRAM_FAIL) {
			prints("%s: %s Page %lu check status program " \
			       "failed.\n", flash->name,
				spinand_action_get_string(flash, WRITE, type), row_addr);
			spinand_mark_bad_block(flash, blk);
			return 3;
		}

		total_len -= len;
		buf += len;
		offset += len;

	} while (total_len);

	wr_cmd[0] = SPINAND_WRITE_DISABLE;
	if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
		return 1;

	return 0;
}

static int nand_erase(struct spi_flash *flash, uint8_t type,
		      uint32_t offset, uint32_t len)
{
	int addr, row_addr, erase_size, ret = 1 ;
	uint8_t er_cmd[4];
	uint8_t cmd_code, sts;
	uint32_t wait_t, blk;

	cmd_code = flash->er_cmds[type];
	if (cmd_code == SPINAND_ERASE_BLOCK) {
		erase_size = flash->erase_block_size;
		wait_t = 10; /* Max Block erase time 10 ms */
	} else {
		prints("%s: %s @ 0x%lx\n", flash->name,
			spinand_action_get_string(flash, ERASE, type), offset);
		return 1;
	}

	offset &=  ~(erase_size - 1);

	if (nand_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION,
					&sts)) {
			prints("%s: Failed to read block protection " \
			       "status.\n", flash->name);
			return 1;
		}

		if (sts & SPINAND_BL_BP0TO2) {
			if (spinand_set_block_protect(flash, 0))
				return 1;
		}

		er_cmd[0] = SPINAND_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		row_addr = addr / flash->page_size;
		er_cmd[0] = cmd_code;
		er_cmd[1] = row_addr & 0xFF;
		er_cmd[2] = ((row_addr & 0xFF00) >> 8);
		er_cmd[3] = 0;

		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		if (nand_check_status_till_ready(flash, wait_t))
			return 1;

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check erase block %d status.\n",
				flash->name, (row_addr >> 6));
			return 1;
		}

		blk = row_addr / NR_OF_PAGES_PER_BLOCK;
		if (sts & SPINAND_STS_ERASE_FAIL) {
			prints("%s: %s: Erase block %lu(row %d) check status" \
				"0x%x failed.\n", flash->name,
				spinand_action_get_string(flash, ERASE, type),
				blk, row_addr, sts);

			spinand_mark_bad_block(flash, blk);
			continue;
		} else
			prints("%s: %s(%02xh) %lu (row_addr 0x%x).\n",
				flash->name, spinand_action_get_string(flash, ERASE, type), cmd_code,
				blk, row_addr);
	}

	return ret;
}

static int nand_erase_all(struct spi_flash *flash)
{
	if (nand_erase(flash, 0, 0, flash->size))
		return 1;

	return 0;
}

static int nand_report_status(struct spi_flash *flash)
{
	uint8_t status;

	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &status))
		prints("%s: Failed to read Block Lock Register.\n",
			flash->name);

	prints("%s: Protection Register(SR1): 0x%x.\n", flash->name, status);

	if (spinand_get_feature(flash, SPINAND_FEA_CFG, &status))
		prints("%s: Failed to read One Time Program Register.\n",
			flash->name);
	prints("%s: Configuration Register(SR2): 0x%x.\n", flash->name, status);

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &status))
		prints("%s: Failed to read Status Register.\n",
			flash->name);
	prints("%s: Status Register(SR3): 0x%x.\n", flash->name, status);

	if (flash->code != 0x21AA) {
		if (spinand_get_feature(flash, SPINAND_FEA_DRV_STRENGTH, &status))
			prints("%s: Failed to read Driver Strength Register.\n",
					flash->name);
		prints("%s: Driver Strength Register: 0x%x.\n", flash->name, status);
	}
	return 0;
}

/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache)
 *  2) 06h (WRITE ENABLE)
 *  3) 84h (PROGRAM LOAD RANDOM DATA x1) / 34h (x4); this is OPTIONAL in sequence.
 *  4) 10h (PROGRAM EXECUTE)
 *  5) 0Fh (GET FEATURE command to read the status)
 *
 * Move maximum one page content from one block to another block each time.
 * If user want to move more than one page, call this function again for next page.
 */
static int nand_intern_move(struct spi_flash *flash, uint32_t src_row,
			    uint32_t dst_row, uint32_t dst_col,
			    void * buf, uint32_t total_len)
{
	uint8_t cmd[4], sts;

	prints("%s: Copy data from row %lu to %lu \n", flash->name,
		src_row, dst_row);

	cmd[0] = SPINAND_READ_TO_CACHE;
	cmd[1] = src_row & 0xFF;
	cmd[2] = ((src_row & 0xFF00) >> 8);
	cmd[3] = 0;
	if (spi_flash_cmd(flash, cmd, NULL, 0)) {
		prints("%s: READ TO CACHE @ row %lu failed\n",
			flash->name, src_row);
		return 1;
	}

	if (nand_check_status_till_ready(flash, 1)) {
		prints("%s: Wait READ TO CACHE ready @ %lu failed\n",
			flash->name, src_row);
		return 1;
	}

	cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	if (buf && total_len) {
		prints("%s: Random data load %lu bytes to col %lu\n",
			flash->name,
			total_len, dst_col);
		cmd[0] = SPINAND_QUAD_RANDOM_WRITE_PAGE;//SPINAND_RANDOM_WRITE_PAGE;
		cmd[1] = dst_col & 0xFF;
		cmd[2] = ((dst_col & 0xFF00) >> 8);
		if (spi_flash_cmd_write(flash, cmd, buf, total_len)) {
			prints("%s: %s: Load Random Data: %lu bytes @ col %lu " \
			       "failed\n", flash->name,
				(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
				total_len, dst_col);
			return 1;
		}
	}

	cmd[0] = SPINAND_PROGRAM_EXECUTE;
	cmd[1] = dst_row & 0xFF;
	cmd[2] = ((dst_row & 0xFF00) >> 8);
	cmd[3] = 0;
	if (spi_flash_cmd(flash, cmd, NULL, 0)) {
		prints("%s: PROGRAM EXECUTE @ row %lu failed\n",
			flash->name, dst_row);
		return 1;
	}

	if (nand_check_status_till_ready(flash, 1)) {
		prints("%s: Wait PROGRAM EXECUTE ready @ %lu failed\n",
			flash->name, dst_row);
		return 1;
	}

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
		prints("%s: Failed to check page %lu program status.\n",
			flash->name, dst_row);
		return 1;
	}

	if (sts & SPINAND_STS_PROGRAM_FAIL)
		prints("%s: Page %lu program failed.\n", flash->name, dst_row);

	return 0;
}

int32_t spi_flash_probe_spinand(struct spi_flash *flsh, uint32_t ctrl_id,
				uint32_t ce, uint8_t *code)
{
	uint8_t i, cfg;
	uint32_t idcode;
	const struct spinand_flash_params *params;
	struct ftspi020_cmd spi_cmd = {0};
	uint32_t buf, changed;
	uint8_t cmd[2];

	//Latest SPI NAND flash READ ID(9Fh) doesn't need to insert
	//one address byte. We already have idcode, not send READ ID here.
	if ((code[0] == 0xC8) && ((code[1] == 0xB1) || (code[1] == 0xA1)) &&
				 (code[2] == 0x48)) {
		idcode = (uint32_t) ((code[2] << 16) | (code[1] << 8) | code[0]);
		goto scan_table;
	}

	//Some vendors require RESET before any other commands
	spi_cmd.start_ce = ce;
	spi_cmd.ins_code = CMD_RESET;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_write;
	spi_cmd.spi_mode = spi_operate_serial_mode;
	FTSPI020_issue_cmd(ctrl_id, &spi_cmd);
	if (FTSPI020_wait_cmd_complete(ctrl_id, 10)) {
		prints("spi nand:%lu-%lu: RESET, wait cmd complete " \
		       "failed\n", ctrl_id, ce);
		return 1;
	}

	spi_cmd.ins_code = CMD_READ_ID;
	spi_cmd.write_en = spi_read;
	spi_cmd.data_cnt = 3;

	/* After READ ID opCode, GigaDevice sends
	 * address 00h/01h, zentel sends dummy byte.
	 */
	spi_cmd.spi_addr = 0;
	spi_cmd.addr_len = addr_1byte;

	FTSPI020_issue_cmd(ctrl_id, &spi_cmd);
	if (FTSPI020_data_access(ctrl_id, NULL, (uint8_t *)(&buf), 3)){
		prints("spi nand:%lu-%lu: READ JEDEC ID, data access " \
		       "failed\n", ctrl_id, ce);
		return 1;
	}

	if (FTSPI020_wait_cmd_complete(ctrl_id, 10)) {
		prints("spi nand:%lu-%lu: READ JEDEC ID, wait cmd complete " \
		       "failed\n", ctrl_id, ce);
		return 1;
	}
#if 0
	if ((buf & 0xFF) != 0xC8) {
		prints("spi nand:%d-%d: READ JEDEC ID, manufacture id 0x%x " \
		       "not support.\n", ctrl_id, ce, buf);
		return 1;
	}
#endif

	idcode = buf;

scan_table:
	for (i = 0; i < ARRAY_SIZE(nand_spi_flash_table); i++) {
		params = &nand_spi_flash_table[i];

		if ((params->idcode1_2 == idcode) ||
			((params->idcode1_2 & 0xFF) == (idcode & 0xFF)))
			break;
	}

	if (i == ARRAY_SIZE(nand_spi_flash_table)) {
		prints("spi nand:%lu-%lu: Unsupported DataFlash ID 0x%04lx\n",
		       ctrl_id, ce, buf);
		return 1;
	}

	memset (flsh, 0, sizeof(struct spi_flash));
	flsh->name = params->name;
	flsh->code = params->idcode1_2;

	prints("Find Flash Name: %s.\n", flsh->name);

	flsh->spi_xfer = spi_xfer_spinand;
	flsh->read = nand_read;
	flsh->write = nand_write;
	flsh->read_spare = nand_read_spare;
	flsh->write_spare = nand_write_spare;
	flsh->copy_data = nand_intern_move;
	flsh->is_bad_block = nand_is_bad_block;
	flsh->scan_bad_blocks = nand_scan_bad_blocks;
	flsh->erase = nand_erase;
	flsh->erase_all = nand_erase_all;
	flsh->report_status = nand_report_status;
	flsh->get_string = spinand_action_get_string;


	flsh->page_size = params->page_size;
	flsh->nr_pages = params->nr_pages ;
	flsh->size = params->page_size * params->nr_pages;
	flsh->erase_block_size = params->block_size;

	flsh->rd_cmds = params->rd_cmds;
	flsh->max_rd_type = params->max_rd_cmds;
	flsh->wr_cmds = params->wr_cmds;
	flsh->max_wr_type = params->max_wr_cmds;
	flsh->er_cmds = params->er_cmds;
	flsh->max_er_type = params->max_er_cmds;

	flsh->isnand = 1;
	flsh->ctrl_id = ctrl_id;
	flsh->ce = ce;

	/*
	 * Read Protection Register/Status Register-1 to make sure:
	 * (1) S1-WPE bit is set to zero for QUAD functions enabled.
	 * (2) S3-S6 (BP0-BP3) is set to zero. Default is one. Block protected.
	 */
	if (spinand_get_feature(flsh, SPINAND_FEA_PROTECTION, &cfg)) {
		prints("%s: 1-Failed to read Protection register\n", flsh->name);
		return 1;
	}

	prints("%s: Protection Registers val: 0x%02x\n", flsh->name, cfg);

	changed = 0;

	if ((idcode & 0xFF) != 0xC8) { //GigaDevice
		if (cfg & (SPINAND_BL_SRP0 | SPINAND_BL_SRP1)) {
			prints("%s: Status Registers protect\n", flsh->name);
			cfg &= ~(SPINAND_BL_SRP0 | SPINAND_BL_SRP1);
			changed = 1;
		} else {
			prints("%s: Status Registers NOT protect\n", flsh->name);
		}

		if (cfg & SPINAND_BL_WPE) {
			prints("%s: WPE=1 QUAD functions are disabled\n", flsh->name);
			cfg &= ~SPINAND_BL_WPE;
			changed = 1;
		} else {
			prints("%s: WPE=0 QUAD functions are enabled\n", flsh->name);
		}
	}

	if (changed) {
		cmd[0] = SPINAND_WRITE_ENABLE;
		if (spi_flash_cmd(flsh, cmd, NULL, 0))
			return 1;

		cmd[0] = SPINAND_SET_FEATURE;
		cmd[1] = SPINAND_FEA_PROTECTION;
		if (spi_flash_cmd_write(flsh, cmd, (const void *) &cfg, 1)) {
			prints("failed.\n");
			return 1;
		}
	}

	if (cfg & SPINAND_BL_BP0TO2) {
		prints("%s: Blocks are protected, disabling it ...\n", flsh->name);
		spinand_set_block_protect(flsh, 0);
	}

	cfg = 0;
	if (spinand_get_feature(flsh, SPINAND_FEA_PROTECTION, &cfg)) {
		prints("%s: 2-Failed to read Protection register\n", flsh->name);
		return 1;
	}

	prints("%s: Protection Registers val: 0x%02x\n", flsh->name, cfg);

	/*
	 * Read Configuration Register/Status Register-2 to make sure:
	 * (1) S3-BUF bit is set to one for normal READ mode.
	 * (2) S3-S6 (BP0-BP3) is set to zero. Default is one. Block protected.
	 */

	cfg = 0;
	if (spinand_get_feature(flsh, SPINAND_FEA_CFG, &cfg)) {
		prints("%s: 2-Failed to read Protection register\n", flsh->name);
		return 1;
	}
	prints("%s: Configuration Registers val: 0x%02x\n", flsh->name, cfg);

	if ((idcode & 0xFF) != 0xC8) { //GigaDevice
		if (cfg & SPINAND_CFG_BUF_MODE) {
			prints("%s: BUF=1 Buffer READ mode\n", flsh->name);
		} else {
			prints("%s: BUF=0 Continuous READ mode\n", flsh->name);
			cfg |= SPINAND_CFG_BUF_MODE;

			cmd[0] = SPINAND_WRITE_ENABLE;
			if (spi_flash_cmd(flsh, cmd, NULL, 0))
				return 1;

			cmd[0] = SPINAND_SET_FEATURE;
			cmd[1] = SPINAND_FEA_CFG;
			if (spi_flash_cmd_write(flsh, cmd, (const void *) &cfg, 1)) {
				prints("failed.\n");
				return 1;
			}
		}
	}

	if (cfg & SPINAND_CFG_ECC_EN) {
		prints("%s: ECC is enabled\n", flsh->name);
	} else {
		prints("%s: ECC is disabled\n", flsh->name);
		spinand_set_fea_en(flsh, SPINAND_CFG_ECC_EN, 1);
	}

	if ((idcode & 0xFF) == 0xC8) { //GigaDevice
		if (cfg & SPINAND_CFG_QE_EN) {
			prints("%s: QE is enabled\n", flsh->name);
		} else {
			prints("%s: QE is disabled\n", flsh->name);
			spinand_set_fea_en(flsh, SPINAND_CFG_QE_EN, 1);
		}
	}

	cfg = 0;
	if (spinand_get_feature(flsh, SPINAND_FEA_CFG, &cfg)) {
		prints("%s: 2-Failed to read Protection register\n", flsh->name);
		return 1;
	}
	prints("%s: Configuration Registers val: 0x%02x\n", flsh->name, cfg);

	return 0;
}
