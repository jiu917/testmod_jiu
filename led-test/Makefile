KERNEL_DIR ?= /home/book/100ask_imx6ull-qemu/linux-4.9.88

obj-m := led.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
