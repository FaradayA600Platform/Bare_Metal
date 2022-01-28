TOP_DIR			?= $(shell pwd)/../..
include $(TOP_DIR)/config.mk

dir := $(wildcard *.c ./core/*.c ./test/*.c)
obj-y	:= $(patsubst %.c,%.o,$(dir))

include $(TOP_DIR)/common.mk