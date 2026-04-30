extern syscall_handler
global _sys_call_handler

_sys_call_handler:
    ; 1. Save state (so user app doesn't lose its variables)
    push rdi
    push rsi
    push rdx
    push rcx
    push rax

    ; 2. SHUFFLE REGISTERS FOR C
    ; User App gave us: ID in RAX, Char in RDI
    ; C function wants: ID in RDI, Char in RSI
    
    mov rsi, rdi    ; Move Char (arg1) into RSI
    mov rdi, rax    ; Move Syscall ID into RDI

    ; 3. Call the C logic
    call syscall_handler

    ; 4. Restore state
    pop rax
    pop rcx
    pop rdx
    pop rsi
    pop rdi

    iretq