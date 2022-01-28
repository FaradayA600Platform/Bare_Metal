#include <stdio.h>
#include "SoFlexible.h"
#include "pwmtmrtest.h"
#include "pwmtmr.h"
//#include "cpu.h"


#define PCLK        0
#define EXT_CLK     1
#define OFF         0
#define ON          1
#define NOTFIQ      0

extern UINT32 gTimer_vectors[];
extern UINT32 gTick[];
extern void Timer_Auto_Test_Handler(UINT32, UINT32);
extern void Timer_One_Shot_Test_Handler(UINT32, UINT32);
extern void Timer_Auto_Zero_Test_Handler(UINT32, UINT32);
extern void Data_Sheet_Pattern_Test_Handler(UINT32, UINT32);
UINT32 TIMER_TEST_RESULT;

void Setup_Timer(UINT32 timer, UINT32 test)
{
	if (test == 1) // auto internal edge test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, EDGE);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], EDGE);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffffff);
		SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
	else if (test == 2) // auto internal level test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);
/*#if 1
		Write_TimerCNTB(timer, 0x27100);
		Write_TimerCMPB(timer, 0x1adb0);
		//SetInvert(timer, ON);
		Enable_PWM(timer);
#else    */
		Write_TimerCNTB(timer, 0xffffff);
//#endif
		SetClockSource(timer, PCLK);
		printf("2 test start...\n");
		//Enable_Timer_Auto(timer);
		Timer_Start(timer);
	}
#if defined(CONFIG_FTPWMTMR_EXTCLK)
	else if (test == 3) // auto external edge test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, EDGE);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], EDGE);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffff);
		Enable_PWM(timer);
		SetClockSource(timer, EXT_CLK);
		Timer_Start(timer);
	}
	else if (test == 4) // auto external level test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffff);
		SetClockSource(timer, EXT_CLK);
		Timer_Start(timer);
	}
#endif
	else if (test == 5) // one shot internal edge test
	{
		Enable_Timer_One_Shot(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, EDGE);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_One_Shot_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], EDGE);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffffff);
		Enable_Timer_Update(timer);
		SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
	else if (test == 6) // one shot internal level test
	{
		Enable_Timer_One_Shot(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_One_Shot_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffffff);
		Enable_Timer_Update(timer);
		SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
#if defined(CONFIG_FTPWMTMR_EXTCLK)
	else if (test == 7) // one shot external edge test
	{
		Enable_Timer_One_Shot(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, EDGE);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_One_Shot_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], EDGE);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffff);
		Enable_Timer_Update(timer);
		SetClockSource(timer, EXT_CLK);
		Timer_Start(timer);
	}
	else if (test == 8) // one shot external level test
	{
		Enable_Timer_One_Shot(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_One_Shot_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffff);
		Enable_Timer_Update(timer);
		SetClockSource(timer, EXT_CLK);
		Timer_Start(timer);
	}
#endif
	else if (test == 9) // auto reload 0 edge
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Zero_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffffff);
		Write_TimerCNTB(timer, 0x0);
		SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
	else if (test == 10) // data sheet pattern test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, LEVEL);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Data_Sheet_Pattern_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], LEVEL);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0x27100); // 160,000(50,000+110,000)
		Write_TimerCMPB(timer, 0x1adb0); // 110,000
		//SetInvert(timer, ON);
		Enable_PWM(timer);
		
		//Enable_Timer_Update(timer);
		//Disable_Timer_Update(timer);
		//SetDeadZone(timer, 0x2);
		//SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
	if (test == 12) // dead zone test
	{
		Enable_Timer_Auto(timer);

		//set timer IRQ
		Set_Timer_Int_Mode(timer, EDGE);
		Enable_Timer_INT(timer);

		//set GIC
		fLib_CloseIRQ(gTimer_vectors[timer]);
		fLib_ConnectIRQ(gTimer_vectors[timer], (PrHandler)Timer_Auto_Test_Handler, timer);
		fLib_SetIRQmode(gTimer_vectors[timer], EDGE);
		fLib_EnableIRQ(gTimer_vectors[timer]);

		Write_TimerCNTB(timer, 0xffffff);
		Enable_PWM(timer);
		SetDeadZone(timer, 0xa);
		SetClockSource(timer, PCLK);
		Timer_Start(timer);
	}
}

void Run_timer_test(UINT32 timer, UINT32 test)
{
	UINT32 counter = 0, current_tick = 0, display_counter = 0;
	BOOL changeMode = TRUE;

	if (test == 11)
		Setup_Timer(timer, 1);
	else
		Setup_Timer(timer, test);
	Timer_Start(timer);
	while(1)
	{
		current_tick = gTick[timer];
		if(current_tick != display_counter)
		{
			display_counter = current_tick;
			printf("%d\n", display_counter);
		}

		if (current_tick >= UPPER_COUNT)
			break;

		if ((test == 10) && (current_tick == 2))
			break;

		if (current_tick == 0)
			counter++;

		if (counter >= TIMEOUT_COUNTER)
		{
			TIMER_TEST_RESULT = FALSE;
			printf("test false\n\r");
			break;
		}

		if ((test == 11) && (current_tick == 0x5) && changeMode)
		{
			changeMode = FALSE;
			fLib_printf("*** change mode ***\n");
			Setup_Timer(timer, 5);
		}
	}

	if (test == 10);
	else if (display_counter != UPPER_COUNT)
		TIMER_TEST_RESULT = FALSE;

	// stop timer
	gTick[timer] = 0;

	int i;
	if (test == 10)
		for (i = 0; i<10000000; i++);

	Timer_Stop(timer);
}

void Burn_in_test()
{
	UINT32 timer, test;

	printf("\r====== Start burn-in test ======\n");
	for (timer=1; timer<MAX_TIMER+1; timer++)
	{
		for (test=1;test<=12;test++)
		{
#if defined(CONFIG_FTPWMTMR010_TLM_MODEL)
			// skip edge interrupt test.
			if ((test == 1) || (test == 3) || (test == 5) || (test == 7) || (test == 11) || (test == 12))
				continue;
#endif
			fLib_printf("<<< run timer %u test %u >>>\n", timer, test);
			Run_timer_test(timer, test);
		}
	}

	printf("\r====== End burn-in test ======\n");
	if ( TIMER_TEST_RESULT == TRUE)
		printf("All Test PASS!!!\n\n");
	else
		printf("Some Tests FAIL\n\n");	
}

void FTPWMTMR_Test_Main()
{
	char buf[128];
	UINT32 timer, test;
	int i, j;
	TIMER_TEST_RESULT = TRUE;

	while(1)
	{
		for (i=1; i<MAX_TIMER+1; i++)
		{
			printf("%d: Timer%d\n", i, i);
		}

//printf("%d: timer burn-in test\n", MAX_TIMER+1);
		printf("0: exit\n");
		printf("which one:");
		gets(buf);
		printf("\r");
		timer = atoi(buf);

		if (timer == MAX_TIMER+1)
			Burn_in_test();
		else if ((timer > 0) && (timer < MAX_TIMER+1))
		{
#if !defined(CONFIG_FTPWMTMR010_TLM_MODEL)
//			printf("1: auto: internal clock source with edge intr test\n");
#endif
			printf("2: auto: internal clock source with level intr test (PWM function enabled)\n");
#if defined(CONFIG_FTPWMTMR010_EXTCLK)
#if !defined(CONFIG_FTPWMTMR010_TLM_MODEL)
//			printf("3: auto: external clock source with edge intr test\n");
#endif
			printf("4: auto: external clock source with level intr test\n");
#endif

#if !defined(CONFIG_FTPWMTMR010_TLM_MODEL)
//			printf("5: one shot: internal clock source with edge intr test\n");
#endif
			printf("6: one shot: internal clock source with level intr test\n");
#if defined(CONFIG_FTPWMTMR010_EXTCLK)
#if !defined(CONFIG_FTPWMTMR010_TLM_MODEL)
//			printf("7: one shot: external clock source with edge intr test\n");
#endif
			printf("8: one shot: external clock source with level intr test\n");
#endif

			printf("9: auto reload zero test\n");
//			printf("10: data sheet pattern test\n");
#if !defined(CONFIG_FTPWMTMR010_TLM_MODEL)
//			printf("11: change mode test(from Linux)\n");
//			printf("12: dead zone test\n");
#endif
			printf("0: exit\n");
			printf("which one:");
			gets(buf);
			printf("\r");
			test = atoi(buf);
			
			if ((test > 0) && (test <= 13))
				Run_timer_test(timer, test);
			else
				continue;
		}
		else if (timer == 0)
		{
			return;
		}
	}
	
	return;
}
