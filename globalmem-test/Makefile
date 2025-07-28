# Makefile for globalmem driver

KERNEL_DIR := /home/book/100ask_imx6ull-qemu/linux-4.9.88

ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-


obj-m += globalmem.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
