/*------------------------------------------------------------------------------
HyperFlash.c

copyright 2019 Faraday technology corporation.
------------------------------------------------------------------------------*/
/* include files */
#include "types.h"
#include "S29GLxxxS.h"
#include "HyperFlash.h"


BOOL hyperFlash_read(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length)
{
			FLASHDATA *bufPtr = (FLASHDATA *)buf;		
	
	
			while(length){								
				*bufPtr++ = lld_ReadOp((FLASHDATA *)(baseAddr+offset), 0);				
				length -= 2;
				offset += 2;
			}
			
			return TRUE;
}
 
BOOL hyperFlash_write(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length)
{
			FLASHDATA *bufPtr = (FLASHDATA *)buf;	

			
			while(length){
				if(lld_ProgramOp((FLASHDATA *)baseAddr, offset, *bufPtr++) != DEV_NOT_BUSY)
					return FALSE;
				
				length--;
				offset++;			
			}
			
			return TRUE;
}

BOOL hyperFlash_erase_sector(uint32_t baseAddr, uint32_t offset)
{
	if(lld_SectorEraseOp((FLASHDATA *)baseAddr, offset) != DEV_NOT_BUSY)
		return FALSE;
	else
		return TRUE;
}

BOOL hyperFlash_erase_chip(uint32_t baseAddr)
{
	if(lld_ChipEraseOp((FLASHDATA *)baseAddr) != DEV_NOT_BUSY)
		return FALSE;
	else
		return TRUE;
}

uint32_t hyperFlash_get_id(uint32_t baseAddr)
{
	return lld_GetDeviceId((FLASHDATA *)baseAddr);
}

void hyperFlash_reset(uint32_t baseAddr)
{
	lld_ResetCmd((FLASHDATA *)baseAddr);
}
