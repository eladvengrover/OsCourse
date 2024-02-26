#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe63d5a03, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xc5850110, "printk" },
	{ 0x32d48119, "__register_chrdev" },
	{ 0x37a0cba, "kfree" },
	{ 0xc3aaf0a9, "__put_user_1" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x167e7f9d, "__get_user_1" },
	{ 0x14994725, "kmem_cache_alloc_trace" },
	{ 0x6104427f, "kmalloc_caches" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

