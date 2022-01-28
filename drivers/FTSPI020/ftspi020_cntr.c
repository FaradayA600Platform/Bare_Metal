/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_cntr.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2019/07/15   BingYao 
 * -------------------------------------------------------------------------
 */
#include <malloc.h>
#include <stdlib.h>

#include "portme_ftspi020.h"
#include "ftspi020.h"
#include "ftspi020_cntr.h"

uintptr_t g_spi020_wr_buf_addr;
int g_spi020_wr_buf_length;
uintptr_t g_spi020_rd_buf_addr;
int g_spi020_rd_buf_length;

int g_cmd_intr_enable[FTSPI020_MAXIMUM_CTRL];
int g_debug = 0;
int g_abort_test[FTSPI020_MAXIMUM_CTRL] = { 0 };
int g_damr_read[FTSPI020_MAXIMUM_CTRL] =  { 0 };

#if FTSPI020_MAXIMUM_CTRL == 1
int g_damr_port_base[FTSPI020_MAXIMUM_CTRL] = {FTSPI020_XIP_PORT_0};
uintptr_t g_reg_base[FTSPI020_MAXIMUM_CTRL] = {FTSPI020_REG_BASE_0};
#else
int g_damr_port_base[FTSPI020_MAXIMUM_CTRL] = {FTSPI020_XIP_PORT_0, FTSPI020_XIP_PORT_1};
uintptr_t g_reg_base[FTSPI020_MAXIMUM_CTRL] = {FTSPI020_REG_BASE_0, FTSPI020_REG_BASE_1};
#endif

Transfer_type g_trans_mode[FTSPI020_MAXIMUM_CTRL] = {PIO, PIO};
uint8_t g_divider = 4;

volatile uint32_t ftspi020_cmd_complete;

unsigned long randGen(void);

// ************************* local func *************************

void FTSPI020_xip_cache(uint32_t ctrl_id, uint8_t enable)
{
	if (enable)
		FTSPI020_32BIT(g_reg_base[ctrl_id] + XIP_FUNC_OPT) &= ~xip_bypass_cache;
	else
		FTSPI020_32BIT(g_reg_base[ctrl_id] + XIP_FUNC_OPT) |= xip_bypass_cache;
}

void FTSPI020_dma_enable(uint32_t ctrl_id, uint8_t enable)
{
	FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) &= ~(dma_handshake_enable | dma_rx_thod_mask);

	if (enable)
		FTSPI020_8BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) |= dma_handshake_enable;

	FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) |= dma_rx_thod_16bytes;
}

void FTSPI020_cmd_complete_intr_enable(uint32_t ctrl_id, uint8_t enable)
{
	FTSPI020_8BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) &= ~cmd_complete_intr_enable;

	if (enable) {
		FTSPI020_8BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) |= cmd_complete_intr_enable;
		ftspi020_enable_interrupts();
	}
}

void FTSPI020_reset_hw(uint32_t ctrl_id)
{
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) |= SW_RESET_BIT;
}

void FTSPI020_operate_mode(uint32_t ctrl_id, uint8_t mode)
{
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) &= ~BIT4;
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) |= ((mode << 4) & BIT4);
}

void FTSPI020_busy_location(uint32_t ctrl_id, uint8_t location)
{
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) &= ~(BIT18 | BIT17 | BIT16);
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) |= ((location << 16) & (BIT18 | BIT17 | BIT16));
}

void FTSPI020_divider(uint32_t ctrl_id, uint8_t divider)
{
	uint8_t val;

	if (divider == 2)
		val = divider_2;
	else if (divider == 4)
		val = divider_4;
	else if (divider == 6)
		val = divider_6;
	else if (divider == 8)
		val = divider_8;
	else {
		prints("Not valid divider value %d\n", divider);
		return ;
	}

	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) &= ~(BIT1 | BIT0);
	FTSPI020_32BIT(g_reg_base[ctrl_id] + CONTROL_REG) |= ((val) & (BIT1 | BIT0));

}

int FTSPI020_rxfifo_full(uint32_t ctrl_id)
{
	int i = 0;
	uint64_t t0, t1;

	t0 = ftspi020_get_time();

	//while (i++ < 0x10000) {
	while (1) {
		t1 = ftspi020_get_time();
		if (((t1 - t0) / COUNTS_PER_MSECOND) > 3000)
				break;

		if (FTSPI020_32BIT(g_reg_base[ctrl_id] + STATUS_REG) & BIT1)
			return 0;
	}

	prints (" Rx FIFO no data !\n");
	//return i;
	return 1;
}

int FTSPI020_txfifo_empty(uint32_t ctrl_id)
{
	int i = 0;

	while (i++ < 0x10000) {
		if (FTSPI020_32BIT(g_reg_base[ctrl_id] + STATUS_REG) & BIT0)
			return 0;
	}

	prints (" Tx FIFO not empty !\n");
	return i;
}

int FTSPI020_txfifo_depth(uint32_t ctrl_id)
{
	int reg;
	// The unit of returning value is byte.
	//return ((FTSPI020_8BIT(g_reg_base[ctrl_id] + FEATURE) & 0xFF) << 2);

	//Get Tx_Transfer Size
	reg = (FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) >> 8) & 0x3;
	switch (reg) {
	case 0:
			return 32;
	case 1:
			return 64;
	case 2:
			return 128;
	default:
	case 3:
			return 256;
	}
}

int FTSPI020_rxfifo_depth(uint32_t ctrl_id)
{
	int reg;

	// The unit of returning value is byte.
	//return ((FTSPI020_8BIT(g_reg_base[ctrl_id] + FEATURE + 1) & 0xFF) << 2);

	//Get Rx_Transfer Size
	reg = (FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_CONTROL_REG) >> 12) & 0x3;
	switch (reg) {
	case 0:
			return 32;
	case 1:
			return 64;
	case 2:
			return 128;
	default:
	case 3:
			return 256;
	}
}

// *********************** external func ***********************
int FTSPI020_support_dtr_mode(uint32_t ctrl_id)
{
	return (FTSPI020_32BIT(g_reg_base[ctrl_id] + FEATURE) & support_dtr_mode);
}

int FTSPI020_init(uint32_t ctrl_id)
{
#ifdef FTSPI020_USE_DMA
	static int dma_already_init = 0;
#endif
#ifdef FTSPI020_USE_INTERRUPT
	int irq;
#endif
	prints(" HW. Revision:0x%08lx\n",
			FTSPI020_32BIT(g_reg_base[ctrl_id] + REVISION));

	FTSPI020_reset_hw(ctrl_id);
	FTSPI020_operate_mode(ctrl_id, mode3);
	FTSPI020_divider(ctrl_id, g_divider);
	FTSPI020_xip_cache(ctrl_id, 0);
	FTSPI020_dma_enable(ctrl_id, 0);

	g_cmd_intr_enable[ctrl_id] = 0;
	ftspi020_disable_interrupts();
	FTSPI020_cmd_complete_intr_enable(ctrl_id, 0);

	/* Already allocate */
	if (!g_spi020_rd_buf_addr || !g_spi020_wr_buf_addr) {
		g_spi020_wr_buf_length =  g_spi020_rd_buf_length = 0x400000;
		g_spi020_wr_buf_addr = (uintptr_t) FTSPI020_RW_BUFFER_ADDR;
		g_spi020_rd_buf_addr = g_spi020_wr_buf_addr + g_spi020_wr_buf_length;

		prints(" Write buffer address 0x%x, read buffer address 0x%x\n",
				g_spi020_wr_buf_addr, g_spi020_rd_buf_addr);
	}
#ifdef FTSPI020_USE_INTERRUPT
	if (ctrl_id == 0) {
		irq = FTSPI020_IRQ_0;
#if SPI_FTSPI020_IRQ_COUNT == 2
	} else if (ctrl_id == 1) {
		irq = FTSPI020_IRQ_1;
#endif
	} else {
		prints(" Use interrupt wrong ctrl id %lu \n", ctrl_id);
		return 1;
	}
	ftspi020_setup_interrupt(irq, FTSPI020_Interrupt_Handler, (void *) ctrl_id);
#endif

#ifdef FTSPI020_USE_DMA
	if (dma_already_init == 0) {
		ftspi020_setup_dma(FTSPI020_DMA_RD_CHNL, FTSPI020_DMA_WR_CHNL);
		dma_already_init = 1;
		prints(" DMA LLD struct address 0x%x\n", FTSPI020_DMA_LLD_STRUCT_ADDR);
	}
#endif
	return 0;
}


int spi_flash_cmd(struct spi_flash *slave, uint8_t * cmd, void *response, size_t len)
{
	int ret;
	unsigned int flags;

	if (g_debug > 3)
		prints(" CMD code 0x%x length %d\n", cmd[0], len);

	flags = SPI_XFER_CMD_STATE | SPI_XFER_CHECK_CMD_COMPLETE;
	ret = slave->spi_xfer(slave, len, cmd, NULL, &flags);
	if (ret) {
		prints("%s: Failed to send command 0x%02x: %d\n", slave->name, cmd[0], ret);
		return ret;
	}

	if (len && response != NULL) {
		flags = SPI_XFER_DATA_STATE;
		ret = slave->spi_xfer(slave, len, NULL, response, &flags);
		if (ret) {
			prints("%s: Failed to read response (%d bytes): %d\n", slave->name, len, ret);
		}
	} else if ((len && response == NULL) || (!len && response != NULL)) {
		prints("%s: Failed to read response due to the mismatch of len and response (%d bytes): %d\n",
			    slave->name, len, ret);
	}

	return ret;
}

int spi_flash_cmd_write(struct spi_flash *slave, uint8_t * cmd, const void *data, int data_len)
{
	int ret;
	unsigned int flags;

	if (g_debug > 3)
		prints(" CMD write 0x%x 0x%x 0x%x 0x%x\n", cmd[0], cmd[1], cmd[2], cmd[3]);

	flags = SPI_XFER_CMD_STATE;
	ret = slave->spi_xfer(slave, data_len, cmd, NULL, &flags);
	if (ret) {
		prints("%s: Failed to send command 0x%02x\n", slave->name, cmd[0]);
		return ret;
	} else if (data_len != 0) {
		if (g_debug > 3)
			prints(" CMD data length %d bytes: 0x%lx\n", data_len, *((uint32_t *)data));

		flags = SPI_XFER_DATA_STATE | SPI_XFER_CHECK_CMD_COMPLETE;
		ret = slave->spi_xfer(slave, data_len, data, NULL, &flags);
		if (ret) {
			prints("%s: Failed to write data (%d bytes): 0x%x\n", slave->name, data_len, (uintptr_t)data);
		}
	}

	return ret;
}

int spi_flash_cmd_read(struct spi_flash *slave, uint8_t * cmd, void *data, int data_len)
{
	int ret;
	unsigned int flags;
	//uint64_t t0, t1;

	if (g_debug > 3)
		prints(" CMD read 0x%x 0x%x 0x%x 0x%x\n", cmd[0], cmd[1], cmd[2], cmd[3]);

	flags = SPI_XFER_CMD_STATE;
	if (g_damr_read[slave->ctrl_id])
		flags |= SPI_XFER_DAMR_READ;

	ret = slave->spi_xfer(slave, data_len, cmd, NULL, &flags);
	if (ret) {
		prints("%s: Failed to send command %02xh: %d\n", slave->name, cmd[0], ret);
		return ret;
	} else if (data_len != 0) {
		if (flags & SPI_XFER_DAMR_READ) {
			char *c_buf;
			short *s_buf;
			int *w_buf;
			uintptr_t offset;
			int l;

			offset = ((*(cmd + 4) << 24) | *(cmd + 3) << 16) |
				  (*(cmd + 2) << 8) | (*(cmd + 1));
			offset |= g_damr_port_base[slave->ctrl_id];

			if (offset & 0x1) {
				c_buf = (char *) ((uintptr_t)data);
				l = 1;
			} else if (offset & 0x2) {
				s_buf = (short *) ((uintptr_t)data);
				l = 2;
			} else {
				w_buf = (int *) ((uintptr_t)data);
				l = 4;
			}

			ret = data_len;
			//t0 = ftspi020_get_time();
			while (ret) {

				if (l == 1) {
					*c_buf++ = *(char *) offset++;
				} else if (l == 2) {
					*s_buf++ = *(short *) offset;
					offset += 2;
				} else {
					*w_buf++ = *(int *) offset;
					offset += 4;
				}

				ret -= l;

				if (ret < l)
					break;
			}

			if (ret && (ret < l)) {
				c_buf = (char *) (data + data_len - ret);

				while (ret) {
					*c_buf++ = *(char *) offset++;
					ret--;
				}
			}

			//t1 = ftspi020_get_time();
			//prints("Rd:%d ms.\n",(t1 - t0) / COUNTS_PER_MSECOND);
		} else {
			if (g_abort_test[slave->ctrl_id])
				data_len = data_len / 2;

			flags = (SPI_XFER_DATA_STATE | SPI_XFER_CHECK_CMD_COMPLETE);
			ret = slave->spi_xfer(slave, data_len, NULL, data, &flags);
		}

		if (g_debug > 3)
			prints(" CMD data length %d bytes: 0x%lx\n", data_len, *((uint32_t *)data));

		if (ret) {
			prints("%s: Failed to read data (%d bytes): 0x%x\n",
				slave->name, data_len, (uintptr_t)data);
		}
	}

	return ret;
}

void FTSPI020_flush_cmd_queue(uint32_t ctrl_id)
{
	FTSPI020_8BIT(g_reg_base[ctrl_id] + CONTROL_REG + 1) |= BIT0;
}

void FTSPI020_show_content(void *data_buf, int len)
{
	int i = 0;
	uint8_t *uint8_t_data_buf = (uint8_t *) (data_buf);

	if (len != 0) {

		do {
			if (i % 16 == 0 && i != 0) {
				prints("\n");
			}
			prints("%02x ", *(uint8_t_data_buf + i));
			i++;

		} while (i != len);
		prints("\n");
	}

}

uint8_t FTSPI020_compare(char * wr_buf, char * rd_buf, uint32_t len)
{
	int i;
	char tmp;

	/*if(FA520_CPUCheckDCacheEnable())
	   {
	   FA520_CPUInvalidateDCacheAll();
	   FA520_CPUCleanDCacheAll();
	   } */

	for (i = 0; i < len; i++) {

		if (wr_buf)
			tmp = *(wr_buf + i);
		else
			tmp = 0xFF;

		if (*(rd_buf + i) != tmp) {
			prints("Compare failed at %d(wr:0x%x but " \
			       "rd:0x%x)\n", i, tmp,
			       *(rd_buf + i));
			if (tmp != 0xFF) {
				prints("wr_buf %x %x %x %x %x %x, rd_buf %x %x %x %x %x %x\n",
						wr_buf[i], wr_buf[i+1], wr_buf[i+2], wr_buf[i+3], wr_buf[i+4], wr_buf[i+5],
						rd_buf[i], rd_buf[i+1], rd_buf[i+2], rd_buf[i+3], rd_buf[i+4], rd_buf[i+5]);
			}
			return 1;
		}

	}
	return 0;
}

uint8_t FTSPI020_issue_cmd(uint32_t ctrl_id, struct ftspi020_cmd * command)
{
	uintptr_t base;
	uint32_t cmd_w1, cmd_w3;
	char read_mode_code;

	base = g_reg_base[ctrl_id];

	FTSPI020_32BIT(base + CONTROL_REG) &= ~(0x1f << 9);

	if (!command->damr_read) {
		if (g_cmd_intr_enable[ctrl_id])
			ftspi020_cmd_complete = 0;

		if (command->addr_len == 0) {
			FTSPI020_32BIT(base + SPI_FLASH_ADDR) = 0;
		} else {
			FTSPI020_32BIT(base + SPI_FLASH_ADDR) =
						command->spi_addr;
		}
		cmd_w1 = ((command->conti_read_mode_en & 0x1) << 28) |
			((command->ins_len & 0x3) << 24) |
			((command->dum_2nd_cyc & 0xFF) << 16) |
			((command->addr_len & 0x7) << 0);
		FTSPI020_32BIT(base + SPI_CMD_W1) = cmd_w1;

		FTSPI020_32BIT(base + SPI_DATA_CNT) = command->data_cnt;

		cmd_w3 = ((command->ins_code & 0xFF) << 24) |
			((command->conti_read_mode_code & 0xFF) << 16) |
			((command->start_ce & 0x3) << 8) |
			((command->spi_mode & 0x7) << 5) |
			((command->dtr_mode & 0x1) << 4) |
			((command->write_en & 0x1) << 1) |
			((command->spi_type & 0x1) << 0);

		if (command->read_status && (command->data_cnt == 0))
			cmd_w3 |= (read_status_by_hw << 2);

		// HW version 1.0 need to set bit 0 for command
		// complete interrupt status
		//cmd_feature2 |= 0x1;

		FTSPI020_32BIT(base + SPI_CMD_W3) = cmd_w3;

		if (g_debug > 1) {
			//prints("*************** Cmd Index:0x%02x ****************\n",
					//command->ins_code);
			prints("cmd0:0x%08lx, cmd1: 0x%08lx, cmd2: 0x%08lx, cmd3: 0x%08lx\n",
					FTSPI020_32BIT(base + SPI_FLASH_ADDR),
					FTSPI020_32BIT(base + SPI_CMD_W1),
					FTSPI020_32BIT(base + SPI_DATA_CNT), cmd_w3);
		}
	} else {
		int addr_len;

		if (command->addr_len == 3)
			addr_len = 0;
		else if (command->addr_len == 4)
			addr_len = 1;
		else {
			prints("address length error!!\n");
			return 1;
		}

		cmd_w1 = command->dum_2nd_cyc & 0xFF;
		cmd_w1 |= ((command->spi_mode & 0x7) << 8);
		cmd_w1 |= (addr_len << 11);
		cmd_w1 |= ((command->ins_code & 0xFF) << 12);
		// DAMR read does not support continuous read mode,
		// flip the bit
		if (command->conti_read_mode_code)
			read_mode_code = (~command->conti_read_mode_code) &
					  0xff;
		else
			read_mode_code = 0;
		cmd_w1 |= (read_mode_code << 20);
		cmd_w1 |= ((command->conti_read_mode_en & 0x1) << 28);
		FTSPI020_32BIT(base + XIP_CMD_OPT) = cmd_w1;
#if 0
	prints("XIP FOP (0x%x): 0x%08x\n", XIP_CMD_OPT, cmd_w1);
#endif
	}

	return 0;
}

int FTSPI020_wait_cmd_complete(uint32_t ctrl_id, uint32_t wait_ms)
{
	uintptr_t base;
	uint64_t t0, t1;

	base = g_reg_base[ctrl_id];

	t0 = ftspi020_get_time();

	do {
		if (g_cmd_intr_enable[ctrl_id]) {

			if (ftspi020_cmd_complete) {
				t1 = ftspi020_get_time();
				break;
			}

		} else {
			int intr_status;

			intr_status = FTSPI020_32BIT(base + INTR_STATUS_REG);

			if (intr_status & cmd_complete) {
				t1 = ftspi020_get_time();
				FTSPI020_32BIT(base + INTR_STATUS_REG) |= cmd_complete;
				if (g_debug > 2)
					prints(" Wait complete OK \n");
				break;
			}
		}

		t1 = ftspi020_get_time();
		if (((t1 - t0) / COUNTS_PER_MSECOND) > (wait_ms << 1)) {
			if (!g_abort_test[ctrl_id]) {
				prints("Timeout when wait %lu ms for cmd " \
				       "complete\n", wait_ms);
				return 1;
			} else {
				FTSPI020_flush_cmd_queue(ctrl_id);
				prints("Doing abort test ... no cmd cmpl" \
				       "(%lu ms)\n", wait_ms);
				break;
			}
		}
	} while (1);

	if (((t1 - t0) / COUNTS_PER_MSECOND) > wait_ms)
		prints("Cmd compl: expect wait %lu, real %lld ms\n",
			wait_ms, (t1 - t0) / COUNTS_PER_MSECOND);

	return 0;
}

static void ftspi020_memcpy(uint8_t * dst, uint8_t * src, uint32_t len)
{
	int i;

	for (i = 0; i < len ; i++) {
		dst[i] = src[i];
	}
}

int FTSPI020_data_access(uint32_t ctrl_id, uint8_t * dout, uint8_t * din, uint32_t len)
{
	int ret = 0;
	int access_byte, total_byte;
	uint8_t *alloc_buf, *buf;

	/* buffer not 4 byte alignment */
	if (dout) {
		if ((uintptr_t)dout & 0x3) {
			alloc_buf = (uint8_t *) FTSPI020_DMA_ADDR; //memalign((size_t)4, (size_t)((len + 4) & ~0x3));
#if 0
			if (!alloc_buf) {
				prints (" alloc_buffer error");
				ret = 1;
				goto done;
			}
#endif
			buf = alloc_buf;
			ftspi020_memcpy(buf, dout, len);
		} else
			buf = dout;
	} else if (din) {
		if ((uintptr_t)din & 0x3) {
			alloc_buf = (uint8_t *) FTSPI020_DMA_ADDR;;//memalign((size_t)4, (size_t)((len + 4) & ~0x3));
#if 0
			if (!alloc_buf) {
				prints (" alloc_buffer error");
				ret = 1;
				goto done;
			}
#endif
			buf = alloc_buf;
		} else
			buf = din;
	} else {
		prints (" No buffer pointer for data in or out !\n");
		return 1;
	}

#ifdef FTSPI020_USE_DMA
	if (g_trans_mode[ctrl_id] == DMA) {

		if (dout != NULL) {
			ftspi020_dcache_flushrange((uintptr_t)buf, len);

			ret = ftspi020_start_dma(ctrl_id, FTSPI020_DMA_RD_CHNL,
					(uint32_t) ((uintptr_t)buf),
					(int) (g_reg_base[ctrl_id] + SPI020_DATA_PORT),
					len, 0x0, 0x0, 4, 0, 2,
					FTSPI020_DMA_PRIORITY);
		} else if (din != NULL) {
			/* Not multiple of 4 bytes */
			if (len & 0x3)
				total_byte = (len + 4) & ~0x3;
			else
				total_byte = len;

			ret = ftspi020_start_dma(ctrl_id, FTSPI020_DMA_RD_CHNL,
					(int) (g_reg_base[ctrl_id] + SPI020_DATA_PORT),
					(uint32_t) ((uintptr_t)buf), total_byte, 0x2, 0x2, 1, 2, 0,
					FTSPI020_DMA_PRIORITY);
			//prints("Rd:%d ms.\n", ftspi020_get_timer(t0));
		}

		if (ret)
			prints (" DMA transfer error !\n");
		else
			total_byte = 0;
	} else {
#endif
#if defined(WORD_ACCESS)
		total_byte = len;

		if (dout != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_txfifo_empty(ctrl_id);
				if (ret)
					goto done;

				access_byte = min_t(total_byte,
						    FTSPI020_txfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					FTSPI020_32BIT(g_reg_base[ctrl_id] +
					   SPI020_DATA_PORT) = *((int *) buf);
					buf += 4;
					access_byte -= 4;
				}
			}

		} else if (din != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_rxfifo_full(ctrl_id);
				if (ret)
					goto done;

				access_byte = min_t(total_byte,
						    FTSPI020_rxfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte < 4) {
						while (access_byte) {
							*buf++ = FTSPI020_8BIT(g_reg_base[ctrl_id] + SPI020_DATA_PORT);
							access_byte--;
						}
					} else {
						*((int *) buf) = FTSPI020_32BIT(g_reg_base[ctrl_id] + SPI020_DATA_PORT);
						buf += 4;
						access_byte -= 4;
					}
				}
			}
			//prints("Rd:%d ms.\n", ftspi020_get_timer(t0));

		}
#elif defined(HALFWORD_ACCESS)
		if (dout != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_txfifo_empty(ctrl_id);
				if (ret)
					goto done;
				access_byte = min_t(total_byte,
						    FTSPI020_txfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					FTSPI020_16BIT(g_reg_base[ctrl_id] +
						     SPI020_DATA_PORT) =
							*((uint16_t *) buf);
					buf += 2;
					access_byte -= 2;
				}
			}
		} else if (din != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_rxfifo_full(ctrl_id);
				if (ret)
					goto done;
				access_byte = min_t(total_byte,
						    FTSPI020_rxfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte < 3) {
						while (access_byte) {
							*buf++ = FTSPI020_8BIT(g_reg_base[ctrl_id] + SPI020_DATA_PORT);
							access_byte--;
						}
					} else {
						*((uint16_t *) buf) = FTSPI020_16BIT(g_reg_base[ctrl_id] + SPI020_DATA_PORT);
						buf += 2;
						access_byte -= 2;
					}
				}
			}
		}
#else
		if (dout != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_txfifo_empty(ctrl_id);
				if (ret)
					goto done;
				access_byte = min_t(total_byte,
						    FTSPI020_txfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					FTSPI020_8BIT(g_reg_base[ctrl_id] +
						      SPI020_DATA_PORT) =
						      *((uint8_t *) dout);
					dout += 1;
					access_byte -= 1;
				}
			}
		} else if (din != NULL) {
			while (total_byte > 0) {
				ret = FTSPI020_rxfifo_full(ctrl_id);
				if (ret)
					goto done;
				access_byte = min_t(total_byte,
					            FTSPI020_rxfifo_depth(ctrl_id));
				total_byte -= access_byte;
				while (access_byte > 0) {
					*((uint8_t *) din) =
					    FTSPI020_8BIT(g_reg_base[ctrl_id] +
							  SPI020_DATA_PORT);
					din += 1;
					access_byte -= 1;
				}
			}
		}
#endif
#ifdef FTSPI020_USE_DMA
	}
#endif

done:
	if (ret) {
		FTSPI020_flush_cmd_queue(ctrl_id);
	} else {
		if ((dout && ((uintptr_t)dout & 0x3)) ||
		    (din && ((uintptr_t)din & 0x3))) {
			if (din)
				ftspi020_memcpy(din, alloc_buf, (len - total_byte));

			//free(alloc_buf);
		}
	}
	return ret;
}

void FTSPI020_Interrupt_Handler(void *data)
{
	uint32_t ctrl_id = (uint32_t) data;
	int intr_status = FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_STATUS_REG);

	if (intr_status & cmd_complete) {
		if (g_debug)
			prints(" ISR: Command Complete\n");

		ftspi020_cmd_complete = 1;
		FTSPI020_32BIT(g_reg_base[ctrl_id] + INTR_STATUS_REG) |= cmd_complete;
	} else {
		prints(" ISR: No status\n");
	}
}

int32_t FTSPI020_probe(struct spi_flash *flash, uint32_t ctrl_id, uint32_t ce)
{
	struct ftspi020_cmd spi_cmd = {0};
	int32_t ret = 1;
	uint8_t idcode[5];

	spi_cmd.start_ce = ce;
	spi_cmd.ins_code = CMD_READ_ID;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_read;
	spi_cmd.dtr_mode = dtr_disable;
	spi_cmd.spi_mode = spi_operate_serial_mode;
	spi_cmd.data_cnt = 5;

	FTSPI020_issue_cmd(ctrl_id, &spi_cmd);
	if (FTSPI020_data_access(ctrl_id, NULL, idcode, sizeof(idcode))){
		FTSPI020_reset_hw(ctrl_id);
		goto err;
	}

	if (FTSPI020_wait_cmd_complete(ctrl_id, 10)){
		FTSPI020_reset_hw(ctrl_id);
		goto err;
	}

	prints("SF:%lu: Got idcode %02x %02x %02x %02x %02x\n", ce, idcode[0],
		idcode[1], idcode[2], idcode[3], idcode[4]);

	memset (flash, 0, sizeof(struct spi_flash));

	flash->ctrl_id = ctrl_id;
	flash->ce = ce;

	switch (idcode[0]) {
	case 0xBF:
		ret = spi_flash_probe_sst(flash, idcode);
		break;
	case 0x20: //Micron
		ret = spi_flash_probe_micron(flash, idcode);
		break;
	case 0xEF: //Winbond
	case 0x01: //Spansion
	case 0xC2: //mxic
	case 0x1C: //EON
		ret = spi_flash_probe_nor(flash, idcode);
		break;
	case 0xC8: //GigaDevice
		//The second byte value 0xB1 is 3.3v, 0xA1 is 1.8v
		if (((idcode[1] == 0xB1) || (idcode[1] == 0xA1)) &&
			 (idcode[2] == 0x48))
			ret = spi_flash_probe_spinand(flash, ctrl_id, ce, idcode);
		else
			ret = spi_flash_probe_nor(flash, idcode);
		break;
	default:
		prints("SF: Unsupported manufacturer 0x%02X @ ctrl %lu, CE %lu\n",
			idcode[0], ctrl_id,  ce);
		break;
	}

err:
	/* Let's try SPI NAND Flash */
	if (ret) {
		prints("SF: Trying to probe SPI NAND flash\n");
		ret = spi_flash_probe_spinand(flash, ctrl_id, ce, idcode);
	}

	return ret;
}
