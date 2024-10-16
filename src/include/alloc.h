///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Base header file for all allocators.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


/// @brief  Abstract base of an allocator.
typedef struct Allocator {
    u64 base_addr;
    u64 blocks;
    u64 space_left;
    u64 (*alloc)();
    void (*free)();
    void (*init)();
} allocator_t;
