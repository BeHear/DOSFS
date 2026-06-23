; DanyaOS Microkernel - Kernel Entry Point (Multiboot)

[BITS 32]

[SECTION .text]
[GLOBAL _start]
[EXTERN kernel_main]

; Multiboot header - must be in first 8KB
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

_start:
    mov [mb_magic], eax
    mov [mb_info], ebx
    mov esp, 0x90000
    push dword [mb_info]
    push dword [mb_magic]
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang

[SECTION .data]
mb_magic: dd 0
mb_info:  dd 0
