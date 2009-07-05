#include <linux/init.h>
#include <linux/module.h>

static int __init tfs_init(void)
{
	printk("TFS: Welcome!\n");
	return 0;
}

static void __exit tfs_exit(void)
{
	printk("TFS: Bye!\n");
}

module_init(tfs_init);
module_exit(tfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guanqun Lu <guanqun.lu.list@gmail.com>");

