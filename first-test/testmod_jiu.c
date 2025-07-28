/*
* test module
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init test_jiu_init(void)
{
	printk(KERN_INFO "Test module successfully initialized!\n");
	return 0;
}

static void __exit test_jiu_exit(void)
{
	printk(KERN_INFO "Test module successfully exited!\n");
}

module_init(test_jiu_init);
module_exit(test_jiu_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JIU");

