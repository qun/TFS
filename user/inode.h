#ifndef _INODE_H
#define _INODE_H

unsigned long inode_to_block(unsigned long ino);
struct tfs_sb *read_sb(int fd, struct tfs_sb *sb);
int write_sb(int fd, struct tfs_sb *sb);
struct tfs_inode *read_inode(int fd, struct tfs_inode *inode, unsigned long ino);
int write_inode(int fd, struct tfs_inode *inode, unsigned long ino);

#endif /* _INODE_H */
