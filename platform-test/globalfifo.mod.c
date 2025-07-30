#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9a454969, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xfff289f3, __VMLINUX_SYMBOL_STR(single_release) },
	{ 0xc2910861, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x5971e172, __VMLINUX_SYMBOL_STR(seq_lseek) },
	{ 0x762124b9, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x8b93010a, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x4eb1a083, __VMLINUX_SYMBOL_STR(seq_puts) },
	{ 0xc89de85, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x59435cb7, __VMLINUX_SYMBOL_STR(single_open) },
	{ 0x929dbad3, __VMLINUX_SYMBOL_STR(PDE_DATA) },
	{ 0x8b2d7d64, __VMLINUX_SYMBOL_STR(dev_warn) },
	{ 0x521b39ba, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xefd442fe, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xf87bf551, __VMLINUX_SYMBOL_STR(device_create_file) },
	{ 0xd664465f, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0x275ef902, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x57718273, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x879e479b, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x9d669763, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0xce9ef1a7, __VMLINUX_SYMBOL_STR(kill_fasync) },
	{ 0xd85cd67e, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0x4c86184b, __VMLINUX_SYMBOL_STR(remove_wait_queue) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xc7bcbc8d, __VMLINUX_SYMBOL_STR(add_wait_queue) },
	{ 0xffd5a395, __VMLINUX_SYMBOL_STR(default_wake_function) },
	{ 0xc031c111, __VMLINUX_SYMBOL_STR(fasync_helper) },
	{ 0xba3b4911, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0xadaf9df6, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0xc510b017, __VMLINUX_SYMBOL_STR(device_remove_file) },
	{ 0xb99135e, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x323cc41c, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x5e50f6cd, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cglobalfifo");
MODULE_ALIAS("of:N*T*CglobalfifoC*");

MODULE_INFO(srcversion, "7B62578BC42D525B32082FF");
