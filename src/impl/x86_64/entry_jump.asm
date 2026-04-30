; src/impl/x86_64/entry_jump.asm
; A simple function to jump to a given 64-bit address.
; This will execute in the current privilege level (kernel mode in this context).

section .text
    global jump_to_entry

jump_to_entry:
    ; The entry_point is passed in RDI (first argument in System V AMD64 ABI)
    mov rax, rdi
    jmp rax