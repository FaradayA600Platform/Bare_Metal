/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:pwmtimer.c                                                          *
* Description: Timer library routine                                       *
* Author: Edward_lin                                                       *
****************************************************************************/

#include "SoFlexible.h"
#include "pwmtmr.h"


#define INT_CASTAT  0x0
#define CTRL        0x10
#define CNTB        0x14
#define CMPB        0x18
#define CNTO        0x1c

#define NOTFIQ      0

typedef struct
{
	UINT32 TM1_INT_STAT:1;
	UINT32 TM2_INT_STAT:1;
	UINT32 TM3_INT_STAT:1;
	UINT32 TM4_INT_STAT:1;
	UINT32 TM5_INT_STAT:1;
	UINT32 TM6_INT_STAT:1;
	UINT32 TM7_INT_STAT:1;
	UINT32 TM8_INT_STAT:1;
}TimerIntStat;

typedef struct
{
	UINT32 CLK_SRC:1;
	UINT32 START:1;
	UINT32 UPDATE:1;
	UINT32 OUT_INV:1;
	UINT32 AUTO:1;
	UINT32 INT_EN:1;
	UINT32 INT_MO:1;
	UINT32 DMA_EN:1;
	UINT32 PWM_EN:1;
	UINT32 RESERVED:15;
	UINT32 DZ:8;
}TimerControl;

typedef struct 
{
	UINT32 cmpb;
}TimerCMPB;

typedef struct
{
	UINT32 cnto;
}TimerCNTO;

typedef struct
{
	UINT32 IntNum;      // interrupt number
	PrHandler Handler;  // interrupt Routine
	UINT32 Tick;        // Tick Per Second
	UINT32 Running;     // Is timer running
}TimerStruct;

UINT32 gTimer_vectors[MAX_TIMER + 1] = {0, 
                                        TIMER_FTPWMTMR010_IRQ0,
                                        TIMER_FTPWMTMR010_IRQ1,
                                        TIMER_FTPWMTMR010_IRQ2,
                                        TIMER_FTPWMTMR010_IRQ3,
                                        TIMER_FTPWMTMR010_IRQ4,
                                        TIMER_FTPWMTMR010_IRQ5,
                                        TIMER_FTPWMTMR010_IRQ6,
                                        TIMER_FTPWMTMR010_IRQ7,                                      
                                       };

UINT32 gTimerBase[MAX_TIMER + 1] = {0, 
                                    TIMER_FTPWMTMR010_PA_BASE+0x0,
                                    TIMER_FTPWMTMR010_PA_BASE+0x10, 
                                    TIMER_FTPWMTMR010_PA_BASE+0x20,
                                    TIMER_FTPWMTMR010_PA_BASE+0x30,
                                    TIMER_FTPWMTMR010_PA_BASE+0x40,
                                    TIMER_FTPWMTMR010_PA_BASE+0x50,
                                    TIMER_FTPWMTMR010_PA_BASE+0x60,
                                    TIMER_FTPWMTMR010_PA_BASE+0x70,                                   
                                   };


UINT32 gTick[MAX_TIMER + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//TimerStruct ftimer[MAX_TIMER+1];

//funtion prototype
BOOL Write_TimerCNTB(UINT32, UINT32);
BOOL Write_TimerCMPB(UINT32, UINT32);
INT32 Observe_Timer_Counter(UINT32);
BOOL Read_Timer_Intr_Status(UINT32);
BOOL Timer_Start(UINT32);
BOOL Timer_Stop(UINT32);
BOOL Enable_Timer_Auto(UINT32);
BOOL Disable_Timer_Auto(UINT32);
BOOL Enable_Timer_Update(UINT32);
BOOL Disable_Timer_Update(UINT32);
BOOL Enable_Timer_INT(UINT32);
BOOL Disable_Timer_INT(UINT32);
BOOL Enable_PWM(UINT32);
BOOL Disable_PWM(UINT32);
BOOL Enable_DMA(UINT32);
BOOL Disable_DMA(UINT32);
BOOL SetClockSource(UINT32, UINT32);
BOOL Set_Timer_Int_Mode(UINT32, UINT32);
BOOL SetInvert(UINT32, UINT32);
BOOL SetDeadZone(UINT32, UINT32);
void ClearIRQ(void);

void fLib_Timer_AutoReloadValue(UINT32 timer, UINT32 value);
void  Timer_ResetAll(void);
INT32 GetTimerTick(UINT32 timer);
INT32 SetTimerTick(UINT32 timer,UINT32 clk_tick);
INT32 SetTimerClkSource(UINT32 timer,UINT32 clk);
INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler);
INT32 fLib_Clear_IntrStateRegister(UINT32 timer);

BOOL Write_TimerCNTB(UINT32 timer, UINT32 value)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	UINT32 *cntbPtr = (UINT32 *)(gTimerBase[timer] + CNTB);
	*cntbPtr = value;

	return TRUE;
}

BOOL Write_TimerCMPB(UINT32 timer, UINT32 value)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	UINT32 *cmpbPtr = (UINT32 *)(gTimerBase[timer] + CMPB);
	*cmpbPtr = value;

	return TRUE;
}

BOOL Write_TimerCNTO(UINT32 timer, UINT32 value)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	UINT32 *cntoPtr = (UINT32 *)(gTimerBase[timer] + CNTO);
       *cntoPtr = value;

	return TRUE;
}

INT32 Observe_Timer_Counter(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return -1;

	TimerCNTO *cntoPtr = (TimerCNTO *)(gTimerBase[timer] + CNTO);

	return cntoPtr -> cnto;
}

BOOL Timer_Start(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> START = 1;

	return TRUE;
}

BOOL Timer_Stop(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> START = 0;

	return TRUE;
}

BOOL Read_Timer_Intr_Status(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerIntStat *intrPtr = (TimerIntStat *)(gTimerBase[timer] + 0x0);
	switch (timer) {
		case 1:
			return intrPtr -> TM1_INT_STAT;

		case 2:
			return intrPtr -> TM2_INT_STAT;

		case 3:
			return intrPtr -> TM3_INT_STAT;
             
		case 4:
			return intrPtr -> TM1_INT_STAT;

		default:
			while(1);
	}
}

BOOL Enable_Timer_Auto(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> AUTO = 1;

	return TRUE;
}

BOOL Enable_Timer_One_Shot(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> AUTO = 0;

	return TRUE;
}

BOOL Enable_Timer_Update(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> UPDATE = 1;

	return TRUE;
}


BOOL Disable_Timer_Update(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> UPDATE = 0;

	return TRUE;
}


BOOL Enable_Timer_INT(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> INT_EN = 1;

	return TRUE;
}

BOOL Disable_Timer_INT(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> INT_EN = 0;

	return TRUE;
}

BOOL Enable_PWM(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> PWM_EN = 1;

	return TRUE;
}

BOOL Disable_PWM(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> PWM_EN = 0;

	return TRUE;
}

BOOL Enable_DMA(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> DMA_EN = 1;

	return TRUE;
}

BOOL Disable_DMA(UINT32 timer)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> DMA_EN = 0;

	return TRUE;
}

BOOL SetDeadZone(UINT32 timer, UINT32 value)
{
	UINT32 dz;
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	dz = value & 0xff;
	cntlPtr -> DZ = dz;

	return TRUE;
}

BOOL SetClockSource(UINT32 timer, UINT32 src)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> CLK_SRC = src;

	return TRUE;
}

BOOL SetInvert(UINT32 timer, UINT32 onoff)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	cntlPtr -> OUT_INV = onoff;

	return TRUE;
}

void Timer_Auto_Test_Handler(UINT32 irq, UINT32 timer)
{

	TimerIntStat *intrPtr = (TimerIntStat *)(TIMER_FTPWMTMR010_PA_BASE + 0x0);
	if (intrPtr -> TM1_INT_STAT)
	{
		intrPtr -> TM1_INT_STAT = 1;
	}
	else if (intrPtr -> TM2_INT_STAT)
	{
		intrPtr -> TM2_INT_STAT = 1;
	}
	else if (intrPtr -> TM3_INT_STAT)
	{
		intrPtr -> TM3_INT_STAT = 1;
	}
	else if (intrPtr -> TM4_INT_STAT)
	{
		intrPtr -> TM4_INT_STAT = 1;
	}
	else if (intrPtr -> TM5_INT_STAT)
	{
		intrPtr -> TM5_INT_STAT = 1;
	}
	else if (intrPtr -> TM6_INT_STAT)
	{
		intrPtr -> TM6_INT_STAT = 1;
	}
	else if (intrPtr -> TM7_INT_STAT)
	{
		intrPtr -> TM7_INT_STAT = 1;
	}
	else if (intrPtr -> TM8_INT_STAT)
	{
		intrPtr -> TM8_INT_STAT = 1;
	}
	/* open this debug code after intc020 nonos driver fixed
	else
	{
		fLib_printf("Not match one of the interrupt state!");
		while(1);
	}
	*/

	gTick[timer]++;
printf("Timer_Auto_Test_Handler timer = %d, gTick[timer] = %d\n",timer, gTick[timer]);
//fLib_EnableIRQ(irq);

}

void Timer_One_Shot_Test_Handler(UINT32 irq, UINT32 timer)
{
	TimerIntStat *intrPtr = (TimerIntStat *)(TIMER_FTPWMTMR010_PA_BASE + 0x0);
	if (intrPtr -> TM1_INT_STAT)
	{
		intrPtr -> TM1_INT_STAT = 1;
	}
	else if (intrPtr -> TM2_INT_STAT)
	{
		intrPtr -> TM2_INT_STAT = 1;
	}
	else if (intrPtr -> TM3_INT_STAT)
	{
		intrPtr -> TM3_INT_STAT = 1;
	}
	else if (intrPtr -> TM4_INT_STAT)
	{
		intrPtr -> TM4_INT_STAT = 1;
	}
	else if (intrPtr -> TM5_INT_STAT)
	{
		intrPtr -> TM5_INT_STAT = 1;
	}
	else if (intrPtr -> TM6_INT_STAT)
	{
		intrPtr -> TM6_INT_STAT = 1;
	}
	else if (intrPtr -> TM7_INT_STAT)
	{
		intrPtr -> TM7_INT_STAT = 1;
	}
	else if (intrPtr -> TM8_INT_STAT)
	{
		intrPtr -> TM8_INT_STAT = 1;
	}
	/* open this debug code after intc020 nonos driver fixed
	else
	{
		fLib_printf("Not match one of the interrupt state!");
		while(1);
	}
	*/

	gTick[timer]++;

printf("Timer_One_Shot_Test_Handler timer = %d, gTick[timer] = %d\n",timer, gTick[timer]);
//    fLib_EnableIRQ(irq);
	//Set one shot timer
	Write_TimerCNTB(timer, 0xffffff);
	Enable_Timer_Update(timer);
}

void Timer_Auto_Zero_Test_Handler(UINT32 irq, UINT32 timer)
{
	TimerIntStat *intrPtr = (TimerIntStat *)(TIMER_FTPWMTMR010_PA_BASE + 0x0);
	if (intrPtr -> TM1_INT_STAT)
	{
		intrPtr -> TM1_INT_STAT = 1;
	}
	else if (intrPtr -> TM2_INT_STAT)
	{
		intrPtr -> TM2_INT_STAT = 1;
	}
	else if (intrPtr -> TM3_INT_STAT)
	{
		intrPtr -> TM3_INT_STAT = 1;
	}
	else if (intrPtr -> TM4_INT_STAT)
	{
		intrPtr -> TM4_INT_STAT = 1;
	}
	else if (intrPtr -> TM5_INT_STAT)
	{
		intrPtr -> TM5_INT_STAT = 1;
	}
	else if (intrPtr -> TM6_INT_STAT)
	{
		intrPtr -> TM6_INT_STAT = 1;
	}
	else if (intrPtr -> TM7_INT_STAT)
	{
		intrPtr -> TM7_INT_STAT = 1;
	}
	else if (intrPtr -> TM8_INT_STAT)
	{
		intrPtr -> TM8_INT_STAT = 1;
	}
	/* open this debug code after intc020 nonos driver fixed
	else
	{
		fLib_printf("Not match one of the interrupt state!");
		while(1);
	}
	*/

	gTick[timer]++;

printf("Timer_Auto_Zero_Test_Handler timer = %d, gTick[timer] = %d\n",timer, gTick[timer]);
//    fLib_EnableIRQ(irq);
	//Set one shot timer
	int i;
	for (i = 0; i<10000; i++); // make a delay
	Write_TimerCNTB(timer, 0xffffff);
	Write_TimerCNTB(timer, 0x0);

}

void Data_Sheet_Pattern_Test_Handler(UINT32 irq, UINT32 timer)
{
	TimerIntStat *intrPtr = (TimerIntStat *)(TIMER_FTPWMTMR010_PA_BASE + 0x0);
	if (intrPtr -> TM1_INT_STAT)
	{
		intrPtr -> TM1_INT_STAT = 1;
	}
	else if (intrPtr -> TM2_INT_STAT)
	{
		intrPtr -> TM2_INT_STAT = 1;
	}
	else if (intrPtr -> TM3_INT_STAT)
	{
		intrPtr -> TM3_INT_STAT = 1;
	}
	else if (intrPtr -> TM4_INT_STAT)
	{
		intrPtr -> TM4_INT_STAT = 1;
	}
	else if (intrPtr -> TM5_INT_STAT)
	{
		intrPtr -> TM5_INT_STAT = 1;
	}
	else if (intrPtr -> TM6_INT_STAT)
	{
		intrPtr -> TM6_INT_STAT = 1;
	}
	else if (intrPtr -> TM7_INT_STAT)
	{
		intrPtr -> TM7_INT_STAT = 1;
	}
	else if (intrPtr -> TM8_INT_STAT)
	{
		intrPtr -> TM8_INT_STAT = 1;
	}

	gTick[timer]++;

printf("Data_Sheet_Pattern_Test_Handler timer = %d, gTick[timer] = %d\n",timer, gTick[timer]);
//    fLib_EnableIRQ(irq);
    
	if (gTick[timer] == 1)
	{
		Write_TimerCNTB(timer, 0x13880); // 80,000(20,000+60,000)
		Write_TimerCMPB(timer, 0xea60); // 60,000
	}

	if (gTick[timer] == 2)
	{
		Enable_Timer_One_Shot(timer); // disable auto load
		Disable_Timer_INT(timer);
	}
}

BOOL Set_Timer_Int_Mode(UINT32 timer, UINT32 mode)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl *cntlPtr = (TimerControl *)(gTimerBase[timer] + CTRL);
	if (mode == LEVEL)
		cntlPtr -> INT_MO = 0;
	else if (mode == EDGE)
		cntlPtr -> INT_MO = 1;
	else
		while(1);

	return TRUE;
}
