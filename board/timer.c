/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:timer.c                                                             *
* Description: Timer library routine                                       *
* Author: Fred Chien                                                       *
****************************************************************************/

#include "SoFlexible.h"
#include "timer.h"

typedef struct
{
	char str[100];
}Str;

extern Str output[];
extern UINT32 ind;

#if defined(CONFIG_FTTMR010)

	#define IPMODULE    TIMER
	#define IPNAME      FTTMR010

	#define TIMER_RegCOUNT      0x0
	#define TIMER_LOAD          0x4
	#define TIMER_MATCH1        0x8
	#define TIMER_MATCH2        0xC
	#define TIMER_CR            0x30
	#define TIMER_INTSTATE      0x34
	#define TIMER_INTMASK       0x38
	#define MAX_TIMER           3

	typedef struct
	{
		UINT32 Tm1En:1;
		UINT32 Tm1Clock:1;
		UINT32 Tm1OfEn:1;
		UINT32 Tm2En:1;
		UINT32 Tm2Clock:1;
		UINT32 Tm2OfEn:1;
		UINT32 Tm3En:1;
		UINT32 Tm3Clock:1;
		UINT32 Tm3OfEn:1;
		UINT32 Tm1UpDown:1;     // Edward
		UINT32 Tm2UpDown:1;
		UINT32 Tm3UpDown:1;
		UINT8  dummy;           //ycmo stupid work around: just avoid byte access
//		UINT32 Reserved;
	}fLib_TimerControl;

#elif defined(CONFIG_FTPWMTMR010)

	#define IPMODULE 	TIMER
	#define IPNAME   	FTPWMTMR010
      
	#define MAX_TIMER					8
	#define TIMER_INTSTAT              	0x0
	#define TIMER_CR                	0x0
	#define TIMER_LOAD					0x4
	#define TIMER_COMPARE				0x8
	#define TIMER_CNTO					0xc

	typedef struct
	{	
		UINT32 TmSrc:1;
		UINT32 TmStart:1;
		UINT32 TmUpdate:1;
		UINT32 TmOutInv:1;
		UINT32 TmAutoLoad:1;
		UINT32 TmIntEn:1;
		UINT32 TmIntMode:1;
		UINT32 TmDmaEn:1;	
		UINT32 TmDeadZone:9; //ycmo stupid work around: just avoid byte access	
		UINT32 Reserved;	
	}fLib_TimerControl;
	
#else
	#error "unsupport timer controller version "
#endif

typedef struct
{
	UINT32 Tm1Match1:1;
	UINT32 Tm1Match2:1;
	UINT32 Tm1Overflow:1;
	UINT32 Tm2Match1:1;
	UINT32 Tm2Match2:1;
	UINT32 Tm2Overflow:1;
	UINT32 Tm3Match1:1;
	UINT32 Tm3Match2:1;
	UINT32 Tm3Overflow:1;
	UINT8 dummy;                //ycmo stupid work around: just avoid byte access
//	UINT32 Reserved;
}fLib_TimerMask;

typedef struct
{
	UINT32 Tm1Match1:1;
	UINT32 Tm1Match2:1;
	UINT32 Tm1Overflow:1;
	UINT32 Tm2Match1:1;
	UINT32 Tm2Match2:1;
	UINT32 Tm2Overflow:1;
	UINT32 Tm3Match1:1;
	UINT32 Tm3Match2:1;
	UINT32 Tm3Overflow:1;
	UINT8 dummy;                //ycmo stupid work around: just avoid byte access
//	UINT32 Reserved;
}fLib_TimerIntrState;

typedef struct 
{
	UINT32 IntNum;      /* interrupt number */      
	PrHandler Handler;  /* interrupt Routine */ 
	UINT32 Tick;        /* Tick Per Second */   
	UINT32 Running;     /* Is timer running */       
}fLib_TimerStruct;

#if defined(CONFIG_FTTMR010)
	UINT32 fLib_Timer_Vectors[MAX_TIMER + 1] = {0, TIMER_FTTMR010_IRQ0};
	UINT32 TimerBase[] ={0, TIMER_FTTMR010_PA_BASE, TIMER_FTTMR010_PA_BASE+0x10, TIMER_FTTMR010_PA_BASE+0x20};
	#define timer_base  TIMER_FTTMR010_PA_BASE
#elif defined(CONFIG_FTPWMTMR010)
    UINT32 fLib_Timer_Vectors[MAX_TIMER + 1] = {0,TIMER_FTPWMTMR010_IRQ0,TIMER_FTPWMTMR010_IRQ1,TIMER_FTPWMTMR010_IRQ2,TIMER_FTPWMTMR010_IRQ3, 
												  TIMER_FTPWMTMR010_IRQ4,TIMER_FTPWMTMR010_IRQ5,TIMER_FTPWMTMR010_IRQ6,TIMER_FTPWMTMR010_IRQ7};
    UINT32 TimerBase[] ={0, TIMER_FTPWMTMR010_PA_BASE+0x10, TIMER_FTPWMTMR010_PA_BASE+0x20,TIMER_FTPWMTMR010_PA_BASE+0x30,TIMER_FTPWMTMR010_PA_BASE+0x40 };
    #define timer_base  TIMER_FTPWMTMR010_PA_BASE
    fLib_TimerStruct ftimer[MAX_TIMER+1];
    volatile fLib_TimerControl *TimerControl[MAX_TIMER+1];
#else
	#error "unsupport timer controller version "
#endif

//fLib_TimerStruct ftimer[MAX_TIMER + 1];
//funtion prototype
void fLib_Timer_AutoReloadValue(UINT32 timer, UINT32 value);
void  Timer_ResetAll(void);
INT32 GetTimerTick(UINT32 timer);
INT32 SetTimerTick(UINT32 timer,UINT32 clk_tick);
INT32 SetTimerClkSource(UINT32 timer,UINT32 clk);
INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler);
INT32 fLib_Timer_IntClear(UINT32 timer);
INT32 fLib_Clear_IntrStateRegister(UINT32 timer);

UINT32 T1_Tick = 0, T2_Tick = 0, T3_Tick = 0, T4_Tick = 0;

/* Routine to disable a timer and free-up the associated IRQ */
INT32 fLib_Timer_Close(UINT32 timer)
{
	UINT32 i;

	if (timer == 0 || timer > MAX_TIMER)
		return FALSE;

	if(!fLib_Timer_Disable(timer))
		return FALSE;               /* Stop the timer first */

	i = fLib_Timer_Vectors[timer];  /* then release the IRQ */
	if (!fLib_CloseIRQ( i))
		return FALSE;

	return TRUE;
}

INT32 fLib_Timer_IOCtrl(fLib_Timer_IoType IoType,UINT32 timer,UINT32 tick)
{
	switch(IoType)
	{
	case IO_TIMER_RESETALL:
		Timer_ResetAll();
		break;
	case IO_TIMER_GETTICK:
		return GetTimerTick(timer);
		break;
	case IO_TIMER_SETTICK:
		return SetTimerTick(timer,tick);
		break;
	case IO_TIMER_SETCLKSRC:
		return SetTimerClkSource(timer,tick);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

#if defined(CONFIG_FTPWMTMR010)
/* Routine to start the specified timer & enable the interrupt */
BOOL fLib_Timer_Init(UINT32 timer,UINT32 tick,PrHandler handler)
{
    int i;
int temp;
    fLib_TimerStruct *ctimer=&ftimer[timer];
    int intNum = fLib_Timer_Vectors[timer];
printf("fLib_Timer_Init \n");
    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

    for(i=0; i<=MAX_TIMER; i++)
    {
        //TimerControl[i]=(fLib_TimerControl *)(TimerBase[0] + TIMER_CR+(i-1)*0x10);
        TimerControl[i]=(fLib_TimerControl *)(TimerBase[i] + TIMER_CR);
    }

    /* Set the timer tick */
    if(!fLib_Timer_IOCtrl(IO_TIMER_SETTICK,timer,tick))
        return FALSE;

    fLib_Timer_AutoReloadValue(timer,APB_CLK/ctimer->Tick);
//    fLib_Timer_Counter(timer,APB_CLK/ctimer->Tick);
//  ycmo:  this is RO register
    /*enable auto and int bit */
    fLib_Timer_AutoReloadEnable(timer);

    if (!fLib_Timer_IntEnable(timer))
        return FALSE;

    /* Start the timer ticking */
    if(!fLib_Timer_Enable(timer))
        return FALSE;

    //connect timer ISR
    if(!Timer_ConnectIsr(timer,handler))
        return FALSE;

    /* Install timer interrupt routine */
    fLib_SetIRQmode((UINT32)intNum,LEVEL);
//    fLib_SetIRQmode((UINT32)intNum,EDGE);
    fLib_EnableIRQ((UINT32)intNum);


    switch(timer)
    {
        case 1:
            T1_Tick = 0;
            break;
        case 2:
            T2_Tick = 0;
            break;
        case 3:
            T3_Tick = 0;
            break;
        case 4:
            T4_Tick = 0;
            break;
        default:
            break;
    }

    return TRUE;
}


//output inverse funtion
INT32 fLib_Timer_OutInvEnable(UINT32 timer)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    TimerControl[timer]->TmOutInv=1;
    return TRUE;
}


INT32 fLib_Timer_AutoReloadEnable(UINT32 timer)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    TimerControl[timer]->TmAutoLoad=1;
    return TRUE;
}


INT32 fLib_Timer_Counter(UINT32 timer)
{
    return inw(TimerBase[timer] + TIMER_CNTO);
}


INT32 fLib_Timer_IntEnable(UINT32 timer)
{


    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmIntEn=1;


    return TRUE;
}

INT32 fLib_Timer_IntDisable(UINT32 timer)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmIntEn=0;


    return TRUE;
}


INT32 fLib_Timer_IntModeEnable(UINT32 timer,UINT32 mode)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmIntMode=mode;


    return TRUE;
}

INT32 fLib_Timer_DmaEnable(UINT32 timer)
{


    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmDmaEn=1;


    return TRUE;
}


INT32 fLib_Timer_DeadZoneEnable(UINT32 timer,UINT32 offset)
{


   volatile fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmDeadZone=offset;


    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_Enable(UINT32 timer)
{
   volatile fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    if(ctimer->Running==TRUE)
        return FALSE;
    TimerControl[timer]->TmUpdate=1;
	TimerControl[timer]->TmStart=1;
	TimerControl[timer]->TmSrc=0; //fix clk source to apb clk

    //set the timer status =true
    ctimer->Running=TRUE;

    return TRUE;
}


/* This routine stops the specified timer hardware. */
INT32 fLib_Timer_Disable(UINT32 timer)
{
   volatile fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    /* Disable the Control register bit */
    TimerControl[timer]->TmStart=0;
    TimerControl[timer]->TmUpdate=0;
    TimerControl[timer]->TmOutInv=0;
    TimerControl[timer]->TmDmaEn=0;
    TimerControl[timer]->TmIntEn=0;
    TimerControl[timer]->TmDeadZone=0;
    TimerControl[timer]->TmSrc=0;
    TimerControl[timer]->TmAutoLoad=0;

     //set the timer status=false
    ctimer->Running=FALSE;

    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_IntClear(UINT32 timer)
{
    int value;
//printf("fLib_Timer_IntClear\n");

     if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    value=1<<(timer-1);
      outw(timer_base + TIMER_INTSTAT, value);


    return TRUE;
}


void fLib_Timer_CmpValue(UINT32 timer, UINT32 value)
{
    outw(TimerBase[timer] + TIMER_COMPARE, value);
}


INT32 SetTimerClkSource(UINT32 timer,UINT32 clk)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl[timer]->TmSrc=clk;

   	return TRUE;
}


void Timer1_Tick()
{
printf("Timer1_Tick\n");
    int intNum = fLib_Timer_Vectors[1];
fLib_DisableIRQ(intNum);
    fLib_Timer_IntClear(1);
    fLib_ClearIRQ( intNum);
    T1_Tick++;
fLib_EnableIRQ(intNum);

}


void Timer2_Tick()
{
    int intNum = fLib_Timer_Vectors[2];
fLib_DisableIRQ(intNum);
    fLib_Timer_IntClear(2);
    fLib_ClearIRQ( intNum);
    T1_Tick++;
fLib_EnableIRQ(intNum);
}


void Timer3_Tick()
{
    int intNum = fLib_Timer_Vectors[3];
fLib_DisableIRQ(intNum);
    fLib_Timer_IntClear(3);
    fLib_ClearIRQ( intNum);
    T1_Tick++;
fLib_EnableIRQ(intNum);
    
}


void Timer4_Tick()
{
    int intNum = fLib_Timer_Vectors[4];
fLib_DisableIRQ(intNum);
    fLib_Timer_IntClear(4);
    fLib_ClearIRQ( intNum);
    T1_Tick++;
fLib_EnableIRQ(intNum);
   
}

#else //FTTMR010
/* Routine to start the specified timer & enable the interrupt */
BOOL fLib_Timer_Init(UINT32 timer, UINT32 tick, PrHandler handler)
{
	fLib_TimerStruct *ctimer = &ftimer[timer];
	int intNum = fLib_Timer_Vectors[timer];

	if (timer == 0 || timer > MAX_TIMER)
		return FALSE;

	/* Set the timer tick */
	if(!fLib_Timer_IOCtrl(IO_TIMER_SETTICK, timer, tick))
		return FALSE;

	fLib_Clear_IntrStateRegister(timer);

	/* Start the timer ticking */
	if(!fLib_Timer_Enable(timer))
		return FALSE;

	printf("register ISR\n");

	//connect timer ISR
	if(!Timer_ConnectIsr(timer, handler))
		return FALSE;

	/* Install timer interrupt routine */
	fLib_SetIRQmode((UINT32)intNum, LEVEL);
	printf("enableIRQ\n");
	fLib_EnableIRQ((UINT32)intNum);

	switch(timer)
	{
	case 1:
		T1_Tick = 0;
		break;
	case 2:
		T2_Tick = 0;
		break;
	case 3:
		T3_Tick = 0;
		break;

	default:
		break;
	}

	return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_IntEnable(UINT32 timer)
{
	fLib_TimerMask *TimerMask = (fLib_TimerMask *)(timer_base + TIMER_INTMASK);
	// fLib_TimerStruct *ctimer=&ftimer[timer];

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerMask->Tm1Overflow=0;
	TimerMask->Tm1Match1=1;
	TimerMask->Tm1Match2=1;
	TimerMask->Tm2Overflow=0;
	TimerMask->Tm2Match1=0;
	TimerMask->Tm2Match2=0;
	TimerMask->Tm3Overflow=0;
	TimerMask->Tm3Match1=0;
	TimerMask->Tm3Match2=0;

	//enable timer
	switch(timer)
	{
	case 1:
		TimerMask->Tm1Overflow=0;
		TimerMask->Tm1Match1=0;
		TimerMask->Tm1Match2=0;
		break;
	case 2:
		TimerMask->Tm2Overflow=0;
		TimerMask->Tm2Match1=0;
		TimerMask->Tm2Match2=0;
		break;
	case 3:
		TimerMask->Tm3Overflow=0;
		TimerMask->Tm3Match1=0;
		TimerMask->Tm3Match2=0;
		break;
	default:
		break;
	}

	// sprintf(output[ind++].str, "Mask = 0x%x\n", *TimerMask);
	return TRUE;
}

INT32 fLib_Write_MR(UINT32 timer, int match1, int match2, int overflow)
{
	fLib_TimerMask *TimerMask = (fLib_TimerMask *)(timer_base + TIMER_INTMASK);

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerMask -> Tm1Overflow = 0;
	TimerMask -> Tm1Match1 = 0;
	TimerMask -> Tm1Match2 = 0;
	TimerMask -> Tm2Overflow = 0;
	TimerMask -> Tm2Match1 = 0;
	TimerMask -> Tm2Match2 = 0;
	TimerMask -> Tm3Overflow = 0;
	TimerMask -> Tm3Match1 = 0;
	TimerMask -> Tm3Match2 = 0;
	switch(timer)
	{
	case 1:
		TimerMask -> Tm1Overflow = overflow;
		TimerMask -> Tm1Match1 = match1;
		TimerMask -> Tm1Match2 = match2;
		break;
	case 2:
		TimerMask -> Tm2Overflow = overflow;
		TimerMask -> Tm2Match1 = match1;
		TimerMask -> Tm2Match2 = match2;
		break;
	case 3:
		TimerMask -> Tm3Overflow = overflow;
		TimerMask -> Tm3Match1 = match1;
		TimerMask -> Tm3Match2 = match2;
		break;
	default:
		break;
	}

	return TRUE;
}

INT32 fLib_Write_CR(UINT32 timer, UINT32 source, UINT32 upDown)
{
	fLib_TimerControl *TimerControl=(fLib_TimerControl *)(timer_base + TIMER_CR);

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	switch(timer)
	{
	case 1:
		TimerControl -> Tm1Clock = source;
		TimerControl -> Tm1OfEn = 1;
		TimerControl -> Tm1UpDown = upDown;
		TimerControl -> Tm1En = 1;
		break;
	case 2:
		TimerControl -> Tm2Clock = source;
		TimerControl -> Tm2OfEn = 1;
		TimerControl -> Tm2UpDown = upDown;
		TimerControl -> Tm2En = 1;
		break;
	case 3:
		TimerControl -> Tm3Clock = source;
		TimerControl -> Tm3OfEn = 1;
		TimerControl -> Tm3UpDown = upDown;
		TimerControl -> Tm3En = 1;
		break;
	default:
		break;
	}

	return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_Enable(UINT32 timer)
{
	fLib_TimerControl *TimerControl=(fLib_TimerControl *)(timer_base + TIMER_CR);
	fLib_TimerStruct *ctimer=&ftimer[timer];

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	if(ctimer->Running == TRUE)
	{
		//printf("Timer is running.\r\n");
		return FALSE;
	}

	ctimer->Running=TRUE;

	return TRUE;
}

/* This routine stops the specified timer hardware. */
INT32 fLib_Timer_Disable(UINT32 timer)
{
	fLib_TimerControl *TimerControl=(fLib_TimerControl *)(timer_base + TIMER_CR);

	fLib_TimerStruct *ctimer=&ftimer[timer];


	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	/* Disable the Control register bit */
	switch(timer)
	{
	case 1:
		TimerControl->Tm1En=0;
		TimerControl->Tm1OfEn=0;
		break;
	case 2:
		TimerControl->Tm2En=0;
		TimerControl->Tm2OfEn=0;
		break;
	case 3:
		TimerControl->Tm3En=0;
		TimerControl->Tm3OfEn=0;
		break;
	default:
		break;
	}

	//set the timer status=false
	ctimer->Running=FALSE;

	return TRUE;
}

//ycmo120711 : align function to fLib_Timer_Counter
INT32 fLib_Timer_Counter(UINT32 timer, UINT32 value)
{
	if (value)
		outw(TimerBase[timer] + TIMER_RegCOUNT, value);

	return inw(TimerBase[timer] + TIMER_RegCOUNT);
}

void fLib_Timer_MatchValue1(UINT32 timer, UINT32 value)
{
	outw(TimerBase[timer] + TIMER_MATCH1, value);
}

void fLib_Timer_MatchValue2(UINT32 timer, UINT32 value)
{
	outw(TimerBase[timer] + TIMER_MATCH2, value);
}

INT32 SetTimerClkSource(UINT32 timer,UINT32 clk)
{
	fLib_TimerControl *TimerControl=(fLib_TimerControl *)(timer_base + TIMER_CR);

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	switch(timer)
	{
	case 1:
		TimerControl->Tm1Clock=clk;
		break;
	case 2:
		TimerControl->Tm2Clock=clk;
		break;
	case 3:
		TimerControl->Tm3Clock=clk;
		break;
	default:
		break;
	}

	return TRUE;
}

/* This routine starts the specified timer hardware. */
/* commented by ctd vp version */
/*
INT32 fLib_Timer_IntClear(UINT32 timer)
{
	fLib_TimerMask *TimerMask=(fLib_TimerMask *)(timer_base + TIMER_INTSTATE);
//	fLib_TimerStruct *ctimer=&ftimer[timer];

	//clear timer
	switch(timer)
	{
	case 1:
		TimerMask->Tm1Overflow=1;
		TimerMask->Tm1Match1=1;
		TimerMask->Tm1Match2=1;
		break;
	case 2:
		TimerMask->Tm2Overflow=1;
		TimerMask->Tm2Match1=1;
		TimerMask->Tm2Match2=1;
		break;
	case 3:
		TimerMask->Tm3Overflow=1;
		TimerMask->Tm3Match1=1;
		TimerMask->Tm3Match2=1;
		break;
	default:
		break;
	}

	return TRUE;
}
*/

INT32 fLib_Clear_IntrStateRegister(UINT32 timer)
{
	/*
	 When a interrupt whose interrupt status register = 7(overflow+match1+match2),
	 the first comparison(TimerIntrState->Tm1Overflow == 1) turns out true.
	 It seems we only clear the Tm1Overflow bit(TimerIntrState->Tm1Overflow=1;),
	 but actually we also clear the Tm1Match1 & Tm1Match2.
	 This is tricky. With this trick, we can clear intrstatus register this way.
	*/
	fLib_TimerIntrState *TimerIntrState = (fLib_TimerIntrState *)(timer_base + TIMER_INTSTATE);
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	switch(timer)
	{
	case 1:
		if (TimerIntrState->Tm1Overflow == 1)
			TimerIntrState->Tm1Overflow=1;
		else if (TimerIntrState->Tm1Match1 == 1)
			TimerIntrState->Tm1Match1=1;
		else if (TimerIntrState->Tm1Match2 == 1)  
			TimerIntrState->Tm1Match2=1;
		break;
	case 2:
		if (TimerIntrState->Tm2Overflow == 1)
			TimerIntrState->Tm2Overflow=1;
		else if (TimerIntrState->Tm2Match1 == 1)
			TimerIntrState->Tm2Match1=1;
		else if (TimerIntrState->Tm2Match2 == 1)
			TimerIntrState->Tm2Match2=1;
		break;
	case 3:
		if (TimerIntrState->Tm3Overflow == 1)
			TimerIntrState->Tm3Overflow=1;
		else if (TimerIntrState->Tm3Match1 == 1)
			TimerIntrState->Tm3Match1=1;
		else if (TimerIntrState->Tm3Match2 == 1)
			TimerIntrState->Tm3Match2=1;
		break;
	default:
		break;
	}

	return TRUE;
}

#ifdef CONFIG_IRQ_FTINTC020_VECTOR
void __attribute__((interrupt("IRQ"))) Timer1_Tick()
#else
void Timer1_Tick()
#endif
{
//	fLib_TimerIntrState *TimerIntrState = (fLib_TimerIntrState *)(timer_base + TIMER_INTSTATE);
//	fLib_printf("IntrState = 0x%x\n", *TimerIntrState);
	int ret;
    int intNum = fLib_Timer_Vectors[1];

    fLib_ClearIRQ(intNum+32);
	T1_Tick++;
//	printf("T1_Tick = %lu\n", T1_Tick);
#ifdef CONFIG_FREERTOS
#include "FreeRTOS.h"
#if 1
extern void FreeRTOS_Tick_Handler( void );
	FreeRTOS_Tick_Handler();
#endif	
#endif
	fLib_Clear_IntrStateRegister(1);
	ret = readl(0x20200034);
	writel(ret, 0x20200034);
	fLib_EnableIRQ(intNum);
}

void Timer2_Tick()
{
//	fLib_TimerIntrState *TimerIntrState = (fLib_TimerIntrState *)(timer_base + TIMER_INTSTATE);
//	printf("IntrState = 0x%x\n", *TimerIntrState);

	T2_Tick++;
//	printf("T2_Tick = %lu\n", T2_Tick);
	fLib_Clear_IntrStateRegister(2);
}

void Timer3_Tick()
{
//	fLib_TimerIntrState *TimerIntrState = (fLib_TimerIntrState *)(timer_base + TIMER_INTSTATE);
//	printf("IntrState = 0x%x\n", *TimerIntrState);

	T3_Tick++;
//	printf("T3_Tick = %lu\n", T3_Tick);
	fLib_Clear_IntrStateRegister(3);
}

void Timer1_Tick_VP()
{
	//fLib_Timer_IntClear(1);
	//fLib_printf("timer interrupt\n");
	T1_Tick++;
	//fLib_printf( "counter = 0x%x\n", fLib_Timer_Counter(1, 0xffffffff/2));  
	fLib_Timer_Counter(1, 0xffffffff/2);
}

void Timer2_Tick_VP()
{
	//fLib_Timer_IntClear(2);
	T2_Tick++;
	fLib_Timer_Counter(2, 0xffffffff/2); 
}

void Timer3_Tick_VP()                                                                        
{
	//fLib_Timer_IntClear(3);
	T3_Tick++;
	fLib_Timer_Counter(3, 0xffffffff/2); 
}

#endif //CONFIG_FTTMR010 & CONFIG_FTPWMTMR010

/* Routine to initialise install requested timer. Stops the timer. */
INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler)
{
	fLib_TimerStruct *ctimer=&ftimer[timer];
	UINT32 i;

	i = fLib_Timer_Vectors[timer];
printf("Timer_ConnectIsr irq = %d\n", i);
	fLib_CloseIRQ(i);

	if (!fLib_ConnectIRQ( i, handler))
		return FALSE;

	ctimer->Handler = handler;
	ctimer->IntNum = i;     /* INT number */

	return timer;
}

UINT32 fLib_CurrentT1Tick()
{
	return T1_Tick;
}

UINT32 fLib_CurrentT2Tick()
{
	return T2_Tick;
}

UINT32 fLib_CurrentT3Tick()
{
	return T3_Tick;
}

UINT32 fLib_CurrentT4Tick()
{
	return T4_Tick;
}

/////////////////////////////////////////////////////
//
//  Only for detail function call subroutine
//
/////////////////////////////////////////////////////

/* Start-up routine to initialise the timers to a known state */
void Timer_ResetAll(void)
{
	UINT32 i;

	//reset all timer to default value
	for (i = 1; i <= MAX_TIMER; i++)
		fLib_Timer_Disable(i);
}

INT32 GetTimerTick(UINT32 timer)
{
	UINT32 cur_tick;

	volatile fLib_TimerStruct *ctimer = &ftimer[timer];

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	cur_tick=ctimer->Tick;

	return cur_tick;
}

INT32 SetTimerTick(UINT32 timer,UINT32 clk_tick)
{
	volatile fLib_TimerStruct *ctimer = &ftimer[timer];

	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	ctimer->Tick=clk_tick;

	return TRUE;
}

void fLib_Timer_AutoReloadValue(UINT32 timer, UINT32 value)
{
	outw(TimerBase[timer] + TIMER_LOAD, value);
}

void fLib_Timer_Control(UINT32 timer, UINT32 value)
{
	outw(TimerBase[timer] + TIMER_CR, value);
}

// End of file - timer.c

static unsigned int begin_timer_count[MAX_TIMER];

static unsigned int get_timer(unsigned int timer_n)
{
#if defined(CONFIG_FTTMR010)
	return fLib_Timer_Counter(timer_n,0);
#elif defined(CONFIG_FTPWMTMR010)
	return fLib_Timer_Counter(timer_n);
#endif
}

int timer_init(unsigned int timer_n, unsigned int ext_clk){


#if defined(CONFIG_FTPWMTMR010)
    TimerControl[timer_n]=(fLib_TimerControl *)(TimerBase[timer_n] + TIMER_CR);
    if(ext_clk)
        TimerControl[timer_n]->TmSrc=1;
#elif defined(CONFIG_FTTMR010)
#warning "NYI: select timer clock source"
#endif

    if(!fLib_Timer_IOCtrl(IO_TIMER_SETTICK,timer_n,1))
        return FALSE;

    fLib_Timer_AutoReloadValue(timer_n,0xFFFFFFFF);
#if defined(CONFIG_FTTMR010)
    fLib_Timer_Counter(timer_n,0xFFFFFFFF);
#elif defined(CONFIG_FTPWMTMR010)
    fLib_Timer_AutoReloadEnable(timer_n);
#endif
    if(!fLib_Timer_Enable(timer_n))
        return FALSE;

}
void timer_start(unsigned int timer_n){

    begin_timer_count[timer_n] = get_timer(timer_n);
#if defined(CONFIG_FTPWMTMR010)
	TimerControl[timer_n]->TmStart=1; //start timer
#elif defined(CONFIG_FTTMR010)
#warning "NYI: stop timer clock until it is the time we want measure "
#endif

}

//return user time (second)
float timer_end(unsigned timer_n){
    unsigned int end_timer_count = get_timer(timer_n);
    unsigned int interval;
    unsigned int clk;


#if defined(CONFIG_FTPWMTMR010)
	TimerControl[timer_n]->TmStart=0; //stop timer
    if(TimerControl[timer_n]->TmSrc==1){
        clk=32768; //RTC CLK
    }else{
        clk=get_apb_clk();
    }
//    printf("clk=%d\n", clk);
#elif defined(CONFIG_FTTMR010)

#warning "NYI: select timer clock source"
        //clk=get_apb_clk();
#warning "NYI: stop timer clock until it is the time we want measure "
#endif

//    printf("begin=%x, end=%x \n",begin_timer_count[timer_n],end_timer_count);
    if (begin_timer_count[timer_n] > end_timer_count){
        interval = begin_timer_count[timer_n]-end_timer_count;
        return (float)interval/ clk;
    }else{ //count to zero, reload
        interval = begin_timer_count[timer_n];
        interval += (0xffffffff - end_timer_count);
        return (float)interval/ clk;
    }
}

