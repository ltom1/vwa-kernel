///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains all things related to paging.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define PAGE_SIZE   0x1000


#define PAGE_PRESENT            (1 << 0)
#define PAGE_WRITE              (1 << 1)
#define PAGE_USER               (1 << 2)
#define PAGE_WRITE_THROUGH      (1 << 3)
#define PAGE_CACHE_DISABLED     (1 << 4)
#define PAGE_ACCESSED           (1 << 5)
#define PAGE_DIRTY              (1 << 6)
#define PAGE_HUGE               (1 << 7)
#define PAGE_GLOBAL             (1 << 8)
#define PAGE_ALLOCATED          (1 << 9)

#define GET_FLAG(pte, flag)     ((pte) & (flag))
#define SET_FLAG(pte_ptr, flag) (*(pte_ptr) |= (flag))
#define CLEAR_FLAG(pte_ptr)     (*(pte_ptr) &= ~CLEAR_MASK)
#define ADDRESS(pte)            ((pte) & CLEAR_MASK)

#define CLEAR_MASK              (0xfffffff000)

#define P2V(paddr)      ((u64)(paddr) + (u64)(pv_base))
#define V2P(vaddr)      ((u64)(vaddr) - (u64)(pv_base))


/// @brief  Type of a Page Table Entry (8 bytes).
typedef u64 PTE, pte_t;

/// @brief  Type of a Page Table (Page Table Entry Array).
typedef pte_t* PT;
typedef pte_t* pt_t;

/// @brief  Structure of a Page Table Entry (8 bytes).
///
/// Used for changing flags and options in a PTE.
typedef struct PACKED PTEFlags {
    u64 present                   :1;
    u64 write                     :1;
    u64 user                      :1;
    u64 write_through             :1;
    u64 cache_disabled            :1;
    u64 accessed                  :1;
    u64 dirty                     :1;
    u64 size                      :1;
    u64 global                    :1;
    u64 ignored1                  :3;
    u64 page_num                  :28;
    u64 reserved1                 :12;
    u64 ignored2                  :11;
    u64 execution_disabled        :1;
} pte_flags_t;


extern u64 pv_base;

u64 page_round_up(u64 addr);
u64 page_round_down(u64 addr);
u64 page_base(u64 addr);
