#include <stdint.h>
#include <stdio.h>
 
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
 
/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
 
#include "TCPServer.h"
 
/* The maximum time to wait for a closing socket to close. */
#define tcpechoSHUTDOWN_DELAY   ( pdMS_TO_TICKS( 5000 ) )
 
// 服务器监听端口
#define tcpechoPORT_NUMBER      ( 8080 )
 
uint16_t acceptTaskSize;
 
static void vAcceptConnectionTask(void *parameters)
{
	int32_t lBytes, lSent, lTotalSent;
	Socket_t xConnectedSocket;
	static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS( 1000 );
	static const TickType_t xSendTimeOut = pdMS_TO_TICKS( 1000 );
	TickType_t xTimeOnShutdown;
	uint8_t *pucRxBuffer = NULL;
 
	xConnectedSocket = ( Socket_t ) parameters;
	pucRxBuffer = ( uint8_t * ) pvPortMalloc( ipconfigTCP_MSS );
	
	if(pucRxBuffer != NULL)
	{
		FreeRTOS_setsockopt( xConnectedSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
		FreeRTOS_setsockopt( xConnectedSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xReceiveTimeOut ) );
		
		for( ;; )
		{
			/* Zero out the receive array so there is NULL at the end of the string
			when it is printed out. */
			memset( pucRxBuffer, 0x00, ipconfigTCP_MSS );
			
			/* Receive data on the socket. */
			lBytes = FreeRTOS_recv( xConnectedSocket, pucRxBuffer, ipconfigTCP_MSS, 0 );
			
			/* If data was received, echo it back. */
			if( lBytes > 0 )
			{
				lSent = 0;
				lTotalSent = 0;
				
				/* Call send() until all the data has been sent. */
				while( ( lSent >= 0 ) && ( lTotalSent < lBytes ) )
				{
					lSent = FreeRTOS_send( xConnectedSocket, pucRxBuffer, lBytes - lTotalSent, 0 );
					lTotalSent += lSent;
				}
				if( lSent < 0 )
				{
					/* Socket closed? */
					break;
				}
			}
			else if(lBytes < 0 )
			{
				/* Socket closed? */
				break;
			}
		}
	}
	/* Initiate a shutdown in case it has not already been initiated. */
	FreeRTOS_shutdown( xConnectedSocket, FREERTOS_SHUT_RDWR );
	
	/* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
	returning an error. */
	xTimeOnShutdown = xTaskGetTickCount();
	do
	{
		if( FreeRTOS_recv( xConnectedSocket, pucRxBuffer, ipconfigTCP_MSS, 0 ) < 0 )
		{
			break;
		}
	} while( ( xTaskGetTickCount() - xTimeOnShutdown ) < tcpechoSHUTDOWN_DELAY );
	
	/* Finished with the socket, buffer, the task. */
	vPortFree( pucRxBuffer );
	FreeRTOS_closesocket( xConnectedSocket );
	
	vTaskDelete( NULL );
}
 
static void vListeningConnectionTask(void *parameters)
{
	struct freertos_sockaddr xClient, xBindAddress;
	Socket_t xListeningSocket, xConnectedSocket;
	socklen_t xSize = sizeof( xClient );
	static const TickType_t xReceiveTimeOut = portMAX_DELAY;
	const BaseType_t xBacklog = 20;
	
	xListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
	configASSERT( xListeningSocket != FREERTOS_INVALID_SOCKET );
	
	FreeRTOS_setsockopt( xListeningSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
	
	xBindAddress.sin_port = tcpechoPORT_NUMBER;
	xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
	FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );
	FreeRTOS_listen( xListeningSocket, xBacklog );
	
	while(1)
	{
		xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );
		configASSERT( xConnectedSocket != FREERTOS_INVALID_SOCKET );
		
		xTaskCreate( vAcceptConnectionTask, "ClientInstance", acceptTaskSize, ( void * ) xConnectedSocket, tskIDLE_PRIORITY, NULL );
	}
}
 
void vStartTCPServerTask(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	xTaskCreate(vListeningConnectionTask, "TCPServerTask", usTaskStackSize, NULL, uxTaskPriority, NULL);
	acceptTaskSize = usTaskStackSize;
}