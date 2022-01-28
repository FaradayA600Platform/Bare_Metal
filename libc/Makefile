#
# Makefile for the drivers
#
include $(TOP_DIR)/config.mk
#include stdio/config.mk


obj-y	:=  \
	    $(subst .c,.o,$(wildcard *.c)) \
	    $(subst .S,.o,$(wildcard *.S)) \
	    $(subst .c,.o,$(wildcard stdlib/*.c)) \
	    $(subst .S,.o,$(wildcard stdlib/*.S)) \
	    $(subst .c,.o,$(wildcard string/*.c)) \
	    $(subst .S,.o,$(wildcard string/*.S))
#	    $(subst .c,.o,$(wildcard stdio/*.c)) \
#	    $(subst .S,.o,$(wildcard stdio/*.S)) 		
#obj-y	:= utility.o display.o cmdui.o fat.o filectrl.o cpat/FLibARM.o fLib.o



include $(TOP_DIR)/common.mk
