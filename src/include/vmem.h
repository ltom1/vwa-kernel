///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Header file for the Virtual Memory Manager.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <paging.h>


#define INDEX_PT1(vaddr)    ((vaddr >> 12) & 0x1ff)
#define INDEX_PT2(vaddr)    ((vaddr >> 21) & 0x1ff)
#define INDEX_PT3(vaddr)    ((vaddr >> 30) & 0x1ff)
#define INDEX_PT4(vaddr)    ((vaddr >> 39) & 0x1ff)


extern pt_t kernel_pt4;

void vmem_map(pt_t pt4, u64 vaddr, u64 paddr, u64 flags);
void vmem_map_raw(pt_t pt4, u64 vaddr, u64 paddr, u64 flags);

void vmem_map_region(pt_t pt4, u64 vaddr, u64 paddr, u64 flags, u64 blocks);
void vmem_map_region_raw(pt_t pt4, u64 vaddr, u64 paddr, u64 flags, u64 blocks);

void vmem_unmap(pt_t pt4, u64 vaddr);
void vmem_unmap_region(pt_t pt4, u64 vaddr, u64 blocks);
u64 vmem_create_address_space(void);
void vmem_init(void);
