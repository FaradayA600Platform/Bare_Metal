export TOP_DIR			= $(shell pwd)
export CCACHE_DIR         := $(TOP_DIR)/.ccache

include config.mk

TAR_OPT :=

ifeq ($(CONFIG_ARMV8),y)
	TAR_OPT = xJf
else ifeq ($(CONFIG_ARMV7A),y)
	TAR_OPT = xjf
else ifeq ($(CONFIG_ARMV5TE),y)
	TAR_OPT = xjf
endif

.PHONY : menuconfig %config  lib sofia_config

.PHONY:all lib init toolchain
all: init sofia_config lib Ram


init:
#	[ -f autoconf.h ] || ( echo "please config first!"; exit 1)
#	arm-none-eabi-gcc -v >& /dev/null || make toolchain
	$(CC) -v >& /dev/null || make toolchain
	make -C utils

#	if [ "$(CONFIG_ARMV8)" == "y" ]; then \
#		TOOLCHAIN_FILE = utils/gcc-linaro-aarch64-none-elf-4.9-2014.09_linux.tar.xz; \
#		echo $(TOOLCHAIN_FILE); \
#	else \
#TOOLCHAIN_FILE = utils/arm-none-eabi-gcc-4.4.0.tar.bz2;
#		echo $(TOOLCHAIN_FILE); \
#	fi
#TOOLCHAIN_FILE = utils/arm-none-eabi-gcc-4.4.0.tar.bz2;

toolchain:
	if [ ! -d $(TOOLCHAIN_FOLDER) ]; then \
	if [ "$(CONFIG_SUPER_USER)" == "y" ]; then \
		echo sudo tar $(TAR_OPT) $(TOOLCHAIN_FILE) -C /opt; \
		sudo tar $(TAR_OPT) $(TOOLCHAIN_FILE) -C /opt; \
	else \
		echo tar $(TAR_OPT) $(TOOLCHAIN_FILE) -C utils; \
		tar $(TAR_OPT) $(TOOLCHAIN_FILE) -C utils; \
	fi \
	fi

lib:
	set -e; for i in $(dir-y); do \
		make -C $$i all; \
	done

	if [ "$(CONFIG_ARMV8)" == "y" ]; then \
		ln -sf $(CPU_DIR)/armv8.a $(TOP_DIR)/cpu/cpu.a;\
	else \
		ln -sf $(CPU_DIR)/armv7.a $(TOP_DIR)/cpu/cpu.a;\
	fi
	
test:
	make a380_ca9_defconfig
	make  ||  ( echo "a380_ca9_defconfig error!"; exit 1)
	make a380_fa626_defconfig
	make  ||  ( echo "a380_fa626_defconfig error!"; exit 1)
	make a369_hiVec_defconfig
	make  ||  ( echo "a369_hiVec_defconfig!"; exit 1)
	make fh36_defconfig
	make  ||  ( echo "fh36_defconfig!"; exit 1)
	make lte_defconfig
	make  ||  ( echo "lte_defconfig!"; exit 1)
	make a369_zynq_defconfig
	make  ||  ( echo "a369_zynq_defconfig!"; exit 1)
	make a369_defconfig
	make  ||  ( echo "a369_defconfig error!"; exit 1)
	

#
# Config stuff
#

SCRIPTSDIR		= $(TOP_DIR)/utils/kconfig/kconfig
include utils/kconfig/Makefile.conf

menuconfig: mconf
	$(SCRIPTSDIR)/mconf Kconfig
	ln -sf ../autoconf.h include/autoconf.h

#%defconfig: distclean conf
%defconfig: distclean conf
	$(SCRIPTSDIR)/conf -D configs/$@ Kconfig
	ln -sf ../autoconf.h include/autoconf.h

sofia_config: conf autoconf.h
	ln -sf ../autoconf.h include/autoconf.h

autoconf.h: .config
	$(SCRIPTSDIR)/conf -D .config Kconfig > /dev/null || ( echo "please config first!"; exit 1)
#
# End of kconfig stuff
#



.PHONY : Ram

-include board/$(CPU_START).d
#LDFLAGS = -LBIN -fpie -v
LDFLAGS = -LBIN -v --verbose
#LDFLAGS += --gc-sections
#Ram:  $(lib-y) board/start.o 
Ram:  	$(lib-y) board/$(CPU_START).o 
	if [ "$(CONFIG_ARMV8)" == "y" ]; then \
	$(LD) $(LDFLAGS) -T platform_op.ld -Ttext $(CONFIG_TEXT_BASE) --start-group board/start_64.o $(lib-y) --end-group -o output/NonOS_QC.elf;\
	else \
	$(LD) $(LDFLAGS)  -Wl,-T platform.ld -Ttext $(CONFIG_TEXT_BASE) -Wl,--start-group board/start.o $(lib-y) -Wl,--end-group -lm -o output/NonOS_QC.elf;\
	fi
	$(FE) -O binary output/NonOS_QC.elf output/NonOS_QC.bin
	$(OD) -x  output/NonOS_QC.elf > output/NonOS_QC.map
	$(OD) -dS  output/NonOS_QC.elf > output/NonOS_QC.S
	[ ! -d /tftproot ] || cp -f output/NonOS_QC.bin /tftproot/
	[ ! -d /tftproot ] || cp -f output/NonOS_QC.elf /tftproot/
#	$(OD) -D  output/NonOS_QC.elf > output/NonOS_QC.S



.PHONY:clean
clean:
#	set -e; for i in $(dir-y) $(dir-n) $(dir-) ; do 
	set -e; for i in $(dir-y) ; do \
		make -C $$i clean; \
	done


.PHONY : distclean dist
distclean:
	find . -name "*.[oad]" | sed -e '/utils\/arm-none-eabi-gcc-4.4.0/d' | xargs rm -f
	rm -f .config .config.old auto.conf include/autoconf.h autoconf.h
	make -C utils/kconfig/kconfig/ clean
	make -C utils clean
	rm -f output/NonOS_QC.*


dist: distclean 
	[ ! -d utils/arm-none-eabi-gcc-4.4.0 ] || rm -rf utils/arm-none-eabi-gcc-4.4.0
	make -C utils getver
	tar czvf ../NonOS_`Utils/getver -s .`.tar.gz .





