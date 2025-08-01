# 指定内核源码路径（根据您的实际路径调整）
KDIR := /home/book/100ask_imx6ull-qemu/linux-4.9.88

# 当前模块路径
PWD := $(shell pwd)

# 模块名称（与源文件同名）
obj-m := ledkey.o

# 默认编译目标
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 清理编译生成的文件
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.order *.symvers

