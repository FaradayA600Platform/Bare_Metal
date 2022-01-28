include $(TOP_DIR)/config.mk

include $(TOP_DIR)/benchmark/config.mk

export CFLAGS := -W -O2 -g -mcpu=$(CPU) -fsection-anchors -fno-common -fno-tree-ch -fsched-stalled-insns=0 -fgcse-sm --param max-aliased-vops=32 -fargument-noalias-global -fno-if-conversion2 -ftracer -falign-functions=8 -fsched2-use-traces -fpromote-loop-indices  -msoft-float 

obj-y	:=	dhry_1.o dhry_2.o

	
include $(TOP_DIR)/common.mk
