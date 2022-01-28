/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2019/07/15   BingYao           
 * -------------------------------------------------------------------------
 */
#include "ftspi020.h"
#include "ftspi020_cntr.h"

struct cmd {
	char *name;
	char *usage;
	int(*func) (int argc, char * const argv[]);
};

typedef struct cmd cmd_t;

#define FMT_UINT	"%08X"
#define CMDLEN	50

static int FTSPI020_check_erase(int ce, int offset, int size);

static int do_dbg(int argc, char * const  argv[]);
static int do_rd32(int argc, char * const  argv[]);
static int do_wr32(int argc, char * const  argv[]);
static int FTSPI020_write(int argc, char * const argv[]);
static int FTSPI020_read(int argc, char * const argv[]);
static int FTSPI020_write_spare(int argc, char * const argv[]);
static int FTSPI020_read_spare(int argc, char * const argv[]);
static int FTSPI020_copy(int argc, char * const argv[]);
static int FTSPI020_erase(int argc, char * const argv[]);
static int FTSPI020_erase_all(int argc, char * const argv[]);
static int FTSPI020_reset(int argc, char * const argv[]);
static int FTSPI020_report_status(int argc, char * const argv[]);
static int FTSPI020_burnin(int argc, char * const argv[]);
static int FTSPI020_report_performance(int argc, char * const argv[]);
static int FTSPI020_transfer_type(int argc, char * const argv[]);
static int FTSPI020_use_interrput(int argc, char * const argv[]);
static int FTSPI020_enter_qpi(int argc, char * const argv[]);
static int FTSPI020_abort_test(int argc, char * const argv[]);
static int FTSPI020_scan_bad_blocks(int argc, char * const argv[]);
static int FTSPI020_xip_set(int argc, char * const argv[]);
static int FTSPI020_list_ce(int argc, char * const argv[]);
static int FTSPI020_rdwr_test(int argc, char * const argv[]);

static cmd_t ftspi020_cmd_tbl[] = {
	{"md", "<addr> [num]", do_rd32},
	{"mw", "<addr> <data> [num [inc]]", do_wr32},
	{"wr", "<ce> <type> <offset> <DataCnt> [1:wr with cmp, 0xff: burn image]", FTSPI020_write},
	{"rd", "<ce> <type> <offset> <DataCnt> [show]", FTSPI020_read},
	{"wr_sp", "<ce> <row> <col> <bytecnt>", FTSPI020_write_spare},
	{"rd_sp", "<ce> <row> <col>", FTSPI020_read_spare},
	{"cp", "<ce> <src_row> <dst_row> <row_count>", FTSPI020_copy},
	{"er", "<ce> <type> <offset> <DataCnt> [er with cmp]",
							FTSPI020_erase},
	{"er_all", "<ce list> [er with cmp]", FTSPI020_erase_all},
	{"rst", "<ce>", FTSPI020_reset},
	{"sts", "<ce> [opcode]", FTSPI020_report_status},
	{"burnin", "<ce> <offset> [1: copy]", FTSPI020_burnin},
	{"perf", "<ce> <len> [type]", FTSPI020_report_performance},
	{"tr", "<ctrl> <pio|dma>", FTSPI020_transfer_type},
	{"intr", "<ctrl> <0|1>", FTSPI020_use_interrput},
	{"qpi", "<ce> <0|2|4>", FTSPI020_enter_qpi},
	{"abort", "<ctrl> <0|1>", FTSPI020_abort_test},
	{"bbt", "[ce] <scan>", FTSPI020_scan_bad_blocks},
	{"xip", "<ctrl> <0|1> [base addr]", FTSPI020_xip_set},
	{"ls", "", FTSPI020_list_ce},
	{"tst", "<ce>", FTSPI020_rdwr_test},
	{"dbg", "", do_dbg},
	{"quit", "", 0},
	{0} /* end of ftspi020_cmd_tbl */
};

struct spi_flash *spi_flash_info[FTSPI020_TOTAL_CE] = {0};


extern int g_spi020_rd_buf_length, g_spi020_wr_buf_length;

int do_dbg(int argc, char * const  argv[])
{
	g_debug =  strtoul(argv[1], 0, 0);
	return 0;
}

#if 0
/**
 * SPI address marking test: HW will mask out MSB bits(see register 0x2C)
 * This is for FPGA verification purpose only
 */
static int FTSPI020_get_mask(struct spi_flash *slave)
{
	int mask_bits;

	mask_bits = (int) ~(slave->size - 1);

	return (int) (rand() & mask_bits);
}
#endif

/* Usage: md <addr> [num] */
int do_rd32(int argc, char * const  argv[])
{
	uint32_t *pd;
	int i, j;
	uint32_t addr;
	int num;
	int data;

	if ((argc < 2) || (argc > 3))
		return 1;

	addr = strtoul(argv[1], 0, 0);

	if (argc == 3)
		num = strtoul(argv[2], 0, 0);
	else
		num = 4;

	pd = (uint32_t *) ((uintptr_t) addr);
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (uintptr_t) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return 0;
}

/* Usage: mw <addr> <data> [num [inc]] */
int do_wr32(int argc, char * const  argv[])
{
	uint32_t *pd;
	int i, j;
	uint32_t addr;
	int num;
	int data;
	int datainc;

	if ((argc < 3) || (argc > 5))
		return 1;

	addr = strtoul(argv[1], 0, 0);
	data = strtoul(argv[2], 0, 16);

	num = 1;
	datainc = 0;
	if (argc >= 4) {
		num = strtoul(argv[3], 0, 0);

		if (argc == 5)
			datainc = strtoul(argv[4], 0, 0);
	}

	pd = (uint32_t *) ((uintptr_t) addr);
	for (i = 0; i < num; i++) {
		*pd++ = data;
		data += datainc;
	}

	pd = (uint32_t *) ((uintptr_t) addr);
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (uintptr_t) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return 0;
}

/* wr <ce> <type> <offset> <DataCnt> [1:wr with cmp, 0xff: burn image] */
int FTSPI020_write(int argc, char * const argv [])
{

	char *write_buf, *read_buf;
	uint32_t i, ce, wr_type, addr, len, burn;
	char ch;
	
	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	if (argc < 5) {
		prints("%d arguments for wr isn't allowed\n", argc);
		return 1;
	} 
	
	ce = strtol(argv[1], 0 , 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %lu\n", ce);
		return 1;
	}

	wr_type = strtol(argv[2], 0, 0);
	addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	/* Check if bad block for SPI NAND flash */
	if (spi_flash_info[ce]->is_bad_block) {
		if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce],
						     addr)) {
			prints ("You are about to program to bad block " \
				"0x%lx.\n", addr);
			return 0;
		}
	}

	if (argc == 6)
		burn = strtol(argv[5], 0 ,0);
	else
		burn = 0;

	/* 1:wr with cmp, 2: burn image */
	if (burn == 0xff){
		prints (" You are about to burn rom image.\n" \
			" Please put rom code to address 0x%x.\n",
			(uintptr_t) write_buf);

		prints (" Press 'c' to continue, 'q' to abort\n");
		while (1) {
			ch = uart_getc();

			if (ch == 'c')
				break;
			else if (ch == 'q')
				return 1;
		}
	} else {
		// Prepare the pattern for writing
		for (i = 0; i < len; i++)
			*(write_buf + i) = i;
	}

	if (spi_flash_info[ce]->write(spi_flash_info[ce], wr_type, addr,
				      len, write_buf))
		return 0;

	if (burn) {
		memset(read_buf, 0, len);

		if (burn == 0xff)
			burn = 0;

		if(spi_flash_info[ce]->read(spi_flash_info[ce], 0, addr,
					    len, read_buf))
			return 0;

		if (!FTSPI020_compare(write_buf, read_buf, len)) {
			prints("Compare OK!!\n");
		}
	}

	return 0;
}

/* rd <ce> <type> <offset> <DataCnt> [show] */
int FTSPI020_read(int argc, char * const argv[])
{
	int ce, addr, len;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int *p;

	if (argc < 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	} 

	addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	memset(read_buf, 0, len);
	if (spi_flash_info[ce]->read(spi_flash_info[ce], strtol(argv[2], 0, 0), addr, len, read_buf)) {
		 FTSPI020_reset_hw(spi_flash_info[ce]->ctrl_id);
		return 0;
	}

	if (argc == 6) {
		int	i;

		if (addr & 0xf) {
			prints("0x%08x: ", (int)addr & ~0xf);

			for (i = 0; i < (addr & 0xf); i++) {
				prints("   ");
			}
		}

		/* Write out page data */
		for (i = 0; i < len; i++) {
			if (((addr + i) & 0xf) == 0) {
				if (i != 0) {
					prints("\n");
				}

				prints("0x%08x: ", (int) (addr + i));
			}

			prints("%02x ",  read_buf[i]);
		}


	} else {
		p = (int *) read_buf;
		prints("Data content: %x %x %x %x\n", p[0], p[1], p[2], p[3]);
	}

	return 0;
}

/* wr_sp <ce> <row> <col> <bytecnt> */
int FTSPI020_write_spare(int argc, char * const argv[])
{
	int ce, i, row_addr, col_addr, len;
	char *write_buf, *read_buf;

	if (argc != 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	row_addr = strtol(argv[2], 0, 0);
	col_addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	for (i = 0; i < len; i++)
		*(write_buf + i) = i;

	if (spi_flash_info[ce]->write_spare)
		spi_flash_info[ce]->write_spare(spi_flash_info[ce],
						row_addr, col_addr,
						len, write_buf);

	memset(read_buf, 0, len);
	if (spi_flash_info[ce]->read_spare)
		spi_flash_info[ce]->read_spare(spi_flash_info[ce],
					       row_addr, col_addr,
					       len, read_buf);

	if (!FTSPI020_compare(write_buf, read_buf, len)) {
		prints("Compare OK!!\n");
	}

	return 0;
}

/* rd_sp <ce> <row> <col> */
int FTSPI020_read_spare(int argc, char * const argv[])
{
	int ce, i, row_addr, col_addr, len;
	char *read_buf = (char *) g_spi020_rd_buf_addr;

	if (argc != 4) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	row_addr = strtol(argv[2], 0, 0);
	col_addr = strtol(argv[3], 0, 0);
	len = 2112 - col_addr;

	memset(read_buf, 0, len);

	if (spi_flash_info[ce]->read_spare)
		spi_flash_info[ce]->read_spare(spi_flash_info[ce],
					       row_addr, col_addr,
					       len, read_buf);

	if (col_addr & 0xf) {
		prints("%d: ", (int)col_addr & ~0xf);

		for (i = 0; i < (col_addr & 0xf); i++) {
			prints("   ");
		}
	}

	/* Write out page data */
	for (i = 0; i < len; i++) {
		if (((col_addr + i) & 0xf) == 0) {
			if (i != 0) {
				prints("\n");
			}

			prints("%d: ", (int) (col_addr + i));
		}

		prints("%02x ",  read_buf[i]);
	}

	return 0;
}

/* cp <ce> <src_row> <dst_row> <row_count> */
int FTSPI020_copy(int argc, char * const argv[])
{
	int ce, i, src_row, dst_row, row_count, page_size;
	char *write_buf, *read_buf;

	if (argc != 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	if (!spi_flash_info[ce]->copy_data) {
		prints(" CE %d has no copy data function\n", ce);
		return 1;
	}

	src_row = strtol(argv[2], 0, 0);
	dst_row = strtol(argv[3], 0, 0);
	row_count = strtol(argv[4], 0, 0);;

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;
	page_size = spi_flash_info[ce]->page_size;

	for (i=0; i < row_count; i++, src_row++, dst_row++) {
		spi_flash_info[ce]->copy_data(spi_flash_info[ce],
					      src_row, dst_row, 0, 0, 0);

		memset(write_buf, 0, page_size);
		memset(read_buf, 0, page_size);

		spi_flash_info[ce]->read(spi_flash_info[ce], 0,
					 (src_row * page_size), page_size,
					 write_buf);
		spi_flash_info[ce]->read(spi_flash_info[ce], 0,
					 (dst_row * page_size), page_size,
					 read_buf);

		if (!FTSPI020_compare(write_buf, read_buf, page_size)) {
			prints("Compare OK!!\n");
		}
	}

	return 0;
}

static int FTSPI020_check_erase(int ce, int offset, int size)
{
	char *rd_buf;
	int i, len, ret;

	rd_buf = (char *) g_spi020_rd_buf_addr;

	prints("Verifiying erased data ... \n");
	while(size > 0) {

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
			if (spi_flash_info[ce]->is_bad_block(
					spi_flash_info[ce], offset)) {
				prints ("Skip check bad block 0x%x.\n",
						offset);
				offset += len;
				continue;
			}
		}

		len = min_t(size, spi_flash_info[ce]->erase_block_size); // min_t(size, g_spi020_rd_buf_length);

		memset(rd_buf, 0, len);

		ret = spi_flash_info[ce]->read(spi_flash_info[ce], 0, offset,
					       len, rd_buf);
		size -= len;
		offset += len;

		if (ret == 3)
			continue;

		for (i=0; i < len; i++) {
			if (rd_buf[i] != 0xFF) {
				prints("Erase: compare data failed at " \
				       "0x%08x = %x %x %x %x %x %x\n",
					   (uintptr_t)(&rd_buf[i]), rd_buf[i], rd_buf[i+1],
					   rd_buf[i+2], rd_buf[i+3], rd_buf[i+4], rd_buf[i+5]);
				return 1;
			}
		}
	}

	return 0;
}

/* er <ce> <type> <offset> <DataCnt> [er with rd] */
int FTSPI020_erase(int argc, char * const argv[])
{
	int ce, type, addr, size;

	if (argc < 5) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        } 

	type = strtol(argv[2], 0, 0);
	addr = strtol(argv[3], 0, 0);
	size = strtol(argv[4], 0, 0);

	/* Check if bad block for SPI NAND flash */
	if (spi_flash_info[ce]->is_bad_block) {
		if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce],
						     addr)) {
			prints ("You are about to erase bad block 0x%x.\n",
				addr);
			return 0;
		}
	}

	if (spi_flash_info[ce]->erase(spi_flash_info[ce], type,
				      addr, size)) {
		prints("Erase Sector failed.\n");
		return 0;
	}

	if (argc == 6)
		FTSPI020_check_erase(ce, addr, size);

	return 0;
}

/* er_all <ce list> [er with rd] */
int FTSPI020_erase_all(int argc, char * const argv[])
{
	int ce, ce_list;

	if (argc < 2) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

	ce_list = strtol(argv[1], 0, 0);

	for (ce = 0; ce < FTSPI020_TOTAL_CE; ce++) {

		if (!(ce_list & (1 << ce)) || !spi_flash_info[ce])
			continue;

		// Perform the erase operation.
		spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
		
		if (argc == 3)
			FTSPI020_check_erase(ce, 0,
					     spi_flash_info[ce]->size);
	}

	return 0;
}

/* rst <ce> */
int FTSPI020_reset(int argc, char * const argv[])
{
	int ce;
	struct ftspi020_cmd spi_cmd = {0};

	if (argc != 2) {
		prints("%d arguments for status isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	spi_cmd.start_ce = ce / FTSPI020_MAX_CE_PER_CTRL;
	spi_cmd.ins_code = CMD_RESET;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_write;

	FTSPI020_issue_cmd(spi_flash_info[ce]->ctrl_id, &spi_cmd);

	if (FTSPI020_wait_cmd_complete(spi_flash_info[ce]->ctrl_id, 10)){
		FTSPI020_reset_hw(spi_flash_info[ce]->ctrl_id);
	}

	return 0;
}

/* sts <ce> [opcode] */
int FTSPI020_report_status(int argc, char * const argv[])
{
	int ce, opcode;
	uint8_t rd_sts_cmd[1];
	uint8_t *sts = (uint8_t *) FTSPI020_DMA_ADDR;

	if (argc < 2) {
		prints("%d arguments for status isn't allowed\n", argc);
		return 1;
	}
        
	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}
	
	if (argc < 3) {
		spi_flash_info[ce]->report_status(spi_flash_info[ce]);
	} else {
		opcode = strtol(argv[2], 0, 16) & 0xFF;
		if (!spi_flash_info[ce]->isnand) {

			rd_sts_cmd[0] = opcode;
			if (spi_flash_cmd_read(spi_flash_info[ce], rd_sts_cmd, sts, 1))
				return 1;
			prints("NOR-Read Status(%xh): 0x%x\n", opcode, *sts);
		} else {
			rd_sts_cmd[0] = SPINAND_GET_FEATURE;
			rd_sts_cmd[1] = opcode;
			if (spi_flash_cmd_read(spi_flash_info[ce], rd_sts_cmd, sts, 1))
				return 1;
			prints("NAND-Read Status(%xh): 0x%x\n", opcode, *sts);

		}
	}

	return 0;
}

#if 0
int FTSPI020_burnin_copy(int ce)
{
	int wr_byte, page_size, max_size, i, j, times, err;
	int offset, dst_offset, src_row, dst_row;
	int modify_row, modify_col;
	size_t modify_len;
	char *write_buf, *src_buf, *dst_buf, *modify_buf;

	write_buf = (char *) g_spi020_wr_buf_addr;
	src_buf = (char *) g_spi020_rd_buf_addr;
	dst_buf = (char *) (g_spi020_rd_buf_addr + 262144);

	page_size = spi_flash_info[ce]->page_size;
	max_size = spi_flash_info[ce]->size - (page_size << 1);
	offset = times = 0;

	prints("Round: 0\n");
	do {
		wr_byte = (rand() % 0x20000) + page_size;

		/* spi nand flash:
		 * Each 512-bytes sector can only be written once,
		 * so increase the row_addr to next 512 bytes
		 */
		if (offset)
			offset = (offset + 512) & ~511;

		if (offset + (wr_byte << 1) >= max_size) {
			wr_byte = (max_size - offset) >> 1;

			if (wr_byte < (page_size << 2))
				goto erase;
		}

		/* Prepare the pattern for writing & reading */
		for (i = 0; i < wr_byte; i++)
			*(write_buf + i) = i;

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
		check_bad_block:
			if (spi_flash_info[ce]->is_bad_block(
						spi_flash_info[ce], offset)) {
				prints ("Skip program to bad block 0x%x.\n",
					offset);
				/* increment to next block */
				offset += spi_flash_info[ce]->
						erase_block_size;
				offset &= ~(spi_flash_info[ce]->
						erase_block_size-1);

				/* Is it a bad block too */
				goto check_bad_block;
			}
		}

		/* write pattern to source row address */
		if (spi_flash_info[ce]->write(spi_flash_info[ce], 0, offset,
					      wr_byte, write_buf))
			break;

		/* Compare the source data */
		memset(src_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0, offset,

					       wr_byte, src_buf);
		if (err || FTSPI020_compare(write_buf, src_buf, wr_byte))
			return 1;


		src_row = offset / page_size;
		dst_row = j = (offset + wr_byte + page_size) / page_size;

		/* Random data load test */
		modify_row = src_row + (rand() % (j - src_row));
		if (modify_row == src_row)
			modify_row = src_row + 1;

		/* Random load happens at last row of source */
		if (modify_row == (j-1)) {
			i = (offset + wr_byte) % page_size;
		} else {
			i = page_size;
		}

		modify_col = rand() % i;
		if (!modify_col)
			modify_col = 1;

		modify_len = i - modify_col;
		modify_buf = src_buf + page_size - (offset % page_size);
		modify_buf += ((modify_row - src_row - 1) * page_size);
		modify_buf +=  modify_col;

		/* Alter the pattern for random data load */
		for (i = 0; i < modify_len; i++) {
			*(modify_buf + i) = (i % 2 == 0) ? 0xAA : 0x55;
			*(src_buf + (modify_row * page_size) + modify_col) = *(modify_buf + i);
		}

		while (src_row < dst_row) {
			if (src_row == modify_row) {
				if (spi_flash_info[ce]->copy_data(
					spi_flash_info[ce], src_row, j,
					modify_col, modify_buf, modify_len))
					return 1;

			} else {
				if (spi_flash_info[ce]->copy_data(
					spi_flash_info[ce], src_row,
					j, 0, 0, 0))
					return 1;

			}
			src_row++;
			j++;
		}

		/* column address at source row address */
		i = offset % page_size;
		dst_offset = (dst_row * page_size) + i;

		memset(dst_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0,
					       dst_offset, wr_byte, dst_buf);
		if (err || FTSPI020_compare(src_buf, dst_buf, wr_byte))
			return 1;

		offset = dst_offset + wr_byte;
		if (offset >= max_size) {
erase:
			times++;
			prints("Round: %d\n", times);

			err = spi_flash_info[ce]->erase_all(
						   spi_flash_info[ce]);
			if (err || FTSPI020_check_erase(ce, 0,
						   spi_flash_info[ce]->size))
				return 1;

#ifdef FTSPI020_USE_DMA
			g_trans_mode = (g_trans_mode == PIO) ? DMA : PIO;
#else
			g_trans_mode == PIO;
#endif
			offset = 0;

			g_divider += 2;
			if (g_divider > 8)
				g_divider = 2;
			FTSPI020_divider(g_divider);
			prints("Set divider to %d\n", g_divider);

		}
		// Press 'q' to leave the burnin
		if ('q' == ftuart_kbhit())
			break;
	} while (1);

	return 0;
}
#endif

/*
 * Copy block 0 to block 1, block 2 to block 3, ..., etc.
 */
int FTSPI020_burnin_copy(int ce)
{
	int wr_byte, page_size, nr_pages_per_block, i, j, times, err;
	int offset, dst_offset, src_row, dst_row;
	int modify_row, modify_col, page_index, modify_len;
	char *write_buf, *read_buf, *modify_buf;
	int ctrl_id;

	ctrl_id = spi_flash_info[ce]->ctrl_id;

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	page_size = spi_flash_info[ce]->page_size;
	nr_pages_per_block = spi_flash_info[ce]->erase_block_size / page_size;
	offset = times = 0;

	/* Prepare the pattern for writing */
	for (i = 0; i < spi_flash_info[ce]->erase_block_size; i++)
		*(write_buf + i) = i;

	/* Always write one block for source pattern */
	wr_byte = spi_flash_info[ce]->erase_block_size;

	prints("Round: 0\n");
	do {

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
		check_bad_block:
			if (spi_flash_info[ce]->is_bad_block(
					spi_flash_info[ce], offset) ||
				spi_flash_info[ce]->is_bad_block(
					spi_flash_info[ce], (offset + spi_flash_info[ce]->
							erase_block_size))) {
				prints ("Skip program to bad block 0x%x.\n",
					offset);
				/* increment to next 2 blocks */
				offset += (spi_flash_info[ce]->
						erase_block_size * 2);
				offset &= ~(spi_flash_info[ce]->
						erase_block_size-1);

				/* Is it a bad block too */
				goto check_bad_block;
			}
		}

		/* write pattern to source address */
		if (spi_flash_info[ce]->write(spi_flash_info[ce], 0, offset,
					      wr_byte, write_buf))
			break;

		/* Compare the source data */
		memset(read_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0, offset,
					       wr_byte, read_buf);
		if (err || FTSPI020_compare(write_buf, read_buf, wr_byte))
			break;

		/*
		 * Destination row address is next block.
		 * Block 0 to block 1, block 2 to block 3, .., etc.
		 */
		src_row = offset / page_size;
		dst_row = src_row + nr_pages_per_block;

		/* Generate page index for doing Random data load test.
		 */
		page_index = rand() % nr_pages_per_block;

		/*
		 * Random generate the column address that we want to modify.
		 * Assume page size is 2048 bytes, evenly divided into 4
		 * 512-bytes area.
		 */
		modify_row = src_row + page_index;
		modify_col = rand() & 3;
		modify_col <<= 9;

		modify_len = page_size - modify_col;
		/* Alter the pattern for random data load */
		modify_buf = (char *) (write_buf + (page_index * page_size) + modify_col);
		for (i = 0; i < modify_len; i++) {
			*(modify_buf + i) = (i % 2 == 0) ? 0xAB : 0xCD;
		}

		j = dst_row;
		while (src_row < dst_row) {
			if (src_row == modify_row) {
				if (spi_flash_info[ce]->copy_data(
					spi_flash_info[ce], src_row, j,
					modify_col, modify_buf, modify_len))
					break;

			} else {
				if (spi_flash_info[ce]->copy_data(
					spi_flash_info[ce], src_row,
					j, 0, 0, 0))
					break;

			}
			src_row++;
			j++;
		}

		/*
		 * Read back the data and compare.
		 */
		memset(read_buf, 0, wr_byte);
		dst_offset = dst_row * page_size;
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0,
					       dst_offset, wr_byte, read_buf);
		if (!err) {
			if (FTSPI020_compare(write_buf, read_buf, wr_byte)) {
				break;
			}
			prints("Compare OK!!\n");
		} else {
			if (err == 1)
				break;
			else if (err == 3)
				goto next_write;
		}

next_write:
		/*
		 * Because we use 2 blocks for one test, Block 0 and 1,
		 * block 2 and 3. So increment two blocks size for next
		 * test iteration.
		 */
		offset = offset + (wr_byte * 2);
		if ((offset >= spi_flash_info[ce]->size) ||
			((offset + (wr_byte * 2))  >= spi_flash_info[ce]->size)) {
			times++;
			prints("Round: %d\n", times);

			err = spi_flash_info[ce]->erase_all(
						   spi_flash_info[ce]);
			if (err || FTSPI020_check_erase(ce, 0,
						   spi_flash_info[ce]->size))
				break;

#ifdef FTSPI020_USE_DMA
			g_trans_mode[ctrl_id] = (g_trans_mode[ctrl_id] == PIO) ? DMA : PIO;

			FTSPI020_dma_enable(ctrl_id, (g_trans_mode[ctrl_id] == DMA));
#else
			g_trans_mode[ctrl_id] == PIO;
#endif
			offset = 0;

			g_divider += 4;
			if (g_divider > 8)
				g_divider = 4;
			FTSPI020_divider(ctrl_id, g_divider);
			prints("Set divider to %d\n", g_divider);

		}
		// Press 'q' to leave the burnin
		if ('q' == uart_kbhit())
			break;
	} while (1);

	prints(" Run  %d loops.\n", times);

	return 0;
}

/* burnin <ce> <offset> [1:copy] */
int FTSPI020_burnin(int argc, char * const argv[])
{
	int wr_byte, wr_type, rd_type, er_type;
	int start_pos;
	int i, err, times = 0;
	int wr_tp_cnt[3] = { 0 };
	int rd_tp_cnt[7] = { 0 };
	int er_tp_cnt[4] = { 0 };
	char *write_buf, *read_buf;
	char mode;
	int ce, ctrl_id, offset;

	if (argc < 3) {
		prints("%d arguments for burnin isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	ctrl_id = spi_flash_info[ce]->ctrl_id;

	if (argc > 3) {
		FTSPI020_burnin_copy(ce);
		return 0;
	}

	start_pos = strtol(argv[2], 0, 0);
	if (start_pos > spi_flash_info[ce]->size) {
		prints(" Start offset larger than flash size %lu\n",
			spi_flash_info[ce]->size);
		return 1;
	}

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;
	er_type = 0;
	mode = mode3;
	g_divider = 4;
	offset = start_pos;

	prints("Round: 0\n");
	do {
		wr_byte = (rand() % (g_spi020_wr_buf_length >> 2)) + 1;
		wr_byte = (rand() % 0x1000) + 1;
		//wr_byte = spi_flash_info[ce]->page_size;

		if (spi_flash_info[ce]->isnand) {

			wr_byte &= ~(spi_flash_info[ce]->page_size - 1);
			if (wr_byte == 0)
				wr_byte = spi_flash_info[ce]->page_size;

			/* spi nand flash:
			 * Each 512-bytes sector can only be written once,
			 * so increase the row_addr to next 512 bytes.
			 * Some NAND flash has NOP problem, increase to next
			 * page.
			 */
			if (offset && (offset & ~(spi_flash_info[ce]->page_size - 1)))
				offset = (offset + spi_flash_info[ce]->page_size) &
						~(spi_flash_info[ce]->page_size - 1);

		}

		if (spi_flash_info[ce]->code == 0x8E25) {
			wr_byte &= ~1;

		}

		if (offset + wr_byte > spi_flash_info[ce]->size) {
			wr_byte = spi_flash_info[ce]->size - offset;
			if (wr_byte < 512)
				goto erase;
		}

		wr_type = rand() % spi_flash_info[ce]->max_wr_type;
		rd_type = rand() % spi_flash_info[ce]->max_rd_type;

		// Prepare the pattern for writing & reading
		for (i = 0; i < wr_byte; i++)
			*(write_buf + i) = i;

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
		check_bad_block:
			if (spi_flash_info[ce]->is_bad_block(
					       spi_flash_info[ce], offset)) {
				prints ("Skip program to bad block 0x%x.\n",
					offset);
				/* increment to next block */
				offset += spi_flash_info[ce]->
						erase_block_size;
				offset &= ~(spi_flash_info[ce]->
						erase_block_size-1);

				/* Is it a bad block too */
				goto check_bad_block;
			}
		}

		err = spi_flash_info[ce]->write(spi_flash_info[ce], wr_type,
						offset, wr_byte, write_buf);
		if (err) {
			if (err == 3) //bad block
				goto next_write;
			else
				break;
		}

		wr_tp_cnt[wr_type]++;

//read_again:
		memset(read_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], rd_type,
					       offset, wr_byte, read_buf);
		if (!err) {
			if (FTSPI020_compare(write_buf, read_buf, wr_byte)) {
#if 0
				if (g_damr_read) {
					prints("DAMR read again!!\n");
					goto read_again;
				}
#endif
				break;
			}
			prints("Compare OK!!\n");
			rd_tp_cnt[rd_type]++;
		} else {
			if (err == 1)
				break;
			else if (err == 3)
				goto next_write;
			else if (err == 2)
				prints(" %s not support by HW.\n",
				       spi_flash_info[ce]->get_string(
						spi_flash_info[ce],
						READ, rd_type));
		}

next_write:
		offset = offset + wr_byte;
		if (offset >= spi_flash_info[ce]->size) {
erase:
			times++;
			prints("Round: %d, ce %d offset %d size %lu\n",
				times, ce, offset, spi_flash_info[ce]->size);

			if (er_type == spi_flash_info[ce]->max_er_type) {
				if (start_pos == 0)
					err = spi_flash_info[ce]->
					      erase_all(spi_flash_info[ce]);
				else
					err = spi_flash_info[ce]->
					      erase(spi_flash_info[ce], 0,
						    start_pos,
						    (spi_flash_info[ce]->size

						    - start_pos));
			 } else 
				err = spi_flash_info[ce]->
				      erase(spi_flash_info[ce], er_type,
					    start_pos,
					    (spi_flash_info[ce]->size -
					     start_pos));

			er_tp_cnt[er_type]++;
			er_type++;

			if (err && (err != 3))
				break;

			err = FTSPI020_check_erase(ce, start_pos,
						   spi_flash_info[ce]->size -
						   start_pos);

			if (err)
				break;

#ifdef FTSPI020_USE_DMA
			g_trans_mode[ctrl_id] = (g_trans_mode[ctrl_id] == PIO) ? DMA : PIO;

			FTSPI020_dma_enable(ctrl_id, (g_trans_mode[ctrl_id] == DMA));
#else
			g_trans_mode[ctrl_id] == PIO;
#endif
			FTSPI020_operate_mode(ctrl_id, (mode == mode3) ? mode0 : mode3);

			if (er_type > spi_flash_info[ce]->max_er_type) {
				er_type = 0;
			}

			offset = start_pos;
#if 0
			g_divider = 8;
#else
			/* spi_clk divider for DTR mode only 4/8 are allowed */
			g_divider += 4;
			if (g_divider > 8)
				g_divider = 4;
#endif
			FTSPI020_divider(ctrl_id, g_divider);
			prints("Set divider to %d\n", g_divider);

		}
		// Press 'q' to leave the burnin
		if ('q' == uart_kbhit())
			break;
	} while (1);

	prints(" Final position at  %d.\n", offset);
	for (i = 0; i < spi_flash_info[ce]->max_rd_type; i++) {
		prints(" %s: %d times.\n", spi_flash_info[ce]->
					   get_string(spi_flash_info[ce],
						      READ, i),
					   rd_tp_cnt[i]);
	}

	for (i = 0; i < spi_flash_info[ce]->max_wr_type; i++) {
		prints(" %s: %d times.\n", spi_flash_info[ce]->
					   get_string(spi_flash_info[ce],
						      WRITE, i),
					   wr_tp_cnt[i]);
	}

	for (i = 0; i <= spi_flash_info[ce]->max_er_type; i++) {
		if (i == spi_flash_info[ce]->max_er_type)
			prints(" Erase Chip: %d times.\n", er_tp_cnt[i]);
		else
			prints(" %s: %d times.\n", spi_flash_info[ce]->
						   get_string(spi_flash_info[ce],
							      ERASE, i),
						   er_tp_cnt[i]);
	}

	return 0;
}

/* perf <ce> <len> [type] */
int FTSPI020_report_performance(int argc, char * const argv[])
{
	int bytes, i;
	char *read_buf/*, *write_buf*/;
	int ce, type;
	uint64_t t0, t1;

	if (argc < 4) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }


	bytes = strtol(argv[2], 0, 0);
#if 0
	// Prepare the pattern for writing & reading
	write_buf = (char *) g_spi020_wr_buf_addr;
	for (i = 0; i < bytes; i++)
		*(write_buf + i) = i;
	// Because we only want to measure performance, don't care for error.
	spi_flash_info[ce]->write(spi_flash_info[ce], 0, 0, bytes, write_buf);
#endif
	read_buf = (char *) g_spi020_rd_buf_addr;

	g_divider = 2;
	FTSPI020_divider(spi_flash_info[ce]->ctrl_id, g_divider);

	if (argc == 5) {
		type = strtol(argv[3], 0, 0);
		t0 = ftspi020_get_time();
		spi_flash_info[ce]->read(spi_flash_info[ce], type, 0,
					 bytes, read_buf);
		t1 = ftspi020_get_time();
		prints("Rd:%lld ms.\n", (t1 - t0) / COUNTS_PER_MSECOND);

	} else {
		for (i = 0; i < spi_flash_info[ce]->max_rd_type ; i++) {

			t0 = ftspi020_get_time();
			spi_flash_info[ce]->read(spi_flash_info[ce], i, 0,
						 bytes, read_buf);
			t1 = ftspi020_get_time();

			prints("Rd:%lld ms.\n", (t1 - t0) / COUNTS_PER_MSECOND);

	#if 0
			//Error message print inside the function
			FTSPI020_compare(write_buf, read_buf, bytes);
	#endif
		}
	}

	return 0;
}

/* tr <ctrl> <pio|dma> */
int FTSPI020_transfer_type(int argc, char * const argv[])
{
	int ctrl_id;

	if (argc != 3)
		return 1;

	ctrl_id = strtol(argv[1], 0, 0);
	if (ctrl_id > FTSPI020_MAXIMUM_CTRL) {
		prints(" Ctrl id (%d) larger than maximum value %d.\n", ctrl_id, FTSPI020_MAXIMUM_CTRL);
		return 0;
	}
	
	if (strcmp(argv[2], "pio") == 0) {
		FTSPI020_dma_enable(ctrl_id, 0);
		g_trans_mode[ctrl_id] = PIO;
	} else {
		FTSPI020_dma_enable(ctrl_id, 1);
		g_trans_mode[ctrl_id] = DMA;
	}
	return 0;
}

/* intr <ctrl> <1|0> */
int FTSPI020_use_interrput(int argc, char * const argv[])
{
	int ctrl_id, i;

	if (argc != 3)
		return 1;

	ctrl_id = strtol(argv[1], 0, 0);
	if (ctrl_id > FTSPI020_MAXIMUM_CTRL) {
		prints(" Ctrl id (%d) larger than maximum value %d.\n",
				ctrl_id, FTSPI020_MAXIMUM_CTRL);
		return 0;
	}

#ifdef FTSPI020_USE_INTERRUPT
	g_cmd_intr_enable[ctrl_id] = strtol(argv[2], 0, 0);
#else
	g_cmd_intr_enable[ctrl_id] = 0;
#endif
	if (g_cmd_intr_enable[ctrl_id]) {
		FTSPI020_cmd_complete_intr_enable(ctrl_id, 1);
	} else {
		FTSPI020_cmd_complete_intr_enable(ctrl_id, 0);
	}

	prints("Ctld id %d %s interrupt\n", ctrl_id,
			g_cmd_intr_enable[ctrl_id] ? "Use" : "Not use");

	ftspi020_disable_interrupts();
	for (i = 0; i < FTSPI020_MAXIMUM_CTRL; i++) {
		if (g_cmd_intr_enable[i])
			ftspi020_enable_interrupts(); /* Clear CPU I bit */
	}

	return 0;
}

/* qpi <ce> <0|2|4> */
int FTSPI020_enter_qpi(int argc, char * const argv[])
{
	int ce, qpi;
	int (*enter_qpi)(struct spi_flash *, int);

	if (argc != 3)
		return 1;

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 0;
	}

	if (spi_flash_info[ce]->code == 0x18BA20)
		enter_qpi = &micron_qpi_mode;
	else
		enter_qpi = &nor_qpi_mode;

	qpi = strtol(argv[2], 0, 0);
	if (enter_qpi(spi_flash_info[ce], qpi))
		prints("%s QPI mode Failed\n",  qpi ? "Enter" : "Exit");
	else
		prints("%s QPI mode Success\n",  qpi ? "Enter" : "Exit");

	return 0;
}

/* abort <ctrl> <0|1> */
int FTSPI020_abort_test(int argc, char * const argv[])
{
	int ctrl_id;

	if (argc != 3)
		return 1;

	ctrl_id = strtol(argv[1], 0, 0);
	if (ctrl_id > FTSPI020_MAXIMUM_CTRL) {
		prints(" Ctrl id (%d) larger than maximum value %d.\n",
				ctrl_id, FTSPI020_MAXIMUM_CTRL);
		return 0;
	}

	g_abort_test[ctrl_id] = strtol(argv[2], 0, 0);

	prints(" Turn %s abort test\n", g_abort_test[ctrl_id] ? "on" : "off");

	if (g_abort_test[ctrl_id]) {
		g_trans_mode[ctrl_id] = PIO;
		FTSPI020_dma_enable(ctrl_id, 0);
		prints(" Force data transfer to PIO mode\n");
	}

	return 0;
}

/* bbt [ce] <scan>*/
int FTSPI020_scan_bad_blocks(int argc, char * const argv[])
{
	int idx, ce, i;
	uint8_t blklist;

	if (argc < 2)
		return 1;

	ce = strtol(argv[1], 0, 0);
	if (!spi_flash_info[ce] ||
		!spi_flash_info[ce]->scan_bad_blocks)
		return 1;

	/* Build bad block table */
	if (argc > 2)
		spi_flash_info[ce]->scan_bad_blocks(spi_flash_info[ce]);

	prints("Bad Block Table:\n");
	for (idx = 0; idx < BBT_MAX_ENT; idx++) {

		blklist = spi_flash_info[ce]->bad_block_table[idx];
		if (blklist) {
			prints("Blk index %d val 0x%x: ", idx, blklist);
			for (i = 0; i < 8; i++) {
				if (blklist & (1 << i)) {
					prints("%d ", ((idx * 8) + i));
				}
			}
			prints("\n");
		}

	}

	return 0;
}

extern int g_dll_delay;
/* xip <ctrl> <0|1> [base addr] */
int FTSPI020_xip_set(int argc, char * const argv[])
{
	int ctrl_id;

	if (argc < 3)
		return 1;

	ctrl_id = strtol(argv[1], 0, 0);
	if (ctrl_id > FTSPI020_MAXIMUM_CTRL) {
		prints(" Ctrl id (%d) larger than maximum value %d.\n",
				ctrl_id, FTSPI020_MAXIMUM_CTRL);
		return 0;
	}

	g_damr_read[ctrl_id] = strtol(argv[2], 0, 0);

	if (g_damr_read[ctrl_id]) {
#if 0 //Fix by HW
		if (argc != 3) {
			prints(" Please specify DAMR port base address\n");
			return 1;
		}

		g_damr_port_base[ctrl_id] = strtol(argv[2], 0, 0);
#endif
	}

	prints(" %s DAMR, base addr 0x%08x\n", g_damr_read[ctrl_id] ? "En":"Dis",
	       g_damr_port_base[ctrl_id]);

	return 0;
}

/* tst <ce> */
int FTSPI020_rdwr_test(int argc, char * const argv[])
{
	char *write_buf, *read_buf;
	int ce, i, j, offset, len, off;
#if 0
	int rd_cnt;
#endif

	if (argc != 2)
		return 1;

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_TOTAL_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 0;
	}

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	for (i = 0; i < 0x10000; i++)
		*(write_buf + i) = i;

	off = 0;

tst_4b:
	for (i = 0; i < spi_flash_info[ce]->max_rd_type ; i++) {
		char wr_type, er_type;

		offset = off;

		er_type = i % spi_flash_info[ce]->max_er_type;
		if (spi_flash_info[ce]->erase(spi_flash_info[ce], er_type,
			    offset, spi_flash_info[ce]->erase_block_size))
			break;

		if (FTSPI020_check_erase(ce, offset,
				         spi_flash_info[ce]->
					 erase_block_size))
			break;

		if (spi_flash_info[ce]->isnand)
			len = 0x10000;
		else
			len = 200;

		for (j = 0; j < 3; j++) {

			wr_type = i % spi_flash_info[ce]->max_wr_type;
			if (spi_flash_info[ce]->write(spi_flash_info[ce], wr_type,
					      offset, len, write_buf))
				break;
#if 0
			rd_cnt = 0;
read_again:
#endif
			memset(read_buf, 0, len);
			if (spi_flash_info[ce]->read(spi_flash_info[ce], i, offset,
					 	 len, read_buf))
				break;

			if (FTSPI020_compare(write_buf, read_buf, len) == 0) {
				prints("Compare OK!!\n");
			} else {
#if 0
				if (rd_cnt < 5) {
					rd_cnt++;
					goto read_again;
				}
#endif
			}

			if (spi_flash_info[ce]->isnand)
				break;

			offset += len;
			len ++;
		}
	}

	if ((spi_flash_info[ce]->size > 0x1000000) && (offset < 0x1000000)) {
		off = 0x1000000;
		goto tst_4b;
	}

	return 0;
}

/* ls */
int FTSPI020_list_ce(int argc, char * const argv[])
{
	int i;
	
	for (i = 0 ; i < FTSPI020_TOTAL_CE; i++) {
		prints(" CE %d:\n", i);
		if (!spi_flash_info[i])
			prints(" - No Flash \n");
		else {
			prints(" - Name: %s.\n", spi_flash_info[i]->name);
			prints(" - Ctrl Id: %lu (%p)\n", spi_flash_info[i]->ctrl_id,
				(void *) g_reg_base[spi_flash_info[i]->ctrl_id]);
			prints(" - Code: 0x%04lx.\n",
				   spi_flash_info[i]->code);
			prints(" - Page Size: %d bytes.\n",
				   spi_flash_info[i]->page_size);
			prints(" - Number of Page: %lu.\n",
				   spi_flash_info[i]->nr_pages);
			prints(" - Sector Size: %d bytes.\n",
				   spi_flash_info[i]->erase_sector_size);
			prints(" - Block Size: %lu bytes.\n",
				   spi_flash_info[i]->erase_block_size);
			prints(" - Chip Size: %lu bytes.\n",
				   spi_flash_info[i]->size);
		}
	}

	return 0;
}

int do_help(int argc, char * const argv[], cmd_t * tbl)
{
	cmd_t *cmdp = &tbl[0];

	if (argc == 1) {	/* display all commands */
		while (cmdp->name != 0) {
			prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else if (argc == 2) {	/*Display argv[0] command */
		while (cmdp->name != 0) {
			if (strcmp(argv[0], cmdp->name) == 0)
				prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else {
		return 1;
	}

	return 0;
}

static int   argc;
static char *argv[16];
int cmd_exec(char *line, cmd_t * tbl)
{
	int i;
	char *tok;
	cmd_t *cmd;

	argc = 0;

	tok  = strtok (line, " \r\n\t");
	while(tok) {
		argv[argc++] = tok;
		tok = strtok (NULL, " \r\n\t");
	}

	if (argc == 0)
		return 0;

	if (!strcmp("quit",argv[0]))
		return 1;

	for (i = 0; ; ++i) {
		cmd = &tbl[i];
		if ((cmd->name == NULL) || !strcmp("help", argv[0]) ) {
			do_help(1, 0, tbl);	/* list all support commands */
			return 0;
		} else if (!strcmp(cmd->name, argv[0]) && cmd->func) {
			if (cmd->func(argc, argv))
				do_help (2, argv, tbl);

			break;
		}
	}

	return 0;
}

int FTSPI020_main(int argc, char * const argv[])
{
	struct spi_flash *flash;
	uint32_t i, j;
	char cmdstr[CMDLEN];

	ftspi020_init_platform();

#if defined ROM_SPI && defined COPY_DATA_ONLY
	prints("\n Program runs at SPI Flash.\n");
	prints(" FTSPI020 can not be used as command mode.\n");
	return 0;
#endif
	prints("\nFTSPI020 FPGA Non-OS verification code %s\n",
		FTSPI020_FW_VERSION);

	for (i = 0; i < FTSPI020_MAXIMUM_CTRL; i++) {
		if (FTSPI020_init(i)) {
			prints(" Init ctrl id %lu failed\n", i);
			return 0;
		}
	}

	ftspi020_disable_interrupts();
	for (i = 0; i < FTSPI020_MAXIMUM_CTRL; i++) {
		if (g_cmd_intr_enable[i])
			ftspi020_enable_interrupts(); /* Clear CPU I bit */
	}
	/* Support Maximum 4 CEs per controller */
	for (i = 0; i < FTSPI020_MAXIMUM_CTRL; i++) {
		for (j = 0 ; j < FTSPI020_MAX_CE_PER_CTRL; j++) {

			prints(" \nScanning CE %lu @ ctrl id %lu ... ", j, i);

			flash = spi_flash_info[(i * FTSPI020_MAX_CE_PER_CTRL) + j];

			if (!flash) {
				flash = malloc(sizeof(struct spi_flash));
				if (!flash) {
					prints("Failed to allocate memory\n");
					return 0;
				}
			}

			if (FTSPI020_probe(flash, i, j)) {
				free(flash);
				continue;
			}

			spi_flash_info[(i * FTSPI020_MAX_CE_PER_CTRL) + j] = flash;
		}
	}

	while (1) {

		prints("\nftspi020:> ");

		scan_string(cmdstr);

		cmd_exec(cmdstr, ftspi020_cmd_tbl);
	}

	ftspi020_cleanup_platform();
}
