/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_eon.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * July-2015   Bing-Yao          
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_NOR_H
#define FTSPI020_NOR_H

#define NOR_WRITE_ENABLE	0x06
#define NOR_WRITE_ENABLE_VOL	0x50
#define NOR_WRITE_DISABLE	0x04

//Register Access
#define NOR_READ_STATUS1	0x05
#define NOR_READ_STATUS2	0x35
#define NOR_READ_STATUS3	0x15
#define NOR_READ_BANK_ADDR	0x16 //Spansion
#define NOR_READ_INFO_REG_EON	0x2B //EON
#define NOR_WRITE_STATUS1 	0x01
#define NOR_WRITE_STATUS2 	0x31
#define NOR_WRITE_STATUS3 	0x11
#define NOR_WRITE_BANK_ADDR 	0x17

#define NOR_READ_STATUS3_S25FL164K	0x33 //LC dummy cycles for read
#define NOR_READ_STATUS3_S25FL128S	0x07 //Program, Erase suspend
//Read memory
#define NOR_READ_DATA		0x03
#define NOR_FAST_READ		0x0B
#define NOR_READ_DUAL		0x3B
#define NOR_READ_DUAL_IO	0xBB
#define NOR_READ_QUAD		0x6B
#define NOR_READ_QUAD_IO	0xEB
#define NOR_DDR_FAST_READ	0x0D
#define NOR_DDR_DUAL_IO_READ	0xBD
#define NOR_DDR_QUAD_IO_READ	0xED

//Read Memory 4 bytes commands
#define NOR_READ_DATA_4B	0x13
#define NOR_FAST_READ_4B	0x0C
#define NOR_READ_DUAL_4B	0x3C
#define NOR_READ_DUAL_IO_4B	0xBC
#define NOR_READ_QUAD_4B	0x6C
#define NOR_READ_QUAD_IO_4B	0xEC
#define NOR_DDR_FAST_READ_4B	0x0D
#define NOR_DDR_DUAL_IO_READ_4B	0xBD
#define NOR_DDR_QUAD_IO_READ_4B	0xED

//Program memory
#define NOR_WRITE_PAGE		0x02
#define NOR_WRITE_PAGE_DUAL 0xA2 //Micron
#define NOR_WRITE_PAGE_QUAD	0x32
#define NOR_WRITE_PAGE_QUAD_MXIC	0x38
#define NOR_WRITE_PAGE_QUAD_MXIC_4B	0x3E

#define NOR_WRITE_PAGE_4B	0x12
#define NOR_WRITE_PAGE_QUAD_4B	0x34

#define NOR_ERASE_SECTOR	0x20
#define NOR_ERASE_32KB_BLOCK 	0x52
#define NOR_ERASE_64KB_BLOCK 	0xD8
#define NOR_ERASE_CHIP  	0xC7

#define NOR_ERASE_SECTOR_4B	0x21
#define NOR_ERASE_32KB_BLOCK_4B 0x5C
#define NOR_ERASE_64KB_BLOCK_4B 0xDC

#define NOR_EN4B  		0xB7
#define NOR_EX4B  		0xE9

#define NOR_ENQPI  		0x38
#define NOR_ENQPI_MXIC  0x35
#define NOR_EXQPI  		CMD_RESET
#define NOR_EXQPI_MXIC  0xF5

typedef enum {
	ERTYPE_SECTOR_ERASE = 0,
	ERTYPE_BLOCK_ERASE,
	ERTYPE_MAX
} NOR_ERASE_TYPE;

typedef enum {
	WRTYPE_PROGRAM = 0,
	WRTYPE_PROGRAM_QUAD,
	WRTYPE_MAX
} NOR_WRITE_TYPE;

typedef enum {
	RDTYPE_READ_DATA = 0,
	RDTYPE_FAST_READ,
	RDTYPE_READ_DUAL,
	RDTYPE_READ_DUAL_IO,
	RDTYPE_READ_QUAD,
	RDTYPE_READ_QUAD_IO,
	RDTYPE_FAST_READ_DTR,
	RDTYPE_READ_DUAL_IO_DTR,
	RDTYPE_READ_QUAD_IO_DTR,
	RDTYPE_MAX
} NOR_READ_TYPE;

#endif
