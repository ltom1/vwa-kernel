;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @file
;;; @brief  Contains ISR stubs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

extern  isr_handler

global  isr_ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @brief  Common ISR preparation point.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

isr_common:
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
    push    r8
    push    r9 
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15

    mov     rax, cr0
    push    rax
    mov     rax, cr2
    push    rax
    mov     rax, cr3
    push    rax
    mov     rax, cr4
    push    rax

    mov     rdi, rsp
    sub     rdi, 8  ; return address of isr_ret will be placed on top
    call    isr_handler

isr_ret:
    pop     rax
    pop     rax
    pop     rax
    pop     rax
    
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9 
    pop     r8
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx

;    mov     ax, 0x23
;    mov     ds, ax
;    mov     es, ax
;    mov     fs, ax
;    mov     gs, ax
    pop     rax

    ; remove int_vec, error code
    add     rsp, 16

    iretq


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @brief  Interrupt stub for syscalls (int 0x80).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global  isr_syscall
isr_syscall:
    push    0   ; no error code on stack yet
    push    128
    jmp     isr_common


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @brief  Interrupt stubs for exceptions and IRQs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%macro isr 1
global  isr%+%1
isr%+%1:
    %if %1 != 8 && !(%1 >= 10 && %1 <= 14) && %1 != 17 && %1 != 30
    push    0   ; no error code on stack yet
    %endif
    push    %1
    jmp     isr_common
%endmacro


%assign i 0
%rep    48
isr i
%assign i i+1
%endrep


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @brief  Table of all exception/IRQ stubs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global  isr_stub_table
isr_stub_table:
%assign i 0 
%rep    48 
dq  isr%+i
%assign i i+1 
%endrep
