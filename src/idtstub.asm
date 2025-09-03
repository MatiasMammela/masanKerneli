

global load_idt_asm
load_idt_asm:
    lidt    [rdi]   ; load the IDT from the pointer in RDI
    sti
    ret

global idt_stub_table
idt_stub_table:
%assign i 0 
%rep 256
    dq idt_stub_%+i
%assign i i+1
%endrep

extern idt_common_handler

%macro IDT_NOERR 1
idt_stub_%1:
    push qword 0
    push qword %1
    jmp idt_common_entry
%endmacro

%macro IDT_ERR 1
idt_stub_%1:
    push qword %1
    jmp idt_common_entry
%endmacro

idt_common_entry:
    ; save registers

    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    add rdi, (15*8) 
    call idt_common_handler

    ; restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16           ; pop int_no and err_code
    iretq


%assign i 0
%rep 256
%if i = 8 || (i >= 10 && i <= 14) || i = 17
    IDT_ERR i
%else
    IDT_NOERR i
%endif
%assign i i+1
%endrep