SHELL := /bin/bash

-include $(TOP_DIR)/.config

GCC_HOME = /opt/toolchain

export CPPFLAGS :=

# for gcc
export DEP_FLAGS = $(CPPFLAGS) -MM -MG
export CPPFLAGS +=  -I$(TOP_DIR)/board -I$(TOP_DIR)/include -I$(TOP_DIR)/cpu -I$(TOP_DIR)/drivers

ASPPFLAGS = -DASM  #for assembler pre-processor
ASFLAGS = $(ASPPFLAGS) $(CPPFLAGS) -DGCC4 -g

#export CFLAGS ?= -g -DCOMP_GCC  -msoft-float -O0
#export CFLAGS ?= -g -DCOMP_GCC -O0 -Os	-fdata-sections -ffunction-sections

export CPPFLAGS +=  -I$(TOP_DIR)/board -I$(TOP_DIR)/include -I$(TOP_DIR)/cpu -I$(TOP_DIR)/drivers -I$(TOP_DIR)/drivers/FTSPI020 -I$(TOP_DIR)/drivers/FTGMAC030 -I$(TOP_DIR)/freeRTOS/Source/include \
                    -I$(TOP_DIR)/freeRTOS/FreeRTOS-Plus-TCP/Source/include -I$(TOP_DIR)/freeRTOS/FreeRTOS-Plus-TCP/User/include -I$(TOP_DIR)/freeRTOS/CORTEX_A53_GC1601 \
					-I$(TOP_DIR)/freeRTOS/Source/portable/GCC/ARM_CA53_64_BIT -I${TOP_DIR}/freeRTOS/FreeRTOS-Plus-TCP/User

ifeq ($(CONFIG_ARMV5TE),y)
    TRGT = arm-none-eabi-
    export CC = $(TRGT)gcc
    export AS = $(TRGT)gcc
    export LD = $(TRGT)gcc
    export AR = $(TRGT)ar
    export RM = rm
    export FE = $(TRGT)objcopy
    export OD = $(TRGT)objdump

    export CFLAGS ?= -g -DCOMP_GCC  -msoft-float -O0
    export PATH   := $(TOP_DIR)/utils/:$(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0/bin:$(GCC_HOME)/arm-none-eabi-gcc-4.4.0/bin:$(PATH)
    export CPU_DIR = $(TOP_DIR)/cpu/armv7
    export CPU_START = start
    export CPPFLAGS += -I$(CPU_DIR)
    export TOOLCHAIN_FILE = $(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0.tar.bz2
	export TOOLCHAIN_FOLDER = $(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0

	CFLAGS += -march=armv5te

%.o:	%.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o:	%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

else ifeq ($(CONFIG_ARMV7A),y)

    TRGT = arm-none-eabi-
    export CC = $(TRGT)gcc
    export AS = $(TRGT)gcc
    export LD = $(TRGT)gcc
    export AR = $(TRGT)ar
    export RM = rm
    export FE = $(TRGT)objcopy
    export OD = $(TRGT)objdump

    export CFLAGS ?= -g -DCOMP_GCC  -msoft-float -O0
    export PATH := $(TOP_DIR)/utils/:$(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0/bin:$(GCC_HOME)/arm-none-eabi-gcc-4.4.0/bin:$(PATH)
    export CPU_DIR = $(TOP_DIR)/cpu/armv7
    export CPU_START = start
    export CPPFLAGS +=  -I$(CPU_DIR)
    export TOOLCHAIN_FILE = $(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0.tar.bz2
    export TOOLCHAIN_FOLDER = $(TOP_DIR)/utils/arm-none-eabi-gcc-4.4.0

	CFLAGS += -march=armv7-a

%.o:	%.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o:	%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

else ifeq ($(CONFIG_ARMV8),y)
    #TRGT = aarch64-none-elf-
    TRGT = aarch64-elf-
    export CC = $(TRGT)gcc
    export AS = $(TRGT)as
    export LD = $(TRGT)ld
    export AR = $(TRGT)ar
    export RM = rm
    export FE = $(TRGT)objcopy
    export OD = $(TRGT)objdump

    export CFLAGS ?= -g -DCOMP_GCC -O0
    export PATH   := $(TOP_DIR)/utils/:$(TOP_DIR)/utils/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-elf/bin:$(GCC_HOME)/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-elf/bin/:$(PATH)
    export CPU_DIR = $(TOP_DIR)/cpu/armv8

    export CPU_START = start_64
    export CPPFLAGS +=  -I$(CPU_DIR)
    export TOOLCHAIN_FILE = $(TOP_DIR)/utils/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-elf.tar.xz
    export TOOLCHAIN_FOLDER = $(TOP_DIR)/utils/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-elf

	CFLAGS = -Wall -O0 -g -DTEXT_BASE=$(CONFIG_TEXT_BASE) -Iinclude \
            -fno-builtin -fno-common -fomit-frame-pointer \
	        -march=armv8-a -mtune=cortex-a53	\
            -fno-strict-aliasing -ffreestanding -fno-stack-protector \
            -fdata-sections -ffunction-sections \
            -mlittle-endian
	export CPU_START = start_64
	export CPPFLAGS +=  -I$(CPU_DIR)

%.o:	%.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o:	%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

endif

#ycmo: if any bugs, please let me know
%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $(DEP_FLAGS)  $< | depFix.py $@ $< > $@' 

%.d: %.S
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $(DEP_FLAGS)  $< | depFix.py $@ $< > $@' 


lib-y := menu/menu.a \
	     board/boardtest/boardtest.a \
		 board/board.a \
		 cpu/cpu.a \
		 middleware/middleware.a \
		 middleware/cli/cli.a \

lib-$(CONFIG_FTAES020)			+= drivers/FTAES020/FTAES020.a
lib-$(CONFIG_FTDMAC030)			+= drivers/FTDMAC030/FTDMAC030.a
lib-$(CONFIG_FTGMAC030)    		+= drivers/FTGMAC030/FTGMAC030.a
lib-$(CONFIG_FTPWMTMR010)  		+= drivers/FTPWMTMR010/FTPWMTMR010.a
lib-$(CONFIG_FTGPIO010)			+= drivers/FTGPIO010/FTGPIO010.a
lib-$(CONFIG_FTIIC010)			+= drivers/FTIIC010/FTIIC010.a
lib-$(CONFIG_FTSPI020)			+= drivers/FTSPI020/FTSPI020.a
lib-$(CONFIG_FTSSP010)			+= drivers/FTSSP010/FTSSP010.a
lib-$(CONFIG_FTUART010)			+= drivers/FTUART010/FTUART010.a
lib-$(CONFIG_FTSDC021)			+= drivers/FTSDC021/FTSDC021.a
lib-$(CONFIG_FTLCD210)			+= drivers/FTLCDC210/FTLCDC210.a
lib-$(CONFIG_FTWDT011)			+= drivers/FTWDT011/FTWDT011.a
lib-$(CONFIG_HYPERBUS)			+= drivers/HYPERBUS/HYPERBUS.a
lib-$(CONFIG_SOTERIA)			+= drivers/SOTERIA/SOTERIA.a
lib-$(CONFIG_DDR_INTERLEAVE)	+= drivers/DDR_INTERLEAVE/DDR_INTERLEAVE.a
lib-$(CONFIG_CADENCE_PCIE_GEN3) += drivers/CADENCE_PCIE_GEN3/CADENCE_PCIE_GEN3.a
lib-$(CONFIG_NEON_CPAT)			+= drivers/NEON_CPAT/NEON_CPAT.a
lib-$(CONFIG_FREERTOS)			+= freeRTOS/freeRTOS.a

lib-$(CONFIG_DHRY)				+= benchmark/dhry/dhry.a
lib-$(CONFIG_COREMARK)			+= benchmark/coremark_v1.0/coremark_v1.0.a

lib-$(CONFIG_USE_FLIB_PRINTF)	+= libc/libc.a
lib-$(CONFIG_QC)				+= QC/QC.a

dir-y := board \
	     menu \
		 cpu \
		 board/boardtest \
		 middleware \
		 middleware/cli

dir-$(CONFIG_ARMV7A)			+= cpu/armv7
dir-$(CONFIG_ARMV5TE)			+= cpu/armv7
dir-$(CONFIG_ARMV8)				+= cpu/armv8

dir-$(CONFIG_FTDMAC030)			+= drivers/FTDMAC030
dir-$(CONFIG_FTGMAC030)     	+= drivers/FTGMAC030
dir-$(CONFIG_FTAES020)			+= drivers/FTAES020
dir-$(CONFIG_FTPWMTMR010)   	+= drivers/FTPWMTMR010
dir-$(CONFIG_FTGPIO010)			+= drivers/FTGPIO010
dir-$(CONFIG_FTIIC010)			+= drivers/FTIIC010
dir-$(CONFIG_FTSPI020)			+= drivers/FTSPI020
dir-$(CONFIG_FTSSP010)			+= drivers/FTSSP010
dir-$(CONFIG_FTUART010)			+= drivers/FTUART010
dir-$(CONFIG_FTSDC021)			+= drivers/FTSDC021
dir-$(CONFIG_FTLCD210)			+= drivers/FTLCDC210
dir-$(CONFIG_FTWDT011)			+= drivers/FTWDT011
dir-$(CONFIG_HYPERBUS)			+= drivers/HYPERBUS
dir-$(CONFIG_SOTERIA)			+= drivers/SOTERIA
dir-$(CONFIG_DDR_INTERLEAVE)	+= drivers/DDR_INTERLEAVE
dir-$(CONFIG_NEON_CPAT)       	+= drivers/NEON_CPAT
dir-$(CONFIG_FREERTOS)			+= freeRTOS
dir-$(CONFIG_DHRY)				+= benchmark/dhry
dir-$(CONFIG_COREMARK)			+= benchmark/coremark_v1.0

dir-$(CONFIG_USE_FLIB_PRINTF)   += libc
dir-$(CONFIG_QC)				+= QC

