///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains type definitions for all things related to the bootinfo struct.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <mmap.h>
#include <vfs.h>


/// @brief  Structure describing a virtual-to-physical address mapping.
typedef struct PACKED Mapping {
    u64     phys;
    u64     virt;
    u64     size;
} mapping_t;

/// @brief  Structure passed to the kernel by the bootloader.
typedef struct PACKED BootInfo {

    u8              boot_drive;

    u32             kernel_filesize;

    u64             kernel_load_addr;
    u64             kernel_load_vaddr;

    mapping_t       identity_map;
    mapping_t       kernel_map;

    u32             vbr_addr;
    u8              boot_partition;
    partition_t     partitions[4];

    u32             num_regions;
    region_t    regions[];

} bootinfo_t;

/// @brief  Global var holding a pointer to the bootinfo struct.
extern bootinfo_t *bootinfo;
