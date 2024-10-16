///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for allocating, organising and freeing virtual memory (pages).
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <bootinfo.h>
#include <mmap.h>
#include <types.h>
#include <vmem.h>
#include <pmem.h>
#include <paging.h>
#include <x86.h>
#include <utils.h>
#include <err.h>
#include <tty.h>
#include <proc.h>


/// @brief  The 4th level page table of the kernel.
pt_t kernel_pt4;

/// @brief  Memory range of the VGA area.
const range_t vga_range = {0xa0000, 0xbffff};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Maps a single physical page frame to a virtual page.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address to map the page to.
/// @param  paddr   The physical address to map.
/// @param  flags   Attributes for the mapped page.
///
/// @warning    Only used for bootstrapping the initial kernel page table as it does only work with
///             the 1 GB identity mapped bootloader page table.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_map_raw(pt_t pt4, u64 vaddr, u64 paddr, u64 flags) {

    pt_t pt3 = 0;
    pt_t pt2 = 0;
    pt_t pt1 = 0;

    pte_t *pt4_entry;
    pte_t *pt3_entry;
    pte_t *pt2_entry;
    pte_t *pt1_entry;

    // check if all page tables are allocated

    pt4_entry = &pt4[INDEX_PT4(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt4_entry, PAGE_PRESENT)) 
        *pt4_entry = pmem_alloc_raw(1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt3 = (pt_t)P2V(ADDRESS(*pt4_entry));

    pt3_entry = &pt3[INDEX_PT3(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt3_entry, PAGE_PRESENT)) 
        *pt3_entry = pmem_alloc_raw(1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt2 = (pt_t)P2V(ADDRESS(*pt3_entry));

    pt2_entry = &pt2[INDEX_PT2(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt2_entry, PAGE_PRESENT)) 
        *pt2_entry = pmem_alloc_raw(1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt1 = (pt_t)P2V(ADDRESS(*pt2_entry));

    // check if mapping already exists

    pt1_entry = &pt1[INDEX_PT1(vaddr)];
    if (GET_FLAG(*pt1_entry, PAGE_PRESENT)) 
        panic("Virtual address already allocated: %x\n", vaddr);

    pt1[INDEX_PT1(vaddr)] = paddr | flags | PAGE_PRESENT;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Maps a single physical page frame to a virtual page.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address to map the page to.
/// @param  paddr   The physical address to map.
/// @param  flags   Attributes for the mapped page.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_map(pt_t pt4, u64 vaddr, u64 paddr, u64 flags) {

    pt_t pt3 = 0;
    pt_t pt2 = 0;
    pt_t pt1 = 0;

    pte_t *pt4_entry;
    pte_t *pt3_entry;
    pte_t *pt2_entry;
    pte_t *pt1_entry;

    // check if all page tables are allocated

    pt4_entry = &pt4[INDEX_PT4(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt4_entry, PAGE_PRESENT)) 
        *pt4_entry = pmem_alloc_clean(pt4, 1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt3 = (pt_t)P2V(ADDRESS(*pt4_entry));

    pt3_entry = &pt3[INDEX_PT3(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt3_entry, PAGE_PRESENT)) 
        *pt3_entry = pmem_alloc_clean(pt4, 1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt2 = (pt_t)P2V(ADDRESS(*pt3_entry));

    pt2_entry = &pt2[INDEX_PT2(vaddr)];
    // create a new page table if not present
    if (!GET_FLAG(*pt2_entry, PAGE_PRESENT)) 
        *pt2_entry = pmem_alloc_clean(pt4, 1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pt1 = (pt_t)P2V(ADDRESS(*pt2_entry));

    // check if mapping already exists

    pt1_entry = &pt1[INDEX_PT1(vaddr)];
    if (GET_FLAG(*pt1_entry, PAGE_PRESENT)) 
        panic("Virtual address already allocated: %x\n", vaddr);

    pt1[INDEX_PT1(vaddr)] = paddr | flags | PAGE_PRESENT;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Unmaps a single page.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address of the page to unmap.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_unmap(pt_t pt4, u64 vaddr) {

    pt_t pt3 = 0;
    pt_t pt2 = 0;
    pt_t pt1 = 0;

    pte_t *pt4_entry;
    pte_t *pt3_entry;
    pte_t *pt2_entry;
    pte_t *pt1_entry;

    pt4_entry = &pt4[INDEX_PT4(vaddr)];
    if (!GET_FLAG(*pt4_entry, PAGE_PRESENT)) goto not_mapped;

    pt3 = (pt_t)P2V(ADDRESS(*pt4_entry));
    pt3_entry = &pt3[INDEX_PT3(vaddr)];
    if (!GET_FLAG(*pt3_entry, PAGE_PRESENT)) goto not_mapped;

    pt2 = (pt_t)P2V(ADDRESS(*pt3_entry));
    pt2_entry = &pt2[INDEX_PT2(vaddr)];
    if (!GET_FLAG(*pt2_entry, PAGE_PRESENT)) goto not_mapped;

    pt1 = (pt_t)P2V(ADDRESS(*pt2_entry));
    pt1_entry = &pt1[INDEX_PT1(vaddr)];
    if (!GET_FLAG(*pt1_entry, PAGE_PRESENT)) goto not_mapped;

    *pt1_entry = 0;
    return;

not_mapped:
    panic("Virtual address has not been allocated yet: %x\n", vaddr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Maps a physical memory region to a virtual address.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address to map the region to.
/// @param  paddr   The physical address of the region to map.
/// @param  flags   Attributes for the mapped region.
/// @param  blocks  The size of the region in pages.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_map_region(pt_t pt4, u64 vaddr, u64 paddr, u64 flags, u64 blocks) {

    for (u64 block = 0; block < blocks; block++) {
        vmem_map(pt4, vaddr + block * PAGE_SIZE, paddr + block * PAGE_SIZE, flags);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Maps a physical memory region to a virtual address.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address to map the region to.
/// @param  paddr   The physical address of the region to map.
/// @param  flags   Attributes for the mapped region.
/// @param  blocks  The size of the region in pages.
///
/// @warning    Only used for bootstrapping the initial kernel page table as it does only work with
///             the 1 GB identity mapped bootloader page table.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_map_region_raw(pt_t pt4, u64 vaddr, u64 paddr, u64 flags, u64 blocks) {

    for (u64 block = 0; block < blocks; block++) {
        vmem_map_raw(pt4, vaddr + block * PAGE_SIZE, paddr + block * PAGE_SIZE, flags);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Unmaps a region.
///
/// @param  pt4     A pointer to the 4th level page table to use.
/// @param  vaddr   The virtual address of the region to unmap.
/// @param  blocks  The size of the region in pages.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_unmap_region(pt_t pt4,  u64 vaddr, u64 blocks) {

    for (u64 block = 0; block < blocks; block++) {
        vmem_unmap(pt4, vaddr + block * PAGE_SIZE);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an address space for a process.
///
/// @returns    A pointer to the new 4th level page table.
///
/// Will copy the kernel map (0xc0000000 - 0xc01xxxxx) into the new page tables.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 vmem_create_address_space(void) {

    pt_t pt4 = (pt_t)P2V(pmem_alloc_clean(kernel_pt4, 1));
    pt_t pt3 = (pt_t)P2V(pmem_alloc_clean(kernel_pt4, 1));

    pt4[0] = (pte_t)V2P((u64)pt3 | PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    u64 index = INDEX_PT3(bootinfo->kernel_map.virt);

    /// copy the kernel map 3rd level page table entry from kernel_pt3 to the newly created pt3
    pt3[index] = ((pt_t)P2V(ADDRESS(*kernel_pt4)))[index];

    return (u64)pt4;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Virtual Memory Manager.
///////////////////////////////////////////////////////////////////////////////////////////////////

void vmem_init(void) {


    tty_puts(WHITE_ON_BLACK, "Setting up VMEM...");

    kernel_pt4 = (pt_t)P2V(pmem_alloc_raw(1));

    // mapping for kernel
    vmem_map_region_raw(
            kernel_pt4,
            bootinfo->kernel_map.virt, 
            0, 
            PAGE_WRITE | PAGE_GLOBAL, 
            kernel_region_end);

    // mapping for VGA (0xa0000 - 0xb8fff)
    u64 vga_size = page_round_up(vga_range.end - vga_range.base);
    vmem_map_region_raw(
            kernel_pt4,
            vga_range.base + bootinfo->kernel_map.virt, 
            vga_range.base, 
            PAGE_WRITE, 
            vga_size);

    // mapping for pmem bitmap and page tables
    vmem_map_region_raw(
            kernel_pt4,
            (u64)bitmap, 
            V2P(bitmap), 
            PAGE_WRITE, 
            page_round_up(bitmap_bit_size / PAGE_SIZE) + blocks_allocated);


    x86_load_pt4((pt_t)V2P(kernel_pt4));

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}
