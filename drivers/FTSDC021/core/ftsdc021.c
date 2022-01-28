/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2009/12//	Mike Yeh	 Original FTSDC020 code
 * 2010/4/2	BingJiun-Luo	 FTSDC021 code	
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021.h"
#include "ftsdc021_cqhci.h"

/* Define to enable pulse latch tuning */
//#define PULSE_LATCH_ENABLE 1

/* This shoud be defined at Capability Register */
#define FTSDC021_BASE_CLOCK  200
#define HZ_25M	25000000
#define HZ_50M	50000000
#define HZ_200M	200000000

volatile ftsdc021_reg *gpReg = (ftsdc021_reg *)FTSDC021_FPGA_BASE;

volatile uint8_t ErrRecover = 0;

/* Default block size is 512 bytes */
uint32_t rd_bl_len = 9;
uint32_t wr_bl_len = 9;
uint32_t sdma_bound_mask;
uint32_t dbg_print = 0;
volatile uint32_t cmd_index;
uint32_t adma2_insert_nop = 0;
uint32_t adma2_use_interrupt = 0;
Infinite_Test infinite_mode;

clock_t timeout_ms;

SDCardInfo cardInfo;
SDHost host;
uint32_t rpmb_key[32];

static Adma2DescTable ADMA2_DESC_TABLE[ADMA2_NUM_OF_LINES];

/* Internal functions */
static uint32_t ftsdc021_autocmd_error_recovery(void);
static uint32_t get_erase_address(uint32_t * start_addr, uint32_t * end_addr,
				uint32_t start, uint32_t block_cnt);

/* External functions */
extern uint8_t *SDC_ShowTransferType(Transfer_Type tType);
extern uint8_t *SDC_ShowTransferSpeed(Bus_Speed speed);

BOOL ftsdc021_fill_adma_desc_table(uint32_t total_data, uint32_t *data_addr,
				   Adma2DescTable *ptr)
{
	uint8_t act, tmp, adma2_random;
	uint32_t byte_cnt, i, ran, bytes_per_line;
	uint8_t *buff;
	Adma2DescTable *tptr;

	if (total_data < 4) {
		printf("Data less than 4 bytes !!\r\n");
		return 1;
	}

	adma2_random = cardInfo.FlowSet.adma2Rand;
	if (!cardInfo.FlowSet.lineBound)
		bytes_per_line = 65536;
	else
		bytes_per_line = cardInfo.FlowSet.lineBound;

	if (!adma2_random) {
		act = ADMA2_TRAN;
	}

	buff = (uint8_t *)data_addr;
	i = 0;
	do {
		/*
		 * Random Mode = 1, we only random the length inside the
		 * descriptor
		 * Random Mode = 2, we random the action and fix length
		 * inside the descriptor
		 * Random Mode = 3, we random the action and length inside
		 * the descriptor
		 */
		if ((adma2_random > 1) && (i < (ADMA2_NUM_OF_LINES - 2))) {
			ran = rand();
			/*
			 * Occupy percentage to prevent too many Noop and 
			 * Reserved
			 */
			tmp = ran & 0xF;
			if (tmp < 8)
				act = ADMA2_TRAN;
			else if (tmp < 13)
				act = ADMA2_LINK;
			else
				act = ADMA2_NOP;
		} else {
			act = ADMA2_TRAN;
		}

		tptr = ptr + i;
		memset(tptr, 0, sizeof(Adma2DescTable));

		switch (act) {
		case ADMA2_TRAN:
			if ((total_data > 256) &&
			    (i < (ADMA2_NUM_OF_LINES - 2))) {
				if (!adma2_random || (adma2_random == 2))
					/* Must be 4 bytes alignment */
					if (total_data < bytes_per_line)
						byte_cnt = total_data;
					else
						byte_cnt = bytes_per_line;
				else
					byte_cnt = (ran % total_data) & 0xfffc;
			} else {
				if (total_data > 0xFFFF) {
					printf(" ERR## ... Not enough "
						    "descriptor to fill.\r\n");
					tptr->attr |= ADMA2_ENTRY_END;
					return 1;
				}
				byte_cnt = total_data;
			}

			if (byte_cnt < 4)	//bad result from randGen()
				byte_cnt = 4;

			tptr->addr = (uint32_t)((uintptr_t)buff);
			tptr->attr = ADMA2_TRAN | ADMA2_ENTRY_VALID;
			tptr->lgth = byte_cnt;

			buff += byte_cnt;
			total_data -= byte_cnt;
			i++;

			break;
		case ADMA2_LINK:
			tmp = ran & 0x7;
			i += tmp;

			if (i > (ADMA2_NUM_OF_LINES - 2))
				i = ADMA2_NUM_OF_LINES - 2;

			tptr->addr = (uint32_t)((uintptr_t)&ptr[i]);
			tptr->attr = ADMA2_LINK | ADMA2_ENTRY_VALID;

			break;
			/* Do not execute this line, go to next line */
		case ADMA2_NOP:
		case ADMA2_SET:
		default:
			tptr->attr = ADMA2_NOP | ADMA2_ENTRY_VALID;
			i++;

			break;
		}
	} while (total_data > 0);

	if (adma2_insert_nop) {
		tptr = ptr + i;
		memset(tptr, 0, sizeof(Adma2DescTable));
	}
	tptr->attr |= (ADMA2_ENTRY_VALID | ADMA2_ENTRY_END | adma2_use_interrupt);

	return 0;
}

static uint32_t ftsdc021_autocmd_error_recovery(void)
{
	uint8_t PCMD;
	uint32_t err;

	printf("Err: ACMD12 !!\r\n");

	/*
	 * Step 1: Check Auto CMD12 not executed status.
	 * Host Controller cannot issue CMD12 to stop multiple
	 * block transfer due to some error.
	 */
	if (cardInfo.autoErr & SDHCI_AUTOCMD12_ERR_NOT_EXECUTED) {
		PCMD = 1;
		/*
		 * Step 2: Wait for Error Interrupt Recovery for CMD_wo_DAT. 
		 */
		err = ftsdc021_ErrorRecovery();
		if (err != ERR_RECOVERABLE)
			return err;
	} else {
		PCMD = 0;
		/* Step 6: Set Software Reset for CMD line (CR) */
		gpReg->SoftRst |= 0x2;
		/* Step 7: Check DR */
		while (gpReg->SoftRst & 0x2) ;
	}

	/* Step 4 & 8: Issue CMD12 */
	if (ftsdc021_send_abort_command())
		return 1;

	if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 1000)) {
		printf("ERR## ... Get Card Status failed(ACMD12 Error "
			    "Recovery) !\r\n");
		return ERR_NON_RECOVERABLE;
	}

	/* Step 5 & 9: Check the result of CMD12 */
	if ((cardInfo.ErrorSts & 0xF) != 0) {
		/* Step 16 */
		printf("non-recoverable Error\r\n");
		return ERR_NON_RECOVERABLE;
	}

	if (PCMD == 1 && !(cardInfo.ErrorSts & 0x10)) {
		/* Step 17 */
		printf("Error has occured in both CMD_wo_DAT, but not in "
			    "the SD memory transfer.\r\n");
	} else {
		/* Step 11 & 14 */
		gpReg->SoftRst |= 0x4;
		/* Step 12 & 15 */
		while (gpReg->SoftRst & 0x4) ;

		/* Step 10: CMD12 not issued */
		if (!(cardInfo.autoErr & SDHCI_AUTOCMD12_ERR_CMD_NOT_ISSUE)) {
			if (PCMD) {
				/* Step 18 */
				printf("Error has occured in CMD_wo_DAT, "
					    "and also in the SD memory "
					    "transfer.\r\n");
			} else {
				/* Step 19 */
				printf("Error did not occur in CMD_wo_DAT,"
					    " but in the SD memory "
					    "transfer.\r\n");
			}
		} else {
			/* Step 20 */
			printf("CMD_wo_DAT has not been issued, and an "
				    "error errored in the SD memory "
				    "transfer.\r\n");
		}
	}

	return ERR_RECOVERABLE;
}

void ftsdc021_print_err_msg(uint32_t errSts)
{
	if (errSts & SDHCI_INTR_ERR_ADMA) {
		/* Must do Abort DMA Operation */
		printf(" INTR: ERR## ... ADMA Error: 0x%x.\r\n",
			    gpReg->ADMAErrSts);
	}

	if (errSts & SDHCI_INTR_ERR_AutoCMD) {
		printf(" INTR: ERR## ... Auto CMD Error.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_CURR_LIMIT) {
		printf(" INTR: ERR## ... Current Limit.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_DATA_ENDBIT) {
		printf(" INTR: ERR## ... Data End Bit.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_DATA_CRC) {
		printf(" INTR: ERR## ... Data CRC.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_DATA_TIMEOUT) {
		printf(" INTR: ERR## ... Data Timeout.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_CMD_INDEX) {
		printf(" INTR: ERR## ... Command Index\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_CMD_ENDBIT) {
		printf(" INTR: ERR## ... Command End Bit.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_CMD_CRC) {
		printf(" INTR: ERR## ... Command CRC.\r\n");
	}

	if (errSts & SDHCI_INTR_ERR_CMD_TIMEOUT) {
		printf(" INTR: ERR## ... Command Timeout.\r\n");
	}
}

uint32_t ftsdc021_ErrorRecovery(void)
{
	/* more than 40us, The max. freq. for SD is 50MHz. */
	uint8_t delayCount = 10;	

	ftsdc021_print_err_msg(cardInfo.ErrorSts);

	/*
	 * Step 8 and Step 9 to save previous error status and 
	 * clear error interrupt signal status. This are already
	 * done at IRQ handler. 
	 */
	if (ErrRecover) {
		/* Step 10: Issue Abort Command (CMD12) */
		ftsdc021_send_abort_command();

		/* Step 11: Check Command Inhibit DAT and CMD */
		while (gpReg->PresentState & (SDHCI_STS_CMD_INHIBIT |
		       SDHCI_STS_CMD_DAT_INHIBIT)) ;

		/* Step 12 */
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_CMD_LINE) {
			printf("Non-recoverable Error:CMD Line Error\r\n");
			return ERR_NON_RECOVERABLE;
		}
		/* Step 13 */
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_DATA_TIMEOUT) {
			printf("Non-recoverable Error:Data Line "
				    "Timeout\r\n");
			return ERR_NON_RECOVERABLE;
		}
		/* Step 14 */
		while (delayCount > 0) {
			delayCount--;
		}
		/* Step 15 */
		if (gpReg->PresentState & SDHCI_STS_DAT_LINE_LEVEL) {
			/* Step 17 */
			printf("Recoverable Error\r\n");
			return ERR_RECOVERABLE;
		} else {
			/* Step 16 */
			printf("Non-recoverable Error\r\n");
			return ERR_NON_RECOVERABLE;
		}
	}

	return ERR_RECOVERABLE;
}

static uint32_t get_erase_address(uint32_t * start_addr, uint32_t * end_addr,
				uint32_t start, uint32_t block_cnt)
{
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		if (((cardInfo.OCR >> 30) & 0x1) == 0) {
			/* Standard Capacity SD memory card */
			*start_addr = start *
				      (1 << cardInfo.CSD_ver_1.WRITE_BL_LEN);
			*end_addr = (start + (block_cnt - 1)) *
				    (1 << cardInfo.CSD_ver_1.WRITE_BL_LEN);
		} else if (((cardInfo.OCR >> 30) & 0x1) == 1) {
			/*
			 * High Capacity SD memory card
			 * Start address isn't modify because the unit of
			 * argument for cmd is block.
			 */
			*start_addr = start;
			*end_addr = start + (block_cnt - 1);
		} else {
			printf("SD:Unknown OCR format\r\n");
			return 1;
		}
	} else if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
		   cardInfo.CardType == MEMORY_eMMC) {
		if (((cardInfo.OCR >> 29) & 0x3) == 2) { /* Sector mode */
			*start_addr = start;
			*end_addr = (start + (block_cnt - 1));
		} else if (((cardInfo.OCR >> 29) & 0x3) == 0) {	/* Byte mode */
			*start_addr = start * FTSDC021_DEF_BLK_LEN;
			*end_addr = *start_addr + 
				    ((block_cnt) * FTSDC021_DEF_BLK_LEN) - 1;
		} else {
			printf("MMC:Unknown OCR format\r\n");
			return 1;
		}
	} else {
		printf("Unknown Card type(Neither MMC nor SD)\r\n");
		return 1;
	}

	return 0;
}

void  ftsdc021_delay(uint32_t ms)
{
	clock_t t0;

	t0 = clock();

	do {
		;
	} while (clock() - t0 < ms);

	return;
}

void ftsdc021_dump_regs(void)
{

	int32_t i, j;
	uint32_t *pd;
	uint32_t data;

	pd = (uint32_t *)((uintptr_t)FTSDC021_FPGA_BASE);
	for (i = 0, j = 0; i < 0x20; i++) {
		if (j == 0)
			printf("0x%08x : ", (uint32_t)((uintptr_t)(pd + i)));

		if (i == 0x8)	//Skip the data port
			data = 0;
		else
			data = pd[i];
		printf("%08X ", data);

		if (++j == 4) {
			printf("\r\n");
			j = 0;
		}
	}

	pd = (uint32_t *)((uintptr_t)(FTSDC021_FPGA_BASE + 0x100));
	for (i = 0, j = 0; i < 0x10; i++) {
		if (j == 0)
			printf("0x%08x : ", (uint32_t)((uintptr_t)(pd + i)));

		data = pd[i];
		printf("%08X ", data);

		if (++j == 4) {
			printf("\r\n");
			j = 0;
		}
	}

	pd = (uint32_t *)((uintptr_t)(FTSDC021_FPGA_BASE + 0x200));
	for (i = 0, j = 0; i < 36; i++) {
		if (j == 0)
			printf("0x%08x : ", (uint32_t)((uintptr_t)(pd + i)));

		data = pd[i];
		printf("%08X ", data);

		if (++j == 4) {
			printf("\r\n");
			j = 0;
		}
	}
}

/* Issue CMD13 to get get card state and wait for input state */
uint32_t ftsdc021_wait_for_state(uint32_t state, uint32_t ms)
{
	clock_t t0;

	if (cardInfo.cqe_on)
		return 0;

	t0 = clock();
	do {
		if ((clock() - t0) > ms) {
			printf(" ERR## ... Card Status (0x%x) not return "
				    "to define(%d) state.\r\n", cardInfo.respLo,
				    state);
			return 1;
		}

		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf(" ERR## ... Send Card Status Failed.\r\n");
			return 1;
		}

		ftsdc021_delay(3);
	} while (((cardInfo.respLo >> 9) & 0xF) != state);

	return 0;
}

uint32_t ftsdc021_read_data_pio(uint32_t * buffer, uint32_t length, clock_t wait_t)
{
	uint32_t trans_sz, len;
	uint16_t mask;
	clock_t t0;
	uint32_t dsize;
	uint8_t *buf8;
	uint16_t *buf16;

	trans_sz = cardInfo.fifo_depth;
	mask = SDHCI_INTR_STS_BUFF_READ_READY;

	dsize = cardInfo.FlowSet.lineBound;
	if (dsize == 1)
		buf8 = (uint8_t *) buffer;
	else if (dsize == 2)
		buf16 = (uint16_t *) buffer;

	while (length) {
		t0 = clock();
		while (!(gpReg->IntrSts & mask)) {
			if (clock() - t0 > wait_t) {
				printf(" ERR## ... Wait Buffer Read Ready "
					    "timeout (%d).\r\n", length);
				return 1;
			}
		}

		/* Clear Interrupt status */
		gpReg->IntrSts = mask;

		len = (length < trans_sz) ? length : trans_sz;
		length -= len;

		while (len) {
			if (dsize == 1) {
				*buf8 = *((volatile uint8_t *)&gpReg->BufData);
				len -= 1;
				buf8++;
			} else if (dsize == 2) {
				*buf16 = *((volatile uint16_t *)&gpReg->BufData);
				len -= 2;
				buf16++;
			} else {
				*buffer = *((volatile uint32_t *)&gpReg->BufData);
				len -= 4;
				buffer++;
			}
		}
	}

	/* Wait for last block to be completed */
	if (infinite_mode && cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {
		/* For synchronous abort in infinite mode transfer */
		cardInfo.cmplMask |= (WAIT_BLOCK_GAP | WAIT_TRANS_COMPLETE);
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
		/* Must read data from fifo until buffer is empty */
		while (gpReg->IntrSts & mask) {
			uint32_t temp;

			gpReg->IntrSts = mask;
			len = trans_sz;
			while (len) {
				temp = *((volatile uint32_t *)&gpReg->BufData);
				len -= 4;
			}
		}
	} else if (infinite_mode) {
		/* For asynchronous abort in infinite mode transfer */
		cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
	}

	return length;
}

uint32_t ftsdc021_write_data_pio(uint32_t *buffer, uint32_t length, clock_t wait_t)
{
	uint32_t trans_sz, len;
	uint16_t mask;
	clock_t t0, t1;
	uint32_t dsize;
	uint8_t *buf8;
	uint16_t *buf16;

	trans_sz = cardInfo.fifo_depth;
	mask = SDHCI_INTR_STS_BUFF_WRITE_READY;

	dsize = cardInfo.FlowSet.lineBound;
	if (dsize == 1)
		buf8 = (uint8_t *) buffer;
	else if (dsize == 2)
		buf16 = (uint16_t *) buffer;

	while (length) {
		t0 = clock();
		while (!(gpReg->IntrSts & mask)) {
			t1 = clock();
			if (t1 - t0 > wait_t) {
				printf(" ERR## ... Wait Buffer Write Ready"
					    " timeout (%d), wait_t (%d)(%d).\r\n", length, wait_t, t1 - t0);
				return 1;
			}
		}

		/* Clear Interrupt status */
		gpReg->IntrSts = mask;
		/* Clear write fifo empty status */
		gpReg->VendorReg7 &= 4;

		len = (length < trans_sz) ? length : trans_sz;
		length -= len;

		while (len) {
			if (dsize == 1) {
				*((volatile uint8_t *)&gpReg->BufData) = *buf8;
				len -= 1;
				buf8++;
			} else if (dsize == 2) {
				*((volatile uint16_t *)&gpReg->BufData) = *buf16;
				len -= 2;
				buf16++;
			} else {
				*((volatile uint32_t *)&gpReg->BufData) = *buffer;
				len -= 4;
				buffer++;
			}
		}
	}

	/* Wait for last block to be completed */
	if (infinite_mode && cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {
		/* For synchronous abort in infinite mode transfer */
		cardInfo.cmplMask |= WAIT_BLOCK_GAP;
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
	} else if (infinite_mode) {
		/* Wait fifo empty */
		t0 = clock();
		while (!(gpReg->VendorReg7 & 4)) {
			if (clock() - t0 > wait_t)
				break;
		}
		gpReg->VendorReg7 &= 4;

		/* For asynchronous abort in infinite mode transfer */
		cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
	}

	return length;
}

uint32_t ftsdc021_transfer_data_sdma(Transfer_Act act, uint32_t *buffer,
				   uint32_t length, uint32_t wait_t)
{
	uint32_t trans_sz, len;
	uint32_t next_addr;
	clock_t t0;

	trans_sz = sdma_bound_mask + 1;

	do {
		t0 = clock();
		/* Make sure SDMA finish before we lacth the next address */
		while (cardInfo.cmplMask) {
			if (cardInfo.ErrorSts)
				return 1;
			if (clock() - t0 > wait_t) {
				printf(" ERR## ... Wait SDMA interrupt "
					    "timeout (%d).\r\n", length);
				ftsdc021_dump_regs();
				return 1;
			}
		}

		next_addr = gpReg->SdmaAddr;
		/* Transfered bytes count */
		len = next_addr - (uint32_t)((uintptr_t)buffer);
		/* Minus the total desired bytes count. SDMA stops at boundary 
		 * but it might already exceed our intended bytes */
		if ((int32_t) (length - len) < 0)
			length = 0;
		else
			length -= len;

		if (!length)
			break;

		/* Boundary Checking */
		if (next_addr & sdma_bound_mask) {
			printf(" ERR## ... SDMA interrupt not at %d "
				    "boundary, addr=0x%08x.\r\n",
				    (sdma_bound_mask + 1), next_addr);
			return 1;
		} else {
			if (dbg_print > 1)
				printf(" SDMA interrupt at "
					    "addr=0x%08x.\r\n", next_addr);
		}

		/* Remaining bytes less than SDMA boundary. 
		 * For finite transfer, Wait for transfer complete interrupt.
		 * Infinite transfer, wait for DMA interrupt. */
		if ((length > trans_sz) ||
		    ((length <= trans_sz) && infinite_mode))
			cardInfo.cmplMask = WAIT_DMA_INTR;
		else 
			cardInfo.cmplMask = WAIT_TRANS_COMPLETE;	
		buffer = (uint32_t *)((uintptr_t)next_addr);
		gpReg->SdmaAddr = (uint32_t)((uintptr_t)buffer);

	} while (1);

	/* Wait for last block to be completed */
	if (infinite_mode && cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {
		/* For synchronous abort in infinite mode transfer */
		cardInfo.cmplMask |= (WAIT_BLOCK_GAP | WAIT_TRANS_COMPLETE);
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
	} else if (infinite_mode) {
		/* For asynchronous abort in infinite mode transfer */
		cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
	}

	return length;
}

/**
 * Return the "residual" number of bytes write.
 * Caller check for this for correctness.
 * Do not use interrupt signal for PIO mode.
 * The unit of length is bytes.
 */
uint32_t ftsdc021_transfer_data(Transfer_Act act, uint32_t *buffer, uint32_t length)
{
	clock_t wait_t;
	clock_t t0;

	if (cardInfo.cqe_on)
		return 0;

	if (cardInfo.FlowSet.UseDMA == PIO) {
		wait_t = timeout_ms * 10;
		if (act == READ)
			length = ftsdc021_read_data_pio(buffer, length, wait_t);
		else
			length = ftsdc021_write_data_pio(buffer, length,
							 wait_t);
	} else if (cardInfo.FlowSet.UseDMA == SDMA) {
		wait_t = timeout_ms * ((sdma_bound_mask + 1) >> 9);
		length = ftsdc021_transfer_data_sdma(act, buffer, length,
						     wait_t);
	} else {
		wait_t = timeout_ms * (length >> rd_bl_len);
		length = 0;

#ifdef FTSDC021_EDMA
		if ((cardInfo.FlowSet.UseDMA == EDMA) && (act == READ)) {
			length = ftsdc021_wait_DMA_done(wait_t);
			if (length != 0) {
				printf(" ERR ## ... DMA error.\r\n");
				goto out;
			}
		}
#endif// FTSDC021_EDMA
	}

	/* Only need to wait transfer complete for DMA */
	t0 = clock();
	while (cardInfo.cmplMask && !cardInfo.ErrorSts) {
		if (clock() - t0 > wait_t) {
			printf(" Wait Transfer Complete Interrupt timeout "
				    "(0x%x, 0x%x, 0x%x).\r\n", length,
				    cardInfo.cmplMask, cardInfo.ErrorSts);
			ftsdc021_dump_regs();
			if (cardInfo.cmplMask)
				length = cardInfo.cmplMask;
			else if (cardInfo.ErrorSts)
				length = cardInfo.ErrorSts;
			goto out;
		}
	}

out:
	if (cardInfo.ErrorSts) {
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_AutoCMD)
			ftsdc021_autocmd_error_recovery();
		else
			ftsdc021_ErrorRecovery();

		return 1;
	}
	return length;
}

uint32_t ftsdc021_prepare_data(uint32_t blk_cnt, uint16_t blk_sz, uint32_t *buff_addr,
			     Transfer_Act act)
{
	uint32_t bound, length;

	gpReg->BlkSize = blk_sz;
	if (!infinite_mode) /* Block Count Reg = Desired Blocks */
		gpReg->BlkCnt = blk_cnt;
	else if (infinite_mode == Infinite_mode_1)
		/* Block Count Reg = 2 * Desired Blocks */
		gpReg->BlkCnt = blk_cnt << 1;
	else if (infinite_mode == Infinite_mode_2) /* Block Count Reg = 0 */
		gpReg->BlkCnt = 0;

	if (gpReg->TxMode & SDHCI_TXMODE_AUTOCMD23_EN) {
		gpReg->SdmaAddr = blk_cnt;

		/* Reliable write flag is set in packed command header block,
		 * not in CMD23) */
		if (cardInfo.doing_packed_cmd)
			gpReg->SdmaAddr |= PACKED_FLAG;
		else if (cardInfo.max_context_id)
			gpReg->SdmaAddr |= (cardInfo.context_id << 25);
		else if (cardInfo.reliable_write_en)
			gpReg->SdmaAddr |= RELIABLE_WRITE_FLAG;
	}

	cardInfo.cmplMask = WAIT_TRANS_COMPLETE;
	if (cardInfo.FlowSet.UseDMA == SDMA) {
		gpReg->SdmaAddr = (uint32_t)((uintptr_t)buff_addr);
		gpReg->BlkSize |= (cardInfo.FlowSet.lineBound << 12);

		length = blk_cnt * blk_sz;
		bound = sdma_bound_mask + 1;
		/*
		 * "Infinite transfer" Or "buff_addr + length cross the SDMA
		 * boundary", Wait for DMA interrupt, no Transfer Complete
		 * interrupt
		 */
		if ((infinite_mode &&
		    (gpReg->TxMode & SDHCI_TXMODE_MULTI_SEL)) ||
		    (((uint32_t)((uintptr_t)buff_addr) + length) >
		     (((uint32_t)((uintptr_t)buff_addr) &
		       ~sdma_bound_mask) + bound)) ) {
			cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
			cardInfo.cmplMask |= WAIT_DMA_INTR;
		} 	
	} else if (cardInfo.FlowSet.UseDMA == ADMA) {
		if (ftsdc021_fill_adma_desc_table((blk_cnt * blk_sz),
						  buff_addr, ADMA2_DESC_TABLE))
			return 1;
		gpReg->ADMAAddr = (uint32_t)((uintptr_t)ADMA2_DESC_TABLE);

		if (adma2_use_interrupt)
			cardInfo.cmplMask |= WAIT_DMA_INTR;

#ifdef FTSDC021_EDMA
	} else if (cardInfo.FlowSet.UseDMA == EDMA) {
		if (act == WRITE)
			ftsdc021_Start_DMA((uint32_t)((uintptr_t)buff_addr),
					   (uint32_t)((uintptr_t)&gpReg->BufData),
					   blk_cnt * blk_sz, 2, 2,
					   cardInfo.fifo_depth, 0, 2);
		else
			ftsdc021_Start_DMA((uint32_t)((uintptr_t)&gpReg->BufData),
					  (uint32_t)((uintptr_t)buff_addr),
					   blk_cnt * blk_sz, 2, 2,
					   cardInfo.fifo_depth, 2, 0);

#endif //FTSDC021_EDMA
	}
	return 0;
}

uint32_t ftsdc021_set_transfer_mode(uint8_t blk_cnt_en, uint8_t auto_cmd, uint8_t dir,
				  uint8_t multi_blk)
{
	uint16_t mode;

	if ((dir != SDHCI_TXMODE_READ_DIRECTION) && (dir != 0)) {
		printf(" ERR## ... Transder Mode, direction value not "
			    "correct.\r\n");
		return 1;
	}

	/*SDMA can not use Auto CMD23 */
	if ((cardInfo.FlowSet.UseDMA == SDMA) && (auto_cmd == 2))
		auto_cmd = 1;

	auto_cmd <<= 2;
	if ((auto_cmd != SDHCI_TXMODE_AUTOCMD12_EN) &&
	    (auto_cmd != SDHCI_TXMODE_AUTOCMD23_EN) && (auto_cmd != 0)) {
		printf(" ERR## ... Transder Mode, auto cmd value not "
			    "correct.\r\n");
		return 1;
	}

	mode = (auto_cmd | dir);

	if ((cardInfo.FlowSet.UseDMA == ADMA) ||
	    (cardInfo.FlowSet.UseDMA == SDMA)) 
		mode |= SDHCI_TXMODE_DMA_EN;

	if (blk_cnt_en)
		mode |= SDHCI_TXMODE_BLKCNT_EN;

	if (multi_blk)
		mode |= SDHCI_TXMODE_MULTI_SEL;

	gpReg->TxMode = mode;

	return 0;
}

/* Send CMD12 to abort transfer. Software reset is needed */
uint32_t ftsdc021_send_abort_command(void)
{
	uint16_t val;
	clock_t t0;

	if (dbg_print > 1)
		printf(" Send abort command ... ");
	cardInfo.ErrorSts = 0;
	cardInfo.cmplMask = (WAIT_CMD_COMPLETE | WAIT_TRANS_COMPLETE);

	gpReg->CmdArgu = 0;

	val = ((SDHCI_STOP_TRANS & 0x3f) << 8) |
	      ((SDHCI_CMD_TYPE_ABORT & 0x3) << 6) |
	      ((SDHCI_CMD_RTYPE_R1BR5B & 0x1F));
	gpReg->CmdReg = val;

	t0 = clock();
	while (cardInfo.cmplMask) {
		/* 1 secs */
		if ((clock() - t0) > 1000) {
			printf(" ERR## ... Abort Command: Wait for INTR "
				    "timeout(0x%x).\r\n", cardInfo.cmplMask);
			return ERR_CMD_TIMEOUT;
		}
	}

	// Reset the Data and Cmd line due to the None-auto CMD12.
	gpReg->SoftRst |= (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT);
	while (gpReg->SoftRst & (SDHCI_SOFTRST_CMD | SDHCI_SOFTRST_DAT)) ;

	if (dbg_print > 1)
		printf("Done.\r\n");

	return cardInfo.ErrorSts;
}

/* Send CMD12 to stop transfer*/
uint32_t ftsdc021_send_stop_command(void)
{
	uint16_t val;
	clock_t t0;

	if (dbg_print > 1)
		printf(" Send Stop command ... ");
	cardInfo.ErrorSts = 0;
	cardInfo.cmplMask = (WAIT_CMD_COMPLETE | WAIT_TRANS_COMPLETE);

	gpReg->CmdArgu = 0;

	val = ((SDHCI_STOP_TRANS & 0x3f) << 8) |
	      ((SDHCI_CMD_TYPE_NORMAL & 0x3) << 6) |
	      ((SDHCI_CMD_RTYPE_R1BR5B & 0x1F));
	gpReg->CmdReg = val;

	t0 = clock();
	while (cardInfo.cmplMask) {
		/* 1 secs */
		if ((clock() - t0) > 1000) {
			printf(" ERR## ... Stop Command: Wait for INTR "
				    "timeout(0x%x).\r\n", cardInfo.cmplMask);
			return ERR_CMD_TIMEOUT;
		}
	}

	if (dbg_print > 1)
		printf("Done.\r\n");

	return cardInfo.ErrorSts;
}

/**
 * Set block count, block size, DMA table address.
 */
uint32_t ftsdc021_send_command(struct mmc_cmd *cmd)
{
	uint16_t val;
	clock_t t0;
	uint32_t err;
	uint8_t wait, dir;
	uint8_t blk_cnt_en, auto_cmd, multi_blk;

	if (cardInfo.cqe_on) {
		return cqhci_send_command(cmd);
	}
	multi_blk = auto_cmd = blk_cnt_en = dir = 0;
	if (cmd->data != NULL) {
		if (cmd->data->blkCnt > 1 || infinite_mode ||
		    cmd->CmdIndex == SDHCI_WRITE_MULTI_BLOCK ||
		    cmd->CmdIndex == SDHCI_READ_MULTI_BLOCK) {
			multi_blk = 1;
			blk_cnt_en = (infinite_mode) ? 0 : 1;
			/* Infinite Mode can not use Auto CMD12/CMD23 */
			auto_cmd = (infinite_mode) ? 0 :
				   cardInfo.FlowSet.autoCmd;

			if (cmd->data->blkCnt > 65535) {
				/*
				 * Block Count register limits the maximum of 
				 * 65535 block transfer. If ADMA2 operation is
				 * less or equal to 65535, Block Count 
				 * register can be used. If ADMA2 larger than
				 * 65535 blocks, use descriptor table as
				 * transfer length.
				 */
				if (cardInfo.FlowSet.UseDMA == ADMA)
					blk_cnt_en = 0;
				else {
					printf(" Non-ADMA transfer can not"
						    " larger than 65535"
						    " blocks.\r\n");
					return 1;
				}
			}
		}

		if (cmd->data->act == READ)
			dir = SDHCI_TXMODE_READ_DIRECTION;

	}

	if (ftsdc021_set_transfer_mode(blk_cnt_en, auto_cmd, dir, multi_blk))
		return 1;

	if (cmd->data != NULL) {
		if (ftsdc021_prepare_data(cmd->data->blkCnt, cmd->data->blkSz,
					  cmd->data->bufAddr, cmd->data->act))
			return 1;
	}

	/* Checking the MMC system spec. version more than 4.0 or newer. */
	if ((cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
	    cardInfo.CardType == MEMORY_eMMC) &&
	    cardInfo.CSD_MMC.SPEC_VERS < 4) {
		if (cmd->CmdIndex == MMC_SWITCH ||
		    cmd->CmdIndex == MMC_SEND_EXT_CSD) {
			printf(" ERR## ... Commmand 6 or 8 is not "
				    "supported in this MMC system spec.\r\n");
			return 1;
		}
	}

	if (gpReg->PresentState & SDHCI_STS_CMD_INHIBIT) {
		printf(" ERR## CMD INHIBIT is one(Index = %d).\r\n",
			    cmd->CmdIndex);
		return 1;
	}

	err = cardInfo.ErrorSts = 0;

	wait = WAIT_CMD_COMPLETE;
	cardInfo.cmplMask |= WAIT_CMD_COMPLETE;
	if (cmd->InhibitDATChk || (cmd->RespType == SDHCI_CMD_RTYPE_R1BR5B)) {
		if (gpReg->PresentState & SDHCI_STS_CMD_DAT_INHIBIT) {
			printf(" ERR## CMD DATA INHIBIT is one.\r\n");
			return 1;
		}

		/* If this is R1B, we need to wait transfer complete here.
		 * Otherwise, wait transfer complete after read/write data.
		 */
		if (cmd->RespType == SDHCI_CMD_RTYPE_R1BR5B) {
			cardInfo.cmplMask |= WAIT_TRANS_COMPLETE;
			wait |= WAIT_TRANS_COMPLETE;
		}

	}

	gpReg->CmdArgu = cmd->Argu;

	val = ((cmd->CmdIndex & 0x3f) << 8) | ((cmd->CmdType & 0x3) << 6) |
	      ((cmd->DataPresent & 0x1) << 5) | (cmd->RespType & 0x1F);

	if ((dbg_print > 1) && (cmd->CmdIndex != 13)) {
		cmd_index = cmd->CmdIndex;

		if (cmd->DataPresent)
			printf("Command Idx:%d, Cmd:0x%x Argu:0x%x, "
				    "Trans:0x%x, Sz:0x%x, cnt:%d.\r\n",
				    cmd->CmdIndex, val, gpReg->CmdArgu,
				    gpReg->TxMode, gpReg->BlkSize,
				    gpReg->BlkCnt);
		else
			printf("Command Idx:%d, Cmd:0x%x Argu:0x%x, "
				    "Trans:0x%x.\r\n", cmd->CmdIndex, val,
				    gpReg->CmdArgu, gpReg->TxMode);
	}

	//start to send command
	gpReg->CmdReg = val;

	if (cmd->CmdIndex == SDHCI_SEND_TUNE_BLOCK ||
	    cmd->CmdIndex == MMC_SEND_TUNE_BLOCK)
		return 0;
 
	t0 = clock();
	while (!cardInfo.ErrorSts && (cardInfo.cmplMask & wait)) {
		/* 1 secs, wait for data timeout interrupt */
		if ((clock() - t0) > 10000) {
			printf(" Wait for Interrupt timeout.\r\n");
			ftsdc021_dump_regs();
			return ERR_CMD_TIMEOUT;
		}
	}

	/* Read Response Data */
	cardInfo.respLo = gpReg->CmdRespLo;
	cardInfo.respHi = gpReg->CmdRespHi;

	if (cardInfo.ErrorSts) {
		err = cardInfo.ErrorSts;

		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_AutoCMD)
			ftsdc021_autocmd_error_recovery();
		else
			ftsdc021_ErrorRecovery();
	}

	return err;
}

/* Read data from card */
uint32_t ftsdc021_card_read(uint32_t startAddr, uint32_t blkcnt, uint8_t *readbuf)
{
	uint32_t ret;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act = READ;
	data.bufAddr = (uint32_t *) readbuf;
	data.blkSz = 1 << rd_bl_len;
	data.blkCnt = blkcnt;
	cmd.data = &data;
	if (blkcnt > 1 || infinite_mode || cardInfo.doing_packed_cmd)
		cmd.CmdIndex = SDHCI_READ_MULTI_BLOCK;
	else
		cmd.CmdIndex = SDHCI_READ_SINGLE_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = cardInfo.block_addr ? startAddr : (startAddr * data.blkSz);
	ret = ftsdc021_send_command(&cmd);
	if (!ret)
		ret = ftsdc021_transfer_data(READ, (uint32_t *)readbuf,
					     (blkcnt << rd_bl_len));

	if (ret) {
		printf(" ERR## ... ftsdc021_card_read failed send "
			    "abort.\r\n");
		/* Abort */
		if (ftsdc021_send_abort_command())
			return 1;
	}
	else if (((blkcnt > 1) && !cardInfo.FlowSet.autoCmd) || infinite_mode) {
		if (cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS)
			gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;

		/* Abort */
		if (ftsdc021_send_abort_command())
			return 1;
	}

	if (cardInfo.respLo & R1_EXCEPTION_EVENT) {
		if (dbg_print > 1)
			printf(" R1_EXCEPTION_EVENT!. respLo=%x\r\n",
				    cardInfo.respLo);
		ftsdc021_mmc_r1_exception();
	}

	return ret;
}

uint32_t ftsdc021_card_write(uint32_t startAddr, uint32_t blkcnt, uint8_t * writebuf)
{
	uint32_t ret;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	data.act = WRITE;
	data.bufAddr = (uint32_t *) writebuf;
	data.blkSz = 1 << wr_bl_len;
	data.blkCnt = blkcnt;
	if (cardInfo.cqe_on && cardInfo.reliable_write_en)
		cmd.cq_flags |= MMC_DATA_REL_WR;
	cmd.data = &data;
	if (blkcnt > 1 || infinite_mode || cardInfo.doing_packed_cmd ||
	    cardInfo.reliable_write_en)
		cmd.CmdIndex = SDHCI_WRITE_MULTI_BLOCK;
	else
		cmd.CmdIndex = SDHCI_WRITE_BLOCK;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = cardInfo.block_addr ? startAddr : (startAddr * data.blkSz);
	ret = ftsdc021_send_command(&cmd);

	if (!ret)
		ret = ftsdc021_transfer_data(WRITE, (uint32_t *) writebuf,
					     (blkcnt << wr_bl_len));

	if (ret) {
		printf(" ERR## ... ftsdc021_card_write failed send "
			    "abort.\r\n");
		/* Abort */
		if (ftsdc021_send_abort_command())
			return 1;
	}
	else if (((blkcnt > 1) && !cardInfo.FlowSet.autoCmd) || infinite_mode) {
		if (cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS)
			gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;

		/* Abort */
		if (ftsdc021_send_abort_command())
			return 1;
	}

	if (cardInfo.respLo & R1_EXCEPTION_EVENT) {
		if (dbg_print > 1)
			printf(" R1_EXCEPTION_EVENT!. respLo=%x\r\n",
				    cardInfo.respLo);
		/* Send header of packed read can not be disturbed */
		if (cardInfo.doing_packed_cmd != PACKED_READ)
			ftsdc021_mmc_r1_exception();
	}

	return ret;
}

uint32_t get_erase_group_size(uint32_t * erase_group_size)
{
	uint32_t grp_size;
	uint32_t grp_mult;

	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		/* CSD version 1.0 */
		if (((cardInfo.CSD_HI >> 54) & 0x3) == 0) {
			if (cardInfo.CSD_ver_1.ERASE_BLK_EN == 1)
				*erase_group_size = 1;
			else
				*erase_group_size =
					cardInfo.CSD_ver_1.SECTOR_SIZE + 1;
		} else if (((cardInfo.CSD_HI >> 54) & 0x3) == 1) {
			/*
			 * CSD version 2.0, ERASE_BLK_EN is fixed to 1. Can
			 * erase in units of 512 bytes
			 */
			*((uint32_t *) (erase_group_size)) = 1;
		} else {
			printf("SD:Unknown CSD format\r\n");
			return 1;
		}
	} else if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
			cardInfo.CardType == MEMORY_eMMC) {
		grp_size = cardInfo.CSD_MMC.ERASE_GRP_SIZE;
		grp_mult = cardInfo.CSD_MMC.ERASE_GRP_MULT;
		*erase_group_size = (grp_size + 1) * (grp_mult + 1);
	} else {
		printf("Unknown Card type(Neither MMC nor SD)\r\n");
		return 1;
	}

	return 0;
}

uint32_t ftsdc021_CheckCardInsert(void)
{
#ifdef EMBEDDED_MMC
	cardInfo.CardInsert = 1;
	cardInfo.CardType = MEMORY_eMMC;
#else
	/* check debouncing */
	if ((gpReg->PresentState & SDHCI_STS_CARD_STABLE)
	    && (gpReg->PresentState & SDHCI_STS_CARD_INSERT)) {
		cardInfo.CardInsert = 1;
	} else {
		cardInfo.CardInsert = 0;
	}
#endif

	return cardInfo.CardInsert;

}

void ftsdc021_SetPower(int16_t power)
{
	uint8_t pwr;

	if (power == (int16_t) - 1) {
		gpReg->PowerCtl = 0;
		goto out;
	}

	/*
	 * Spec says that we should clear the power reg before setting
	 * a new value.
	 */
	gpReg->PowerCtl = 0;

	pwr = SDHCI_POWER_ON;

	switch (power) {
	case 7:
		pwr |= SDHCI_POWER_180;
		break;
	case 17:
	case 18:
		pwr |= SDHCI_POWER_300;
		break;
	case 20:
	case 21:
		pwr |= SDHCI_POWER_330;
		break;
	default:
		printf(" ERR## ... Voltage value not correct.\r\n");
	}

	gpReg->PowerCtl = pwr;

      out:
	host.power = power;

	printf(" Power Control Register: 0x%01x.\r\n", gpReg->PowerCtl);
}

void ftsdc021_SetSDClock(uint32_t clock)
{
	int div, timeout, s_clk;
	uint16_t clk;

	gpReg->ClkCtl = 0;

	if (cardInfo.max_dtr < clock)
		printf(" WARN## ... Set clock(%d Hz) larger than Max "
			    "Rate(%d Hz).\r\n", clock, cardInfo.max_dtr);

	if (clock == 0)
		goto out;

	if (host.max_clk <= clock)
		div =0;
	else {
		for (div = 1; div < 0x3FF; div++) {
			if ((host.max_clk / (2 * div)) <= clock)
				break;
		}
	}

	clk = div << 8;
	clk |= SDHCI_CLKCNTL_INTERNALCLK_EN;
	gpReg->ClkCtl = clk;

	/* Wait max 10 ms */
	timeout = 10;
	while (!(gpReg->ClkCtl & SDHCI_CLKCNTL_INTERNALCLK_STABLE)) {
		if (timeout == 0) {
			printf(" ERR## ... Internal clock never "
				    "estabilised.\r\n");
			return;
		}
		timeout--;
		ftsdc021_delay(1);
	}

	clk |= SDHCI_CLKCNTL_SDCLK_EN;
	gpReg->ClkCtl = clk;

	/* Make sure to use pulse latching when run below 50 MHz */
	/* SDR104 use multiphase DLL latching, others use pulse latching */
	if (!div)
		div = 1;
	else
		div *= 2;

	s_clk = host.max_clk / div;

	if (s_clk < 100000000) {
		/* Clear the setting before */
		gpReg->VendorReg0 &= ~((0x3F << 8) | 0x1);
		gpReg->VendorReg0 |= ( 1 | (0x1 << 8));

		/* Adjust the Pulse Latch Offset if div > 0 */
		if (cardInfo.CardType != SDIO_TYPE_CARD &&
		   cardInfo.CardType != MEMORY_eMMC && cardInfo.CardInsert) {
			if (s_clk > 1000000) {
				ftsdc021_pulselatch_tuning(div);
			}
			else {
				if (div/4 > 16) {
					gpReg->VendorReg0 &= ~(0x3f << 8);
					gpReg->VendorReg0 |= (0x10 << 8);
				}
				else {
					gpReg->VendorReg0 &= ~(0x3f << 8);
					gpReg->VendorReg0 |= ((div/4) << 8);
				}
			}
		}

	} else {
		gpReg->VendorReg0 &= ~1;
	}

out:
	host.clock = clock;

	printf(" Set clock %d Hz, Clock Control Register: 0x%04x.\r\n",
		    clock, gpReg->ClkCtl);
}

uint32_t ftsdc021_set_bus_width(uint8_t width)
{
	uint8_t wdth;

	// Commented by MikeYeh 081127: the BW means Bus width
	/* Setting the Bus width */
	if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
	    cardInfo.CardType == MEMORY_eMMC) {
		if (cardInfo.CSD_MMC.SPEC_VERS < 4 && width != 1) {
			printf(" MMC: Change width isn't allowed for "
				    "version less than MMC4.1.\r\n");
			printf(" MMC: Force the bus width to be 1.\r\n");
			width = 1;
		}

		/**
		 * MMC BUS_WIDTH[183] of Extended CSD. 
		 * 0=1 bit, 1=4bits, 2=8bits.
		 */
		switch (width) {
		case 1:
			wdth = 0;
			break;
		case 4:
			wdth = 1;
			break;
		case 8:
			wdth = 2;
			break;
		default:
			printf(" ERR## ... MMC: Not supported bus witdth "
				    "%d.\r\n", width);
			return 1;
		}

		if (cardInfo.CSD_MMC.SPEC_VERS >= 4) {
			if (ftsdc021_mmc_switch(&cardInfo,
						EXT_CSD_CMD_SET_NORMAL,
						EXT_CSD_BUS_WIDTH, wdth,
						1)) {
				printf(" ERR## ... MMC: Set Bus width "
					    "failed.\r\n");
				return 1;
			}
		}
	} else if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		/*
		 * The Bit2 in SCR shows whether the "4bit bus width(DAT0-3)" is
		 * supported. ACMD6 command, '00'=1bit or '10'=4bit bus.
		 */
		switch (width) {
		case 1:
			if (!(cardInfo.SCR.SD_BUS_WIDTHS &
			      SDHCI_SCR_SUPPORT_1BIT_BUS)) {
				printf(" ERR## ... SD: 1 bit width not "
					    "support by this card.\r\n");
				return 1;
			}
			wdth = SDHCI_1BIT_BUS_WIDTH;
			break;
		case 4:
			if (!(cardInfo.SCR.SD_BUS_WIDTHS &
			      SDHCI_SCR_SUPPORT_4BIT_BUS)) {
				printf(" ERR## ... SD: 4 bit width not "
					    "support by this card.\r\n");
				return 1;
			}
			wdth = SDHCI_4BIT_BUS_WIDTH;
			break;
		default:
			printf(" ERR## ... SD: Not supported bus witdth "
				    "%d.\r\n", width);
			return 1;
		}

		if (ftsdc021_ops_app_set_bus_width(&cardInfo, wdth)) {
			printf(" ERR## ... SD: Set Bus Width %d failed!\r\n",
				    width);
			return 1;
		}

	} else if (cardInfo.CardType == SDIO_TYPE_CARD) {
		ftsdc021_sdio_set_bus_width(&cardInfo, width);
	}

	gpReg->HCReg &= ~(SDHCI_HC_BUS_WIDTH_8BIT | SDHCI_HC_BUS_WIDTH_4BIT);
	switch (width) {
	case 1:
		cardInfo.bus_width = 1;
		break;
	case 4:
		gpReg->HCReg |= SDHCI_HC_BUS_WIDTH_4BIT;
		cardInfo.bus_width = 4;
		break;
	case 8:
		gpReg->HCReg |= SDHCI_HC_BUS_WIDTH_8BIT;
		cardInfo.bus_width = 8;
		break;
	default:
		printf(" Unsupport bus width %d for HW register "
			    "setting.\r\n", width);
		break;
	}

	printf(" Set Bus width %d bit(s), Host Control Register: 0x%x.\r\n",
		    width, gpReg->HCReg);
	return 0;
}

/**
 * 0x0 : Default/SDR12/Normal Speed
 * 0x1 : SDR25/High Speed
 * 0x2 : SDR50
 * 0x3 : SDR104
 * 0x4 : DDR50
 */
uint32_t ftsdc021_set_bus_speed_mode(uint8_t speed)
{
	uint32_t err, i;

	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		/* Version 1.01 Card does not support CMD6 */
		if (cardInfo.SCR.SD_SPEC == 0) {
			speed = 0;
			goto out;
		}
		
		/* Check Group 1 function support */
		printf(" Checking function Group 1 (Bus Speed Mode) ...");
		for (i = 0; i < 5; i++) {
			if (ftsdc021_ops_sd_switch(&cardInfo, 0, 0, i,
			    &cardInfo.SwitchSts[0])) {
				printf(" ERR## ... Problem reading Switch "
					    "function(Bus Speed Mode).\r\n");
				return 1;
			}

			if (cardInfo.SwitchSts[13] & (1 << i)) {
				cardInfo.bs_mode |= (1 << i);
				printf(" %d ", i);
			}
		}
		printf("\r\n");

		if (speed > 4) {
			printf(" ERR## ... Bus Speed Mode value %d "
				    "error.\r\n", speed);
		}

		if (!(gpReg->HostCtrl2 & SDHCI_18V_SIGNAL) && (speed > 1)) {
			printf(" No 1.8V Signaling Can not set speed more "
				    "than 1");
			return 1;
		}

		/* Check function support */
		if (!(cardInfo.bs_mode & (1 << speed))) {
			printf(" ERR## ... %s not support by Card.\r\n",
				    SDC_ShowTransferSpeed(speed));
			return 1;
		}

		/* Change the clock to 400K Hz to prevent 64Bytes status data
		   from DataCRC on some cards.*/
		ftsdc021_SetSDClock(400000);

		err = ftsdc021_ops_sd_switch(&cardInfo, 1, 0, speed,
					     &cardInfo.SwitchSts[0]);
		/* Read it back for confirmation */
		if (err || ((cardInfo.SwitchSts[16] & 0xF) != speed)) {
			printf(" ERR## ... Problem switching(Group 1) card"
				    " into %d mode!\r\n", speed);
			return 1;
		}
	} else if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
		   cardInfo.CardType == MEMORY_eMMC) {
		if (ftsdc021_mmc_set_bus_speed(&cardInfo, speed))
			return 1;
	} else {
		if (ftsdc021_sdio_set_bus_speed(&cardInfo, speed))
			return 1;
	}

out:
	/* SDR mode only valid for 1.8v IO signal */
	if (gpReg->HostCtrl2 & SDHCI_18V_SIGNAL) {
		gpReg->HostCtrl2 &= ~0x7;
		gpReg->HCReg |= 0x4;
		if (speed >= 2 && cardInfo.CardType == MEMORY_eMMC)
			gpReg->HostCtrl2 |= 3;
		else
			gpReg->HostCtrl2 |= speed;

	} else {
		if (speed == 0)
			gpReg->HCReg &= ~(uint8_t) 0x4;
		else if (speed == 1)
			gpReg->HCReg |= 0x4;
		else {
			printf(" No 1.8V Signaling Can not set speed more "
				    "than 1.\r\n");
			return 1;
		}
	}
	cardInfo.speed = speed;

	if (speed < 4) {
		cardInfo.max_dtr = HZ_25M << speed;
		if (speed > 1 && cardInfo.CardType == MEMORY_eMMC)
			cardInfo.max_dtr = HZ_200M;
		if (cardInfo.max_dtr > host.max_clk)
			cardInfo.max_dtr = host.max_clk;
	} else
		cardInfo.max_dtr = HZ_50M;

	ftsdc021_SetSDClock(cardInfo.max_dtr);

	if (speed > 2 || (speed == 2 && cardInfo.CardType == MEMORY_eMMC)) {
		ftsdc021_execute_tuning(31);
	}

	// Make sure the state had been returned to "Transfer State"
	if (cardInfo.CardType == MEMORY_CARD_TYPE_MMC ||
	    cardInfo.CardType == MEMORY_CARD_TYPE_SD ||
	    cardInfo.CardType == MEMORY_eMMC) {
		if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 2000)) {
			printf(" ERR## ... Card can't return to transfer "
				    "state\r\n");
			return 1;
		}
	}

	printf("Set Switch function(Bus Speed Mode = %d), Host Control "
		    "Register: 0x%x, 0x%04x.\r\n",
		    speed, gpReg->HCReg, gpReg->HostCtrl2);

	return 0;
}

uint32_t ftsdc021_set_1v8_signal_voltage(void)
{
	/* Disable sd_1V8_en_fail check */
	gpReg->VendorReg8 &= 0x7fffffff;
	/* Set 1.8V signal enable */
	gpReg->HostCtrl2 |= SDHCI_18V_SIGNAL;

	/* wait 5 ms, but there is a time delay to do function call 
	 * if we put 5, the real delay become 42 ms.
	 */
	ftsdc021_delay(5);

	/* Enable sd_1V8_en_fail check */
	gpReg->VendorReg8 |= 0x80000000;
	ftsdc021_delay(1);

	/* Check 1.8v Signal Enable value */
	if (!(gpReg->HostCtrl2 & SDHCI_18V_SIGNAL))
		return 1;

	return 0;
}

uint32_t ftsdc021_signal_voltage_switch(void)
{
	uint32_t clock;

	/* Send CMD11 VOLATAGE_SWITCH */
	if (ftsdc021_ops_send_voltage_switch()) {
		printf("CMD11 failed !\r\n");
		return 1;
	}

	/* Set SD_CLOCK_ENABLE = 0 */
	clock = host.clock;
	ftsdc021_SetSDClock(0);

	/* DAT[3:0] must become low */
	if (gpReg->PresentState & SDHCI_STS_DAT_LINE_LEVEL) {
		printf(" ERR## ... DAT[3:0] is not zero.\r\n");
		return 1;
	}

	if (ftsdc021_set_1v8_signal_voltage()) {
		printf(" ERR## ... 1.8v Signal Enable Failed.\r\n");
		return 1;
	}

	/* Set SD_CLOCK_ENABLE = 1 */
	ftsdc021_SetSDClock(clock);

	/* wait 1 ms */
	ftsdc021_delay(1);

	/* DAT[3:0] must become high */
	if (!(gpReg->PresentState & SDHCI_STS_DAT_LINE_LEVEL)) {
		printf(" ERR## ... DAT[3:0] is Zero.\r\n");
		gpReg->HostCtrl2 &= ~SDHCI_18V_SIGNAL;
		return 1;
	}

	return 0;
}

#ifdef PULSE_LATCH_ENABLE
uint32_t ftsdc021_pulselatch_tuning(uint32_t div) {

	uint8_t val, *pass_array, *buf;
	uint32_t i, j, up_bound = div;

	pass_array = malloc(up_bound);
	buf = malloc(512);

	memset(pass_array, 1, up_bound);

	// Record
	for (i = 0; i < up_bound; i++) {
		gpReg->VendorReg0 &= ~(0x3f << 8);
		gpReg->VendorReg0 |= (i << 8);

		printf("Try Latch offset:%d\r\n", i);
		if (ftsdc021_card_read(0, 1, buf)) {
			*(pass_array + i) = 0;
		}
	}

	// Find the most proper value to set
	if (up_bound == 1) {
		if (*(pass_array) == 0) {
			printf("WARN## ... Can't find the proper value for"
				    " latching.\r\n");
		}
		val = 0;
	}
	else {
		for (i = 0; i < up_bound; i++) {
			if (*(pass_array + i) == 0 &&
			   *(pass_array + ((i + 1) % up_bound)) == 1) {
				i = i + 1;
				break;
			}
			if (i == up_bound - 1) {
				if (*(pass_array) == 0) {
					printf("WARN## ... Can't find the "
						    "proper Latch offset\r\n");
					goto err;
				}
				else {
					val = (up_bound >> 1);
					goto out;
				}
			}
		}

		for (j = (i + 1); j < ((i + 1) + up_bound) - 1; j++) {
			if (*(pass_array + (j % up_bound)) == 0) {
				j = j - 1;
				break;
			}
		}

		val = (i + j) >> 1;
		if (val >= up_bound)
			val = val % up_bound;
	}
out:
	printf("The most proper Latch offset is %d\r\n", val);
	gpReg->VendorReg0 &= ~(0x3f << 8);
	gpReg->VendorReg0 |= (val << 8);
err:
	free(pass_array);
	free(buf);
	return 0;
}
#else
uint32_t ftsdc021_pulselatch_tuning(uint32_t div) {
	return 0;
}
#endif

uint32_t ftsdc021_execute_tuning(uint32_t try)
{
	uint8_t dma, delay, cnt;
	clock_t t0;

	if (!((gpReg->HostCtrl2 & SDHCI_SDR104) &&
	    (gpReg->HostCtrl2 & SDHCI_18V_SIGNAL))) {
		printf(" ERR## ... Tuning only require for SDR104 "
			    "mode.\r\n");
		return 1;
	}

	/* UHS mode must tune 4 or 8 data lines */
	if (cardInfo.bus_width < 4)
		ftsdc021_set_bus_width(4);
	
	/* Prevent using DMA when do tuning */
	dma = cardInfo.FlowSet.UseDMA;
	cardInfo.FlowSet.UseDMA = PIO;
#ifdef FTSDC021_EDMA
	hndshk = gpReg->DmaHndshk & 1;
	gpReg->DmaHndshk = 0;
#endif

	cnt = 0;
retune:
	delay = 0;

	gpReg->VendorReg3 &= ~((uint32_t)0xff << 24);
	/*
	 * Spec says tuning error is set except during tuning procedure.
	 * Setting crc16_error_thres = 0x1f can avoid this.
	 */
	gpReg->VendorReg3 |= 0x00001f00 | (try << 24);

	gpReg->SoftRst |= SDHCI_SOFTRST_DAT;
	/* Set Execute Tuning at Host Control2 Register */
	gpReg->HostCtrl2 |= SDHCI_EXECUTE_TUNING;

	reset_dll();

	/* Issue CMD19 repeatedly until Execute Tuning is zero.
	 * Abort the loop if reached 40 times or 150 ms
	 */
	do {
		wait_dll_lock();

		if (ftsdc021_ops_send_tune_block(&cardInfo, NULL))
			break;
		/* Wait until Buffer Read Ready */
		t0 = clock();
		do {
			if ((clock() - t0 > timeout_ms)) {
				printf(" ERR##: Tuning wait for BUFFER "
					    "READ READY timeout.\r\n");
				break;
			}
		} while (!(gpReg->IntrSts & SDHCI_INTR_STS_BUFF_READ_READY));

		delay = (gpReg->VendorReg3 >> 16) & 0x1F;
		gpReg->IntrSts &= SDHCI_INTR_STS_BUFF_READ_READY;

	} while (gpReg->HostCtrl2 & SDHCI_EXECUTE_TUNING);

	wait_dll_lock();

	/* Check Sampling Clock Select */
	if (gpReg->HostCtrl2 & SDHCI_SMPL_CLCK_SELECT) {
		/* FPGA only */
		delay = (gpReg->VendorReg3 >> 16) & 0x1F;
		printf("Tuning Complete(0x%08x)(0x%08x), SD Delay : 0x%x.\r\n",
			    gpReg->VendorReg3, gpReg->VendorReg4, delay);

		if (!(gpReg->VendorReg4 & (1 << delay))) {
			printf("SD Delay does not match tuning "
				    "record(0x%08x).\r\n", gpReg->VendorReg4);
			
			if (cnt++ < 3) {
				if (gpReg->VendorReg3 & 0x1f)
					gpReg->VendorReg3--;
				goto retune;
			} else {
				printf("Re-Tuning Failed.\r\n");
				cardInfo.FlowSet.UseDMA = dma;
				return 1;
			}
		}
	} else {
		printf("Tuning Failed(0x%08x)(0x%08x), SD Delay : 0x%x.\r\n",
			    gpReg->VendorReg3, gpReg->VendorReg4, delay);
		if (cnt++ < 3) {
			if (gpReg->VendorReg3 & 0x1f)
				gpReg->VendorReg3--;
			goto retune;
		} else {
			printf("Re-Tuning Failed.\r\n");
			cardInfo.FlowSet.UseDMA = dma;
			return 1;
		}
	}

	cardInfo.FlowSet.UseDMA = dma;
#ifdef FTSDC021_EDMA
	gpReg->DmaHndshk |= hndshk;
#endif

	return 0;
}

void ftsdc021_set_base_clock(uint32_t clk)
{
	host.max_clk = clk * 1000000;
	host.min_clk = host.max_clk / 2046;
}

/** 
 * SDMA: line_bound is Buffer Boundary value.
 * ADMA: line_boud is bytes per descriptor line.
 */
void ftsdc021_set_transfer_type(Transfer_Type type, uint32_t line_bound,
				uint8_t ran_mode)
{
	gpReg->HCReg &= ~(uint8_t) (0x3 << 3);
#ifdef FTSDC021_EDMA
	gpReg->DmaHndshk &= ~1;
#endif

	cardInfo.FlowSet.UseDMA = type;
	cardInfo.FlowSet.lineBound = line_bound;
	cardInfo.FlowSet.adma2Rand = ran_mode;

	if (type == PIO) {
		gpReg->IntrSigEn = SDHCI_INTR_SIGN_EN_PIO;

	} else if (type == SDMA) {
		sdma_bound_mask = (1 << (cardInfo.FlowSet.lineBound + 12)) - 1;
		gpReg->IntrSigEn = SDHCI_INTR_SIGN_EN_SDMA;

	} else if (type == ADMA) {
		gpReg->ADMAAddr = (uint32_t)((uintptr_t)ADMA2_DESC_TABLE);
		gpReg->HCReg |= (uint8_t) SDHCI_HC_USE_ADMA2;
		gpReg->IntrSigEn = SDHCI_INTR_SIGN_EN_ADMA;

#ifdef FTSDC021_EDMA
	} else {
		gpReg->DmaHndshk |= 1;
#endif
	
	}

	gpReg->ErrSigEn = SDHCI_ERR_SIG_EN_ALL;
}

void ftsdc021_HCReset(uint8_t type)
{
	if (type & SDHCI_SOFTRST_ALL)
		ftsdc021_init();
	else
		gpReg->SoftRst = type;
}

uint32_t ftsdc021_card_erase(uint32_t StartBlk, uint32_t BlkCount)
{
	uint32_t erase_start_addr;
	uint32_t erase_end_addr;
	uint32_t err;
	struct mmc_cmd cmd = {0};

	if (cardInfo.CardType != MEMORY_CARD_TYPE_MMC &&
	    cardInfo.CardType != MEMORY_eMMC &&
	    cardInfo.CardType != MEMORY_CARD_TYPE_SD) {
		printf(" ERR## ... This card type doesn't support "
			    "erasing\r\n");
		return 1;
	}

	if (get_erase_address(&erase_start_addr, &erase_end_addr, StartBlk,
			      BlkCount)) {
		printf("Getting the erase start and end address are "
			    "failed\r\n");
		return 1;
	}

        /* If eMMC CQE is on, the erase commands can not be interrupted. */
        if (cardInfo.cqe_on)
                cmd.cq_flags |= MMC_DATA_QBR;

	/* Erase start */
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD)
		cmd.CmdIndex = SDHCI_ERASE_WR_BLK_START;
	else
		cmd.CmdIndex = SDHCI_ERASE_GROUP_START;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.InhibitDATChk = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.Argu = erase_start_addr;
	err = ftsdc021_send_command(&cmd);
	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf(" ERR ... Erase CMD%d failed, arg:0x%x\r\n",
			    cmd.CmdIndex, erase_start_addr);
		return err;
	}

	/* Erase end */
	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD)
		cmd.CmdIndex = SDHCI_ERASE_WR_BLK_END;
	else
		cmd.CmdIndex = SDHCI_ERASE_GROUP_END;
	cmd.Argu = erase_end_addr;
	err = ftsdc021_send_command(&cmd);
	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf(" ERR## ... Erase CMD%d failed, arg:0x%x\r\n",
			    cmd.CmdIndex, erase_end_addr);
		return err;
	}

	/* CMD 38 */
	cmd.CmdIndex = SDHCI_ERASE;
	cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	cmd.InhibitDATChk = 1;
	cmd.Argu = 0;
	err = ftsdc021_send_command(&cmd);
	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf(" ERR## ... Erase CMD%d failed.\r\n", SDHCI_ERASE);
		if (cardInfo.hpi_en)
			err = ftsdc021_mmc_hpi();
		return err;
	}

	return ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000); 
}

/* Discard is similar to Erase. The difference between them
 * is the argument of CMD38. */
uint32_t ftsdc021_card_discard(uint32_t StartBlk, uint32_t BlkCount)
{
	uint32_t erase_start_addr;
	uint32_t erase_end_addr;
	uint32_t err;
	struct mmc_cmd cmd = {0};

	if (cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		printf(" ERR## ... This card type doesn't support discard "
			    "command\r\n");
		return 1;
	}

	if (get_erase_address(&erase_start_addr, &erase_end_addr, StartBlk,
			      BlkCount)) {
		printf("Getting the erase start and end address are "
			    "failed\r\n");
		return 1;
	}

	/* CMD 35 */
	cmd.CmdIndex = SDHCI_ERASE_GROUP_START;
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 0;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 0;
	cmd.Argu = erase_start_addr;
	err = ftsdc021_send_command(&cmd);
	if (dbg_print > 1)
		printf("resp:%x\r\n", cardInfo.respLo);

	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf("Erase command: %d, arg:0x%x failed.\r\n",
			    SDHCI_ERASE_GROUP_START, erase_start_addr);
		return 1;
	}
	/* CMD 36 */
	cmd.CmdIndex = SDHCI_ERASE_GROUP_END;
	cmd.Argu = erase_end_addr;
	err = ftsdc021_send_command(&cmd);
	if (dbg_print > 1)
		printf("resp:%x\r\n", cardInfo.respLo);

	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf("Erase command: %d, arg:0x%x failed.\r\n",
			    SDHCI_ERASE_GROUP_END, erase_end_addr);
		return 1;
	}
	/* CMD 38 */
	cmd.CmdIndex = SDHCI_ERASE;
	cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
	cmd.InhibitDATChk = 1;
	cmd.Argu = DISCARD_CMD_ARG;
	err = ftsdc021_send_command(&cmd);
	if (dbg_print > 1)
		printf("resp:%x\r\n", cardInfo.respLo);

	if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
		printf("discard command: %d failed. Card response=%x\r\n",
			    SDHCI_ERASE, cardInfo.respLo);
		return 1;
	}

	if (ftsdc021_wait_for_state(CUR_STATE_TRAN, 5000))
		return 1;

	return 0;
}

/*
 * Helper function to check if Block Gap Event interrupt happens.
 */
uint32_t ftsdc021_wait_for_block_gap_event(void)
{
	clock_t t0;

	t0 = clock();
	while (!(gpReg->IntrSts & SDHCI_INTR_STS_BLKGAP)) {
		if (clock() - t0 > timeout_ms) {
			printf("No Stop at Block Gap Event.\r\n");
			break;
		}
	};

	t0 = clock();
	while (!(gpReg->IntrSts & SDHCI_INTR_STS_TXR_COMPLETE)) {
		if (clock() - t0 > timeout_ms) {
			printf("No Transfer Complete interrupt.\r\n");
			break;
		}
	};

	if (gpReg->IntrSts & SDHCI_INTR_STS_BLKGAP)
		printf("INTR: Stop at Block Gap Event.\r\n");

	if (gpReg->IntrSts & SDHCI_INTR_STS_TXR_COMPLETE)
		printf("INTR: Transfer Complete.\r\n");

	gpReg->IntrSts &= (SDHCI_INTR_STS_TXR_COMPLETE | SDHCI_INTR_STS_BLKGAP);

	return 0;
}

/*
 * Helper function to check if "Read Tran Active" happens.
 */
uint32_t ftsdc021_wait_for_read_tran_active(void)
{
	clock_t t0;

	t0 = clock();
	while (!(gpReg->PresentState & SDHCI_STS_READ_TRAN_ACT)) {
		if (clock() - t0 > timeout_ms) {
			printf("No Read Tran Act in PresSts.\r\n");
			break;
		}
	};	
	return 0;
}

/*
 * Helper function to check if "Write Tran Active" happens.
 */
uint32_t ftsdc021_wait_for_write_tran_active(void)
{
	clock_t t0;

	t0 = clock();
	while (!(gpReg->PresentState & SDHCI_STS_WRITE_TRAN_ACT)) {
		if (clock() - t0 > timeout_ms) {
			printf("No Write Tran Act in PresSts\r\n");
			break;
		}
	};	
	return 0;
}

/**
 * Implementation note:
 * Stop and continue every 4 blocks for maximum 3 times. If you want to
 * test infinite transfer and abort the transfer to stop, "blkCnt" must
 * be larger than 16 blocks. Otherwise, we will stop at last block.
 */
#define STOP_MAX 3
#define STOP_INTERVAL 4
void ftsdc021_PIO_write_abort_test(uint32_t stop, uint32_t blks_trans,
				    uint32_t blkCnt)
{
	clock_t t0;

	if (stop < STOP_MAX) {
		printf("Request to Stop at Block Gap (%d) ... \r\n",
			    blks_trans);
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
		ftsdc021_wait_for_block_gap_event();
		
		// Clear the "Stop At Block Gap Request" before continuing.
		gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
		// restart transfer
		gpReg->BlkGapCtl |= SDHCI_CONTINUE_REQ;
		ftsdc021_wait_for_write_tran_active();
	} else if (stop == STOP_MAX) {
		if (cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {
			/*
			 * Synchronous Abort
			 * Step1:
			 * Set Stop at Block Gap Request
			*/
			gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
			ftsdc021_wait_for_block_gap_event();
			gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;

			printf("Synchronous abort transfer at block %d ..."
				    " \r\n", blks_trans);
		} else {
			if (blks_trans == blkCnt) {
				/* wait for transfer int. */
				t0 = clock();
				while (!(gpReg->IntrSts &
					 SDHCI_INTR_STS_TXR_COMPLETE)) {
					if (clock() - t0 > timeout_ms) {
						printf("no transfer "
							    "complete "
							    "interrupt.\r\n");
						break;
					}
				}
				gpReg->IntrSts |= SDHCI_INTR_STS_TXR_COMPLETE;
			}	
			/* We need this interrupt when send CMD12 */
			printf("Asynchronous abort transfer at block %d "
				    "... \r\n", blks_trans);
		}
		
		/* Issue Abort Command */
		/* CMD12 */
		gpReg->IntrSigEn |= SDHCI_INTR_STS_TXR_COMPLETE;
		if (ftsdc021_send_abort_command())
			printf("ERR## ...Abort command isn't sent... \r\n");
	}
	return;
}

void ftsdc021_PIO_read_abort_test(uint32_t stop, uint32_t *blks_trans,
				   uint32_t blkCnt, uint8_t **data_buf)
{
	clock_t t0;

	if (stop < STOP_MAX) {
		
		printf("Request to Stop at Block Gap (%d) ... \r\n",
			    *blks_trans); 
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
		/*
		 * Read until BUFFER_READ_READY is zero to cleanup hardware FIFO
		 * after we set the SDHCI_STOP_AT_BLOCK_GAP_REQ in BlkGapCtl.
		 */
		while (gpReg->IntrSts & SDHCI_INTR_STS_BUFF_READ_READY) {
			if (!ftsdc021_transfer_data(READ, (uint32_t *)*data_buf,
						    (1 << rd_bl_len))) {
				*blks_trans++;
				*data_buf += FTSDC021_DEF_BLK_LEN;
			}
		}
		ftsdc021_wait_for_block_gap_event();
			
		// Clear the "Stop At Block Gap Request" before continuing. 
		gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
		// restart transfer
		gpReg->BlkGapCtl |= SDHCI_CONTINUE_REQ;
		ftsdc021_wait_for_read_tran_active();
	} else if (stop == STOP_MAX) {
		if (cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {	
			/*
			 * Synchronous Abort
			 * Step1:
			 * Set Stop at Block Gap Request
			 */
			gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;

			/*
			 * Due to HW design, we must clean up the FIFO,
			 * Otherwise no Transfer Complete Interrupt happen.
			 */
			while (gpReg->IntrSts &
			       SDHCI_INTR_STS_BUFF_READ_READY) {
				if (!ftsdc021_transfer_data(READ,
							    (uint32_t *)*data_buf,
							    (1 << rd_bl_len))) {
					*data_buf += FTSDC021_DEF_BLK_LEN;
				}
			}
			ftsdc021_wait_for_block_gap_event();
			gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
				
			if ((gpReg->IntrSts & SDHCI_INTR_STS_BLKGAP) &&
			    infinite_mode == 0)
				printf("ERR## ... Stop at block gap intr "
					    "can't trigged when stop at the "
					    "last block!\r\n");
			
			printf("Synchronous abort transfer at block %d ..."
				    " \r\n", *blks_trans);
		} else {
			if (*blks_trans == blkCnt) {
				/* wait for transfer int. */
				t0 = clock();
				while (!(gpReg->IntrSts &
					 SDHCI_INTR_STS_TXR_COMPLETE)) {
					if (clock() - t0 > timeout_ms) {
						printf("no transfer "
							    "complete "
							    "interrupt.\r\n");
						break;
					}
				}
				gpReg->IntrSts |= SDHCI_INTR_STS_TXR_COMPLETE;
			}	
			printf("Asynchronous abort transfer at block %d "
				    "... \r\n", *blks_trans);
		}
		
		/* Issue Abort Command */
		/* CMD12 */
		gpReg->IntrSigEn |= SDHCI_INTR_STS_TXR_COMPLETE;
		if (ftsdc021_send_abort_command()) {
			printf("ERR## ...Abort command isn't sent... \r\n");
		}
	}

	return;
}

void ftsdc021_SDMA_abort_test(Transfer_Act act, uint32_t stop,
			      uint32_t blks_trans, uint32_t blkCnt)
{
	clock_t t0;
				
	if (stop < STOP_MAX) {
		gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
		ftsdc021_wait_for_block_gap_event();
		gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;
		// restart transfer
		gpReg->BlkGapCtl |= SDHCI_CONTINUE_REQ;
		if (act == WRITE) {
			ftsdc021_wait_for_write_tran_active();
		} else if (act == READ) {
			ftsdc021_wait_for_read_tran_active();
		}

		t0 = clock();
		while (!(gpReg->IntrSts & SDHCI_INTR_STS_DMA)) {
			if (clock() - t0 > timeout_ms) {
				printf("DMA interrupt timeout.\r\n");
				break;
			}
		}
		gpReg->IntrSts |= SDHCI_INTR_STS_DMA;

		printf("Request to Stop at Block Gap (%d) ... \r\n",
			    blks_trans);

	} else if (stop == STOP_MAX) {
		if (cardInfo.FlowSet.SyncAbt == ABORT_SYNCHRONOUS) {
			gpReg->BlkGapCtl |= SDHCI_STOP_AT_BLOCK_GAP_REQ;
			ftsdc021_wait_for_block_gap_event();
			gpReg->BlkGapCtl &= ~SDHCI_STOP_AT_BLOCK_GAP_REQ;

			if ((gpReg->IntrSts & SDHCI_INTR_STS_BLKGAP) &&
			    infinite_mode == 0)
				printf("ERR## ... Stop at block gap intr "
					    "can't trigged when stop at the "
					    "last block!\r\n");
			
			printf(" Synchronous abort transfer at round %d..."
				    " \r\n", blks_trans);
		} else {
			if (blks_trans == blkCnt) {
				/* wait for transfer int. */
				t0 = clock();
				while (!(gpReg->IntrSts &
					 SDHCI_INTR_STS_TXR_COMPLETE)) {
					if (clock() - t0 > timeout_ms) {
						printf("no transfer "
							    "complete "
							    "interrupt.\r\n");
						break;
					}
				}
				gpReg->IntrSts |= SDHCI_INTR_STS_TXR_COMPLETE;
			}	
			printf("Asynchronous abort transfer at round %d "
				    "... \r\n", blks_trans);
		}

		/* Issue Abort Command */
		/* CMD12 */
		gpReg->IntrSigEn |= SDHCI_INTR_STS_TXR_COMPLETE;
		if (ftsdc021_send_abort_command()) {
			printf("ERR## ...Abort command isn't sent... \r\n");
		}
	}
	return;
}

uint8_t ftsdc021_do_abort_test(uint32_t start_addr, Transfer_Act act,
			     uint32_t bufAddr, uint32_t blkCnt)
{
	uint32_t blks_trans, ret;
	uint32_t stop;
	volatile uint32_t boundary, next_addr;
	uint8_t *data_buf = (uint8_t *)((uintptr_t)bufAddr);
	clock_t t0;
	struct mmc_cmd cmd = {0};
	struct mmc_data data = {0};

	if (cardInfo.FlowSet.UseDMA == ADMA) {
		printf("Can not test abort with ADMA2 mode.\r\n");
		return 1;
	}

	/* CMD16 */
	ret = ftsdc021_ops_set_blocklen(&cardInfo, FTSDC021_DEF_BLK_LEN);
	if (ret) {
		printf("Set block size failed.\r\n");
		return 1;
	}

	/* For convenient test, force blkCnt to multiple of 4 */
	//blkCnt &= ~0x3;

	/**
	 * If block count larger than STOP_MAX + 1 means this transcation
	 * will be terminated by Abort. In other words, transaction never 
	 * reaches last block.
	 */
	infinite_mode = 0;

	data.act = act;
	data.bufAddr = (uint32_t *)data_buf;
	data.blkSz = (act == READ) ? (1 << rd_bl_len) : (1 << wr_bl_len);
	data.blkCnt = blkCnt;
	cmd.data = &data;
	/*
	 * CMD 18, we times the blkCnt with 8 for SDMA transfer that has 4096
	 * bytes aligment. Assume block size is 512 bytes.
	 */
	if (blkCnt > 1 || infinite_mode || cardInfo.doing_packed_cmd) {
		if (act == READ)
			cmd.CmdIndex = SDHCI_READ_MULTI_BLOCK;
		else
			cmd.CmdIndex = SDHCI_WRITE_MULTI_BLOCK;
	}
	else {
		if (act == READ)
			cmd.CmdIndex = SDHCI_READ_SINGLE_BLOCK;
		else
			cmd.CmdIndex = SDHCI_WRITE_BLOCK;
	}
	cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
	cmd.DataPresent = 1;
	cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.InhibitDATChk = 1;
	cmd.Argu = cardInfo.block_addr ? start_addr : (start_addr * data.blkSz);

	if (cardInfo.FlowSet.UseDMA == PIO) {
		data.blkCnt = blkCnt;
		if (ftsdc021_send_command(&cmd)) {
			return 1;
		}
	}
	else if (cardInfo.FlowSet.UseDMA == SDMA) {
		data.blkCnt = blkCnt << 3;
		// The sdma addr must be filled before R/W command is issued.
		if (ftsdc021_send_command(&cmd)) {
			return 1;
		}

		t0 = clock();
		while (!(gpReg->IntrSts & SDHCI_INTR_STS_DMA)) {
			if (clock() - t0 > timeout_ms) {
				printf("DMA interrupt timeout.\r\n");
				break;
			}
		}
		gpReg->IntrSts |= SDHCI_INTR_STS_DMA;
	}

	/* 
	 * Tell the "ftsdc021_transfer_data" not to check transfer complete for
	 * abort test.
	 */
	cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
	
	blks_trans = 1; 
	stop = 0;

	do {
		if (cardInfo.FlowSet.UseDMA == PIO && act == WRITE) {
			ftsdc021_transfer_data(act, (uint32_t *)data_buf,
					       (1 << wr_bl_len));
			data_buf += FTSDC021_DEF_BLK_LEN;

			if ((blks_trans % STOP_INTERVAL) == 0) {
				ftsdc021_PIO_write_abort_test(stop, blks_trans,
							      blkCnt);
				if (stop == STOP_MAX)
					break;
				stop++;
			}

			blks_trans++;
			
		} else if (cardInfo.FlowSet.UseDMA == PIO && act == READ) {
			ftsdc021_transfer_data(act, (uint32_t *)data_buf,
					       (1 << rd_bl_len));
			data_buf += FTSDC021_DEF_BLK_LEN;
			/*
			 * We set the "stop at block gap" bit after the 3, 7,
			 * 11, and 15 block is received. Taking the the "stop
			 * at block gap" is set after the 3 block as an example,
			 * 4, 5, and 6 blocks have been received into ftsdc021,
			 * simutaneously due to the HW design.  
			 */	
			if ((blks_trans % STOP_INTERVAL) == 0) {
				ftsdc021_PIO_read_abort_test(stop, &blks_trans,
							     blkCnt, &data_buf);
				if (stop == STOP_MAX)
					break;
				stop++;
			}
			blks_trans++;
			
		} else if (cardInfo.FlowSet.UseDMA == SDMA) {
		
			boundary = next_addr = gpReg->SdmaAddr;
			gpReg->SdmaAddr = next_addr;
			blks_trans++;
				
			if ((blks_trans % (STOP_INTERVAL)) == 0) {
				ftsdc021_SDMA_abort_test(act, stop, blks_trans,
							 blkCnt);
				if (stop == STOP_MAX) {
					blks_trans = ((blks_trans - 1) << 3);
					next_addr = gpReg->SdmaAddr;
					blks_trans += (next_addr - boundary) /
						      (1 << wr_bl_len);
					break;
				}

				if (stop == STOP_MAX)
					break;
				stop++;
			} else {
				t0 = clock();
				while (!(gpReg->IntrSts & SDHCI_INTR_STS_DMA)) {
					if (clock() - t0 > timeout_ms) {
						printf("DMA interrupt "
							    "timeout.\r\n");
						break;
					}
				}
				gpReg->IntrSts |= SDHCI_INTR_STS_DMA;
			}
		}

	} while (1);


	/* Issue Abort Command */
	/* CMD12 */
	/*
	if (ftsdc021_send_abort_command())
		return 1;
	*/

	/*
	 * Commented by MikeYeh 081205: Checking the state has been changed to
	 * transfer from programming 
	 */
	if (act == WRITE) {
		do {
			if (ftsdc021_ops_send_card_status(&cardInfo)) {
				printf("ERR## ... Get Card Status "
					    "failed(Abort Write) !\r\n");
				return 1;
			}
		} while (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN);
	}

	gpReg->TxMode = 0;
	gpReg->BlkSize = 0;
	gpReg->BlkCnt = 0;

	/*if (cardInfo.FlowSet.UseDMA == SDMA) {
		return (blks_trans << 3);
	}
	else if (cardInfo.FlowSet.UseDMA == PIO) {
		return blks_trans;
	}*/
	return blks_trans;
	
}

/**
 * This test assume SDMA is 4096 bytes boundary. Must change the code for other 
 * value.
 */
uint32_t ftsdc021_abort_test(uint8_t *readbuf, uint8_t *writebuf, uint32_t blk_cnt)
{
	uint32_t tmp, real_blkcnt;
	uint32_t start_addr = 1000;

	tmp = gpReg->IntrSigEn;
	printf("Write\r\n");
	/*
	 * Check Trasfer Complete and SDMA Interrupt by polling in
	 * ftsdc021_do_abort_test
	 */
	gpReg->IntrSigEn &= ~(SDHCI_INTR_STS_TXR_COMPLETE | SDHCI_INTR_STS_DMA |
			    SDHCI_INTR_STS_BLKGAP);
	real_blkcnt = ftsdc021_do_abort_test(start_addr, WRITE,
					     (uint32_t)((uintptr_t)writebuf),
					     blk_cnt);
	gpReg->IntrSigEn = tmp;

	// Read the data for verifying
	memset(readbuf, 0, real_blkcnt * (1 << rd_bl_len));
	if (!ftsdc021_card_read(start_addr, real_blkcnt, readbuf))
		printf("Read success.\r\n");

	// Compare
	if ((real_blkcnt << rd_bl_len) >
	    SDC_data_check(real_blkcnt, (uint32_t *)writebuf,
			   (uint32_t *)readbuf)) {
		printf(", compare failed in abort write.\r\n");
	}
	else {
		printf(", compare success in abort write.\r\n");
	}	
	
	// Write the golden data 
	start_addr = 2000;
	if ( cardInfo.FlowSet.UseDMA == PIO) {
		if (!ftsdc021_card_write(start_addr, blk_cnt, writebuf))
			printf("Wrtie golden success.\r\n");
	}
	else {
		if (!ftsdc021_card_write(start_addr, (blk_cnt << 3), writebuf))
			printf("Wrtie golden success.\r\n");
	}

	printf("Read\r\n");
	/*
	 * Check Trasfer Complete and SDMA Interrupt by polling in
	 * ftsdc021_do_abort_test
	 */
	memset(readbuf, 0, real_blkcnt * (1 << rd_bl_len));
	gpReg->IntrSigEn &= ~(SDHCI_INTR_STS_TXR_COMPLETE | SDHCI_INTR_STS_DMA |
			    SDHCI_INTR_STS_BLKGAP);
	real_blkcnt = ftsdc021_do_abort_test(start_addr, READ,
					     (uint32_t)((uintptr_t)readbuf),
					     blk_cnt);
	gpReg->IntrSigEn = tmp;

	// Compare
	if ((real_blkcnt << rd_bl_len) >
		SDC_data_check(real_blkcnt, (uint32_t *)writebuf,
			       (uint32_t *)readbuf)) {
		printf(", compare failed in abort read.\r\n");
	}
	else {
		printf(", compare success in abort read.\r\n");
	}


	printf("Transfer Type:%d, Abort Type:%d test successfully\r\n",
		    cardInfo.FlowSet.UseDMA, cardInfo.FlowSet.SyncAbt);
	return 0;
}

void ftsdc021_IntrHandler(void *data)
{
	uint16_t sts, i;
	//volatile uint32_t next_addr;

	if (dbg_print > 1)
		printf("INTR:0x%x\r\n", gpReg->IntrSts);


	/* Read Interrupt Status */
	sts = gpReg->IntrSts;

	if (sts & SDHCI_INTR_STS_ERR) {
		printf("\r\nINTR: ERR## ... Error status 0x%08x.\r\n", sts);
		ftsdc021_dump_regs();
	}

	if (cardInfo.cqe_on)
		cqhci_irq(gpReg->ErrSts);

	/* As soon as the command complete, data ready to be read/write.
	 * Buffer Read/Write Ready usually used when PIO mode.
	 * We don't expect to use interrupt here, but polling.
	 * Leave it to read/write data function.
	 */
	sts &= ~(SDHCI_INTR_STS_BUFF_READ_READY |
		 SDHCI_INTR_STS_BUFF_WRITE_READY);

	/* Clear Interrupt Status immediately */
	gpReg->IntrSts &= sts;

	/* Writing 1 to Card Interrupt Status does not clear it.
	 * Instead, disable the Interrupt Status to clear.
	 */
	if (sts & SDHCI_INTR_STS_CARD_INTR) {
		gpReg->IntrEn &= ~SDHCI_INTR_STS_CARD_INTR;
		printf("INTR: Card Interrupt.\r\n");
		gpReg->IntrEn |= SDHCI_INTR_STS_CARD_INTR;
	}

	if (sts & SDHCI_INTR_STS_CARD_INSERT) {
		printf("INTR: Card Insert\r\n");
		cardInfo.CardInsert = 1;
		if (cardInfo.already_init == FALSE) {
			ftsdc021_init();
		} else {
			ftsdc021_SetSDClock(host.min_clk);
			ftsdc021_SetPower(20);
		}
	}
	if (sts & SDHCI_INTR_STS_CARD_REMOVE) {
		printf("INTR: Card Remove\r\n");
		cardInfo.CardInsert = 0;
		cardInfo.already_init = FALSE;

		// Stop to provide clock
		ftsdc021_SetSDClock(0);
		// Stop to provide power
		ftsdc021_SetPower(-1);
	}

	if (sts & SDHCI_INTR_STS_CMD_COMPLETE) {
		cardInfo.cmplMask &= ~WAIT_CMD_COMPLETE;
	}

	if (sts & SDHCI_INTR_STS_TXR_COMPLETE) {
		if (cardInfo.cmplMask & WAIT_TRANS_COMPLETE) {
			cardInfo.cmplMask &= ~WAIT_TRANS_COMPLETE;
		} else {
			printf("\r\nUnexpected interrupt!\r\n");
		}
	}

	if (sts & SDHCI_INTR_STS_BLKGAP) {
		cardInfo.cmplMask &= ~WAIT_BLOCK_GAP;
		printf("$");
	}

	if (sts & SDHCI_INTR_STS_DMA) {
		cardInfo.cmplMask &= ~WAIT_DMA_INTR;
	}

	if (sts & SDHCI_INTR_STS_ERR) {
		cardInfo.ErrorSts = gpReg->ErrSts;

		/* Step 2: Check CMD Line Error */
		/* When only data timeout error occured, sdc021 cannot issue CMD12, CMD13.
		 * Work around: CMD Line is also reset. FTSDC021 version: 1.8 */
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_CMD_LINE) {
			/* Step 3: Software Reset for CMD line */
			printf("INTR: Error reset CMD line\r\n");
			gpReg->SoftRst |= SDHCI_SOFTRST_CMD;
			/* Step 4 */
			while (gpReg->SoftRst & SDHCI_SOFTRST_CMD) ;
		}

		/* Step 5: Check DAT Line Error */
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_DAT_LINE) {
			/* Step 6: Software Reset for DAT line */
			printf("INTR: Error reset DAT line\r\n");
			gpReg->SoftRst |= SDHCI_SOFTRST_DAT;
			/* Step 7 */
			while (gpReg->SoftRst & SDHCI_SOFTRST_DAT) ;
		}
		
		/* Auto CMD Error Status register is reset */
		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_AutoCMD)
			cardInfo.autoErr = gpReg->AutoCMDErr;

		gpReg->ErrSts = cardInfo.ErrorSts;

		if (cardInfo.ErrorSts & SDHCI_INTR_ERR_TUNING) {
			printf(" INTR: ERR## ... Tuning Error.\r\n");
			ftsdc021_execute_tuning(31);
		}

	}
}

uint32_t ftsdc021_select_voltage(uint32_t ocr)
{
	uint16_t bit_pos;

	if (cardInfo.CardType == CARD_TYPE_UNKNOWN) {
		printf("No Supported Card found !\r\n");
		return 0;
	}

	printf("Card OCR value is 0x%08x\r\n", ocr);

	if (ocr & 0x7F) {
		printf("Card claims to support voltages below the defined "
			    "range. These will be ignored.\r\n");
		ocr &= ~0x7F;
	}

	if ((ocr & (1 << 7)) && cardInfo.CardType == MEMORY_CARD_TYPE_SD) {
		printf(" WARN## ... SD card claims to support the "
			    "incompletely defined 'low voltage range'. This "
			    "will be ignored.\r\n");
		ocr &= ~(1 << 7);
	}

	/* Mask off any voltage we do not support */
	ocr &= host.ocr_avail;

#if 1
	/* Select the maximum voltage */
	for (bit_pos = 23; bit_pos > 0; bit_pos--) {
		if (ocr & (1 << bit_pos))
			break;
	}
	bit_pos--;
#else
	/* Select the minimum voltage */
	for (bit_pos = 1; bit_pos < 24; bit_pos++) {
		if (ocr & (1 << bit_pos))
			break;
	}
#endif

	if (bit_pos == 0) {
		printf(" ERR## ... Can not find correct voltage value.\r\n");
		return 0;
	}

	ftsdc021_SetPower(bit_pos);

	ocr &= (3 << bit_pos);

	return ocr;
}

uint32_t ftsdc021_scan_cards(void)
{
	uint32_t resp, err;

	/* It's not necessary to save from error_recovery function until the
	 * card is in transfer state. Because some commands are not supported
	 * in MMC/SD/SDIO in the mix-initiation.
	 * Error_recover function may destoried the initial procedure. */
	ErrRecover = 0;

        if (ftsdc021_ops_go_idle_state(0)) {
                return 1;
        }

	if (!ftsdc021_attach_sdio())
		goto done;
	if (!ftsdc021_attach_sd())
		goto done;
	if (!ftsdc021_attach_mmc())
		goto done;

	return 1;
done:
	/* enable error recovery */
	ErrRecover = 1;

	return 0;
}

uint32_t ftsdc021_init(void)
{
	uint32_t clk;

	printf(" - Host Controller Version: 0x%02x(0x%08x).\r\n",
		    gpReg->HCVer, gpReg->IpVersion);

	memset(&cardInfo, 0, sizeof(SDCardInfo));
	memset(rpmb_key, 0, 32);

	cardInfo.FlowSet.SyncAbt = ABORT_ASYNCHRONOUS;
	cardInfo.FlowSet.Erasing = 0;
	cardInfo.FlowSet.autoCmd = 0;
	cardInfo.hpi_en = 0;
	cardInfo.bkops_en = 0;
	cardInfo.reliable_write_en = 0;
	cardInfo.max_dtr = 400000;

	infinite_mode = 0;

	// Stop to provide clock
	ftsdc021_SetSDClock(0);
	// Stop to provide power
	ftsdc021_SetPower(-1);
	ftsdc021_delay(500);

	/* Reset the controller */
	gpReg->SoftRst = SDHCI_SOFTRST_ALL;
	ftsdc021_delay(500);
	
	clk = (gpReg->CapReg >> 8) & 0xFF;
	if (clk == 0) {
		printf(" ERR## Hardware doesn't specify base clock "
			    "frequency.\r\n");

		/* Controller does not specify the base clock frequency.
		 * Current design base clock  = SD ODC frequency x 2.       
		 */
		/* (gpReg->CapReg >> 8) & 0xFF; */
		clk = FTSDC021_BASE_CLOCK;
	}
	ftsdc021_set_base_clock(clk);

	if (gpReg->CapReg & SDHCI_CAP_VOLTAGE_33V)
		host.ocr_avail = (3 << 20);

	if (gpReg->CapReg & SDHCI_CAP_VOLTAGE_30V)
		host.ocr_avail |= (3 << 17);

	if (gpReg->CapReg & SDHCI_CAP_VOLTAGE_18V)
		host.ocr_avail |= (1 << 7);

	printf(" - Base Clock Frequency: Min %d Hz, Max %d Hz.\r\n",
		    host.min_clk, host.max_clk);
	printf(" - Max Block Length: 0x%x bytes.\r\n",
		    0x200 << ((gpReg->CapReg >> 16) & 0x3));
	printf(" - 8-bit: %s, ADMA2: %s, HS: %s, SDMA: %s, S/R: %s, "
		    "Voltage: 0x%x,SDR50/104,DDR50: 0x%x.\r\n\r\n",
		    ((gpReg->CapReg >> 18) & 1) ? "Yes" : "No",
		    ((gpReg->CapReg >> 19) & 1) ? "Yes" : "No",
		    ((gpReg->CapReg >> 21) & 1) ? "Yes" : "No",
		    ((gpReg->CapReg >> 22) & 1) ? "Yes" : "No",
		    ((gpReg->CapReg >> 23) & 1) ? "Yes" : "No",
		    ((gpReg->CapReg >> 24) & 7), (gpReg->CapReg2 & 7));

	/* Support SDR50, SDR104 and DDR50, we might request wot switch 1.8V IO
	 * signal */
	if (gpReg->CapReg2 & 7) {
		host.ocr_avail |= (1 << 24);
	}

	/* Clock must be < 400KHz for initialization */
	ftsdc021_SetPower(20);
	ftsdc021_SetSDClock(400000);

#ifdef EMBEDDED_MMC
	/* If RST_n_FUNCTION is enabled, this bit must be set to one to finish eMMC
	 * resetting.*/
	gpReg->VendorReg1 |= 0x8;
#endif

	if (gpReg->HWAttributes & 0x1C) {
		/* FIFO use SRAM: 1K, 2K or 4K */
		cardInfo.fifo_depth = 512;
	} else if (gpReg->HWAttributes & 0x2)
		cardInfo.fifo_depth = 16 << 2;
	else if (gpReg->HWAttributes & 0x1)
		cardInfo.fifo_depth = 8 << 2;

	printf(" - FIFO depth %d bytes.\r\n", cardInfo.fifo_depth);

	/* set timeout ctl */
	gpReg->TimeOutCtl = 14;
	timeout_ms = (1000ULL * (1 << (gpReg->TimeOutCtl + 13))) / CLOCKS_PER_SEC;
	printf(" - timeout_ms = %d, CLOCKS_PER_SEC = %d\r\n", timeout_ms, CLOCKS_PER_SEC);

	/* Optionally enable interrupt signal to CPU */
	ftsdc021_setup_interrupt(FTSDC021_FPGA_IRQ, ftsdc021_IntrHandler);

	/* Always enable all interrupt */
	gpReg->IntrEn = SDHCI_INTR_EN_ALL;
	gpReg->ErrEn = SDHCI_ERR_EN_ALL;

	ftsdc021_set_transfer_type(PIO, 4, 0);

	printf(" Checking card ... ");
	if (ftsdc021_CheckCardInsert()) {
		printf("Inserted.\r\n");
		ftsdc021_delay(1);
	} else {
		printf("Not Inserted.\r\n");
	}

	/* Do not require to enable Error Interrupt Signal. 
	 * Bit 15 of Normal Interrupt Status will tell if 
	 * error happens.
	 */

	printf("Init OK\r\n");

	/* Enable stop SD clock event status */
	gpReg->VendorReg8 |= 0x3f;
	cardInfo.already_init = TRUE;

	return 0;
}
