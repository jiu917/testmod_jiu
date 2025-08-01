# 指定内核源码路径（根据你的实际路径调整）
KERNEL_DIR ?= /home/book/100ask_imx6ull-qemu/linux-4.9.88

# 指定编译的模块名称（通常与源文件名一致）
obj-m := ledstest.o

# 默认构建目标
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# 清理生成的文件
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
