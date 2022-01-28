/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_sdio.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 *  DATE        AUTHOR       	 DESCRIPTION
 * 2016/1/13/	Sanjin Liu	 Header file for SDIO
 * -----------------------------------------------------------------------------
 */

#ifndef FTSDC021_SDIO_H
#define FTSDC021_SDIO_H

typedef struct SDIO_Card_Feature {
	int8_t *name;
	int8_t support;
} SDIO_Card_Feature;

static SDIO_Card_Feature SDIO_capability[8] = {
	{"SDC \0", 0},
	{"SMB \0", 0},
	{"SRW \0", 0},
	{"SBS \0", 0},
	{"S4MI\0", 0},
	{"E4MI\0", 0},
	{"LSC \0", 0},
	{"4BLS\0", 0}
};

static SDIO_Card_Feature SDIO_bus_interface_control[8] = {
	{"Bus width \0", 0},
	{"Bus width \0", 0},
	{"RFU 	    \0", 0},
	{"RFU 	    \0", 0},
	{"RFU 	    \0", 0},
	{"ECSI      \0", 0},
	{"SCSI      \0", 0},
	{"CD Disable\0", 0},
};

#define R5_COM_CRC_ERROR        (1 << 15)	/* er, b */
#define R5_ILLEGAL_COMMAND      (1 << 14)	/* er, b */
#define R5_ERROR                (1 << 11)	/* erx, c */
#define R5_FUNCTION_NUMBER      (1 << 9)	/* er, c */
#define R5_OUT_OF_RANGE         (1 << 8)	/* er, c */
#define R5_STATUS(x)            (x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)  ((x & 0x3000) >> 12)	/* s, b */

#endif
