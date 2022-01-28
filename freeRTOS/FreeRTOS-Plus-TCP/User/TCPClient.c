#include <stdlib.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
 
/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
 
#include "TCPClient.h" 

#define config_SERVER_ADDR0     ( 192 )
#define config_SERVER_ADDR1     ( 168 )
#define config_SERVER_ADDR2     ( 1 )
#define config_SERVER_ADDR3     ( 100 )

// Server Port
#define SERVER_PORT             ( 8080 )

// Buffer Size
#define BUFFER_SIZES            ( ipconfigTCP_MSS * 2 )

// Transmit and Receive Timeout
static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS( 5000 ); // Default 5000ms
static const TickType_t xSendTimeOut    = pdMS_TO_TICKS( 5000 ); // Default 5000ms

// Transmit and Receive Buffer
static char cTxBuffers[ BUFFER_SIZES ], cRxBuffers[ BUFFER_SIZES ];

// 连接服务器的Socket
static Socket_t xSocket;

uint8_t tcp_send = 1;
uint8_t isConnet = 0;

uint8_t getisConnet(void)
{
	return isConnet;
}

static void prvClientSendTask(void *pvParameters)
{
	struct freertos_sockaddr xServerAdd;// 服务器地址
	uint8_t retrycount = 0;  // 重连次数
	BaseType_t sendbytelen;  // 发送字节数 
	char * pcSendString = cTxBuffers; // 发送缓存地址
	char * pcRecvString = cRxBuffers; // 接收缓存地址
	uint32_t uTxCount = 0; // 发送次数自增
	
	// 填充 服务器地址和端口
	xServerAdd.sin_port = FreeRTOS_htons(SERVER_PORT);
	xServerAdd.sin_addr = FreeRTOS_inet_addr_quick(config_SERVER_ADDR0, config_SERVER_ADDR1, config_SERVER_ADDR2, config_SERVER_ADDR3);
	
	while(1)
	{
		xSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
		configASSERT(xSocket != FREERTOS_INVALID_SOCKET);
		
		// 设置接收、发送超时
		FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof(xReceiveTimeOut));
		FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof(xSendTimeOut));
		
		// if ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h
		// 不需要bind
		// 连接成功 返回0
		if(FreeRTOS_connect(xSocket, &xServerAdd, sizeof(xServerAdd)) == 0)
		{
			isConnet = 1;
			retrycount = 0;
			while(1)
			{
				if(tcp_send)
				{
					memset(pcSendString, 0, sizeof(cTxBuffers));
					sprintf(pcSendString, "tcpclient message number %u \r\n", uTxCount++);
					
					sendbytelen = FreeRTOS_send(xSocket, pcSendString, strlen(pcSendString), 0);
					
					// 发送错误
					if(sendbytelen < 0)
					{
						switch(sendbytelen)
						{
							case -pdFREERTOS_ERRNO_ENOTCONN:
								// 套接字已关闭或已关闭而无法发送数据
								break;
							case -pdFREERTOS_ERRNO_ENOMEM:
								// 内存不足而无法发送数据
								break;
							case -pdFREERTOS_ERRNO_EINVAL:
								// xSocket不是有效的TCP套接字而无法发送数据
								break;
							case -pdFREERTOS_ERRNO_ENOSPC:
								// 在任何数据被发送之前发生超时
								break;
						}
						break;
					}
					
					if(sendbytelen == 0)
					{
						break;
					}
				}
				vTaskDelay(20 / portTICK_PERIOD_MS);
			}
			
			// 当重复发送失败
			if(FreeRTOS_issocketconnected(xSocket))
			{
				// 优雅的关闭断开连接: FIN, FIN+ACK, ACK.
				FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
				// 期望FreeRTOS_recv()立刻返回error，表示shutdown完成
				BaseType_t xReturn;
				TickType_t xTimeOnEnting = xTaskGetTickCount();// 记录进入接收的时间
				
				// 接收到的数据长度小于0或者超时，则shutdown完成跳出循环
				do{
					xReturn = FreeRTOS_recv(xSocket, pcRecvString, BUFFER_SIZES, 0);
				}
				while((xReturn >= 0) && ((xTaskGetTickCount() - xTimeOnEnting) < xReceiveTimeOut));
			}
		}
		
		retrycount++; // 重连次数自增
		
		FreeRTOS_closesocket(xSocket); // 关闭套接字
		
		vTaskDelay(500 / portTICK_PERIOD_MS); // 延时500ms重连
	}
}

void shoutDownTCPClient()
{
	char * pcRecvString = cRxBuffers; // 接收缓存地址
	
	if(FreeRTOS_issocketconnected(xSocket))
	{
		// 优雅的关闭断开连接: FIN, FIN+ACK, ACK.
		FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
		
		// 期望FreeRTOS_recv()立刻返回error，表示shutdown完成
		BaseType_t xReturn;
		TickType_t xTimeOnEnting = xTaskGetTickCount();// 记录进入接收的时间
		
		// 接收到的数据长度小于0或者超时，则shutdown完成跳出循环
		do{
			xReturn = FreeRTOS_recv(xSocket, pcRecvString, BUFFER_SIZES, 0);
		}
		while((xReturn >= 0) && ((xTaskGetTickCount() - xTimeOnEnting) < xReceiveTimeOut));
	}
}

void stopTCPClient(void)
{
	tcp_send = 0;
}

void startTCPClient(void)
{
	tcp_send = 1;
}

static void prvClientRecvTask(void *pvParameters)
{
	char * pcReceivedString = cRxBuffers;        // 接收内存指针
	BaseType_t xReturned; 						 // 接收到的数据长度
	
	while(1)
	{
		if(FreeRTOS_issocketconnected(xSocket))
		{
			xReturned = FreeRTOS_recv(	xSocket,						/* The socket being received from. */
										( void * )pcReceivedString,		/* The buffer into which the received data will be written. */
										BUFFER_SIZES,					/* The size of the buffer provided to receive the data. */
										0 );
 
			// 当收到数据大于0时，返回接收到的数据
			if(xReturned > 0)
			{
				if(memcmp(pcReceivedString, "stop", 4) == 0)
				{
					tcp_send = 0;
				} 
				else if(memcmp(pcReceivedString, "start", 5) == 0)
				{
					tcp_send = 1;
				}
				
				FreeRTOS_send(	xSocket,							/* The socket being sent to. */
								( void * ) pcReceivedString,		/* The data being sent. */
								xReturned,							/* The length of the data being sent. */
								0 );
			}
			else if(xReturned < 0)
			{
				/* Error? */
				switch(xReturned)
				{
					case -pdFREERTOS_ERRNO_ENOTCONN:
						// 套接字已关闭或已关闭
						break;
					case -pdFREERTOS_ERRNO_ENOMEM:
						// 没有足够的内存使套接字能够创建Rx或Tx流
						break;
					case -pdFREERTOS_ERRNO_EINTR:
						// 套接字收到信号，导致读取操作中止
						break;
					case -pdFREERTOS_ERRNO_EINVAL:
						// 套接字无效，不是TCP套接字，或者未绑定
						break;
				}
				break;
			}
		}
		else
		{
			vTaskDelay( 1000 / portTICK_PERIOD_MS );
		}
	}
	
	vTaskDelete(NULL);
}

// TCPClient 客户端任务创建
void vStartTCPClientTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	portBASE_TYPE rc;
	
	xTaskCreate(	prvClientSendTask,			/* The function that implements the task. */
					"ClientTranslate",			/* Just a text name for the task to aid debugging. */
					usTaskStackSize,			/* The stack size is defined in FreeRTOSIPConfig.h. */
					NULL,						/* The task parameter, not used in this case. */
					uxTaskPriority,				/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
					NULL );						/* The task handle is not used. */
	if (rc != pdPASS)
		FreeRTOS_printf( ("failed to create prvClientSendTask!\n") );
	
	xTaskCreate(	prvClientRecvTask,			/* The function that implements the task. */
					"ClientRecv",				/* Just a text name for the task to aid debugging. */
					usTaskStackSize,			/* The stack size is defined in FreeRTOSIPConfig.h. */
					NULL,						/* The task parameter, not used in this case. */
					uxTaskPriority,				/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
					NULL );						/* The task handle is not used. */
	if (rc != pdPASS)
		FreeRTOS_printf( ("failed to create prvClientRecvTask!\n") );
}