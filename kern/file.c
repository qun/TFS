#include <linux/fs.h>

#include "tfs.h"

const struct inode_operations tfs_file_inops;

const struct file_operations tfs_file_operations = {
	.read		= do_sync_read,
};

