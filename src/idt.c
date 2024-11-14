#include "idt.h"
struct idt_descriptor idt[IDT_ENTRIES];

void load_idt(void)
{
    struct idt_ptr idt_ptr;
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;

    __asm__ __volatile__("lidt (%0)" : : "r"((uint64_t)&idt_ptr));
    asm volatile("sti");
}
void general_exception_handler()
{
    printf("General exception occurred\n");
    // You can also print error code or additional context here
    while (1)
        ; // Halt the system
}
void page_fault_handler()
{
    printf("Page fault exception occurred");
    uintptr_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));
    printf("\n%lx", faulting_address);
    while (1)
        ;
}

// Divide-by-zero handler
void divide_by_zero_handler()
{
    printf("Divide by zero exception occurred");
    while (1)
        ; // Halt the system
}

void set_idt_entry(uint8_t index, void *handler, uint8_t dpl)
{
    uint64_t handler_addr = (uint64_t)handler;

    struct idt_descriptor *entry = &idt[index];
    entry->address_low = handler_addr & 0xFFFF;
    entry->address_mid = (handler_addr >> 16) & 0xFFFF;
    entry->address_high = (handler_addr >> 32) & 0xFFFFFFFF;

    entry->selector = 0x8; // KERNEL CODE SELECTOR

    entry->flags = INTERRUPT_GATE | ((dpl & 0b11) << 5) | (1 << 7);

    entry->ist = 0;

    entry->reserved = 0;
}

void idt_init()
{
    set_idt_entry(0, divide_by_zero_handler, 0);
    set_idt_entry(14, page_fault_handler, 0);
    for (uint8_t i = 1; i < 32; i++)
    {
        if (i != 0 && i != 14)
        { // Skip already defined handlers
            set_idt_entry(i, general_exception_handler, 0);
        }
    }

    load_idt();
}
