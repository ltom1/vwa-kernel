///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Header file containing all things related to parsing ELF64 files.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <vfs.h>
#include <proc.h>


#define ELF_MAGIC0          0x7f
#define ELF_MAGIC1          'E'
#define ELF_MAGIC2          'L'
#define ELF_MAGIC3          'F'

#define ELF_IDENT_CLASS     4
#define ELF_CLASS64         2

#define ELF_IDENT_ORDER     5
#define ELF_ORDER_LE        1

#define ELF_IDENT_VERSION   5
#define ELF_VERSION1        1

#define ELF_TYPE_NONE       0
#define ELF_TYPE_REL        1
#define ELF_TYPE_EXE        2

#define ELF_ARCH_X86_64     0x3e


/// @brief  Enum of program segment types.
typedef enum SegType {
  SEG_NULL,
  SEG_LOAD,
  SEG_DYNAMIC,
  SEG_INTERP,
  SEG_NOTE,
  SEG_SHLIB,
  SEG_PHDR,
  SEG_LOPROC = 0x70000000,
  SEG_HIPROC = 0x7FFFFFFF
} seg_type_t;


/// @brief  Structure of an ELF64 header.
typedef struct PACKED ElfHeader64 {
	u8  ident[16];
	u16 type;
	u16 arch;
	u32 version;
	u64 code_entry;
	u64 pht_off;
	u64 sht_off;
	u32 flags;
	u16 size;
	u16 pht_entry_size;
	u16 pht_entries;
	u16 sht_entry_size;
	u16 sht_entries;
	u16 string_tbl_header_index;
} elf_header_64_t;


/// @brief  Structure of an ELF64 program header.
typedef struct PACKED ElfPHeader64 {
	u32 type;
	u32 flags;
	u64 off;
	u64 vaddr;
	u64 paddr;
	u64 size_file;
	u64 size_mem;
	u64 align;
} elf_pheader_64_t;


void elf64_check(file_t *f);
void elf64_extract(pcb_t *proc);
