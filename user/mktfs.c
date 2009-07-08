#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	struct stat st;

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

	/* write super block */
	sb.s_magic = TFS_MAGIC;
	sb.s_version = TFS_VERSION;
	memset(sb.s_inode, 0, sizeof(sb.s_inode));
	sb.s_inode_nr = 0;
	memset(sb.s_data, 0, sizeof(sb.s_data));
	sb.s_data_nr = 0;
	write(fd, &sb, sizeof(sb));

	/* leave others un-touched */

	close(fd);
	return 0;
}
