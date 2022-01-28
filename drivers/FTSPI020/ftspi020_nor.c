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
#include <malloc.h>
#include <stdlib.h>

#include "ftspi020.h"

#if defined(FTSPI020_NOR_H)

#include "ftspi020_cntr.h"

#define FTSPI020_ERASE_AUTOPOLL	0

static int nor_check_status_till_ready(struct spi_flash *flash,
				       uint32_t wait_ms);

static uint8_t generic_rd_cmds[] = {
	NOR_READ_DATA, NOR_FAST_READ, NOR_READ_DUAL, NOR_READ_DUAL_IO,
	NOR_READ_QUAD, NOR_READ_QUAD_IO};

static uint8_t dtr_rd_cmds[] = {
	NOR_READ_DATA, NOR_FAST_READ, NOR_READ_DUAL, NOR_READ_DUAL_IO,
	NOR_READ_QUAD, NOR_READ_QUAD_IO, NOR_DDR_FAST_READ,
	NOR_DDR_DUAL_IO_READ, NOR_DDR_QUAD_IO_READ};

static uint8_t eon_rd_cmds[] = {
	NOR_READ_DATA, NOR_FAST_READ, NOR_READ_DUAL, NOR_READ_DUAL_IO,
	NOR_READ_QUAD_IO};

static uint8_t mxic_rd_cmds[] = {
	NOR_READ_DATA, NOR_FAST_READ, NOR_READ_DUAL_IO, NOR_READ_QUAD_IO};

static uint8_t qpi_rd_cmds[] = {
	//NOR_FAST_READ, NOR_READ_QUAD_IO, NOR_DDR_FAST_READ};
	NOR_FAST_READ, NOR_READ_QUAD_IO};

static uint8_t mx25l_qpi_rd_cmds[] = {
	NOR_READ_QUAD_IO};

static uint8_t winbond_qpi_rd_cmds[] = {
	NOR_FAST_READ, NOR_READ_QUAD_IO, NOR_DDR_FAST_READ};

static uint8_t qpi_wr_cmds[] = {
	NOR_WRITE_PAGE};

static uint8_t generic_wr_cmds[] = {
	NOR_WRITE_PAGE, NOR_WRITE_PAGE_QUAD};

static uint8_t mxic_wr_cmds[] = {
	NOR_WRITE_PAGE};

static uint8_t basic_wr_cmds[] = {
	NOR_WRITE_PAGE};

//Erase commands list
#define COMMON_ER_CMDS	NOR_ERASE_SECTOR, NOR_ERASE_64KB_BLOCK

static uint8_t generic_er_cmds[] = {
	COMMON_ER_CMDS};

static uint8_t uniform_er_cmds[] = {
	NOR_ERASE_64KB_BLOCK};

static uint8_t has32kb_er_cmds[] = {
	COMMON_ER_CMDS, NOR_ERASE_32KB_BLOCK};

static const struct nor_spi_flash_params nor_spi_flash_table[] = {
	     {0x1740C8, 256, 32768, 4096, 65536, 25000, 300, 100, //No QPI mode
	     0, 9, 0xff, "GD25Q64C",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
		 has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1760C8, 256, 32768, 4096, 65536, 25000, 350, 150, //Has QPI mode
	     0, 9, 0xff, "GD25LQ64C",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1840C8, 256, 65536, 4096, 65536, 60000, 300, 50,
	     0, 9, 0xff, "GD25Q128C",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x1940C8, 256, 131072, 4096, 65536, 80000, 300, 50,
	     0, 6, 13, "GD25Q256C",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x1960EF, 256, 131072, 4096, 65536, 400000, 2000, 400,
	     0, 9, 16, "W25Q256JW",
	     dtr_rd_cmds, sizeof(dtr_rd_cmds),
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x1970EF, 256, 131072, 4096, 65536, 400000, 2000, 400,
	     0, 9, 16, "W25Q256JV-DTR",
	     dtr_rd_cmds, sizeof(dtr_rd_cmds),
	     //generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1940EF, 256, 131072, 4096, 65536, 80000, 300, 100,
	     0, 9, 16, "W25Q256FV",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x1840EF, 256, 65536, 4096, 65536, 45000, 180, 100,
	     0, 9, 0xff, "W25Q128FV-SPI",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1860EF, 256, 65536, 4096, 65536, 68000, 350, 100, //32K erase 197ms, 64K erase 308ms
	     0, 9, 0xff, "W25Q128FV-QPI",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1740EF, 256, 32768, 4096, 65536, 20000, 180, 100,
	     0, 9, 0xff, "W25Q64FV-SPI",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1760EF, 256, 32768, 4096, 65536, 35000, 350, 100, //32K erase 197ms, 64K erase 308ms
	     0, 9, 0xff, "W25Q64FV-QPI",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x174001, 256, 32768, 4096, 65536, 64000, 550, 75,
	     0, 9, 0xff, "S25FL164K",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     basic_wr_cmds, sizeof(basic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x182001, 256, 65536, 4096, 65536, 165000, 10400, 1500,
	     0, 9, 31, "S25FL128SAG",
	     dtr_rd_cmds, sizeof(dtr_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     uniform_er_cmds, sizeof(uniform_er_cmds)},
	     //{0x190201, 256, 131072, 4096, 65536, 165000, 10400, 650,
	     {0x190201, 256, 65536, 4096, 65536, 165000, 10400, 650,
	     0, 9, 31, "S25FL256SAG",
	     dtr_rd_cmds, sizeof(dtr_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
	     {0x3725c2, 256, 32768, 4096, 65536, 50000, 700, 60,
	     0, 6, 0xff, "MX25U6435F",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1720C2, 256, 32768, 4096, 65536, 50000, 700, 60,
	     0, 6, 0xff, "MX25L6406E",
	     mxic_rd_cmds, sizeof(mxic_rd_cmds),
	     mxic_wr_cmds, sizeof(mxic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1820C2, 256, 65536, 4096, 65536, 160000, 2000, 200,
	     0, 6, 0xff, "MX25L12835F",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x1920C2, 256, 131072, 4096, 65536, 160000, 2000, 200,
	     0, 6, 21, "MX25L25635F",
	     generic_rd_cmds, sizeof(generic_rd_cmds),
	     generic_wr_cmds, sizeof(generic_wr_cmds),
	     has32kb_er_cmds, sizeof(has32kb_er_cmds)},
	     {0x19701C, 256, 131072, 4096, 65536, 100000, 400, 100,
	     0, 6, 26, "EN25QH256",
	     eon_rd_cmds, sizeof(eon_rd_cmds),
	     basic_wr_cmds, sizeof(basic_wr_cmds),
	     generic_er_cmds, sizeof(generic_er_cmds)},
};

static uint32_t nor_wait_ms;

static char *nor_action_get_string(struct spi_flash *flash,
				   uint32_t act, uint32_t type)
{
	uint8_t cmd;

	if (act == WRITE) {
		cmd = flash->wr_cmds[type];

		switch(cmd) {
		case NOR_WRITE_PAGE:
			return "Page Program";
		case NOR_WRITE_PAGE_QUAD:
			return "Page Program Quad";
		default:
			return "Unknown Write Type";
		}
	} else if (act == READ) {
		cmd = flash->rd_cmds[type];

		switch(cmd) {
		case NOR_READ_DATA:
			return "Read";
		case NOR_FAST_READ:
			return "Fast Read";
		case NOR_READ_DUAL:
			return "Read Dual";
		case NOR_READ_DUAL_IO:
			return "Read Dual IO";
		case NOR_READ_QUAD:
			return "Read Quad";
		case NOR_READ_QUAD_IO:
			return "Read Quad IO";
		case NOR_DDR_FAST_READ:
			return "DTR Fast Read";
		case NOR_DDR_DUAL_IO_READ:
			return "DTR Fast Read Dual IO";
		case NOR_DDR_QUAD_IO_READ:
			return "DTR Fast Read Quad IO";
		default:
			return "Unknown Read Type";
		}
	} else if (act == ERASE) {
		cmd = flash->er_cmds[type];

		switch(cmd) {
		case NOR_ERASE_SECTOR:
			return "Erase Sector";
		case NOR_ERASE_32KB_BLOCK:
			return "Erase Block 32KB";
		case NOR_ERASE_64KB_BLOCK:
			return "Erase Block 64KB";
		default:
			return "Unknown Erase Type";
		}
	}

	return "Unknown Action";
}

int32_t nor_read_status_register(struct spi_flash *flash,
				 uint32_t bit_pos, uint8_t *status)
{
	uint8_t rd_sts_cmd[1];
	uint8_t reg_num;
	uint8_t *sts = (uint8_t *) FTSPI020_DMA_ADDR;

	if (bit_pos == 0xff)
		return 1;

	reg_num = bit_pos / 8;
	switch (reg_num){
	case 0:	
		rd_sts_cmd[0] = NOR_READ_STATUS1;
		break;
	case 1:
		//MXIC Read Configuration register
		if ((flash->code & 0xFFFF) == 0x20C2) {
			rd_sts_cmd[0] = NOR_READ_STATUS3;
		} else {
			rd_sts_cmd[0] = NOR_READ_STATUS2;
		}
		break;
	case 2:
		rd_sts_cmd[0] = NOR_READ_STATUS3;
		break;
	default:
		//spansion Bank Address Register
		if ((flash->code & 0xff) == 0x01)
			rd_sts_cmd[0] = NOR_READ_BANK_ADDR;
		//EON Information Register
		else if ((flash->code & 0xff) == 0x1C)
			rd_sts_cmd[0] = NOR_READ_INFO_REG_EON;
		else
			return 1;
	}
	
	nor_wait_ms = 5000;
	if (spi_flash_cmd_read(flash, rd_sts_cmd, sts, 1))
		return 1;

	*status = *sts;

	return 0;
}

int32_t nor_write_status_register(struct spi_flash *flash,
				  uint32_t bit_pos, uint8_t *val)
{
	uint8_t wr_sts_cmd[1];
	uint8_t reg_num, sts[2];
	uint32_t wr_len;

	if (bit_pos == 0xff)
		return 1;

	wr_sts_cmd[0] = NOR_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0))
		return 1;

	if (nor_check_status_till_ready(flash, 200))
		return 1;

	if (nor_read_status_register(flash, 1, &sts[0]))
		return 1;

	if (!(sts[0] & 0x2)) {
		prints("%s: WEL bit not set, can not write\n", flash->name);
		return 1;
	}
	
	//spansion and mxic only has Write Status 1 with 16 bits value
	if (((flash->code & 0xff) == 0x01) ||
	//if ((flash->code == 0x182001) ||
	    (flash->code == 0x1820C2)){

		if (bit_pos < 24) {
			wr_sts_cmd[0] = NOR_WRITE_STATUS1;
			wr_len = 2;
			sts[0] = val[0];
			sts[1] = val[1];
		} else {
			//Bank Register for 4 byte address
			wr_sts_cmd[0] = NOR_WRITE_BANK_ADDR;
			wr_len = 1;
			sts[0] = val[0];
		}
	} else {
		wr_len = 1;
		sts[0] = val[0];
		reg_num = bit_pos / 8;

		switch (reg_num){
		case 0:
			wr_sts_cmd[0] = NOR_WRITE_STATUS1;
			break;
		case 1:
			wr_sts_cmd[0] = NOR_WRITE_STATUS2;
			break;
		case 2:
			wr_sts_cmd[0] = NOR_WRITE_STATUS3;
			break;
		default:
			//spansion Bank Address Register
			if ((flash->code & 0xff) == 0x01)
				wr_sts_cmd[0] = NOR_WRITE_BANK_ADDR;
			else
				return 1;
		}
	}

	if (g_debug > 2)
		prints(" Write status 0x%x 0x%x\n", sts[0], sts[1]);

	nor_wait_ms = 200;
	if (spi_flash_cmd_write(flash, wr_sts_cmd,
				(const void *) sts, wr_len))
		return 1;

	/* Wait for Write Status Complete */
	if (nor_check_status_till_ready(flash, 200))
		return 1;

	wr_sts_cmd[0] = NOR_WRITE_DISABLE;
	if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0))
		return 1;

	if (nor_check_status_till_ready(flash, 200))
		return 1;

	return 0;
}

/*
 * Have 2 ways for 4-byte mode:
 * 1. Enter 4-byte mode (EN4B) (B7h)
 *    - Controller send 4 bytes address 
 *    - May use for GD, Winbond.
 *    - Spansion has no EN4B must use method 2.
 * 2. EON : Enter High Bank Latch mode (ENHBL) (67h)
 *    - 16MBytes offset becomes zero
 *    - controller send 3 bytes address
 *   GigaDevice : Write Extended Register (C5h)
 *    - Write A24 of address bit to this register
 *   Spansion : Write Bank Address Register (17h)
 *    - Set BAR[7] EXTADD = 1, controller send 4 bytes address.
 *    - May use 4-bytes address read/write insruction code
 */
static int nor_4byte_mode(struct spi_flash *flash, int en)
{
	uint8_t cmd[1];
	uint8_t info, addr_mode_pos;

	if (nor_read_status_register(flash, flash->sts_addr_bit, &info))
		return 1;

	addr_mode_pos = 1 << (flash->sts_addr_bit % 8);


	if ((en && (info & addr_mode_pos)) ||
	    (!en && !(info & addr_mode_pos))) {
		prints("%s: %s 4-byte mode, config no changed(0x%x)\n", flash->name,
		       en ? "Enter" : "Exit", info);
		return 0;
	}

	if ((flash->code & 0xff) == 0x01) { /* Spansion */
		if (en)
			info |= addr_mode_pos;
		else
			info &= ~addr_mode_pos;

		if (nor_write_status_register(flash, flash->sts_addr_bit,
					      &info))
			return 1;

	} else {
		if (en)
			cmd[0] = NOR_EN4B;
		else
			cmd[0] = NOR_EX4B;

		if (spi_flash_cmd(flash, cmd, NULL, 0))
			return 1;
	}

	info = 0;
	if (nor_read_status_register(flash, flash->sts_addr_bit, &info))
		return 1;
	if (g_debug > 2)
		prints(" Read back info 0x%x \n", info);
	
	if (en && !(info & addr_mode_pos)) {
		prints("%s: Enter 4-byte Mode failed(0x%x).\n",
		       flash->name, info);
		return 1;
	}

	if (!en && (info & addr_mode_pos)) {
		prints("%s: Exit 4-byte Mode failed.(0x%x)\n",
		       flash->name, info);
		return 1;
	}

	return 0;
}

static int nor_set_quad_enable(struct spi_flash * flash, int en)
{
	uint8_t sts[2], status, qe_bit_pos;
	uint16_t try = 0;

set_quad:
	if (nor_read_status_register(flash, flash->sts_qe_bit, &status))
		return 1;

	// Winbond Q-bit and Spansion
	// can not be disabled
	if ((((flash->code & 0xFF) == 0xEF) ||
	     (flash->code == 0x174001)) &&  
	    !en)
		return 0;

	qe_bit_pos = 1 << (flash->sts_qe_bit % 8);

	if (en && (status & qe_bit_pos))
		prints("QE bit already enabled\n");

	if (en)
		status |= qe_bit_pos;
	else
		status &= ~qe_bit_pos;

	if (((flash->code & 0xff) == 0x01) ||
	//if ((flash->code == 0x182001) ||
	    (flash->code == 0x1820C2)){

		if (nor_read_status_register(flash, 0, &sts[0]))
			return 1;

		if (nor_read_status_register(flash, 8, &sts[1]))
			return 1;

		if (flash->sts_qe_bit < 8) {
			sts[0] = status;
		} else {
			sts[1] = status;
		}
	} else {
		sts[0] = status;
	}

	if (nor_write_status_register(flash, flash->sts_qe_bit, sts))
		return 1;

	if (flash->code == 0x1820C2)
		if (nor_write_status_register(flash, flash->sts_qe_bit,
					      sts))
			return 1;

	status = 0;
	if (nor_read_status_register(flash, flash->sts_qe_bit, &status))
		return 1;

	if (g_debug > 2)
		prints(" Read back status 0x%x \n", status);
	
	if (en && !(status & qe_bit_pos)) {

		if (try) {
			prints("%s: Enable Quad Mode failed.\n",
				flash->name);	
			return 1;
		} else {
			try++;
			goto set_quad;
		}
	}

	if (!en && (status & qe_bit_pos)) {
		prints("%s: Quad Mode can't be disabled.\n", flash->name);	
		return 0;
	}

	return 0;
}

int nor_qpi_mode(struct spi_flash *flash, int en)
{
	uint8_t cmd[1];
	uint8_t info, qe_bit_pos;

	if (flash->code == 0x1740C8) {
		prints("%s: Not support QPI mode.\n", flash->name);
		return 0;
	}

	en = !!en;
	if (en == flash->qpi_mode) {
		prints("%s: Already %s QPI mode.\n", flash->name,
		       en ? "in" : "exit");
		return 0;
	}

	if (nor_read_status_register(flash, flash->sts_qe_bit, &info))
		return 1;

	qe_bit_pos = 1 << (flash->sts_qe_bit % 8);

	if (en && !(info & qe_bit_pos)) {
		prints("%s: %s QPI mode, QE bit must be set(0x%x)\n", flash->name,
		       en ? "Enter" : "Exit", info);
		if (nor_set_quad_enable(flash, 1))
			return 1;
	}

	if (en)
		cmd[0] = NOR_ENQPI;
	else
		cmd[0] = NOR_EXQPI_MXIC;

	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	if (en) {
		flash->qpi_mode = 1;

		if (flash->code == 0x1970EF) {
			flash->rd_cmds = winbond_qpi_rd_cmds;
			flash->max_rd_type = sizeof(winbond_qpi_rd_cmds);
		} else if ((flash->code & 0xFFFF) == 0x20C2) {
			flash->rd_cmds = mx25l_qpi_rd_cmds;
			flash->max_rd_type = sizeof(mx25l_qpi_rd_cmds);
		} else {
			flash->rd_cmds = qpi_rd_cmds;
			flash->max_rd_type = sizeof(qpi_rd_cmds);
		}
		flash->wr_cmds = qpi_wr_cmds;
		flash->max_wr_type = sizeof(qpi_wr_cmds);
	} else {
		flash->qpi_mode = 0;
		if (((flash->code & 0xff) == 0x01) ||
		//if ((flash->code == 0x182001) ||
		    (flash->code == 0x1970EF)) {
			flash->rd_cmds = dtr_rd_cmds;
			flash->max_rd_type = sizeof(dtr_rd_cmds);
		} else {
			flash->rd_cmds = generic_rd_cmds;
			flash->max_rd_type = sizeof(generic_rd_cmds);
		}
		flash->wr_cmds = generic_wr_cmds;
		flash->max_wr_type = sizeof(generic_wr_cmds);
	}

	/* Some read commands not support 4-byte address in QPI mode.
	 * Make thing simple, just exit 4-byte mode if we are in QPI.
	 */
#if 0
	if ((flash->size > 0x1000000) && !flash->qpi_mode) {
		if(nor_4byte_mode(flash, 1))
			return 1;

		flash->addr_width = 4;
	} else {
		if(nor_4byte_mode(flash, 0))
			return 1;

		flash->addr_width = 3;

		/* Make sure don't access over 16 MB */
		while (flash->size >= 0x1000000)
			flash->size = flash->size - flash->erase_block_size;
	}
#endif
	return 0;
}

static int nor_xfer_cmd(struct spi_flash *flash, unsigned int len,
			const void *dout, void *din, unsigned int *flags)
{

	struct ftspi020_cmd spi_cmd;
	uint8_t *data_out = (uint8_t *) dout;
	uint8_t code;

	memset(&spi_cmd, 0, sizeof(struct ftspi020_cmd));

	code = *data_out;

	if (*flags & SPI_XFER_DAMR_READ) {
		spi_cmd.damr_read = 1;
	}

	if (*flags & SPI_XFER_CMD_STATE) {
		spi_cmd.start_ce = flash->ce;
		spi_cmd.ins_code = code;
		spi_cmd.spi_addr = ((*(data_out + 4) << 24)|
				    (*(data_out + 3) << 16) |
				    (*(data_out + 2) << 8) |
				    *(data_out + 1));

		switch (code) {
		case CMD_READ_ID:	// Read JEDEC ID
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = 3;
			break;
		case CMD_RESET:	// Reset the Flash or Exit QPI
		case NOR_WRITE_ENABLE_VOL:
		case NOR_WRITE_ENABLE:
		case NOR_WRITE_DISABLE:
		case NOR_EN4B:
		case NOR_EX4B:
		case NOR_ENQPI:
		case NOR_EXQPI_MXIC:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;

			if (flash->qpi_mode)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;

			if ((code == NOR_ENQPI) && ((flash->code & 0xFF) ==  0xc2)) {
				spi_cmd.ins_code = NOR_ENQPI_MXIC;
			} else if ((code == NOR_EXQPI_MXIC) && ((flash->code & 0xFF) !=  0xc2)) {
				spi_cmd.ins_code = CMD_RESET;
			}
			break;
		case NOR_ERASE_SECTOR:
		case NOR_ERASE_32KB_BLOCK:
		case NOR_ERASE_64KB_BLOCK:
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode) {
					if (code == NOR_ERASE_SECTOR)
						spi_cmd.ins_code = NOR_ERASE_SECTOR_4B;
					else if (code == NOR_ERASE_64KB_BLOCK)
						spi_cmd.ins_code = NOR_ERASE_64KB_BLOCK_4B;
					//W25Q256JV-DTR support 32K erase 4-bytes address
					//but use the same code with 3-byte address
					else if ((code == NOR_ERASE_32KB_BLOCK) && (flash->code !=  0x1970EF))
						spi_cmd.ins_code = NOR_ERASE_32KB_BLOCK_4B;
#if 0
					else {
						prints("%s: 32KB Block erase 4-byte mode "
						       "not support.\n", flash->name);
						return 1;
					}
#endif
				}
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_enable;
			spi_cmd.dtr_mode = dtr_disable;
#if (FTSPI020_ERASE_AUTOPOLL == 1)
			spi_cmd.read_status = 1;
			spi_cmd.data_cnt = 0;
#endif
			if (flash->qpi_mode)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case NOR_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case NOR_WRITE_STATUS1:
		case NOR_WRITE_STATUS2:
		case NOR_WRITE_STATUS3:
		case NOR_WRITE_BANK_ADDR:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;

			if (flash->qpi_mode)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case NOR_READ_STATUS1:
		case NOR_READ_STATUS2:
		case NOR_READ_STATUS3:
		case NOR_READ_BANK_ADDR:
		case NOR_READ_INFO_REG_EON:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.data_cnt = len;
			spi_cmd.read_status = 1;
			spi_cmd.dtr_mode = dtr_disable;

			if (flash->qpi_mode)
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			else
				spi_cmd.spi_mode = spi_operate_serial_mode;

			//Read status can't use DAMR
			spi_cmd.damr_read = 0;
			*flags &= ~SPI_XFER_DAMR_READ;
			break;
		case NOR_WRITE_PAGE:
		case NOR_WRITE_PAGE_DUAL:
		case NOR_WRITE_PAGE_QUAD:
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode) {
					if (code == NOR_WRITE_PAGE)
						spi_cmd.ins_code = NOR_WRITE_PAGE_4B;
					else if (code == NOR_WRITE_PAGE_QUAD)
						spi_cmd.ins_code = NOR_WRITE_PAGE_QUAD_4B;
					else {
						prints("%s: Write Page Quad IO 4-byte not support.\n", flash->name);
						return 1;
					}
				}
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (code == NOR_WRITE_PAGE) {
				if (flash->qpi_mode)
					spi_cmd.spi_mode = spi_operate_ins4bit_mode;
				else
					spi_cmd.spi_mode = spi_operate_serial_mode;
			} else if (code == NOR_WRITE_PAGE_DUAL) {
				spi_cmd.spi_mode = spi_operate_dual_mode;
			} else {
				//mxic uses quad io
				if ((flash->code == 0x1820C2) || (flash->code ==  0x3725c2)) {
					spi_cmd.spi_mode = spi_operate_quadio_mode;
					spi_cmd.ins_code = NOR_WRITE_PAGE_QUAD_MXIC;
				} else
					spi_cmd.spi_mode = spi_operate_quad_mode;
			}
			break;
		case NOR_READ_DATA:
			if (flash->addr_width == 4) {
				spi_cmd.ins_code = NOR_READ_DATA_4B;
				spi_cmd.addr_len = addr_4byte;
			} else
				spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case NOR_FAST_READ: /*QPI mode 4-byte address use 3-byte code */
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode)
					spi_cmd.ins_code = NOR_FAST_READ_4B;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode) {
				if (((flash->code & 0xFF) ==  0xc2) || //MXIC
					((flash->code & 0xFF) ==  0xC8)) { //GigaDevice
					spi_cmd.dum_2nd_cyc = 4;
				} else {
					spi_cmd.dum_2nd_cyc = 2;
				}
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else {
				spi_cmd.dum_2nd_cyc = 8;
				spi_cmd.spi_mode = spi_operate_serial_mode;
			}
			break;
		case NOR_READ_DUAL:
		case NOR_READ_QUAD:
			if (flash->addr_width == 4) {
				if (code == NOR_READ_DUAL)
					spi_cmd.ins_code = NOR_READ_DUAL_4B;
				else
					spi_cmd.ins_code = NOR_READ_QUAD_4B;

				spi_cmd.addr_len = addr_4byte;
			} else
				spi_cmd.addr_len = addr_3byte;

			//mxic and spansion: see Configuration Register bit[7:6] Dummy Cycle bits
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_dual_mode;

			if (code == NOR_READ_DUAL) {
				spi_cmd.spi_mode = spi_operate_dual_mode;
			} else {
				spi_cmd.spi_mode = spi_operate_quad_mode;
			}
			break;
		case NOR_READ_DUAL_IO:
			if (flash->addr_width == 4) {
				spi_cmd.ins_code = NOR_READ_DUAL_IO_4B;
				spi_cmd.addr_len = addr_4byte;
			} else
				spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 0;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case NOR_READ_QUAD_IO: /* QPI mode 4-byte address use 3-byte code */
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode)
					spi_cmd.ins_code = NOR_READ_QUAD_IO_4B;
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.conti_read_mode_en = 1;
			/* mode code A5h, 5Ah, F0h or 0Fh to enable
			 * continous read mode 
			 * mode code AAh, 55h, FFh or 00h to disable
			 * continous read mode
			 */
			spi_cmd.conti_read_mode_code = 0;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			if (flash->qpi_mode) {
				if ((flash->code & 0xFF) ==  0xc2) { //MXIC
					spi_cmd.dum_2nd_cyc = 4;
				} else if ((flash->code & 0xFF) ==  0xC8) { //GigaDevice
					spi_cmd.dum_2nd_cyc = 2;
				} else {
					spi_cmd.dum_2nd_cyc = 0;
				}
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else {
				if ((flash->code & 0xff) == 0x01) {
					spi_cmd.dum_2nd_cyc = 4;
				} else {
					//mxic: see Configuration Register bit[7:6] Dummy Cycle bits
					//mxic does not have mode bits, we use them as Dummy Cycles
					spi_cmd.dum_2nd_cyc = 4;
				}
				spi_cmd.spi_mode = spi_operate_quadio_mode;
			}
			break;
		case NOR_DDR_FAST_READ: /* QPI mode 4-byte address use 3-byte code */
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode) {
					spi_cmd.ins_code = NOR_DDR_FAST_READ_4B;
				}
			} else
				spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			if (flash->qpi_mode) {
				spi_cmd.dum_2nd_cyc = 8;
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else {
				if ((flash->code & 0xff) == 0x01) {
				//if (flash->code == 0x182001) {
					spi_cmd.dum_2nd_cyc = 2;
					spi_cmd.conti_read_mode_en = 1;
					spi_cmd.conti_read_mode_code = 0xFF;
				} else {
					spi_cmd.dum_2nd_cyc = 6;
				}
				spi_cmd.spi_mode = spi_operate_serial_mode;
			}
			break;
		case NOR_DDR_DUAL_IO_READ:
			if (flash->addr_width == 4) {
				spi_cmd.ins_code = NOR_DDR_DUAL_IO_READ_4B;
				spi_cmd.addr_len = addr_4byte;
			} else
				spi_cmd.addr_len = addr_3byte;

			if (flash->code == 0x150201) {
				spi_cmd.dum_2nd_cyc = 5;
			} else if (flash->code == 0x1970EF) {
				spi_cmd.dum_2nd_cyc = 4;
			} else if ((flash->code == 0x182001) ||
				   (flash->code == 0x190201)) {
				spi_cmd.dum_2nd_cyc = 4;
			}

			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0xFF;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case NOR_DDR_QUAD_IO_READ:/* QPI mode 4-byte address use 3-byte code */
			if (flash->addr_width == 4) {
				spi_cmd.addr_len = addr_4byte;

				if (!flash->qpi_mode) {
					spi_cmd.ins_code = NOR_DDR_QUAD_IO_READ_4B;
				}
			} else {
				spi_cmd.addr_len = addr_3byte;
			}

			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;

			if (flash->qpi_mode) {
				spi_cmd.dum_2nd_cyc = 8;
				spi_cmd.spi_mode = spi_operate_ins4bit_mode;
			} else {
				spi_cmd.conti_read_mode_en = 1;
				spi_cmd.conti_read_mode_code = 0xFF;

				if ((flash->code & 0xff) == 0x01) {
				//if (flash->code == 0x182001) {
					spi_cmd.dum_2nd_cyc = 6;
				} else {
					spi_cmd.dum_2nd_cyc = 7;
				}
				spi_cmd.spi_mode = spi_operate_quadio_mode;
			}
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
		if (FTSPI020_wait_cmd_complete(flash->ctrl_id, nor_wait_ms))
			return 1;
	}

	return 0;
}

static int nor_read(struct spi_flash *flash, uint8_t type, uint32_t offset,
		    uint32_t len, void *buf)
{
	int ret;
	uint8_t rd_cmd[5];

	if (type > flash->max_rd_type) {
		prints("%s: Read type exceed max\n", flash->name);
		return 2;
	}

	prints("%s: %s: %s(%02xh) %lu bytes @ %lu\n", flash->name,
		(g_damr_read[flash->ctrl_id]) ? "DAMR" : (g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		nor_action_get_string(flash, READ, type), flash->rd_cmds[type], len, offset);

	rd_cmd[0] = flash->rd_cmds[type];

	/* DAMR not support DTR read */
	if ((g_damr_read[flash->ctrl_id]) &&
	    ((rd_cmd[0] == NOR_DDR_FAST_READ) ||
	      (rd_cmd[0] == NOR_DDR_DUAL_IO_READ) ||
	      (rd_cmd[0] ==  NOR_DDR_QUAD_IO_READ))) {

		type = 1;
		rd_cmd[0] = flash->rd_cmds[type];
	}
#if 0
	if ((rd_cmd[0] == NOR_READ_QUAD) || 
	    (rd_cmd[0] == NOR_READ_QUAD_IO) ||
	    (rd_cmd[0] ==  NOR_DDR_QUAD_IO_READ))
		ret = nor_set_quad_enable(flash, 1);
	else
		ret = nor_set_quad_enable(flash, 0);
	if (ret)
		return 1;

	if (nor_check_status_till_ready(flash, 200))
		return 1;
#endif
	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	rd_cmd[4] = ((offset & 0xFF000000) >> 24);
#if 0
	//Offset before 16Mb, but plus length exceeds 16Mb
	if (!rd_cmd[4] &&
	    (offset + len) > ((16 << 20) - 1)) {
		remain = len;
		len = (16 << 20) - offset;
		remain -= len;
	}
#endif
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);

#if 0
	if (remain) {
		offset += len;
		buf = (char *)buf + len;

		rd_cmd[1] = offset & 0xFF;
		rd_cmd[2] = ((offset & 0xFF00) >> 8);
		rd_cmd[3] = ((offset & 0xFF0000) >> 16);
		rd_cmd[4] = ((offset & 0xFF000000) >> 24);

		ret = spi_flash_cmd_read(flash, rd_cmd, buf, remain);
		prints("%s: %s: %s %d bytes @ %d\n", flash->name,
			(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
			nor_action_get_string(flash, READ, type),
			remain, offset);
	}
#endif
	return ret;
}

static int nor_write(struct spi_flash *flash, uint8_t type, uint32_t offset,
		     uint32_t len, void *buf)
{
	int start_page, offset_in_start_page, len_each_times;
	uint32_t i, original_offset = offset, original_len = len;
	uint8_t wr_en_cmd[1], wr_cmd[5], sts;
	uint8_t *buff = buf, *alignment_buf;

	prints("%s: %s: %s(%02xh) %lu bytes @ %lu\n", flash->name,
		(g_trans_mode[flash->ctrl_id] == PIO) ? "PIO":"DMA",
		nor_action_get_string(flash, WRITE, type),
		flash->wr_cmds[type],
		original_len, original_offset);

	if (type > flash->max_wr_type) {
		prints("%s: Write type exceed max\n", flash->name);
		return 1;
	}

	alignment_buf = (uint8_t *) FTSPI020_DMA_ADDR;
	start_page = offset / flash->page_size;
	offset_in_start_page = offset % flash->page_size;

	/*
	   This judgement, "if(len + offset_in_start_page <=
			       flash->page_size)"
	   is for the case of (offset + len) doesn't exceed the nearest
	   next page boundary. 
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
		len_each_times = ((((start_page + 1) *
				  flash->page_size) - 1) - offset) + 1;
	}

	wr_en_cmd[0] = NOR_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	if (nor_read_status_register(flash, 2, &sts))
		return 1;

	if (!(sts & 0x2)) {
		prints ("%s: %s: Write Enable bit not set\n",
			flash->name,
			nor_action_get_string(flash, WRITE, type));
		return 1;
	}


#if 0
	if (flash->wr_cmds[type] == NOR_WRITE_PAGE_QUAD)
		ret = nor_set_quad_enable(flash, 1);
	else
		ret = nor_set_quad_enable(flash, 0);
	if (ret)
		return ret;

	if (nor_check_status_till_ready(flash, 200)) {
		prints ("%s: %s: set quad failed\n",
			flash->name,
			nor_action_get_string(flash, WRITE, type));
		return 1;
	}
#endif
	wr_cmd[0] = flash->wr_cmds[type];
	wr_cmd[1] = offset & 0xFF;
	wr_cmd[2] = ((offset & 0xFF00) >> 8);
	wr_cmd[3] = ((offset & 0xFF0000) >> 16);
	wr_cmd[4] = ((offset & 0xFF000000) >> 24);

	//prints("offset:0x%x len:0x%x\n", offset, len_each_times);
	if (spi_flash_cmd_write(flash, wr_cmd, buff, len_each_times))
		return 1;

	buff = buff + len_each_times;

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

		if (nor_check_status_till_ready(flash, 200))
			return 1;

		// To avoid the "buff" isn't alignment.
		for (i = 0; i < len_each_times; i++) {
			alignment_buf[i] = buff[i];
		}

		wr_en_cmd[0] = NOR_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		wr_cmd[0] = flash->wr_cmds[type];
		wr_cmd[1] = offset & 0xFF;
		wr_cmd[2] = ((offset & 0xFF00) >> 8);
		wr_cmd[3] = ((offset & 0xFF0000) >> 16);
		wr_cmd[4] = ((offset & 0xFF000000) >> 24);

		if (spi_flash_cmd_write(flash, wr_cmd, alignment_buf,
					len_each_times))
			return 1;
		buff = buff + len_each_times;

	} while (1);

	if (nor_check_status_till_ready(flash, 200))
		return 1;

	return 0;
}

static int nor_erase(struct spi_flash *flash, uint8_t type,
		     uint32_t offset, uint32_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[5];
	uint8_t cmd_code, sts;
	uint32_t wait_t;

	if (type > flash->max_er_type) {
		prints("Erase type exceed max\n");
		return 1;
	}

	cmd_code = flash->er_cmds[type];
	if (cmd_code == NOR_ERASE_SECTOR) {
		erase_size = flash->erase_sector_size;
		wait_t = flash->sector_erase_time;
	} else if (cmd_code == NOR_ERASE_32KB_BLOCK) {
		erase_size = flash->erase_block_size >> 1;
		wait_t = flash->block_erase_time;
	} else if (cmd_code == NOR_ERASE_64KB_BLOCK) {
		erase_size = flash->erase_block_size;
		wait_t = flash->block_erase_time;
	} else {
		prints("%s: %s @ 0x%lx\n", flash->name,
			nor_action_get_string(flash, ERASE, type), offset);
		return 1;
	}

	addr = offset & ~(erase_size - 1);

	if (nor_check_status_till_ready(flash, 200))
		return 1;
#if 0
	//TBPARM bit at CR1 bit 2
	if ((flash->code & 0xff) == 0x01) {
	//if (flash->code == 0x182001) {
		if (nor_read_status_register(flash, 10, &cr))
			return 0;

		cr &= (1 << 2);
		sector4k = 32 * 4096;
	}
#endif
	for ( ; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = NOR_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		if (nor_read_status_register(flash, 1, &sts))
			break;

		if (!(sts & 0x2)) {
			prints("%s: WEL bit not set, can not write\n", flash->name);
			break;
		}
#if 0
		if ((flash->code & 0xff) == 0x01) {
		//if (flash->code == 0x182001) {
			//TBPARM is 1, 4KB sector at top
			//TBPARM is 0, 4KB sector at bottom
			if ((cr && (addr >= (flash->size - sector4k))) ||
			    (!cr && (addr < (flash->size - sector4k)))) {
				cmd_code = NOR_ERASE_SECTOR;
				erase_size = flash->erase_sector_size;
				wait_t = flash->sector_erase_time;
				type = 0;
			} else {
				cmd_code = NOR_ERASE_64KB_BLOCK;
				erase_size = flash->erase_block_size;
				wait_t = flash->block_erase_time;
				type = 1;
			}
			addr &= ~(erase_size - 1);
		}
#endif
		er_cmd[0] = cmd_code;
		er_cmd[1] = addr & 0xFF;
		er_cmd[2] = ((addr & 0xFF00) >> 8);
		er_cmd[3] = ((addr & 0xFF0000) >> 16);
		er_cmd[4] = ((addr & 0xFF000000) >> 24);

		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;
#if (FTSPI020_ERASE_AUTOPOLL != 1) //Set Cmd_w3[3:1]=0b011, FTSPI020 auto polling.
		if (nor_check_status_till_ready(flash, wait_t))
			return 1;
#endif
		prints("%s: %s(%02xh) @ 0x%x\n", flash->name,
			nor_action_get_string(flash, ERASE, type), cmd_code, addr);

	}

	return ret;
}

static int nor_read_status(struct spi_flash *flash)
{
	uint8_t sts;

	if (nor_read_status_register(flash, 0, &sts))
		return 1;
	prints("%s: Read Status 1 0x%x\n", flash->name, sts);

	if (nor_read_status_register(flash, 8, &sts))
		return 1;
	prints("%s: Read Status 2 0x%x\n", flash->name, sts);

	if (nor_read_status_register(flash, 16, &sts))
		return 1;
	prints("%s: Read Status 3 0x%x\n", flash->name, sts);

	return 0;
}

static int nor_erase_all(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	er_all_cmd[0] = NOR_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = NOR_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	if (nor_check_status_till_ready(flash, flash->chip_erase_time))
		return 1;

	return 0;
}

static int nor_check_status_till_ready(struct spi_flash *flash,
				       uint32_t wait_ms)
{
#if 0
	uint8_t rd_sts_cmd[1];

	FTSPI020_busy_location(flash->ctrl_id, flash->sts_bsy_bit);
	nor_wait_ms = wait_ms;

	rd_sts_cmd[0] = NOR_READ_STATUS1;

	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0))
		return 1;
#else
	uint8_t sts = 1;

	do {
		if (nor_read_status_register(flash, 0, &sts))
			return 1;

	} while (sts & (1 << flash->sts_bsy_bit));
#endif
	return 0;
}

int32_t spi_flash_probe_nor(struct spi_flash *norf, uint8_t *code)
{
	uint8_t i;
	uint32_t idcode;
	const struct nor_spi_flash_params *params;

	memcpy(&idcode, code, 3);
	idcode &= ~0xff000000;

	for (i = 0; i < ARRAY_SIZE(nor_spi_flash_table); i++) {
		params = &nor_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == ARRAY_SIZE(nor_spi_flash_table)) {
		prints("nor: Unsupported device id 0x%lx\n", idcode);
		return 1;
	}

	norf->name = params->name;
	norf->code = params->idcode1_2;
	prints("Find Flash Name: %s.\n", norf->name);

	norf->spi_xfer = nor_xfer_cmd;		
	norf->read = nor_read;
	norf->write = nor_write;
	norf->erase = nor_erase;
	norf->erase_all = nor_erase_all;
	norf->report_status = nor_read_status;
	norf->get_string = nor_action_get_string;

	norf->page_size = params->page_size;
	norf->nr_pages = params->nr_pages ;
	norf->size = params->page_size * params->nr_pages;
	norf->erase_sector_size = params->sector_size;
	norf->erase_block_size = params->block_size;

	norf->chip_erase_time = params->chip_erase_time;
	norf->block_erase_time = params->block_erase_time;
	norf->sector_erase_time = params->sector_erase_time;

	norf->sts_bsy_bit = params->sts_bsy_bit;
	norf->sts_qe_bit = params->sts_qe_bit;
	norf->sts_addr_bit = params->sts_addr_bit;

	norf->rd_cmds = params->rd_cmds;
	norf->max_rd_type = params->max_rd_cmds;
	norf->wr_cmds = params->wr_cmds;
	norf->max_wr_type = params->max_wr_cmds;
	norf->er_cmds = params->er_cmds;
	norf->max_er_type = params->max_er_cmds;

	norf->qpi_mode = 0;
	norf->addr_width = 3;

	if (norf->size > 0x1000000) {
		uint8_t sts;

		//Winbond ADS at bit 16, MXIC at bit 5 of Configuration register
#if 0
		if(nor_4byte_mode(norf, 0)) {
			prints("nor: Exit 4-byte mode failed\n");
		} else {
			prints("nor: Exit 4-byte mode success\n");
		}
#else
		if(nor_4byte_mode(norf, 1))
			prints("nor: Enter 4-byte mode failed\n");
		else
			norf->addr_width = 4;
#endif
		sts = 0;
		if (nor_read_status_register(norf, norf->sts_addr_bit, &sts))
			return 0;

		if ((sts & (1 << (norf->sts_addr_bit % 8))) == 0) {
			norf->size = norf->size >> 1;
			prints("nor: ADS bit not set, reduce size to 0x%lx\n", norf->size);
		}

		prints("nor: Address mode bits 0x%x\n", sts);
	}

	//if ((norf->code == 0x182001) ||
	if (((norf->code & 0xff) == 0x01) ||
	    (norf->code == 0x1820C2)){
		uint8_t sts[2];
		char bp;

		if (nor_read_status_register(norf, 0, &sts[0]))
			return 0;

		if (nor_read_status_register(norf, 15, &sts[1]))
			return 0;

		if ((norf->code & 0xff) == 0x01)
		//if (norf->code == 0x182001)
			bp = (0x7 << 2);
		else if (norf->code == 0x1820C2)
			bp = (0xf << 2);
		//BP0-2
		if (sts[0] & bp)
			prints("nor: bp bit(s) not all zero 0x%x\n", sts[0]);

		//Block Protect
		sts[0] &= ~(bp);

		//Dummy Cycle
		sts[1] &= ~(3 << 6);

		if (nor_write_status_register(norf, 2, sts))
			return 0;

		if (nor_read_status_register(norf, 0, &sts[0]))
			return 0;

		prints("nor: bp bit(s) after clear 0x%x\n", sts[0]);

		if (nor_read_status_register(norf, 15, &sts[1]))
			return 0;

		prints("nor: dummy cycle bits 0x%x\n", sts[1]);

	}

	if (nor_set_quad_enable(norf, 1))
		prints("nor: set quad enable failed\n");

	return 0;
}

#endif
