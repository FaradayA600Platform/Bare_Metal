#ifndef PORTME_FTUART010_H
#define PORTME_FTUART010_H

#include "autoconf.h"

/************************NonOS Lite************************/
#if defined(CONFIG_BSP_LITE)

#include <stdio.h>
#include <stdint.h>

#include "platform_AXI.h"
#include "dmac030.h"
#include "../drivers/ftuart010/ftuart010.h"

#define UART_CLOCK                  BOARD_UART_CLOCK
#define IP_COUNT                    2
#define printf                      prints

extern unsigned int Do_Delay(unsigned int num);
extern unsigned int inw(unsigned int *port);
extern void         outw(unsigned int *port,unsigned int data);

/************************NonOS A380************************/
#elif defined(CONFIG_BSP_A380)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "SoFlexible.h"
#include "../FTSCU010/pmu.h"

/************************NonOS A600************************/
#elif defined(CONFIG_PLATFORM_A600)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define IPMODULE                    UART
#define IPNAME                      FTUART010
#define FTDMAC030_REG_BASE          DMAC_FTDMAC030_1_PA_BASE

#include "SoFlexible.h"
#include "serial.h"

/************************Without BSP************************/
#else

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <time.h>

#include "ftuart010.h"

#define PARITY_NONE                 0
#define PARITY_ODD                  1
#define PARITY_EVEN                 2
#define PARITY_MARK                 3
#define PARITY_SPACE                4

#define UART_FTUART010_0_PA_BASE    0x90400000
#define UART_CLOCK                  18432000

#define UART_COUNT                  3
#define UART_FTUART010_COUNT        3
#define IP_pa_base                  UART_FTUART010_pa_base
#define IP_UART3                    9
#define IP_COUNT                    2

#define DEFAULT_CONSOLE             0x90400000

/* UART */
const unsigned UART_FTUART010_pa_base[UART_FTUART010_COUNT] = {
     0x90400000,
     0x90500000,
     0x90600000
};

extern unsigned int Do_Delay(unsigned int num);
extern unsigned int inw(unsigned int *port);
extern void         outw(unsigned int *port,unsigned int data);

#endif

#endif
