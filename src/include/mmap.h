///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains type definitions for all things related to the memory map.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


/// @brief  Structure of a Memory Region Entry.
typedef struct PACKED Region {
    u64 base;
    u64 length;
    u32 type;
    u32 apci;
} region_t;

/// @brief  Structure describing a memory range from base to end.
typedef struct Range {
    u64 base;
    u64 end;
} range_t;

/// @brief  Enum describing the type of a memory region.
typedef enum {
    FREE        = 1,
    RESERVED    = 2,
    RECLAIMABLE = 3,
    NVS         = 4,
    BAD_MEM     = 5
} REGION_TYPE;
