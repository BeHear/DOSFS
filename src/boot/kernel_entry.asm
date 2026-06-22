; DanyaOS Microkernel - Kernel Entry Point
; Sets up stack and calls kernel_main

[BITS 32]
[EXTERN kernel_main]

[GLOBAL _start]
[SECTION .text]

_start:
    mov esp, 0x90000
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
