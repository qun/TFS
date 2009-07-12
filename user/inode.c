#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "tfs.h"
#include "inode.h"

unsigned long inode_to_block(unsigned long ino)
{
	return ino;
}

struct tfs_sb *read_sb(int fd, struct tfs_sb *sb)
{
	lseek(fd, 0, SEEK_SET);
	if (read(fd, sb, sizeof(*sb)) != sizeof(*sb)) {
		fprintf(stderr, "read super block error!\n");
		return NULL;
	}
	return sb;
}

int write_sb(int fd, struct tfs_sb *sb)
{
	lseek(fd, 0, SEEK_SET);
	if (write(fd, sb, sizeof(*sb)) != sizeof(*sb)) {
		fprintf(stderr, "write super block error!\n");
		return -EIO;
	}
	return 0;
}

struct tfs_inode *read_inode(int fd, struct tfs_inode *inode, unsigned long ino)
{
	loff_t offset;

	if (ino < TFS_ROOT_INO || ino >= TFS_MAX_INODE_BLOCKS)
		return NULL;

	offset = ino * TFS_BLOCK_SIZE;
	lseek(fd, offset, SEEK_SET);
	if (read(fd, inode, sizeof(*inode)) != sizeof(*inode)) {
		fprintf(stderr, "read inode %lu error!\n", ino);
		return NULL;
	}
	return inode;
}

int write_inode(int fd, struct tfs_inode *inode, unsigned long ino)
{
	loff_t offset;

	if (ino < TFS_ROOT_INO || ino >= TFS_MAX_INODE_BLOCKS)
		return -EINVAL;

	offset = ino * TFS_BLOCK_SIZE;
	lseek(fd, offset, SEEK_SET);
	if (write(fd, inode, sizeof(*inode)) != sizeof(*inode)) {
		fprintf(stderr, "write inode %lu error!\n", ino);
		return -EIO;
	}
	return 0;
}

