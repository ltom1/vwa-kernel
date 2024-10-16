///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions and structures related to processes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <paging.h>
#include <isr.h>
#include <vfs.h>
#include <alloc.h>


#define MAX_NAME    16


/// @brief  State of a process.
typedef enum ProcState { 
    UNINITIALIZED, 
    SLEEPING, 
    RUNNABLE, 
    RUNNING, 
    BLOCKED 
} proc_state_t;


/// @brief  Structure containing information about a process.
typedef struct PCB {
    struct PCB      *prev;
    u8              pid;
    char            name[MAX_NAME];
    file_t          *file;
    pt_t            pt4;

    proc_state_t    state;
    int_args_t      *ctx;
    u64             kstack;
    u64             cpu_ms;
    
    struct PCB      *parent;
    struct PCB      *next;
} pcb_t;


void proc_init(void);
pcb_t *proc_create(allocator_t *allocator, pcb_t *parent, const char *name, u8 length, file_t *f);

void switch_ctx(pcb_t *new);

extern pcb_t *cur_proc;
