#ifndef GDT_H
#define GDT_H

#include "lib.h"
#include "../limine/limine.h"

#define GDT_VALID_SEGMENT (1 << 7)
#define GDT_DPL_USER (0b11 << 5)
#define GDT_CODE_DATA_SELECTOR (1 << 4)
#define GDT_EXECUTABLE (1 << 3)
#define GDT_READ_WRITE (1 << 1)

#define GDT_PAGE_GRANULITY (1 << 7)
#define GDT_LONG_MODE (1 << 5)

struct gdt_entry_struct
{
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void flush_gdt(void *);
extern void gdt_init();

#endif // GDT_H
