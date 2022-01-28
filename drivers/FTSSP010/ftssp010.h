/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftssp010.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      Registers offset definition     
 * -------------------------------------------------------------------------
 */
#include "portme_ftssp010.h"

#define FTSSP010_REG_CR0	0x0
#define FTSSP010_REG_CR1	0x4
#define FTSSP010_REG_CR2	0x8
#define FTSSP010_REG_STS	0xc
#define FTSSP010_REG_INTR_CR	0x10
#define FTSSP010_REG_INTR_STS	0x14
#define FTSSP010_REG_DATA_PORT	0x18
#define FTSSP010_REG_CR3	0x1C
#define FTSSP010_REG_REVISION	0x60
#define FTSSP010_REG_FEATURE	0x64

/* REG_CR0 field */
#define FTSSP010_CR0_LCDDCX	(1 << 21)
#define FTSSP010_CR0_LCDDCXS	(1 << 20)
#define FTSSP010_CR0_SPICONTX	(1 << 19)
#define FTSSP010_CR0_FLASHTX	(1 << 18)
#define FTSSP010_CR0_FSDBK	(1 << 17)
#define FTSSP010_CR0_SCLKFDBK	(1 << 16)
#define FTSSP010_CR0_SPIFSPO	(1 << 15) /* Frame/Sync polarity, SPI only */
#define FTSSP010_CR0_FFMT_MASK	(7 << 12)
#define FTSSP010_CR0_FFMT_SSP	(0 << 12)
#define FTSSP010_CR0_FFMT_SPI	(1 << 12)
#define FTSSP010_CR0_FFMT_MWR	(2 << 12)
#define FTSSP010_CR0_FFMT_I2S	(3 << 12)
#define FTSSP010_CR0_FFMT_ACL	(4 << 12)
#define FTSSP010_CR0_FFMT_SPDIF	(5 << 12)
#define FTSSP010_CR0_SPI_FLASH	(1 << 11) 
#define FTSSP010_CR0_VALIDITY	(1 << 10) // SPDIF validity
#define FTSSP010_CR0_FSDIS_MASK	(3 << 8)
#define FTSSP010_CR0_FSDIST(x)	((x & 0x3) << 8) // frame/sync and data distance, I2S only
#define FTSSP010_CR0_LOOPBACK	(1 << 7)
#define FTSSP010_CR0_LSB	(1 << 6) // 0: MSB, 1:LSB tx and rx first
#define FTSSP010_CR0_FSPO	(1 << 5) // Frame/Sync polarity, I2S or MWR only
#define FTSSP010_CR0_FSJSTFY	(1 << 4) // Padding data in front(1) or back(0) of serial data
#define FTSSP010_CR0_MODE_MASK	(3 << 2)
#define FTSSP010_CR0_MSTR_STREO	(3 << 2)
#define FTSSP010_CR0_MSTR_MONO	(2 << 2)
#define FTSSP010_CR0_SLV_STREO	(1 << 2)
#define FTSSP010_CR0_SLV_MONO	(0 << 2)
#define FTSSP010_CR0_MSTR_SPI	(3 << 2)
#define FTSSP010_CR0_SLV_SPI	(1 << 2)
#define FTSSP010_CR0_SCLKPO_0	(0 << 1) // SCLK polarity, SPI only
#define FTSSP010_CR0_SCLKPO_1	(1 << 1) // SCLK polarity, SPI only
#define FTSSP020_CR0_SCLKPH_0	(0 << 0) // SCLK phase, SPI only
#define FTSSP020_CR0_SCLKPH_1	(1 << 0) // SCLK phase, SPI only

/* REG_CR1 field */
#define FTSSP010_CR1_PDL(x)	((x & 0xff) << 24) // Padding data length
#define FTSSP010_CR1_PDL_MASK	(0xff << 24)
#define FTSSP010_CR1_SDL(x)	((x & 0x7f) << 16) // Serial data length
#define FTSSP010_CR1_SDL_MASK	(0x7f << 16)
#define FTSSP010_SDL_MAX_BYTES_MASK	(0x7f)
#define FTSSP010_CR1_SCLKDIV(x)	(x & 0xffff)
#define FTSSP010_CR1_SCLKDIV_MASK	(0xffff)

/* REG_CR2 field */
#define FTSSP010_CR2_FSOS(x)	((x & 0x3) << 10) // frame/sync output select, SPI only
#define FTSSP010_CR2_FSOS_MASK	(3 << 10)
#define FTSSP010_CR2_FS 	(1 << 9) // 0: low, 1: high frame/sync output
#define FTSSP010_CR2_TXEN	(1 << 8)
#define FTSSP010_CR2_RXEN	(1 << 7)
#define FTSSP010_CR2_SSPRST	(1 << 6)
#define FTSSP010_CR2_ACRST	(1 << 5)
#define FTSSP010_CR2_ACWRST	(1 << 4)
#define FTSSP010_CR2_TXFCLR	(1 << 3) // W1C, Clear TX FIFO
#define FTSSP010_CR2_RXFCLR	(1 << 2) // W1C, Clear RX FIFO
#define FTSSP010_CR2_TXDOE	(1 << 1) // TX Data Output Enable, SSP slave only
#define FTSSP010_CR2_SSPEN	(1 << 0)

/* REG_STS 0xC field */
#define FTSSP010_STS_TFVE(x)	((x >> 12) & 0x3f) // TX FIFO valid entries
#define FTSSP010_STS_RFVE(x)	((x >> 4) & 0x3f) // RX FIFO valid entries
#define FTSSP010_STS_BUSY	(1 << 2)
#define FTSSP010_STS_TFNF	(1 << 1) // TX FIFO not full
#define FTSSP010_STS_RFF	(1 << 0) // RX FIFO full

/* REG_INTR_CR 0x10 field */
#define FTSSP010_INTCR_TXCIEN	(1 << 18)
#define FTSSP010_INTCR_RFTHOD_UNIT	(1 << 17)
#define FTSSP010_INTCR_TFTHOD(x)	((x & 0x1f) << 12)
#define FTSSP010_INTCR_TFTHOD_MASK	(0x1f << 12)
#define FTSSP010_INTCR_RFTHOD(x)	((x & 0x1f) << 7)
#define FTSSP010_INTCR_RFTHOD_MASK	(0x1f << 7)
#define FTSSP010_INTCR_AC97FCEN	(1 << 6)
#define FTSSP010_INTCR_TFDMAEN	(1 << 5)
#define FTSSP010_INTCR_RFDMAEN	(1 << 4)
#define FTSSP010_INTCR_TFTHIEN	(1 << 3)
#define FTSSP010_INTCR_RFTHIEN	(1 << 2)
#define FTSSP010_INTCR_TFURIEN	(1 << 1)
#define FTSSP010_INTCR_RFORIEN	(1 << 0)

/* REG_INTR_STS 0x14 field */
#define FTSSP010_INTSTS_TXCI	(1 << 5)
#define FTSSP010_INTSTS_AC97CI	(1 << 4)
#define FTSSP010_INTSTS_TFTHI	(1 << 3) // TX FIFO threshold
#define FTSSP010_INTSTS_RFTHI	(1 << 2) // RX FIFO threshold
#define FTSSP010_INTSTS_TFUI	(1 << 1) // TX FIFO underrun
#define FTSSP010_INTSTS_RFORI	(1 << 0) // RX FIFO overrun

/* REG_CR3 0x1C field */
#define FTSSP010_CR3_DPDL(x)	((x & 0xff) << 16) // Padding Data length
#define FTSSP010_CR3_DPDL_MASK	(0xff << 16)
#define FTSSP010_CR3_DPDLEN	(1 << 12)
#define FTSSP010_CR3_PCL(x)	(x & 0x3ff) // Padding Cycle length
#define FTSSP010_CR3_PCL_MASK	0x3ff

// REG_FEATURE 0x64 field
#define FTSSP010_FEA_TXFIFO_DEPTH(x)	((x >> 16) & 0xff)
#define FTSSP010_FEA_RXFIFO_DEPTH(x)	((x >> 8) & 0xff)
 
const char * ftssp010_get_fformat_string(int base);
void ftssp010_set_sclkdiv(int base);
void ftssp010_ssp_reset(int base_addr);
void ftssp010_clear_txrxfifo(int base_addr);
int ftssp010_set_data_length(int base_addr, int sdl);
void ftssp010_enable(int base_addr, int tx, int rx);
int ftssp010_txfifo_valid_entries(int base_addr);
int ftssp010_rxfifo_valid_entries(int base_addr);
int ftssp010_transfer_data(int tx_addr, const void *tx_buf,
			   int rx_addr, void *rx_buf,
			   int len);
extern int sdl_in_bytes;

// SPI only
typedef enum {
	SPI_CS_LOW = 0,
	SPI_CS_HI = FTSSP010_CR0_SPIFSPO,
} SPI_CHIP_SELECT;

typedef enum {
	// CLKPO = 0, CLKPHA = 0
	SPI_MODE_0 = (FTSSP010_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_0),
	// CLKPO = 0, CLKPHA = 1
	SPI_MODE_1 = (FTSSP010_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_1),
	// CLKPO = 1, CLKPHA = 0
	SPI_MODE_2 = (FTSSP010_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_0),
	// CLKPO = 1, CLKPHA = 1
	SPI_MODE_3 = (FTSSP010_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_1),
	SPI_MODE_MAX
} SPI_MODE_TYPE;

int ftssp010_set_pcl(int base, int val, int penable);
void ftssp010_ssp_slave_init(int cr0);
void ftssp010_ssp_master_init(int cr0);
void ftssp010_ssp_init(void);
void ftssp010_spi_init(SPI_CHIP_SELECT cs_low, int lsb, SPI_MODE_TYPE mode);
int ftssp010_transfer_data_bidirect(int addr1, const void *tx1_buf,
				    void *rx1_buf, int *len1,
			   	    int addr2, const void *tx2_buf,
				    void *rx2_buf, int *len2);

extern int pcl;
// I2S only
void ftssp010_i2s_init(int lsb);
void ftssp010_set_pdl(int base);
void ftssp010_set_fsjtfy(int base);
void ftssp010_set_fsdist(int base);
void ftssp010_set_mode(int base);


extern int pdl_r, pdl_l;
extern int stereo;
extern int pad_data_back;
extern int fs_data_dist;
