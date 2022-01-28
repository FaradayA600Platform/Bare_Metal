/* 
    porting function:
    printf(),atob(),gets()
*/
#ifndef _PORTING_H_
#define _PORTING_H_

//#define printf      fLib_printf
//#define gets        fLib_scanf
extern char *atob(char *str,unsigned int *ptrnum,unsigned int base,int seg);/* ascii to binary/no defaults (idtclib.c) */

#endif
