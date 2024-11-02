#include "gdt.h"

// Define an array for the GDT entries and a pointer structure for the GDT.
struct gdt_entry_struct gdt_entries[5]; // The GDT has 6 entries
                                        // Pointer structure to load the GDT

void setGdtGate(uint64_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[num].base_low = (base & 0xFFFF);        // Set the lower 16 bits of the base address
    gdt_entries[num].base_middle = (base >> 16) & 0xFF; // Set the middle 8 bits of the base
    gdt_entries[num].base_high = (base >> 24) & 0xFF;   // Set the high 8 bits of the base
    gdt_entries[num].limit = (limit & 0xFFFF);          // Set the lower 16 bits of the limit
    gdt_entries[num].flags = (limit >> 16) & 0x0F;      // Set the upper 4 bits of the limit
    gdt_entries[num].flags |= (gran & 0xF0);            // Add granularity and size flags
    gdt_entries[num].access = access;                   // Set access flags (e.g., readable, executable, ring level)

    // printf("GDT ENTRY %d\n", num);

    // printf("Base_low 0x%lx\n", gdt_entries[num].base_low);
    // printf("Base_middle 0x%lx\n", gdt_entries[num].base_middle);
    // printf("Base_high 0x%lx\n", gdt_entries[num].base_high);
    // printf("limit 0x%lx\n", gdt_entries[num].limit);
    // printf("Flags %lx\n", gdt_entries[num].flags);
    // printf("Access %lx\n", gdt_entries[num].access);
}

void gdt_flush()
{
    struct gdt_ptr_struct gdt_ptr;
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint64_t)&gdt_entries;

    asm volatile("cli");
    __asm__ __volatile__("lgdt (%0)" : : "r"((uint64_t)&gdt_ptr));
}

void gdt_init()
{

    setGdtGate(0, 0, 0, 0, 0);

    // Kernel Code Segment
    setGdtGate(1, 0, 0xFFFFFFFFFFFFFFFF, GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_EXECUTABLE | GDT_READ_WRITE,
               (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F);

    // Kernel Data Segment
    setGdtGate(2, 0, 0xFFFFFFFFFFFFFFFF, GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_READ_WRITE,
               (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F);

    // User Code Segment
    setGdtGate(3, 0, 0xFFFFFFFFFFFFFFFF, GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_DPL_USER | GDT_EXECUTABLE | GDT_READ_WRITE,
               (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F);

    // User Data Segment
    setGdtGate(4, 0, 0xFFFFFFFFFFFFFFFF, GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_DPL_USER | GDT_READ_WRITE,
               (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F);

    asm volatile("cli");

    gdt_flush(); // Load the new GDT with inline assembly

    // unsigned short cs, ds, es, fs, gs, ss;
    // __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    // __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));
    // __asm__ __volatile__("mov %%es, %0" : "=r"(es));
    // __asm__ __volatile__("mov %%fs, %0" : "=r"(fs));
    // __asm__ __volatile__("mov %%gs, %0" : "=r"(gs));
    // __asm__ __volatile__("mov %%ss, %0" : "=r"(ss));

    // printf("CS: 0x%x\n", cs);
    // printf("DS: 0x%x\n", ds);
    // printf("ES: 0x%x\n", es);
    // printf("FS: 0x%x\n", fs);
    // printf("GS: 0x%x\n", gs);
    // printf("SS: 0x%x\n", ss);
}
