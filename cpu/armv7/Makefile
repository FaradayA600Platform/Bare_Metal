#
# Makefile for the CPU
#
TOP_DIR			?= $(shell pwd)/..
include $(TOP_DIR)/config.mk

obj-y	:=	cpu.o mmugen.o cp15.o cp15s.o
obj-$(CONFIG_ARMV5TE) += v5te/cp15_v5.o
obj-$(CONFIG_ARMV7A) +=  v7a/cp15_v7.o v7a/cache_v7.o v7a/cpu_v7.o

include $(TOP_DIR)/common.mk
