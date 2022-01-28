/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "timer.h"
#include "SoFlexible.h"

#define TIMER_CHECK_THRESHOLD	5

/* The Tx and Rx tasks as described at the top of this file. */
static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void prvYxTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );

/* The queue used by the Tx and Rx tasks, as described at the top of this
file. */
static TaskHandle_t xTxTask;
static TaskHandle_t xRxTask;
static TaskHandle_t xYxTask;
static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;
char HWstring[15] = "Hello World";
long RxtaskCntr = 0;
static void test_task(void *pvParameters);

#define mainQUEUE_LENGTH					( 1 )
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainQUEUE_YX_TASK_PRIORITY		( tskIDLE_PRIORITY +2 )

#define mainQUEUE_SEND_FREQUENCY_MS			200

extern uint64_t ullPortInterruptNesting;
/*-----------------------------------------------------------*/

static void tmr_init(void)
{
	//writel(0x400000, 0x20200000);
	//writel(0x400000, 0x20200004);
	//writel(0x00400000, 0x20200004);
	//writel(0x00100000, 0x20200008);
    //writel(0x5, 0x20200030);	
	fLib_Timer_Init(1, 1, Timer1_Tick);
	//while(1);
}

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
unsigned int x10seconds =10000UL;
unsigned int x1second =1UL;

static void prvSetupHardware( void )
{
	tmr_init();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/
/* get from main_blinky.c */
static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const uint32_t ulValueToSend = 100UL;

	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again. */
		/* Send to the queue - causing the queue receive task to unblock and
		toggle the LED.  0 is used as the block time so the sending operation
		will not block - it shouldn't need to block as the queue should always
		be empty at this point in the code. */
		xQueueSend( xQueue, &ulValueToSend, 0U );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
uint32_t ulReceivedValue;
const uint32_t ulExpectedValue = 100UL;

	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
		/* To get here something must have been received from the queue, but is
		it the expected value?  If it is, toggle the LED. */
		if( ulReceivedValue == ulExpectedValue )
		{
		//	printf( "100 received\r\n" );
			ulReceivedValue = 0U;
		}
	}
}

extern void Run_timer_test(int timer, int test, PrHandler handler);
extern BOOL fLib_Timer_Init(UINT32 timer, UINT32 tick, PrHandler handler);

void main_blinky( void )
{
	const TickType_t x10seconds = 20000;

	TickType_t xNextWakeTime;
	const uint32_t ulValueToSend = 100UL;
	
	/* Initialise xNextWakeTime - this only needs to be done once. */
	//xNextWakeTime = xTaskGetTickCount();
	//printf("xNextWakeTime %x\n",xNextWakeTime);
	//vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );
	
    
	/* Create the queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( xQueue != NULL )
	{
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( prvQueueReceiveTask,				/* The function that implements the task. */
					"Rx", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
					NULL, 								/* The parameter passed to the task - not used in this case. */
					mainQUEUE_RECEIVE_TASK_PRIORITY, 	/* The priority assigned to the task. */
					NULL );								/* The task handle is not required, so NULL is passed. */

		xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		/* Start the tasks and timer running. */
		printf("start timer demo task\n");
		vStartTimerDemoTask( 1);
		vTaskStartScheduler();
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was either insufficient FreeRTOS heap memory available for the idle
	and/or timer tasks to be created, or vTaskStartScheduler() was called from
	User mode.  See the memory management section on the FreeRTOS web site for
	more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
	mode from which main() is called is set in the C start up code and must be
	a privileged mode (not user mode). */
	for( ;; );
}



void vMainAssertCalled( const char *pcFileName, uint32_t ulLineNumber )
{
	printf( "ASSERT!  Line %lu of file %s\r\n", ulLineNumber, pcFileName );
	taskENTER_CRITICAL();
	for( ;; );
}

void *____memset(void *str, int c, size_t n)
{
size_t x;
uint8_t *puc = ( uint8_t * ) str;

	for( x = 0; x < c; x++ )
	{
		puc[ x ] = ( uint8_t ) c;
	}

	return str;
}

/*-----------------------------------------------------------*/

void freeRTOS_main(void)
{
	
	prvSetupHardware();

	/* Create the queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );
	printf("gointo timer demo\n");

	if( xQueue == NULL )
	{
		printf("Create queue fail. Exit\n");
		return 0;
	}
#if 1
	printf( "Hello from Freertos example main\r\n" );

	xTimer = xTimerCreate( (const char *) "Timer",
							15,
							pdFALSE,
							(void *) TIMER_ID,
							vTimerCallback);
	/* Check the timer was created. */
	configASSERT( xTimer );

	/* start the timer with a block time of 0 ticks. This means as soon
	   as the schedule starts the timer will start running and will expire after
	   10 seconds */
	xTimerStart( xTimer, 0 );

		/* Create the two tasks.  The Tx task is given a lower priority than the
		Rx task, so the Rx task will leave the Blocked state and pre-empt the Tx
		task as soon as the Tx task places an item in the queue. */
		xTaskCreate( 	prvTxTask, 					/* The function that implements the task. */
					( const char * ) "Tx", 		/* Text name for the task, provided to assist debugging only. */
					configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
					NULL, 						/* The task parameter is not used, so set to NULL. */
					mainQUEUE_SEND_TASK_PRIORITY,			/* The task runs at the idle priority. */
					&xTxTask );
		
		xTaskCreate( 	prvRxTask, 					/* The function that implements the task. */
					( const char * ) "Rx", 		/* Text name for the task, provided to assist debugging only. */
					configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
					NULL, 						/* The task parameter is not used, so set to NULL. */
					mainQUEUE_RECEIVE_TASK_PRIORITY,			/* The task runs at the idle priority. */
					&xRxTask );					
		xTaskCreate( 	prvYxTask, 					/* The function that implements the task. */
					( const char * ) "Yx", 		/* Text name for the task, provided to assist debugging only. */
					configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
					NULL, 						/* The task parameter is not used, so set to NULL. */
					mainQUEUE_YX_TASK_PRIORITY,			/* The task runs at the idle priority. */
					&xYxTask );					

		vTaskStartScheduler();	
		for(;;);
		
#else
	main_blinky();
#endif
}

static void prvTxTask( void *pvParameters )
{
	static int loop_cnt = 0;	

	for( ;; )
	{
		//__mdelay(100);		
		//vTaskDelay(1);
        //printf("%s loop %d\n",__func__,loop_cnt++);
		/* Send the next value on the queue.  The queue should always be
		empty at this point so a block time of 0 is used. */
		#if 1 
		xQueueSend( xQueue,			/* The queue being written to. */
					HWstring, /* The address of the data being sent. */
					0UL );			/* The block time. */
		#endif			
	}
}

/*-----------------------------------------------------------*/
static void prvRxTask( void *pvParameters )
{
	static int loop_cnt = 0;
	for( ;; )
	{
		/* Block to wait for data arriving on the queue. */
	// printf("%s loop %d\n",__func__,loop_cnt++);
		#if 1 
		xQueueReceive( 	xQueue,				/* The queue being read. */
						HWstring,	/* Data is read into this address. */
						portMAX_DELAY );	/* Wait without a timeout for data. */

		/* Print the received data. */
		RxtaskCntr++;
		#endif
	}
}

/*-----------------------------------------------------------*/
static void prvYxTask( void *pvParameters )
{
	static int loop_cnt = 0;
	for( ;; )
	{
		/* Block to wait for data arriving on the queue. */
		//__mdelay(100);
        //printf("%s loop %d\n",__func__,loop_cnt++);
		#if 1 
		xQueueReceive( 	xQueue,				/* The queue being read. */
						HWstring,	/* Data is read into this address. */
						portMAX_DELAY );	/* Wait without a timeout for data. */

		/* Print the received data. */
		#endif
	}
}

/*-----------------------------------------------------------*/
static void vTimerCallback( TimerHandle_t pxTimer )
{
	/* If the RxtaskCntr is updated every time the Rx task is called. The
	 Rx task is called every time the Tx task sends a message. The Tx task
	 sends a message every 1 second.
	 The timer expires after 10 seconds. We expect the RxtaskCntr to at least
	 have a value of 9 (TIMER_CHECK_THRESHOLD) when the timer expires. */
	if (RxtaskCntr >= TIMER_CHECK_THRESHOLD) {
		printf("FreeRTOS Hello World Example PASSED");
	} else {
		printf("FreeRTOS Hello World Example FAILED");
	}
while(1);
	vTaskDelete( xRxTask );
	vTaskDelete( xTxTask );
	vTaskDelete( xYxTask );	
	
}

