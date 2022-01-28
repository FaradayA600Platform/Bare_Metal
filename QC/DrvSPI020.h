#ifndef __SPI020_H__
#define __SPI020_H__
//#include "common.h"
//#include "ram.h"
//#include "types.h"

#define IPMODULE SPI
#define IPNAME FTSPI020

#define FTSPI020_PA_BASE IP_PA_BASE(0)


/*******************************************
 * for constant definition
 ********************************************/
#define FLASH_64K			0x10000
#define FLASH_PAGE_SIZE		256
#define FLASH_NORMAL		0x00
#define FLASH_2X_READ		0x01
#define FLASH_DUAL_READ		0x02
#define FLASH_DMA_READ		0x04
#define FLASH_DMA_WRITE		0x08
#define FLASH_2XIO_READ		0x10
#define FLASH_BYTE_MODE		0x20
#define FLASH_PIO_RW		0x40
#define FLASH_FIRST_READ	0x80 //Winbond W25P16

#define FLASH_CTL_SPI010	0x01
#define FLASH_CTL_SPI020	0x02

#define FLASH_NOT_SUPP_DUAL	0x00
#define FLASH_SUPP_3B_DUAL	FLASH_DUAL_READ	/* for Winbond */
#define FLASH_SUPP_BB_DUAL	FLASH_2XIO_READ	/* for MXIC */

typedef enum
{
	SPI_CLK_MODE0=0,
	SPI_CLK_MODE3=0x10,	
}SPI_CLK_MODE;

/*******************************************
 * for operation define definition
 ********************************************/
#define	SPI_CLK_DIVIDER_2	0x00	
#define	SPI_CLK_DIVIDER_4	0x01
#define	SPI_CLK_DIVIDER_6	0x02
#define	SPI_CLK_DIVIDER_8	0x03


#define SPI020_CE_0			0x0000
#define SPI020_CE_1			0x0100
#define SPI020_CE_2			0x0200
#define SPI020_CE_3			0x0300
#define SPI020_CE_VALUE		SPI020_CE_0
//#define SPI020_INTR_CFG		0x00000001
#define SPI020_INTR_CFG		0x00000000 // For ip V1.1~ version, this bit becomes reserved bit

/*******************************************
 * for register address definition
 ********************************************/
#if 0
#define SPI020REG_CMD0		(FTSPI020_PA_BASE+0x00)
#define SPI020REG_CMD1		(FTSPI020_PA_BASE+0x04)
#define SPI020REG_CMD2		(FTSPI020_PA_BASE+0x08)
#define SPI020REG_CMD3		(FTSPI020_PA_BASE+0x0C)
#define SPI020REG_CONTROL	(FTSPI020_PA_BASE+0x10)
#define SPI020REG_ACTIMER	(FTSPI020_PA_BASE+0x14)
#define SPI020REG_STATUS	(FTSPI020_PA_BASE+0x18)
#define SPI020REG_INTERRUPT	(FTSPI020_PA_BASE+0x20)
#define SPI020REG_INTR_ST	(FTSPI020_PA_BASE+0x24)
#define SPI020REG_READ_ST	(FTSPI020_PA_BASE+0x28)
#define SPI020REG_VERSION	(FTSPI020_PA_BASE+0x50)
#define SPI020REG_FEATURE	(FTSPI020_PA_BASE+0x54)
#define SPI020REG_SCKINDLY	(FTSPI020_PA_BASE+0x58)
#define SPI020REG_DATAPORT	(FTSPI020_PA_BASE+0x100)
#else
#define SPI020REG_CMD0		(0x00)
#define SPI020REG_CMD1		(0x04)
#define SPI020REG_CMD2		(0x08)
#define SPI020REG_CMD3		(0x0C)
#define SPI020REG_CONTROL	(0x10)
#define SPI020REG_ACTIMER	(0x14)
#define SPI020REG_STATUS	(0x18)
#define SPI020REG_INTERRUPT	(0x20)
#define SPI020REG_INTR_ST	(0x24)
#define SPI020REG_READ_ST	(0x28)
#define SPI020REG_VERSION	(0x50)
#define SPI020REG_FEATURE	(0x54)
#define SPI020REG_SCKINDLY	(0x58)
#define SPI020REG_DATAPORT	(0x100)
#endif
/*******************************************
 * for register bits definition
 ********************************************/
/* for SPI020REG_CONTROL */
#define SPI020_ABORT		BIT8
#define SPI020_CLK_MODE		BIT4
#define SPI020_CLK_DIVIDER	(BIT0|BIT1)
/* for SPI020REG_STATUS */
#define SPI020_RXFIFO_READY	BIT1
#define SPI020_TXFIFO_READY	BIT0
/* for SPI020REG_INTERRUPT */
#define SPI020_DMA_EN		BIT0
/* for SPI020REG_INTR_ST */
#define SPI020_CMD_CMPLT	BIT0
/* for SPI020REG_FEATURE */
#define SPI020_RX_DEPTH		0xFF00
#define SPI020_TX_DEPTH		0x00FF


/* for DMA020REG_CONTROL */
#define DMA020_DST_HE_ENABLE    BIT13
#define DMA020_SRC_HE_ENABLE    BIT7
#ifdef CONFIG_PLATFORM_GM8220
#define SPI020_ACK_REQ  0
#else
#define SPI020_ACK_REQ  11
#define SPI020_ACK_REQ_0  9
#define SPI020_ACK_REQ_1  11
#endif
/*******************************************
 * for read chip id command (0x9F)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_9F_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_9F_CMD1	0x01000000
/* Set command word 2, set data count to 3 */
#define SPI020_9F_CMD2	0x3
/* Set command word 4, instrction code = 0x9F, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_9F_CMD3	(0x9F000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x05)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_05_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_05_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_05_CMD2	0x0
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_05_CMD3	(0x05000004|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x35)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_35_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_35_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_35_CMD2	0x0
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_35_CMD3	(0x35000004|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for read data command (0x03)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_03_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_03_CMD1	0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_03_CMD2	0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_03_CMD3	(0x03000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read data command (0x0B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0B_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_0B_CMD1	0x01080003  //bessel:change value from 0x01000003 to 0x01080003(Fast Read instruction need to add eight "dummy"clocks after 24-bit address) 
/* Set command word 2, set data count by input parameter */
#define SPI020_0B_CMD2	0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0B_CMD3	(0x0B000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for write enable command (0x06)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_06_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_06_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_06_CMD2	0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_06_CMD3	(0x06000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for write disable command (0x04)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_04_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_04_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_04_CMD2	0x0
/* Set command word 4, instrction code = 0x04, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_04_CMD3	(0x04000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for page write command (0x02)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_02_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_02_CMD1	0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_02_CMD2	0x0
/* Set command word 4, instrction code = 0x02, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_02_CMD3	(0x02000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for chip earse command (0xC7):Erase all
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_C7_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_C7_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_C7_CMD2	0x0
/* Set command word 4, instrction code = 0xC7, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_C7_CMD3	(0xC7000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for block earse command (0xD8):64KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_D8_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_D8_CMD1	0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_D8_CMD2	0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_D8_CMD3	(0xD8000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for block earse command (0x52):32KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_52_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
 dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_52_CMD1	0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_52_CMD2	0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
 start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
 status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_52_CMD3	(0x52000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for sector earse command (0x20):Sector Erase(4K for W25Q64CV)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_20_CMD0 0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_20_CMD1 0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_20_CMD2 0x0
/* Set command word 4, instrction code = 0x20, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_20_CMD3 (0x20000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for dual read data command for windond device (0x3B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_3B_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_3B_CMD1	0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_3B_CMD2	0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_3B_CMD3	(0x3B000020|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for 2xIO read data command for MXIC device (0xBB)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BB_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 4, dum_1st_cyc = 0, address length = 3 */
#define SPI020_BB_CMD1	0x01040003
/* Set command word 2, set data count by input parameter */
#define SPI020_BB_CMD2	0x0
/* Set command word 4, instrction code = 0xBB, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BB_CMD3	(0xBB000060|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for EWSR (enable-write-status-register) command (0x50)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_50_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_50_CMD1	0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_50_CMD2	0x0
/* Set command word 4, instrction code = 0x50, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_50_CMD3	(0x50000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for WRSR (write-status-register) command (0x01)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_01_CMD0	0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_01_CMD1	0x01000000
/* Set command word 2, set data count to 1 */
#define SPI020_01_CMD2	0x2
/* Set command word 4, instrction code = 0x01, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_01_CMD3	(0x01000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for REMS (read electronic manufacturer & device ID) command (0x90)
 ********************************************/
/* Set command word 0x000001, output the manufacturer ID first, the second byte is device ID */
#define SPI020_90_CMD0	0x0 //0x01 -> 0x00
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_90_CMD1	0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_90_CMD2	0x02 //0x4 //0x04 -> 0x02
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_90_CMD3	(0x90000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

extern UINT32  spi_reg_base;
extern UINT32  spi_ack_req;

/*******************************************
 * for structure definition
 ********************************************/
/* For system information. Be careful, this must be 4 alignment */
typedef struct _flash_info {
    UINT8	reserved;		/* original -> FLASH_CTL_SPI010/FLASH_CTL_SPI020 - for spi010/spi020 */
    UINT8	manufacturer;	/* Manufacturer id */
    UINT16	flash_id;		/* device id */
    UINT32	flash_size;		/* flash size in byte */
    UINT8	support_dual;	/* flash support dual read mode or not */
    UINT8	sys_version;	/* system version, get from SYS_VERSION_ADDR */
    UINT8	dev_mode;		/* current usb link type, 0/1/2/3 for unknow/SS/HS/FS */
    UINT8	vender_specific; /* specific purpose for vendor */
} SPI_Flash_T;

//--------------------------------------------------
// Function Prototypes
//--------------------------------------------------
extern void spi020_init(SPI_CLK_MODE spi_clk_mode,UINT8 spi_clk_divider);
extern UINT32 spi020_flash_probe(SPI_Flash_T *flash);

extern UINT32 spi020_rxfifo_depth(void);
extern void spi020_wait_rx_full(void);
extern void spi020_flash_w_state(UINT16 value);
extern UINT8 spi020_flash_r_state_OpCode_05(void);
extern UINT8 spi020_flash_r_state_OpCode_35(void);

extern void spi020_flash_read(UINT8 type, UINT32 offset, UINT32 len, void *buf);
extern void spi020_dma_read_stop(void);
extern void spi020_flash_write(UINT8 type, UINT32 offset, UINT32 len, void *buf);
extern void spi020_dma_write_stop(void);
extern void spi020_flash_chip_erase(void);
extern void spi020_flash_64kErase(UINT32 offset);
extern void spi020_flash_32kErase(UINT32 offset);
extern void spi020_flash_4kErase(UINT32 offset);
extern void dma_channel_set(UINT32 dmaRegCSR, UINT32 dmaRegCFG, UINT32 dmaSrcAddr, UINT32 dmaDstAddr);
extern void dma_channel_action(UINT32 size);
extern UINT32 spi020_txfifo_depth(void);
extern void spi020_wait_tx_empty(void);
#endif/* __SPI020_H__ */

