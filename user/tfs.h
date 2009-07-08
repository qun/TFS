#ifndef _TFS_H
#define _TFS_H

#define TFS_MAGIC 0x544653
#define TFS_VERSION 0x1
#define TFS_BLOCK_SIZE 1024
#define TFS_MAX_INODE_BLOCKS 1024
#define TFS_MAX_DATA_BLOCKS (1024 * 100)
#define TFS_MAX_BLOCKS_PER_FILE 100
#define TFS_MAX_FILENAME 124
#define TFS_ROOT_INO 0

struct tfs_sb {
	int s_magic;
	int s_version;
	int s_inode[TFS_MAX_INODE_BLOCKS];
	int s_inode_nr;
	int s_data[TFS_MAX_DATA_BLOCKS];
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
	int i_blocks[TFS_MAX_BLOCKS_PER_FILE];
	int i_blocks_nr;
};

struct tfs_dirent {
	char d_name[TFS_MAX_FILENAME];
	int d_ino;
};

#endif /*_TFS_H */
