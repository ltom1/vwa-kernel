///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Header file for the bump allocator.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <alloc.h>


/// @brief  Bump Allocator class.
typedef struct BumpAllocator {
    allocator_t allocator;
    u64 top;
} bump_allocator_t;

#define CREATE_BUMP_ALLOCATOR(blocks) (bump_allocator_t) { \
        (allocator_t) { \
            P2V(pmem_alloc_clean(kernel_pt4, blocks)), \
            blocks, \
            blocks * PAGE_SIZE, \
            bump_alloc, \
            bump_free, \
            0 \
        }, 0 \
    }; \


u64 bump_alloc(bump_allocator_t *self, u64 n_bytes);
void bump_free(bump_allocator_t *self, u64 n_bytes);
