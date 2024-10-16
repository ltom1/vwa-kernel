///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Header file for the Physical Memory Manager.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <bootinfo.h>
#include <mmap.h>
#include <types.h>
#include <paging.h>


extern u64 kernel_region_end;
extern u8 *bitmap;
extern u64 bitmap_byte_size;
extern u64 bitmap_bit_size;
extern u64 blocks_allocated;


void pmem_init(void);

range_t pmem_get_usable_mem_range(void);
void pmem_bitmap_mark_block(u64 block, bool used);
bool pmem_bitmap_get_block(u64 block);
void pmem_bitmap_mark_blocks(u64 block, u64 count, bool used);
u64 pmem_alloc(pt_t pt4, u64 size);
u64 pmem_alloc_clean(pt_t pt4, u64 size);
u64 pmem_alloc_raw(u64 size);
void pmem_free(pt_t pt4, u64 block, u64 size);
u64 pmem_find_free_region(u64 size);
