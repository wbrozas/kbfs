#ifndef KBFS_GROUP_DESCRIPTOR_H
#define KBFS_GROUP_DESCRIPTOR_H
#include <stdint.h>
typedef struct _kbfs_group_descriptor {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
} kbfs_group_descriptor;
#endif
