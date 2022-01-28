/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:swi.c                                                               *
* Description: SWI library routine                                         *
* Author: Fred Chien                                                       *
****************************************************************************/
#include "SoFlexible.h"
#include "swi.h"

extern UINT32 TopOfHeap;
extern UINT32 BaseOfHeap;
extern UINT32 TopOfStack;
extern UINT32 BaseOfStack;

/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////

void SysSWIHandler(UINT32 *InstruAddr, UINT32 PrevIs16BitMode, UINT32 *ArguPtr)
{
	UINT32 SWIComment;
	
	if(PrevIs16BitMode)
		SWIComment = *((UINT16 *)InstruAddr) & 0x00ff;
	else
		SWIComment = *InstruAddr & 0x00ffffff;
		
	switch(SWIComment)
	{
		case SysSWI:
		case SysSWI_16BIT:
			SysSWICall(ArguPtr[0], ArguPtr);
			break;
		default:
			break;
	}
}

char semihost_detect=1;
UINT32 SysSWICall(UINT32 SysCode, UINT32 *RegPtr)
{
	UINT32 *EntryPtr;
	semihost_detect=0;
	
	EntryPtr = (UINT32 *)RegPtr[1];
	switch(SysCode)
	{
		case SysSWI_Reason_Open:
			RegPtr[0] = (UINT32)fLib_OpenFile((char *)EntryPtr[0], EntryPtr[1], EntryPtr[2]);
			break;
		case SysSWI_Reason_Close:
			RegPtr[0] = (UINT32)fLib_CloseFile(EntryPtr[0]);
			break;
		case SysSWI_Reason_WriteC:
			fLib_DebugPrintChar(*(char *)EntryPtr);
			break;
		case SysSWI_Reason_Write0:
			fLib_DebugPrintString((char *)EntryPtr[0]);
			break;
		case SysSWI_Reason_Write:
			RegPtr[0] = (UINT32)fLib_WriteFile(EntryPtr[0], (char *)EntryPtr[1], EntryPtr[2]);
			break;
		case SysSWI_Reason_Read:
			RegPtr[0] = (UINT32)fLib_ReadFile(EntryPtr[0], (char *)EntryPtr[1], EntryPtr[2]);
			break;
		case SysSWI_Reason_ReadC:
		    *(char *)EntryPtr=fLib_DebugGetChar();
			break;
		case SysSWI_Reason_IsTTY:
			RegPtr[0] = fLib_IsTTYFile(EntryPtr[0]);
			break;
		case SysSWI_Reason_Seek:
			RegPtr[0] = (UINT32)fLib_SeekFile(EntryPtr[0], EntryPtr[1]);
			break;
		case SysSWI_Reason_FLen:
			RegPtr[0] = (UINT32)fLib_FileLen(EntryPtr[0]);
			break;
		case SysSWI_Reason_TmpNam:
			break;
		case SysSWI_Reason_Remove:
			RegPtr[0] = (UINT32)fLib_DeleteFile((char *)EntryPtr[0], EntryPtr[1]);
			break;
		case SysSWI_Reason_Rename:
			RegPtr[0] = (UINT32)fLib_RenameFile((char *)EntryPtr[0], EntryPtr[1], (char *)EntryPtr[2], EntryPtr[3]);
			break;
#ifdef CONFIG_FTTIMER
		case SysSWI_Reason_Clock:
			RegPtr[0] = fLib_CurrentT1Tick();
			break;
#endif // CONFIG_FTTIMER
		case SysSWI_Reason_Time:
			break;
		case SysSWI_Reason_System:
			break;
		case SysSWI_Reason_Errno:
			break;
		case SysSWI_Reason_GetCmdLine:
			EntryPtr[0] = NULL;
			EntryPtr[1] = 0; 
			RegPtr[0] = 0;
			break;
		case SysSWI_Reason_HeapInfo:
			SysHeapInfo((UINT32 *)EntryPtr[0]);
			break;
		case SysSWI_Reason_EnterSVC:
//			return SysEnterSVC(RegPtr);
			break;
		case SysSWI_Reason_ReportException:
			break;
		case ADP_Stopped_ApplicationExit:
			break;
		case ADP_Stopped_RunTimeError:
			break;
		default:
			break;
	}
	return 0;
}

/* far
heap base in r0

stack base in r1, that is, the highest address in the stack region

heap limit in r2

stack limit in r3, that is, the lowest address in the stack region.

r0 = Argu[0]
r1 = Argu[2]
r2 = Argu[1]
r3 = Argu[3]
*/
void SysHeapInfo(UINT32 *Argu)
{
/*
	Argu[0] = (UINT32)TopOfHeap;
	Argu[1] = (UINT32)BaseOfHeap;
	Argu[2] = (UINT32)TopOfStack;
	Argu[3] = (UINT32)BaseOfStack;
*/
	Argu[0] = (UINT32)BaseOfHeap;
	Argu[1] = (UINT32)TopOfHeap;
	Argu[2] = (UINT32)TopOfStack;
	Argu[3] = (UINT32)BaseOfStack;

}





 
