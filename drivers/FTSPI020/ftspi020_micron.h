/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_micron.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * Aug.2015     Bing-Yao          
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_MICRON_H
#define FTSPI020_MICRON_H

/* Read Memory Array Commands */
#define MICRON_READ			0x03
#define MICRON_FAST_READ		0x0B
#define MICRON_DUAL_OUT_READ		0x3B
#define MICRON_DUAL_IO_READ		0xBB
#define MICRON_QUAD_OUT_READ		0x6B
#define MICRON_QUAD_IO_READ		0xEB

/* Register Access Commands */
#define MICRON_WRITE_ENABLE		0x06
#define MICRON_WRITE_DISABLE		0x04
#define MICRON_READ_STATUS		0x05
#define MICRON_WRITE_STATUS		0x01
#define MICRON_READ_FLAG_STATUS		0x70
#define MICRON_CLEAR_FLAG_STATUS	0x50
#define MICRON_READ_NONVOL_CONFIG	0xB5
#define MICRON_WRITE_NONVOL_CONFIG	0xB1
#define MICRON_READ_VOL_CONFIG		0x85
#define MICRON_WRITE_VOL_CONFIG		0x81
#define MICRON_READ_ENH_VOL_CONFIG	0x65
#define MICRON_WRITE_ENH_VOL_CONFIG	0x61

/* Program Flash Array Commands */
#define MICRON_PAGE_PROGRAM		0x02
#define MICRON_DUAL_PAGE_PROGRAM	0xA2
#define MICRON_QUAD_PAGE_PROGRAM	0x32

/* Erase Flash Array Commands */
#define MICRON_ERASE_SUBSECTOR 	0x20
#define MICRON_ERASE_SECTOR	0xD8
#define MICRON_ERASE_CHIP	0xC7


/* Status Register */
#define MICRON_STS_BUSY			(1 << 0)
#define MICRON_STS_WEL			(1 << 1)
#define MICRON_STS_BPROTECT		((1 << 6) | (0x7 << 2))
#define MICRON_STS_PROT_BOTTOM		(1 << 5)
#define MICRON_STS_REG_WRITE_DIS	(1 << 7)

/* Nonvolatile Configuration Register */
#define MICRON_NONVOL_CFG_DUMMY_CYCLE(x)	((x & 0xf) << 12)
#define MICRON_NONVOL_CFG_XIP_FAST_READ		(0x00 << 9)
#define MICRON_NONVOL_CFG_XIP_DOUT_READ		(0x01 << 9)
#define MICRON_NONVOL_CFG_XIP_DIO_READ		(0x02 << 9)
#define MICRON_NONVOL_CFG_XIP_QOUT_READ		(0x03 << 9)
#define MICRON_NONVOL_CFG_XIP_QIO_READ		(0x04 << 9)
#define MICRON_NONVOL_CFG_QUAD_IO		(1 << 3)
#define MICRON_NONVOL_CFG_DUAL_IO		(1 << 2)
#define MICRON_NONVOL_CFG_CONFIG_LOCK		(1 << 0)

/* Volatile Configuration Register */
#define MICRON_VOL_CFG_DUMMY_CYCLE(x)		((x & 0xf) << 4)
#define MICRON_VOL_CFG_XIP_ENABLE		(1 << 3)
//Default 11b: Continous reading, All bytes are read sequentially
#define MICRON_VOL_CFG_WRAP(x)			((x & 0x3) << 0)

/**
 * Enhance dual and quad IO SPI not
 * support, because command is transmitting
 * across two or four data lines.
 */

/* Enhanced Volatile Configuration Register */
#define MICRON_ENH_VOL_CFG_QUAD_IO		(1 << 7)
#define MICRON_ENH_VOL_CFG_DUAL_IO		(1 << 6)

/* Flag Status Register */
#define MICRON_FLAG_STS_READY		(1 << 7)
#define MICRON_FLAG_STS_ERASE_SUSPEND	(1 << 6)
#define MICRON_FLAG_STS_ERASE_ERR	(1 << 5)
#define MICRON_FLAG_STS_PROGRAM_ERR	(1 << 4)
#define MICRON_FLAG_STS_PROGRAM_SUSPEND	(1 << 2)
#define MICRON_FLAG_STS_PROTECT_ERR	(1 << 1)

typedef enum {
	N25Q_PARAM_4K_ERASE = 0,
	N25Q_BLOCK_64K_ERASE,
	N25Q_MAX_ERASE_TYPE
}N25Q_ERASE_TYPE;

typedef enum {
	N25Q_PAGE_PROGRAM = 0,
	N25Q_DUAL_PROGRAM,
	N25Q_QUAD_PROGRAM,
	N25Q_MAX_WRITE_TYPE
}N25Q_WRITE_TYPE;

typedef enum {
	N25Q_READ = 0,
	N25Q_FAST_READ,
	N25Q_READ_DUAL_OUT,
#ifdef FTSPI020_SUPPORT_QUAD
	N25Q_READ_QUAD_OUT,
#endif
	N25Q_DUAL_IO_READ,
#ifdef FTSPI020_SUPPORT_QUAD
	N25Q_QUAD_IO_READ,
#endif
	N25Q_MAX_READ_TYPE
}N25Q_READ_TYPE;

#endif
