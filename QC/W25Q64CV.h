#ifndef __FLASH_H__
#define __FLASH_H__

//#include "common.h"
/*** Define error code ***/
#define NO_ERROR		0
#define ERR_ARGUMENT	5
#define ERR_FLASH		6
#define ERR_FIRMWARE	16
#define ERR_DHMI		17
#define ERR_DRIVER		18


/*******************************************
 * for spi flash id definition
 ********************************************/
#define FLASH_WB_DEV			0xEF
#define FLASH_MXIC_DEV			0xC2
#define FLASH_IS_WB_DEV(x)		((x>>16)&0xFF == FLASH_WB_DEV)
#define FLASH_IS_MXIC_DEV(x)	((x>>16)&0xFF == FLASH_MXIC_DEV)

#define WB_W25Q128BVFG_ID		0x1840
#define WB_W25Q64CV_ID_9F		0x1740
#define WB_W25Q256JW_ID_9F	0x1960

#define WB_W25P16_ID_9F			0x1520
#define WB_W25P16_ID_90			0x14
#define MX_MX25L12845EM1		0x1820
#define MX_MX25L6405D			0x1720
#define INVALID_CHIP_ID			0xFFFF
#define INVALID_MANU_ID			0xFF

/*******************************************
 * for function prototype definition
 ********************************************/
#endif/* __FLASH_H__ */

