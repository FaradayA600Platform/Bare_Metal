/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_cntr.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/07   Mike          
 * 2010/08/13   BingJiun	 Scan all CEs for available flash.         
 * -------------------------------------------------------------------------
 */
#ifndef FTSPI020_CNTR_H
#define FTSPI020_CNTR_H

#define SPI_XFER_BEGIN			0x00000000
#define SPI_XFER_END			0x00000001
#define SPI_XFER_CMD_STATE		0x00000002
#define SPI_XFER_DATA_STATE		0x00000004
#define SPI_XFER_CHECK_CMD_COMPLETE	0x00000008
#define SPI_XFER_DAMR_READ	        0x00000010

// ********************* external variable *********************
extern uintptr_t g_spi020_wr_buf_addr;
extern int g_spi020_wr_buf_length;
extern uintptr_t g_spi020_rd_buf_addr;
extern int g_spi020_rd_buf_length;
// *********************** external func ***********************
extern int FTSPI020_init(uint32_t ctrl_id);
extern int FTSPI020_support_dtr_mode(uint32_t ctrl_id);
extern int32_t FTSPI020_probe(struct spi_flash *flash, uint32_t ctrl_id, uint32_t ce);
extern int spi_flash_cmd(struct spi_flash *slave, uint8_t * uint8_t_cmd, void *response, size_t len);
extern int spi_flash_cmd_write(struct spi_flash *slave, uint8_t * uint8_t_cmd, const void *data, int data_len);
extern int spi_flash_cmd_read(struct spi_flash *slave, uint8_t * uint8_t_cmd, void *data, int data_len);

extern void FTSPI020_flush_cmd_queue(uint32_t ctrl_id);
extern void FTSPI020_show_status(void);
extern void FTSPI020_read_status(uint8_t * status);
extern void FTSPI020_show_content(void *data_buf, int len);
extern uint8_t FTSPI020_compare(char * data1_buf, char * data2_buf, uint32_t len);
extern uint8_t FTSPI020_issue_cmd(uint32_t ctrl_id, struct ftspi020_cmd *command);
extern int FTSPI020_wait_cmd_complete(uint32_t ctrl_id, uint32_t wait_ms);
extern int FTSPI020_data_access(uint32_t ctrl_id, uint8_t * dout, uint8_t * din, uint32_t len);
extern void FTSPI020_Interrupt_Handler(void * data);
// ************************* local func *************************
void FTSPI020_dma_enable(uint32_t ctrl_id, uint8_t enable);
void FTSPI020_cmd_complete_intr_enable(uint32_t ctrl_id, uint8_t enable);
void FTSPI020_reset_hw(uint32_t ctrl_id);
void FTSPI020_operate_mode(uint32_t ctrl_id, uint8_t mode);
void FTSPI020_busy_location(uint32_t ctrl_id, uint8_t location);
void FTSPI020_divider(uint32_t ctrl_id, uint8_t divider);
int FTSPI020_rxfifo_full(uint32_t ctrl_id);
int FTSPI020_txfifo_empty(uint32_t ctrl_id);
int FTSPI020_txfifo_depth(uint32_t ctrl_id);
int FTSPI020_rxfifo_depth(uint32_t ctrl_id);

//Initial value set to zero
extern int g_cmd_intr_enable[FTSPI020_MAXIMUM_CTRL];
extern int g_debug;
extern int g_abort_test[FTSPI020_MAXIMUM_CTRL];
extern int g_damr_read[FTSPI020_MAXIMUM_CTRL];
extern int g_damr_port_base[FTSPI020_MAXIMUM_CTRL];
extern uintptr_t g_reg_base[FTSPI020_MAXIMUM_CTRL];

//Has initial value
extern Transfer_type g_trans_mode[FTSPI020_MAXIMUM_CTRL];
extern uint8_t g_divider;
extern char g_check_status;
extern volatile uint32_t ftspi020_cmd_complete;

#endif
