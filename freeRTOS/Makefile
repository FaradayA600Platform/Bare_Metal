include $(TOP_DIR)/config.mk
FREERTOS_USER_PATH = FreeRTOS-Plus-TCP/User
FREERTOS_SOURCE_PATH = FreeRTOS-Plus-TCP/Source
CPPFLAGS += -I$(TOP_DIR)/Source/portable/GCC/ARM_CA53_64_BIT -I${TOP_DIR}/freeRTOS/Demo/Common/include -I${TOP_DIR}/freeRTOS/User/include -I${TOP_DIR}/drivers/FTGMAC030
CFLAGS += -I$(TOP_DIR)/Source/portable/GCC/ARM_CA53_64_BIT -I${TOP_DIR}/freeRTOS/Demo/Common/Include -I${TOP_DIR}/freeRTOS/User/include -I${TOP_DIR}/drivers/FTGMAC030

obj-y := \
	Source/tasks.o \
	Source/queue.o \
	Source/list.o \
	Source/event_groups.o \
	Source/stream_buffer.o \
	Source/croutine.o \
	Source/timers.o

obj-y += \
	Source/portable/GCC/ARM_CA53_64_BIT/port.o \
	Source/portable/GCC/ARM_CA53_64_BIT/portASM.o

obj-y += \
	CORTEX_A53_GC1601/FreeRTOS_tick_config.o \
	CORTEX_A53_GC1601/rtos_demo.o

#obj-y += \
	CORTEX_A53_GC1601/FreeRTOS_asm_vectors.o

obj-y += \
	Source/portable/MemMang/heap_1.o \
	Source/portable/MemMang/heap_2.o \
	Source/portable/MemMang/heap_3.o \
	Source/portable/MemMang/heap_4.o

obj-y += \
	Demo/Common/Minimal/TimerDemo.o

obj-y += \
	$(FREERTOS_SOURCE_PATH)/BufferAllocation_1.o \
	$(FREERTOS_SOURCE_PATH)/BufferAllocation_2.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_ARP.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_DHCP.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_DNS.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_IP.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_Sockets.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_Stream_Buffer.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_TCP_IP.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_TCP_WIN.o \
	$(FREERTOS_SOURCE_PATH)/FreeRTOS_UDP_IP.o

obj-y += \
	$(FREERTOS_USER_PATH)/ftgmac030_hal_eth.o \
	$(FREERTOS_USER_PATH)/NetworkConfig.o \
	$(FREERTOS_USER_PATH)/phyHandling.o \
	$(FREERTOS_USER_PATH)/SimpleUDPEchoServer.o \
	$(FREERTOS_USER_PATH)/TCPServer.o \
	$(FREERTOS_USER_PATH)/NetworkInterface.o \
	$(FREERTOS_USER_PATH)/SimpleTCPEchoServer.o \
	$(FREERTOS_USER_PATH)/TCPClient.o

include $(TOP_DIR)/common.mk
