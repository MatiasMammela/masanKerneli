#include "idt.h"

struct idt_descriptor idt_table[256];
extern void (*idt_stub_table[256])();
extern void load_idt_asm(void *);

static inline void load_idt(void)
{
    struct idt_ptr idt_ptr;
    idt_ptr.limit = (uint16_t)(sizeof(idt_table) - 1);
    idt_ptr.base = (uint64_t)&idt_table;
    printf("\nIDT ADDR (base) : %lx", (uint64_t)idt_ptr.base);

    load_idt_asm(&idt_ptr);
}

void idt_set_entry(uint8_t index, uint64_t handler_addr, uint8_t dpl, uint8_t gate, uint8_t ist)
{
    struct idt_descriptor *descriptor = &idt_table[index];
    descriptor->address_low = handler_addr & 0xFFFF;
    descriptor->address_mid = (handler_addr >> 16) & 0xFFFF;
    descriptor->address_high = (handler_addr >> 32) & 0xFFFFFFFF;
    descriptor->selector = CODE_SEGMENT_SELECTOR;
    descriptor->flags = gate | ((dpl & 0b11) << 5) | (1 << 7);
    descriptor->ist = 0;
    descriptor->reserved = 0;
}
void dump_idt_entry(int i)
{
    struct idt_descriptor *e = &idt_table[i];
    uint64_t addr = ((uint64_t)e->address_high << 32) | ((uint64_t)e->address_mid << 16) | (uint64_t)e->address_low;
    printf("IDT[%d]: addr=%lx sel=%x ist=%lx flags=%lx\n",
           i, (unsigned long long)addr, e->selector, e->ist, e->flags);
    printf("stub table: %lx\n", idt_stub_table[i]);
}
extern void int_test_handler(void);
void idt_init()
{
    for (int i = 0; i < 256; i++)
    {
        idt_set_entry(i, (uint64_t)idt_stub_table[i], 0, INTERRUPT_GATE, 0);
        // dump_idt_entry(i);
    }
    load_idt();
}
void idt_common_handler(void *stack_ptr)
{
    printf("Jee\n");
}