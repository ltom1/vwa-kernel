///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Header file for the buddy allocator.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <alloc.h>


/// @brief  Buddy Allocator class.
typedef struct BuddyAllocator {
    allocator_t allocator;
} buddy_allocator_t;

#define CREATE_BUDDY_ALLOCATOR(blocks) (buddy_allocator_t) { \
        (allocator_t) { \
            P2V(pmem_alloc_clean(kernel_pt4, blocks)), \
            blocks, \
            blocks * PAGE_SIZE, \
            buddy_alloc, \
            buddy_free, \
            buddy_init \
        } \
    }; \


// around 3.2% of allocatable space will be wasted per page
#define BUDDY_BITMAP_SIZE   128
#define LAYERS              10
#define SMALLEST            8
#define GREATEST            PAGE_SIZE
#define TOTAL_BLOCKS        1023


void buddy_visualize_bitmap(u8* bitmap);

u64 buddy_layer_block_size(u64 layer);
u64 buddy_layer_from_size(u64 n_bytes);
u64 buddy_bits_in_layer(u64 layer);
u64 buddy_bit_offset_layer(u64 layer);

bool buddy_bitmap_get_bit(u8 *bitmap, u64 bit);
void buddy_bitmap_mark_bits(u8 *bitmap, u64 off, u64 count, bool val);
void buddy_bitmap_mark_bit(u8 *bitmap, u64 off, bool val);

u64 buddy_alloc(buddy_allocator_t *self, u64 n_bytes);
void buddy_free(buddy_allocator_t *self, u64 vaddr);
void buddy_init(buddy_allocator_t *self);
