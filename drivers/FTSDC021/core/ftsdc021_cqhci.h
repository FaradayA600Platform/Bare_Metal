/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2009/12//	Mike Yeh	 Original FTSDC020 code
 * 2010/4/2	BingJiun-Luo	 FTSDC021 code	
 * -----------------------------------------------------------------------------
 */

#ifndef FTSDC021_CQHCI_H
#define FTSDC021_CQHCI_H

#include "ftsdc021.h"

#define FTSDC021_CQHCI_BASE (FTSDC021_FPGA_BASE + 0x200)

typedef struct {
	uint32_t VER;		// 0x00-0x03
	uint32_t CAP;		// 0x04-0x07
	uint32_t CFG;		// 0x08-0x0B
	uint32_t CTL;		// 0x0C-0x0F
	uint32_t IS;		// 0x10-0x13
	uint32_t ISTE;		// 0x14-0x17
	uint32_t ISGE;		// 0x18-0x1B
	uint32_t IC;		// 0x1C-0x1F
	uint32_t TDLBA;		// 0x20-0x23
	uint32_t TDLBAU;		// 0x24-0x27
	uint32_t TDBR;		// 0x28-0x2B
	uint32_t TCN;		// 0x2C-0x2F
	uint32_t DQS;		// 0x30-0x33
	uint32_t DPT;		// 0x34-0x37
	uint32_t TCLR;		// 0x38-0x3B
	uint32_t Reserved;	// 0x3C-0x3F
	uint32_t SSC1;		// 0x40-0x43
	uint32_t SSC2;		// 0x44-0x47
	uint32_t CRDCT;		// 0x48-0x4B
	uint32_t Reserved1;	// 0x4C-0x4F
	uint32_t RMEM;		// 0x50-0x53
	uint32_t TERRI;		// 0x54-0x57
	uint32_t CRI;		// 0x58-0x5B
	uint32_t CRA;		// 0x5C-0x5F
	uint32_t ARBSEL;	// 0x60-0x63
} ftsdc021_cqhci_reg;

typedef struct {
        uint16_t attr;
        uint16_t blkcnt;
        uint32_t blkaddr;
} CQTaskDesc;

typedef struct {
	CQTaskDesc task;
	Adma2DescTable trans;
} CQSlot;

#ifdef CQHCI_DESC_SIZE_128
#define CQ_TASK_DESC_SIZE 16
#define CQ_TRAN_DESC_SIZE 16
#else
#define CQ_TASK_DESC_SIZE 8
#define CQ_TRAN_DESC_SIZE 8
#endif

/* Max scatter getter numbers */
#define CQ_MAX_SEGS	128

#define CQ_SLOTS	31
#define CQ_SLOT_SIZE	(CQ_TASK_DESC_SIZE + CQ_TRAN_DESC_SIZE)
#define CQ_DESC_SIZE	(CQ_SLOT_SIZE * CQ_SLOTS)
/* Max data size for transfer descriptors of link transfer */
#define CQ_DATA_SIZE	(CQ_TRAN_DESC_SIZE * CQ_MAX_SEGS * (CQ_SLOTS - 1))

#define CQHCI_DCMD		0x00001000
#define CQHCI_TASK_DESC_SZ	0x00000100

#define CQHCI_ENABLE		0x00000001
#define CQHCI_HALT		0x00000001

#define CQHCI_IS_HAC	(1 << 0)	
#define CQHCI_IS_TCC	(1 << 1)
#define CQHCI_IS_RED	(1 << 2)
#define CQHCI_IS_TCL	(1 << 3)

#define CQHCI_IS_MASK (CQHCI_IS_TCC | CQHCI_IS_RED)

/* attribute fields */
#define CQHCI_VALID(x)		(((x) & 1) << 0)
#define CQHCI_END(x)		(((x) & 1) << 1)
#define CQHCI_INT(x)		(((x) & 1) << 2)
#define CQHCI_ACT(x)		(((x) & 0x7) << 3)

/* data command task descriptor fields */
#define CQHCI_FORCED_PROG(x)	(((x) & 1) << 6)
#define CQHCI_CONTEXT(x)	(((x) & 0xF) << 7)
#define CQHCI_DATA_TAG(x)	(((x) & 1) << 11)
#define CQHCI_DATA_DIR(x)	(((x) & 1) << 12)
#define CQHCI_PRIORITY(x)	(((x) & 1) << 13)
#define CQHCI_QBAR(x)		(((x) & 1) << 14)
#define CQHCI_REL_WRITE(x)	(((x) & 1) << 15)
#define CQHCI_BLK_COUNT(x)	(((x) & 0xFFFF) << 16)
#define CQHCI_BLK_ADDR(x)	((((uint64_t)x) & 0xFFFFFFFF) << 32)

/* direct command task descriptor fields */
#define CQHCI_CMD_INDEX(x)              (((x) & 0x3F) << 0)
#define CQHCI_CMD_TIMING(x)             (((x) & 1) << 6)
#define CQHCI_RESP_TYPE(x)              (((x) & 0x3) << 7)

/* interrupt coalescing */
#define CQHCI_IC_ENABLE		(1 << 31)
#define CQHCI_IC_STATUS		(1 << 20)
#define CQHCI_IC_RESET		(1 << 16)
#define CQHCI_IC_ICCTHWEN	(1 << 15)
#define CQHCI_IC_ICCTH(x)	(((x) & 0x1F) << 8)
#define CQHCI_IC_ICTOVALWEN	(1 << 7)
#define CQHCI_IC_ICTOVAL(x)	((x) & 0x7F)

void cqhci_init(void);
void cqhci_enable(void);
void cqhci_disable(uint32_t wait_empty);
void cqhci_on(void);
uint32_t cqhci_get_queued_tasks(void);
#endif
