/*
** Copyright (C) ARM Limited, 2002. All rights reserved.
*/

/*
** This file contains re-implementations of functions whose
** C library implementations rely on semihosting.  
** This includes I/O and clocking functionality.
**
** Defining USE_SERIAL_PORT targets the I/O
** to the Integrator AP serial port A.  Otherwise, I/O is targeted
** to the debugger console.  
** 
** Defining USE_TARGET_CLOCK targets the clocking mechanism to the
** Integrator core module reference clock.  Otherwise, clocking is 
** timed off the host system clock.  
**
** In an image built to run on a standalone target USE_SERIAL_PORT
** USE_HOST_CLOCK must be defined.  
**
*/

#include <stdio.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

/*
** Importing __use_no_semihosting ensures that our image doesn't link
** with any C Library code that makes direct use of semihosting. 
**
** Build with STANDALONE to include this symbol.
**
*/

#ifdef STANDALONE
#define USE_SERIAL_PORT
#define USE_TARGET_CLOCK
#pragma import(__use_no_semihosting)
#endif


/*
** Retargeted I/O
** ==============
** The following C library functions make use of semihosting
** to read or write characters to the debugger console: fputc(),
** fgetc(), and _ttywrch().  They must be retargeted to write to
** the Integrator AP UART.  __backspace() must also be retargeted
** with this layer to enable scanf().  See the Compiler and
** Libraries Guide.
*/

/*
** These must be defined to avoid linking in stdio.o from the
** C Library
*/
struct __FILE { int handle;   /* Add whatever you need here */};
FILE __stdout;
FILE __stdin;


#ifdef USE_SERIAL_PORT
/* Following are defined in serial.c */
extern void init_serial_A(void);
extern void sendchar(unsigned char *ch);
extern unsigned char receive_char(void);

/*
** __backspace must return the last char read to the stream
** fgetc() needs to keep a record of whether __backspace was
** called directly before it
*/
int last_char_read;
int backspace_called;

int fputc(int ch, FILE *f)
{
    unsigned char tempch = ch;

    sendchar(&tempch);

    return ch;
}

int fgetc(FILE *f)
{
    unsigned char tempch;

    /* if we just backspaced, then return the backspaced character */
    /* otherwise output the next character in the stream */
    if (backspace_called == TRUE)
    {
      backspace_called = FALSE;
      return last_char_read;
    }

    tempch = receive_char();
    last_char_read = (int)tempch;       /* backspace must return this value */
    return tempch;
}

void _ttywrch(int ch)
{
    unsigned char tempch = ch;

    sendchar(&tempch);
}

/*
** The effect of __backspace() should be to return the last character
** read from the stream, such that a subsequent fgetc() will
** return the same character again.
*/

int __backspace(FILE *f)
{
    backspace_called = TRUE;
    return 0;
}

/* END of Retargeted I/O */
#endif        //end of '#ifdef USE_SERIAL_PORT'

/*
** Retargeted Clocking
** ===================
** The C library implementation of clock() uses the host system
** clock to measure time.  It has been retargeted to use a counter.
** incremented by an IRQ generated every 1/100 second by a timer on
** the Integrator\AP motherboard.
*/


#ifdef USE_TARGET_CLOCK
extern unsigned ref_clock(void);
extern unsigned init_clock(void);

clock_t clock(void)
{
    return (clock_t)ref_clock();
}

void _clock_init(void)
{
    init_clock();
}

/* END of Retargeted Clocking */

#endif        //end of '#ifdef USE_TARGET_CLOCK'

/*
** Exception Signaling and Handling
** ================================
** The C library implementations of ferror() uses semihosting directly
** and must therefore be retargeted.  This is a minimal reimplementation.  
** _sys_exit() is called after the user's main() function has exited.  The C library
** implementation uses semihosting to report to the debugger that the application has
** finished executing.  
*/


#ifdef STANDALONE

int ferror(FILE *f)
{
    return EOF;
}

void _sys_exit(int return_code)
{
    while(1);
}

#endif        //end of '#ifdef STANDALONE'
