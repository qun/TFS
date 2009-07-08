#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "tfs.h"

const char *prog_name = NULL;

void show_help_message(void)
{
	printf("Usage: %s <device-file>\n", prog_name);
}

int main(int argc, char *argv[])
{
	int fd;
	int total_blocks;
	struct tfs_sb sb;
	struct tfs_inode inode;
	struct tfs_dirent dirent;
	struct stat st;
	time_t cur_time;
	off_t offset;

	prog_name = argv[0];
	if (argc != 2) {
		show_help_message();
		return 1;
	}

	if (stat(argv[1], &st) < 0) {
		fprintf(stderr, "No such file or directory: %s\n", argv[1]);
		return 1;
	}

	/* check the available size */
	total_blocks = 1 /* super block */ + TFS_MAX_INODE_BLOCKS + TFS_MAX_DATA_BLOCKS;
	if (st.st_size < total_blocks * TFS_BLOCK_SIZE) {
		fprintf(stderr, "No enough space (minimum required: %dKB).\n",
			total_blocks * TFS_BLOCK_SIZE / 1024);
		return 1;
	}

	fd = open(argv[1], O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open file: %s\n", argv[1]);
		return 1;
	}

	/* super block */
	sb.s_magic = TFS_MAGIC;
	sb.s_version = TFS_VERSION;
	memset(sb.s_inode, 0, sizeof(sb.s_inode));
	sb.s_inode_nr = 0;
	memset(sb.s_data, 0, sizeof(sb.s_data));
	sb.s_data_nr = 0;

	sb.s_inode[sb.s_inode_nr++] = TFS_ROOT_INO + 1;
	sb.s_data[sb.s_data_nr++] = 1 + TFS_MAX_INODE_BLOCKS;
	offset = 0;
	lseek(fd, offset, SEEK_SET);
	write(fd, &sb, sizeof(sb));

	/* root inode */
	cur_time = time(NULL);
	inode.i_gid = 0;
	inode.i_uid = 0;
	inode.i_ctime = cur_time;
	inode.i_mtime = cur_time;
	inode.i_atime = cur_time;
	inode.i_mode = S_IFDIR | 0666;
	inode.i_links = 2; /* ??? */
	inode.i_size = 2 * sizeof(dirent);
	inode.i_blocks[inode.i_blocks_nr++] = 1 + TFS_MAX_INODE_BLOCKS;

	offset = (TFS_ROOT_INO + 1) * TFS_BLOCK_SIZE;
	lseek(fd, offset, SEEK_SET);
	write(fd, &inode, sizeof(inode));

	/* directory entiry '.' */
	memcpy(dirent.d_name, ".", 1);
	dirent.d_ino = TFS_ROOT_INO;
	offset = (1 + TFS_MAX_INODE_BLOCKS) * TFS_BLOCK_SIZE;
	lseek(fd, offset, SEEK_SET);
	write(fd, &dirent, sizeof(dirent));

	/* directory entry '..' */
	memcpy(dirent.d_name, "..", 2);
	dirent.d_ino = TFS_ROOT_INO;
	offset += TFS_BLOCK_SIZE;
	lseek(fd, offset, SEEK_SET);
	write(fd, &dirent, sizeof(dirent));

	close(fd);
	return 0;
}
