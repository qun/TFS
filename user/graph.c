#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "tfs.h"
#include "inode.h"

int fd;
FILE *fdout;
int *inode_bitmap;

void get_dirent(struct tfs_inode *inode, int sz, struct tfs_dirent *de)
{
	int block, offset;
	int real_size = (TFS_BLOCK_SIZE / sizeof(*de)) * sizeof(*de);

	block = sz / real_size;
	block = inode->i_blocks[block];
	offset = sz % real_size;

	lseek(fd, block * TFS_BLOCK_SIZE + offset, SEEK_SET);
	memset(de, 0, sizeof(*de));
	read(fd, de, sizeof(*de));
}

void show_node(unsigned long ino)
{
	struct tfs_inode inode;
	struct tfs_dirent de;
	int sz;

	if (ino < TFS_ROOT_INO || ino >= TFS_MAX_INODE_BLOCKS)
		return;

	if (inode_bitmap[ino])
		return;
	inode_bitmap[ino] = 1;

	read_inode(fd, &inode, ino);
	if (S_ISDIR(inode.i_mode)) {
		int label = 0;
		for (sz = 0;
		     sz < inode.i_size;
		     sz += sizeof(struct tfs_dirent)) {
			get_dirent(&inode, sz, &de);
			fprintf(fdout, "inode%lu:l%d -> inode%d:top;\n",
				ino, label++, de.d_ino);
		}

	}
	fprintf(fdout, "inode%lu [shape=record,label=\""
		"{<top> inode:\\ %lu | gid:\\ %d | uid:\\ %d |"
		"mtime:\\ %d | atime:\\ %d | ctime:\\ %d |"
		"mode:\\ %x | size:\\ %d",
		ino, ino, inode.i_gid, inode.i_uid,
		inode.i_mtime, inode.i_atime, inode.i_ctime,
		inode.i_mode, inode.i_size);
	if (S_ISDIR(inode.i_mode)) {
		int label = 0;
		for (sz = 0;
		     sz < inode.i_size;
		     sz += sizeof(struct tfs_dirent)) {
			get_dirent(&inode, sz, &de);
			fprintf(fdout, "| <l%d> %s",
				label++, de.d_name);
		}
	}
	fprintf(fdout, "}\" ];\n");

	if (S_ISDIR(inode.i_mode)) {
		for (sz = 0;
		     sz < inode.i_size;
		     sz += sizeof(struct tfs_dirent)) {
			get_dirent(&inode, sz, &de);
			show_node(de.d_ino);
		}
	}
}

int main(int argc, char *argv[])
{
	struct tfs_sb sb;
	int i;

	if (argc != 2)
		return 1;

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		return 1;
	fdout = fopen("pic.dot", "w");
	if (!fdout)
		return 1;

	read_sb(fd, &sb);
	inode_bitmap = malloc(sizeof(int) * (sb.s_inode_nr + 1));
	for (i = 0; i < sb.s_inode_nr; i++)
		inode_bitmap[i] = 0;

	fprintf(fdout, "digraph tfs {\n");
	show_node(TFS_ROOT_INO);
	fprintf(fdout, "}\n");

	free(inode_bitmap);
	return 0;
}

