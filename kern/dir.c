#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>

#include "../user/tfs.h"
#include "tfs.h"

static int tfs_create(struct inode *dir, struct dentry *dentry,
		      int mode, struct nameidata *nd)
{
	return 0;
}

/* TODO mutex lock */
static int tfs_readdir(struct file *f, void *dirent, filldir_t filldir)
{
	int real_size = (TFS_BLOCK_SIZE / sizeof(struct tfs_dirent)) *
		sizeof(struct tfs_dirent);
	struct inode *dir = f->f_path.dentry->d_inode;
	struct tfs_inode_info *info = TFS_INODE_I(dir);
	struct buffer_head *bh;
	struct tfs_dirent *de;
	int block, offset;

	pr_info("readdir invoked!\n");
	if ((f->f_pos % real_size) & (sizeof(struct tfs_dirent) -1)) {
		pr_err("tfs_readdir: Bad f_pos: %lx.\n",
		       (unsigned long)f->f_pos);
		return -EBADF;
	}
	while (f->f_pos < dir->i_size) {
		block = f->f_pos / real_size;
		block = info->ni_blocks[block];
		offset = f->f_pos % real_size;

		pr_info("readdir: block:%d at offset: %d  dir_size: %lu\n",
			 block, offset, (unsigned long)dir->i_size);
		bh = sb_bread(dir->i_sb, block);
		if (!bh)
			return -EIO;

		do {
			pr_info("  [block %d]: offset: %d.\n", block, offset);
			de = (struct tfs_dirent *)(bh->b_data + offset);
			pr_info("    ->[inode: %d] name: %s\n", de->d_ino, de->d_name);
			if (de->d_ino) {
				int len = strnlen(de->d_name, TFS_MAX_FILENAME);
				if (filldir(dirent, de->d_name, len, f->f_pos,
					    de->d_ino,
					    DT_UNKNOWN) < 0) {
					brelse(bh);
					return 0;
				}
			}

			offset += sizeof(struct tfs_dirent);
			f->f_pos += sizeof(struct tfs_dirent);
		} while (offset < TFS_BLOCK_SIZE && f->f_pos < dir->i_size);
		brelse(bh);
	}
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
	.readdir	= tfs_readdir,
};

