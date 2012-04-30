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
	{ 0xd42b7232, "_write_unlock_bh" },
	{ 0x6e7e4bf9, "sock_release" },
	{ 0xe50938e, "nf_register_hook" },
	{ 0xc2b8a998, "netlink_kernel_create" },
	{ 0xe4c1df3e, "_read_lock_bh" },
	{ 0xa2a1e5c9, "_write_lock_bh" },
	{ 0x9ced38aa, "down_trylock" },
	{ 0xb72397d5, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0xbc14001d, "netlink_unicast" },
	{ 0x3d1b48b1, "init_net" },
	{ 0xad2cde6b, "__alloc_skb" },
	{ 0x99e05b8f, "kfree_skb" },
	{ 0x49da9a9a, "_read_unlock_bh" },
	{ 0x2f0585b2, "nf_unregister_hook" },
	{ 0x3f1899f1, "up" },
	{ 0x40ba24d7, "skb_put" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0FA51EAEA93CBEE976EE843");
