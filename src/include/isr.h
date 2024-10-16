///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains type definitions for all things related to the Interrupt Service Routines.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define IST0    0


/// @brief  Structure describing the general registers in a trapframe.
typedef struct PACKED GeneralRegisters {
   u64 r15;
   u64 r14;
   u64 r13;
   u64 r12;
   u64 r11;
   u64 r10;
   u64 r9;
   u64 r8;
   u64 rdi;
   u64 rsi;
   u64 rdx;
   u64 rcx;
   u64 rbx;
   u64 rax;
} gen_regs_t;

/// @brief  Structure describing the control registers in a trapframe.
typedef struct PACKED ControlRegisters {
    u64 cr4;
    u64 cr3;
    u64 cr2;
    u64 cr0;
} ctrl_regs_t;

/// @brief  The trapframe saving a process state.
typedef struct PACKED InterruptArgs {
    u64 ret;
    ctrl_regs_t control_regs;
    gen_regs_t general_regs;
    u64 int_vec;
    u64 err_code;
    u64 rip;
    u64 cs;
    u64 flags;
    u64 rsp;
    u64 ds;
} int_args_t;

/// @brief  Funtion type of an Interrupt Service Routine.
typedef void (*isr_t)(int_args_t *);


extern void isr_ret(void);

void isr_init(void);
