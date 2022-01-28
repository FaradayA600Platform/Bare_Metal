/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/*
 * Creates two transmitting tasks and two receiving tasks.  The transmitting
 * tasks send values that are received by the receiving tasks.  One set of tasks
 * uses the standard API.  The other set of tasks uses the zero copy API.
 *
 * See the following web page for essential demo usage and configuration
 * details:
 * http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/examples_FreeRTOS_simulator.html
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#define simpTINY_DELAY	( ( TickType_t ) 2 )

/*
 * Uses a socket to receive the data sent by the prvSimpleClientTask() task.
 * Does not use the zero copy option.
 */
static void prvSimpleServerTask( void *pvParameters );

/*-----------------------------------------------------------*/

void vStartSimpleUDPEchoServerTasks( uint16_t usStackSize, uint32_t ulPort, UBaseType_t uxPriority )
{
	/* Create the client and server tasks that do not use the zero copy
	interface. */
	xTaskCreate( prvSimpleServerTask, "SimpCpySrv", usStackSize, ( void * ) ulPort, uxPriority + 1, NULL );
}
/*-----------------------------------------------------------*/

static void prvSimpleServerTask( void *pvParameters )
{
int32_t lBytes;
uint8_t cReceivedString[ 60 ];
struct freertos_sockaddr xClient, xBindAddress;
uint32_t xClientLength = sizeof( xClient );
Socket_t xListeningSocket;

	/* Just to prevent compiler warnings. */
	( void ) pvParameters;

	/* Attempt to open the socket. */
	xListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
	configASSERT( xListeningSocket != FREERTOS_INVALID_SOCKET );

	/* This test receives data sent from a different port on the same IP
	address.  Configure the freertos_sockaddr structure with the address being
	bound to.  The strange casting is to try and remove	compiler warnings on 32
	bit machines.  Note that this task is only created after the network is up,
	so the IP address is valid here. */
	xBindAddress.sin_port = ( uint16_t ) ( ( uint32_t ) pvParameters ) & 0xffffUL;
	xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );

	/* Bind the socket to the port that the client task will send to. */
	FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

	for( ;; )
	{
		/* Zero out the receive array so there is NULL at the end of the string
		when it is printed out. */
		memset( cReceivedString, 0x00, sizeof( cReceivedString ) );

		/* Receive data on the socket.  ulFlags is zero, so the zero copy option
		is not set and the received data is copied into the buffer pointed to by
		cReceivedString.  By default the block time is portMAX_DELAY.
		xClientLength is not actually used by FreeRTOS_recvfrom(), but is set
		appropriately in case future versions do use it. */
		lBytes = FreeRTOS_recvfrom( xListeningSocket, cReceivedString, sizeof( cReceivedString ), 0, &xClient, &xClientLength );

		if (lBytes > 0) {
			FreeRTOS_printf( ("UDP echo server received(%d bytes): %s\n", lBytes, cReceivedString) );
			
			/* Error check. */
			configASSERT( lBytes == ( BaseType_t ) strlen( ( const char * ) cReceivedString ) );
			
			FreeRTOS_sendto( xListeningSocket, cReceivedString, lBytes, 0, &xClient, sizeof( xClient ) );
		}
	}
}
