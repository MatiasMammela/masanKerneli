#include "gdt.h"
descriptor_t gdt_table[ENTRY_AMOUNT];
void gdt_load_table()
{
    ptr_struct_t gdt_ptr;
    gdt_ptr.limit = (sizeof(descriptor_t) * ENTRY_AMOUNT) - 1;
    gdt_ptr.base = (uint64_t)&gdt_table;
    __asm__ __volatile__(
        "lgdt (%0)"
        :
        : "r"(&gdt_ptr)
        : "memory");
}

// NULL descriptor
void gdt_construct_table()
{
    // NULL descriptor
    gdt_table[0] = (descriptor_t){0, 0, 0, 0, 0, 0};

    // Kernel code segment (64-bit)
    gdt_table[1] = (descriptor_t){0, 0, 0, 0x9A, 0x20, 0};

    // Kernel data segment
    gdt_table[2] = (descriptor_t){0, 0, 0, 0x92, 0x0, 0};

    // User code segment (64-bit)
    gdt_table[3] = (descriptor_t){0, 0, 0, 0xFA, 0x20, 0};

    // User data segment
    gdt_table[4] = (descriptor_t){0, 0, 0, 0xF2, 0x0, 0};
}

void gdt_init()
{
    gdt_construct_table();
    asm volatile("cli");
    gdt_load_table();
}