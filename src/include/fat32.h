///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains type definitions for all things related to the FAT32 file system.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <alloc.h>
#include <ata.h>
#include <vfs.h>


#define MAX_FAT_CACHE               2
#define MAX_FAT_CLUSTER_ENTRIES     1024
#define MAX_DIR_CACHE               2
#define MAX_DIR_CLUSTER_ENTRIES     128

#define FAT32_ENTIRE_NAME_SIZE      11
#define FAT32_NAME_SIZE             8
#define FAT32_EXT_SIZE              3

#define FAT32_EOF                   0x0FFFFFF8
#define ATTR_READONLY               0x01
#define ATTR_HIDDEN                 0x02
#define ATTR_SYSTEM                 0x03
#define ATTR_DIR                    0x10
#define ATTR_ARCHIVE                0x20


/// @brief  Structure of a FAT32 Directory Entry.
typedef struct PACKED DirectoryEntry {
	char name[8];
	char ext[3];
	u8 attr;
	u8 reserved;
 
    u8 create_100ms; 
	u16 create_time;
	u16 create_date;
	u16 access_date;
	u16 cluster_high;
 
	u16 modified_time;
	u16 modified_date;
	u16 cluster_low;
	u32 filesize;
} directory_entry_t;

/// @brief  Structure representing a FAT32 file system.
typedef struct FAT32 {
    fs_t fs;
    u64 data_start_lba;
    u64 fat_start_lba;
    u64 root_start_cluster;
    u64 cur_fat_cluster;
    u64 sectors_per_cluster;

    // cache for FAT (MAX_FAT_CACHE clusters)
    u32 fats_loaded[MAX_FAT_CACHE];
    u8 *fats;

    // cache for Directorys (MAX_DIR_CACHE clusters)
    u32 dirs_loaded[MAX_DIR_CACHE];
    u8 *dirs;

    // root directory always cached
    u8 *root_dir;
} fat32_t;

/// @brief  Structure of the VBR of a FAT32 formatted partition.
typedef struct PACKED VBR {

    u8  jmp[3];
    u8  oem_id[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  num_fats;
    u16 max_root_entries;
    u16 num_sectors;
    u8  media_descriptor;

    // IMPORTANT: not used by FAT32
    u16 sectors_per_fat;

    u16 sectors_per_track;
    u16 sectors_per_head;
    u32 hidden_sectors;
    u32 total_sectors;

    // IMPORTANT: used by FAT32
    u32 big_sectors_per_fat;

    u16 flags;
    u16 fs_version;
    u32 root_directory_start;
    u16 fs_info_sector;
    u16 backup_boot_sector;

    u8  reserved[12];

    u8  drive_num;
    u8  reserved_byte;
    u8  signature;
    u32 volume_id;
    u8  volume_label[11];
    u8  system_id[8];

} vbr_t;


fat32_t* fat32_init(allocator_t *allocator, ata_t drive, u8 partition, vbr_t *vbr);
file_t* fat32_load_file(fat32_t *fs, allocator_t *allocator, const char *filename);

void fat32_load_cluster(fat32_t *fs, u8 *dest, u32 cluster);
u32 fat32_load_cluster_chain(fat32_t *fs, u8 *dest, u32 cluster, u32 max_clusters);

u32 fat32_next_cluster(fat32_t *fs, u32 cur_cluster);
u64 fat32_cmp_path(const char *path_input, const char *path_entry);
u8* fat32_cache_dir(fat32_t *fs, u32 dir_cluster);
