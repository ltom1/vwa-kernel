///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for allocating, organising and freeing physical memory (pages).
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <mmap.h>
#include <types.h>
#include <bootinfo.h>
#include <pmem.h>
#include <vmem.h>
#include <err.h>
#include <x86.h>
#include <tty.h>
#include <paging.h>
#include <utils.h>


/// @brief  The bitmap used by the bitmap allocator.
///
/// Contains a bit for each page frame: 0 -> unused, 1 -> used.
u8 *bitmap;
/// @brief  The bitmap size in bytes.
u64 bitmap_byte_size;
/// @brief  The bitmap size in bits (= number of pages/blocks).
u64 bitmap_bit_size;

/// @brief  The number of pages already allocated.
u64 blocks_allocated = 0;
/// @brief  Used by the bitmap allocator (next fit).
u64 next = 0;

/// @brief  The end of the kernel region in blocks.
u64 kernel_region_end = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Physical Memory Manager (bitmap allocator, next fit).
///////////////////////////////////////////////////////////////////////////////////////////////////

void pmem_init(void) {

    tty_puts(WHITE_ON_BLACK, "Setting up PMEM...");

    // update physical-to-virtual translation base
    pv_base = bootinfo->kernel_map.virt;

    // calculate the size of the bitmap according to the usable memory range
    range_t range = pmem_get_usable_mem_range();
    bitmap_bit_size = (range.end - range.base) / PAGE_SIZE;
    bitmap_byte_size = bitmap_bit_size / 8;

    // chose a location for the bitmap
    // get the first unused region that is big enough after the kernel load address
    u64 bitmap_addr = -1;
    for (u64 i = 0; i < bootinfo->num_regions; i++) {
        if (bootinfo->regions[i].type != FREE) continue;
        if (bootinfo->regions[i].length < bitmap_byte_size) continue;
        if (bootinfo->regions[i].base < bootinfo->kernel_load_addr) continue;
        if (bootinfo->regions[i].base > bitmap_addr) continue;

        bitmap_addr = bootinfo->regions[i].base;
    }

    // panic if no region was found or if the mapped area is too small
    if (bitmap_addr == (u64)-1 || 
        bitmap_addr + bitmap_byte_size > bootinfo->kernel_map.phys + bootinfo->kernel_map.size
        ) panic("Couldn't find bitmap location");

    bitmap = (u8*)P2V(bitmap_addr);
    
    // BUILD BITMAP

    // initilize to all used
    mem_set((u8*)bitmap, -1, bitmap_byte_size);

    u64 start_block;
    u64 end_block;

    // mark free regions
    for (u64 i = 0; i < bootinfo->num_regions; i++) {
    
        if (bootinfo->regions[i].type != FREE) continue;

        start_block = page_round_up(bootinfo->regions[i].base);
        end_block = page_round_down(bootinfo->regions[i].base + bootinfo->regions[i].length);
        pmem_bitmap_mark_blocks(
                    start_block,
                    end_block - start_block,
                    false
                );
    }
     

    // mark reserved regions
    for (u64 i = 0; i < bootinfo->num_regions; i++) {
    
        if (bootinfo->regions[i].type == FREE) continue;

        start_block = page_round_down(bootinfo->regions[i].base);
        end_block = page_round_up(bootinfo->regions[i].base + bootinfo->regions[i].length);

        // bitmap_size doesn't include the last used regions so we'll ignore them
        if (end_block > bitmap_bit_size) continue;
        
        pmem_bitmap_mark_blocks(
                    start_block,
                    end_block - start_block,
                    true
                );
    }

    // mark regions used by the bootloader and kernel (entire low memory) as reserved
    u64 nearest_address = -1;
    for (u64 i = 0; i < bootinfo->num_regions; i++) {
        if (bootinfo->regions[i].type == FREE) continue;
        if (bootinfo->regions[i].base < bootinfo->kernel_load_addr) continue;

        if (bootinfo->regions[i].base < nearest_address) {
            nearest_address = bootinfo->regions[i].base;
        }
        
    }
    kernel_region_end = page_round_up(nearest_address);
    pmem_bitmap_mark_blocks(0, kernel_region_end, true);

    // mark bitmap region as reserved
    pmem_bitmap_mark_blocks(
            page_round_down(V2P((u64)bitmap)), page_round_up(bitmap_byte_size), 
            true);

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Marks a block in the Physical Memory Allocator bitmap as used or unused.
///
/// @param  block   The block to map (bit number).
/// @param  used    The value for the block. 
///////////////////////////////////////////////////////////////////////////////////////////////////

void pmem_bitmap_mark_block(u64 block, bool used) {

    u64 byte = block / 8;
    u64 off = block % 8;

    if (used) {
        bitmap[byte] |= (1 << (7 - off));
    } else {
        bitmap[byte] &= ~(1 << (7 - off));
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks if a block is used or not.
///
/// @param  block   The block to check.
///
/// @returns    True if it the block is used - False if it is unused.
///////////////////////////////////////////////////////////////////////////////////////////////////

bool pmem_bitmap_get_block(u64 block) {

    u64 byte = block / 8;
    u64 off = block % 8;

    return (bitmap[byte] >> (7 - off)) & 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Marks blocks in the bitmap as used or unused.
///
/// @param  block   The first block to mark.
/// @param  count   How many blocks to mark.
/// @param  used    The value for the blocks.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pmem_bitmap_mark_blocks(u64 block, u64 count, bool used) {

    for (; block % 8 && count > 0; ++block, --count)
        pmem_bitmap_mark_block(block, used);

    // speed things up by writing whole bytes
    if (count >= 8) {

        mem_set(&bitmap[block / 8], used * -1, count / 8);
        block += (count - count % 8);
        count = count % 8;
    }

    // rest
    for (; count > 0; ++block, --count)
        pmem_bitmap_mark_block(block, used);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Returns the usable memory range.
///
/// @returns    The usable memory range as a range_t struct.
///
/// Range goes from the smallest free region base address to the biggest free region end address.
///////////////////////////////////////////////////////////////////////////////////////////////////

range_t pmem_get_usable_mem_range(void) {

    u64 mem_base = -1;
    u64 mem_end = 0;

    u64 base;
    u64 end;

    for (u64 i = 0; i < bootinfo->num_regions; i++) {

        if (bootinfo->regions[i].type != FREE) continue;
        
        
        base = bootinfo->regions[i].base;
        end = base + bootinfo->regions[i].length;

        if (base < mem_base) mem_base = base;
        if (end > mem_end) mem_end = end;
    }

    return (range_t) {mem_base, mem_end};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Allocates the specified amount of physical memory frames.
///
/// @param  size    The amount of pages to allocate.
///
/// @returns    A pointer to the beginning of the allocated region.
///
/// @warning    Does NOT zero-initialize the region. DOES also map it to virtual memory.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 pmem_alloc(pt_t pt4, u64 size) {

    u64 block = pmem_find_free_region(size);
    if (block == (u64)-1) panic("Out of memory");

    vmem_map_region(
            pt4,
            bootinfo->kernel_map.virt + block * PAGE_SIZE, 
            block * PAGE_SIZE, 
            PAGE_WRITE, 
            size);

    pmem_bitmap_mark_blocks(block, size, true);
    
    blocks_allocated++;
    return block * PAGE_SIZE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Allocates (and zero-initializes) the specified amount of physical memory frames.
///
/// @param  size    The amount of pages to allocate.
///
/// @returns    A pointer to the beginning of the allocated region.
///
/// @warning    DOES zero-initialize the region. DOES also map it to virtual memory.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 pmem_alloc_clean(pt_t pt4, u64 size) {

    u64 block = pmem_find_free_region(size);
    if (block == (u64)-1) panic("Out of memory");

    vmem_map_region(
            pt4,
            bootinfo->kernel_map.virt + block * PAGE_SIZE,
            block * PAGE_SIZE,
            PAGE_WRITE, 
            size);

    pmem_bitmap_mark_blocks(block, size, true);
    
    mem_set((u8*)P2V(block * PAGE_SIZE), 0, size * PAGE_SIZE);

    blocks_allocated++;
    return block * PAGE_SIZE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Allocates (and zero-initializes) 
/// the specified amount of physical memory frames without mapping.
///
/// @param  size    The amount of pages to allocate.
///
/// @returns    A pointer to the beginning of the allocated region.
///
/// @warning    DOES zero-initialize the region. Does NOT map it to virtual memory.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 pmem_alloc_raw(u64 size) {

    u64 block = pmem_find_free_region(size);
    if (block == (u64)-1) panic("Out of memory");

    pmem_bitmap_mark_blocks(block, size, true);
    
    mem_set((u8*)P2V(block * PAGE_SIZE), 0, size * PAGE_SIZE);

    blocks_allocated++;
    return block * PAGE_SIZE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Frees and unmaps the specified amount of physical memory frames.
///
/// @param  base_addr   The base address of the region to deallocate.
/// @param  size        The amount of pages to deallocate.
///
/// @warning    DOES also unmap the region.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pmem_free(pt_t pt4, u64 base_addr, u64 size) {

    vmem_unmap_region(pt4, base_addr, size);
    pmem_bitmap_mark_blocks(base_addr / PAGE_SIZE, size, false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Searches for a region of a specified size using the next fit method.
///
/// @param  size    The size of the requested region.
///
/// @returns    The start block of the found region or -1 if no region could be found.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 pmem_find_free_region(u64 size) {

    // reset the next if the previous block has been freed
    if (next > 0 && !pmem_bitmap_get_block(next - 1)) next = 0;

    u64 cur_size = 0;
    u64 cur_start = next % bitmap_bit_size;
    u64 cur_block = 0;

    for (u64 i = 0; i < bitmap_bit_size; i++) {

        cur_block = (next + i) % bitmap_bit_size;

        if (cur_block == 0) {
            cur_size = 0;
            cur_start = 0;
        }
        
        if (pmem_bitmap_get_block(cur_block)) {
            cur_size = 0;
            cur_start = cur_block + 1;
        } else {
        
            cur_size++;
            if (cur_size >= size) {
                next = (cur_start + 1) % bitmap_bit_size;
                return cur_start;
            }
        }
    }

    return -1;
}
