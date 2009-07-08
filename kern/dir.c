#include <linux/fs.h>

static int tfs_create(struct inode *dir, struct dentry *dentry,
		      int mode, struct nameidata *nd)
{
	return 0;
}

const struct inode_operations tfs_dir_inops = {
	.create		= tfs_create,
};

const struct file_operations tfs_dir_operations = {
	.read		= generic_read_dir,
};

