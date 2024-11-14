


global context_switch

context_switch:
    push rbx
    push rbp
    push r15
    push r14
    push r13
    push r12

    mov qword [rdi], rsp
    mov rsp, qword [rsi]

    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
    pop rbx

    mov rax, rdi         ; Return old_context pointer
    ret
