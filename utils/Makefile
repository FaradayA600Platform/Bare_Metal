CC		= gcc
CFLAGS	= 
LDFLAGS	= 

TOP_DIR			?= $(shell pwd)/..
-include $(TOP_DIR)/.config
TRGT ?= arm-none-eabi-

all: getver ccache


getver: getver.o

.PHONY: ccache
ccache:
	rm -f $(TRGT)gcc
ifeq  ($(CONFIG_CCACHE),y)
	@if [ ! -f ccache-3.1.4/ccache ]; then \
		tar jxvf ccache-3.1.4.tar.bz2; \
		cd ccache-3.1.4 \
			&& ./configure \
			&& make; \
	 fi
	-ln -sf ccache-3.1.4/ccache $(TRGT)gcc
endif

clean:
	rm -rf *.bak *.o getver
