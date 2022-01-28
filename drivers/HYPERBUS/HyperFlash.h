#ifndef __HYPERFLASH_H__
#define __HYPERFLASH_H__

#include "types.h"
/*******************************************
 * for function prototype definition
 ********************************************/
extern BOOL hyperFlash_read(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length);
extern BOOL hyperFlash_write(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length);  
extern BOOL hyperFlash_erase_sector(uint32_t baseAddr, uint32_t offset);
extern BOOL hyperFlash_erase_chip(uint32_t baseAddr);
extern uint32_t hyperFlash_get_id(uint32_t baseAddr);
extern void hyperFlash_reset(uint32_t baseAddr);

#endif/* __HYPERFLASH_H__ */

