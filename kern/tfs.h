#ifndef _KERNEL_TFS_H
#define _KERNEL_TFS_H

#include "../user/tfs.h"

/* TFS file system in-core super block info */
struct tfs_sb_info {
	int si_magic;
	int si_inode_nr;
	int *si_inode;
	int si_data_nr;
	int *si_data;
	struct buffer_head *si_sbh;
};

/* TFS file system in-core inode info */
struct tfs_inode_info {
	struct inode vfs_inode;
};

/* dir.c */
extern const struct inode_operations tfs_dir_inops;
extern const struct file_operations tfs_dir_operations;

/* file.c */
extern const struct inode_operations tfs_file_inops;
extern const struct file_operations tfs_file_operations;

#endif /* _KERNEL_TFS_H */
