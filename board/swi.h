/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:swi.h                                                               *
* Description: SWI define                                                  *
* Author: Fred Chien                                                       *
****************************************************************************/



#ifndef _SWI_H
#define _SWI_H

#define SWI_SAVED_REG_NUM			15

/***************************************************************************/
/*                              SWI numbers                                */
/***************************************************************************/

#define SWI_WriteC                 	0x00
#define SWI_Write0                 	0x02
#define SWI_ReadC                  	0x04
#define SWI_CLI                    	0x05
#define SWI_GetEnv                 	0x10
#define SWI_Exit                   	0x11
#define SWI_EnterOS                	0x16

#define SWI_GetErrno               	0x60
#define SWI_Clock                  	0x61
#define SWI_Time                   	0x63
#define SWI_Remove                 	0x64
#define SWI_Rename                 	0x65
#define SWI_Open                   	0x66

#define SWI_Close                  	0x68
#define SWI_Write                  	0x69
#define SWI_Read                   	0x6a
#define SWI_Seek                   	0x6b
#define SWI_Flen                   	0x6c

#define SWI_IsTTY                  	0x6e
#define SWI_TmpNam                 	0x6f
#define SWI_InstallHandler         	0x70
#define SWI_GenerateError          	0x71


#define SysSWI 						(0x123456)
#define SysSWI_16BIT				(0xAB)

/* The reason codes: */
#define SysSWI_Reason_Open				(0x01)
#define SysSWI_Reason_Close				(0x02)
#define SysSWI_Reason_WriteC			(0x03)
#define SysSWI_Reason_Write0			(0x04)
#define SysSWI_Reason_Write				(0x05)
#define SysSWI_Reason_Read				(0x06)
#define SysSWI_Reason_ReadC				(0x07)
#define SysSWI_Reason_IsTTY				(0x09)
#define SysSWI_Reason_Seek				(0x0A)
#define SysSWI_Reason_FLen				(0x0C)
#define SysSWI_Reason_TmpNam			(0x0D)
#define SysSWI_Reason_Remove			(0x0E)
#define SysSWI_Reason_Rename			(0x0F)
#define SysSWI_Reason_Clock				(0x10)
#define SysSWI_Reason_Time				(0x11)
#define SysSWI_Reason_System			(0x12)
#define SysSWI_Reason_Errno				(0x13)
#define SysSWI_Reason_GetCmdLine 		(0x15)
#define SysSWI_Reason_HeapInfo 			(0x16)
#define SysSWI_Reason_EnterSVC 			(0x17)
#define SysSWI_Reason_ReportException 	(0x18)
#define ADP_Stopped_ApplicationExit 	((2 << 16) + 38)
#define ADP_Stopped_RunTimeError 		((2 << 16) + 34)

extern void SysSWIHandler(UINT32 *Instru, UINT32 PrevIs16BitMode, UINT32 *ArguPtr);
extern UINT32 SysSWICall(UINT32 SysCode, UINT32 *ArguPtr);
extern void SysHeapInfo(UINT32 *Argu);

#endif
