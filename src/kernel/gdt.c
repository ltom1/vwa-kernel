///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains GDT related stuff.
///
/// Contains function definitions for loading and editing of the Global Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "paging.h"
#include <types.h>
#include <gdt.h>
#include <tty.h>
#include <x86.h>


/// @brief  The Task State Segment (TSS) as a global var.
///
/// Used for software multitasking. 
/// Contains Stack Pointers.
tss_t tss = {0};
/// @brief  The Global Descriptor Table (GDT) as a global var.
///
/// Used to describe Memory Segments. 
gdt_entry_t gdt[GDT_ENTRIES];
/// @brief  The Global Descriptor Table Register (GDTR) as a global var.
///
/// Used for loading a GDT.
/// Loaded by a lgdt instruction.
static gdtr_t gdtr = {0};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Global Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

void gdt_init(void) {

    tty_puts(WHITE_ON_BLACK, "Setting up GDT...");

    gdtr.base = (u64)&gdt[0];
    // todo: I have no idea why but ltr seems to throw a #GPF without the extra entry 
    gdtr.limit = (u16)sizeof(gdt_entry_t) * (GDT_ENTRIES + 1) - 1; 

    gdt_build();
    gdt_load(&gdtr);

    // initialize Task State Segment
    tss.iopb = sizeof(tss_t);
    tss.rsp0 = 0xc0010000;
    gdt_entry_t tss_entry = 
        gdt_create_entry((u64)&tss, ((u64)&tss) + sizeof(tss_t) - 1, ACCESS_TSS, 0);
    gdt[TSS_DESC / sizeof(gdt_entry_t)] = tss_entry;

    tss_load(TSS_DESC);

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Builds the Global Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

void gdt_build(void) {

    // null descriptor
    gdt[0] = gdt_create_entry(0, 0, 0, 0);

    // kernel mode segments
    gdt[1] = gdt_create_entry(0, -1, ACCESS_KERNEL | ACCESS_CODE, LONG_MODE | LIMIT_IN_PAGES);
    gdt[2] = gdt_create_entry(0, -1, ACCESS_KERNEL | ACCESS_DATA, LONG_MODE | LIMIT_IN_PAGES);

    // user mode segments
    gdt[3] = gdt_create_entry(0, -1, ACCESS_USER | ACCESS_CODE, LONG_MODE | LIMIT_IN_PAGES);
    gdt[4] = gdt_create_entry(0, -1, ACCESS_USER | ACCESS_DATA, LONG_MODE | LIMIT_IN_PAGES);

    // TSS (will be inserted and loaded later)
    gdt[5] = gdt_create_entry(0, 0, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Creates a GDT Entry for a segment.
///
/// @param  base    Segment's base address.
/// @param  limit   Segment's end address.
/// @param  access  Segment's access attributes.
/// @param  flags   Segment's flags.
/// 
/// @return A GDT Entry.
///////////////////////////////////////////////////////////////////////////////////////////////////

gdt_entry_t gdt_create_entry(u64 base, u64 limit, u8 access, u8 flags) {

    gdt_entry_t entry = {0};

    entry.limit_low = limit & 0xFFFF;
    entry.base_low = base & 0xFFFF;
    entry.base_mid = (base >> 16) & 0xFF;
    entry.access = access;
    entry.limit_high = (limit >> 16) & 0xF;
    entry.flags = flags & 0xF;
    entry.base_high = (base >> 24) & 0xFF;

    return entry;
}
