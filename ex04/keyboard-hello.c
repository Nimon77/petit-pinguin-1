#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros

MODULE_LICENSE("GPL");

static int __init init(void)
{
    printk(KERN_INFO "keyboard-hello: Keyboard connected !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit cleanup(void)
{
}

module_init(init);
module_exit(cleanup);
