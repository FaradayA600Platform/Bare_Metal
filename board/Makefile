#
# Makefile for the board.
#

TOP_DIR			?= $(shell pwd)/..
ifneq ($(CONFIG_ARMV8),y)
export CFLAGS := -g -DCOMP_GCC  -msoft-float -O0
else
export CFLAGS := -g -DCOMP_GCC -O0
endif
include $(TOP_DIR)/config.mk


obj-y	:=	interrupt.o board.o serial.o

ifneq ($(CONFIG_ARMV8),y)
obj-y	+=	trap.o
else
obj-y	+=	exceptions.o
endif

ifneq ($(CONFIG_USE_FLIB_PRINTF),y)
obj-y	+= swi.o 
endif

obj-y	+= xlate.o

obj-$(CONFIG_JIFFIES)	+= jiffies.o

obj-$(CONFIG_FTINTC010)  += ftintc.o 
obj-$(CONFIG_FTINTC020)  += ftintc.o 
obj-$(CONFIG_FTINTC030)  += ftintc030.o 
obj-$(CONFIG_GIC)  += gic.o 
obj-$(CONFIG_GIC400)  += gic400.o 
obj-$(CONFIG_EXTERNAL_GIC)  += gic.o 

obj-$(CONFIG_PLATFORM_A380) += a380/spec.o a380/platform.o
obj-$(CONFIG_PLATFORM_A500) += a500/spec.o a500/platform.o
obj-$(CONFIG_PLATFORM_A600) += a600/spec.o a600/platform.o
obj-$(CONFIG_FTTIMER)  += timer.o 

include $(TOP_DIR)/common.mk

