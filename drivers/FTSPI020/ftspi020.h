/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2019/07/15   BingYao
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_H
#define FTSPI020_H

#include "portme_ftspi020.h"

/* Transfer Type*/
typedef enum {
	PIO,
	DMA
} Transfer_type;

#define FTSPI020_FW_VERSION "ver:19.07.15"

#include "ftspi020_sst.h"
#include "ftspi020_nand.h"
#include "ftspi020_micron.h"
#include "ftspi020_nor.h"

#define WORD_ACCESS
//#define HALFWORD_ACCESS

#define FTSPI020_32BIT(base_offset)	*((volatile uint32_t *)((uintptr_t)base_offset))
#define FTSPI020_16BIT(base_offset)	*((volatile uint16_t *)((uintptr_t)base_offset))
#define FTSPI020_8BIT(base_offset)	*((volatile uint8_t  *)((uintptr_t)base_offset))

#define SPI_FLASH_ADDR		0x00

#define SPI_CMD_W1   		0x04

// bit[2:0]
#define no_addr_state		0
#define addr_1byte			1
#define addr_2byte			2
#define addr_3byte			3
#define addr_4byte			4
// bit[23:16]
#define no_dummy_cycle_2nd		0
// bit[25:24]
#define no_instr_state			0
#define instr_1byte			1
#define instr_2byte			2
// bit28
#define no_op_code_state		0
#define op_code_1byte			1

#define SPI_DATA_CNT		0x08

#define SPI_CMD_W3  		0x0C
// bit1
#define spi_read			0
#define spi_write			1
// bit2
#define read_status_disable		0
#define read_status_enable		1
// bit3 
#define read_status_by_hw		1
#define read_status_by_sw		0
// bit4
#define dtr_disable			0
#define dtr_enable			1
// bit[7:5]
#define spi_operate_serial_mode		0
#define spi_operate_dual_mode		1
#define spi_operate_quad_mode		2
#define spi_operate_dualio_mode		3
#define spi_operate_quadio_mode		4
#define spi_operate_ins2bit_mode	5
#define spi_operate_ins4bit_mode	6
// bit[9:8]
#define	start_ce0			0
#define	start_ce1			1
#define	start_ce2			2
#define	start_ce3			3
// bit[23:16] continuous read mode code

// bit[31:24] instruction code

#define CONTROL_REG			0x10
// bit[1:0] divider
#define divider_2			0
#define divider_4			1
#define divider_6			2
#define divider_8			3
// bit 4 mode
#define mode0				0
#define mode3				1
// bit 8 abort
// bit 9 wp_en
// bit[18:16] busy_loc
#define BUSY_BIT0			0
#define BUSY_BIT1			1
#define BUSY_BIT2			2
#define BUSY_BIT3			3
#define BUSY_BIT4			4
#define BUSY_BIT5			5
#define BUSY_BIT6			6
#define BUSY_BIT7			7

#define SW_RESET_BIT		BIT8
#define XIP_PORT_IDLE		BIT7

#define AC_TIMING			0x14
// bit[3:0] CS timing
// bit[7:4] Trace delay

#define STATUS_REG			0x18
// bit0 TX FIFO ready
// bit1 RX FIFO ready

#define INTR_CONTROL_REG		0x20
#define dma_handshake_enable		BIT0
#define cmd_complete_intr_enable	BIT1
#define dma_rx_thod_mask	(0x7 << 20)
#define dma_rx_thod_4bytes	(0 << 20)
#define dma_rx_thod_16bytes	(1 << 20)
#define dma_rx_thod_32bytes	(2 << 20)
#define dma_rx_thod_64bytes	(3 << 20)
#define dma_rx_thod_128bytes	(4 << 20)
#define dma_rx_thod_256bytes	(5 << 20)

#define INTR_STATUS_REG			0x24
#define cmd_complete			BIT0
#define reset_done				BIT1

#define XIP_CMD_OPT 		0x30

#define XIP_FUNC_OPT 		0x34
#define xip_bypass_cache	BIT3 //cache support set bit 3 to zero
#define xip_invalid_cache	BIT10

#define REVISION			0x0050
#define FEATURE				0x0054
#define		support_dtr_mode		(1 << 24)
#define SPI020_DATA_PORT		0x0100

// =====================================   Common commands   =====================================
#define CMD_READ_ID  	                0x9F
#define CMD_RESET			0xFF
// ===================================== Variable and Struct =====================================
enum {
	WRITE,
	READ,
	ERASE
};

struct nor_spi_flash_params {
	uint32_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint16_t sector_size;
	uint32_t block_size;
	//erase time in milliseconds
	uint32_t chip_erase_time;
	uint16_t block_erase_time;
	uint16_t sector_erase_time;

	//Status register
	uint16_t sts_bsy_bit;
	uint16_t sts_qe_bit;
	uint16_t sts_addr_bit;

	const char *name;

	uint8_t *rd_cmds;
	uint32_t max_rd_cmds;
	uint8_t *wr_cmds;
	uint32_t max_wr_cmds;
	uint8_t *er_cmds;
	uint32_t max_er_cmds;
};

struct spi_flash {
	uint32_t ctrl_id;
	uint32_t ce;
	uint32_t code;
	const char *name;
	uint16_t erase_sector_size;
	uint32_t erase_block_size;
	uint16_t page_size;
	uint32_t nr_pages;
	uint32_t size;
	int(*spi_xfer)(struct spi_flash * flash, unsigned int len,
		       const void *dout, void *din, unsigned int *flags);
	int(*read) (struct spi_flash * flash, uint8_t type, uint32_t offset,
		    uint32_t total_len, void *buf);
	int(*write) (struct spi_flash * flash, uint8_t type,
		     uint32_t offset, uint32_t total_len, void *buf);
	int(*erase) (struct spi_flash * flash, uint8_t type,
		     uint32_t offset, uint32_t total_len);
	int(*erase_all) (struct spi_flash * flash);
	int(*report_status) (struct spi_flash * flash);
	char * (*get_string) (struct spi_flash * flash,
			      uint32_t act, uint32_t type);
	int (*set_quad_enable)(struct spi_flash * flash, int en);

	uint8_t *rd_cmds;
	uint16_t max_rd_type;
	uint8_t *wr_cmds;
	uint16_t max_wr_type;
	uint8_t *er_cmds;
	uint16_t max_er_type;

	uint16_t sts_bsy_bit;
	uint16_t sts_qe_bit;
	uint16_t sts_addr_bit;
	//erase time in milliseconds
	uint32_t chip_erase_time;
	uint16_t block_erase_time;
	uint16_t sector_erase_time;

	/* SPI Nand flash */
	int(*read_spare) (struct spi_flash * flash, uint32_t row_addr,
			  uint32_t col_addr, uint32_t total_len, char *buf);
	int(*write_spare) (struct spi_flash * flash, uint32_t row_addr,
			   uint32_t col_addr, uint32_t total_len, void *buf);
	int(*copy_data) (struct spi_flash * flash, uint32_t src_row,
			 uint32_t dst_row, uint32_t dst_col, void * buf,
			 uint32_t total_len);
	int(*is_bad_block) (struct spi_flash * flash, uint32_t offset);
	int(*scan_bad_blocks) (struct spi_flash * flash);

	uint8_t isnand;
	uint8_t addr_width;
	uint8_t qpi_mode;

	/* bit position is block index, 8 x 64 = 512 bits(blocks) */
	/* bit position is block index, 8 x 128 = 1024 bits(blocks) */
	uint8_t bad_block_table[BBT_MAX_ENT];
};

extern int32_t spi_flash_probe_micron(struct spi_flash *flsh, uint8_t *code);
extern int32_t spi_flash_probe_nor(struct spi_flash *norf, uint8_t *code);
extern int32_t spi_flash_probe_sst(struct spi_flash *ssf, uint8_t *idcode);
extern int32_t spi_flash_probe_spinand(struct spi_flash *flsh, uint32_t ctrl_id,
									   uint32_t ce, uint8_t *idcode);

struct ftspi020_cmd {
	// offset 0x00
	uint32_t spi_addr;
	// offset 0x04
	uint8_t addr_len;
	uint8_t dum_2nd_cyc;
	uint8_t ins_len;
	uint8_t conti_read_mode_en;
	// offset 0x08
	uint32_t data_cnt;
	// offset 0x0C
	uint8_t spi_type; //1: NAND, 0: NOR
	uint8_t write_en;
	uint8_t read_status_en;
	uint8_t read_status;
	uint8_t dtr_mode;
	uint8_t spi_mode;
	uint8_t start_ce;
	uint8_t conti_read_mode_code;
	uint8_t ins_code;

	/* This command prepare for DAMR port read */
	int damr_read;
};

extern int nor_qpi_mode(struct spi_flash *flash, int en);
extern int micron_qpi_mode(struct spi_flash *flash, int en);

#endif
