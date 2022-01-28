/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftssp010_spi.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      commands to do verification   
 * -------------------------------------------------------------------------
 */

#include "ftssp010.h"

int pcl;

int ftssp010_set_pcl(int base, int val, int penable)
{
	int cr3;
	int val_tmp;

	if (val & ~FTSSP010_CR3_PCL_MASK) {
		printf("pcl val range from 0 to 1023 cycles\n\r");
		return 1;
	}
	cr3 = inl(base + FTSSP010_REG_CR3);
	cr3 &= ~FTSSP010_CR3_PCL_MASK;
	if(val != 0) {
		val_tmp = val - 1;
	} else {
		val_tmp = val;
	}
	cr3 |= FTSSP010_CR3_PCL(val_tmp);
	outl(cr3, base + FTSSP010_REG_CR3);

	if (penable == 1) {
	printf(" Addr 0x%x CR3 0x%x, pcl %d\n\r", base + FTSSP010_REG_CR3,
			inl(base + FTSSP010_REG_CR3), ((val)));
	}
	return 0;
}

/**
 * Enable Slave before Master
 *
 * cs_low equals to 0 means  frame/sync(chip select) active low.
 * lsb equals to 0 means MSB tx first. 
 */
char *mode_string[SPI_MODE_MAX] = { "CLKPO = 0, CLKPHA = 0",
				    "CLKPO = 0, CLKPHA = 1",
				    "CLKPO = 1, CLKPHA = 0",
				    "CLKPO = 1, CLKPHA = 1",
				 };
void ftssp010_spi_init(SPI_CHIP_SELECT cs_low, int lsb, SPI_MODE_TYPE mode)
{
	int cr0, md;

	switch (mode) {
		case SPI_MODE_0:
			md = 0;
			break;
		case SPI_MODE_1:
			md = 1;
			break;
		case SPI_MODE_2:
			md = 2;
			break;
		case SPI_MODE_3:
			md = 3;
			break;
		default:
			printf("SPI undefined mode 0x%x\n\r", mode);
			return;
	}
	printf("SPI init mode %s\n\r", mode_string[md]);

	cr0 = FTSSP010_CR0_FFMT_SPI;
	cr0 |= mode;
	cr0 |= cs_low;
	if (lsb)
		cr0 |= FTSSP010_CR0_LSB;

	ftssp010_ssp_slave_init(cr0);
	ftssp010_ssp_master_init(cr0);
}
