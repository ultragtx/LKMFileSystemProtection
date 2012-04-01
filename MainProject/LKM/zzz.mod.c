#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa7672d5a, "module_layout" },
	{ 0x2e9014ad, "per_cpu__current_task" },
	{ 0xd0d8621b, "strlen" },
	{ 0x7b8d93fd, "remove_proc_entry" },
	{ 0x891d4342, "proc_mkdir" },
	{ 0xb72397d5, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x4ac7113d, "create_proc_entry" },
	{ 0xf2036f33, "fget" },
	{ 0x954cbb26, "vsprintf" },
	{ 0x15307093, "filp_open" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "F3AFB0DB97FCA4722F5E324");
