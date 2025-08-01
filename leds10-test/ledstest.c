#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define LED1_GPIO 131  // GPIO5_IO03
#define LED2_GPIO 3    // GPIO1_IO03
#define LED3_GPIO 5    // GPIO1_IO05
#define LED4_GPIO 6    // GPIO1_IO06

#define DEVICE_NAME "led_control"
#define CLASS_NAME  "led_class"

static dev_t dev_num;              // 设备号
static struct cdev led_cdev;       // 字符设备结构
static struct class *led_class;    // 设备类
static struct device *led_device;  // 设备

// 打开设备
static int led_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "LED device opened\n");
    return 0;
}

// 关闭设备
static int led_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "LED device closed\n");
    return 0;
}

// 写入设备 - 处理用户空间的控制命令
static ssize_t led_write(struct file *file, const char __user *buf, 
                         size_t count, loff_t *ppos) {
    char *cmd;
    int led_num, value;
    
    // 分配内存存储用户空间的数据
    cmd = kmalloc(count + 1, GFP_KERNEL);
    if (!cmd) {
        return -ENOMEM;
    }
    
    // 将数据从用户空间复制到内核空间
    if (copy_from_user(cmd, buf, count)) {
        kfree(cmd);
        return -EFAULT;
    }
    cmd[count] = '\0';  // 添加字符串结束符
    
    // 解析命令格式: "ledX Y" 其中X是1-4, Y是0或1
    if (sscanf(cmd, "led%d %d", &led_num, &value) != 2) {
        printk(KERN_ERR "Invalid command! Use: echo 'ledX Y' > /dev/led_control (X=1-4, Y=0-1)\n");
        kfree(cmd);
        return count;
    }
    
    // 验证LED编号和值
    if (led_num < 1 || led_num > 4) {
        printk(KERN_ERR "Invalid LED number! Use 1-4\n");
        kfree(cmd);
        return count;
    }
    
    if (value != 0 && value != 1) {
        printk(KERN_ERR "Invalid value! Use 0 or 1\n");
        kfree(cmd);
        return count;
    }
    
    // 根据命令控制相应的LED
    switch (led_num) {
        case 1:
            gpio_set_value(LED1_GPIO, value);
            printk(KERN_INFO "LED1 set to %d\n", value);
            break;
        case 2:
            gpio_set_value(LED2_GPIO, value);
            printk(KERN_INFO "LED2 set to %d\n", value);
            break;
        case 3:
            gpio_set_value(LED3_GPIO, value);
            printk(KERN_INFO "LED3 set to %d\n", value);
            break;
        case 4:
            gpio_set_value(LED4_GPIO, value);
            printk(KERN_INFO "LED4 set to %d\n", value);
            break;
    }
    
    kfree(cmd);
    return count;
}

// 文件操作结构体
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .write = led_write,
};

static int __init led_init(void) {
    int ret;
    
    // 申请设备号
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }
    
    // 初始化字符设备
    cdev_init(&led_cdev, &fops);
    led_cdev.owner = THIS_MODULE;
    
    // 添加字符设备到系统
    ret = cdev_add(&led_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }
    
    // 创建设备类
    led_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(led_class)) {
        printk(KERN_ERR "Failed to create class\n");
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(led_class);
    }
    
    // 创建设备节点
    led_device = device_create(led_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        printk(KERN_ERR "Failed to create device\n");
        class_destroy(led_class);
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(led_device);
    }
    
    // 释放可能被占用的GPIO
    gpio_free(LED1_GPIO);
    gpio_free(LED2_GPIO);
    gpio_free(LED3_GPIO);
    gpio_free(LED4_GPIO);
    
    // 申请并配置LED1
    if (gpio_request(LED1_GPIO, "led1")) {
        printk(KERN_ERR "Failed to request GPIO %d\n", LED1_GPIO);
        ret = -EBUSY;
        goto cleanup;
    }
    gpio_direction_output(LED1_GPIO, 0);  // 初始低电平点亮
    
    // 申请并配置LED2
    if (gpio_request(LED2_GPIO, "led2")) {
        printk(KERN_ERR "Failed to request GPIO %d\n", LED2_GPIO);
        gpio_free(LED1_GPIO);
        ret = -EBUSY;
        goto cleanup;
    }
    gpio_direction_output(LED2_GPIO, 0);  // 初始低电平点亮
    
    // 申请并配置LED3
    if (gpio_request(LED3_GPIO, "led3")) {
        printk(KERN_ERR "Failed to request GPIO %d\n", LED3_GPIO);
        gpio_free(LED1_GPIO);
        gpio_free(LED2_GPIO);
        ret = -EBUSY;
        goto cleanup;
    }
    gpio_direction_output(LED3_GPIO, 0);  // 初始低电平点亮
    
    // 申请并配置LED4
    if (gpio_request(LED4_GPIO, "led4")) {
        printk(KERN_ERR "Failed to request GPIO %d\n", LED4_GPIO);
        gpio_free(LED1_GPIO);
        gpio_free(LED2_GPIO);
        gpio_free(LED3_GPIO);
        ret = -EBUSY;
        goto cleanup;
    }
    gpio_direction_output(LED4_GPIO, 0);  // 初始低电平点亮
    
    printk(KERN_INFO "Four LED driver initialized\n");
    return 0;
    
cleanup:
    device_destroy(led_class, dev_num);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit led_exit(void) {
    // 关闭所有LED（输出高电平）
    gpio_set_value(LED1_GPIO, 1);
    gpio_set_value(LED2_GPIO, 1);
    gpio_set_value(LED3_GPIO, 1);
    gpio_set_value(LED4_GPIO, 1);
    
    // 释放所有GPIO
    gpio_free(LED1_GPIO);
    gpio_free(LED2_GPIO);
    gpio_free(LED3_GPIO);
    gpio_free(LED4_GPIO);
    
    // 清理设备
    device_destroy(led_class, dev_num);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_num, 1);
    
    printk(KERN_INFO "Four LED driver exited\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Four LED Control Driver with User Space Interface");
MODULE_AUTHOR("Your Name");

