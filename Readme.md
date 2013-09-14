kbfs
====

### Why am I writing a file system?
I am writing this file system to learn. Keep in mind I am not an expert at this, but I hope that it may be a useful resource for some one else in the future.

### The Plan
My plan is to write a file system in userspace using fuse and to later integrate it with some version of the linux kernel. To start the fuse file system will write its superblock, blocks and inode table in a file as if it were a block device.
