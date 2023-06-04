obj-m += hello_module.o

KERNEL=${HOME}/Documents/beaglebone/kernelbuildscripts/KERNEL
GCC=${HOME}/Documents/beaglebone/kernelbuildscripts/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-

all:
	make -C ${KERNEL} ARCH=arm CROSS_COMPILE=${GCC} M=$(PWD) modules

clean:
	make -C ${KERNEL} M=$(PWD) clean