///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  This file contains the kernel's main function.
///
/// It is the central point of the kernel.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <paging.h>
#include <bootinfo.h>
#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <isr.h>
#include <err.h>
#include <irq.h>
#include <kbd.h>
#include <pit.h>
#include <pmem.h>
#include <vmem.h>
#include <ata.h>
#include <alloc.h>
#include <buddy.h>
#include <bump.h>
#include <vfs.h>
#include <fat32.h>
#include <x86.h>
#include <tty.h>
#include <proc.h>
#include <elf64.h>


/// @brief  Global variable holding a pointer to the bootinfo struct.
bootinfo_t *bootinfo;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Kernel's main entry point.
/// 
/// The first part of the kernel written in C.
/// Initializes the operating system.
///////////////////////////////////////////////////////////////////////////////////////////////////

NORETURN void kmain(bootinfo_t *info) {

    bootinfo = info;
    pv_base = bootinfo->kernel_map.virt;

    tty_init();
    tty_putf(
            MIX(GREEN, BLACK), 
            "\n\nKernel filesize: %ukb\n\n", 
            bootinfo->kernel_filesize / 1000);

    x86_cli();
    gdt_init();
//    ASM("mov    ax, 0x28");
//    ASM("ltr    ax");
//    while(1);
    idt_init();
    isr_init();
    pic_init();
    kbd_init();
    pit_init(1000);
    proc_init();
    pmem_init();
    vmem_init();
    ata_init();

    buddy_allocator_t heap = CREATE_BUDDY_ALLOCATOR(20);
    heap.allocator.init(&heap);
    
    fat32_t *fs = fat32_init(
            (allocator_t*)&heap, 
            boot_drive, bootinfo->boot_partition, 
            (vbr_t*)P2V(bootinfo->vbr_addr)
            );

    file_t *f = fat32_load_file(fs, (allocator_t*)&heap, "/PROG/HELLO.ELF");
    pcb_t *proc1 = proc_create((allocator_t*)&heap, 0, "proc1", 5, f);
    x86_sti();
    while(1);
    switch_ctx(proc1);

    x86_sti();
    for (;;);
}
