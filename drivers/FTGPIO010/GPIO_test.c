// --------------------------------------------------------------------
//	todo list:1. connect int
// --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SoFlexible.h"

#include "GPIO.h"			//library
#include "GPIO_test.h"

#define IPMODULE 		GPIO
#define IPNAME 			FTGPIO010


extern void DelayTime1(UINT32);
extern void Output2DebugPort(UINT32);
UINT32 GPIOGetS(void);


extern unsigned int DebugSerialPort;
extern char fLib_getch(UINT32 port);

int button_status=0;


static void GPIO_LEDTest(unsigned int io_base, unsigned int pattern)
{
	int i=0;

	printf("Begin GPIO LED Test..., press ESC to exit\n");
	
	for (i=0; i<32; ++i)
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_SetGpioDir(io_base, i, GPIO_DIR_OUTPUT);
		}
	}
	
	// Turn off LED
	fLib_Gpio_SetData(io_base, pattern);

	for (i=0; i<32; ++i)
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_Gpio_ClearData(io_base, 1<<i);				// turn on LED
#if defined(CONFIG_PLATFORM_A380)			
			udelay(100000);
#endif
		}
	}

	for (i=0; i<32; ++i)
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_Gpio_SetData(io_base, 1<<i);				// turn off LED
#if defined(CONFIG_PLATFORM_A380)			
			udelay(100000);
#endif
		}
	}
	
	printf("End GPIO LED Test!\n");
}



static void GPIOServiceRoutine(int irq, unsigned int io_base)
{
	button_status=fLib_GetGpioIntMaskStatus(io_base);

	//clear CPIO interrupt
	fLib_ClearGpioInt(io_base, button_status);
	fLib_ClearIRQ( irq);
}


static void GPIO_InputTest(UINT32 io_base, UINT32 irq, UINT32 pattern,UINT32 single_both_edge)
{
    UINT32 i;
    UINT32 trigger_level, trigger_mode;
    unsigned char chr;

	

	// Because some GPIO are pull low, so must use edge trigger!!
	trigger_level = GPIO_EDGE;


	// In the normal case, GPIO will pull high, when push button, GPIO will pull low
	// So set interrupt to be low active
	trigger_mode = GPIO_Low;			/// edge trigger ==> failing edge, level trigger ==> low level
	
    for (i=0; i<32; ++i)
    {
    	if ( (pattern>>i)&0x1 )
    	{
        	fLib_DisableGpioInt(io_base, i);
        }
    }

	// Input and low level trigger
	for (i=0; i<32; ++i)
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_SetGpioDir(io_base, i,GPIO_DIR_INPUT);
			fLib_SetGpioEdgeMode(io_base, i, single_both_edge);
			fLib_SetGpioTrigger(io_base, i, trigger_level);
			fLib_SetGpioActiveMode(io_base, i, trigger_mode);
			//clear CPIO interrupt
			fLib_ClearGpioInt(io_base, 1<<i);

			fLib_EnableGpioBounce(io_base, i, APB_CLK / 3); // 0.33 second
				
			//Enable GPIO interrupt
			fLib_SetGpioIntEnable(io_base, i);
			fLib_SetGpioIntUnMask(io_base, i);
		}
	}

	printf("Begin GPIO Button Test...\n");

	//Set ISR
	fLib_ConnectIRQ( irq, (PrHandler)GPIOServiceRoutine, io_base);
	fLib_EnableIRQ( irq);

	printf("Press Any GPIO Button...\n");

}

void GPIO_wait_for_test(UINT32 pattern){
    UINT32 i,j;
    unsigned char chr;

    while (TRUE)
	{
		for (i=0; i<32; ++i)
		{
			if ( (pattern>>i)&0x1 )
			{
				if ( (button_status&(1<<i)) != 0)
				{
					printf("button %d is pressed!\n", i);
					button_status = 0;
				}
			}
		}
		chr = fLib_getch(DebugSerialPort);
        if (chr==0x1b)
		{
			break;
		}
	}

	//Disable GPIO interrupt
	for (j=0;j<IP_COUNT;j++){

		for (i=0; i<32; ++i)
		{
			if ( (pattern>>i)&0x1 )
			{
				fLib_SetGpioIntDisable(IP_pa_base[j], i);
			}
		}
		fLib_CloseIRQ( IP_irq[j]);					//close ISR
	}

	printf("End GPIO Test!\n");


}




void GPIO_InputTest_CPUIdel(unsigned int pattern)
{
	UINT32 num;

	printf("\rwhich GPIO would you want to test (0~%d)?",IP_COUNT-1);

	num = GPIOGetS();
	printf("num=%d\n",num);
	switch (num){
		case 0:
			pin_mux_enable(IP_GPIO0,0);
			break;

#if (IP_COUNT>1)
		case 1:
			pin_mux_enable(IP_GPIO1,0);
			break;
#endif
		default:
			printf("on such GPIO!!! \n\n");
			return;
	}

    fLib_SetIRQmode(IP_irq[num], LEVEL);
	GPIO_InputTest(IP_pa_base[num], IP_irq[num], pattern, BOTH);

	printf("Press Any GPIO Button... RIchardLin Please enter CPU Idle \n");
}

#if 0
void GPIO_InputTest_CPUIdel(unsigned int io_base, int irq, unsigned int pattern)
{
    UINT32 i;
    UINT32 trigger_level, trigger_mode, single_both_edge;
    unsigned char chr;

	printf("Please input interrupt edge trigger method, 0:single edge, 1 for both edge addr 0x%0x irq %d \n", io_base, irq);
	single_both_edge = GPIOGetS();
	if (single_both_edge == 0)
	{
	    single_both_edge = SINGLE;
	}
	else
	{
	    single_both_edge = BOTH;	
	}
	
#if 0
	printf("Please input 0:level trigger, 1 for edge trigger:\n");
	trigger_level = GPIOGetS();
	if (trigger_level == 0){
	    trigger_level = GPIO_LEVEL;
	}
	else{
	    trigger_level = GPIO_EDGE;	
	}
#endif

	// Because some GPIO are pull low, so must use edge trigger!!
	trigger_level = GPIO_EDGE;

#if 0
	printf("Please input 0:LOW/FALLING, 1:HIGH/RISING:\n");
	trigger_mode = GPIOGetS();
	if (trigger_mode == 0){
	    trigger_mode = GPIO_Low;
	}
	else{
	    trigger_mode = GPIO_High;	
	}
#endif

	// In the normal case, GPIO will pull high, when push button, GPIO will pull low
	// So set interrupt to be low active
	trigger_mode = GPIO_Low;			/// edge trigger ==> failing edge, level trigger ==> low level
	
    for (i=0; i<32; ++i)
    {
    	if ( (pattern>>i)&0x1 )
    	{
        	fLib_DisableGpioInt(io_base, i);
        }
    }

	// Input and low level trigger
	for (i=0; i<32; ++i)
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_SetGpioDir(io_base, i,GPIO_DIR_INPUT);
			fLib_SetGpioEdgeMode(io_base, i, single_both_edge);
			fLib_SetGpioTrigger(io_base, i, trigger_level);
			fLib_SetGpioActiveMode(io_base, i, trigger_mode);
			//clear CPIO interrupt
			fLib_ClearGpioInt(io_base, 1<<i);

			fLib_EnableGpioBounce(io_base, i, APB_CLK / 3); // 0.33 second
				
			//Enable GPIO interrupt
			fLib_SetGpioIntEnable(io_base, i);
			fLib_SetGpioIntUnMask(io_base, i);
		}
	}

	printf("Begin GPIO Button Test...\n");

	//Set ISR
	fLib_ConnectIRQ( irq, (PrHandler)GPIOServiceRoutine, io_base);
	fLib_EnableInt(IRQ_IAmIRQ, irq);

	printf("Press Any GPIO Button... RIchardLin Please enter CPU Idle \n");
}
#endif

UINT32 GPIOGetS(void)
{
	char Buffer[256];
	char seps[]  = " ";
	char *token;
	char Cmd[256];
	UINT32 i;

	gets(Buffer);

	token = strtok( Buffer, seps );
	if( token == NULL )
		return (0);
       
	strcpy(Cmd, token);
	
	i = atoi( Cmd );

	return(i);
}



void FTGPIO010_Test_Main()
{
	UINT32 single_both_edge=BOTH;

	printf("\rPlease input interrupt edge trigger method, 0:single edge, 1 for both edge\n");
	single_both_edge = GPIOGetS();
	if (single_both_edge == 0)
	{
	    single_both_edge = SINGLE;
	}
	else
	{
	    single_both_edge = BOTH;	
	}

#if defined(CONFIG_PLATFORM_A380)
	#define SCU_BASE	0x90A00000	
	#define PMUR_MFS0	0x1120	
	#define PMUR_MFS1	0x1124	
	#define PMUR_MFS2	0x1128	
	#define PMUR_MFS3	0x112c	
	
	unsigned long scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3; 
	
	// Enable GPIO pin mux	
	scu_pmur_mfs0 = readl(SCU_BASE + PMUR_MFS0);	
	scu_pmur_mfs0 &= (~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT4|BIT15));	
	scu_pmur_mfs0 |= (BIT0);	
	writel(scu_pmur_mfs0, SCU_BASE + PMUR_MFS0);			

	scu_pmur_mfs1 = readl(SCU_BASE + PMUR_MFS1);			

	scu_pmur_mfs2 = readl(SCU_BASE + PMUR_MFS2);		

	scu_pmur_mfs3 = readl(SCU_BASE + PMUR_MFS3);		

	printf("scu_pmur_mfs0: 0x%x, scu_pmur_mfs1: 0x%x, scu_pmur_mfs2: 0x%x, scu_pmur_mfs3: 0x%x\n",
			scu_pmur_mfs0, scu_pmur_mfs1, scu_pmur_mfs2, scu_pmur_mfs3);

	// Test LED
	GPIO_LEDTest(IP_PA_BASE(0), 0xff);

#endif //CONFIG_PLATFORM_A380

	printf("which GPIO would you want to test (0~%d)?",IP_COUNT-1);

	switch (GPIOGetS()){
		case 0:		
//			pin_mux_enable(IP_GPIO0,0);
            		fLib_SetIRQmode(IP_IRQ(0), LEVEL);
			GPIO_InputTest(IP_PA_BASE(0), IP_IRQ(0), 0xFFFFFFFF, single_both_edge);
			break;

#if (IP_COUNT>1)
		case 1:
			pin_mux_enable(IP_GPIO1,0);
            fLib_SetIRQmode(IP_IRQ(1), LEVEL);
			GPIO_InputTest(IP_PA_BASE(1), IP_IRQ(1), 0xff000000, single_both_edge);
			break;
#endif

		default:
			printf("on such GPIO!!! \n\n");
			return;
	}

	GPIO_wait_for_test(0xFFFFFFFF);




}
