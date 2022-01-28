/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:serial.c                                                            *
* Description: serial library routine                                      *
* Author: Fred Chien                                                       *
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SoFlexible.h"
#include "system.h"

#include "serial.h"

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL       0x07

//used for CLI
#define CLI_PORT DebugSerialPort

//ycmo: for u-boot porting
int tstc () {
        return ((inw(CLI_PORT+SERIAL_LSR) & SERIAL_LSR_DR) != 0);
}

INT32 fLib_printf(const char *f, ...)	/* variable arguments */
{
    INT32 cnt,i;	    
    va_list arg_ptr;    
    char buffer[256];
    unsigned long flags;
      
	//save_flags_cli(flags);				
	
   
   	//put the character to buffer
   	va_start(arg_ptr, f);
   	cnt = vsprintf(&buffer[0], f, arg_ptr);
   	va_end(arg_ptr);   
	
   	//output the buffer
    i=0;
    while(buffer[i])	
    {
    	fLib_putchar(buffer[i]);
    	i++;
    }

	/* restore the previous mode */
	//restore_flags(flags);			
	
   
    return OK;
}

int fLib_scanf(char *buf)
{
    char    *cp;
    char    data;
    UINT32  count;
    count = 0;
    cp = buf;
    
    do
    {
        data = fLib_getchar();

        switch(data)
        {
            case RETURN_KEY:
                if(count < 256)
                {
                    *cp = '\0';
                    fLib_putchar('\n');
                }          
                break;
            case BACKSP_KEY:
            case DELETE_KEY:
                if(count)
                {
                    count--;
                    *(--cp) = '\0';
                    fLib_putstr("\b \b");
                }         
                break;
            default:         
                if( data > 0x1F && data < 0x7F && count < 256)
                {
                    *cp = (char)data;
                    cp++;
                    count++;
                    fLib_putchar(data);
                }
                break;
        }
    } while(data != RETURN_KEY);  
    
  return (count);
}
//for SWI call
void fLib_DebugPrintChar(INT8 ch)
{
    if(ch != '\0' && ch != '\n')
    {
		  fLib_PutSerialChar(DebugSerialPort, ch);
    }
    else if(ch == '\n')
    {
  	  fLib_PutSerialChar(DebugSerialPort, '\r');//CR
      fLib_PutSerialChar(DebugSerialPort, '\n');//LF
    }
}

void fLib_DebugPrintString(INT8 *str)
{
	while(*str)
	{
		fLib_DebugPrintChar(*str);
		str++;
	}
}

INT8 fLib_DebugGetChar(void)
{
	return fLib_GetSerialChar(DebugSerialPort);
}

UINT32 fLib_DebugGetUserCommand(UINT8 * buffer, UINT32 Len)
{
    int offset = 0, c;

    buffer[0] = '\0';
    while (offset < (Len - 1)) {
	c = fLib_GetSerialChar(DebugSerialPort);

	if (c == '\b')		//backspace
	{
	    if (offset > 0) {
		// Rub out the old character & update the console output
		offset--;
		buffer[offset] = 0;

		fLib_DebugPrintString("\b \b");
	    }
	} else if (c == DELETE_KEY)	//backspace
	{
	    if (offset > 0) {
		// Rub out the old character & update the console output
		offset--;
		buffer[offset] = 0;

		fLib_DebugPrintString("\b \b");
	    }
	}

	else {
	    if (c == '\r')
		c = '\n';	// treat \r as \n

	    fLib_PutSerialChar(DebugSerialPort, c);

	    buffer[offset++] = c;

	    if (c == '\n')
		break;
	}
    }

    buffer[offset] = '\0';

    return offset;
}

int fLib_atoi(char *string)
{
    int length;
    int retval = 0;
    int i;
    int sign = 1;

    length = strlen(string);
    for (i = 0; i < length; i++) {
        if (0 == i && string[0] == '-') {
            sign = -1;
            continue;
        }
        if (string[i] > '9' || string[i] < '0') {
            break;
        }
        retval *= 10;
        retval += string[i] - '0';
    }
    retval *= sign;
    return retval;
}
