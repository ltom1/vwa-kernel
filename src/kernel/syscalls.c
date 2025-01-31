///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  Contains system calls.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <err.h>
#include <isr.h>
#include <syscalls.h>
#include <x86.h>
#include <tty.h>
#include <proc.h>


/// @brief  Array of handlers for each syscall.
isr_t syscalls[MAX_SYSCALL];


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes system calls.
///////////////////////////////////////////////////////////////////////////////////////////////////

void syscall_init(void) {
     
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Handles system calls.
///
/// @param  args    A pointer to the current trapframe.
///////////////////////////////////////////////////////////////////////////////////////////////////

void syscall_handler(int_args_t *args) {

    if (args->general_regs.rax >= MAX_SYSCALL)
            panic("Illegal syscall number");

    syscalls[args->general_regs.rax](args);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Registers a new system call.
///
/// @param  func    The function pointer of the system call handler.
///////////////////////////////////////////////////////////////////////////////////////////////////

void syscall_add(isr_t func) {

}
