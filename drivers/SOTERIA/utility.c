#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "utility.h"

static char *mainGetNextStringStart(char *str)
{
	/* find end of this word*/
	while(*str!=' ' && *str!='\t' && *str!=0)
	{
		str++;
	}
	/* find the next word */
	while(*str==' ' || *str=='\t')
	{
		str++;
	}
	return str;
}

unsigned long mainStringToNum(char *str, char **next)
{
	unsigned long	retVal;

	if (str[0] =='0' && str[1] == 'x')
		retVal = strtoul(&str[2], next, 16);
	else
		retVal = strtoul(str, next, 10);

	*next = mainGetNextStringStart(*next);
	return retVal;
}

BOOL mainCheckCmd(char *cmd, char **str)
{
	if(memcmp(*str, cmd, strlen(cmd)) == 0)
	{
		*str = mainGetNextStringStart(&((*str)[strlen(cmd)]));
		return TRUE;
	}
	return FALSE;
}

void mainMemDump(UINT32 addr, UINT32 len, UINT32 showAddr)
{
	UINT32	count = (len+3)/4;
	UINT32	*ptrMem = (UINT32 *)addr;
	char	tmpStr[100];

	while(count)
	{
		switch(count)
		{
			case 3:
				sprintf(tmpStr, "%08X  %08x %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1), *(ptrMem+2));
				count = 0;
				break;
			case 2:
				sprintf(tmpStr, "%08X  %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1));
				count = 0;
				break;
			case 1:
				sprintf(tmpStr, "%08X  %08x\n\r",(UINT32)showAddr,  *ptrMem);
				count = 0;
				break;
			default:
				sprintf(tmpStr, "%08X  %08x %08x %08x %08x\n\r",(UINT32)showAddr,  *ptrMem, *(ptrMem+1), *(ptrMem+2), *(ptrMem+3));
				ptrMem=ptrMem+4;
				showAddr+=16;
				count-=4;
				break;
		}
		fLib_printf("%s", tmpStr);
	}
}

void mainMemDumpByte(UINT32 addr, UINT32 len, UINT32 showAddr)
{
	int		i;
	UINT8	*ptrMem = (UINT8 *)addr;

	for(i=0; i<len; i++)
	{
		if((i%0x10) == 0x00)
			fLib_printf("%08X  ", showAddr);
		fLib_printf("%02x ", ptrMem[i]);
		if((i%0x10) == 0x0F)
			fLib_printf("\n");
		showAddr++;
	}
}
