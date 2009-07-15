#ifndef _KERNEL_TFS_H
#define _KERNEL_TFS_H

#include "../user/tfs.h"

/* TFS file system in-core super block info */
struct tfs_sb_info {
	int si_magic;
	int si_inode_nr;
	int si_inode[TFS_MAX_INODE_BLOCKS];
	int si_data_nr;
	int si_data[TFS_MAX_DATA_BLOCKS];
	struct buffer_head *si_sbh;
};

/* TFS file system in-core inode info */
struct tfs_inode_info {
	int ni_blocks[TFS_MAX_BLOCKS_PER_FILE];
	int ni_blocks_nr;
	struct inode vfs_inode;
};

static inline struct tfs_inode_info *TFS_INODE_I(struct inode *inode)
{
	return container_of(inode, struct tfs_inode_info, vfs_inode);
}

/* dir.c */
extern const struct inode_operations tfs_dir_inops;
extern const struct file_operations tfs_dir_operations;

/* file.c */
extern const struct inode_operations tfs_file_inops;
extern const struct file_operations tfs_file_operations;

/* super.c */
struct inode *tfs_iget(struct super_block *sb, unsigned long ino);

#endif /* _KERNEL_TFS_H */
