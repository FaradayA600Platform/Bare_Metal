include $(TOP_DIR)/config.mk

include $(TOP_DIR)/benchmark/config.mk

#export CFLAGS +=  -g -O0 -mcpu=$(CPU) -I$(TOP_DIR)/Benchmark/coremark_v1.0 -DFLAGS_STR=\""-O3 -mcpu=$(CPU)   "\" -DITERATIONS=0 -msoft-float
#export CFLAGS :=  -O3 -mcpu=$(CPU) -I$(TOP_DIR)/Benchmark/coremark_v1.0 -DFLAGS_STR=\""-O3 -mcpu=$(CPU)   "\" -DITERATIONS=0 -msoft-float
#export CFLAGS :=  -O3 -mcpu=$(CPU) -I$(TOP_DIR)/Benchmark/coremark_v1.0 -DFLAGS_STR=\""-O3 -mcpu=$(CPU)   "\" -DITERATIONS=0 -march=armv8-a+fp 
export CFLAGS :=  -O3 -mcpu=$(CPU) -I$(TOP_DIR)/Benchmark/coremark_v1.0 -DFLAGS_STR=\""-O3 -mcpu=$(CPU)   "\" -DITERATIONS=0 
obj-y	:=	core_list_join.o core_main.o core_matrix.o core_state.o core_util.o core_portme.o

include $(TOP_DIR)/common.mk
