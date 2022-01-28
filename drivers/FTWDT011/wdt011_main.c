#include <stdio.h>
#include    "SoFlexible.h"
//#include "WD_test.h"
//#include "fLib.h"
//#include "common.h"
#include "wdt011_main.h"
#include "wdt011.h"
//#include "interrupt.h"
#include "timer.h"

void PWMTMR1_IRQHandler(void);
void IWDT_IRQHandler();

#define SLOW_SYSTEM
#define USE_FLIB_PRINTF

#define IPMODULE 		WDT
#define IPNAME 			FTWDT010

uint32_t WD_ClkSrc = WdClkInt;
uint8_t WD_PreScaler = 4;
uint32_t WD_Clock = 500000; // 5M

uint32_t volatile msTicks;                       // Counter for millisecond Interval
volatile UINT32 OverflowFlag, UnderflowFlag, ExtSignalFlag;

static void iwdt_test_init(void) {
	OverflowFlag  = 0;
	UnderflowFlag = 0;
	ExtSignalFlag = 0;
}

void PWMTMR1_IRQHandler(void)
{
//printf("PWMTMR1_IRQHandler\n");
fLib_DisableIRQ(TIMER_FTPWMTMR010_IRQ0);
    fLib_Timer_IntClear(1);
fLib_EnableIRQ(TIMER_FTPWMTMR010_IRQ0);
    msTicks++;
}

void IWDT_IRQHandler()
{
    UINT32 intSts = fLib_WatchDog_GetGlobalIntrStatus();
    UINT8 i, ovfSts, udfSts;
printf("IWDT_IRQHandler intSts = 0x%x\n", intSts);
	  //fLib_WatchDog_ClearGlobalIntrStatus(intSts);
	  udfSts = (intSts & WdUDFIntrMask) >> UDFShiftBits;
	  ovfSts = (intSts & WdOVFIntrMask) >> OVFShiftBits;
	
    for (i = 0; i < DRVIWDT_MAX; i++) 
    {
        if (intSts & (1<<i)) 
        {
            fLib_printf("\n\rIWDT%d Interrupt ==> ", i);
            if (udfSts & (1<<i)) 
            {
                fLib_printf("underflow !!!\n");
                fLib_WatchDog_ClearUnderflowStatus((DRVIWDT) i);
                UnderflowFlag |= (1<<i);						
            }
            else if (ovfSts & (1<<i)) 
            {
                fLib_printf("overflow !!!\n");
                fLib_WatchDog_ClearOverflowStatus((DRVIWDT) i);
                OverflowFlag |= (1<<i);
            }
            else 
            {
                fLib_printf("[Error] Interrupt Status is mismatched with over/under flow status !!!\n");
            }
        }
    }
}

void WaitForTick (void)  {
    UINT32 curTicks;

        asm volatile ("wfi");
}

void delay_ms(int msec)
{
    for (;msec != 0; --msec)
    {
        WaitForTick();		
    }
}

void delay_s(int sec)
{
    for (;sec != 0; --sec)
    {   
        WaitForTick();		
    }
}

void WatchDogStart(DRVIWDT iwdt, UINT8 second)
{
    fLib_WatchDog_SetSignalLength(iwdt, 0xFF);            //signal asserting 256 clock cycles
    fLib_WatchDog_SetAutoReLoad(iwdt, WD_Clock/WD_PreScaler*second);
printf("WatchDogStart counter = 0x%x\n", inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_KR));
    fLib_WatchDog_Enable(iwdt);                           //Enable WD
}

void Reset_Board(DRVIWDT iwdt)
{
    fLib_WatchDog_SetSignalLength(iwdt, 0xFF);
    fLib_WatchDog_SetAutoReLoad(iwdt, 1);
    fLib_WatchDog_SysResetEnable(iwdt);
    fLib_WatchDog_Enable(iwdt);                           //Enable WD
}

// -----------------------------------------------------------------------------------------
// check whether watchdog will generate system interrupt (should within 2 sec) within 5 sec
// -----------------------------------------------------------------------------------------
int WatchDogOverflowSysIntTest(DRVIWDT iwdt)
{
    UINT32 nResultStatus;
    int result = 0;
    unsigned long start_jiffies = 0, end_jiffies = 0;
	
    fLib_printf("\r%s start ...\n", __func__);
    OverflowFlag = 0;
    nResultStatus = FALSE;
    fLib_WatchDog_SysIntEnable(iwdt);
    
    fLib_EnableIRQ(WDT_FTWDT011_IRQ);		

//    start_jiffies = msTicks;
    start_jiffies = jiffies();
    WatchDogStart(iwdt, 1);

    while (OverflowFlag == 0)
    {
//        if ( (msTicks - start_jiffies) >= (5*1000) /*JIFF_TO_SEC(jiffies - start_jiffies) >= 3*/ )	// 5 sec
        if ( (jiffies() - start_jiffies) >= (5*1000) /*JIFF_TO_SEC(jiffies - start_jiffies) >= 3*/ )	// 5 sec
        {
            break;
        }
    }
		fLib_WatchDog_ReProg(iwdt); // stop wdt
//    fLib_printf("iwdt%d overflow duration: %d ms ", iwdt, msTicks - start_jiffies);
    fLib_printf("iwdt%d overflow duration: %d ms ", iwdt, jiffies() - start_jiffies);    

    if ( (OverflowFlag & (1<<iwdt)) )
        nResultStatus = TRUE;

    fLib_WatchDog_SysIntDisable(iwdt);

    fLib_DisableIRQ(WDT_FTWDT011_IRQ);		

    if (nResultStatus == TRUE)
    {
        fLib_printf("==> [Pass!]\n");
        result = 1;
    }
    else
    {
        fLib_printf("==> [Fail!]\n");
        result = 0;
    }

    fLib_printf("\n%s end ...\n", __func__);
    return result;
}

int WatchDogUnderflowSysIntTest(DRVIWDT iwdt)
{
    UINT32 nResultStatus;
    int result = 0;
    unsigned long start_jiffies;
	
    fLib_printf("\r%s start ...\n", __func__);
    UnderflowFlag = 0;
    nResultStatus = FALSE;
	  
    fLib_WatchDog_SetUnderflowValue(iwdt, WD_Clock/WD_PreScaler*3); //3 Sec
    fLib_WatchDog_UnderflowIntEnable(iwdt);
    fLib_WatchDog_UnderflowEnable(iwdt);
	
    fLib_WatchDog_SysIntEnable(iwdt);

    fLib_EnableIRQ(WDT_FTWDT011_IRQ);		

    WatchDogStart(iwdt, 5);
//    fLib_printf("\rDelay 1s!\n");
//	  delay_ms(1000);
	
    fLib_printf("\rKick(Tickle) Dog!\n");
    fLib_WatchDog_KickDog(iwdt);
	
	  start_jiffies = msTicks;
    while (UnderflowFlag == 0)
    {
//        if ( (msTicks - start_jiffies) >= (1*1000) )	// 1 sec
//        {
//            break;
//        }
    }
		fLib_WatchDog_ReProg(iwdt); // stop wdt
    fLib_printf("iwdt%d underflow duration: %d ms ", iwdt, msTicks - start_jiffies);

    if ( (UnderflowFlag & (1<<iwdt)) )
        nResultStatus = TRUE;

	  fLib_WatchDog_SysIntDisable(iwdt);
		
    fLib_WatchDog_UnderflowIntDisable(iwdt);
    fLib_WatchDog_UnderflowDisable(iwdt);

    fLib_DisableIRQ(WDT_FTWDT011_IRQ);		

    if (nResultStatus == TRUE)
    {
        fLib_printf("==> [Pass!]\n");
        result = 1;
    }
    else
    {
        fLib_printf("==> [Fail!]\n");
        result = 0;
    }

    fLib_printf("\n%s end ...\n", __func__);
    return result;
}

#define LOOP_RESET_COUNTER          0x50000         /// program will continue to reset watch dog counter
                                                    /// when loop count is lower than this value
#define LOOP_END_TEST               0x1000000       /// if loop count reach this value and still not hw reset
                                                    /// then watch dog is fail
#define PWMTMR_1MSEC_PERIOD     (UINT32)(250000000/1000)

// --------------------------------------------------------------------
//	check whether watchDog restart is useful (can stop watch dog to reboot)
//	1. for loop to execute watchdog restart
//	2. after reach LOOP_RESET_COUNTER, wait for cpu reset
// --------------------------------------------------------------------
void WatchDogOverflowSysRstTest(DRVIWDT iwdt)
{
    int i;

    fLib_WatchDog_SysResetEnable(iwdt);
    WatchDogStart(iwdt, 2);

    fLib_printf("\r%s ...\n", __func__);

    for (i=0; i<LOOP_END_TEST; ++i)
    {
        if (i==LOOP_RESET_COUNTER)
        {
            fLib_printf("\nThe test is PASS when CPU reset\n");
            fLib_printf("Waiting CPU reset\n");
        }
        else if (i<LOOP_RESET_COUNTER)
        {
            if (i&0xfff)
            {
                fLib_WatchDog_KickDog(iwdt);
            }
        }

        if (!(i&0xfff))
        {
            // There is some problem about
            fLib_printf(".");
        }
    }
    fLib_printf("\n\rIf you see this message \"The watchdog test is FAIL\", it presents WDT function fail\n");
}

// --------------------------------------------------------------------
// kick dog before underflow counter fired, wait for cpu reset
// --------------------------------------------------------------------
void WatchDogUnderflowSysRstTest(DRVIWDT iwdt)
{
    fLib_printf("\r%s ...\n", __func__);
    fLib_WatchDog_SetUnderflowValue(iwdt, WD_Clock/WD_PreScaler*3); // 3  Sec
    fLib_WatchDog_SysResetEnable(iwdt);
    fLib_WatchDog_UnderflowEnable(iwdt);

    WatchDogStart(iwdt, 5);
    fLib_printf("\rDelay 1s!\n");
//      delay_ms(1000);
	
    fLib_printf("\rKick(Tickle) Dog!\n");
    fLib_WatchDog_KickDog(iwdt);
    fLib_printf("\n\rIf you see this message \"The watchdog test is FAIL\", it presents WDT function fail\n");
}

// --------------------------------------------------------------------
//	check whether all watchDog is useful
//	1. for loop to execute watchdog restart
//	2. after reach LOOP_RESET_COUNTER, wait for cpu reset
// --------------------------------------------------------------------

void WatchDogAllKickedTest(void)
{
    UINT8 i, input, iwdt = 0xFF;

    fLib_printf("\r%s ...\n", __func__);
	
    for(i=0;i<DRVIWDT_MAX;i++)
    {
        fLib_WatchDog_SetUnderflowValue((DRVIWDT) i, WD_Clock/WD_PreScaler*1);  // 1 Sec
        fLib_WatchDog_UnderflowIntEnable((DRVIWDT) i);
        fLib_WatchDog_UnderflowEnable((DRVIWDT) i);
        fLib_WatchDog_SysIntEnable((DRVIWDT) i);
        WatchDogStart((DRVIWDT) i, 5+i);
    }

    fLib_EnableIRQ(WDT_FTWDT011_IRQ);		
					
    do {		
        delay_s(1);
        input -= '0';
        for (i=0; i<DRVIWDT_MAX; i++)
        {
            fLib_printf("[%d] counter: 0x%x\n", i, fLib_WatchDog_GetCurrentCounter((DRVIWDT) i));
            if ( input < DRVIWDT_MAX )
		{
			  iwdt = input;
			  fLib_printf("Stop kick iwdt%d\n", iwdt);						
		} 
		
		if (i != iwdt)
                fLib_WatchDog_KickDog((DRVIWDT) i);
        }
        input = fLib_getchar();
    } while (input != ESC);

    for(i=0;i<DRVIWDT_MAX;i++)
    {
        fLib_WatchDog_ReProg((DRVIWDT) i); // stop wdt
        fLib_WatchDog_SysIntDisable((DRVIWDT) i);
    }

    fLib_DisableIRQ(WDT_FTWDT011_IRQ);		
	
    fLib_printf("%s ...\n", __func__);
}

void WatchDogAllFiredTest(void)
{
    UINT8 i, input;

    fLib_printf("\r%s...\n", __func__);
	
    for(i=0;i<DRVIWDT_MAX;i++)
   {
    	fLib_WatchDog_SysIntEnable((DRVIWDT) i);

    	if (WD_ClkSrc == WdClkInt)
    		fLib_WatchDog_SetAutoReLoad((DRVIWDT) i, APB_CLK/WD_PreScaler);
    	else
    		fLib_WatchDog_SetAutoReLoad((DRVIWDT) i, EXT_CLK/WD_PreScaler);

    	if ((i%2) == 0)
    		fLib_WatchDog_SetPrescaler((DRVIWDT) i, (DIVIDE_16+i/2));
    	else
    		fLib_WatchDog_SetPrescaler((DRVIWDT) i, (DIVIDE_4+i/2));
    	
    	fLib_WatchDog_SetSignalLength((DRVIWDT) i, 0xFF);
    	fLib_WatchDog_Enable((DRVIWDT) i);
    }
		
    fLib_EnableIRQ(WDT_FTWDT011_IRQ);

					
    do {
        input = fLib_getchar();
		} while (input != ESC);

    for(i=0;i<DRVIWDT_MAX;i++)
	  {
        fLib_WatchDog_ReProg((DRVIWDT) i); // stop wdt
        fLib_WatchDog_SysIntDisable((DRVIWDT) i);
        fLib_WatchDog_UnderflowIntDisable((DRVIWDT) i);
		}

    fLib_DisableIRQ(WDT_FTWDT011_IRQ);		
	
    fLib_printf("End %s !\n", __func__);
}

void FTWDT011_Test_Main()
{
    UINT32 item;
    char buf[80];
    unsigned int val;
    DRVIWDT iwdt = DRVIWDT_0;
/*
    val = readl(SCU_FTSCU010_PA_BASE+0x00);		// Boot-up Status register in FTSCU100
    if ( (val & 0x200)==0x200 )
        fLib_printf("[re-boot] ** watchdog reset **\n");
    else if ( (val & 0x100)==0x100 )
        fLib_printf("[re-boot] ** hardware reset **\n");
    else if ( (val & 0x10000)==0x10000 )
        fLib_printf("[boot up] ** power button **\n");
*/
    while (1)
    {
	  iwdt_test_init();
			
        do {
            fLib_printf("\nChoose IWDTn (n=0~%d) or %d to start all wdt\n", DRVIWDT_MAX-1, DRVIWDT_MAX);
            fLib_printf("> ");
            scanf( "%d", &item);
            iwdt = (DRVIWDT) item;
        } while (iwdt > DRVIWDT_MAX);
        printf("Choose IWDT%d\n", iwdt);
        do {
            fLib_printf("Choose clock source:\n");
            fLib_printf("1: Internal (APB: %dHz)\n", APB_CLK);
            fLib_printf("2: External (EXT: %dHz)\n", EXT_CLK);
            fLib_printf("> ");
            scanf( "%d", &WD_ClkSrc);
	} while (WD_ClkSrc > (WdClkExt+1));
printf("Choose WD_ClkSrc %d\n", WD_ClkSrc);
	WD_ClkSrc -= 1;
	
	if (WD_ClkSrc == WdClkInt) {
//        	WD_Clock = APB_CLK;
        	fLib_WatchDog_IntClockSrcEnable();
			
	}
	else {
		WD_Clock = EXT_CLK;
		fLib_WatchDog_ExtClockSrcEnable();
	}
				
        if (iwdt == DRVIWDT_MAX) // test all iwdt in the same time
        {
            fLib_printf("All IWDT0 ~ IWDT%d Test \n",DRVIWDT_MAX-1);
            fLib_printf("1: Normal Test (kick) \n");
            fLib_printf("2: Fired Test (without kick)\n");
            fLib_printf("> ");
            scanf( "%d", &item );

            switch (item) {
            case 1:
                WatchDogAllKickedTest();
                break;
            case 2:
                WatchDogAllFiredTest();
                break;
            default:
                break;
            }
        }
        else
        {				
            fLib_printf("Choose a test item:\n");
            fLib_printf("1: IWDT System Interrupt (overflow) Test \n");
            fLib_printf("2: IWDT System Interrupt (underflow) Test \n");
            fLib_printf("3: IWDT System Reset (overflow) Test \n");
//          fLib_printf("4: IWDT System Reset (underflow) Test \n");
//            fLib_printf("5: IWDT External Signal Test \n");
            fLib_printf("5: Reset Board\n");
            fLib_printf("> ");
            scanf( "%d", &item );
					
            switch (item) {
            case 1:
                WatchDogOverflowSysIntTest(iwdt);  // WdIntr (wd_ovf_intr)
                break;
            case 2:
                WatchDogUnderflowSysIntTest(iwdt); // WdUDFIntr (wd_udf_intr)
                break;
            case 3:
                WatchDogOverflowSysRstTest(iwdt);  // WdRst
                break;
            case 4:
                WatchDogUnderflowSysRstTest(iwdt); // WdRst
                break;
#if 0
            case 5:
                WatchDogExtSignalTest(iwdt);       // WdWxt
                break;
#endif
            case 5:
                Reset_Board(iwdt);                 // wd_rst directly
                break;
            default:
                fLib_printf("*** Non-existed item! ***\n");
                break;
            }
        }
    }
}

int FTWDT011_main(void)
{
//    fLib_Int_Init();

    msTicks = 0;
/*
    if(!fLib_Timer_Init(1, 1, (UINT32)PWMTMR1_IRQHandler))//tick every 1ms
    {
        fLib_printf("Init timer%d Fail!\n",1);
    }
*/
    fLib_ConnectIRQ(WDT_FTWDT011_IRQ, IWDT_IRQHandler);
    
    fLib_printf("\n\n");
    fLib_printf("+-----------------------------------------+\n");
    fLib_printf("|               IWDT011 Test              |\n");
    fLib_printf("+-----------------------------------------+\n");

    FTWDT011_Test_Main();
    while(1);
}
