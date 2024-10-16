///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains type definitions for all things related to the file systems.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <ata.h>


/// @brief  Structure describing a MBR partition.
typedef struct PACKED Partition {
    u8  attr;
    u8  c_start;
    u8  h_start;
    u8  s_start;
    u8  type;
    u8  c_end;
    u8  h_end;
    u8  s_end;
    u32 lba_start;
    u32 num_sectors;
} partition_t;


/// @brief  Structure describing a file system.
typedef struct FS {
    ata_t drive;
    u8 partition;
    u64 partition_start_lba;
} fs_t;


/// @brief  Structure describing a file.
typedef struct File {
    char name[8];
    char ext[8];
	u8 attr;

    u8 create_100ms; 
	u16 create_time;
	u16 create_date;
	u16 access_date;

	u16 modified_time;
	u16 modified_date;

    u32 filesize;
    u8 *data;
} file_t;
