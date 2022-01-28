/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_cqe.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2018/8/29	Sanjin-Liu	 eMMC command queue engine(CQE) support code
 * -----------------------------------------------------------------------------
 */
#include "ftsdc021_cqhci.h"

#define QWAIT
#define RANDOM_TEST
#define DISCARD_TASK_TEST
#define INTR_COALESCING

CQSlot *CQSlots;
volatile ftsdc021_cqhci_reg *cqReg = (ftsdc021_cqhci_reg *)FTSDC021_CQHCI_BASE;
uint32_t queue_tag = 0;
uint32_t queue_wait = 0;
uint32_t wait_count = 0;
uint32_t queue_depth = 16;
uint32_t next_tag = 0;
uint8_t *cq_task_desc_base;
uint8_t *cq_trans_desc_base;
#ifdef DISCARD_TASK_TEST
unsigned long last_discard_time;
#endif

void cqhci_init(void)
{
	cq_task_desc_base = (uint8_t *)memalign(0x400, CQ_DESC_SIZE);
	cq_trans_desc_base = (uint8_t *)memalign(0x400, CQ_DATA_SIZE);
	printf("cq_trans_desc:%p\r\n", cq_trans_desc_base);
}

void cqhci_on(void)
{
	/* Enable irqs */
	cqReg->ISTE = CQHCI_IS_MASK;
	cqReg->ISGE = CQHCI_IS_MASK;

	gpReg->IntrEn = SDHCI_INTR_STS_CQ_INTR;
	gpReg->ErrEn = SDHCI_CQE_INT_ERR_MASK;
	gpReg->IntrSigEn = SDHCI_INTR_STS_CQ_INTR;
	gpReg->ErrSigEn = SDHCI_CQE_INT_ERR_MASK;
	cqReg->CTL = 0;
	/*
	 * Ready Task Arbitration Select
	 * 0: smalles ID task
	 * 1: earlier task
	 */
	cqReg->ARBSEL = 1;
	cardInfo.cqe_on = 1;
	printf("\ncqhci: CQE on\r\n");
}

void cqhci_off(uint32_t wait_empty)
{
	clock_t t0;

	if (wait_empty) {
#ifdef QWAIT
		if (queue_wait) {
			printf("cqhci: QW=0x%08x QT=0x%08x\r\n", queue_wait, queue_tag);
			cqReg->TDBR = queue_wait;
			queue_wait = 0;
			wait_count = 0;
		}
#endif
		t0 = clock();
		while (queue_tag) {
			if (cqReg->TDBR == 0 && cqReg->DPT == 0) {
				queue_tag = 0;
				break;
			}
			if ((clock() - t0) > 10000) { //wait for 10 seconds
				printf("cqhci: wait tasks timeout!\r\n");
				break;
			}
		}
	}

	cqReg->CTL = CQHCI_HALT;

	t0 = clock();
	while (1) {
		if (cqReg->CTL & CQHCI_HALT) {
			printf("\ncqhci: CQE off (QT=0x%08x)\r\n", queue_tag);
			break;
		}
		if ((clock() - t0) > 10000) { //wait for 10 seconds
			printf("\ncqhci: CQE stuck on\r\n");
			break;
		}
	}

	/* Disable irqs */
	cqReg->ISTE = 0;
	cqReg->ISGE = 0;

	gpReg->IntrEn = SDHCI_INTR_EN_ALL;
	gpReg->ErrEn = SDHCI_ERR_EN_ALL;
	gpReg->IntrSigEn = SDHCI_INTR_EN_ALL;
	gpReg->ErrSigEn = SDHCI_ERR_EN_ALL;

	cardInfo.cqe_on = 0;
}

void cqhci_enable(void)
{
	uint32_t cqcfg;

	if (cardInfo.cqe_en) {
		printf("cqhci: CQE is enabled!\r\n");
		return;
	}
	/* For enabling CQE, enabling ADMA mode is needed too. */
	gpReg->HCReg |= (uint8_t) SDHCI_HC_USE_ADMA2;
	/* Set block size to 512 bytes */
	gpReg->BlkSize = 512;
	cardInfo.cqe_wait_task_done = 0;
	cardInfo.FlowSet.UseDMA = ADMA;
	cardInfo.FlowSet.autoCmd = 1;
        cardInfo.FlowSet.lineBound = 0;
        cardInfo.FlowSet.adma2Rand = 0;
	cardInfo.FlowSet.Erasing = 0;
	infinite_mode = 0;
	adma2_insert_nop = 0;
	adma2_use_interrupt = 0;

	cqcfg = cqReg->CFG;
	/* If CQE is enalbed, disable it first. */
	if (cqcfg & CQHCI_ENABLE) {
		cqcfg &= ~CQHCI_ENABLE;
		cqReg->CFG = cqcfg;
	}

	/* Disable irqs */
	cqReg->ISTE = 0;
	cqReg->ISGE = 0;

	/* 128 bit descriptor size is not supported */
	cqcfg &= ~CQHCI_TASK_DESC_SZ;
	cqcfg |= CQHCI_DCMD;
	cqReg->CFG = cqcfg;

	CQSlots = (CQSlot *)cq_task_desc_base;
	cqReg->TDLBA = (uint32_t)(uintptr_t)cq_task_desc_base;

	cqReg->SSC2 = cardInfo.RCA;
        cqcfg |= CQHCI_ENABLE;
	cqReg->CFG = cqcfg;
#ifdef INTR_COALESCING
	cqReg->IC = CQHCI_IC_ENABLE |
		    CQHCI_IC_ICCTHWEN | CQHCI_IC_ICCTH(1) |
		    CQHCI_IC_ICTOVALWEN | CQHCI_IC_ICTOVAL(32);
#endif
	queue_tag = 0;
	queue_depth = cardInfo.EXT_CSD_MMC.CMDQ_DEPTH;
	cardInfo.cqe_en = 1;
	cqhci_on();
#ifdef DISCARD_TASK_TEST
	last_discard_time = clock();
#endif
}

uint32_t error_recovery()
{
	printf("cqhci: Error recovery!\r\n");
	if (cqReg->TDBR) {
		cqReg->CTL |= 0x100;
		while (cqReg->CTL & 0x100) ;
		queue_tag = 0;
#ifdef QWAIT
		queue_wait = 0;
		wait_count = 0;
#endif
	}
	cqhci_disable(0);
	ftsdc021_HCReset(SDHCI_SOFTRST_ALL);
	ftsdc021_scan_cards();
	ftsdc021_set_bus_width(8);
	ftsdc021_set_bus_speed_mode(2);
	ftsdc021_mmc_switch(&cardInfo, EXT_CSD_CMD_SET_NORMAL, 15, 1, 1);
	cqhci_enable();
	return 0;
}

uint32_t cqhci_send_discard(uint32_t task_id)
{
	struct mmc_cmd cmd = {0};
	uint32_t err;
	uint32_t timeout = 10000000;

        /* CMD 48 */
        cmd.CmdIndex = MMC_CMDQ_TASK_MGMT;
        cmd.CmdType = SDHCI_CMD_TYPE_NORMAL;
        //cmd.RespType = SDHCI_CMD_RTYPE_R1BR5B;
        cmd.RespType = SDHCI_CMD_RTYPE_R1R5R6R7;
	cmd.DataPresent = 0;
	cmd.InhibitDATChk = 1;
	if (task_id > 31)
		cmd.Argu = 1;
	else
		cmd.Argu = (task_id << 16) | 2;
        err = ftsdc021_send_command(&cmd);

        if (err || (cardInfo.respLo & SD_STATUS_ERROR_BITS)) {
                printf("Discard Task command: %d, arg:0x%x failed.\r\n",
                            MMC_CMDQ_TASK_MGMT, cmd.Argu);
                return 1;
        }

	do {
		if (gpReg->PresentState & SDHCI_STS_DAT0_LEVEL)
			break;
		timeout--;
	} while (timeout);
	if (timeout == 0) {
		printf("Discard Task command: %d, arg:0x%x timeout.\r\n",
			    MMC_CMDQ_TASK_MGMT, cmd.Argu);
		error_recovery();
	}

	return 0;
}

uint32_t cqhci_discard_all_task(void)
{
	uint32_t err = 0;

	if (dbg_print > 0)
		printf("cqhci: cqhci_discard_all_task\r\n");

	if (cqReg->DPT)
		err = cqhci_send_discard(32);

	if (cqReg->TDBR) {
		cqReg->CTL |= 0x100;
		while (cqReg->CTL & 0x100) ;
		queue_tag = 0;
#ifdef QWAIT
		queue_wait = 0;
		wait_count = 0;
#endif
	}


	return err;
}

uint32_t cqhci_discard_task(uint32_t task_id)
{
	uint32_t err = 0;

	if (dbg_print > 0)
		printf("cqhci: cqhci_discard_task: %d\r\n", task_id);

	if (cqReg->DPT & (1 << task_id)) {
		if (ftsdc021_ops_send_card_status(&cardInfo)) {
			printf(" ERR## ... Send Card Status Failed.\r\n");
			return 1;
		}
		if (dbg_print > 1)
			printf("cqhci: device status is 0x%llx\r\n", cardInfo.respLo);
		if (((cardInfo.respLo >> 9) & 0xF) != CUR_STATE_TRAN) {
			printf(" ERR## ... Card is not in transfer state.\r\n");
			return 1;
		}

		err = cqhci_send_discard(task_id);
		if (!err) {
			cqReg->TCLR = 1 << task_id;
			while (cqReg->TCLR & (1 << task_id)) ;
			queue_tag &= ~(1 << task_id);
		}
	}

	return err;
}

void cqhci_disable(uint32_t wait_empty)
{
#ifdef QWAIT
	if (dbg_print > 0)
		printf("queue_wait:0x%08x, queue_tag:0x%08x", queue_wait, queue_tag);

	if (queue_wait) {
		cqReg->TDBR = queue_wait;
		queue_wait = 0;
		wait_count = 0;
	}
#endif
	if (cardInfo.cqe_on)
		cqhci_off(wait_empty);

        cqReg->CFG &= ~CQHCI_ENABLE;
	cardInfo.cqe_en = 0;
}

void cqhci_discard_task_test(uint32_t ran)
{
	uint32_t pending_tasks = 0, i;

	//Halt CQE
	cqhci_off(0);

	if (dbg_print > 0)
		printf("cqhci: cqhci_discard_task_test TDBR: 0x%x DPT:0x%x DQS:0x%x\r\n",
			    cqReg->TDBR, cqReg->DPT, cqReg->DQS);

	for (i = 0; i < 32; i++) {
		if (cqReg->DPT & (1 << i))
			pending_tasks++;
	}

	if (pending_tasks) {
		if ((ran % pending_tasks) == 0)
			cqhci_discard_all_task();
		else {
			for (i = 0; i < 32, pending_tasks > 1; i++, pending_tasks--)
				if (cqReg->DPT & (1 << i) & ran)
					cqhci_discard_task(i);
		}
	}

	if (dbg_print > 0)
		printf("cqhci: cqhci_discard_task done TDBR: 0x%x DPT:0x%x DQS:0x%x\r\n",
			    cqReg->TDBR, cqReg->DPT, cqReg->DQS);

	cqhci_on();
}

void cqhci_irq(uint16_t int_err)
{
	uint32_t status, comp_status, cmd_error, data_error, tag;

	if (int_err & (SDHCI_INTR_ERR_CMD_INDEX |
		       SDHCI_INTR_ERR_CMD_ENDBIT |
		       SDHCI_INTR_ERR_CMD_CRC |
		       SDHCI_INTR_ERR_CMD_TIMEOUT))
		cmd_error = 1;
	else
		cmd_error = 0;

	if (int_err & (SDHCI_INTR_ERR_DATA_ENDBIT |
		       SDHCI_INTR_ERR_DATA_CRC |
		       SDHCI_INTR_ERR_DATA_TIMEOUT |
		       SDHCI_INTR_ERR_ADMA))
		data_error = 1;
	else
		data_error = 0;

	status = cqReg->IS;
	cqReg->IS = status;

	if (dbg_print > 1)
		printf("cqhci: CQIS:0x%x DQS:0x%x TCN:0x%x TDBR:0x%x DPT:0x%x QT:0x%x\r\n",
			    status, cqReg->DQS, cqReg->TCN, cqReg->TDBR, cqReg->DPT, queue_tag);

	if ((status & CQHCI_IS_RED) || cmd_error || data_error) {
		//Recovery is needed?
		printf("cqhci_irq: status:0x%08x error IRQ status:0x%08x cmd error:%d "
			    "data error:%d TERRI:0x%08x\r\n", status, int_err, cmd_error,
			    data_error, cqReg->TERRI);
		error_recovery();
		return;
	}

#ifdef INTR_COALESCING
	if (dbg_print > 1)
		printf("cqhci: CQIC:0x%x\r\n", cqReg->IC);
	if (cqReg->IC & CQHCI_IC_ENABLE)
		cqReg->IC |= CQHCI_IC_RESET;
#endif

	if (status & CQHCI_IS_TCC) {
                /* read TCN and complete the request */
                comp_status = cqReg->TCN;
		cqReg->TCN = comp_status;

		queue_tag &= ~comp_status;
		if (comp_status & 0x80000000)
			cardInfo.respLo = cqReg->CRDCT;
	}
	if (dbg_print > 1)
		printf("cqhci: TCN:0x%x CMP:0x%x TDBR:0x%x DPT:0x%x QT:0x%x\r\n",
			    cqReg->TCN, comp_status, cqReg->TDBR, cqReg->DPT, queue_tag);

	return;
}

uint32_t cqhci_get_queued_tasks(void)
{
	return queue_tag;
}

/* tag 31 only for DCMD */
uint32_t get_next_tag(void)
{
	uint32_t timeout = 10000000;
	uint32_t ret_tag;

	next_tag = 0;
	do {
		ret_tag = next_tag;
		next_tag++;
		if (next_tag == queue_depth)
			next_tag = 0;
		if (((queue_tag >> ret_tag) & 1) == 0)
			break;
		timeout--;
	} while (timeout);

	if (timeout == 0) {
		printf("Get next tag timeout.\r\n");
		error_recovery();
		ret_tag = 0;
	}

	return ret_tag;
}

static inline uintptr_t get_trans_desc_addr(uint8_t tag)
{
        return (uintptr_t)(cq_trans_desc_base +
                (CQ_MAX_SEGS * tag * CQ_TRAN_DESC_SIZE));
}

void cqhci_prep_task_desc(struct mmc_cmd *cmd, CQTaskDesc *task, uint32_t intr)
{
        uint32_t req_flags = cmd->cq_flags;

	task->attr = CQHCI_VALID(1) | CQHCI_END(1) |
		     CQHCI_INT(intr) | CQHCI_ACT(0x5) |
		     CQHCI_FORCED_PROG(!!(req_flags & MMC_DATA_FORCED_PRG)) |
		     CQHCI_CONTEXT(cardInfo.context_id) |
		     CQHCI_DATA_TAG(!!(req_flags & MMC_DATA_DAT_TAG)) |
		     CQHCI_DATA_DIR(cmd->data->act) |
		     CQHCI_PRIORITY(!!(req_flags & MMC_DATA_PRIO)) |
		     CQHCI_QBAR(!!(req_flags & MMC_DATA_QBR)) |
		     CQHCI_REL_WRITE(!!(req_flags & MMC_DATA_REL_WR));
	task->blkcnt = cmd->data->blkCnt;
	task->blkaddr = cmd->Argu;
}

uint32_t cqhci_prep_tran_desc(struct mmc_cmd *cmd, Adma2DescTable *desc, uint32_t tag)
{
	uint32_t data_length = cmd->data->blkCnt * cmd->data->blkSz;
	uintptr_t trans_desc = get_trans_desc_addr(tag);
	uint32_t len;

	if (data_length > CQ_MAX_SEGS * 0x10000) {
		printf("cqhci: Data length is exceed the max size!\r\n");
		return 1;
	}

	if (data_length <= 0x10000) {
		desc->addr = (uint32_t)(uintptr_t)cmd->data->bufAddr;
		desc->attr = ADMA2_TRAN | ADMA2_ENTRY_END | ADMA2_ENTRY_VALID;
		desc->lgth = data_length;
		return 0;
	}

	/* link descriptor */
	desc->addr = (uint32_t)((uintptr_t)trans_desc);
	desc->attr = ADMA2_LINK | ADMA2_ENTRY_VALID;

	ftsdc021_fill_adma_desc_table(data_length, cmd->data->bufAddr,
				      (Adma2DescTable *)trans_desc);

	return 0;
}

void cqhci_prep_dcmd_desc(struct mmc_cmd *cmd)
{
        uint32_t req_flags = cmd->cq_flags;
	CQTaskDesc *task;
	uint8_t resp_type;
	uint8_t timing;

	if (!cmd->DataPresent) {
		resp_type = 0x0;
		timing = 0x1;
	} else {
		if (cmd->RespType & SDHCI_CMD_RTYPE_R1BR5B) {
			resp_type = 0x3;
			timing = 0x0;
		} else {
			resp_type = 0x2;
			timing = 0x1;
		}
	}

	task = &CQSlots[cmd->tag].task;
	memset(task, 0, sizeof(CQTaskDesc));
	task->attr = CQHCI_VALID(1) | CQHCI_END(1) |
		     CQHCI_INT(1) | CQHCI_ACT(0x5) |
		     CQHCI_QBAR(!!(req_flags & MMC_DATA_QBR));
	task->blkcnt = CQHCI_CMD_INDEX(cmd->CmdIndex) |
		       CQHCI_CMD_TIMING(timing) |
		       CQHCI_RESP_TYPE(resp_type);
	task->blkaddr = cmd->Argu;
	if (dbg_print > 1)
		printf("cqhci: dcmd: desc_addr: %p cmd: %d timing: %d "
			    "resp: %d\r\n", task, cmd->CmdIndex, timing,
			    resp_type);
}

uint32_t cqhci_check_blk_addr(struct mmc_cmd *cmd)
{
	CQTaskDesc *task;
        uint32_t tag = 0, s0, e0, s1, e1;

        s0 = cmd->Argu;
        e0 = s0 + cmd->data->blkCnt - 1;

        if (!queue_tag)
                return 0;

	for (tag = 0; tag < 31; tag++) {
		if (queue_tag & (1 << tag))
			task = &CQSlots[tag].task;
		else
			continue;

                s1 = task->blkaddr;
                e1 = s1 + task->blkcnt - 1;
                if (s1 > e0 || e1 < s0)
                        continue;
                else {
			if (dbg_print > 1)
				printf("cqhci: QT:%x tag:%d s0:%d e0:%d s1:%d e1:%d\r\n",
				queue_tag, tag, s0, e0, s1, e1);
                        return 1;
                }
        }

        return 0;
}

uint32_t cqhci_send_command(struct mmc_cmd *cmd)
{
	uint32_t err;
	CQTaskDesc *task_desc;
	Adma2DescTable *trans_desc;
#ifdef RANDOM_TEST
	uint32_t ran;
#ifdef DISCARD_TASK_TEST
	unsigned long time_pass;
#endif
#endif

	err = 0;

	if (!cardInfo.cqe_en) {
                printf("CQE is not enabled!!\r\n");
                return 1;
        }

	if (!cardInfo.cqe_on) {
                printf("CQE is off!!\r\n");
                return 1;
	}

#ifdef RANDOM_TEST
	ran = (uint64_t) rand();

#ifdef DISCARD_TASK_TEST
	if (queue_tag && ((ran % 10) > 6) && !cardInfo.cqe_wait_task_done) {
		time_pass = clock() - last_discard_time;
		if (time_pass > 300) {
			cqReg->TDBR = queue_wait;
			queue_wait = 0;
			wait_count = 0;
			cqhci_discard_task_test(ran);
			last_discard_time = clock();
		}
	}
#endif
#endif

	if (dbg_print > 1)
		printf("cqhci: send CMD%d\r\n", cmd->CmdIndex);
        if (cmd->data) {
		cmd->tag = get_next_tag();
		task_desc = &CQSlots[cmd->tag].task;
		memset(task_desc, 0, sizeof(CQTaskDesc));

		if (cqhci_check_blk_addr(cmd))
			cmd->cq_flags |= MMC_DATA_QBR;

		if ((ran % 10) == 2 && cmd->data->act == WRITE)
			cmd->cq_flags |= MMC_DATA_FORCED_PRG;

		if ((ran % 10) == 5)
			cmd->cq_flags |= MMC_DATA_PRIO;

		if ((ran % 20) == 8)
			cmd->cq_flags |= MMC_DATA_QBR;

		if (dbg_print > 1)
			printf("cqhci: tag:%d task_desc:%p\r\n", cmd->tag, task_desc);
#ifdef INTR_COALESCING
		if (cmd->data->act == WRITE)
			cqhci_prep_task_desc(cmd, task_desc, 0);
		else
#endif
		cqhci_prep_task_desc(cmd, task_desc, 1);

		if (dbg_print > 1)
			printf("cqhci: tag:%d task_desc:%p data:%llx\r\n",
				    cmd->tag, task_desc, *((uint64_t *)task_desc));

		trans_desc = &CQSlots[cmd->tag].trans;
		memset(trans_desc, 0, sizeof(Adma2DescTable));
                err = cqhci_prep_tran_desc(cmd, trans_desc, cmd->tag);
                if (err) {
                        printf("cqhci: failed to setup tx desc: %d\r\n", err);
                        return err;
                }
#ifdef RANDOM_TEST
		cqReg->SSC1 = 1;
		/* Test for CBC of register CQSSC1 */
		if (cmd->data->blkCnt > 0xf)
			ran = ran & 0xf;
		else
			ran = ran % cmd->data->blkCnt;
		cqReg->SSC1 |= ran << 16;
		/* Test for CIT of register CQSSC1 */
		ran = (uint32_t) rand();
		ran = ran & 0xffff;
		if (ran == 0)
			ran++;
		cqReg->SSC1 |= ran;
#endif
        } else {
		cmd->tag = 31;
		//wait previous DCMD done.
		while ((queue_tag & (1 << cmd->tag)));
		if (cmd->CmdIndex != 0 && cmd->CmdIndex != 12 && cmd->CmdIndex != 13)
			cmd->cq_flags |= MMC_DATA_QBR;
                cqhci_prep_dcmd_desc(cmd);
        }

	if (dbg_print > 0)
		printf("<0x%08x><%d>", cqReg->TDBR, cmd->tag);
	cqReg->ISTE = 0;
	queue_tag |= 1 << cmd->tag;
	cqReg->ISTE = CQHCI_IS_MASK;
#ifdef QWAIT
	if (cmd->cq_flags & MMC_DATA_QBR) {
		cqReg->TDBR = queue_wait;
		queue_wait = 0;
		wait_count = 0;
		if (dbg_print > 0)
			printf("[0x%08x]", cqReg->TDBR);
	}
	queue_wait |= 1 << cmd->tag;
	wait_count++;
	if (!cmd->data || cardInfo.cqe_wait_task_done ||
	    wait_count >= (ran % (queue_depth - 1))) {
		cqReg->TDBR = queue_wait;
		if (dbg_print > 0)
			printf("<0x%08x>", cqReg->TDBR);
		queue_wait = 0;
		wait_count = 0;
	}
#else
	cqReg->TDBR = 1 << cmd->tag;
	if (dbg_print > 0) printf("[<0x%08x><0x%08x>]", cqReg->TDBR, cqReg->DPT);
#endif

	//wait task complete
	if (cardInfo.cqe_wait_task_done) {
		while ((queue_tag & (1 << cmd->tag)));
		cardInfo.cqe_wait_task_done = 0;
		if (cardInfo.ErrorSts) {
			printf("cqhci: error:0x%08x\r\n", cardInfo.ErrorSts);
			ftsdc021_dump_regs();
			err = 1;
		}
#ifdef INTR_COALESCING
		if (CQHCI_IC_ICTOVAL(ran) == 0)
			ran++;
		cqReg->IC = CQHCI_IC_ENABLE |
			    CQHCI_IC_ICCTHWEN | CQHCI_IC_ICCTH(ran) |
			    CQHCI_IC_ICTOVALWEN | CQHCI_IC_ICTOVAL(ran);
#endif
	}

	return err;
}
