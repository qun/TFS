#ifndef _TFS_H
#define _TFS_H

#define TFS_MAGIC 0x544653;
#define TFS_VERSION 0x1
#define BLOCK_SIZE 1024
#define MAX_INODE_BLOCKS 1024
#define MAX_DATA_BLOCKS (1024 * 100)
#define MAX_BLOCKS_PER_FILE 100

struct tfs_sb {
	int s_magic;
	int s_version;
	int s_inode[MAX_INODE_BLOCKS];
	int s_inode_nr;
	int s_data[MAX_DATA_BLOCKS];
	int s_data_nr;
};

struct tfs_inode {
	int i_gid;
	int i_uid;
	int i_ctime;
	int i_mtime;
	int i_atime;
	int i_mode;
	int i_links;
	int i_size;
	int i_blocks[MAX_BLOCKS_PER_FILE];
	int i_blocks_nr;
};

#endif /*_TFS_H */
