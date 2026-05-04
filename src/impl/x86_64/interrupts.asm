extern syscall_handler
global _sys_call_handler

_sys_call_handler:
    ; SAVE ALL REGISTERS
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; SHUFFLE REGISTERS FOR C ABI (4 Arguments)
    
    mov rcx, rdx    ; Move Arg3 (Size) to 4th param
    mov rdx, rsi    ; Move Arg2 (Buffer/Mode) to 3rd param
    mov rsi, rdi    ; Move Arg1 (FD/Path) to 2nd param
    mov rdi, rax    ; Move Syscall ID to 1st param

    ; CALL THE C LOGIC
    call syscall_handler

    ; PRESERVE THE RETURN VALUE
    mov [rsp + 112], rax

    ; RESTORE ALL REGISTERS (Reverse Order)
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax         

    iretq