;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @file 
;;; @brief  Kernel entry point.
;;;
;;; The bootloader jumps to this entry point.
;;; Loaded at address 0x10000 (Virtual address 0xc0010000).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bits    64

extern  kmain


section .entry

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; @brief  Kernel entry point.
;;; @todo   Proper initialization.
;;;
;;; The bootloader will jump to this label.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

KERNEL_ENTRY:
    ; todo: initialize stuff
    ; stack is in front of kernel code
    mov     rsp, 0xc000f000
    mov     rbp, rsp
    mov     rdi, rbx

    jmp     kmain
