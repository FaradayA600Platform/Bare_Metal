/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_nand.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE         AUTHOR       DESCRIPTION
 * 2013/08/30   BingYao      SPI nand flash driver
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_NAND_H
#define FTSPI020_NAND_H

#define BBT_MAX_ENT    128

/* Command Op Code */
#define SPINAND_ERASE_BLOCK		0xD8
#define SPINAND_READ_STATUS		0x05
#define SPINAND_GET_FEATURE		0x0F
#define SPINAND_WRITE_STATUS	0x01
#define SPINAND_SET_FEATURE		0x1F
#define SPINAND_WRITE_DISABLE	0x04
#define SPINAND_WRITE_ENABLE	0x06
#define SPINAND_PAGE_LOAD		0x02 /* PROGRAM LOAD */
#define SPINAND_PAGE_LOAD_QUAD	0x32 /* PROGRAM LOAD x4 */
#define SPINAND_RANDOM_WRITE_PAGE	0x84 /* PROGRAM LOAD RANDOM DATA */
#define SPINAND_QUAD_RANDOM_WRITE_PAGE	0x34 /* PROGRAM LOAD RANDOM DATA x4 */
#define SPINAND_PROGRAM_EXECUTE		0x10 /* Write data from cache to main array */
#define SPINAND_READ_TO_CACHE		0x13
#define SPINAND_READ_FROM_CACHE		0x03
#define SPINAND_READ_FAST_FROM_CACHE	0x0B
#define SPINAND_READ_FROM_CACHE_DUAL	0x3B
#define SPINAND_READ_FROM_CACHE_QUAD	0x6B
#define SPINAND_READ_FROM_CACHE_DUAL_IO	0xBB
#define SPINAND_READ_FROM_CACHE_QUAD_IO	0xEB

/* Feature address */
/**
 * Block Lock register, default value 0x38(All locked).
 * Block Lock register can not be changed if SRP0 is enabled.
 * When WP-E is set to 1, the device is in Hardware Protection mode,
 * all Quad functions are disabled and /WP & /HOLD pins become dedicated
 * control input pins.
 */
#define SPINAND_FEA_PROTECTION	0xA0
#define    SPINAND_BL_SRP0	(1 << 0)
#define    SPINAND_BL_WPE	(1 << 1)
#define    SPINAND_BL_TB	(1 << 2)
#define    SPINAND_BL_BP0TO2	(0xf << 3)
#define    SPINAND_BL_SRP1	(1 << 7)

#define    SPINAND_BL_GB_BPROT	(0x7 << 3)

/**
 * Configuration register
 */
#define SPINAND_FEA_CFG		0xB0
#define    SPINAND_CFG_QE_EN	(1 << 0)
/* RW, BUF=0 Continuous Read Mode, BUF=1 Normal Read */
#define    SPINAND_CFG_BUF_MODE	(1 << 3)
#define    SPINAND_CFG_ECC_EN	(1 << 4) /* RW, ECC enable */
#define    SPINAND_CFG_SR1L     (1 << 5) /* RW */
#define    SPINAND_CFG_ENABLE	(1 << 6) /* RW */
#define    SPINAND_CFG_LOCK     (1 << 7) /* RW */

/**
 * Status register
 * WE_LATCH:
 * - Set/Clear by Write Enable/Disable command
 * ECC_STS0_1:
 *  0 - No errors,
 *  1 - 1-bit error and corrected
 *  2 - 2-bits errors and not corrected
 *  3 - Reserved
 */
#define SPINAND_FEA_STATUS		0xC0
#define    SPINAND_STS_BUSY      	(1 << 0) /* RO */
#define    SPINAND_STS_WE_LATCH  	(1 << 1) /* RW */
#define    SPINAND_STS_ERASE_FAIL	(1 << 2) /* RO */
#define    SPINAND_STS_PROGRAM_FAIL	(1 << 3) /* RO */
#define    SPINAND_STS_ECC_STS0_1	(3 << 4) /* RO */

#define SPINAND_FEA_DRV_STRENGTH 	0xD0

#endif
