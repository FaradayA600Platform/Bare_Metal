#include "SoFlexible.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool/utility.h"
#include "io.h"
#include "types.h"

#include "autoconf.h"

extern void FTAES020_QC_Main(void);
extern void Mem_QC_Main(void);
extern void FTDMAC020_QC_Main(void); 
extern void FTDMAC030_QC_Main(void);
extern void FTUART010_QC_Main(void);
extern void FTSSP010_QC_Main(void);
//extern void FTGMAC100_QC_main(void);
extern void FTGPIO010_QC_Main(void);
extern void FTIIC010_QC_Main(void);
extern void FTRTC011_QC_Main(void);
extern void FTWDT010_QC_Main();
extern void FTTMR010_QC_Main();
extern void FTKBC010_QC_Main();
extern void FTSDC021_QC_Main();
extern void FOTG210_DEV_QC_Main();
extern int OTG200_Host_main();
extern void FTSMC030_QC_Main();
extern void FTSPI020_QC_Main();
extern void FTLCDC210_QC_Main();
extern void PCIE_QC_Main();

struct burnin_cmd qc_a500_cmd_value[] = {

#ifdef CONFIG_FTAES020
	{"A/D ES QC", FTAES020_QC_Main},
#endif

#ifdef CONFIG_FTIIC010
	{"IIC010 QC", FTIIC010_QC_Main},
#endif

#ifdef CONFIG_FTLCDC200
	{"LCD QC", FTLCDC200_QC_Main},
#endif
	{"Mem QC", Mem_QC_Main},

	{"Timer QC", FTTMR010_QC_Main},
#ifdef CONFIG_FTSDC021
       {"SDC010 QC", FTSDC021_QC_Main},
#endif

#ifdef CONFIG_FTSCU010
	{"SCU QC", FTSCU010_QC_Main},
#endif

#ifdef CONFIG_FTUART010
	{"UART010 QC", FTUART010_QC_Main},
#endif
#ifdef CONFIG_FTDMAC020
	{"FTDMAC020 QC", FTDMAC020_QC_Main},
#endif

#ifdef CONFIG_FTSPI020
	{"FTSPI020 QC", FTSPI020_QC_Main},
#endif
	{"PCIE QC", PCIE_QC_Main},
	{"FTLCDC210 QC", FTLCDC210_QC_Main},
	{"", NULL}
};

//
struct burnin_cmd qc_a600_cmd_value[] = {

#ifdef CONFIG_FTAES020
	{"A/D ES QC", FTAES020_QC_Main},
#endif

#ifdef CONFIG_FTIIC010
	{"IIC010 QC", FTIIC010_QC_Main},
#endif

#ifdef CONFIG_FTLCDC200
	{"LCD QC", FTLCDC200_QC_Main},
#endif
	{"Mem QC", Mem_QC_Main},

#ifdef CONFIG_FTTMR010
	{"Timer QC", FTTMR010_QC_Main},
#endif

#ifdef CONFIG_FTSDC021
       {"SDC010 QC", FTSDC021_QC_Main},
#endif

#ifdef CONFIG_FTSCU010
	{"SCU QC", FTSCU010_QC_Main},
#endif

#ifdef CONFIG_FTUART010
	{"UART010 QC", FTUART010_QC_Main},
#endif
#ifdef CONFIG_FTDMAC020
	{"FTDMAC020 QC", FTDMAC020_QC_Main},
#endif

#ifdef CONFIG_FTSPI020
	{"FTSPI020 QC", FTSPI020_QC_Main},
#endif
	{"PCIE QC", PCIE_QC_Main},
	{"FTLCDC210 QC", FTLCDC210_QC_Main},
	{"", NULL}
};
//

#define COL_WIDTH			26				/// each col is 26 character
void A500_QC_Main()
{
#ifdef CONFIG_USE_FLIB_PRINTF
                mem_malloc_init();
#endif

               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8118, 0xff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x811c, 0xf);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8124, 0x7ff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8128, 0x7ff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8130, 0xffffffff);
       
		printf("A500 QC TEST\n",__func__);
		ManualTesting(qc_a500_cmd_value, COL_WIDTH, 1);    /* never return */
}

//
void A600_QC_Main()
{
#ifdef CONFIG_USE_FLIB_PRINTF
                mem_malloc_init();
#endif

               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8118, 0xff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x811c, 0xf);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8124, 0x7ff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8128, 0x7ff);
               vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8130, 0xffffffff);
       
		printf("A600 QC TEST\n",__func__);
		ManualTesting(qc_a600_cmd_value, COL_WIDTH, 1);    /* never return */
}
//
