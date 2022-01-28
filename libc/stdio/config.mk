obj-  :=
obj-n :=
obj-y := \
	scanf.o \
	ff.o \
	ffeconv.o \
	fopen.o \
	fclose.o \
	fflush.o \
	fgets.o \
	fseek.o \
	fread.o \
	fwrite.o
	
#obj-$(CONFIG_LIBC_PRINTF_MIN) += \
obj-y += \
	printf_min.o

obj-$(CONFIG_LIBC_PRINTF_STD) += \
	printf_std.o

#obj-$(CONFIG_VFS) += \
obj-y += \
	ffeconv.o \
	fopen.o \
	fclose.o \
	fflush.o \
	fgets.o \
	fseek.o \
	fread.o \
	fwrite.o
