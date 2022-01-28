/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2019/07/15   BingYao           ZYNQ board.
 * -------------------------------------------------------------------------
 */
#include "ftspi020.h"

#if defined FTSPI020_MICRON_H

#include "ftspi020_cntr.h"

extern int32_t nor_read_status_register(struct spi_flash *flash,
					uint32_t bit_pos, uint8_t *status);
extern int32_t nor_write_status_register(struct spi_flash *flash,
					 uint32_t bit_pos, uint8_t *val);

static int micron_check_status_till_ready(struct spi_flash *flash,
					  uint32_t wait_ms);

static uint8_t extended_rd_cmds[] = {
	MICRON_READ, MICRON_FAST_READ, MICRON_DUAL_OUT_READ, MICRON_DUAL_IO_READ,
	MICRON_QUAD_OUT_READ, MICRON_QUAD_IO_READ};

static uint8_t dualspi_rd_cmds[] = {
	MICRON_FAST_READ, MICRON_DUAL_OUT_READ, MICRON_DUAL_IO_READ};

static uint8_t quadspi_rd_cmds[] = {
	MICRON_FAST_READ, MICRON_QUAD_OUT_READ, MICRON_QUAD_IO_READ};

static uint8_t extended_wr_cmds[] = {
	MICRON_PAGE_PROGRAM, MICRON_DUAL_PAGE_PROGRAM, MICRON_QUAD_PAGE_PROGRAM};

static uint8_t dualspi_wr_cmds[] = {
	MICRON_PAGE_PROGRAM, MICRON_DUAL_PAGE_PROGRAM};

static uint8_t quadspi_wr_cmds[] = {
	MICRON_PAGE_PROGRAM, MICRON_QUAD_PAGE_PROGRAM};

static uint8_t generic_er_cmds[] = {
	MICRON_ERASE_SUBSECTOR, MICRON_ERASE_SECTOR};

static const struct nor_spi_flash_params micron_spi_flash_table[] = {
	{0x18BA20, 256, 65536, 4096, 65536, 50000, 1000, 200,
	 0, 0xff, 0xff, "N25Q128A",
	 extended_rd_cmds, sizeof(extended_rd_cmds),
	 extended_wr_cmds, sizeof(extended_wr_cmds),
	 generic_er_cmds, sizeof(generic_er_cmds)},
};

char *micr_er_string[N25Q_MAX_ERASE_TYPE + 1] = {
	"4KB Sector Erase",
	"64KB Sector Erase",
	"Invalid erase type"
};

char *micr_wr_dual_string[N25Q_MAX_WRITE_TYPE + 1] = {
	"Page Program",
	"Dual Program",
	"Invalid write type"
};

char *micr_wr_qpi_string[N25Q_MAX_WRITE_TYPE + 1] = {
	"Page Program",
	"Quad Program",
	"Invalid write type"
};

char *micr_wr_string[N25Q_MAX_WRITE_TYPE + 1] = {
	"Page Program",
	"Dual Program",
#ifdef FTSPI020_SUPPORT_QUAD
	"Quad Program",
#endif
	"Invalid write type"
};

char *micr_rd_dual_string[N25Q_MAX_READ_TYPE + 1] = {
	"Fast Read",
	"Dual Output Read",
	"Dual IO Read",
	"Invalid read type"
};

char *micr_rd_qpi_string[N25Q_MAX_READ_TYPE + 1] = {
	"Fast Read",
	"Quad Output Read",
	"Quad IO Read",
	"Invalid read type"
};

char *micr_rd_string[N25Q_MAX_READ_TYPE + 1] = {
	"Read",
	"Fast Read",
	"Dual Output Read",
	"Dual IO Read",
#ifdef FTSPI020_SUPPORT_QUAD
	"Quad Output Read",
	"Quad IO Read",
#endif
	"Invalid read type"
};

uint32_t micr_wait_ms;
static int cont_read;

static char *n25q_action_get_string(struct spi_flash *flash,
				    uint32_t act, uint32_t type)
{
	if (act == WRITE) {
		if (flash->qpi_mode == 4) {
			return micr_wr_qpi_string[type];
		} if (flash->qpi_mode == 2) {
			return micr_wr_dual_string[type];
		} else {
			return micr_wr_string[type];
		}
	} else if (act == READ) {
		if (flash->qpi_mode == 4) {
			return micr_rd_qpi_string[type];
		} if (flash->qpi_mode == 2) {
			return micr_rd_dual_string[type];
		} else {
			return micr_rd_string[type];
		}
	} else {
		return micr_er_string[type];
	}
}

static int micron_4byte_mode(struct spi_flash *flash, int en)
{
	return 0;
}

int micron_qpi_mode(struct spi_flash *flash, int en)
{
	uint8_t cmd[1];
	uint8_t *sts = (uint8_t *) FTSPI020_DMA_ADDR;
	uint8_t vol;

	if ((en != 0) && (en != 2) && (en != 4)){
		prints("%s: QPI mode %d error.\n", flash->name, en);
		return 0;
	}
#if 0
	if (en == flash->qpi_mode) {
		prints("%s: Already %s QPI mode %d.\n", flash->name,
		       en ? "in" : "exit", en);
		return 0;
	}
#endif
	micr_wait_ms = 100;
	cmd[0] = MICRON_READ_ENH_VOL_CONFIG;
	sts[0] = 0;
	if (spi_flash_cmd_read(flash, cmd, sts, 1)) {
		return 1;
	}
	vol = sts[0];
	prints("%s: Enhanced Volatile Config(0x%x): 0x%x\n", flash->name, cmd[0], vol);

	cmd[0] = MICRON_WRITE_ENABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	cmd[0] = MICRON_READ_STATUS;
	sts[0] = 0;
	if (spi_flash_cmd_read(flash, cmd, sts, 1)) {
		return 1;
	}

	if (!(sts[0] & MICRON_STS_WEL)) {
		prints("%s: WEL bit not set, can not write\n", flash->name);
		return 1;
	}

	// 1 = Disabled, 0 = Enabled
	// If bits 6 and 7 are both set to 0, the device operates in quad I/O.
	switch (en) {
	case 4:
		vol &= ~(MICRON_ENH_VOL_CFG_QUAD_IO | MICRON_ENH_VOL_CFG_DUAL_IO);
		flash->rd_cmds = quadspi_rd_cmds;
		flash->max_rd_type = sizeof(quadspi_rd_cmds);
		flash->wr_cmds = quadspi_wr_cmds;
		flash->max_wr_type = sizeof(quadspi_wr_cmds);
		break;
	case 2:
		vol &= ~MICRON_ENH_VOL_CFG_DUAL_IO;
		vol |= MICRON_ENH_VOL_CFG_QUAD_IO;
		flash->rd_cmds = dualspi_rd_cmds;
		flash->max_rd_type = sizeof(dualspi_rd_cmds);
		flash->wr_cmds = dualspi_wr_cmds;
		flash->max_wr_type = sizeof(dualspi_wr_cmds);
		break;
	default:
		vol |= (MICRON_ENH_VOL_CFG_QUAD_IO | MICRON_ENH_VOL_CFG_DUAL_IO);
		flash->rd_cmds = extended_rd_cmds;
		flash->max_rd_type = sizeof(extended_rd_cmds);
		flash->wr_cmds = extended_wr_cmds;
		flash->max_wr_type = sizeof(extended_wr_cmds);
		break;
	}

	cmd[0] = MICRON_WRITE_ENH_VOL_CONFIG;
	sts[0] = vol;
	micr_wait_ms = 200;
	if (spi_flash_cmd_write(flash, cmd,
				(const void *) sts, 1)) {
		goto err;
	}

	flash->qpi_mode = en;

	micr_wait_ms = 100;
	cmd[0] = MICRON_READ_ENH_VOL_CONFIG;
	sts[0] = 0;
	if (spi_flash_cmd_read(flash, cmd, sts, 1)) {
		goto err;
	}

	prints("%s: Enhanced Volatile Config(0x%x): 0x%x\n", flash->name, cmd[0], sts[0]);

	switch (en) {
	case 4:
		if (sts[0] & MICRON_ENH_VOL_CFG_QUAD_IO) {
			prints("%s: Enter quad io failed 0x%x.\n", flash->name,
				sts[0]);
			goto err;
		}

		break;
	case 2:
		if (sts[0] & MICRON_ENH_VOL_CFG_DUAL_IO) {
			prints("%s: Enter dual io failed 0x%x.\n", flash->name,
				sts[0]);
			goto err;
		}

		break;
	default:
		if (!(sts[0] & (MICRON_ENH_VOL_CFG_QUAD_IO | MICRON_ENH_VOL_CFG_DUAL_IO))) {
			prints("%s: Exit dual/quad io failed 0x%x.\n", flash->name,
				sts[0]);
			goto err;
		}
		break;
	}

	return 0;

err:
	flash->rd_cmds = extended_rd_cmds;
	flash->max_rd_type = sizeof(extended_rd_cmds);
	flash->wr_cmds = extended_wr_cmds;
	flash->max_wr_type = sizeof(extended_wr_cmds);

	return 1;
}

static int spi_xfer_n25q(struct spi_flash *flash, unsigned int len,
			const void *dout, void *din,unsigned int *flags)
{

	struct ftspi020_cmd spi_cmd;
	uint8_t *data_out = (uint8_t *) dout;
	uint8_t code, addr4;

	memset(&spi_cmd, 0, sizeof(struct ftspi020_cmd));

	/* No DTR and QPI mode for DAMR read */
	if (*flags & SPI_XFER_DAMR_READ) {
		spi_cmd.damr_read = 1;
	}

	if (*flags & SPI_XFER_CMD_STATE) {
		spi_cmd.start_ce = flash->ce;
		spi_cmd.ins_code = code = *data_out;
		spi_cmd.spi_addr = (*(data_out + 3) << 16) |
				   (*(data_out + 2) << 8) |
				   (*(data_out + 1));
		addr4 = *(data_out + 4);

		switch (code) {
		case CMD_READ_ID:	// Read JEDEC ID
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = 3;
			break;
		case CMD_RESET:	// Reset the Flash
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			break;
		case MICRON_WRITE_ENABLE:
		case MICRON_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 4)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MICRON_ERASE_SUBSECTOR:
		case MICRON_ERASE_SECTOR:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 4)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MICRON_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 4)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MICRON_READ_STATUS:
		case MICRON_READ_FLAG_STATUS:
		case MICRON_READ_VOL_CONFIG:
		case MICRON_READ_ENH_VOL_CONFIG:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.data_cnt = len;
			spi_cmd.read_status = 1;

			if (flash->qpi_mode == 4)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;

			//Read status can't use DAMR
			spi_cmd.damr_read = 0;
			*flags &= ~SPI_XFER_DAMR_READ;
			break;
		case MICRON_WRITE_STATUS:
		case MICRON_WRITE_VOL_CONFIG:
		case MICRON_WRITE_NONVOL_CONFIG:
		case MICRON_WRITE_ENH_VOL_CONFIG:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.data_cnt = len;
			if (flash->qpi_mode == 4)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MICRON_PAGE_PROGRAM:
		case MICRON_DUAL_PAGE_PROGRAM:
		case MICRON_QUAD_PAGE_PROGRAM:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			if (flash->qpi_mode == 4) {
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else if (flash->qpi_mode == 2) {
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			} else {
				if (code == MICRON_DUAL_PAGE_PROGRAM)
					spi_cmd.spi_mode = spi_operate_dual_mode;
				else if (code == MICRON_QUAD_PAGE_PROGRAM)
					spi_cmd.spi_mode = spi_operate_quad_mode;
				else
					spi_cmd.spi_mode = spi_operate_serial_mode;
			}
			break;
		case MICRON_READ:
		// Read Non-Volatile Config return 2 bytes
		case MICRON_READ_NONVOL_CONFIG:
			if (code != MICRON_READ_NONVOL_CONFIG) {
				if (flash->addr_width == 4) {
					spi_cmd.spi_addr |= (addr4 << 24);
					spi_cmd.addr_len = addr_4byte;
				} else {
					spi_cmd.addr_len = addr_3byte;
				}
			} else {
				spi_cmd.read_status = 1;
				spi_cmd.addr_len = no_addr_state;
				//Read status can't use DAMR
				spi_cmd.damr_read = 0;
				*flags &= ~SPI_XFER_DAMR_READ;
			}
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;

			if (code == MICRON_READ_NONVOL_CONFIG) {
				if (flash->qpi_mode == 4)
					spi_cmd.spi_mode = spi_operate_ins4bit_mode;
				else if (flash->qpi_mode == 2)
					spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			}
			break;
		case MICRON_FAST_READ:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 4) {
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
				spi_cmd.dum_2nd_cyc = 10;
			} else if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MICRON_DUAL_OUT_READ:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_dual_mode;
			break;
		case MICRON_QUAD_OUT_READ:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 4) {
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
				spi_cmd.dum_2nd_cyc = 10;
			} else
				spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case MICRON_DUAL_IO_READ:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode == 2)
				spi_cmd.spi_mode = spi_operate_ins2bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case MICRON_QUAD_IO_READ:
			if (flash->addr_width == 4) {
				spi_cmd.spi_addr |= (addr4 << 24);
				spi_cmd.addr_len = addr_4byte;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.dum_2nd_cyc = 10;
			if (flash->qpi_mode == 4) {
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else {
				spi_cmd.spi_mode = spi_operate_quadio_mode;
			}
			break;
		default:
			prints("%s: Wrong Command Code 0x%x.\n",
				flash->name, spi_cmd.ins_code);
			return 1;
		}

		FTSPI020_issue_cmd(flash->ctrl_id, &spi_cmd);

	} else if (*flags & SPI_XFER_DATA_STATE) {
		if (FTSPI020_data_access(flash->ctrl_id, (uint8_t *) dout, (uint8_t *) din,
					 len))
			return 1;

	}

	if (*flags & SPI_XFER_CHECK_CMD_COMPLETE) {
		if (FTSPI020_wait_cmd_complete(flash->ctrl_id, micr_wait_ms))
			return 1;
	}
	return 0;
}

static int dataflash_read_fast_n25q(struct spi_flash *flash, uint8_t type,
				   uint32_t offset, uint32_t len, void *buf)
{
	int ret;
	uint8_t rd_cmd[5];

	if (type > flash->max_rd_type) {
		prints("Read type exceed max\n");
		return 2;
	}

	prints("%s: %s: %s(%02xh) %lu len @ %lu\n", flash->name,
	       (g_damr_read[flash->ctrl_id])? "DAMR" :
	       (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
	       flash->get_string(flash, READ, type), flash->rd_cmds[type],
	       len, offset);

	cont_read = 0;
	rd_cmd[0] = flash->rd_cmds[type];
	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
#if 0
	if (offset & 0xFF000000) {
		ret = micron_4byte_mode(flash, 1);
		rd_cmd[4] = ((offset & 0xFF000000) >> 24);
	} else {
		ret = micron_4byte_mode(flash, 0);
		rd_cmd[4] = 0;
	}

	if (micron_check_status_till_ready(flash, 100)) {
		prints ("%s: write type %s, set quad failed\n",
			flash->name, micr_wr_string[type]);
		return 1;
	}
#endif
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);

	return ret;
}

static int dataflash_write_fast_n25q(struct spi_flash *flash, uint8_t type,
				    uint32_t offset, uint32_t len, void *outbuf)
{
	int start_page, offset_in_start_page, len_each_times;
	uint32_t i, original_offset = offset, original_len = len;
	uint8_t wr_en_cmd[1], wr_cmd[5];
	uint8_t *buf = outbuf, *alignment_buf;

	prints("%s: %s: %s(%02xh) %lu bytes @ %lu\n", flash->name,
		(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		flash->get_string(flash, WRITE, type), flash->wr_cmds[type], original_len, original_offset);

	alignment_buf = (uint8_t *) FTSPI020_DMA_ADDR;
	start_page = offset / flash->page_size;
	offset_in_start_page = offset % flash->page_size;

	/**
	 *  This judgement, "if(len + offset_in_start_page <=
	 * flash->page_size)" is for the case of (offset + len)
	 * doesn't exceed the nearest next page boundary. 
	   0                                255
	   -------------------------------------
	   | | | | |.......................| | |
	   -------------------------------------
	   256                               511                                
	   -------------------------------------
	   | |*|*|*|.......................|*| |
	   -------------------------------------
	   512                               767
	   -------------------------------------
	   | | | | |.......................| | |
	   -------------------------------------
	 */
	if (len + offset_in_start_page <= flash->page_size) {
		len_each_times = len;
	} else {
		len_each_times = ((((start_page + 1) * flash->page_size) -
				 1) - offset) + 1;
	}

	wr_en_cmd[0] = MICRON_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	wr_cmd[0] = flash->wr_cmds[type];
	wr_cmd[1] = offset & 0xFF;
	wr_cmd[2] = ((offset & 0xFF00) >> 8);
	wr_cmd[3] = ((offset & 0xFF0000) >> 16);
#if 0
	if (offset & 0xFF000000) {
		ret = micron_4byte_mode(flash, 1);
		wr_cmd[4] = ((offset & 0xFF000000) >> 24);
	} else {
		ret = micron_4byte_mode(flash, 0);
		wr_cmd[4] = 0;
	}
#endif
	if (spi_flash_cmd_write(flash, wr_cmd, buf, len_each_times))
		return 1;

	buf = buf + len_each_times;

	do {
		len = len - len_each_times;
		offset = ((offset / flash->page_size) + 1) *
			  flash->page_size;
		if (len < (flash->page_size)) {
			if (len == 0)
				break;
			else
				len_each_times = len;
		} else
			len_each_times = flash->page_size;

		// To avoid the "buff" isn't alignment.
		for (i = 0; i < len_each_times; i++) {
			alignment_buf[i] = buf[i];
		}

		micron_check_status_till_ready(flash, 100);

		wr_en_cmd[0] = MICRON_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		wr_cmd[0] = flash->wr_cmds[type];
		wr_cmd[1] = offset & 0xFF;
		wr_cmd[2] = ((offset & 0xFF00) >> 8);
		wr_cmd[3] = ((offset & 0xFF0000) >> 16);
		if (offset & 0xFF000000) {
			micron_4byte_mode(flash, 1);
			wr_cmd[4] = ((offset & 0xFF000000) >> 24);
		} else {
			micron_4byte_mode(flash, 0);
			wr_cmd[4] = 0;
		}

		if (spi_flash_cmd_write(flash, wr_cmd, alignment_buf,
					len_each_times))
			return 1;
		buf = buf + len_each_times;

	} while (1);

	return 0;
}

/*
 * If TBPARM=0, valid address for 4K and 8K erase is 0 to 0x1FFFF.
 */
static int dataflash_erase_fast_n25q(struct spi_flash *flash, uint8_t type,
				    uint32_t offset, uint32_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[5];
	uint8_t cmd_code;
	uint32_t wait_t;

	cmd_code = flash->er_cmds[type];
	if (cmd_code == MICRON_ERASE_SUBSECTOR) {
		erase_size = flash->erase_sector_size;
		wait_t = flash->sector_erase_time;
	} else if (cmd_code == MICRON_ERASE_SECTOR) {
		erase_size = flash->erase_block_size;
		wait_t = flash->block_erase_time;
	} else {
		prints("%s: %s @ 0x%lx\n", flash->name,
			micr_er_string[type], offset);
		return 1;
	}

	offset &= ~(erase_size - 1);

	if (micron_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = MICRON_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		/* Assume TBPARM=0, valid address for 4K and 8K erase
		   is 0 to 0x1FFFF.
		 */
		if ((addr > 0x1FFFF) && (cmd_code != MICRON_ERASE_SECTOR)) {
			cmd_code = MICRON_ERASE_SECTOR;
			type = 1;
			erase_size = 64 << 10;
			wait_t = 1000;
			addr &= ~(erase_size - 1);
		}

		er_cmd[0] = cmd_code;
		er_cmd[1] = addr & 0xFF;
		er_cmd[2] = ((addr & 0xFF00) >> 8);
		er_cmd[3] = ((addr & 0xFF0000) >> 16);
		if (addr & 0xFF000000) {
			micron_4byte_mode(flash, 1);
			er_cmd[4] = ((addr & 0xFF000000) >> 24);
		} else {
			micron_4byte_mode(flash, 0);
			er_cmd[4] = 0;
		}

		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		/* Top/Bottom 64K erase requires 2080 ms to 10400 ms */
		if (addr < 0x20000)
			micron_check_status_till_ready(flash, 10400 );
		else
			micron_check_status_till_ready(flash, wait_t);

		prints("%s: %s(%02xh) @ 0x%x size 0x%x\n", flash->name,
				flash->get_string(flash, ERASE, type), cmd_code, addr, erase_size);

	}

	return ret;
}

static int dataflash_report_status_n25q(struct spi_flash *flash)
{
	int ret;
	uint8_t *sts = (uint8_t *) FTSPI020_DMA_ADDR;
	uint8_t rd_sts_cmd[1];

	micr_wait_ms = 100;
	rd_sts_cmd[0] = MICRON_READ_STATUS;
	if (spi_flash_cmd_read(flash, rd_sts_cmd, sts, 1)) {
		prints("%s: Status Register(0x%x): read failed\n", flash->name, rd_sts_cmd[0]);
		return 1;
	}
	prints("%s: Status Register(0x%x): 0x%x\n", flash->name, rd_sts_cmd[0], sts[0]);

	micr_wait_ms = 100;
	rd_sts_cmd[0] = MICRON_READ_NONVOL_CONFIG;
	ret = spi_flash_cmd_read(flash, rd_sts_cmd, sts, 2);
	prints("%s: Non-Volatile Config(0x%x): 0x%x 0x%x\n", flash->name, rd_sts_cmd[0], sts[0], sts[1]);

	micr_wait_ms = 100;
	rd_sts_cmd[0] = MICRON_READ_VOL_CONFIG;
	if (spi_flash_cmd_read(flash, rd_sts_cmd, sts, 1)) {
		prints("%s: Volatile Config(0x%x): read failed\n", flash->name, rd_sts_cmd[0]);
		return 1;
	}
	prints("%s: Volatile Config(0x%x): 0x%x\n", flash->name, rd_sts_cmd[0], sts[0]);

	micr_wait_ms = 100;
	rd_sts_cmd[0] = MICRON_READ_ENH_VOL_CONFIG;
	if (spi_flash_cmd_read(flash, rd_sts_cmd, sts, 1)) {
		prints("%s: Enhance Volatile Config(0x%x): read failed\n", flash->name, rd_sts_cmd[0]);
		return 1;
	}
	prints("%s: Enhanced Volatile Config(0x%x): 0x%x\n", flash->name, rd_sts_cmd[0], sts[0]);
	return ret;
}

static int dataflash_erase_all_n25q(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	micr_wait_ms = 50000;

	er_all_cmd[0] = MICRON_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = MICRON_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	if (micron_check_status_till_ready(flash, 75000))
		return 1;

	return 0;
}

static int micron_check_status_till_ready(struct spi_flash * flash,
					  uint32_t wait_ms)
{
#if 0
	uint8_t rd_sts_cmd[1];

	micr_wait_ms = wait_ms;
	FTSPI020_busy_location(flash->ctrl_id, BUSY_BIT0);

	rd_sts_cmd[0] = MICRON_READ_STATUS;

	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		prints("%s: Failed to check status by SW\n",
				flash->name);
		return 1;
	}
#else
	uint8_t sts = 1;

	do {
		if (nor_read_status_register(flash, 0, &sts))
			return 1;

	} while (sts & (1 << flash->sts_bsy_bit));
#endif

	return 0;
}

int32_t spi_flash_probe_micron(struct spi_flash *nq25f, uint8_t * code)
{
	uint8_t i;
	uint32_t idcode;
	const struct nor_spi_flash_params *params;

	memcpy(&idcode, code, 3);
	idcode &= ~0xff000000;

	for (i = 0; i < ARRAY_SIZE(micron_spi_flash_table); i++) {
		params = &micron_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == ARRAY_SIZE(micron_spi_flash_table)) {
		prints("Micron: Unsupported DataFlash ID 0x%02lx\n", idcode);
		return 1;
	}

	nq25f->name = params->name;
	nq25f->code = params->idcode1_2;
	prints("Find Flash Name: %s.\n", nq25f->name);

	nq25f->spi_xfer = spi_xfer_n25q;
	nq25f->read = dataflash_read_fast_n25q;
	nq25f->write = dataflash_write_fast_n25q;
	nq25f->erase = dataflash_erase_fast_n25q;
	nq25f->erase_all = dataflash_erase_all_n25q;
	nq25f->report_status = dataflash_report_status_n25q;
	nq25f->get_string = n25q_action_get_string;

	nq25f->page_size = params->page_size;
	nq25f->nr_pages = params->nr_pages ;
	nq25f->size = params->page_size * params->nr_pages;
	nq25f->erase_sector_size = params->sector_size;
	nq25f->erase_block_size = params->block_size;

	nq25f->chip_erase_time = params->chip_erase_time;
	nq25f->block_erase_time = params->block_erase_time;
	nq25f->sector_erase_time = params->sector_erase_time;

	nq25f->sts_bsy_bit = params->sts_bsy_bit;
	nq25f->sts_qe_bit = params->sts_qe_bit;
	nq25f->sts_addr_bit = params->sts_addr_bit;

	nq25f->rd_cmds = params->rd_cmds;
	nq25f->max_rd_type = params->max_rd_cmds;
	nq25f->wr_cmds = params->wr_cmds;
	nq25f->max_wr_type = params->max_wr_cmds;
	nq25f->er_cmds = params->er_cmds;
	nq25f->max_er_type = params->max_er_cmds;

	nq25f->qpi_mode = 0;
	nq25f->addr_width = 3;

{
		uint8_t sts;

		sts = 0;
		if (nor_write_status_register(nq25f, 2, &sts))
			return 0;

		if (nor_read_status_register(nq25f, 0, &sts))
			return 0;

		prints("sts= %x\n", sts);
}
	return 0;
}

#endif
