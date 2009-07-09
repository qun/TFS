#include <linux/fs.h>

static int tfs_create(struct inode *dir, struct dentry *dentry,
		      int mode, struct nameidata *nd)
{
	return 0;
}

struct dentry *tfs_lookup(struct inode *inode, struct dentry *de, struct nameidata *nd)
{
	printk("tfs_lookup invoked!\n");
	return NULL;
}

const struct inode_operations tfs_dir_inops = {
	.create		= tfs_create,
	.lookup		= tfs_lookup,
};

const struct file_operations tfs_dir_operations = {
	.read		= generic_read_dir,
};

