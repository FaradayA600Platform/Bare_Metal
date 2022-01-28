/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftssp010.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       DESCRIPTION
 * 2015/06      BingYao      commands to do verification   
 * -------------------------------------------------------------------------
 */

#include "ftssp010.h"


int sclkdiv = 32;
int ssp_debug = 0;
int sdl_in_bytes = 1;
int master_transmit_done = 0;

const char *ffmt_str[] = { "SSP", "SPI", "MicroWire", "I2S", "ACL", "SPDIF" };

const char *ftssp010_get_fformat_string(int base)
{
	int cr0;

	cr0 = inl(base + FTSSP010_REG_CR0);
	cr0 = (cr0 >> 12) & 0x7;

	return ffmt_str[cr0];
}

void ftssp010_set_sclkdiv(int base)
{
	int cr1;

	cr1 = inl(base + FTSSP010_REG_CR1);
	cr1 &= ~FTSSP010_CR1_SCLKDIV_MASK;
	cr1 |= FTSSP010_CR1_SCLKDIV(sclkdiv);

	outl(cr1, base + FTSSP010_REG_CR1); 
}

void ftssp010_ssp_reset(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_SSPRST;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}

void ftssp010_clear_txfifo(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_TXFCLR;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}

void ftssp010_clear_rxfifo(int base_addr)
{
	int cr2;

	cr2 = inl(base_addr + FTSSP010_REG_CR2);

	cr2 |= FTSSP010_CR2_RXFCLR;

	outl(cr2, base_addr + FTSSP010_REG_CR2);
}
/**
 * CR1.SDL has 7 bits means maximum: 2^7 = 127 + 1 bits.
 * 128 bits = 16 bytes
 */
int ftssp010_set_data_length(int base_addr, int sdl)
{
	int cr1;

	//convert to bits;
	sdl <<= 3;
	sdl -= 1;
	if (sdl & ~FTSSP010_SDL_MAX_BYTES_MASK) {
		printf(" sdl val range from 1 to 16 bytes\n\r");
		return 1;
	}

	cr1 = inl(base_addr + FTSSP010_REG_CR1);
	cr1 &= ~FTSSP010_CR1_SDL_MASK;

	cr1 |= FTSSP010_CR1_SDL(sdl);

	outl(cr1, base_addr + FTSSP010_REG_CR1);

	printf(" Addr 0x%x CR1 0x%x, sdl %d\n\r", base_addr,
		inl(base_addr + FTSSP010_REG_CR1), ((sdl+1)>>3));

	return 0;
}

void ftssp010_enable(int base_addr, int tx, int rx)
{
	int cr2 = 0;

	if (tx || rx)
		cr2 = (FTSSP010_CR2_SSPEN | FTSSP010_CR2_TXDOE);

	if (tx)
		cr2 |= FTSSP010_CR2_TXEN;

	if (rx)
		cr2 |= FTSSP010_CR2_RXEN;

	outl(cr2, base_addr + FTSSP010_REG_CR2);

	if (ssp_debug)
		printf(" Addr 0x%x CR2 0x%x\n\r", base_addr,
			inl(base_addr + FTSSP010_REG_CR2));
}

/**
 * Return the number of entries TX FIFO can be written to
 */
int ftssp010_txfifo_depth(int base_addr)
{
	int depth;

	depth = FTSSP010_FEA_TXFIFO_DEPTH(
		inl(base_addr + FTSSP010_REG_FEATURE));
	depth += 1;
	return depth;
}

int ftssp010_txfifo_not_full(int base_addr)
{
	int sts;

	sts = inl(base_addr + FTSSP010_REG_STS);

	return (sts & FTSSP010_STS_TFNF);
}

int ftssp010_txfifo_valid_entries(int base_addr)
{
	return FTSSP010_STS_TFVE(inl(base_addr + FTSSP010_REG_STS));

}

/**
 * Return the number of entries RX FIFO can be written to
 */
int ftssp010_rxfifo_depth(int base_addr)
{
	int depth;

	depth = FTSSP010_FEA_RXFIFO_DEPTH(
		inl(base_addr + FTSSP010_REG_FEATURE));
	depth += 1;
	return depth;
}

int ftssp010_rxfifo_full(int base_addr)
{
	int sts;

	sts = inl(base_addr + FTSSP010_REG_STS);

	return (sts & FTSSP010_STS_RFF);
}

int ftssp010_rxfifo_valid_entries(int base_addr)
{
	int ent;

	ent = FTSSP010_STS_RFVE(inl(base_addr + FTSSP010_REG_STS));

	if (ssp_debug)
		printf("Rx fifo entries %d\n\r", ent);

	return ent;
}

static void ftssp010_write_word(int base, const void *data, int wsize)
{
	unsigned int    tmp = 0;

	if (data) {
		switch (wsize) {
		case 1:
			tmp = *(const uint8_t *)data;
			break;

		case 2:
			tmp = *(const uint16_t *)data;
			break;

		default:
			tmp = *(const uint32_t *)data;
			break;
		}
	}

	outl(tmp, base + FTSSP010_REG_DATA_PORT);
}

static void ftssp010_read_word(int base, void *buf, int wsize)
{
	unsigned int    data = inl(base + FTSSP010_REG_DATA_PORT);

	if (buf) {
		switch (wsize) {
		case 1:
			*(uint8_t *) buf = data;
			break;

		case 2:
			*(uint16_t *) buf = data;
			break;

		default:
			*(uint32_t *) buf = data;
			break;
		}
	}
}

/**
 * len unit is bytes
 *
 * Return number of fifo written.
 */
int ftssp010_fill_in_fifo(int tx_addr, const void *buf,
			  int rx_addr, int *len)
{
	int count = 0;
	int rxfifo, fifo, wsize, i;

	rxfifo = ftssp010_rxfifo_depth(rx_addr);

	//clear before start filling in
	ftssp010_clear_txfifo(tx_addr);

	i = 0;
	while (ftssp010_txfifo_not_full(tx_addr)) {

		if (!*len || !buf)
			break;

		if (i == 0) {
			i = sdl_in_bytes;
			fifo = (sdl_in_bytes + 3) / 4;
		}

		//rx fifo doesn't have enough entries to receive
		if ((count + fifo) > rxfifo)
			break;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		ftssp010_write_word(tx_addr, buf, wsize);

		i -= wsize;
		*len -= wsize;
		fifo--;
		count++;
		// Always add 4 bytes for buffer pointer
		buf += 4;
	}

	return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ftssp010_take_out_fifo(int rx_addr, void *buf, int count)
{
	int i, wsize;

	i = 0;
	while (count) {

		if (!buf)
			break;

		while (!ftssp010_rxfifo_valid_entries(rx_addr)) {
			;
		}

		if (i == 0)
			i = sdl_in_bytes;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		ftssp010_read_word(rx_addr, buf, wsize);

		i -= wsize;
		count--;
		buf += 4;

	}

	return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ftssp010_take_out_fifo_bidirect(int rx1_addr, void *buf1,
				    int rx2_addr, void *buf2,
				    int count)
{
	int i, wsize;

	i = 0;
	while (count) {

		if (!buf1 || !buf2)
			break;

		while (!ftssp010_rxfifo_valid_entries(rx1_addr)) {
			;
		}

		if (i == 0)
			i = sdl_in_bytes;

		if (i > 3)
			wsize = 4;
		else
			wsize = i;

		ftssp010_read_word(rx1_addr, buf1, wsize);
		ftssp010_read_word(rx2_addr, buf2, wsize);

		i -= wsize;
		count--;
		buf1 += 4;
		buf2 += 4;

	}

	return count;
}


/**
 * len unit is bytes
 *
 * Output:
 *   len1, len2: remaining number of bytes not transfered.
 */
int ftssp010_transfer_data_bidirect(int addr1, const void *tx1_buf,
				    void *rx1_buf, int *len1,
			   	    int addr2, const void *tx2_buf,
				    void *rx2_buf, int *len2)
{
	int count1, count2, fifo;

	// Always disable Master before Slave
	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);
	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	count1 = ftssp010_fill_in_fifo(addr1, tx1_buf, addr2, len1);
	count2 = ftssp010_fill_in_fifo(addr2, tx2_buf, addr1, len2);

	if (count1 != count2) {
		printf("Fill Tx fifo count not same cnt1 = %d, cnt2 = %d\n\r",
		       count1, count2);
		return 1;
	}

	ftssp010_clear_rxfifo(addr1);
	ftssp010_clear_rxfifo(addr2);

	// Must enable Slave first
	ftssp010_enable(FTSSP010_REG_BASE_S, 1, 1);
	ftssp010_enable(FTSSP010_REG_BASE_M, 1, 1);

	// Receive the data
	fifo = ftssp010_take_out_fifo_bidirect(addr1, rx1_buf, addr2,
					       rx2_buf, count1);
	if (fifo != 0) {
		printf("0x%0x: Tx fifo use %d entries,  rx fifo use %d entries\n\r",
			addr1, count1, (count1 - fifo));
		return 1;
	}

	return 0;
}

/**
 * len unit is bytes
 *
 * Return number of bytes already transfered.
 */
int ftssp010_transfer_data(int tx_addr, const void *tx_buf,
			   int rx_addr, void *rx_buf,
			   int len)
{
	int total_bytes, count = 0;
	int fifo, tx;

	total_bytes = len;

	// Always disable Master before Slave
	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);
	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	count = ftssp010_fill_in_fifo(tx_addr, tx_buf, rx_addr, &len);

	ftssp010_clear_rxfifo(rx_addr);

	// Always enable rxside before txside
	tx = (tx_addr == FTSSP010_REG_BASE_S)? 1 : 0;
	ftssp010_enable(FTSSP010_REG_BASE_S, tx, !tx);
	ftssp010_enable(FTSSP010_REG_BASE_M, !tx, tx);

	// Receive the data
	fifo = ftssp010_take_out_fifo(rx_addr, rx_buf, count);

	//Count should be zero: tx fifo entries used should the same
	// with rx fifo entries
	if (fifo != 0) {
		printf("Tx fifo use %d entries,  rx fifo use %d entries\n\r",
			count, (count - fifo));
	}

	return total_bytes - len;
}

void ftssp010_ssp_slave_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_S);

	cr0 |= FTSSP010_CR0_SLV_SPI;
	outl(cr0, FTSSP010_REG_BASE_S + FTSSP010_REG_CR0);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_S);

	ftssp010_enable(FTSSP010_REG_BASE_S, 0, 0);

	ftssp010_set_data_length(FTSSP010_REG_BASE_S, sdl_in_bytes);

	ftssp010_set_pcl(FTSSP010_REG_BASE_S, pcl, 0);

	printf(" Slave CR0 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR0));
	printf(" Slave CR1 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR1));
	printf(" Slave CR2 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR2));
	printf(" Slave CR3 0x%x\n\r", inl(FTSSP010_REG_BASE_S +
				       FTSSP010_REG_CR3));
}

void ftssp010_ssp_master_init(int cr0)
{
	ftssp010_ssp_reset(FTSSP010_REG_BASE_M);

	cr0 |= FTSSP010_CR0_MSTR_SPI;
	outl(cr0, FTSSP010_REG_BASE_M + FTSSP010_REG_CR0);

	ftssp010_set_sclkdiv(FTSSP010_REG_BASE_M);

	ftssp010_enable(FTSSP010_REG_BASE_M, 0, 0);

	ftssp010_set_data_length(FTSSP010_REG_BASE_M, sdl_in_bytes);

	ftssp010_set_pcl(FTSSP010_REG_BASE_M, pcl, 0);

	printf(" Master CR0 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR0));
	printf(" Master CR1 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR1));
	printf(" Master CR2 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR2));
	printf(" Master CR3 0x%x\n\r", inl(FTSSP010_REG_BASE_M +
					FTSSP010_REG_CR3));
}

void ftssp010_ssp_init(void)
{
	int cr0 = 0;

	printf("SSP init\n\r");

	cr0 = FTSSP010_CR0_FFMT_SSP;
	ftssp010_ssp_slave_init(cr0);
	ftssp010_ssp_master_init(cr0);
}
