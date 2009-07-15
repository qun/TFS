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

/* input: de->d_name.name
 * output: return NULL if everything's OK. and need to d_add(de, inode)
 */
struct dentry *tfs_lookup(struct inode *dir, struct dentry *de, struct nameidata *nd)
{
	struct tfs_inode_info *info = TFS_INODE_I(dir);
	struct tfs_dirent *tfs_de;
	struct inode *inode;
	struct buffer_head *bh;
	int size, block, i, offset;

	pr_info("tfs_lookup invoked!\n");

	if (de->d_name.len >= TFS_MAX_FILENAME)
		return ERR_PTR(-ENAMETOOLONG);

	size = 0;
	i = 0;
	while (size < dir->i_size && i < info->ni_blocks_nr) {
		block = info->ni_blocks[i];

		pr_info("  lookup block: %d\n", block);
		bh = sb_bread(dir->i_sb, block);
		if (!bh)
			return ERR_PTR(-EACCES);

		/* one block */
		offset = 0;
		while (size < dir->i_size && offset < TFS_BLOCK_SIZE) {
			tfs_de = (struct tfs_dirent *) (bh->b_data + offset);
			if (tfs_de->d_ino) {
				pr_info("  lookup dir: ino: %d name:%s\n",
					tfs_de->d_ino, tfs_de->d_name);
				if (!strncmp(de->d_name.name, tfs_de->d_name,
					     de->d_name.len)) {
					/* found */
					inode = tfs_iget(dir->i_sb, tfs_de->d_ino);
					if (IS_ERR(inode)) {
						brelse(bh);
						return ERR_PTR(-EACCES);
					}
					d_add(de, inode);
					brelse(bh);
					return NULL;
				}
				size += sizeof(struct tfs_dirent);
			}
			offset += sizeof(struct tfs_dirent);
		}
		brelse(bh);

		i++;
	}
	return ERR_PTR(-ENOENT);
}

const struct inode_operations tfs_dir_inops = {
	.create		= tfs_create,
	.lookup		= tfs_lookup,
};

const struct file_operations tfs_dir_operations = {
	.read		= generic_read_dir,
	.readdir	= tfs_readdir,
};

