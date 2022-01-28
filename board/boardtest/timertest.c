#include <stdio.h>
#include "SoFlexible.h"
#include "timertest.h"
#include "timer.h"

/* //for debug purpose by Edward
typedef struct
{
    char str[100];
}Str;

Str output[10000];

UINT32 ind = 0;
// */

UINT32 TIMER_TEST_RESULT;


UINT32 GetCurrentTick(int timer)
{
        if (timer == 1)
                return fLib_CurrentT1Tick();
        else if (timer == 2)
                return fLib_CurrentT2Tick();
        else if (timer == 3)
                return fLib_CurrentT3Tick();
}

void Run_timer_test(int timer, int test, PrHandler handler)
{       
        UINT32  counter = 0, current_tick = 0, display_counter = 0;
        
        if (test == 1)
        {
#ifdef CONFIG_QC
                fLib_Timer_Counter(timer, 0xfeffffff);
                fLib_Timer_AutoReloadValue(timer, 0xfeffffff);
#else
                fLib_Timer_Counter(timer, 0xed6fffff);
                fLib_Timer_AutoReloadValue(timer, 0xed6fffff);
#endif
                fLib_Timer_MatchValue1(timer, 0xffffffff);
                fLib_Timer_MatchValue2(timer, 0xffffffff);
                fLib_Write_CR(timer, PCLK, UP);
        }
        else if (test == 2)
        {
                fLib_Timer_Counter(timer, 0x12900000);
                fLib_Timer_AutoReloadValue(timer, 0x12900000);
                fLib_Timer_MatchValue1(timer, 0xc600000);
                fLib_Timer_MatchValue2(timer, 0x3600000);
                fLib_Write_CR(timer, PCLK, DOWN);
        }
        else if (test == 3)
        {   
                fLib_Timer_Counter(timer, 0xffffffff/2);
                fLib_Timer_AutoReloadValue(timer, 0xffffffff);
                fLib_Timer_MatchValue1(timer, 0xffffffff);                  
                fLib_Timer_MatchValue2(timer, 0xffffffff);
                fLib_Write_CR(timer, PCLK, UP);
        }
        else if (test == 4)
        {
                fLib_Timer_Counter(timer, 0xffffffff/2);
                fLib_Timer_AutoReloadValue(timer, 0);
                fLib_Timer_MatchValue1(timer, 0x0);
                fLib_Timer_MatchValue2(timer, 0x0);
                fLib_Write_CR(timer, PCLK, DOWN);
        }

#if defined(CONFIG_TMR_EXTCLK)
        else if (test == 5)
        {
                fLib_Timer_Counter(timer, 0xf9cfffff);
                fLib_Timer_AutoReloadValue(timer, 0xf9cfffff);
                fLib_Timer_MatchValue1(timer, 0xfbdfffff);
                fLib_Timer_MatchValue2(timer, 0xfdefffff);
                fLib_Write_CR(timer, EXTCLK, UP);
        }
        else if (test == 6)
        {
                fLib_Timer_Counter(timer, 0x6300000);
                fLib_Timer_AutoReloadValue(timer, 0x6300000);
                fLib_Timer_MatchValue1(timer, 0x4200000);
                fLib_Timer_MatchValue2(timer, 0x2100000);
                fLib_Write_CR(timer, EXTCLK, DOWN);
        }       
        else if (test == 7)
        {   
                fLib_Timer_Counter(timer, 0xffffffff/2);
                fLib_Timer_AutoReloadValue(timer, 0xffffffff);
                fLib_Timer_MatchValue1(timer, 0xffffffff);
                fLib_Timer_MatchValue2(timer, 0xffffffff);
                fLib_Write_CR(timer, PCLK, UP);
        }
        else if (test == 8)
        {   
                fLib_Timer_Counter(timer, 0xffffffff/2);
                fLib_Timer_AutoReloadValue(timer, 0); 
                fLib_Timer_MatchValue1(timer, 0x0);
                fLib_Timer_MatchValue2(timer, 0x0);
                fLib_Write_CR(timer, PCLK, DOWN);
        }

#endif

        // sprintf(output[ind++].str, "\r=========== start testing ===========\n");

        if (!fLib_Timer_Init(timer, 1, handler))
        {
                printf("Init Fail!\n");
        }
        else
        {
                while(1)
                {
                        current_tick = GetCurrentTick(timer);
                        if(current_tick != display_counter)
                        {
                                display_counter = current_tick;
                                printf("%d\n", display_counter);
                                // sprintf(output[ind++].str, "display_counter = %d\n", display_counter);
                        }

                        if (current_tick >= UPPER_COUNT)
                                break;

                        if (current_tick == 0)
                                counter++;

                        if (counter >= TIMEOUT_COUNTER)
                        {
                                TIMER_TEST_RESULT = FALSE;
                                break;
                        }
                }
        }

        fLib_Timer_Close(timer);
        if (display_counter != UPPER_COUNT)
                TIMER_TEST_RESULT = FALSE;
}

void Setup(int timer, int test, PrHandler handler)
{
        printf("\r====== Start Timer%d Test%d overflow test ======\n", timer, test);
        if (test == 3 || test == 4 || test == 7 || test == 8)
		fLib_printf("<one shot test>\n");
	
	if (test > 4 )
		fLib_printf("<Use external clock>\n");

        if (!fLib_Write_MR(timer, 0, 0, 0)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);
        if ( TIMER_TEST_RESULT == TRUE)
                printf("Test PASS\n");
        else
                printf("Test FAIL\n");

        printf("\r====== Start Timer%d Test%d match1 test ======\n", timer, test);
        if (!fLib_Write_MR(timer, 0, 1, 1)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);
        if ( TIMER_TEST_RESULT == TRUE)
                printf("Test PASS\n");
        else
                printf("Test FAIL\n");

        printf("\r====== Start Timer%d Test%d match2 test ======\n", timer, test);
        if (!fLib_Write_MR(timer, 1, 0, 1)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);
        if ( TIMER_TEST_RESULT == TRUE)
                printf("Test PASS\n");
        else
                printf("Test FAIL\n");

        printf("\r====== Start Timer%d Test%d combination test ======\n", timer, test);
        if (!fLib_Write_MR(timer, 1, 1, 0)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);

        if (!fLib_Write_MR(timer, 1, 0, 1)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");
        
        Run_timer_test(timer, test, handler);

        if (!fLib_Write_MR(timer, 0, 0, 0)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);

        if (!fLib_Write_MR(timer, 0, 0, 1)) // timer, match1, match2, over
                printf("Write mask register Fail!\n");

        Run_timer_test(timer, test, handler);
        if ( TIMER_TEST_RESULT == TRUE)
                printf("Test PASS\n");
        else
                printf("Test FAIL\n");  
}

void Burn_in_test()
{
        int timer, test;
        printf("\r====== Start burn-in test ======\n");
        timer = 1;
        test = 1;
        Setup(timer, test, (PrHandler)Timer1_Tick);
        
        timer = 1;
        test = 2;
        Setup(timer, test, (PrHandler)Timer1_Tick);
        
        timer = 2;
        test = 1;
        Setup(timer, test, (PrHandler)Timer2_Tick);
        
        timer = 2;
        test = 2;
        Setup(timer, test, (PrHandler)Timer2_Tick);
        
        timer = 3;
        test = 1;
        Setup(timer, test, (PrHandler)Timer3_Tick);
        
        timer = 3;
        test = 2;
        Setup(timer, test, (PrHandler)Timer3_Tick);

#if defined(CONFIG_TMR_EXTCLK)
        timer = 1;
        test = 5;
        Setup(timer, test, (PrHandler)Timer3_Tick);                               
            
        timer = 1;
        test = 6;
        Setup(timer, test, (PrHandler)Timer3_Tick);
 
        timer = 2;
        test = 5;
        Setup(timer, test, (PrHandler)Timer3_Tick);    
        
	    timer = 2;
        test = 6;
        Setup(timer, test, (PrHandler)Timer3_Tick);        

        timer = 3;
        test = 5;
        Setup(timer, test, (PrHandler)Timer3_Tick);
        
        timer = 3;
        test = 6;
        Setup(timer, test, (PrHandler)Timer3_Tick);

        timer = 1;
        test = 7;
        Setup(timer, test, (PrHandler)Timer3_Tick);     
        timer = 1;
        test = 8;
        Setup(timer, test, (PrHandler)Timer3_Tick);

#endif

        timer = 1;
        test = 3;
        //Setup(timer, test, (PrHandler)Timer1_Tick_VP);
        
        timer = 1;
        test = 4;
        //Setup(timer, test, (PrHandler)Timer1_Tick_VP);
        
        printf("\r====== End burn-in test ======\n");
        if ( TIMER_TEST_RESULT == TRUE)
                printf("All Test PASS!!!\n\n");
        else
                printf("Some Tests FAIL\n\n");  
}

void Timer_Test_Main()
{
        char buf[128];
        int timer, test;        
        int i;
        TIMER_TEST_RESULT = TRUE;

        for (;;)
        {
                for (i=1; i <= NUMOFTIMER; i++)
                {
                        printf("%d: Timer%d\n", i, i);
                }

                printf("%d: timer burn-in test\n", NUMOFTIMER+1);
                printf("0: exit\n");
                printf("which one:");
                gets(buf);
                printf("\r");
                timer = atoi(buf);
                if (timer == NUMOFTIMER+1)
                {
                        Burn_in_test();
                }
                else if ((timer > 0) && (timer <= NUMOFTIMER))
                {
                        printf("== internal clock source test ==\n");
                        printf("1: internal clock source with up count test\n");
                        printf("2: internal clock source with down count test\n");
                        printf("3: one shot: internal clock source with up count test\n");
                        printf("4: one shot: internal clock source with down count test\n");
#if defined(CONFIG_TMR_EXTCLK)
                        printf("== external clock source test ==\n");
                        printf("5: external clock source with up count test\n");
                        printf("4: external clock source with down count test\n");
                        printf("7: one shot: external clock source with up count test\n");
                        printf("8: one shot: external clock source with down count test\n");
#endif
                        printf("0: exit\n");
                        printf("which one:");
                        gets(buf);
                        printf("\r");
                        test = atoi(buf);
                        if ((test > 0) && (test <= 4))
                        {
                                if (timer == 1)
                                        Setup(timer, test, (PrHandler)Timer1_Tick);
                                else if (timer == 2)
                                        Setup(timer, test, (PrHandler)Timer2_Tick);
                                else if (timer == 3)
                                        Setup(timer, test, (PrHandler)Timer3_Tick);
                        }       
#if defined(CONFIG_TMR_EXTCLK)
                        else if ((test > 0) && (test > 4 && test <= 8)) 
                        {   
                                if (timer == 1)
                                        Setup(timer, test, (PrHandler)Timer1_Tick_VP);
                                else if (timer == 2)
                                        Setup(timer, test, (PrHandler)Timer2_Tick_VP);
                                else if (timer == 3)
                                        Setup(timer, test, (PrHandler)Timer3_Tick_VP);
                        }   
#endif
                        else if (timer == 0)
                        {   
                                continue;
                        }   
                }   
                else if (timer == 0)
                {   
                        return;
                }   
        }   
            
        return;
}
