/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
 
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h" 
#include "queue.h"
#include "list.h" 
#include "semphr.h" 
#include "string.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "SimpleUDPEchoServer.h"
#include "SimpleTCPEchoServer.h"
#include "TCPServer.h"
#include "TCPClient.h"
#include "DrvUART010.h"
#include "NetworkConfig.h"

/* Simple UDP echo task parameters. */
#define mainSIMPLE_UDP_ECHO_SERVER_TASK_STACK_SIZE 		( configMINIMAL_STACK_SIZE * 2 )
#define mainSIMPLE_UDP_ECHO_SERVER_TASK_PRIORITY		( tskIDLE_PRIORITY )
#define mainSIMPLE_UDP_ECHO_SERVER_TASK_PORT			( 5005UL )

/* Echo client task parameters - used for both TCP and UDP echo clients. */
#define mainECHO_CLIENT_TASK_STACK_SIZE 				( configMINIMAL_STACK_SIZE * 2 )	/* Not used in the Windows port. */
#define mainECHO_CLIENT_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )

/* Echo server task parameters. */
#define mainECHO_SERVER_TASK_STACK_SIZE					( configMINIMAL_STACK_SIZE * 2 )	/* Not used in the Windows port. */
#define mainECHO_SERVER_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )

/* Simple TCP server task parameters. */
#define mainTCP_SERVER_TASK_STACK_SIZE 					( configMINIMAL_STACK_SIZE * 2 )
#define mainTCP_SERVER_TASK_PRIORITY					( tskIDLE_PRIORITY )

/* Simple TCP client task parameters. */
#define mainTCP_CLIENT_TASK_STACK_SIZE 					( configMINIMAL_STACK_SIZE * 2 )
#define mainTCP_CLIENT_TASK_PRIORITY					( tskIDLE_PRIORITY )

#define mainHOST_NAME               "RTOSDemo"
#define mainDEVICE_NICK_NAME        "windows_demo"

/* Set the following constants to 1 or 0 to define which tasks to include and
exclude:

mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS:  When set to 1 two UDP client tasks
and two UDP server tasks are created.  The clients talk to the servers.  One set
of tasks use the standard sockets interface, and the other the zero copy sockets
interface.  These tasks are self checking and will trigger a configASSERT() if
they detect a difference in the data that is received from that which was sent.
As these tasks use UDP, and can therefore loose packets, they will cause
configASSERT() to be called when they are run in a less than perfect networking
environment.

mainCREATE_TCP_ECHO_TASKS_SINGLE:  When set to 1 a set of tasks are created that
send TCP echo requests to the standard echo port (port 7), then wait for and
verify the echo reply, from within the same task (Tx and Rx are performed in the
same RTOS task).  The IP address of the echo server must be configured using the
configECHO_SERVER_ADDR0 to configECHO_SERVER_ADDR3 constants in
FreeRTOSConfig.h.

mainCREATE_TCP_ECHO_SERVER_TASK:  When set to 1 a task is created that accepts
connections on the standard echo port (port 7), then echos back any data
received on that connection.
*/
#define mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS	1
#define mainCREATE_TCP_ECHO_SERVER_TASK				1
#define mainCREATE_TCP_SERVER_TASK					1
#define mainCREATE_TCP_CLIENT_TASK					1
/*-----------------------------------------------------------*/

const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 }; 
const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 }; 
const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 }; 
const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 }; 
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };

QueueHandle_t xPingReplyQueue;

UBaseType_t uxRand()
{
	return (UBaseType_t) rand();
}

const char *pcApplicationHostnameHook( void )
{
	return mainHOST_NAME;
}

BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
BaseType_t xReturn; 
	
	if ( strcmp( pcName, pcApplicationHostnameHook() ) == 0 )
	{
		xReturn = pdPASS;
	}
	else if ( strcmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
	{
		xReturn = pdPASS;
	}
	else
	{
		xReturn = pdFAIL;
	} 
	
	return xReturn; 
}

void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
	switch ( eStatus ) 
	{ 
	case eSuccess: 
		xQueueSend( xPingReplyQueue, &usIdentifier, 10 / portTICK_PERIOD_MS ); 
		break; 
	
	case eInvalidChecksum:
		break;
	
	case eInvalidData:
		break; 
	}
}

void xPingReplyQueueCreate(void)
{
	xPingReplyQueue = xQueueCreate( 20, sizeof( uint16_t ) );
}

BaseType_t vSendPing( const char *pcIPAddress )
{
uint16_t usRequestSequenceNumber, usReplySequenceNumber;
uint32_t ulIPAddress;
	
	ulIPAddress = FreeRTOS_inet_addr( pcIPAddress );
	
	if ( xPingReplyQueue == NULL )
		xPingReplyQueueCreate();
	
	usRequestSequenceNumber = FreeRTOS_SendPingRequest( ulIPAddress, 8, 100 / portTICK_PERIOD_MS );
	if ( usRequestSequenceNumber == pdFAIL )
	{
		
	}
	else
	{
		if( xQueueReceive( xPingReplyQueue, &usReplySequenceNumber, 200 / portTICK_PERIOD_MS ) == pdPASS )
		{
			if( usRequestSequenceNumber == usReplySequenceNumber )
			{
				
			}
		}
	}
	
	return ulIPAddress;
}

BaseType_t IP_init( void )
{
	return FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress ); 
}

int lUDPLoggingPrintf( const char *fmt, ... )
{
	fLib_printf(fmt);
	
	return 0;
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
char cBuffer[ 16 ];
static BaseType_t xTasksAlreadyCreated = pdFALSE;
	
	FreeRTOS_printf( ( "vApplicationIPNetworkEventHook: event %ld\n", eNetworkEvent ) );
	
	if( eNetworkEvent == eNetworkUp )
	{
		if( xTasksAlreadyCreated == pdFALSE )
		{
			#if( mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS == 1 )
			{
				vStartSimpleUDPEchoServerTasks( mainSIMPLE_UDP_ECHO_SERVER_TASK_STACK_SIZE, mainSIMPLE_UDP_ECHO_SERVER_TASK_PORT, mainSIMPLE_UDP_ECHO_SERVER_TASK_PRIORITY );
			}
			#endif /* mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS */
			
			#if( mainCREATE_TCP_ECHO_SERVER_TASK == 1 )
			{
				vStartSimpleTCPServerTasks( mainECHO_SERVER_TASK_STACK_SIZE, mainECHO_SERVER_TASK_PRIORITY );
			}
			#endif
			
			#if( mainCREATE_TCP_SERVER_TASK == 1 )
			{
				vStartTCPServerTask( mainTCP_SERVER_TASK_STACK_SIZE, mainTCP_SERVER_TASK_PRIORITY );
			}
			#endif
			
			#if( mainCREATE_TCP_CLIENT_TASK == 1 )
			{
				vStartTCPClientTasks( mainTCP_CLIENT_TASK_STACK_SIZE, mainTCP_CLIENT_TASK_PRIORITY );
			}
			#endif
			
			xTasksAlreadyCreated = pdTRUE;
		}
		
		FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
		
		FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
		
		FreeRTOS_printf( ( "IP Address: %s\n", cBuffer ) );
		
		FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
		
		FreeRTOS_printf( ( "Subnet Mask: %s\n", cBuffer ) );
		
		FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
		
		FreeRTOS_printf( ( "Gateway Address: %s\n", cBuffer ) );
		
		FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
		
		FreeRTOS_printf( ( "DNS Server Address: %s\n", cBuffer ) );
	}
}

extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
	( void ) ulSourceAddress;
	( void ) usSourcePort;
	( void ) ulDestinationAddress;
	( void ) usDestinationPort;
	
	return uxRand();
}
 
BaseType_t xApplicationGetRandomNumber( uint32_t* pulNumber )
{
	*(pulNumber) = uxRand();
	
	return pdTRUE;
}

void vLoggingPrintf( const char *pcFormat, ... )
{
	fLib_printf(pcFormat);
}