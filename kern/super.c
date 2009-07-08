#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/buffer_head.h>
#include <linux/list.h>

#include "../user/tfs.h"
#include "tfs.h"

static struct kmem_cache *tfs_inode_cachep;

static void tfs_inode_init_once(void *obj)
{
	inode_init_once(obj);
}

static int tfs_init_inode_cache(void)
{
	tfs_inode_cachep = kmem_cache_create("tfs_inode",
					     sizeof(struct tfs_inode_info),
					     0, (SLAB_RECLAIM_ACCOUNT |
						 SLAB_MEM_SPREAD),
					     tfs_inode_init_once);
	if (!tfs_inode_cachep)
		return -ENOMEM;
	return 0;
}

static void tfs_destroy_inode_cache(void)
{
	kmem_cache_destroy(tfs_inode_cachep);
}

static struct inode *tfs_alloc_inode(struct super_block *sb)
{
	struct tfs_inode_info *inode;

	inode = kmem_cache_alloc(tfs_inode_cachep, GFP_KERNEL);
	if (!inode)
		return NULL;
	return &inode->vfs_inode;
}

static struct super_operations tfs_sops = {
	.alloc_inode	= tfs_alloc_inode,
};

static struct inode *tfs_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct tfs_inode *raw_inode;
	struct buffer_head *bh;
	unsigned long block;

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	/* check for validity of ino number */
	if (ino < TFS_ROOT_INO || ino >= TFS_MAX_INODE_BLOCKS)
		return ERR_PTR(-EINVAL);

	/* translation between inode number and real section */
	block = ino + 1 /* block 0 (super block) */;
	bh = sb_bread(sb, block);
	if (!bh) {
		iget_failed(inode);
		return ERR_PTR(-EIO);
	}

	/* initialization for new inode */
	raw_inode = (struct tfs_inode *) bh->b_data;
	inode->i_mode = raw_inode->i_mode;
	inode->i_atime.tv_sec = raw_inode->i_atime;
	inode->i_mtime.tv_sec = raw_inode->i_mtime;
	inode->i_ctime.tv_sec = raw_inode->i_ctime;
	inode->i_ctime.tv_nsec = 0;
	inode->i_atime.tv_nsec = 0;
	inode->i_mtime.tv_nsec = 0;
	inode->i_uid = raw_inode->i_uid;
	inode->i_gid = raw_inode->i_gid;
	inode->i_nlink = raw_inode->i_links;
	inode->i_size = TFS_BLOCK_SIZE;
	inode->i_blocks = 1;
	if (inode->i_mode & S_IFDIR) {
		inode->i_op = &tfs_dir_inops;
		inode->i_fop = &tfs_dir_operations;
	} else {
		inode->i_op = &tfs_file_inops;
		inode->i_fop = &tfs_file_operations;
	}

	brelse(bh);
	unlock_new_inode(inode);

	return inode;
}

static int tfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	struct buffer_head *bh;
	struct tfs_sb *raw_sb;
	struct tfs_sb_info *sbi;
	int ret = -EINVAL;
	int i;

	sbi = kzalloc(sizeof(*sbi), GFP_KERNEL);
	if (!sb)
		return -ENOMEM;
	sb->s_fs_info = sbi;

	sb_set_blocksize(sb, TFS_BLOCK_SIZE);
	bh = sb_bread(sb, 0);
	if (!bh)
		goto out;
	raw_sb = (struct tfs_sb *) bh->b_data;
	if (raw_sb->s_magic != TFS_MAGIC)
		goto out;

	sb->s_magic = TFS_MAGIC;
	sbi->si_sbh = bh; /* when free? */

	sb->s_op = &tfs_sops;
	root = tfs_iget(sb, TFS_ROOT_INO);
	if (IS_ERR(root)) {
		ret = PTR_ERR(root);
		goto out;
	}
	sb->s_root = d_alloc_root(root);
	if (!sb->s_root) {
		iput(root); /* ??? unlock_ */
		ret = -ENOMEM;
		goto out;
	}

	sbi->si_inode_nr = raw_sb->s_inode_nr;
	sbi->si_data_nr = raw_sb->s_data_nr;
	for (i = 0; i < TFS_MAX_INODE_BLOCKS; i++)
		sbi->si_inode[i] = raw_sb->s_inode[i];
	for (i = 0; i < TFS_MAX_DATA_BLOCKS; i++)
		sbi->si_data[i] = raw_sb->s_data[i];

out:
	brelse(bh);
	kfree(sbi);
	sb->s_fs_info = NULL;
	return ret;
}

static int tfs_get_sb(struct file_system_type *fs_type, int flags,
		      const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name,
			   data, tfs_fill_super, mnt);
}

static struct file_system_type tfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "tfs",
	.get_sb		= tfs_get_sb,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init tfs_init(void)
{
	int err;

	err = tfs_init_inode_cache();
	if (err)
		return err;
	err = register_filesystem(&tfs_fs_type);
	if (err)
		tfs_destroy_inode_cache();
	return err;
}

static void __exit tfs_exit(void)
{
	unregister_filesystem(&tfs_fs_type);
	tfs_destroy_inode_cache();
}

module_init(tfs_init);
module_exit(tfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guanqun Lu <guanqun.lu.list@gmail.com>");

