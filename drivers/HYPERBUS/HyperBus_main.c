/*------------------------------------------------------------------------------
flash.c

copyright 2010-2015 Faraday technology corporation.
------------------------------------------------------------------------------*/
/* include files */
//#include <intrins.h>
//#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "io.h"
//#include "flib.h"
#include "hyperbusi.h"
#include "HyperFlash.h"
//#include "DrvUART010.h"
//#include "utility.h"


#define HYPERBUS_BASE_ADDR	 0x28800000
#define HYPERRAM_BASE_ADDR   0x3C000000
#define HYPERFLASH_BASE_ADDR 0x38000000

#define HYPERRAM_CS			CS1_SEL
#define HYPERFLASH_CS		CS0_SEL

#define RW_LEN	1024//256

UINT8 rw_buf[RW_LEN];

#define HYPERFLASH_RW_BUFFER_ADDR		0x88000000//0x10000000

//
//extern UINT32 makeargs(INT8 * cmd, INT32 * argcptr, INT8 *** argvptr);
//extern UINT32 ExecCmd(UINT8 * cmdline);
//

void dumpMenu(void)
{
	fLib_printf("+-----------------------------------------+\n");
	fLib_printf("?) This help menu\n");
  /* Id commands used to get device identification from either ram (HyperRAM) or flash (HyperFlash) */
	fLib_printf("id [ram|flash]\n");
	/* Read command options:	 
   * ram:         HyperRAM access
	 *              <addr> should be an offset inside HyperRAM
	 *              <length> should be a multiple of 4		 	
	 * flash:       HyperFlash access 
	 *              <addr> should be an offset inside HyperFlash	 	 
	 *              <length> should be a multiple of 4		 	   
	 */
	fLib_printf("read <addr> <length> [ram|flash]\n");	
	/* Write command options:
	 * ram:         HyperRAM access
	 *              <addr> should be an offset inside HyperRAM
	 *              <val> should be 32 bits in hexadecimal, which will be writeen in periodic order, depending on <length> 	 	
   *              <length> should be a multiple of 4
	 *              <inc> means 0 ~ 255 will be written in ascending order (i.e., 0, 1, 2, ...), depending on <length>
	 *              <dec> means 0 ~ 255 will be written in descending order (i.e., 255, 254, 253, ...), depending on <length> 
	 * flash:       HyperFlash access 
	 *              <addr> should be an offset inside HyperFlash and a multiple of 2
	 *              <val> should be 32 bits in hexadecimal, which will be written in periodic order, depending on <length>	 	
   *              <length> should be a multiple of 4
	 *              <inc> means 0 ~ 255 will be written in ascending order (i.e., 0, 1, 2, ...), depending on <length>
	 *              <dec> means 0 ~ 255 will be written in descending order (i.e., 255, 254, 253, ...), depending on <length>    
	 */
	fLib_printf("write <addr> [<val>|inc|dec] <length> [ram|flash]]\n");		
	/* Erase commands used to erase all HyperFlash chip or a sector by address
	 * <addr> should be a sector address 	 
	 */
	fLib_printf("erase [<addr>|chip]\n");	
	fLib_printf("+-----------------------------------------+\n");
}

#if 0
void hyperBus_read(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length)
{
			uint32_t *bufPtr = (uint32_t *)buf;
	
	
			while(length){								
				*bufPtr++ = HYPERBUSI_REG_READ(baseAddr, offset);				 
				length -= 4;
				offset += 4;
			}
}
 
void hyperBus_write(uint32_t baseAddr, uint32_t offset, uint8_t *buf, uint32_t length)
{
			uint32_t *bufPtr = (uint32_t *)buf;
	
	
			while(length)
			{				
				HYPERBUSI_REG_WRITE(baseAddr, offset, *bufPtr++);				
				length -= 4;
				offset += 4;				
			}
}

BOOL hyperbusReadCmd(char *cmd)
{
	uint32_t	offset;
	uint32_t	length;
	uint32_t	readLen;	
	uint8_t		*buf;	
	
	
	/* get read parameters */
	if(cmd[0] == 0)
		return FALSE;

		offset = mainStringToNum(cmd, &cmd);
		
	if(cmd[0] == 0)
		return FALSE;
	
			length = mainStringToNum(cmd, &cmd);
			
			if(length % 4) 
			{
					fLib_printf("<length> should be a multiple of 4\n");
					return FALSE;
			}
			
			buf = rw_buf;
			
	if(cmd[0] == 0)
		return FALSE;
	
	if(mainCheckCmd("ram", &cmd) == TRUE)
			{				
			while(length)
				{
				readLen = Min(length, RW_LEN);							
				memset(buf, 0, readLen);						
				hyperBus_read(HYPERRAM_BASE_ADDR, offset, buf, readLen);
				DumpData(buf, offset, readLen);
				length -= readLen;
				offset += readLen;
				}
				}
				else if(mainCheckCmd("flash", &cmd) == TRUE)
				{						
						while(length)
						{
								readLen = Min(length, RW_LEN);							
				memset(buf, 0, readLen);
								hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
				DumpData(buf, offset, readLen);
								length -= readLen;
								offset += readLen;
						}
			}
			else
				return FALSE;
			
			return TRUE;
		}		

BOOL hyperbusWriteCmd(char *cmd)
{
	uint32_t	offset;
	uint32_t	length, idx;
	uint32_t	wLen;
  uint32_t  wVal;	
	uint8_t		*bufPtr = NULL;	
	
	
	/* get write parameters */
	if(cmd[0] == 0)
		return FALSE;
	
		offset = mainStringToNum(cmd, &cmd);		
		
	if(cmd[0] == 0)
		return FALSE;
		
			if(mainCheckCmd("inc", &cmd) == TRUE)
			{
				for(idx = 0; idx < RW_LEN; idx++)
				{
					rw_buf[idx] = idx;
				}				
			}
			else if(mainCheckCmd("dec", &cmd) == TRUE)
			{
				for(idx = 0; idx < RW_LEN; idx++)
				{
					rw_buf[idx] = RW_LEN-idx-1;
				}				
			}
			else			
			{
				wVal = mainStringToNum(cmd, &cmd);		
				bufPtr = (uint8_t *)&wVal;
			}		

	if(cmd[0] == 0)
						return FALSE;				
					
	length = mainStringToNum(cmd, &cmd);		
					
	if(length % 4) 
				{
		fLib_printf("<length> should be a multiple of 4\n");
						return FALSE;
				}					
					
					if(bufPtr)						
					{						
		wLen = Min(length, RW_LEN);	
		
		for(idx = 0; idx < wLen; idx++)
						{							
							rw_buf[idx] = *bufPtr++;						
							
							if(idx % 4 == 3)
								bufPtr -= 4;						
						}					
					}
					
					bufPtr = rw_buf;					
					
	if(cmd[0] == 0)			
		return FALSE;
	
						if(mainCheckCmd("ram", &cmd) == TRUE)
						{							
							while(length)
							{							
								wLen = Min(length, RW_LEN);							
								hyperBus_write(HYPERRAM_BASE_ADDR, offset, bufPtr, wLen);															
								length -= wLen;
								offset += wLen;
							}
						}	
						else if(mainCheckCmd("flash", &cmd) == TRUE)
						{
							while(length)
							{
					if(offset % 2) 
					{
						fLib_printf("<addr> should be a multiple of 2\n");
						return FALSE;
					}
								
								wLen= Min(length, RW_LEN);
								
					if(hyperFlash_write(HYPERFLASH_BASE_ADDR, offset/2, bufPtr, wLen/2) != TRUE)
								{
									fLib_printf("HyperFlash program failed at offset %x\n", offset);
									return FALSE;
								}
								
								length -= wLen;
								offset += wLen;
							}
			}			
	else
	return FALSE;	
	
	return TRUE;	
}

BOOL hyperbusEraseCmd(char *cmd)
{
	uint32_t	offset;
	
	
	/* get erase parameters */
	if(cmd[0] != 0)
	{			
		if(mainCheckCmd("chip", &cmd) == TRUE)
		{
			return hyperFlash_erase_chip(HYPERFLASH_BASE_ADDR);
		}
		else
		{
			offset = mainStringToNum(cmd, &cmd);				
			return hyperFlash_erase_sector(HYPERFLASH_BASE_ADDR, offset/2);
		}		
	}			
	
	return FALSE;	
}

BOOL hyperbusGetId(char *cmd)
{
	DWORD	orgReg, val = 0;
	
	
	if(cmd[0] != 0)
	{			
		if(mainCheckCmd("ram", &cmd) == TRUE)
		{
			if(getHYPERBUSI_MCR_CRT(HYPERBUS_BASE_ADDR, &orgReg, HYPERRAM_CS) == NO_ERROR)
			{
				/* Set CR space to make CA[46] = 1 */
				if(setHYPERBUSI_MCR_CRT(HYPERBUS_BASE_ADDR, 1, HYPERRAM_CS) == NO_ERROR)
				{
					val = HYPERBUSI_REG_READ(HYPERRAM_BASE_ADDR, 0);
					fLib_printf("HyperRAM ID: %x\n", val);
					
					if(setHYPERBUSI_MCR_CRT(HYPERBUS_BASE_ADDR, orgReg, HYPERRAM_CS) == NO_ERROR)
					{
						return TRUE;
					}
				}
			}			
		}
		else if(mainCheckCmd("flash", &cmd) == TRUE)
		{	
			DWORD val = hyperFlash_get_id(HYPERFLASH_BASE_ADDR);
			fLib_printf("HyperFlash ID: %x\n", val);
			return TRUE;
		}		
	}

	return FALSE;
}

void executeCmd(char *cmd)
{
	char		*tmpStr;
	BOOL	retVal=TRUE;
  
	
	tmpStr = cmd;
	
	if(tmpStr[0] == 0)
		;/* Do nothing for NULL string */
	else if(mainCheckCmd("?", &tmpStr) == TRUE)
		dumpMenu();
	else if(mainCheckCmd("id", &tmpStr) == TRUE)
	{		
		retVal = hyperbusGetId(tmpStr);
	}
	else if(mainCheckCmd("read", &tmpStr) == TRUE)
	{
		retVal = hyperbusReadCmd(tmpStr);
	}		
	else if(mainCheckCmd("write", &tmpStr) == TRUE)
	{
		retVal = hyperbusWriteCmd(tmpStr);
	}	
	else if(mainCheckCmd("erase", &tmpStr) == TRUE)
	{
		retVal = hyperbusEraseCmd(tmpStr);
	}	
	else
		fLib_printf("Unknown CMD %s\n", cmd);

	if(retVal == FALSE)
		fLib_printf("Fail to run CMD %s\n", cmd);
}
#endif
void hyperBusInit(void)
{
	/*** Init hyperFlash ***/
	/* setup MBR */
	setHYPERBUSI_MBR(HYPERBUS_BASE_ADDR, HYPERFLASH_BASE_ADDR, HYPERFLASH_CS);		
	/* setup MCR */
	setHYPERBUSI_MCR(HYPERBUS_BASE_ADDR, 0x0, HYPERFLASH_CS);
	/* setup timing */
	setHYPERBUSI_MTR(HYPERBUS_BASE_ADDR, 0x11111104, HYPERFLASH_CS);
	
	/*** Init hyperRAM ***/
	/* setup MBR */
	//setHYPERBUSI_MBR(HYPERBUS_BASE_ADDR, HYPERRAM_BASE_ADDR, HYPERRAM_CS);
	/* setup MCR */
	//setHYPERBUSI_MCR(HYPERBUS_BASE_ADDR, 0x10, HYPERRAM_CS);
	/* setup timing */
	//setHYPERBUSI_MTR(HYPERBUS_BASE_ADDR, 0x11111102, HYPERRAM_CS);
}


void HYPERBUS_Test_Main(void)
{
	unsigned char	getCmd[MAX_CMD_LEN];

	INT32	orgReg, tmp_val, val = 0;
	
	uint32_t	test_t1, test_t2;
	uint32_t	offset, wLen;
	uint32_t	length, idx;
	uint32_t	readLen;
	uint8_t		*buf;
	uint8_t		*bufPtr = NULL;
	
	//PinMux_UART010(0);
	/* TODO: PinMux HyperBus*/
	//PinMux_HyperBus(0);
//#if defined(__MCU_FA606TE__) || defined(__MCU_N25F__)
//	fLib_Int_Init();
//#endif	
	
	
	
	//fLib_SerialInit(DEBUG_CONSOLE,DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);
	fLib_printf("\n\n\rHyperBus Revision: %d\n\r", HYPERBUSI_IP_VER);
	//2020-1022, Terry ask for reset Hyperbus IP test
	//tmp_val = readl(0x2AE0006C);
	//fLib_printf("\n\n\r0x2EA0006C: %X\n\r", tmp_val);
	//tmp_val &= ~0x00000010;
	//writel(tmp_val, 0x2AE0006C);
	//tmp_val |= 0x10;
	//writel(tmp_val, 0x2AE0006C);
	//fLib_printf("\n\n\r111111\n\r");
	//2020-1022, Terry ask for reset Hyperbus IP test
	//for (test_t1=0; test_t1<500000; test_t1++) ;
	hyperBusInit();
	
	//hyperFlash_reset(HYPERFLASH_BASE_ADDR);
	
	// device id test
	//for (test_t1=0; test_t1<500000; test_t1++) ;
	fLib_printf("Get HyperFlash ID\n");
	val = hyperFlash_get_id(HYPERFLASH_BASE_ADDR);
	fLib_printf("HyperFlash ID: %x\n", val);
	
	// Loading IPL bin file in 0x88000000 by using ICE and run below pattern to write into hyper flash to run boot by hyper flash test.
	#if 0
	fLib_printf("pre read mem\n");
	buf = (INT8 *) HYPERFLASH_RW_BUFFER_ADDR;
	offset = 0;
	length = 0x3000;
	
	while(length)
	{
		readLen = 0x200;							
		//memset(buf, 0, readLen);
		//hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
		DumpData(buf, offset, readLen);
		length -= readLen;
		offset += readLen;
		buf += readLen;
	}
	
	// erase test
	fLib_printf("Erase test\n");
	hyperFlash_erase_chip(HYPERFLASH_BASE_ADDR);
	
	length = 0x3000;
	
	bufPtr = (uintptr_t) HYPERFLASH_RW_BUFFER_ADDR;
	//bufPtr = (INT8 *) HYPERFLASH_RW_BUFFER_ADDR;
	
	offset = 0;
		
	while(length)
	{		
		//wLen= Min(length, RW_LEN);
		wLen = 0x200;
				
		if(hyperFlash_write(HYPERFLASH_BASE_ADDR, offset/2, bufPtr, wLen/2) != TRUE)
		{
			fLib_printf("HyperFlash program failed at offset %x\n", offset);			
		}
				
		length -= wLen;
		offset += wLen;
		bufPtr += wLen;
	}
	
	fLib_printf("verify\n");
	buf = rw_buf;
	offset = 0;
	length = 0x3000;
	
	while(length)
	{
		readLen = 0x200;							
		memset(buf, 0, readLen);
		hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
		DumpData(buf, offset, readLen);
		length -= readLen;
		offset += readLen;
		buf	   += readLen;
	}
	//while(1);
	#endif	
	
	// read test, 0~0x1000
	fLib_printf("Read test 1\n");
	buf = rw_buf;
	offset = 0;
	length = 0x200;
	
	while(length)
	{
		readLen = 0x200;							
		memset(buf, 0, readLen);
		hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
		DumpData(buf, offset, readLen);
		length -= readLen;
		offset += readLen;
	}
	// read test, 0~0x1000
	
	// write test, 0~0x1000
	fLib_printf("Write test\n");
	bufPtr = rw_buf;
	offset = 0;
	length = 0x400;
	for(idx = 0; idx < RW_LEN; idx++)
	{
		if(idx < 0x100)
		{
			rw_buf[idx] = idx;
		}
		else if(idx < 0x200)
		{
			rw_buf[idx] = 0xFF-(idx - 0x100);
		}
		else if(idx < 0x300)
		{
			rw_buf[idx] = 0xA5;
		}
		else
		{
			rw_buf[idx] = 0x5A;
		}
//		printf("%2X ", rw_buf[idx]);
//		if((idx+1)%16 == 0)
//			printf("\n");
	}
	
	while(length)
	{		
		//wLen= Min(length, RW_LEN);
		wLen = 0x400;
				
		if(hyperFlash_write(HYPERFLASH_BASE_ADDR, offset/2, bufPtr, wLen/2) != TRUE)
		{
			fLib_printf("HyperFlash program failed at offset %x\n", offset);			
		}
				
		length -= wLen;
		offset += wLen;
	}
	// write test, 0~0x1000
	
	// read test, 0~0x1000
	fLib_printf("Read test\n");
	buf = rw_buf;
	offset = 0;
	length = 0x400;
	
	while(length)
	{
		readLen = 0x400;							
		memset(buf, 0, readLen);
		hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
		DumpData(buf, offset, readLen);
		length -= readLen;
		offset += readLen;
	}
	// read test, 0~0x1000
	
	// erase test
	fLib_printf("Erase test\n");
	hyperFlash_erase_chip(HYPERFLASH_BASE_ADDR);
	// erase test
	
	// read test, 0~0x1000
	fLib_printf("Read ae test\n");
	buf = rw_buf;
	offset = 0;
	length = 0x400;
	
	while(length)
	{
		readLen = 0x400;							
		memset(buf, 0, readLen);
		hyperFlash_read(HYPERFLASH_BASE_ADDR, offset, buf, readLen);													
		DumpData(buf, offset, readLen);
		length -= readLen;
		offset += readLen;
	}	
	// read test, 0~0x1000
	
	//dumpMenu();
	//fLib_printf("CMD> ");
	
	//while(1)
	//{
		//mainGetUartCmd(getCmd);
		//fLib_scanf(getCmd);
		//executeCmd(getCmd);
		//fLib_printf("CMD> ");
	//}
}

void DumpData(INT8U *pp, INT16U start_addr, INT32U size)
{
    INT32U i=0;
	fLib_printf("\n-------------------------------------------------------------------");
	do
	{                        
		if((i%16)==0)
		{
			fLib_printf("\n");
			fLib_printf("[0x%04x]:",start_addr);
			start_addr+=16;
		}
		fLib_printf("0x%02x ", *(pp++));
		i++;
	}while((pp!=NULL)&&(i<size));
	fLib_printf("\n");
}

