#ifndef PLATFORM_A500_H
#define PLATFORM_A500_H

#define FREE_MEM_BASE			CONFIG_STACK_ADDR
//#define FREE_MEM_BASE			0x41000000
#define DEFAULT_CONSOLE                 UART_FTUART010_PA_BASE  
#define DEFAULT_CONSOLE_BAUD            BAUD_115200
//#define DEFAULT_CONSOLE_BAUD            BAUD_57600
//#define DEFAULT_CONSOLE_BAUD            BAUD_19200
#define UART_CLOCK   			18432000
//#define UART_CLOCK   			20000000

#endif
