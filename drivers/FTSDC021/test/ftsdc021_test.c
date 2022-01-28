/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_test.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2020/1/10	Sanjin Liu	 FTSDC021 test code	
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021_test.h"

static uint32_t g_error_cnt;
static uint32_t packed_test_en;
static uint32_t *packed_hdr;
static uint8_t buffer_filled;
static uint8_t *check_buf;
uint8_t *write_buf;
uint8_t *read_buf;

static Adma2DescTable ADMA2_DESC_TABLE[ADMA2_NUM_OF_LINES];

/* Helper functions */
uint8_t SDC_SD_menu(void);
uint8_t SDC_SDIO_menu(void);

uint32_t tuning_pattern[] = {
	0xFF0FFF00, 0xFFCCC3CC, 0xC33CCCFF, 0xFEFFFEEF,
	0xFFDFFFDD, 0xFFFBFFFB, 0xBFFF7FFF, 0x77F7BDEF,
	0xFFF0FFF0, 0x0FFCCC3C, 0xCC33CCCF, 0xFFEFFFEE,
	0xFFFDFFFD, 0xDFFFBFFF, 0xBBFFF7FF, 0xF77F7BDE
};

static uint8_t *card_state_table[] = {
	"Card Idle State",
	"Card Ready State",
	"Card Identify State",
	"Card Stanby State",
	"Card Transfer State",
	"Card Send Data State",
	"Card Receive Data State",
	"Card Programming State",
	"Card Disconnect State",
	"Undefined State"
};

static uint8_t *transfer_type_table[] = {
	"ADMA",
	"SDMA",
	"PIO",
	"External DMA",
	"Undefined Transfer Type"
};

static uint8_t *transfer_speed_table[] = {
	"Normal Speed / SDR12",
	"High Speed / SDR25",
	"SDR50-100MHz",
	"SDR104-208MHz",
	"DDR50",
	"Undefined Speed"
};

static uint8_t *abort_type_table[] = {
	"Asynchronous abort",
	"Synchronous abort",
	"Undefined abort type"
};

uint8_t *SDC_ShowCardState(Card_State state)
{
	if (state > CUR_STATE_RSV)
		state = CUR_STATE_RSV;

	return card_state_table[state];
}

uint8_t *SDC_ShowTransferType(Transfer_Type tType)
{
	if (tType > TRANS_UNKNOWN)
		tType = TRANS_UNKNOWN;

	return transfer_type_table[tType];
}

uint8_t *SDC_ShowTransferSpeed(Bus_Speed speed)
{
	if (speed > SPEED_RSRV)
		speed = SPEED_RSRV;

	return transfer_speed_table[speed];
}

uint8_t *SDC_ShowAbortType(Abort_Type type)
{
	if (type > ABORT_UNDEFINED)
		type = ABORT_UNDEFINED;

	return abort_type_table[type];
}

/* adma [blkCnt] */
int32_t SDC_ShowDscpTbl(int32_t argc, char *const argv[])
{
	Adma2DescTable *ptr = (Adma2DescTable *)ADMA2_DESC_TABLE;
	Adma2DescTable *tmp;
	uint8_t act;
	uint32_t i = 0;

	if (argc == 2)
		ftsdc021_fill_adma_desc_table(atoi(argv[1]) * 512,
					      (uint32_t *)((uintptr_t)write_buf),
					      ADMA2_DESC_TABLE);

	printf("ADMA2 Descriptor Table: \r\n");
	do {

		tmp = ptr + i;
		printf("(%d) Table address (0x%08p): ", i, tmp);

		act = tmp->attr & 0x30;
		if (act == ADMA2_TRAN) {
			printf("Data 0x%08x, Length %d, attr 0x%02x.\r\n",
				    tmp->addr, tmp->lgth, tmp->attr);
			i++;
		} else if (act == ADMA2_LINK) {
			printf("Next desc 0x%08x, Link, attr 0x%02x.\r\n",
				    tmp->addr, tmp->attr);

			while (tmp->addr != (uint32_t)((uintptr_t)&ptr[i])) {
				i++;
			}
		} else {
			printf("Do not execute this line, go to next line,"
				    " attr 0x%02x.\r\n", tmp->attr);
			i++;
		}
	} while (!(tmp->attr & ADMA2_ENTRY_END));

	return 0;
}

/* dump ftsdc021 registers */
int32_t SDC_dump_registers(int32_t argc, char *const argv[])
{
	ftsdc021_dump_regs();
	return 0;
}

/*
 * burnIn <fix|auto>. fix means burnIn based on current settings.
 * auto means Speed, Bus width, Transfer mode, ACMD12, .., etc. 
 * 
 * burnIn <fix|auto> [mins] [tr] [spd] [blk cnt type] [ran type]
 */
int32_t SDC_burnin_test(int32_t argc, char *const argv[])
{
	uint32_t rc = 0, mins;
	uint8_t tr, spd, cnt_tp, ran_cnt_tp;

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_MMC) ||
			(cardInfo.CardType == MEMORY_CARD_TYPE_SD) ||
			(cardInfo.CardType == MEMORY_eMMC)) {

		if (argc < 2)
			return 1;

		g_error_cnt = 0;

		if (strcmp(argv[1], "auto") == 0) {
	
			if (argc > 2)
				mins = atoi(argv[2]) & 0xFFFF;
			else
				mins = 2;

			/* ADMA, SDMA, PIO and External DMA list */
			if (argc > 3)
				tr = atoi(argv[3]) & 0xF;
			else
				tr = 0xF;

			/* SDR12, SDR25, SDR50, SDR104 list */
			if (argc > 4)
				spd = atoi(argv[4]) & 0x1F;
			else
				spd = 0x3;

			/* Block count type list 0: random, the others: fix as
			 * number user assigns*/
			if(argc > 5)
				cnt_tp = (atoi(argv[5]) & 0xFF);
			else
				cnt_tp = 0;

			if(argc > 6)
				ran_cnt_tp = (atoi(argv[6]) & 0xFF);
			else
				ran_cnt_tp = 0;

			SDC_readWrite_auto(mins, tr, spd, cnt_tp, ran_cnt_tp);
		} else {
			SDC_readWrite_burnin(&rc, 0xFFFF, 0, 0);
		}
	}

	return 0;
}

int32_t SDC_abort_test(int32_t argc, char *const argv[])
{
	uint32_t blk_cnt;

	if (argc < 2)
		return 1;

	blk_cnt = atoi(argv[1]);

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_MMC) ||
			(cardInfo.CardType == MEMORY_CARD_TYPE_SD) ||
			(cardInfo.CardType == MEMORY_eMMC)) {
		SDC_PrepareWB(0);
 
		ftsdc021_abort_test(read_buf, write_buf, blk_cnt);
	}
	return 0;
}

int32_t SDC_cprm_test(int32_t argc, char *const argv[])
{
	uint32_t i, addr, count;
	uint32_t ran;

	if (cardInfo.CardType != MEMORY_CARD_TYPE_SD) {
		printf(" No SD Memory Card insert.\r\n");
		return 0;
	}

	if (argc != 2)
		return 1;

	auto_ed_cbc = atoi(argv[1]);

	//ftsdc021_set_transfer_type(ADMA, 2048, 2);

	SDC_PrepareWB(0);

	i = 0;
	while (1) {
		if ('q' == kbhit()) {
			printf("CPRM test %d times.\r\n", i);
			return 0;
		}

		ran = (uint32_t) rand();
	
		addr = ran % 1000;
		count = (ran % 128) + 1;

		if (SDC_CPRM(addr, count)) {
			if (cardInfo.FlowSet.UseDMA == ADMA)
				SDC_ShowDscpTbl(1, 0);
			printf("CPRM test %d times.\r\n", i);
			return 0;
		}

		i++;
	}

	return 0;
}

/* trans <pio|sdma|adma|edma> [line|bound] [0|1|2|3] [nop] [intr] */
int32_t SDC_settrans_cmd(int32_t argc, char *const argv[])
{
	Transfer_Type tp;
	uint32_t ln;
	uint8_t rnd;

	if (strcmp(argv[1], "pio") == 0) {
		if (argc < 3)
			return 1;

		tp = PIO;
		ln = atoi(argv[2]); 
	
	} else if (strcmp(argv[1], "sdma") == 0) {
		if (argc < 3)
			return 1;

		tp = SDMA;
		ln = atoi(argv[2]) & 0x7;
	
	} else if (strcmp(argv[1], "adma") == 0) {
		if (argc != 6)
			return 1;

		tp = ADMA;
		ln = atoi(argv[2]) & 0xFFFC;
		rnd = atoi(argv[3]) & 0x3;

		adma2_insert_nop = atoi(argv[4]);
		adma2_use_interrupt = atoi(argv[5]) ? ADMA2_ENTRY_INT : 0;
	
		/* Infinite transfer is not supported by ADMA */
		infinite_mode = 0;
#ifdef FTSDC021_EDMA
	} else if (strcmp(argv[1], "edma") == 0) {		
		tp = EDMA;
		ln = rnd = 0;
#endif
	} else {
		goto show;
	}

	ftsdc021_set_transfer_type(tp, ln, rnd);

show:
	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) ||
		(cardInfo.CardType == MEMORY_CARD_TYPE_MMC) ||
		(cardInfo.CardType == MEMORY_eMMC)) {
		SDC_SD_menu();
	}
	else if (cardInfo.CardType == SDIO_TYPE_CARD) {
		SDC_SDIO_menu();
	}
	else {
		printf("Unknown Card Type !");
	}

	return 0;
}

/* setflow <autoCmd> <async|sync> <Erasing>*/
int32_t SDC_setflow_cmd(int32_t argc, char *const argv[])
{
	uint8_t auto_cmd;

	if (argc < 4) {
		return 1;
	}

	auto_cmd = atoi(argv[1]) & 0x3;
	if ((auto_cmd == 2) && !cardInfo.SCR.CMD23_SUPPORT &&
	    (cardInfo.CardType == MEMORY_CARD_TYPE_SD)) {
		printf(" WARN## ... Card does not support CMD23.\r\n");
		auto_cmd = 1;
	}
	cardInfo.FlowSet.autoCmd = auto_cmd;

	if (strcmp(argv[2], "async") == 0)
		cardInfo.FlowSet.SyncAbt = ABORT_ASYNCHRONOUS;
	else
		cardInfo.FlowSet.SyncAbt = ABORT_SYNCHRONOUS;

	cardInfo.FlowSet.Erasing = atoi(argv[3]);

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) ||
			(cardInfo.CardType == MEMORY_CARD_TYPE_MMC) ||
			(cardInfo.CardType == MEMORY_eMMC)) {
		SDC_SD_menu();
	} else if (cardInfo.CardType == SDIO_TYPE_CARD) {
		SDC_SDIO_menu();
	} else {
		printf("Unknown Card Type !");
	}

	return 0;
}

/* inf <0-2> */
int32_t SDC_setInfinite_mode(int32_t argc, char *const argv[])
{
	if (argc != 2)
		return 1;

	if (cardInfo.FlowSet.UseDMA != ADMA)
		infinite_mode = atoi(argv[1]);
	else {
		infinite_mode = 0;
		printf("Infinite transfer is not supported by ADMA.\r\n");
	}

	if (infinite_mode > 2)
		return 1;

	return 0;
}

/* speed <ds|hs|sdr50|hs200|sdr104|ddr50> [clock] */
int32_t SDC_switch_cmd(int32_t argc, char *const argv[])
{
	Bus_Speed speed;

	if (argc < 2) {
		return 1;
	}

	if (strcmp(argv[1], "ds") == 0)
		speed = 0;
	else if (strcmp(argv[1], "hs") == 0)
		speed = 1;
	else if (strcmp(argv[1], "sdr50") == 0)
		speed = 2;
	else if (strcmp(argv[1], "hs200") == 0)
		speed = 2;
	else if (strcmp(argv[1], "sdr104") == 0)
		speed = 3;
	else
		speed = 4;

	if (!ftsdc021_set_bus_speed_mode(speed)) {
		if (argc == 3)
			ftsdc021_SetSDClock(atoi(argv[2]) * 1000000);
	}

	return 0;
}

/* clk <MHz> */
int32_t SDC_set_clock(int32_t argc, char *const argv[])
{
	uint32_t target;

	if (argc < 2)
		return 1;

	target = atoi(argv[1]) * 1000000;
	ftsdc021_SetSDClock(target);

	return 0;
}

/* bclk <MHz> */
int32_t SDC_set_base_clock(int32_t argc, char *const argv[])
{
	uint32_t base;

	if (argc < 2)
		return 1;

	base = atoi(argv[1]);
	ftsdc021_set_base_clock(base);
	printf(" Base Clock change to %d MHz.\r\n", base);

	return 0;
}

/* bw <1|4|8> */
int32_t SDC_bus_width_cmd(int32_t argc, char *const argv[])
{
	uint8_t width;

	width = atoi(argv[1]);

	if ((width != 1) && (width != 4) && (width != 8)) {
		printf("Please specify valid bus witdh(1 or 4 or 8).\r\n");
		return 1;
	}

	ftsdc021_set_bus_width(width);

	return 0;
}

/* scr */
int32_t SDC_read_scr_cmd(int32_t argc, char *const argv[])
{
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD)
		ftsdc021_sd_read_scr();

	return 0;
}

/* reset <all|cmd|dat> */
int32_t SDC_software_reset(int32_t argc, char *const argv[])
{
	uint8_t type;

	if (argc < 2)
		return 1;

	if (strcmp(argv[1], "cmd") == 0)
		type = SDHCI_SOFTRST_CMD;
	else if (strcmp(argv[1], "dat") == 0)
		type = SDHCI_SOFTRST_DAT;
	else
		type = SDHCI_SOFTRST_ALL;

	ftsdc021_HCReset(type);

	return 0;
}

int32_t SDC_read_card_status(int32_t argc, char *const argv[])
{
	if (cardInfo.CardType != SDIO_TYPE_CARD) {

		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf("ERR## Get Card Status Failed !\r\n");
			return 0;
		}

		printf(" Card Status = 0x%08x\r\n",
			    (uint32_t) cardInfo.respLo);

		printf("  in %s.\r\n",
			    SDC_ShowCardState((cardInfo.respLo >> 9) & 0xF));

		if(cardInfo.respLo & (0x1 << 3))
			printf("  AKE_Seq_error\r\n");

		if(cardInfo.respLo & (0x1 << 5))
			printf("  The following cmd will be ACMD\r\n");

		if(cardInfo.respLo & (0x1 << 8))
			printf("  Ready for Data\r\n");

		if(cardInfo.respLo & (0x1 << 13))
			printf("  Erase Reset: set\r\n");

		if(cardInfo.respLo & (0x1 << 14))
			printf("  Card ECC: Disabled\r\n");
		else
			printf("  Card ECC: Enabled\r\n");

		if(cardInfo.respLo & (0x1 << 15))
			printf("  WP Erase Skip: protected\r\n");

		if(cardInfo.respLo & (0x1 << 16))
			printf("  CSD Overwrite: err\r\n");

		if(cardInfo.respLo & (0x1 << 19))
			printf("  General or Unknown Error\r\n");

		if(cardInfo.respLo & (0x1 << 20))
			printf("  Internal Card Controller err\r\n");

		if(cardInfo.respLo & (0x1 << 21))
			printf("  Card ECC Failure\r\n");

		if(cardInfo.respLo & (0x1 << 22))
			printf("  Illegal Command err\r\n");

		if(cardInfo.respLo & (0x1 << 23))
			printf("  Prev. Command CRC err\r\n");

		if(cardInfo.respLo & (0x1 << 24))
			printf("  Lock/Unlock err\r\n");

		if(cardInfo.respLo & (0x1 << 25))
			printf("  Card locked\r\n");

		if(cardInfo.respLo & (0x1 << 26))
			printf("  WP Violation\r\n");

		if(cardInfo.respLo & (0x1 << 27))
			printf("  Erase Param. err\r\n");

		if(cardInfo.respLo & (0x1 << 28))
			printf("  Erase Seq. err\r\n");

		if(cardInfo.respLo & (0x1 << 29))
			printf("  Block Len err\r\n");

		if(cardInfo.respLo & (0x1 << 30))
			printf("  Addr. err\r\n");

		if(cardInfo.respLo & ((uint32_t)0x1 << 31))
			printf("  Out of Range\r\n");
	}

	return 0;
}

int32_t SDC_repo_written_num(int32_t argc, char *const argv[])
{
	uint32_t written_num;
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {

		if (ftsdc021_ops_app_repo_wr_num(&cardInfo, &written_num)) {
			printf("ERR## Get Written Block Num Failed !\r\n");
			return 0;
		}

		/* The info. in card is big-endian and should be converted to
		 * little-endian. Check by human */
		if (cardInfo.block_addr)
			printf(" Written block(s) of last write = "
				    "0x%02x%02x%02x%02x\r\n",
				    ((written_num & (uint32_t)0xFF) << 24),
				    (written_num & (0xFF << 8)) >> 8,
				    (written_num & (0xFF << 16)) >> 16,
				    (written_num & ((uint32_t)0xFF << 24)) >> 24);
		else
			printf(" Written byte(s) of last write = "
				    "0x%02x%02x%02x%02x\r\n",
				    ((written_num & (uint32_t)0xFF) << 24),
				    (written_num & (0xFF << 8)) >> 8,
				    (written_num & (0xFF << 16)) >> 16,
				    (written_num & ((uint32_t)0xFF << 24)) >> 24);

	}
	return 0;
}

/* abort */
int32_t SDC_send_abort_cmd(int32_t argc, char *const argv[])
{

	ftsdc021_send_abort_command();

	return 0;
}

/* tune <num> */
int32_t SDC_tune_cmd(int32_t argc, char *const argv[])
{
	uint32_t num;

	if (argc != 2)
		return 1;

	num = atoi(argv[1]);
	ftsdc021_execute_tuning(num);

	return 0;
}

uint32_t SDC_data_check(uint32_t blkcnt, uint32_t *source_buf, uint32_t *dest_buf)
{
	uint32_t i;

	for (i = 0; i < ((1 << wr_bl_len) / 4 * blkcnt); i++) {
		if (dest_buf[i] != source_buf[i]) {
			printf(" Compare error at idx 0x%x Source "
				    "0x%08x(0x%08x) Destination "
				    "0x%08x(0x%08x).\r\n", (i << 2),
				    (source_buf + i), *(source_buf + i),
				    (dest_buf + i), *(dest_buf + i));
			break;
		}
	}

	return (i << 2);
}

void SDC_PrepareWB(uint32_t length)
{
	uint32_t i, pat_sz;
	uint32_t *data_addr;

	if (buffer_filled)
		return;

	pat_sz = sizeof(tuning_pattern) / sizeof(uint32_t);

	data_addr = (uint32_t *)write_buf;

	if (!length)
		length = FTSDC021_BUFFER_LENGTH >> 1;

	printf(" Filling buffer to 0x%08x length %d bytes (%d blocks)... "
		    "(This might take several minutes)\r\n", data_addr,
		    length, length >> 9);

	length >>= 2;
	for (i = 0; i < length; i++) {
		if ('q' == kbhit()) {
			break;
		}
		if ((i % 128) < pat_sz)
			data_addr[i] = tuning_pattern[i % 128];
		else
			data_addr[i] = (0xA5A50000 + i);
	}
	buffer_filled = 1;
}

/* rd <addr> <blkcnt> [repeat] */
int32_t SDC_read_cmd(int32_t argc, char *const argv[])
{
	uint32_t i, addr, cnt, repeat;
	clock_t t0, t1;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > cardInfo.numOfBlocks) {
		/* For FPGA verification purpose, just give warning and allow
		 * the operation. */
		printf(" WARN## ...  Out of Range (%d + %d) > %d.\r\n", addr,
			    cnt, cardInfo.numOfBlocks);
	}

	if (argc == 4)
		repeat = atoi(argv[3]);
	else
		repeat = 1;

	/* Check if we have enough buffer */
	if ((cnt * (1 << rd_bl_len)) > FTSDC021_BUFFER_LENGTH)
		cnt = FTSDC021_BUFFER_LENGTH / (1 << rd_bl_len);

	/* CMD 16 */
	ftsdc021_ops_set_blocklen(&cardInfo, (1 << rd_bl_len));

	for (i = 0; i < repeat; i++) {
		memset(read_buf, 0, cnt * (1 << rd_bl_len));

		printf(" Read %d blocks from address %d ", cnt, addr);

		t0 = clock();
		if (!ftsdc021_card_read(addr, cnt, read_buf)) {
			t1 = clock();
			printf(" success. (%d ms)\r\n", (t1 - t0));
		} else
			printf(" FAILED.\r\n");

		//addr += cnt;
	}

	return 0;
}

/* wr <addr> <blkcnt> [repeat] [compare] */
int32_t SDC_write_cmd(int32_t argc, char *const argv[])
{
	uint32_t i, addr, cnt, repeat;
	clock_t t0, t1;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > cardInfo.numOfBlocks) {
		/* For FPGA verification purpose, just give warning and allow
		 * the operation. */
		printf(" WARN## ...  Out of Range (%d + %d) > %d.\r\n", addr,
			    cnt, cardInfo.numOfBlocks);
		//return 0;
	}

	/* Check if we have enough buffer */
	if ((cnt * (1 << wr_bl_len)) > FTSDC021_BUFFER_LENGTH)
		cnt = FTSDC021_BUFFER_LENGTH / (1 << wr_bl_len);

	SDC_PrepareWB(0);

	if (argc > 3)
		repeat = atoi(argv[3]);
	else
		repeat = 1;

	/* send CMD16 for (1 << wr_bl_len) Bytes fixed block length */
	/* If RPMB partition is selected, CMD16 is not support. */
	if ((cardInfo.EXT_CSD_MMC.PARTITION_CONF & 7) != 3)
		ftsdc021_ops_set_blocklen(&cardInfo, (1 << wr_bl_len));

	for (i = 0; i < repeat; i++) {

		printf(" Write %d blocks to address %d, ", cnt, addr);

		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) 
			return 0;

		t0 = clock();
		if (ftsdc021_card_write(addr, cnt, write_buf)) {
			printf("FAILED.\r\n");
			if (cardInfo.FlowSet.UseDMA == ADMA)
				SDC_ShowDscpTbl(1, 0);
			return 0;
		}
		t1 = clock();

		if (argc == 5) {
			memset(read_buf, 0, (cnt << rd_bl_len));

			if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) 
				return 1;

			if (cardInfo.cqe_on)
				cardInfo.cqe_wait_task_done = 1;
			if (ftsdc021_card_read(addr, cnt, read_buf)) {
				printf(", read back FAILED.\r\n");

				if (cardInfo.FlowSet.UseDMA == ADMA)
					SDC_ShowDscpTbl(1, 0);
				return 0;
			}

			if ((cnt << rd_bl_len) >
			    SDC_data_check(cnt, (uint32_t *)write_buf,
					   (uint32_t *)read_buf)) {
				return 0;
			}
			printf(", compare success. (%d ms)\r\n", (t1 - t0));
		} else {
			printf("success. (%d ms)\r\n", (t1 - t0));
		}

		addr += cnt;
	}

	return 0;
}

uint32_t SDC_data_check_for_erasing(uint32_t start_block, uint32_t blk_cnt)
{
	uint32_t i, t;
	uint32_t pattern;
	uint32_t *buf;

	// Getting the essential info. depend on different card type
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		pattern = (cardInfo.SCR.DATA_STAT_AFTER_ERASE) ? 0xFFFFFFFF : 0x0;
	} else if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC || cardInfo.CardType == MEMORY_eMMC) {
		pattern = (cardInfo.EXT_CSD_MMC.ERASED_MEM_CONT) ? 0xFFFFFFFF : 0x0;
	} else if (cardInfo.CardType == SDIO_TYPE_CARD) {
		printf("It's not effect for SDIO card\r\n");
		return 1;
	} else {
		printf("Unknown the Card type\r\n");
		return 1;
	}

	memset(read_buf, 0xaa, blk_cnt * (1 << wr_bl_len));

	if (cardInfo.cqe_on)
		cardInfo.cqe_wait_task_done = 1;
	if (ftsdc021_card_read(start_block, blk_cnt, read_buf)) {
		printf(" ERR## ... Check Erase, Read FAILED.\r\n");
		return 1;
	}

	t = (blk_cnt * (1 << wr_bl_len)) >> 2;
	buf = (uint32_t *) read_buf;
	// Checking the buf's content after erasing
	for (i = 0; i < t; i++) {
		if (*(buf + i) != pattern) {
			printf(" The addr 0x%x is FAILED. The value is 0x%x\r\n", (read_buf + i), *(read_buf + i));
			return 1;
		}
	}

	printf("Data Compare OK!!\r\n");
	return 0;
}

/* er <addr> <blkcnt> */
int32_t SDC_erase_cmd(int32_t argc, char *const argv[])
{
	uint32_t addr, cnt,  size;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > (cardInfo.numOfBlocks + 1)) {
		printf("Selecting area exceedes the Max. block number: %d!!\r\n", cardInfo.numOfBlocks);
		return 0;
	}

	if (get_erase_group_size(&size))
		return 0;

	/* Means erase all*/
	if (cnt == 0) { 
		while (addr < cardInfo.numOfBlocks) {
			cnt = cardInfo.numOfBlocks - addr;

			if (cnt > 32768)
				cnt = 32768;

			printf("Erasing from address %d total %d blocks", addr, cnt); 
			if (ftsdc021_card_erase(addr, cnt)) {
				printf(" FAILED.\r\n");
				return 0;
			} else
				printf(" success.\r\n");
#if 0
			SDC_data_check_for_erasing(addr, cnt);
#endif
			addr += cnt;
		}

	} else {
		printf("Erasing from block %d total %d, size %d blocks", addr, cnt, size);
		if (ftsdc021_card_erase(addr, cnt)) {
			printf(" FAILED.\r\n");
			return 0;
		} else
			printf(" success.\r\n");
#if 0
		SDC_data_check_for_erasing(addr, cnt);
#endif
	}

	return 0;
}

/* discard <addr> <blkcnt> */
int32_t SDC_discard_cmd(int32_t argc, char *const argv[])
{
	uint32_t addr, cnt,  size;

	if (argc < 3) {
		return 1;
	}

	addr = atoi(argv[1]);
	cnt = atoi(argv[2]);

	if (addr + cnt > (cardInfo.numOfBlocks + 1)) {
		printf("Selecting area exceedes the Max. block number: %d!!\r\n", cardInfo.numOfBlocks);
		return 0;
	}

	if (get_erase_group_size(&size))
		return 0;

	/* Means erase all*/
	if (cnt == 0) {
		while (addr < cardInfo.numOfBlocks) {
			cnt = cardInfo.numOfBlocks - addr;

			if (cnt > 32768)
				cnt = 32768;

			printf("Discard from address %d total %d blocks", addr, cnt);
			if (ftsdc021_card_discard(addr, cnt)) {
				printf(" FAILED.\r\n");
				return 0;
			} else
				printf(" success.\r\n");
			addr += cnt;
		}

	} else {
		printf("Discard from block %d total %d, size %d blocks", addr, cnt, size);
		if (ftsdc021_card_discard(addr, cnt)) {
			printf(" FAILED.\r\n");
			return 0;
		} else
			printf(" success.\r\n");
	}

	return 0;
}

void SDC_error_show(void)
{
#if 0
	uint32_t state;

	ftsdc021_HCReset((SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT));

	if (cardInfo.FlowSet.UseDMA == ADMA)
		SDC_ShowDscpTbl(1, 0);

	SDC_SD_menu();

	if (ftsdc021_ops_send_card_status(&cardInfo)) {
		printf("ERR## Get Card Status Failed (Init) !\r\n");
	}

	state = (cardInfo.respLo >> 9) & 0xF;

	printf(" Card Status = 0x%08x, in %s.\r\n", (uint32_t) cardInfo.respLo,
		    SDC_ShowCardState(state));

	if (state != CUR_STATE_TRAN)
		ftsdc021_send_abort_command();
#endif
	g_error_cnt++;
}

uint32_t SDC_readWrite_burnin(uint32_t * result, uint32_t mins, uint8_t cnt_tp, uint8_t ran_cnt_tp)
{
	uint32_t addr, blk_cnt, tmp, blk_boundary, ret;
	uint32_t wr_time, wr_cnt;
	uint32_t rd_time, rd_cnt;
	uint32_t act;		/* 0 means read, 1 for write */
	clock_t t0, t1, ts;
	uint64_t ran;
	char ks;

	ftsdc021_ops_set_blocklen(&cardInfo, (1 << wr_bl_len));

	wr_time = rd_time = wr_cnt = rd_cnt = 0;
	SDC_PrepareWB(0);

	blk_boundary = (sdma_bound_mask + 1) >> wr_bl_len;

	mins = mins * 60; //to seconds
	ts = clock();

	printf("\r\n Overnight burn-in start (press 'q' to stop the test).\r\n");
	do {

		ran = (uint64_t) rand();
#if 1
		if ((ran % 20) == 0 && cardInfo.cqe_en && cardInfo.cqe_on) {
			cqhci_off(1);
			ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 0, 1);
		}
		else if ((ran % 20) && cardInfo.cqe_en && !cardInfo.cqe_on) {
			ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 1, 1);
			cqhci_on();
		}
#endif
		if (cardInfo.max_context_id) {
			cardInfo.context_id = (ran % cardInfo.max_context_id) + 1;
		}

		if (cardInfo.FlowSet.Erasing == 1 &&
		    (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
		     cardInfo.CardType == MEMORY_eMMC)) {
			// Getting the random number from 1 to 4
			blk_cnt = (ran & 0x3) + 1;
			// Getting the block count from 512, 1024, 1536, and 2048, randomly.
			blk_cnt = blk_cnt * 512;
		} else {
			if(cnt_tp == 0) {
				if(ran_cnt_tp == 0) {
					tmp = (wr_cnt + rd_cnt) % 500;
					if (tmp < 150) {
						blk_cnt = (ran & 0x1F) + 1;
					} else if (tmp < 250) {
						blk_cnt = 32 + (ran & 0x3F);
					} else if (tmp < 330) {
						blk_cnt = 96 + (ran & 0x7F);
					} else if (tmp < 400) {
						blk_cnt = 224 + (ran & 0x1FF);
					} else if (tmp < 460){
						blk_cnt = 736 + (ran & 0x3FF);
					} else {
						blk_cnt = 1760 + (ran & 0x3FF);
					}
				}
				else
					blk_cnt = (ran & 0x3FF) + 1;
			}
			else {
				blk_cnt = (uint32_t)cnt_tp;
			}
		}

		/* Must consider SDMA buffer boundary */
		if (cardInfo.FlowSet.UseDMA == SDMA) {
			addr = ran % (cardInfo.numOfBlocks - blk_boundary);

			if (addr + blk_cnt > (cardInfo.numOfBlocks - blk_boundary)) {
				addr -= blk_boundary;
				blk_cnt = blk_boundary;
			}
			
		} else {
			addr = ran % cardInfo.numOfBlocks;

			if (addr >= cardInfo.numOfBlocks)
				addr = cardInfo.numOfBlocks - blk_cnt;

			if (addr + blk_cnt > cardInfo.numOfBlocks) {
				addr -= 1;
				blk_cnt = cardInfo.numOfBlocks - addr;
			}
		}

		/* ----------------------------------------------------------------
		 * We stimulate about 60% write and 40% read. 
		 * For write action, read it back to compare but we don't calculate
		 * the speed.
		 */
		act = ran & 0xF;

		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) 
			return 1;

		if (act < 9) {
			wr_cnt++;
			if (dbg_print > 0)
				printf(" Writing %d blocks to addr %d",
					    blk_cnt, addr);
			/* Do write */
			t0 = clock();
			if (cardInfo.RPMB_access) {
				//rpmb write
				ret = mmc_rpmb_write(&cardInfo, write_buf, addr,
						     blk_cnt,
						     (uint8_t *)rpmb_key);
			} else if (packed_test_en &&
				   cardInfo.FlowSet.autoCmd == 2 &&
				   cardInfo.FlowSet.UseDMA != SDMA &&
				   !infinite_mode) {
				cardInfo.doing_packed_cmd = PACKED_WRITE;
				ftsdc021_mmc_gen_packed_hdr(WRITE, addr, blk_cnt,
							    packed_hdr);
				printf(" MMC: send packed write "
					    "commands.\r\n");
				ret = ftsdc021_card_write(addr, blk_cnt + 1,
							  (uint8_t *)packed_hdr);
				cardInfo.doing_packed_cmd = NO_PACKED_CMD;
			} else
				ret = ftsdc021_card_write(addr, blk_cnt,
							  write_buf);
			if (ret) {
				printf(" FAILED!\r\n");
				SDC_error_show();
				goto fail;
			}
			t1 = clock();
			if ((t1-t0) == 0) {
				printf("*");
			} else
				wr_time += ((blk_cnt << wr_bl_len) / (t1 - t0));

			/* Pass read back check if act < 6 */
			if (act < 9)
				goto done;

			if (dbg_print > 0)
				printf(", Reading back,");

			if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000)) 
				return 1;

			/* Do read to compare the data content */
			memset(check_buf, 1, blk_cnt * (1 << rd_bl_len));
			/* if packed commands test is enabled, a packed header
			 * must be send before CMD18*/
			if (cardInfo.RPMB_access) {
				//rpmb read
				ret = mmc_rpmb_read(&cardInfo, check_buf, addr,
						    blk_cnt, (uint8_t *)rpmb_key);
			} else if (packed_test_en &&
				   cardInfo.FlowSet.autoCmd == 2 &&
				   cardInfo.FlowSet.UseDMA != SDMA &&
				   !infinite_mode) {
				cardInfo.doing_packed_cmd = PACKED_READ;
				ftsdc021_mmc_gen_packed_hdr(READ, addr, blk_cnt,
							    packed_hdr);
				printf(" MMC: send packed read "
					    "header(%d,%d,%p).\r\n", addr,
					    blk_cnt, packed_hdr);
				ftsdc021_card_write(addr, 1, 
						    (uint8_t *)packed_hdr);
				ret = ftsdc021_card_read(addr, blk_cnt,
							 check_buf);
				cardInfo.doing_packed_cmd = NO_PACKED_CMD;
			}
			else {
				if (cardInfo.cqe_on)
					cardInfo.cqe_wait_task_done = 1;
				ret = ftsdc021_card_read(addr, blk_cnt,
							 check_buf);
			}

			if (ret) {
				printf(", Read back FAILED.\r\n");
				SDC_error_show();
				goto fail;
			}

			ret = SDC_data_check(blk_cnt, (uint32_t *)write_buf,
					     (uint32_t *)check_buf);
			if ((blk_cnt << wr_bl_len) > ret) {
				SDC_error_show();
				goto fail;
			}

done:
			if (dbg_print > 0)
				printf(" PASSED.\r\n");
		} else {
			rd_cnt++;
			
			if (dbg_print > 0)
				printf(" Reading %d blocks to addr %d",
					    blk_cnt, addr);

			t0 = clock();
			if (cardInfo.RPMB_access) {
				//rpmb write
				ret = mmc_rpmb_write(&cardInfo, write_buf, addr,
						     blk_cnt,
						     (uint8_t *)rpmb_key);
			}
			else {
				ret = ftsdc021_card_read(addr, blk_cnt,
							 read_buf);
			}
			if (ret) {
				printf(" FAILED.\r\n");
				SDC_error_show();
				goto fail;
			}
			t1 = clock();
			if ( (t1-t0) == 0 ) {
				printf("*");
			} else
				rd_time += ((blk_cnt << rd_bl_len) / (t1 - t0));

			if (dbg_print > 0)
				printf(" PASSED.\r\n");
		}

		if (cardInfo.FlowSet.Erasing && (act < 9)) {
			if (ftsdc021_card_erase(addr, blk_cnt))
				goto out;

			if (SDC_data_check_for_erasing(addr, blk_cnt)) {
				goto out;
			}
		}

		if ((clock() - ts) > CLOCKS_PER_SEC) {
			mins--;
			ts = clock();
			if (!mins) {
				*result = 0;
				goto out;
			}
		}

		ks = kbhit();
		/* skip to next settings */
		if ('s' == ks) {
			*result = 2;
			goto out;
		}

		if ('q' == ks) {
			*result = 1;
			goto out;
		}

		addr = addr + blk_cnt;

	} while (1);

fail:
	/* Fail */	
	*result = 1;
out:
	if (wr_cnt)
		printf("Total Write: %d times (Average %d Bytes/ms).\r\n",
			    wr_cnt, wr_time / wr_cnt);

	if (rd_cnt)
		printf("Total Read: %d times (Average %d Bytes/ms).\r\n",
			    rd_cnt, rd_time / rd_cnt);

	printf("Total Error: %d.\r\n", g_error_cnt);

	return 0;
}

/* Helper function for SDC_readWrite_auto */
uint32_t SDC_burnin_speed_and_width(uint32_t mins, uint8_t md_list, uint8_t cnt_tp,
				  uint8_t ran_cnt_tp)
{
	uint8_t spd, wdth;
	uint32_t rc;

	/* Only use 4-bit for UHS-1 card */
	if (ftsdc021_uhs_mode())
		wdth = 4;
	else
		wdth = 1;

	for (spd = 0; spd < 5; spd++) {
		/* Not in the test list */
		if (!(md_list & (1 << spd)))
			continue;

		/* Speed */	
		if (cardInfo.CardType == MEMORY_CARD_TYPE_SD &&
		    cardInfo.SCR.SD_SPEC)
			if (!(cardInfo.bs_mode & (1 << spd))) {
				printf(" Card not support %d speed "
					    "mode.\r\n", spd);
				continue;
			}


		if (cardInfo.speed != spd)
			if (ftsdc021_set_bus_speed_mode(spd))
				return 1;

		/* Bus width */
		while (wdth < 9) {
			if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
			    cardInfo.CardType == MEMORY_eMMC) {
				if (cardInfo.CSD_MMC.SPEC_VERS < 4 && wdth > 1) {
					printf(" MMC version less than "
						    "MMC4.1 not support 4 bit "
						    "bus width.\r\n");
					return 0;
				}
			} else if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {

				if (wdth > 4)
					break;

				if ((wdth == 1) && !(cardInfo.SCR.SD_BUS_WIDTHS &
				    SDHCI_SCR_SUPPORT_1BIT_BUS)) {
					printf(" SD: 1 bit width not "
						    "support by this "
						    "card.\r\n");
					wdth = 4;
					continue;
				}

				if ((wdth == 4) && !(cardInfo.SCR.SD_BUS_WIDTHS &
				    SDHCI_SCR_SUPPORT_4BIT_BUS)) {
					printf(" SD: 4 bit width not "
						    "support by this "
						    "card.\r\n");
					return 0;
				}
			}

			if (cardInfo.bus_width != wdth)
				if (ftsdc021_set_bus_width(wdth))
					return 1;
		
			rc = 0;
			SDC_SD_menu();
			SDC_readWrite_burnin(&rc, mins, cnt_tp, ran_cnt_tp);
			if (rc == 1)
				return 1;
 
			if (wdth == 1)
				wdth = 4;
			else if (wdth == 4)
				wdth = 8;
			else	/* Arbitary value to leave the loop */
				wdth = 16;
		}
		/* Only use 4-bit for UHS-1 card */
		if (ftsdc021_uhs_mode())
			wdth = 4;
		else
			wdth = 1;
	}

	return 0;
}

uint32_t SDC_burnin_sdma(uint32_t mins, uint8_t md_list, uint8_t cnt_tp,
		       uint8_t ran_cnt_tp)
{
	uint32_t i;

	infinite_mode = 0;

	for (cardInfo.FlowSet.autoCmd = 0; cardInfo.FlowSet.autoCmd < 2;
	     cardInfo.FlowSet.autoCmd++) {
		i = 0;
		while(i < 8) { 		
			ftsdc021_set_transfer_type(SDMA, i, 3);

			if (SDC_burnin_speed_and_width(mins, md_list, cnt_tp,
						       ran_cnt_tp))
				return 1;

			/* Change SDMA Address boundary from 4K, 32K, 64K and
			 * 512K */
			if (!i)
				i = 3;
			else if (i == 3)
				i = 4;
			else if (i == 4)
				i = 7;
			else
				i = 8;
		}
	}

	return 0;
}

uint32_t SDC_burnin_adma(uint32_t mins, uint8_t md_list, uint8_t cnt_tp,
		       uint8_t ran_cnt_tp)
{
	uint8_t autoCmd;

	adma2_insert_nop = 0;
	adma2_use_interrupt = 0;

	/* Infinite transfer is not supported by ADMA */
	infinite_mode = 0;

	/* ADMA always use random length and action */
	ftsdc021_set_transfer_type(ADMA, 0, 3);

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) && 
	    ftsdc021_uhs_mode() && cardInfo.SCR.CMD23_SUPPORT)
                autoCmd = 2;
        else
                autoCmd = 0;

	for (cardInfo.FlowSet.autoCmd = autoCmd; cardInfo.FlowSet.autoCmd < 3;
	     cardInfo.FlowSet.autoCmd++) {
		if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) &&
		    (cardInfo.FlowSet.autoCmd == 2) &&
		    !cardInfo.SCR.CMD23_SUPPORT)
			continue;

		for (adma2_insert_nop = 0; adma2_insert_nop < 2;
		     adma2_insert_nop++) {
			for (adma2_use_interrupt = 0;  adma2_use_interrupt < 5;
			     adma2_use_interrupt += 4) {
				if (SDC_burnin_speed_and_width(mins, md_list,
							       cnt_tp,
							       ran_cnt_tp)) {
					SDC_ShowDscpTbl(1, 0);
					return 1;
				}
			}
		}
	}

	return 0;
}

uint32_t SDC_burnin_pio(uint32_t mins, uint8_t md_list, uint8_t cnt_tp, uint8_t ran_cnt_tp)
{
	uint32_t wd;
	uint8_t autoCmd;

	infinite_mode = 0;

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) && 
	    ftsdc021_uhs_mode() && cardInfo.SCR.CMD23_SUPPORT)
                autoCmd = 2;
        else
                autoCmd = 0;

	/* Access data port width 1-byte, 2-bytes and 4-bytes */
	for (cardInfo.FlowSet.autoCmd = autoCmd; cardInfo.FlowSet.autoCmd < 3;
	     cardInfo.FlowSet.autoCmd++) {
		if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) &&
		    (cardInfo.FlowSet.autoCmd == 2) &&
		    !cardInfo.SCR.CMD23_SUPPORT)
			break;

		for (wd = 1; wd <= 4; wd <<= 1) {
			ftsdc021_set_transfer_type(PIO, wd, 3);

		tst:
			if (SDC_burnin_speed_and_width(mins, md_list, cnt_tp, ran_cnt_tp))
				return 1; 

			/* No Auto CMD12 or Auto CMD23, can do infinite transfer */
			if (cardInfo.FlowSet.autoCmd == 0) {
				if (infinite_mode < 2) {
					infinite_mode++;
					goto tst;
				} else {
					infinite_mode = 0;
				}
			}
		}
	}

	return 0;
}

uint32_t SDC_burnin_edma(uint32_t mins, uint8_t md_list, uint8_t cnt_tp,
		       uint8_t ran_cnt_tp)
{
#ifdef FTSDC021_EDMA
	uint8_t autoCmd;

	infinite_mode = 0;

	ftsdc021_set_transfer_type(EDMA, 0, 0);

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) && 
	    ftsdc021_uhs_mode() && cardInfo.SCR.CMD23_SUPPORT)
		autoCmd = 2;
	else
		autoCmd = 0;

	/* Access data port width 1-byte, 2-bytes and 4-bytes */
	for (cardInfo.FlowSet.autoCmd = autoCmd; cardInfo.FlowSet.autoCmd < 3;
	     cardInfo.FlowSet.autoCmd++) {
		if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) &&
		    (cardInfo.FlowSet.autoCmd == 2) &&
		    !cardInfo.SCR.CMD23_SUPPORT)
			break;

		if (SDC_burnin_speed_and_width(mins, md_list, cnt_tp,
					       ran_cnt_tp))
			return 1; 

	}

#else
	return 0;
#endif
}

uint32_t SDC_readWrite_auto(uint32_t mins, uint8_t tp_list, uint8_t md_list,
			  uint8_t cnt_tp, uint8_t ran_cnt_tp)
{
	int cnt;
	
	printf("Burn-In list 0x%x duration %d mins.\r\n", tp_list, mins);
	for (cnt = 0; cnt < 4; cnt++) {
		switch (cnt) {
			case 0:
				cardInfo.FlowSet.Erasing = 0;
				cardInfo.FlowSet.SyncAbt = ABORT_ASYNCHRONOUS;
				break;
			case 1:
				cardInfo.FlowSet.Erasing = 0;
				cardInfo.FlowSet.SyncAbt = ABORT_SYNCHRONOUS;
				break;
			case 2:
				cardInfo.FlowSet.Erasing = 1;
				cardInfo.FlowSet.SyncAbt = ABORT_ASYNCHRONOUS;
				break;
			case 3:
				cardInfo.FlowSet.Erasing = 1;
				cardInfo.FlowSet.SyncAbt = ABORT_SYNCHRONOUS;
				break;
		}

		/* PIO(2), SDMA(1), ADMA(0) or EDMA(3) */
		if (tp_list & (1 << ADMA)) {
			if (SDC_burnin_adma(mins, md_list, cnt_tp, ran_cnt_tp))
				goto out;
		}

		if (tp_list & (1 << SDMA)) {
			if (SDC_burnin_sdma(mins, md_list, cnt_tp, ran_cnt_tp))
				goto out;
		}

		if (tp_list & (1 << PIO)) {
			if (SDC_burnin_pio(mins, md_list, cnt_tp, ran_cnt_tp))
				goto out;
		}

#ifdef FTSDC021_EDMA
		if (tp_list & (1 << EDMA)) {
			if (SDC_burnin_edma(mins, md_list, cnt_tp, ran_cnt_tp))
				goto out;
		}
#endif
	}

	printf("Burn-In finished.\r\n");
out:
	SDC_SD_menu();

	return 0;
}

uint8_t SDC_SD_menu(void)
{
	printf("****************  Information  ****************\r\n");
	printf("* Bus width: %d.\r\n", cardInfo.bus_width);
	if (cardInfo.CardType == MEMORY_eMMC && cardInfo.speed == 2)
		printf("* Transfer speed: HS200.\r\n");
	else
		printf("* Transfer speed: %s.\r\n", SDC_ShowTransferSpeed(cardInfo.speed));
	printf("* Transfer type: %s.\r\n", SDC_ShowTransferType(cardInfo.FlowSet.UseDMA));
	if (cardInfo.FlowSet.UseDMA == ADMA) {
		printf("* ADMA2 Bytes per Lines: %d.\r\n", cardInfo.FlowSet.lineBound);
		printf("* ADMA2 random mode: %d.\r\n", cardInfo.FlowSet.adma2Rand);
		printf("* ADMA2 inset Nop: %d.\r\n", adma2_insert_nop);
		printf("* ADMA2 interrupt at last line: %d.\r\n", adma2_use_interrupt);
	} else if (cardInfo.FlowSet.UseDMA == SDMA) {
		printf("* SDMA Boundary: %d.\r\n", 1 << (cardInfo.FlowSet.lineBound + 12));
	} else {
		printf("* Data Port Width: %d.\r\n", cardInfo.FlowSet.lineBound);
	}
	printf("* Auto Command: %d.\r\n", cardInfo.FlowSet.autoCmd);
	printf("* Infinite Test Mode: %d.\r\n", infinite_mode);
	printf("* Erase within burnin: %s.\r\n", cardInfo.FlowSet.Erasing ? "Yes" : "No");
	printf("* Abort: %s.\r\n", SDC_ShowAbortType(cardInfo.FlowSet.SyncAbt));

	return 0;
}

uint8_t SDC_SDIO_menu(void)
{
	printf("***********  Information  ***********\r\n");
	printf("* Bus wide: %d                       *\r\n", cardInfo.bus_width);
	printf("*************************************\r\n");

	return 0;
}

/* scan <ds|hs|sdr12|sdr25|sdr50|sdr104|ddr50> <1|4> */
int32_t SDC_scan_card_cmd(int32_t argc, char *const argv[])
{
	uint8_t speed, bus_width;

	if (!ftsdc021_CheckCardInsert()) {
		printf(" No card inserted !\r\n");
		return 0;
	}

	speed = 0;

	if (argc > 1) {
		if (strcmp(argv[1], "ds") == 0) {
			speed = 0;
			cardInfo.signal_vol = 0; //Not switch to 1.8v
		} else if (strcmp(argv[1], "hs") == 0) {
			speed = 1;
			cardInfo.signal_vol = 0; //Not switch to 1.8v
		} else if (strcmp(argv[1], "sdr12") == 0) {
			speed = 0;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else if (strcmp(argv[1], "sdr25") == 0) {
			speed = 1;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else if (strcmp(argv[1], "sdr50") == 0) {
			speed = 2;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else if (strcmp(argv[1], "hs200") == 0) {
			speed = 2;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else if (strcmp(argv[1], "sdr104") == 0) {
			speed = 3;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else if (strcmp(argv[1], "ddr50") == 0) {
			speed = 4;
			cardInfo.signal_vol = 1; //switch to 1.8v
		} else {
			printf(" WARN## ...  Invalid speed. We use normal speed instead.\r\n");
			speed = 0;
		}
	}

	if (ftsdc021_scan_cards()) {
		printf(" Scan card FAILED !\r\n");
		return 0;
	}

	bus_width = 1;
	if (argc == 3) {
		if (ftsdc021_uhs_mode() && (atoi(argv[2]) < 4))
			bus_width = 4;
		else
			bus_width = atoi(argv[2]);
	}

	ftsdc021_set_bus_width(bus_width);
	ftsdc021_set_bus_speed_mode(speed);

	if ((cardInfo.CardType == MEMORY_CARD_TYPE_SD) ||
			(cardInfo.CardType == MEMORY_CARD_TYPE_MMC) ||
			(cardInfo.CardType == MEMORY_eMMC)) {
		SDC_SD_menu();
	} else if (cardInfo.CardType == SDIO_TYPE_CARD) {
		SDC_SDIO_menu();
	} else {
		printf("Unknown Card Type !");
	}

	return 0;
}

int32_t SDC_performance_test(int32_t argc, char *const argv[])
{
	uint32_t addr, blk_cnt, test_blks, max_blks, min_blks, total_blks, i;
	clock_t ts, te, wr_time, rd_time;
	uint64_t ran;

	ftsdc021_ops_set_blocklen(&cardInfo, (1 << wr_bl_len));
	SDC_PrepareWB(0);
	if (cardInfo.cqe_en && !cardInfo.cqe_on) {
		ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 1, 1);
		cqhci_on();
	}
	//100MB
	total_blks = 100 * 1024 * 1024 / (1 << wr_bl_len);
	max_blks = 8192; //Max blocks per transfer
	min_blks = 32; //Min blocks per transfer
	for (i = 0; i < 2; i++) {
		test_blks = 0;
		wr_time = 0;

		while(1) {
			ran = (uint64_t) rand();
			addr = ran % cardInfo.numOfBlocks;
			ran = (uint64_t) rand();
			blk_cnt = ran % max_blks + 1;
			if (blk_cnt < min_blks)
				blk_cnt = min_blks;
			if (addr + blk_cnt > cardInfo.numOfBlocks)
				addr -= blk_cnt;

			if (cardInfo.FlowSet.Erasing)
				ftsdc021_card_erase(addr, blk_cnt);

			ts = clock();
			if (ftsdc021_card_write(addr, blk_cnt, write_buf))
				return 1;
			te = clock();
			if (te > ts) {
				printf("Write %d blocks to address %d. (%d ms) (%d MB/s)\r\n", blk_cnt, addr, (te - ts),
						((blk_cnt * 500) / 1024) / (te - ts));
				wr_time += (te - ts);
				test_blks += blk_cnt;
			}

			if (test_blks >= total_blks)
				break;
		}

		if (cardInfo.cqe_en && cardInfo.cqe_on) {
			ts = clock();
			while (cqhci_get_queued_tasks());
			te = clock();
			if (te > ts) {
				wr_time += te - ts;
			}
		}
		printf("Total Write: %d blocks in %ld ms (Average %d MB/s).\r\n",
			    test_blks, wr_time, ((test_blks * 500) / 1024) / wr_time);

		test_blks = 0;
		rd_time = 0;

		while (1) {
			ran = (uint64_t) rand();
			addr = ran % cardInfo.numOfBlocks;
			ran = (uint64_t) rand();
			blk_cnt = ran % max_blks + 1;
			if (blk_cnt < min_blks)
				blk_cnt = min_blks;
			if (addr + blk_cnt > cardInfo.numOfBlocks)
				addr -= blk_cnt;

			ts = clock();
			if (ftsdc021_card_read(addr, blk_cnt, read_buf))
				return 1;
			te = clock();
			if (te > ts) {
				printf("Read %d blocks to address %d. (%d ms) (%d MB/s)\r\n", blk_cnt, addr, (te - ts),
						((blk_cnt * 500) / 1024) / (te - ts));
				rd_time += te - ts;
				test_blks += blk_cnt;
			}

			if (test_blks >= total_blks)
				break;
		}
		if (cardInfo.cqe_en && cardInfo.cqe_on) {
			ts = clock();
			while (cqhci_get_queued_tasks());
			te = clock();
			if (te > ts)
				rd_time += te - ts;
		}
		printf("Total Read: %d blocks in %ld ms (Average %d MB/s).\r\n",
			    test_blks, rd_time, ((test_blks * 500) / 1024) / rd_time);

		if (cardInfo.cqe_en && cardInfo.cqe_on) {
			cqhci_off(1);
			ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 0, 1);
		}
		else if (cardInfo.cqe_en && !cardInfo.cqe_on) {
			ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 1, 1);
			cqhci_on();
		}
	}

	return 0;
}

void SDC_test_init(void)
{
	write_buf = (uint8_t *)memalign(0x1000, FTSDC021_BUFFER_LENGTH);
	if (write_buf == NULL)
		printf("allocate write buffer failed\r\n");

	read_buf = (uint8_t *)memalign(0x1000, FTSDC021_BUFFER_LENGTH);
	if (read_buf == NULL)
		printf("allocate read buffer failed\r\n");

	check_buf = (uint8_t *)memalign(0x1000, FTSDC021_BUFFER_LENGTH);
	if (check_buf == NULL)
		printf("allocate check buffer failed\r\n");

	printf("write_buf:%p read_buf:%p check_buf:%p\r\n", write_buf, read_buf, check_buf);
}
