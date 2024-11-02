#ifndef IDT_H
#define IDT_H
#define IDT_ENTRIES 256
#include "lib.h"
#include "../limine/limine.h"

#define TRAP_GATE 0b1111
#define INTERRUPT_GATE 0b1110

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

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void idt_init();

#endif // IDT_H
