/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_mmc.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 *
 * -----------------------------------------------------------------------------
 */
#ifndef FTSDC021_MMC_H
#define FTSDC021_MMC_H

#include "../portme_ftsdc021.h"

typedef struct {
	uint32_t CSD_STRUCTURE:2;
	uint32_t SPEC_VERS:4;
	uint32_t Reserved1:2;
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
	uint32_t ERASE_GRP_SIZE:5;
	uint32_t ERASE_GRP_MULT:5;
	uint32_t WP_GRP_SIZE:5;
	uint32_t WP_GRP_ENABLE:1;
	uint32_t DEFAULT_ECC:2;
	uint32_t R2W_FACTOR:3;
	uint32_t WRITE_BL_LEN:4;
	uint32_t WRITE_BL_PARTIAL:1;
	uint32_t Reserved3:4;
	uint32_t CONTENT_PROT_APP:1;
	uint32_t FILE_FORMAT_GRP:1;
	uint32_t COPY:1;
	uint32_t PERM_WRITE_PROTECT:1;
	uint32_t TMP_WRITE_PROTECT:1;
	uint32_t FILE_FORMAT:2;
	uint32_t ECC:2;
} MMC_CSD;

typedef struct {
	/* Modes Segment */
	uint8_t Reserved29[15];
	uint8_t CMDQ_MODE_EN;			/* [15] */
	uint8_t SECURE_REMOVAL_TYPE;		/* [16] */
	uint8_t PRODUCT_STATE_AWARENESS_ENABLEMENT;/* [17] */
	uint8_t MAX_PRE_LOADING_DATA_SIZE[4];	/* [21:18] */
	uint8_t PRE_LOADING_DATA_SIZE[4];		/* [25:22] */
	uint8_t FFU_STATUS;                       /* [26] */
	uint8_t Reserved28[2];
	uint8_t MODE_OPERATION_CODES;		/* [29] */
	uint8_t MODE_CONFIG;			/* [30] */
	uint8_t BARRIER_CTRL;			/* [31] */
	uint8_t FLUSH_CACHE;			/* [32] */
	uint8_t CACHE_CTRL;			/* [33] */
	uint8_t POWER_OFF_NOTIFICATION;		/* [34] */
	uint8_t PACKED_FAILURE_INDEX;		/* [35] */
	uint8_t PACKED_COMMAND_STATUS;		/* [36] */
	uint8_t CONTEXT_CONF[15];			/* [51:37] */
	uint8_t EXT_PARTITIONS_ATTRIBUTE[2];	/* [53:52] */
	uint8_t EXCEPTION_EVENTS_STATUS[2];	/* [55:54] */
	uint8_t EXCEPTION_EVENTS_CTRL[2];		/* [57:56] */
	uint8_t DYNCAP_NEEDED;			/* [58] */
	uint8_t CLASS_6_CTRL;			/* [59] */
	uint8_t INI_TIMEOUT_EMU;			/* [60] */
	uint8_t DATA_SECTOR_SIZE;			/* [61] */
	uint8_t USE_NATIVE_SECTOR;		/* [62] */
	uint8_t NATIVE_SECTOR_SIZE;		/* [63] */
	uint8_t Reserved27[70];			/* [133:64] */
	uint8_t SEC_BAD_BLK_MGMNT;
	uint8_t Reserved26; 
	uint8_t ENH_SATRT_ADDR[4];
	uint8_t ENH_SIZE_MULT[3];
	uint8_t GP_SIZE_MULT[12];
	uint8_t PARTITION_SETTING_COMPLETED;
	uint8_t PARTITIONING_ATTRIBUTE;
	uint8_t MAX_ENH_SIZE_MULT[3];
	uint8_t PARTITIONING_SUPPORT;
	uint8_t HPI_MGMT;		/* [161] */
	uint8_t RST_n_FUNCTION;
	uint8_t BKOPS_EN;		/* [163] */
	uint8_t BKOPS_START;	/* [164] */
	uint8_t SANITIZE_START;	/* [165] */
	uint8_t WR_REL_PARAM;	/* [166] */
	uint8_t WR_REL_SET;	/* [167] */
	uint8_t RPMB_SIZE_MULT;
	uint8_t FW_CONFIG;
	uint8_t Reserved23;
	uint8_t USER_WP;
	uint8_t Reserved22;
	uint8_t BOOT_WP;		/* [173] R/W & R/W/C_P */
	uint8_t Reserved21;
	uint8_t ERASE_GROUP_DEF;
	uint8_t Reserved20;
	uint8_t BOOT_BUS_WIDTH;	/* [177] R/W/E */
	uint8_t BOOT_CONFIG_PROT;	/* [178] R/W & R/W/C_P */
	uint8_t PARTITION_CONF;	/* [179] */
	uint8_t Reserved19;
	uint8_t ERASED_MEM_CONT;
	uint8_t Reserved18;
	uint8_t BUS_WIDTH;	/* [183] W/E_P */
	uint8_t Reserved17;
	uint8_t HS_TIMING;	/* [185] R/W/E_P */
	uint8_t Reserved16;
	uint8_t POWER_CLASS;
	uint8_t Reserved15;
	uint8_t CMD_SET_REV;
	uint8_t Reserved14;
	uint8_t CMD_SET;
	/* Properties Segment */
	uint8_t EXT_CSD_REV;
	uint8_t Reserved13;
	uint8_t CSD_STRUCTURE;
	uint8_t Reserved12;
	uint8_t CARDTYPE;
	uint8_t DRIVER_STRENGTH;
	uint8_t OUT_OF_INTERRUPT_TIME;
	uint8_t PARTITION_SWITCH_TIME;
	uint8_t PWR_CL_52_195;
	uint8_t PWR_CL_26_195;
	uint8_t PWR_CL_52_360;
	uint8_t PWR_CL_26_360;
	uint8_t Reserved10;
	uint8_t MIN_PERF_R_4_26;
	uint8_t MIN_PERF_W_4_26;
	uint8_t MIN_PERF_R_8_26_4_52;
	uint8_t MIN_PERF_W_8_26_4_52;
	uint8_t MIN_PERF_R_8_52;
	uint8_t MIN_PERF_W_8_52;
	uint8_t Reserved9;
	uint32_t SEC_COUNT;	/* [215:212] R */
	uint8_t SLEEP_NOTIFICATION_TIME; /* [216] R */
	uint8_t S_A_TIMEOUT;
	uint8_t Reserved7;
	uint8_t S_C_VCCQ;
	uint8_t S_C_VCC;
	uint8_t HC_WP_GRP_SIZE;
	uint8_t REL_WR_SEC_C;
	uint8_t ERASE_TIMEOUT_MULT;
	uint8_t HC_ERASE_GRP_SIZE;
	uint8_t ACC_SIZE;
	uint8_t BOOT_SIZE_MULTI;	/* [226] R */
	uint8_t Reserved6;	/* [227] , (embedded mmc )is 2 bytes width. */
	uint8_t BOOT_INFO;	/* [228] R */
	uint8_t SEC_TRIM_MULT;
	uint8_t SEC_ERASE_MULT;
	uint8_t SEC_FEATURE_SUPPORT;
	uint8_t TRIM_MULT;
	uint8_t Reserved5;
	uint8_t MIN_PERF_DDR_R_8_52;
	uint8_t MIN_PERF_DDR_W_8_52;
	uint8_t Reserved4[2];
	uint8_t PWR_CL_DDR_52_195; 	/* [238] */
	uint8_t PWR_CL_DDR_52_360;	/* [239] */
	uint8_t Reserved3;
	uint8_t INI_TIMEOUT_AP;		/* [241] */
	uint8_t Reserved2[4];
	uint8_t BKOPS_STATUS;		/* [246] */
	uint8_t POWER_OFF_LONG_TIME;	/* [247] */
	uint8_t Reserved2_1[59];
	uint8_t CMDQ_DEPTH;		/* [307] */
	uint8_t CMDQ_SUPPORT;		/* [308] */
	uint8_t Reserved2_2[177];
	uint8_t BARRIER_SUPPORT;		/* [486] */
	uint8_t FFU_ARG[4];		/* [490:487] */
	uint8_t OPERATION_CODES_TIMEOUT;	/* [491] */
	uint8_t FFU_FEATURES;		/* [492] */
	uint8_t SUPPORTED_MODES;		/* [493] */
	uint8_t EXT_EUPPORT;		/* [494] */
	uint8_t LARGE_UNIT_SIZE_M1;	/* [495] */
	uint8_t CONTEXT_CAPABILITIES;	/* [496] */
	uint8_t TAG_RES_SIZE;		/* [497] */
	uint8_t TAG_UNIT_SIZE;		/* [498] */
	uint8_t DATA_TAG_SUPPORT;		/* [499] */
	uint8_t MAX_PACKED_WRITES;	/* [500] */
	uint8_t MAX_PACKED_READS;		/* [501] */
	uint8_t BKOPS_SUPPORT;		/* [502] */
	uint8_t HPI_FEATURES;		/* [503] */
	uint8_t S_CMD_SET;		/* [504] */
	uint8_t Reserved1[7];		/* [511:505] */
} MMC_EXT_CSD;

typedef struct {
	uint8_t Version;
	uint8_t RTC_INFO_TYPE;
	uint64_t SECONDS_PASSED;
	uint8_t Reserved[502];
} MMC_RTC_INFO;

#define MMC_RTC_ABSOLUTE_TIME		1
#define MMC_RTC_SET_RELATIVE_BASE	2
#define MMC_RTC_RELATIVE_TIME		3

/* Cmd Set [2:0] of argument SWITCH command*/
#define EXT_CSD_CMD_SET_NORMAL          (1<<0)
#define EXT_CSD_CMD_SET_SECURE          (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE        (1<<2)

/* Offset at EXT CSD to access */
#define EXT_CSD_POWER_OFF_NOTIFICATION	34
#define EXT_CSD_EXCEPTION_EVENTS_CTRL	56
#define EXT_CSD_PARTITION_SETTING_COMPLETED	156
#define EXT_CSD_HPI_MGMT	161
#define EXT_CSD_RST_n_EN        162
#define EXT_CSD_BKOPS_EN        163
#define EXT_CSD_BKOPS_START     164
#define EXT_CSD_PARTITION_CONF			179
#define EXT_CSD_BUS_WIDTH       183	/* R/W */
#define EXT_CSD_HS_TIMING       185	/* R/W */
#define EXT_CSD_CARD_TYPE       196	/* RO */
#define EXT_CSD_DRIVER_STRENGTH 197	/* RO */
#define EXT_CSD_SEC_CNT         212	/* RO, 4 bytes */
#define EXT_CSD_BOOT_SIZE_MULT	226


#define EXT_CSD_Command_set		0x0
#define EXT_CSD_Set_bit			0x1
#define	EXT_CSD_Clear_byte		0x2
#define EXT_CSD_Write_byte		0x3

#define EXT_CSD_Bus8bit			0x2
#define EXT_CSD_Bus4bit			0x1
#define EXT_CSD_Bus1bit			0x0

#define MMC_CMD6_Access_mode(x)	(uint32_t)( x << 24)
#define MMC_CMD6_Index(x)		(uint32_t)( x << 16)
#define MMC_CMD6_Value(x)		(uint32_t)( x << 8)
#define MMC_CMD6_Cmd_set(x)		(uint32_t)( x )

#define EXT_CSD_HPI_IMPLEMENTATION	(1 << 1)
#define EXT_CSD_HPI_SUPPORT		(1 << 0)

#define PACKED_FLAG			(1 << 30)
#define RELIABLE_WRITE_FLAG		(1 << 31)

//EXCEPTION EVENTS CTRL
#define EXT_CSD_EXP_DYNCAP_EN		(1 << 1)
#define EXT_CSD_EXP_SYSPOOL_EN		(1 << 2)
#define EXT_CSD_EXP_PACKED_EN		(1 << 3)
#define EXT_CSD_EXP_SECURITY_EN		(1 << 3)
//EXCEPTION EVENTS STATUS
#define EXT_CSD_EXP_URGENT_BKOPS	(1 << 0)
#define EXT_CSD_EXP_DYNCAP_NEEDED	(1 << 1)
#define EXT_CSD_EXP_SYSPOOL_EXHAUSTED	(1 << 2)
#define EXT_CSD_EXP_PACKED_FAILURE	(1 << 3)
#define EXT_CSD_EXP_SECURITY_FAILURE	(1 << 4)

#define DISCARD_CMD_ARG			3

#define EXT_CSD_BKOPS_SUPPORT           (1 << 0)
#define R1_EXCEPTION_EVENT		(1 << 6)
#define R1_SWITCH_ERROR			(1 << 7)

#define MMC_CARD_BUSY   0x80000000	/* Card Power up status bit */

#endif
