/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftssp010_i2s.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      I2S frame format   
 * -------------------------------------------------------------------------
 */

#include "ftssp010.h"

/**
 * CR0.FSJSTFY[4:4]] : 0 is padding data in back, 1 padding data
 *		       in front of serial data. Only used when
 *		       CR1.PDL != 0.
 *
 * CR0.FSDIST[9:8] : Frame/Sync. and data distance
 */
int pdl_r = 1, pdl_l = 1;
int stereo = 1;
int pad_data_back;
int fs_data_dist;

/**
 * If use different length of padding data, CR3.DPDLEN == 1, then:
 * PDL at CR1 for fs equal to one
 * PDL at CR3 for fs equal to zero
 */
void ftssp010_set_pdl(int base)
{
	int cr1, cr3;

	cr1 = inl(base + FTSSP010_REG_CR1);
	cr1 &= ~FTSSP010_CR1_PDL_MASK;
	cr1 |= FTSSP010_CR1_PDL(pdl_r);
	outl(cr1, base + FTSSP010_REG_CR1);

	cr3 = inl(base + FTSSP010_REG_CR3);
	cr3 &= ~FTSSP010_CR3_DPDLEN;
	if (pdl_r != pdl_l) {
		cr3 &= ~FTSSP010_CR3_DPDL_MASK;
		cr3 |= (FTSSP010_CR3_DPDL(pdl_l) | FTSSP010_CR3_DPDLEN);
	}
	outl(cr3, base + FTSSP010_REG_CR3);
}

void ftssp010_set_fsjtfy(int base)
{
	int cr0;

	cr0 = inl(base + FTSSP010_REG_CR0);

	if (pad_data_back)
		cr0 &= ~FTSSP010_CR0_FSJSTFY;
	else
		cr0 |= FTSSP010_CR0_FSJSTFY;

	outl(cr0, base + FTSSP010_REG_CR0);
}

void ftssp010_set_fsdist(int base)
{
	int cr0;

	cr0 = inl(base + FTSSP010_REG_CR0);

	cr0 &= ~FTSSP010_CR0_FSDIS_MASK;
	cr0 |= FTSSP010_CR0_FSDIST(fs_data_dist);

	outl(cr0, base + FTSSP010_REG_CR0);
}

void ftssp010_set_mode(int base)
{
	int cr0;

	cr0 = inl(base + FTSSP010_REG_CR0);

	cr0 &= ~FTSSP010_CR0_MODE_MASK;
	if (stereo) {
		if (base == FTSSP010_REG_BASE_M)
			cr0 |= FTSSP010_CR0_MSTR_STREO;
		else
			cr0 |= FTSSP010_CR0_SLV_STREO;
	} else {
		if (base == FTSSP010_REG_BASE_M)
			cr0 |= FTSSP010_CR0_MSTR_MONO;
		else
			cr0 |= FTSSP010_CR0_SLV_MONO;
	}

	outl(cr0, base + FTSSP010_REG_CR0);
}

void ftssp010_i2s_slave_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);

	cr0 |= FTSSP010_CR0_FFMT_I2S;
	outl(cr0, FTSSP010_REG_BASE_S + FTSSP010_REG_CR0);

	ftssp010_set_pdl(FTSSP010_REG_BASE_S);
	ftssp010_set_fsjtfy(FTSSP010_REG_BASE_S);
	ftssp010_set_fsdist(FTSSP010_REG_BASE_S);
	ftssp010_set_mode(FTSSP010_REG_BASE_S);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S); 

	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl_in_bytes);

	printf(" Slave CR0 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR0));
	printf(" Slave CR1 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR1));
	printf(" Slave CR2 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR2));
	printf(" Slave CR3 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR3));
}

void ftssp010_i2s_master_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	cr0 |= FTSSP010_CR0_FFMT_I2S;
	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);

	ftssp010_set_pdl(FTSSP010_REG_BASE_M);
	ftssp010_set_fsjtfy(FTSSP010_REG_BASE_M);
	ftssp010_set_fsdist(FTSSP010_REG_BASE_M);
	ftssp010_set_mode(FTSSP010_REG_BASE_M);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M); 
	
	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);

	ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes);

	printf(" Master CR0 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR0));
	printf(" Master CR1 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR1));
	printf(" Master CR2 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR2));
	printf(" Master CR3 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR3));
}

void ftssp010_i2s_init(int lsb)
{
	int cr0 = 0;

	pad_data_back = 1;
	fs_data_dist = 1;

	printf("I2S init mode %s\n\r", stereo ?
	       "stereo" : "mono");

	if (lsb)
		cr0 |= FTSSP010_CR0_LSB;

	ftssp010_i2s_slave_init(cr0 | FTSSP010_CR0_LSB);

	ftssp010_i2s_master_init(cr0 | FTSSP010_CR0_FSDBK
				 | FTSSP010_CR0_SCLKFDBK
				 | FTSSP010_CR0_LSB);

}
