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

#ifndef FTSDC021_H
#define FTSDC021_H

#include "../portme_ftsdc021.h"
#include "ftsdc021_mmc.h"

/* For Initial eMMC */
#undef EMBEDDED_MMC
#undef FTSDC021_MMC_CQE

#define	MAXARGS			20

#ifndef	isblank
#define	isblank(ch)	(((ch) == ' ') || ((ch) == '\t'))
#endif

/* Assume MAX read write sectors is 16 MBytes. */
#define FTSDC021_BUFFER_LENGTH	0x1000000

#define FTSDC021_DEF_BLK_LEN	0x200

#define SDHCI_FILESYSTEM_MODE	1

#define SDHCI_INFINITE_TRANSFER	0
#if (SDHCI_INFINITE_TRANSFER)
#define SYNC_ABORT		0
#endif

#define Standard_Capacity	0
#define Normal_Capacity		1
#define High_Capacity		2

#define FIFO_depth_4_words	0
#define FIFO_depth_8_words	1
#define FIFO_depth_16_words	2
#define FIFO_depth_128_words	3
#define FIFO_depth_256_words	4
#define FIFO_depth_512_words	5

#define CARD_TYPE_UNKNOWN	0
#define MEMORY_CARD_TYPE_SD	1
#define MEMORY_CARD_TYPE_MMC	2
#define SDIO_TYPE_CARD		3
#define MEMORY_SDIO_COMBO	4
#define MEMORY_eMMC			5

/* For SD Memory Register (unit of byte) */
#define SCR_LENGTH				8
#define SD_WRITTEN_NUM_LENGTH	4
#define SD_STATUS_LENGTH		64

/* For MMC Memory Register */
#define EXT_CSD_LENGTH		512

typedef struct {
	uint32_t SdmaAddr;	// 0x00-0x03
	uint16_t BlkSize;		// 0x04-0x05
	uint16_t BlkCnt;		// 0x06-0x07
	uint32_t CmdArgu;		// 0x08-0x0B
	uint16_t TxMode;		// 0x0C-0x0D
	uint16_t CmdReg;		// 0x0E-0x0F
	uint64_t CmdRespLo;	// 0x10-0x17
	uint64_t CmdRespHi;	// 0x18-0x1F
	uint32_t BufData;		// 0x20-0x23
	uint32_t PresentState;	// 0x24-0x27
	uint8_t HCReg;		// 0x28
	uint8_t PowerCtl;		// 0x29
	uint8_t BlkGapCtl;	// 0x2A
	uint8_t WakeUpCtl;	// 0x2B
	uint16_t ClkCtl;		// 0x2C-0x2D
	uint8_t TimeOutCtl;	// 0x2E
	uint8_t SoftRst;		// 0x2F
	uint16_t IntrSts;		// 0x30-0x31
	uint16_t ErrSts;		// 0x32-0x33
	uint16_t IntrEn;		// 0x34-0x35
	uint16_t ErrEn;		// 0x36-0x37
	uint16_t IntrSigEn;	// 0x38-0x39
	uint16_t ErrSigEn;	// 0x3A-0x3B
	uint16_t AutoCMDErr;	// 0x3C-0x3D
	uint16_t HostCtrl2;	// 0x3E-0x3F
	uint32_t CapReg;		// 0x40-0x43
	uint32_t CapReg2;		// 0x44-0x47
	uint64_t MaxCurr;		// 0x48-0x4F
	uint16_t CMD12ForceEvt;	// 0x50-0x51
	uint16_t ForceEvt;	// 0x52-0x53
	uint32_t ADMAErrSts;	// 0x54-0x57
	uint64_t ADMAAddr;	// 0x58-0x5F
	/*
	 *  Register offset 0x60 - 0x6F change at FTSDC021
	 */
	uint16_t PresetValInit;	// 0x60-0x61
	uint16_t PresetValDS;	// 0x62-0x63
	uint16_t PresetValHS;	// 0x64-0x65
	uint16_t PresetValSDR12;	// 0x66-0x67
	uint16_t PresetValSDR25;	// 0x68-0x69
	uint16_t PresetValSDR50;	// 0x6A-0x6B
	uint16_t PresetValSDR104;	// 0x6C-0x6D
	uint16_t PresetValDDR50;	// 0x6E-0x6F

	uint32_t Reserved[28];	// 0x70-0xDF
	uint32_t ShareBusCtrl;	// 0xE0-0xE3
	uint32_t Reserved2[6];	// 0xE4-0xFB
	uint16_t SltIntrSts;	// 0xFC-0xFD
	uint16_t HCVer;		// 0xFE-0xFF
	uint32_t VendorReg0;	// 0x100-0x103
	uint32_t VendorReg1;	// 0x104-0x107
	uint32_t VendorReg2;	// 0x108-0x10B
	uint32_t VendorReg3;	// 0x10C-0x10F
	uint32_t VendorReg4;	// 0x110-0x113
	uint32_t VendorReg5;	// 0x114-0x117
	uint32_t VendorReg6;	// 0x118-0x11B
	uint32_t VendorReg7;   	// 0x11C-0x11F
	uint32_t VendorReg8;   	// 0x120-0x124
	uint32_t VendorReg9;	// 0x124-0x127
	uint32_t DmaHndshk;	// 0x128-0x12C
	uint32_t Reserved4[19];	// 0x12C-0x177
	uint32_t HWAttributes;	// 0x178-0x17B
	uint32_t IpVersion;	// 0x17C-0x17F
	uint32_t Ciph_M_Ctl;	// 0x180-0x183
	uint32_t Ciph_M_Sts;	// 0x184-0x187
	uint16_t Ciph_M_Sts_En;	// 0x188-0x189
	uint16_t Ciph_M_Sig_En;	// 0x18A-0x18B
	uint32_t In_Data_Lo;	// 0x18C-0x18F
	uint32_t In_Data_Hi;	// 0x190-0x193
	uint32_t In_Key_Lo;	// 0x194-0x197
	uint32_t In_Key_Hi;	// 0x198-0x19B
	uint32_t Out_Data_Lo;	// 0x19C-0x19F
	uint32_t Out_Data_Hi; 	// 0x1A0-0x1A3
	uint32_t Secr_Table_Port;	// 0x1A4-0x1A7
} ftsdc021_reg;

typedef enum {
	Infinite_no = 0,
	Infinite_mode_1,	/* Block Count Reg = 2 * Desired Blocks */
	Infinite_mode_2		/* Block Count Reg = 0 */
} Infinite_Test;

typedef enum {
	WRITE = 0,
	READ
} Transfer_Act;

typedef enum {
	ADMA = 0,
	SDMA,
	PIO,
	EDMA,
	TRANS_UNKNOWN
} Transfer_Type;

/* For Asyn./ Syn abort*/
typedef enum {
	ABORT_ASYNCHRONOUS = 0,
	ABORT_SYNCHRONOUS,
	ABORT_UNDEFINED
} Abort_Type;

typedef enum {
	CPRM_PROTECT_RW,
	CPRM_FILESYS,
	CPRM_UNKNOWN
} Cprm_Test;

typedef struct {
	Transfer_Type UseDMA;	/* 0: PIO 1: SDMA 2: ADMA */
	uint16_t lineBound;
	uint16_t adma2Rand;
	Abort_Type SyncAbt;	/* 0: Async 1: Sync */
	uint8_t Erasing;		/* 0: No earsing in buruin 1:Include erase testing */
	uint8_t autoCmd;
} FlowInfo;

typedef struct {
	uint32_t Reserved1:5;	/* 508:502 */
	uint32_t SECURED_MODE:1;  /* 509 */
	uint32_t DAT_BUS_WIDTH:2; /* 511:510 */
	uint32_t SD_CARD_TYPE_HI:8; /* 495:488 */
	uint32_t Reserved2:8;	/* 501:496 */
	uint32_t SD_CARD_TYPE_LO:8; /* 487:480 */ 
	uint32_t SIZE_OF_PROTECTED_AREA; /* 479:448 */
	uint8_t SPEED_CLASS;
	uint8_t PERFORMANCE_MOVE;
	uint32_t Reserved3:4;     /* 427:424 */
	uint32_t AU_SIZE:4;       /* 431:428 */
	uint8_t ERASE_SIZE[2];   /* 423:408 */
	uint32_t ERASE_OFFSET:2;  /* 401:400 */
	uint32_t ERASE_TIMEOUT:6; /* 407:402 */
	uint8_t Reserved4[11];
	uint8_t Reserved5[39];
} SDStatus;

typedef struct {
	uint32_t CSD_STRUCTURE:2;
	uint32_t Reserved1:6;
	uint8_t TAAC;
	uint8_t NSAC;
	uint8_t TRAN_SPEED;
	uint32_t CCC:12;
	uint32_t READ_BL_LEN:4;
	uint32_t READ_BL_PARTIAL:1;
	uint32_t WRITE_BLK_MISALIGN:1;
	uint32_t READ_BLK_MISALIGN:1;
	uint32_t DSR_IMP:1;
	uint32_t Reserved2:2;
	uint32_t C_SIZE:12;
	uint32_t VDD_R_CURR_MIN:3;
	uint32_t VDD_R_CURR_MAX:3;
	uint32_t VDD_W_CURR_MIN:3;
	uint32_t VDD_W_CURR_MAX:3;
	uint32_t C_SIZE_MULT:3;
	uint32_t ERASE_BLK_EN:1;
	uint32_t SECTOR_SIZE:7;
	uint32_t WP_GRP_SIZE:7;
	uint32_t WP_GRP_ENABLE:1;
	uint32_t Reserved3:2;
	uint32_t R2W_FACTOR:3;
	uint32_t WRITE_BL_LEN:4;
	uint32_t WRITE_BL_PARTIAL:1;
	uint32_t Reserved4:5;
	uint32_t FILE_FORMAT_GRP:1;
	uint32_t COPY:1;
	uint32_t PERM_WRITE_PROTECT:1;
	uint32_t TMP_WRITE_PROTECT:1;
	uint32_t FILE_FORMAT:2;
	uint32_t Reserver5:2;

} CSD_v1;

typedef struct {
	uint32_t CSD_STRUCTURE:2;
	uint32_t Reserved1:6;
	uint8_t TAAC;
	uint8_t NSAC;
	uint8_t TRAN_SPEED;
	uint32_t CCC:12;
	uint32_t READ_BL_LEN:4;
	uint32_t READ_BL_PARTIAL:1;
	uint32_t WRITE_BLK_MISALIGN:1;
	uint32_t READ_BLK_MISALIGN:1;
	uint32_t DSR_IMP:1;
	uint32_t Reserved2:6;
	uint32_t C_SIZE:22;
	uint32_t Reserved3:1;
	uint32_t ERASE_BLK_EN:1;
	uint32_t SECTOR_SIZE:7;
	uint32_t WP_GRP_SIZE:7;
	uint32_t WP_GRP_ENABLE:1;
	uint32_t Reserved4:2;
	uint32_t R2W_FACTOR:3;
	uint32_t WRITE_BL_LEN:4;
	uint32_t WRITE_BL_PARTIAL:1;
	uint32_t Reserved5:5;
	uint32_t FILE_FORMAT_GRP:1;
	uint32_t COPY:1;
	uint32_t PERM_WRITE_PROTECT:1;
	uint32_t TMP_WRITE_PROTECT:1;
	uint32_t FILE_FORMAT:2;
	uint32_t Reserver6:2;
} CSD_v2;

// The sequence of variable in SD_SCR structure is constrained. 
typedef struct {
	uint32_t SD_SPEC:4;	/* [59:56] */
	uint32_t SCR_STRUCTURE:4;	/* [60:63] */

	uint32_t SD_BUS_WIDTHS:4;	/* [51:48] */
	uint32_t SD_SECURITY:3;	/* [52:54] */
	uint32_t DATA_STAT_AFTER_ERASE:1;	/* [55:55] */

	uint32_t Reserved1:7;	/* [46:40] */
	uint32_t SD_SPEC3:1;	/* [47:47] */

	uint32_t CMD20_SUPPORT:1;	/* [32:32] */
	uint32_t CMD23_SUPPORT:1;	/* [33:33] */
	uint32_t Reserverd2:6;	/* [34:39] */
	uint32_t Reserverd3;	/* [31:0] */

} SD_SCR;

/* for SCR */
#define SDHCI_SCR_SUPPORT_4BIT_BUS	0x4
#define SDHCI_SCR_SUPPORT_1BIT_BUS	0x1

typedef enum {
	SPEED_DEFAULT = 0,	/* or SDR12 in 1.8v IO signalling level */
	SPEED_HIGH,		/* or SDR25 in 1.8v IO signalling level */
	SPEED_SDR50,
	SPEED_SDR104,
	SPEED_DDR50,
	SPEED_RSRV
} Bus_Speed;

/* To indicate which complete we want to wait */
#define WAIT_CMD_COMPLETE   (1 << 0)
#define WAIT_TRANS_COMPLETE (1 << 1)
#define WAIT_BLOCK_GAP	    (1 << 2)
#define WAIT_DMA_INTR	    (1 << 3)

typedef enum {
	NO_PACKED_CMD = 0,
	PACKED_READ,
	PACKED_WRITE
} Packed_CMDs;

typedef struct {
	uint32_t CardInsert;
	FlowInfo FlowSet;
	uint16_t RCA;
	uint16_t DSR;
	SD_SCR SCR;
	uint32_t OCR;
	uint64_t CSD_LO;
	uint64_t CSD_HI;
	CSD_v1 CSD_ver_1;
	CSD_v2 CSD_ver_2;
	uint64_t CID_LO;
	uint64_t CID_HI;
        uint64_t respLo;
        uint64_t respHi;
	uint32_t numOfBlocks;
	uint8_t SwitchSts[64];
	volatile uint16_t ErrorSts;
	volatile uint16_t autoErr;
	volatile uint8_t cmplMask;
	SDStatus sd_status;
	uint16_t bs_mode;		/* Bus Speed Mode */
	uint8_t bus_width;
	uint8_t already_init;
	uint8_t card_init;
	uint8_t block_addr;
	uint32_t max_dtr;
	Bus_Speed speed;
	uint32_t fifo_depth;
	uint32_t signal_vol;

	/* MMC */
	MMC_CSD CSD_MMC;
	MMC_EXT_CSD EXT_CSD_MMC;
	uint32_t numOfBootBlocks; 
	uint8_t hpi_en;
	uint8_t hpi_cmd;
	uint8_t bkops_en;
	uint8_t doing_packed_cmd;
	uint8_t reliable_write_en;
	uint8_t max_context_id;
	uint8_t context_id;
	uint8_t RPMB_access;

	/* SDMA */
	uint8_t u8_sdma_lock;
	uint8_t sdmaIntr;

	uint32_t CardType;

	/* SDIO */
	uint8_t Num_IO_Func;
	uint8_t Memory_Present;

	/* Drive Name */
	uint32_t Drive;

	/* system configurations */
	uint32_t SysFrequency;

	/* CPRM */
	uint32_t Protected_Drive;
	uint32_t CPRM_Init;
	uint32_t Kmu_LO;
	uint32_t Kmu_HI;
	uint32_t AUTO_CBC;	//only used in read/write protected area

	/* eMMC CQE */
	uint32_t cqe_en;
	uint32_t cqe_on;
	uint32_t cqe_wait_task_done;
	uint32_t cqe_queued_tasks;
} SDCardInfo;

typedef struct {
	uint32_t max_clk;
	uint32_t min_clk;
	uint32_t clock;
	uint8_t power;
	uint32_t ocr_avail;
} SDHost;

struct mmc_data {
	Transfer_Act act;
	uint32_t *bufAddr;
	uint32_t blkCnt;
	uint32_t blkSz;
};
/* Extra flags used by CQE */
#define MMC_DATA_QBR		(1 << 10)         /* CQE queue barrier*/
#define MMC_DATA_PRIO		(1 << 11)         /* CQE high priority */
#define MMC_DATA_REL_WR		(1 << 12)         /* Reliable write */
#define MMC_DATA_DAT_TAG	(1 << 13)         /* Tag request */
#define MMC_DATA_FORCED_PRG	(1 << 14)         /* Forced programming */

struct mmc_cmd {
        uint8_t CmdIndex;
        uint8_t CmdType;
        uint8_t RespType;
	uint8_t DataPresent;
	uint8_t InhibitDATChk;
        uint32_t Argu;
	struct mmc_data *data;
/* eMMC CQE tag */
	uint32_t tag;
	uint32_t cq_flags;
};

// Value return from Send Command function
enum {
	ERR_CMD_TIMEOUT = 1,
	ERR_NON_RECOVERABLE,
	ERR_RECOVERABLE
};

#define SDHCI_DBG

/* 0x0C: TxMode */
#define SDHCI_TXMODE_DMA_EN		(1 << 0)
#define SDHCI_TXMODE_BLKCNT_EN		(1 << 1)
#define SDHCI_TXMODE_AUTOCMD12_EN	(1 << 2)
#define SDHCI_TXMODE_AUTOCMD23_EN	(2 << 2)
#define SDHCI_TXMODE_READ_DIRECTION	(1 << 4)
#define SDHCI_TXMODE_WRITE_DIRECTION	(0 << 4)
#define SDHCI_TXMODE_MULTI_SEL		(1 << 5)

/* 0x0E: CmdReg */
/* response type: bit 0 - 4 */
#define SDHCI_CMD_NO_RESPONSE		0x0	// For no response command
#define SDHCI_CMD_RTYPE_R2		0x9	// For R2
#define SDHCI_CMD_RTYPE_R3R4		0x2	// For R3,R4
#define SDHCI_CMD_RTYPE_R1R5R6R7	0x1A	// For R1,R5,R6,R7
#define SDHCI_CMD_RTYPE_R1BR5B		0x1B	// For R1b, R5b

#define SDHCI_CMD_TYPE_NORMAL		0x0
#define SDHCI_CMD_TYPE_SUSPEND		0x1
#define SDHCI_CMD_TYPE_RESUME		0x2
#define SDHCI_CMD_TYPE_ABORT		0x3

#define SDHCI_CMD_DATA_PRESENT		0x1

/* 0x20: Buf data port*/
#define SDHCI_DATA_PORT			0x20

/* 0x24: Present State Register */
#define SDHCI_Pre_State			0x24
#define SDHCI_STS_CMD_INHIBIT		(1 << 0)
#define SDHCI_STS_CMD_DAT_INHIBIT	(1 << 1)
#define SDHCI_STS_DAT_LINE_ACT		(1 << 2)
#define SDHCI_STS_WRITE_TRAN_ACT	(1 << 8)
#define SDHCI_STS_READ_TRAN_ACT		(1 << 9)
#define SDHCI_STS_BUFF_WRITE		(1 << 10)
#define SDHCI_STS_BUFF_READ		(1 << 11)
#define SDHCI_STS_CARD_INSERT		(1 << 16)
#define SDHCI_STS_CARD_STABLE		(1 << 17)
#define SDHCI_STS_DAT_LINE_LEVEL	(0xF << 20)
#define SDHCI_STS_DAT0_LEVEL		(1 << 20)
#define SDHCI_STS_CMD_LINE_LEVEL	(1 << 24)

/* 0x28: HCReg */
#define SDHCI_HC_LED_ON			(1 << 0)
#define SDHCI_HC_BUS_WIDTH_4BIT		(1 << 1)
#define SDHCI_HC_HI_SPEED		(1 << 2)
#define SDHCI_HC_USE_ADMA2		(2 << 3)
#define SDHCI_HC_BUS_WIDTH_8BIT		(1 << 5)
#define SDHCI_HC_CARD_DETECT_TEST	(1 << 6)
#define SDHCI_HC_CARD_DETECT_SIGNAL	(1 << 7)

/* 0x29: */
#define SDHCI_POWER_ON			(1 << 0)
#define SDHCI_POWER_180			(5 << 1)
#define SDHCI_POWER_300			(6 << 1)
#define SDHCI_POWER_330			(7 << 1)

/* 0x2A: BlkGapCtl*/
#define SDHCI_STOP_AT_BLOCK_GAP_REQ 	(1 << 0)
#define SDHCI_CONTINUE_REQ		(1 << 1)
#define SDHCI_READ_WAIT_CTL		(1 << 2)
#define SDHCI_INT_AT_BLOCK_GAP		(1 << 3)

/* 0x2C: ClkCntl */
#define SDHCI_CLKCNTL_INTERNALCLK_EN	(1 << 0)
#define SDHCI_CLKCNTL_INTERNALCLK_STABLE	(1 << 1)
#define SDHCI_CLKCNTL_SDCLK_EN		(1 << 2)

/* 0x2F: SoftRst */
#define SDHCI_SOFTRST_ALL		(1 << 0)
#define SDHCI_SOFTRST_CMD		(1 << 1)
#define SDHCI_SOFTRST_DAT		(1 << 2)

/* 0x30: IntrSts */
#define SDHCI_INTR_State		0x30
#define SDHCI_INTR_STS_ERR		(1 << 15)
#define SDHCI_INTR_STS_CQ_INTR		(1 << 14)
#define SDHCI_INTR_STS_CARD_INTR	(1 << 8)
#define SDHCI_INTR_STS_CARD_REMOVE	(1 << 7)
#define SDHCI_INTR_STS_CARD_INSERT	(1 << 6)
#define SDHCI_INTR_STS_BUFF_READ_READY	(1 << 5)
#define SDHCI_INTR_STS_BUFF_WRITE_READY	(1 << 4)
#define SDHCI_INTR_STS_DMA		(1 << 3)
#define SDHCI_INTR_STS_BLKGAP		(1 << 2)
#define SDHCI_INTR_STS_TXR_COMPLETE	(1 << 1)
#define SDHCI_INTR_STS_CMD_COMPLETE	(1 << 0)

/* 0x32: ErrSts */
#define SDHCI_INTR_ERR_TUNING		(1 << 10)
#define SDHCI_INTR_ERR_ADMA		(1 << 9)
#define SDHCI_INTR_ERR_AutoCMD		(1 << 8)
#define SDHCI_INTR_ERR_CURR_LIMIT	(1 << 7)
#define SDHCI_INTR_ERR_DATA_ENDBIT	(1 << 6)
#define SDHCI_INTR_ERR_DATA_CRC		(1 << 5)
#define SDHCI_INTR_ERR_DATA_TIMEOUT	(1 << 4)
#define SDHCI_INTR_ERR_CMD_INDEX	(1 << 3)
#define SDHCI_INTR_ERR_CMD_ENDBIT	(1 << 2)
#define SDHCI_INTR_ERR_CMD_CRC		(1 << 1)
#define SDHCI_INTR_ERR_CMD_TIMEOUT	(1 << 0)
#define SDHCI_INTR_ERR_CMD_LINE		(SDHCI_INTR_ERR_CMD_INDEX | SDHCI_INTR_ERR_CMD_ENDBIT | SDHCI_INTR_ERR_CMD_CRC | SDHCI_INTR_ERR_CMD_TIMEOUT)
#define SDHCI_INTR_ERR_DAT_LINE		(SDHCI_INTR_ERR_DATA_ENDBIT | SDHCI_INTR_ERR_DATA_CRC | SDHCI_INTR_ERR_DATA_TIMEOUT)

#define SDHCI_CQE_INT_ERR_MASK ( \
        SDHCI_INTR_ERR_ADMA | SDHCI_INTR_ERR_CURR_LIMIT | SDHCI_INTR_ERR_DATA_ENDBIT | \
        SDHCI_INTR_ERR_DATA_CRC | SDHCI_INTR_ERR_DATA_TIMEOUT | SDHCI_INTR_ERR_CMD_INDEX | \
        SDHCI_INTR_ERR_CMD_ENDBIT | SDHCI_INTR_ERR_CMD_CRC | SDHCI_INTR_ERR_CMD_TIMEOUT)

#define SDHCI_CQE_INT_MASK ((SDHCI_CQE_INT_ERR_MASK << 16) | SDHCI_INTR_STS_CQ_INTR)

/* 0x34: IntrEn */
#define SDHCI_INTR_EN_ALL		(0x10FF)

/* 0x36: ErrEn */
#define SDHCI_ERR_EN_ALL		(0xF7FF)

/* 0x38: IntrSigEn */
//#define SDHCI_INTR_SIG_EN_ALL           (0xF0CC)
//#define SDHCI_INTR_SIG_EN             (0x10CC)
#define SDHCI_INTR_SIG_EN (SDHCI_INTR_STS_CARD_REMOVE | SDHCI_INTR_STS_CARD_INSERT | SDHCI_INTR_STS_CMD_COMPLETE | SDHCI_INTR_STS_TXR_COMPLETE)
#define SDHCI_INTR_SIGN_EN_SDMA (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_DMA | SDHCI_INTR_STS_BLKGAP)
#define SDHCI_INTR_SIGN_EN_ADMA (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_DMA)
#define SDHCI_INTR_SIGN_EN_PIO (SDHCI_INTR_SIG_EN | SDHCI_INTR_STS_BLKGAP)

/* 0x3A: ErrSigEn */
//#define SDHCI_ERR_SIG_EN_ALL            (0xF2FF)
#define SDHCI_ERR_SIG_EN_ALL		(0xF7FF)

/* 0x3C: AutoCMD12 Err */
#define SDHCI_AUTOCMD12_ERR_NOT_EXECUTED 	(1 << 0)
#define SDHCI_AUTOCMD12_ERR_TIMEOUT		(1 << 1)
#define SDHCI_AUTOCMD12_ERR_CRC			(1 << 2)
#define SDHCI_AUTOCMD12_ERR_END_BIT		(1 << 3)
#define SDHCI_AUTOCMD12_ERR_INDEX		(1 << 4)
#define SDHCI_AUTOCMD12_ERR_CMD_NOT_ISSUE	(1 << 7)

/* 0x3E: Host Control 2 */
#define SDHCI_HOST_CONTROL2     0x3E
#define  SDHCI_PRESET_VAL_EN    (1 << 15)
#define  SDHCI_ASYNC_INT_EN     (1 << 14)
#define  SDHCI_SMPL_CLCK_SELECT (1 << 7)
#define  SDHCI_EXECUTE_TUNING   (1 << 6)	/* Write 1 Auto clear */
#define  SDHCI_DRV_TYPE_MASK    (3 << 4)
#define  SDHCI_DRV_TYPE_SHIFT   4
#define   SDHCI_DRV_TYPEB	0
#define   SDHCI_DRV_TYPEA	1
#define   SDHCI_DRV_TYPEC       2
#define   SDHCI_DRV_TYPED 	3
#define  SDHCI_18V_SIGNAL       (1 << 3)
#define  SDHCI_UHS_MODE_MASK    (7 << 0)
#define   SDHCI_SDR12           0
#define   SDHCI_SDR25           1
#define   SDHCI_SDR50           2
#define   SDHCI_SDR104          3
#define   SDHCI_DDR50           4

/* 0x40: Capabilities */
#define SDHCI_CAP_VOLTAGE_33V		(1 << 24)
#define SDHCI_CAP_VOLTAGE_30V		(1 << 25)
#define SDHCI_CAP_VOLTAGE_18V		(1 << 26)
#define SDHCI_CAP_FIFO_DEPTH_16BYTE	(0 << 29)
#define SDHCI_CAP_FIFO_DEPTH_32BYTE	(1 << 29)
#define SDHCI_CAP_FIFO_DEPTH_64BYTE	(2 << 29)
#define SDHCI_CAP_FIFO_DEPTH_512BYTE	(3 << 29)
#define SDHCI_CAP_FIFO_DEPTH_1024BYTE	(4 << 29)
#define SDHCI_CAP_FIFO_DEPTH_2048BYTE	(5 << 29)

/*BingJiun: 0x44 - 0x47 */
#define  SDHCI_SUPPORT_SDR50    	(1 << 0)
#define  SDHCI_SUPPORT_SDR104   	(1 << 1)
#define  SDHCI_SUPPORT_DDR50    	(1 << 2)
#define  SDHCI_SUPPORT_DRV_TYPEA        (1 << 4)
#define  SDHCI_SUPPORT_DRV_TYPEC        (1 << 5)
#define  SDHCI_SUPPORT_DRV_TYPED        (1 << 6)
#define  SDHCI_RETUNING_TIME_MASK       0xF
#define  SDHCI_RETUNING_TIME_SHIFT      8
#define  SDHCI_SDR50_TUNING     	(1 << 13)
#define  SDCHI_RETUNING_MODE_MASK     	0x3
#define  SDHCI_RETUNING_MODE_SHIFT      14

/* Vendor Defined0(0x100) */
/* Vendor Defined1(0x104) */
#define MMC_BOOT_ACK			(1 << 2)
#define MMC_BUS_TEST_MODE		0x3
#define MMC_ALTERNATIVE_BOOT_MODE	0x2
#define MMC_BOOT_MODE			0x1
#define NORMAL_MODE			0x0
/* Vendor Defined2(0x108) */
/* Vendor Defined3(0x10C) */

#define SDHCI_GO_IDLE_STATE		0
#define MMC_SEND_OP_COND		1
#define SDHCI_SEND_ALL_CID		2
#define SDHCI_SEND_RELATIVE_ADDR	3
#define SDHCI_IO_SEND_OP_COND		5
#define MMC_SWITCH			6
#define SDHCI_SWITCH_FUNC		6
#define SDHCI_SET_BUS_WIDTH		6
#define SDHCI_SELECT_CARD		7
#define SDHCI_SEND_IF_COND		8
#define MMC_SEND_EXT_CSD		8
#define SDHCI_SEND_CSD			9
#define SDHCI_SEND_CID			10
#define SDHCI_VOLTAGE_SWITCH		11
#define SDHCI_STOP_TRANS		12
#define SDHCI_SEND_STATUS		13
#define SDHCI_SD_STATUS			13
#define SDHCI_SET_BLOCKLEN		16
#define SDHCI_READ_SINGLE_BLOCK		17
#define SDHCI_READ_MULTI_BLOCK		18
#define SDHCI_SEND_TUNE_BLOCK		19
#define MMC_SEND_TUNE_BLOCK		21
#define SDHCI_SEND_NUM_WR_BLKS		22
#define SDHCI_SET_BLOCK_COUNT		23
#define SDHCI_WRITE_BLOCK		24
#define SDHCI_WRITE_MULTI_BLOCK		25

#define SDHCI_SET_WRITE_PROT		28
#define SDHCI_CLR_WRITE_PROT		29
#define SDHCI_SEND_WRITE_PROT		30
#define SDHCI_SEND_WRITE_PROT_TYPE	31

#define SDHCI_ERASE_WR_BLK_START	32
#define SDHCI_ERASE_WR_BLK_END		33
#define SDHCI_ERASE_GROUP_START		35
#define SDHCI_ERASE_GROUP_END		36
#define SDHCI_ERASE			38
#define SDHCI_SD_SEND_OP_COND		41
#define SDHCI_GET_MKB			43
#define SDHCI_GET_MID			44
#define SDHCI_CER_RN1			45
#define SDHCI_CER_RN2			46
#define SDHCI_CER_RES2			47
#define SDHCI_CER_RES1			48
#define MMC_CMDQ_TASK_MGMT		48
#define MMC_SET_TIME			49
#define SDHCI_SEND_SCR			51
#define SDHCI_IO_RW_DIRECT		52
#define SDHCI_IO_RW_EXTENDED		53
#define SDHCI_APP_CMD			55
#define SDHCI_GEN_CMD			56
#define MMC_VENDOR_CMD			62

#define SDHCI_SEND_IF_COND_ARGU			0x1AA
#define SDHCI_SD_SEND_OP_COND_HCS_ARGU		0xC0FF8000
#define SDHCI_SD_SEND_OP_COND_ARGU		0x00FF8000
#define SDHCI_MMC_SEND_OP_COND_BYTE_MODE	0x80FF8000
#define SDHCI_MMC_SEND_OP_COND_SECTOR_MODE	0xC0FF8000

#define CMD_RETRY_CNT		5
#define SDHCI_TIMEOUT		0xFFF

/* SD state diagram */
/*#define SD_CID_MODE		0x1
#define SD_STANDBY_STATE	0x2
#define SD_TRANSFER_STATE	0x3
#define SD_SND_DATA_STATE	0x4
#define SD_RCV_DATA_STATE	0x5
#define SD_PROGRAM_STATE	0x6
#define SD_DISCONNECT_STATE	0x7
*/

/* For CMD52*/
#define SD_CMD52_RW_in_W		0x80000000
#define SD_CMD52_RW_in_R		0x00000000
#define SD_CMD52_RAW			0x08000000
#define SD_CMD52_no_RAW			0x00000000
#define SD_CMD52_FUNC(Num)		(Num  << 28)
#define SD_CMD52_Reg_Addr(Addr) (Addr << 9)
/* For CMD53*/
#define SD_CMD53_RW_in_W		0x80000000
#define SD_CMD53_RW_in_R		0x00000000
#define SD_CMD53_FUNC(Num)		(Num  << 28)
#define SD_CMD53_Block_Mode		0x08000000
#define SD_CMD53_Byte_Mode		0x00000000
#define SD_CMD53_OP_inc			0x04000000
#define SD_CMD53_OP_fix			0x00000000
#define SD_CMD53_Reg_Addr(Addr) (Addr << 9)
//************************************

/**
 * Card status return from R1 response format.
 * Or use CMD13 to get this status 
 */
#define SD_STATUS_OUT_OF_RANGE        0x80000000
#define SD_STATUS_ADDRESS_ERROR       (1 << 30)
#define SD_STATUS_BLOCK_LEN_ERROR     (1 << 29)
#define SD_STATUS_ERASE_SEQ_ERROR     (1 << 28)
#define SD_STATUS_ERASE_PARAM         (1 << 27)
#define SD_STATUS_WP_VIOLATION        (1 << 26)
#define SD_STATUS_CARD_IS_LOCK        (1 << 25)
#define SD_STATUS_LOCK_UNLOCK_FAILED  (1 << 24)
#define SD_STATUS_COM_CRC_ERROR       (1 << 23)
#define SD_STATUS_ILLEGAL_COMMAND     (1 << 22)
#define SD_STATUS_CARD_ECC_FAILED     (1 << 21)
#define SD_STATUS_CC_ERROR            (1 << 20)
#define SD_STATUS_ERROR               (1 << 19)
#define SD_STATUS_UNDERRUN            (1 << 18)
#define SD_STATUS_OVERRUN             (1 << 17)
#define SD_STATUS_CSD_OVERWRITE       (1 << 16)
#define SD_STATUS_WP_ERASE_SKIP       (1 << 15)
#define SD_STATUS_CARD_ECC_DISABLE    (1 << 14)
#define SD_STATUS_ERASE_RESET         (1 << 13)
#define SD_STATUS_CURRENT_STATE(x)    ((x >> 9) & 0xF)
typedef enum {
	CUR_STATE_IDLE = 0,
	CUR_STATE_READY,
	CUR_STATE_IDENT,
	CUR_STATE_STBY,
	CUR_STATE_TRAN,
	CUR_STATE_DATA,
	CUR_STATE_RCV,
	CUR_STATE_PRG,
	CUR_STATE_DIS,
	CUR_STATE_RSV
} Card_State;

#define SD_STATUS_READY_FOR_DATA      (1 << 8)
#define MMC_STATUS_SWITCH_ERROR       (1 << 7)
#define SD_STATUS_APP_CMD             (1 << 5)
#define SD_STATUS_AKE_SEQ_ERROR       (1 << 3)

#define SD_STATUS_ERROR_BITS          (SD_STATUS_OUT_OF_RANGE | SD_STATUS_ADDRESS_ERROR | \
                                       SD_STATUS_BLOCK_LEN_ERROR | SD_STATUS_ERASE_SEQ_ERROR | \
                                       SD_STATUS_ERASE_PARAM | SD_STATUS_WP_VIOLATION | \
                                       SD_STATUS_LOCK_UNLOCK_FAILED | SD_STATUS_CARD_ECC_FAILED | \
                                       SD_STATUS_CC_ERROR | SD_STATUS_ERROR | \
                                       SD_STATUS_UNDERRUN | SD_STATUS_OVERRUN | \
                                       SD_STATUS_CSD_OVERWRITE | SD_STATUS_WP_ERASE_SKIP | \
                                       SD_STATUS_AKE_SEQ_ERROR | MMC_STATUS_SWITCH_ERROR)

#define SDHCI_1BIT_BUS_WIDTH	0x0
#define SDHCI_4BIT_BUS_WIDTH	0x2

/* ADMA Descriptor Table Generator */
#define ADMA2_ENTRY_VALID	(1 << 0)
#define ADMA2_ENTRY_END		(1 << 1)
#define ADMA2_ENTRY_INT		(1 << 2)

#define ADMA2_NOP		(0 << 4)
#define ADMA2_SET		(1 << 4)
#define ADMA2_TRAN		(2 << 4)
#define ADMA2_LINK		(3 << 4)

typedef struct {
	uint16_t attr;
	uint16_t lgth;
	uint32_t addr;
} Adma2DescTable;

#define ADMA2_NUM_OF_LINES	1536

/* ftsdc021_main.c */
uint8_t *SDC_ShowCardState(Card_State state);
#ifdef SDHCI_DEBUG
void SDC_ShowDscpTbl(uint32_t blkcnt, uint32_t tbl_addr);
#endif

/* ftsdc021_ops.c */
/**
 * SD card related operations(Some applies to MMC)
 */
uint32_t ftsdc021_ops_go_idle_state(uint32_t arg);
uint32_t ftsdc021_ops_send_if_cond(SDCardInfo * card, uint32_t arg);
uint32_t ftsdc021_ops_app_cmd(SDCardInfo * card, uint32_t argu, uint8_t datChk);
uint32_t ftsdc021_ops_send_app_op_cond(SDCardInfo * card, uint32_t ocr, uint32_t * rocr);
uint32_t ftsdc021_ops_send_voltage_switch(void);
uint32_t ftsdc021_ops_all_send_cid(SDCardInfo * card);
uint32_t ftsdc021_ops_send_rca(SDCardInfo * card);
uint32_t ftsdc021_ops_send_csd(SDCardInfo * card);
uint32_t ftsdc021_ops_select_card(SDCardInfo * card);
uint32_t ftsdc021_ops_app_send_scr(SDCardInfo * card);
uint32_t ftsdc021_ops_app_repo_wr_num(SDCardInfo * card, uint32_t *written_num);
uint32_t ftsdc021_ops_app_set_wr_blk_cnt(SDCardInfo * card, uint32_t n_blocks);
uint32_t ftsdc021_ops_app_set_bus_width(SDCardInfo * card, uint32_t width);
uint32_t ftsdc021_ops_sd_switch(SDCardInfo * card, uint32_t mode, uint32_t group,
			      uint8_t value, uint8_t * resp);
uint32_t ftsdc021_ops_send_tune_block(SDCardInfo * card, uint32_t * buf);
uint32_t ftsdc021_ops_send_card_status(SDCardInfo * card);
uint32_t ftsdc021_ops_set_blocklen(SDCardInfo * card, uint32_t block_len);
uint32_t ftsdc021_ops_send_sd_status(SDCardInfo * card);

/* ftsdc021_mmc_ops.c */
/**
 * MMC card related operations
 */
uint32_t ftsdc021_mmc_send_hpi(SDCardInfo *card);
uint32_t ftsdc021_mmc_send_op_cond(SDCardInfo *card, uint32_t ocr, uint32_t *rocr);
uint32_t ftsdc021_mmc_set_rca(SDCardInfo *card);
uint32_t ftsdc021_mmc_switch(SDCardInfo *card, uint8_t set, uint8_t index, uint8_t value,
			   uint32_t use_busy);
uint32_t ftsdc021_mmc_send_ext_csd(SDCardInfo *card);
uint32_t ftsdc021_mmc_set_write_prot(SDCardInfo *card, uint32_t address);
uint32_t ftsdc021_mmc_clear_write_prot(SDCardInfo *card, uint32_t address);
uint32_t ftsdc021_mmc_send_write_prot(SDCardInfo *card, uint32_t *WP_bits, uint32_t length);
uint32_t ftsdc021_mmc_send_write_prot_type(SDCardInfo *card, uint32_t *WP_bits, uint32_t length);
uint32_t ftsdc021_mmc_set_time(SDCardInfo *card, uint64_t time, uint8_t type);

/* ftsdc021_sdio_ops.c */
/**
 * SDIO card related operations
 */
uint32_t ftsdc021_sdio_send_io_op_cond(SDCardInfo *card, uint32_t ocr, uint32_t *rocr);
uint32_t ftsdc021_sdio_io_rw_direct(SDCardInfo *card, uint32_t write, uint8_t fn,
				  uint32_t addr, uint8_t in, uint8_t *out);
uint32_t ftsdc021_sdio_io_rw_extended(SDCardInfo *card, uint32_t write, uint8_t fn,
				    uint32_t addr, uint32_t incr_addr, uint32_t *buf,
				    uint32_t blocks, uint32_t blksz);
uint32_t ftsdc021_sdio_set_bus_width(SDCardInfo *card, uint32_t width);
uint32_t ftsdc021_sdio_set_bus_speed(SDCardInfo *card, uint8_t speed);
uint32_t ftsdc021_sdio_Card_Info(SDCardInfo *card);

/* ftsdc021.c */
BOOL ftsdc021_fill_adma_desc_table(uint32_t total_data, uint32_t *data_addr,
				   Adma2DescTable *ptr);
uint32_t ftsdc021_ErrorRecovery(void);
void ftsdc021_delay(uint32_t ms);
void ftsdc021_dump_regs(void);
uint32_t ftsdc021_transfer_data(Transfer_Act act, uint32_t *buffer,
			      uint32_t length);
uint32_t ftsdc021_CheckCardInsert(void);
void ftsdc021_SetPower(int16_t power);
void ftsdc021_SetSDClock(uint32_t clock);
uint32_t ftsdc021_set_bus_width(uint8_t width);
uint32_t ftsdc021_set_bus_speed_mode(uint8_t speed);
uint32_t ftsdc021_set_1v8_signal_voltage(void);
uint32_t ftsdc021_signal_voltage_switch(void);
uint32_t ftsdc021_pulselatch_tuning_1(void);
uint32_t ftsdc021_pulselatch_tuning(uint32_t div);
uint32_t ftsdc021_execute_tuning(uint32_t try);
void ftsdc021_set_base_clock(uint32_t clk);
void ftsdc021_set_transfer_type(Transfer_Type type, uint32_t line_bound,
				uint8_t ran_mode);
void ftsdc021_HCReset(uint8_t type);
uint32_t ftsdc021_wait_for_state(uint32_t state, uint32_t ms);
uint32_t ftsdc021_prepare_data(uint32_t blk_cnt, uint16_t blk_sz, uint32_t *buff_addr,
			     Transfer_Act act);
uint32_t ftsdc021_set_transfer_mode(uint8_t blk_cnt_en, uint8_t auto_cmd, uint8_t dir,
			     uint8_t multi_blk);
uint32_t ftsdc021_send_abort_command(void);
uint32_t ftsdc021_send_command(struct mmc_cmd *mcmd);
uint32_t ftsdc021_card_read(uint32_t startAddr, uint32_t blkcnt, uint8_t *readbuf);
uint32_t ftsdc021_card_write(uint32_t startAddr, uint32_t blkcnt, uint8_t *writebuf);
uint32_t get_erase_group_size(uint32_t *erase_group_size);
uint32_t ftsdc021_card_erase(uint32_t StartBlk, uint32_t BlkCount);
uint32_t ftsdc021_card_discard(uint32_t StartBlk, uint32_t BlkCount);
uint32_t ftsdc021_abort_test(uint8_t *readbuf, uint8_t *writebuf, uint32_t blk_cnt);
uint32_t ftsdc021_select_voltage(uint32_t ocr);
uint32_t ftsdc021_scan_cards(void);
uint32_t ftsdc021_init(void);

uint32_t SDC_data_check(uint32_t blkcnt, uint32_t *source_buf, uint32_t *dest_buf);
void SDC_PrepareWB(uint32_t length);

uint32_t ftsdc021_attach_sd(void);
uint32_t ftsdc021_attach_sdio(void);

/* ftsdc021_sd.c */
uint32_t ftsdc021_sd_read_scr(void);

/* ftsdc021_mmc.c */
uint32_t ftsdc021_mmc_read_ext_csd(void);
uint32_t ftsdc021_mmc_set_bus_speed(SDCardInfo *card, uint8_t speed);
uint32_t ftsdc021_attach_mmc(void);
uint32_t ftsdc021_mmc_set_partition_access(uint8_t partition);
uint32_t ftsdc021_mmc_set_bootmode(uint8_t partition);
uint32_t ftsdc021_mmc_set_bootack(uint8_t ack);
uint32_t ftsdc021_mmc_boot(uint8_t boot_mode, uint8_t ack, uint8_t boot_spd,
                         uint8_t boot_bw, uint32_t *boot_addr, uint32_t boot_blocks);
uint32_t ftsdc021_mmc_hpi(void);
uint32_t ftsdc021_mmc_r1_exception(void);
uint32_t ftsdc021_mmc_send_vendor_cmd(uint32_t argument);
uint32_t ftsdc021_mmc_gen_packed_hdr(Transfer_Act act, uint32_t startAddr,
			       uint32_t blkcnt, uint32_t *buf);
/* ftsdc021_cqhci.c */
uint32_t cqhci_send_command(struct mmc_cmd *cmd);
void cqhci_on(void);
void cqhci_off(uint32_t wait_empty);                                                             
void cqhci_irq(uint16_t int_err);

extern uint32_t dbg_print;
extern volatile ftsdc021_reg *gpReg;
extern uint32_t adma2_insert_nop;
extern uint32_t adma2_use_interrupt;
extern uint32_t rd_bl_len, wr_bl_len;
extern Infinite_Test infinite_mode;
extern uint32_t sdma_bound_mask;
extern uint8_t auto_ed_cbc;
extern SDHost host;
extern uint32_t rpmb_key[32];
extern SDCardInfo cardInfo;
extern clock_t timeout_ms;

static inline uint32_t ftsdc021_uhs_mode(void)
{
	return (gpReg->HostCtrl2 & SDHCI_18V_SIGNAL);
}

static inline uint32_t ftsdc021_uhs_mode_support(void)
{
	return gpReg->CapReg2 & (SDHCI_SUPPORT_SDR50 | SDHCI_SUPPORT_SDR104 |
		SDHCI_SUPPORT_DDR50);
}

#endif
