#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>

#define LED_GPIO 131  // GPIO5_IO03

static int __init led_init(void)
{
    // 先尝试释放GPIO（如果已被占用）
    gpio_free(LED_GPIO);
    
    // 申请GPIO
    if (gpio_request(LED_GPIO, "my_led")) {
        printk(KERN_ERR "Failed to request GPIO %d\n", LED_GPIO);
        return -EBUSY;
    }
    
    // 配置为输出，初始低电平（点亮LED）
    gpio_direction_output(LED_GPIO, 0);
    
    printk(KERN_INFO "LED driver initialized\n");
    return 0;
}

static void __exit led_exit(void)
{
    // 关闭LED（输出高电平）
    gpio_set_value(LED_GPIO, 1);
    
    // 释放GPIO
    gpio_free(LED_GPIO);
    
    printk(KERN_INFO "LED driver exited\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
