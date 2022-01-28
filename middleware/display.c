#include <stdio.h>

#include "types.h"


//extern void fLib_PutSerialChar(int,char);
/* proc=0,1,2,3 */
unsigned int proc=0;

#ifdef USE_FLIB_PRINTF
extern char semihost_detect;
#endif
extern UINT32 DebugSerialPort;
void display_process(unsigned int addr)
{
#ifdef USE_FLIB_PRINTF
    if(semihost_detect)
        return;
#endif
    if(proc==3)
        proc=0;
    else
        proc++;
    
    if(addr!=0)
    {
//#ifdef USE_FLIB_PRINTF
        fLib_printf("\r0x%x  ",addr);
//#else
//        printf("\r0x%x  ",addr);
//        fflush(stdout);
//#endif
    }    
    else
    {
        switch(proc)
        {
            case 0:
                fLib_PutSerialChar(DebugSerialPort,'\r');
                fLib_PutSerialChar(DebugSerialPort,'|');
                break;
            case 1:
                fLib_PutSerialChar(DebugSerialPort,'\r');
                fLib_PutSerialChar(DebugSerialPort,'/');
                break;        
            case 2:
                fLib_PutSerialChar(DebugSerialPort,'\r');
                fLib_PutSerialChar(DebugSerialPort,'-');
                break;        
            case 3:
                fLib_PutSerialChar(DebugSerialPort,'\r');
                fLib_PutSerialChar(DebugSerialPort,0x5c);
                break;        
            default:
                while(1)
                    ;
        }
    }
}

