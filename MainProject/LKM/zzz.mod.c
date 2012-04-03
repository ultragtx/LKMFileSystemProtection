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
	{ 0x6d9d966, "d_path" },
	{ 0x2e9014ad, "per_cpu__current_task" },
	{ 0x20623cac, "kmalloc_caches" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xd0d8621b, "strlen" },
	{ 0x1a75caa3, "_read_lock" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0x7b8d93fd, "remove_proc_entry" },
	{ 0xddf663af, "path_get" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x891d4342, "proc_mkdir" },
	{ 0xb72397d5, "printk" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0xfa733bba, "fput" },
	{ 0x61651be, "strcat" },
	{ 0x6eb25b1e, "kmem_cache_alloc" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x4ac7113d, "create_proc_entry" },
	{ 0x37a0cba, "kfree" },
	{ 0xf2036f33, "fget" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x954cbb26, "vsprintf" },
	{ 0xe914e41e, "strcpy" },
	{ 0x15307093, "filp_open" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3D2E2BCCA48697DCC86A1B0");
