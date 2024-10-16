///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for parsing ELF64 headers.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <fat32.h>
#include <vfs.h>
#include <err.h>
#include <tty.h>
#include <x86.h>
#include <elf64.h>
#include <proc.h>
#include <vmem.h>
#include <pmem.h>
#include <tty.h>
#include <paging.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks the integrity of an ELF64 header in a file.
///
/// @param  f   The file to check.
///////////////////////////////////////////////////////////////////////////////////////////////////

void elf64_check(file_t *f) {

    elf_header_64_t *elf = (elf_header_64_t*)f->data;

    if ((elf->ident[0] != ELF_MAGIC0) ||
        (elf->ident[1] != ELF_MAGIC1) ||
        (elf->ident[2] != ELF_MAGIC2) ||
        (elf->ident[3] != ELF_MAGIC3))
        panic("Invalid Magic");
    
    if (elf->ident[ELF_IDENT_CLASS] != ELF_CLASS64)
        panic("Not a 64bit executable");
    
    if (elf->ident[ELF_IDENT_ORDER] != ELF_ORDER_LE)
        panic("Byte order not little endian");

    if (elf->ident[ELF_IDENT_VERSION] != ELF_VERSION1)
        panic("Wrong ELF version");

    if (elf->type != ELF_TYPE_EXE)
        panic("Not an executable");

    if (elf->arch != ELF_ARCH_X86_64)
        panic("Not an x86_64 executable");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Extracts an ELF64 file of a new process onto a new address space.
///
/// @param  proc    The process to extract.
///////////////////////////////////////////////////////////////////////////////////////////////////

void elf64_extract(pcb_t *proc) {

    elf_header_64_t *elf = (elf_header_64_t*)proc->file->data;
    elf_pheader_64_t *pheaders = (elf_pheader_64_t*)(proc->file->data + elf->pht_off);
    
    for (u64 i = 0; i < elf->pht_entries; i++) {

        if (pheaders[i].size_mem != pheaders[i].size_file) 
            panic("Additional memory has to be allocated!\n");
        if (pheaders[i].type != SEG_LOAD) continue;
        
//        tty_putf(WHITE_ON_BLACK, "%u: %x - %x\n", pheaders[i].type, pheaders[i].vaddr, pheaders[i].vaddr + pheaders[i].size_mem);

        
//        tty_putf(WHITE_ON_BLACK, "%x\n", pheaders[i].flags);

        vmem_map_region(
                proc->pt4, 
                pheaders[i].vaddr, 
                V2P(proc->file->data) + pheaders[i].off, 
                PAGE_USER | PAGE_WRITE | PAGE_PRESENT,
                page_round_up(pheaders[i].size_mem)
            );
    }
}
