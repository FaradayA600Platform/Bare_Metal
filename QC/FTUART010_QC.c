#include "SoFlexible.h"
#include "serial.h"

#define MAX_BAUDRATE_INDEX	9
extern ScanBaudRate[MAX_BAUDRATE_INDEX];
extern UINT32 UART_TEST_RESULT;
//extern void uart_main(UINT32 u32Port);

void FTUART010_QC_Main(void)
{

#if 0
	UINT32 QCMode = 0;
    UINT32 u32Mode;
	UINT32 u32PreUartPort;
	char *pUartName=NULL;
	UINT32 u32baudrate;
	UINT32 u32BR_index;
	UINT32 u32Port;
   
   	printf("==Start the FTUART010_QC test==\n");
	u32baudrate=0;
	u32Mode = 1;
   	UART_TEST_RESULT = FALSE;
	u32PreUartPort=DebugSerialPort;
   	u32Port=uart_select(u32Port);
    	
	switch(u32Port){   	
		case UART_FTUART010_0_PA_BASE:
			pUartName="UART0";
			break;	
#if IP_COUNT > 1	
		case UART_FTUART010_1_PA_BASE:
			pUartName="UART1";
			break;		
		case UART_FTUART010_2_PA_BASE:
			pUartName="UART2";
			break;		
#endif
	}
    for(u32BR_index=0;u32BR_index<MAX_BAUDRATE_INDEX;u32BR_index++){
	    u32baudrate=BaudRateSelect(ScanBaudRate[u32BR_index]);
   

		    printf("\r%s Internal Loopback Test(Baudrate %d00 bps)\n",pUartName,ScanBaudRate[u32BR_index]);		
			
		 	//set uart loopback mode
		 	fLib_SetSerialLoopBack(u32Port,ON);		    
			    
			if(!UartTxRxTest(u32Port,u32baudrate))
			{
				//Show_Number(99);
				printf("Fail!\n");
			}else{
				//Show_Number(88);			
			    printf("Pass!\n");
			}
			fLib_SetSerialLoopBack(u32Port,OFF);   
	}
#endif
	printf("==Start the FTUART010_QC test==\n");
	//printf("== Test UART #1 (0x%x)== \n", UART_FTUART010_1_PA_BASE);
	//FTUART010_Test_Main();
	//uart_main(1);
	printf("== Test UART #2 (0x%x)== \n", UART_FTUART010_2_PA_BASE);
	uart_main(2);
	printf("End the FTUART010_QC test\n");
}
