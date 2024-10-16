///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for managing processes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <gdt.h>
#include <isr.h>
#include <paging.h>
#include <types.h>
#include <proc.h>
#include <alloc.h>
#include <vfs.h>
#include <elf64.h>
#include <utils.h>
#include <err.h>
#include <tty.h>
#include <x86.h>
#include <pmem.h>
#include <vmem.h>
#include <gdt.h>


/// @brief  Global var holding the current process.
pcb_t *cur_proc;

/// @brief  Global var holding the last used process ID.
u8 last_pid = 0;
/// @brief  Global var holding the last created PCB.
pcb_t *last_proc;

/// @brief  PCB of the kernel (used for memory mapping).
pcb_t kernel_proc;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Initializes the cur_proc var.
///////////////////////////////////////////////////////////////////////////////////////////////////

void proc_init(void) {
    cur_proc = &kernel_proc;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Allocates a Process Control Block and a kernel stack.
///
/// @param  allocator   The allocator to use for allocating the PCB and the stack.
/// @param  parent      A pointer to the PCB of the parent process.
/// @param  name        The process name.
/// @param  length      The length of the name (max. 16 chars).
/// @param  elf         The ELF64 header of the process binary.
///
/// @returns    A pointer to the newly created PCB.
///////////////////////////////////////////////////////////////////////////////////////////////////

pcb_t *proc_create(allocator_t *allocator, pcb_t *parent, const char *name, u8 length, 
        file_t *f) {

    elf64_check(f);

    if (length > MAX_NAME)
        panic("Name too long");

    pcb_t *proc = (pcb_t*)allocator->alloc(allocator, sizeof(pcb_t));
    mem_cpy((u8*)proc->name, (u8*)name, length);

    elf_header_64_t *elf = (elf_header_64_t*)f->data;
    proc->file = f;

    proc->pt4 = (pt_t)vmem_create_address_space();


    // todo: proper trapframe filling
    proc->kstack = P2V(pmem_alloc_clean(proc->pt4, 1));
    proc->ctx = (int_args_t*)proc->kstack;
    proc->ctx->cs = USER_CODE | PL_USER;
    proc->ctx->ds = USER_DATA | PL_USER;
    proc->ctx->rip = elf->code_entry;
    proc->ctx->rsp = 0x401000;
    proc->ctx->flags = 0;
    proc->ctx->int_vec = 0;
    proc->ctx->err_code = 0;
    proc->ctx->control_regs.cr0 = 0;
    proc->ctx->control_regs.cr2 = 0;
    proc->ctx->control_regs.cr3 = 0;
    proc->ctx->control_regs.cr4 = 0;
    proc->ctx->general_regs.rax = 0;
    proc->ctx->general_regs.rbx = 0;
    proc->ctx->general_regs.rcx = 0;
    proc->ctx->general_regs.rdx = 0;
    proc->ctx->general_regs.rsi = 0;
    proc->ctx->general_regs.rdi = 0;
    proc->ctx->general_regs.r8 = 0;
    proc->ctx->general_regs.r9 = 0;
    proc->ctx->general_regs.r10 = 0;
    proc->ctx->general_regs.r11 = 0;
    proc->ctx->general_regs.r12 = 0;
    proc->ctx->general_regs.r13 = 0;
    proc->ctx->general_regs.r14 = 0;
    proc->ctx->general_regs.r15 = 0;

    proc->pid = ++last_pid;
    proc->state = UNINITIALIZED;
    proc->cpu_ms = 0;
    proc->parent = 0;

    proc->prev = last_proc;
    if (last_proc != 0)
        last_proc->next = proc;
    last_proc = proc;

    elf64_extract(proc);

    return proc;
}


void switch_ctx(pcb_t *new) {

    x86_load_pt4((pt_t)V2P(new->pt4));
    x86_change_kstack(new->kstack);

    new->ctx->ret = (u64)isr_ret;
    ASM("ret");
}
