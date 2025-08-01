#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define DRIVER_NAME "ledkey_press_hold"

// GPIO定义
#define LED1_GPIO 3    // GPIO1_IO03 (低电平点亮)
#define LED2_GPIO 5    // GPIO1_IO05 (低电平点亮)
#define KEY1_GPIO 129  // GPIO5_IO01
#define KEY2_GPIO 18   // GPIO1_IO18

static int key1_irq, key2_irq;

// 中断处理函数
static irqreturn_t key_isr(int irq, void *dev_id)
{
    if (irq == key1_irq) {
        int key_state = gpio_get_value(KEY1_GPIO);
        gpio_set_value(LED1_GPIO, key_state); // 按键按下(0)时输出0点亮LED
        printk(KERN_INFO "KEY1 %s, LED1 %s\n",
               key_state ? "released" : "pressed",
               key_state ? "OFF" : "ON");
    }
    else if (irq == key2_irq) {
        int key_state = gpio_get_value(KEY2_GPIO);
        gpio_set_value(LED2_GPIO, key_state);
        printk(KERN_INFO "KEY2 %s, LED2 %s\n",
               key_state ? "released" : "pressed",
               key_state ? "OFF" : "ON");
    }
    return IRQ_HANDLED;
}

static int __init ledkey_init(void)
{
    int ret;
    
    printk(KERN_INFO "Initializing %s (Press-to-light)\n", DRIVER_NAME);
    
    // 初始化LED GPIO（默认熄灭）
    gpio_request(LED1_GPIO, "LED1");
    gpio_direction_output(LED1_GPIO, 1); // 初始高电平=熄灭
    
    gpio_request(LED2_GPIO, "LED2");
    gpio_direction_output(LED2_GPIO, 1);
    
    // 初始化按键GPIO
    gpio_request(KEY1_GPIO, "KEY1");
    gpio_direction_input(KEY1_GPIO);
    
    gpio_request(KEY2_GPIO, "KEY2");
    gpio_direction_input(KEY2_GPIO);
    
    // 申请中断（双边沿触发）
    key1_irq = gpio_to_irq(KEY1_GPIO);
    ret = request_irq(key1_irq, key_isr, 
                     IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                     "key1_irq", NULL);
    
    key2_irq = gpio_to_irq(KEY2_GPIO);
    ret = request_irq(key2_irq, key_isr,
                     IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                     "key2_irq", NULL);
    
    printk(KERN_INFO "%s ready: Hold keys to light LEDs\n", DRIVER_NAME);
    return 0;
}

static void __exit ledkey_exit(void)
{
    free_irq(key1_irq, NULL);
    free_irq(key2_irq, NULL);
    
    gpio_set_value(LED1_GPIO, 1); // 退出时熄灭LED
    gpio_set_value(LED2_GPIO, 1);
    
    gpio_free(LED1_GPIO);
    gpio_free(LED2_GPIO);
    gpio_free(KEY1_GPIO);
    gpio_free(KEY2_GPIO);
    
    printk(KERN_INFO "%s removed\n", DRIVER_NAME);
}

module_init(ledkey_init);
module_exit(ledkey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JIU");
MODULE_DESCRIPTION("Press-and-hold LED control for Low-active LEDs");
