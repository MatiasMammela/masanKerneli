#pragma once
#include "lib.h"
#define TRAP_GATE 0b1111
#define INTERRUPT_GATE 0b1110
#define CODE_SEGMENT_SELECTOR 0x8
// IDT entry structure
struct idt_descriptor
{
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags; // 0-3 TYPE / 4 RESERVED(SET TO 0) / 5 - 6 DPL / 7 PRESENT
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__((packed));

struct register_frame
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcd, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_common_handler(void *stack_ptr);
void idt_init();
void idt_set_entry(uint8_t index, uint64_t handler_addr, uint8_t dpl, uint8_t gate, uint8_t ist);
