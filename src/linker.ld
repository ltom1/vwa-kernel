OUTPUT_FORMAT("elf64-x86-64")
KERNEL_ENTRY_VADDR = 0xc0010000;

SECTIONS
{
    . = KERNEL_ENTRY_VADDR;
    .entry          :   {   *(.entry)       }
    .text           :   {   *(.text)        }
    .data           :   {   *(.data)        }
    .rodata         :   {   *(.rodata)      }
    .bss            :   {   *(.bss)         }
}
