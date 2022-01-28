
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool/utility.h"
#include "io.h"
#include "types.h"

#include "autoconf.h"
#include "../drivers/HYPERBUS/hyperbusi.h"

extern void Timer_Test_Main(void);
extern void FTPWMTMR_Test_Main(void);
extern void FTAES020_Test_Main(void);
extern void flash_test_main(void);
extern void FTSMC030_main(void);
extern void FTGPIO010_Test_Main(void);
extern void FTIIC010_Test_Main(void);
extern void FTKBC010_Test_Main(void);
extern void FTSDC021_main(void);
extern int FTLCDC210_Test_Main(void);
extern void FTSSP010_Test_Main(void);
extern void FTSPI020_main(void);
extern void FTUART010_Test_Main(void);
extern void FTWDT011_main(void);
extern void Mem_Test_Main(void);
extern void mem_addr_test();
extern void CPU_AllCache_Enable(void);
extern void CPU_AllCache_Disable(void);
extern void CPU_ACP_Test(void);
extern void Switch_LCD(void);
extern void LCD_clock(void);
//extern void core_main(void);
extern void dhry_main(void);
extern void jiffies_test(void);

extern int FTDMAC020_main();
extern int FTDMAC030_main();
extern void neon_main(void);
extern void FTGMAC030_main(void);
extern void SS_main(void);
extern void DDR_interleave_main(void);

extern void freeRTOS_main(void);

struct burnin_cmd Legacy_cmd_value[] = {


//#ifdef CONFIG_FTTIMER
#ifdef CONFIG_FTTMR010
	{"Timer Test", Timer_Test_Main},
#endif

#ifdef CONFIG_FTPWMTMR010
	{"FTPWMTMR010 Test", FTPWMTMR_Test_Main},
#endif

#ifdef CONFIG_FTAES020
	{"A/D ES Test", FTAES020_Test_Main},
#endif

#ifdef CONFIG_FTGPIO010
	{"GPIO Test", FTGPIO010_Test_Main},
#endif
#ifdef CONFIG_FTIIC010
	{"IIC010 Test", FTIIC010_Test_Main},
#endif

#ifdef CONFIG_FTSDC021
	{"SDC021 Test", FTSDC021_main},
#endif

#ifdef CONFIG_FTSSP010
	{"SSP010 Test", FTSSP010_Test_Main},
#endif

#ifdef CONFIG_FTSPI020
	{"SPI020 Test", FTSPI020_main},
#endif

#ifdef CONFIG_FTUART010
	{"UART010 Test", FTUART010_Test_Main},
#endif

#ifdef CONFIG_FTWDT011
	{"WDT011 Test", FTWDT011_main},
#endif

#ifdef CONFIG_FTDMAC020
	{"DMA020 Test", FTDMAC020_main},
#endif

#ifdef CONFIG_FTDMAC030
	{"DMA030 Test", FTDMAC030_main},
#endif

	{"Mem Test", Mem_Test_Main},

#ifdef CONFIG_MMU
//	{"Enable Cache", CPU_AllCache_Enable},
//	{"Disable Cache", CPU_AllCache_Disable},
#endif

#ifdef CONFIG_DHRY
 	{"Dhrystone", dhry_main},
#endif

//#ifdef CONFIG_COREMARK
// 	{"CoreMark", core_main},
//#endif

#ifdef CONFIG_FTLCD210 
 	{"FTLCDC210", FTLCDC210_Test_Main},
#endif

#ifdef CONFIG_FTGMAC030
    {"FTGMAC030 test", FTGMAC030_main},
#endif

#ifdef CONFIG_HYPERBUS
	{"HYPERBUS test", HYPERBUS_Test_Main},
#endif

#if CONFIG_JIFFIES
 	{"jiffies test", jiffies_test},
#endif

#ifdef CONFIG_NEON_CPAT
    {"NEON c pattern test", neon_main},
#endif

#ifdef CONFIG_SOTERIA
	{"SOTERIA test", SS_main},
#endif

#ifdef CONFIG_DDR_INTERLEAVE
	{"DDR interleave test", DDR_interleave_main},
#endif

#ifdef CONFIG_FREERTOS
	{"FreeRTOS test", freeRTOS_main},
#endif
	{"", NULL}
};

#define COL_WIDTH			26				/// each col is 26 character
void legacy_Main()
{
    double testing;
    int binary;
#ifdef CONFIG_USE_FLIB_PRINTF
                mem_malloc_init();
#endif

		printf("legacy_main\n",__func__);

		ManualTesting(Legacy_cmd_value, COL_WIDTH, 1);    /* never return */
}
