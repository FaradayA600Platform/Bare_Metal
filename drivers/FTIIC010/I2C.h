/***************************************************************************************
* Revise History                                                                       *
* 2003/05/28 Peter LIAO, Correct constant macro definition:  I2C_TGSR and I2C_BMR      *
* 2014/03/26 Alex Chang, Modify codes to sync with SW programming guide                *
****************************************************************************************/

#ifndef __I2C_H
#define __I2C_H

#include "types.h"

#define SLAVE_DEVICE_ADDR	0x2f
#define MAX_ARRAY_SIZE		1024

/* I2C Register */

#define I2C_CR		               	0x00
#define I2C_SR 		               	0x04
#define I2C_CDR		              	0x08
#define I2C_DR                   	0x0C
#define I2C_SAR		              	0x10
#define I2C_TGSR                  	0x14  /* 2003/05/28 Correct by Peter */
#define I2C_BMR                   	0x18  /* 2003/05/28 Correct by Peter */
#define I2C_BSTMR                   	0x1C
#define I2C_MAXTR                  	0x20
#define I2C_MINTR                  	0x24
#define I2C_METR                   	0x28
#define I2C_SETR                  	0x2C
#define I2C_REVISION               	0x30
#define I2C_FEATURE                	0x34


/* I2C Control register */

#define FLOW_CTRL_READ					0x2000000 /*burst read*/
#define FLOW_CTRL_WRITE					0x1000000 /*burst write*/
#define I2C_CR_SBI_EN					0x200000 /* start byte interrupt */
#define I2C_CR_HSI_EN					0x100000 /* high spped mode interrupt enable*/
#define I2C_CR_HS_MODE					0x80000 /* high spped mode */
#define I2C_CR_ARB_OFF					0x40000 /* turn arbitration lost off */
#define I2C_CR_STARTI_EN				0x4000  /* start condition */
#define I2C_CR_ALI_EN					0x2000  /* Arbitration lose */
#define I2C_CR_SAMI_EN					0x1000  /* slave address match */
#define I2C_CR_STOPI_EN					0x0800  /* stop condition */
#define I2C_CR_NACKI_EN					0x0400  /* non ACK response */
#define I2C_CR_TDI_EN					0x0200  /* data receive */
#define I2C_CR_BSTTHODI_EN				0x0100  /* data transmit */
#define I2C_CR_TB_EN					0x0080  /* transfer byte enable */
#define I2C_CR_NACK					0x0040  /* ack sent */
#define I2C_CR_STOP					0x0020  /* stop */
#define I2C_CR_START					0x0010  /* start */
#define I2C_CR_GC_EN					0x0008  /* general call */
#define I2C_CR_MST_EN					0x0004  /* enable clock */
#define I2C_CR_I2C_EN					0x0002  /* enable I2C */
#define I2C_CR_I2C_RST					0x0001  /* reset I2C */

#define I2C_CR_ENABLE					(I2C_CR_ALI_EN|I2C_CR_NACKI_EN|I2C_CR_TDI_EN|I2C_CR_MST_EN|I2C_CR_I2C_EN)
#define I2C_CR_SLAVE_ENABLE				(I2C_CR_ALI_EN|I2C_CR_SAMI_EN|I2C_CR_TDI_EN|I2C_CR_I2C_EN|\
							 I2C_CR_STOPI_EN|I2C_CR_STARTI_EN)
#define I2C_CR_DISABLE					(0)

/* I2C Status Register */
#define I2C_SR_SBS						0x800000
#define I2C_SR_HSS						0x400000
#define I2C_SR_START						0x800
#define I2C_SR_AL						0x400
#define I2C_SR_GC						0x200
#define I2C_SR_SAM						0x100
#define I2C_SR_STOP						0x080
#define I2C_SR_NACK						0x040
#define I2C_SR_TD						0x020
#define I2C_SR_BSTTHOD						0x010
#define I2C_SR_BB						0x008
#define I2C_SR_BUSY						0x004
#define I2C_SR_RESERVED						0x002
#define I2C_SR_RW						0x001

/*I2CAddressRegister*/
#define ADDR		0
#define ADDR2		7
#define MEM_IDX		16
#define MEM_IDX2	24

/*I2CBurstModeRegister*/
#define BSTTHOD		0
#define BSTTDC		8
#define BUFHW		16

/* I2C clock divided register */

#define I2C_CLKCOUNT                0x3ff


/* I2C slave address register */

#define ADDR_MASK			0x7F
#define TENBITADDR_MASK			0x3FF
#define ADDR10EN			0x1000	/* 10-bit address slave mode */
#define M2BIDX_EN			0x2000
#define MEM_IDX_MASK			0xFF0000
#define MEM_IDX2_MASK			0xFF000000

#define ADDR2_OFF			7
#define MEM_IDX_OFF			16
#define MEM_IDX2_OFF			24

/* Bus Monitor Register */

#define I2C_SCL                     0x2
#define I2C_SDA                     0x1


// --------------------------------------------------------------------
// config
// --------------------------------------------------------------------
#define I2C_Default_FREQ        	(45 * 1024)
#define I2C_MAX_FREQ                (400 * 1024)
#define WRITE_TIME_OUT				1000		// 100000
#define READ_TIME_OUT				1000		// 100000
#define BURST_WRITE_TIME_OUT			100000
#define BURST_READ_TIME_OUT			100000

#define I2C_GSR_Value		0x400		// general value of gsr & tsr
#define I2C_TSR_Value		0x1

#define LOW_SPEED	51200		// 50KHz
#define HIGH_SPEED	2726298		// 3.4MHz

// --------------------------------------------------------------------
// I2C_Dev->address_mode
// --------------------------------------------------------------------
#define ADDR_MODE_NONE			0			/// no internal address
#define ADDR_MODE_8BIT			1			/// internal address is 8 bit
#define ADDR_MODE_16BIT			2			/// internal address is 16 bit

#define I2C_BUFFER_SIZE			256

typedef struct
{
	unsigned long io_base;
	int irq;
	int isFiq;
	unsigned long status;
	int address_mode;

	// for slave, unused for master 
	int slaveState; // the current state machine of I2C_Slave_ISR()
	int ind;         //index of I2C_DataBuf
	unsigned int dataBuf[I2C_BUFFER_SIZE]; // internal storage of slave
}I2C_Dev;



// --------------------------------------------------------------------
// API
// --------------------------------------------------------------------
void I2C_Init(I2C_Dev *dev, unsigned long io_base, uint32_t gsr, uint32_t tsr, uint32_t ls, uint32_t hs);
int I2C_MasterReceive(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num);
int I2C_MasterTransfer(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num);
int I2C_CombinedFormatTest_EEPROM(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num);
int I2C_CombinedFormatTest(I2C_Dev *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num);
int I2C_Read(I2C_Dev *dev, unsigned int address, unsigned int index);
int I2C_Write (I2C_Dev *dev, unsigned int address, unsigned int index, unsigned char value);
void I2C_SetAddressMode(I2C_Dev *dev, int mode);

// --------------------------------------------------------------------
// private API
// --------------------------------------------------------------------
uint32_t fLib_FTIIC010_ReadReg(I2C_Dev *dev, uint32_t reg_offset);
void fLib_FTIIC010_WriteReg(I2C_Dev *dev, uint32_t reg_offset, uint32_t value);



#endif
