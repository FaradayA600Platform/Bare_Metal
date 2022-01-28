#include    "SoFlexible.h"
//#include	"fLib.h"
#include 	"wdt011.h"

void fLib_WatchDog_Enable(DRVIWDT iwdt)
{
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_KR, START_KEY);
}

void fLib_WatchDog_ReProg(DRVIWDT iwdt)
{
printf("fLib_WatchDog_ReProg counter = 0x%x\n", inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_KR));
	  /* Disable written access protection */
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_KR, REPROG_KEY);

    /* wait for watchdog timer to stop */
    while(inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR) & WdEnable);
}

void fLib_WatchDog_KickDog(DRVIWDT iwdt)
{
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_KR, KICKDOG_KEY);
}

void fLib_WatchDog_SetPrescaler(DRVIWDT iwdt, UINT8 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_PR, value);

    while (inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_SR) & WdPVU);
}

void fLib_WatchDog_SetAutoReLoad(DRVIWDT iwdt, UINT32 value)
{
printf("fLib_WatchDog_SetAutoReLoad value = 0x%x\n", value);
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_RLR, value);

    while (inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_SR) & WdRVU);
}

void fLib_WatchDog_SetUnderflowValue(DRVIWDT iwdt, UINT32 value)
{
printf("fLib_WatchDog_SetUnderflowValue value = 0x%x\n", value);
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_UDF, value);
}

void fLib_WatchDog_SetSignalLength(DRVIWDT iwdt, UINT8 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_INTRLEN, value);
}

void fLib_WatchDog_ExtClockSrcEnable(void)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg((DRVIWDT) 0);

    tmp = inw(WDT_FTWDT011_PA_BASE + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdClockSrc;

    outw(WDT_FTWDT011_PA_BASE + IWDT_CR, tmp);
}

void fLib_WatchDog_IntClockSrcEnable(void)
{
    UINT32 tmp;
    printf("fLib_WatchDog_IntClockSrcEnable\n");
    fLib_WatchDog_ReProg((DRVIWDT) 0);
    printf("fLib_WatchDog_ReProg\n");
    tmp = inw(WDT_FTWDT011_PA_BASE + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdClockSrc);
	printf("set CR = %lx\n",tmp);
    outw(WDT_FTWDT011_PA_BASE + IWDT_CR, tmp);
	printf("read CR = %lx\n", inw(WDT_FTWDT011_PA_BASE + IWDT_CR));	
}

void fLib_WatchDog_ExtSignalEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdExtEn;

    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_ExtSignalDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdExtEn);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysIntEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdIntrEn;

    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysIntDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdIntrEn);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowIntEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdUDFIntrEn;

    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowIntDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdUDFIntrEn);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdUDFEn;

    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdUDFEn);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysResetEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdRstEn|WdIntrEn ;
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR,tmp);
}

void fLib_WatchDog_SysResetDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdRstEn);
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

BOOL fLib_WatchDog_IsCounterZero(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_INTSR);
    if (tmp == 1)
        return TRUE;
    else
        return FALSE;
}

void fLib_WatchDog_ClearOverflowStatus(DRVIWDT iwdt)
{
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_INTSR, 0x1);
}

void fLib_WatchDog_ClearUnderflowStatus(DRVIWDT iwdt)
{
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_INTSR, 0x2);
}

void fLib_WatchDog_SetUnderflowVaule(DRVIWDT iwdt, UINT32 value)
{
    outw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_UDF, value);
}

UINT32 fLib_WatchDog_GetCurrentCounter(DRVIWDT iwdt)
{
    return inw(WDT_FTWDT011_PA_BASE + (iwdt * IWDT_OFFSET) + IWDT_CURR);
}

UINT32 fLib_WatchDog_GetGlobalIntrStatus(void)
{
    return inw(WDT_FTWDT011_PA_BASE + IWDT_INTSTS);
}

void fLib_WatchDog_ClearGlobalIntrStatus(UINT32 value)
{
    outw(WDT_FTWDT011_PA_BASE + IWDT_INTSTS, value);
}

