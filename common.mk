

LIBNAME = $(shell pwd | sed -e 's/.*\///').a

.PHONY: all
all: $(LIBNAME)

$(LIBNAME): $(obj-y) Makefile
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(obj-y)

.PHONY:clean
clean:
	find . -name "*.o" | xargs rm -f
	find . -name "*.a" | xargs rm -f

distclean: clean
	find . -name "*.d" | xargs rm -f
	find . -name "Makefile.depend" | xargs rm -f


-include $(obj-y:.o=.d)
