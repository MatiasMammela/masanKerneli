#ifndef GDT_H
#define GDT_H

#include "lib.h"
#include "../limine/limine.h"

// Flags to set up GDT entries:
#define GDT_VALID_SEGMENT (1 << 7)      // Segment is valid
#define GDT_DPL_USER (0b11 << 5)        // Descriptor Privilege Level for user (ring 3)
#define GDT_CODE_DATA_SELECTOR (1 << 4) // Descriptor is code/data (not system)
#define GDT_EXECUTABLE (1 << 3)         // Code segment is executable
#define GDT_READ_WRITE (1 << 1)         // Readable for code, writable for data

// Additional flags for memory granularity and 64-bit long mode
#define GDT_PAGE_GRANULITY (1 << 7) // Granularity flag (1 = page, 0 = byte)
#define GDT_LONG_MODE (1 << 5)      // Enables 64-bit code segments

// Structure defining a GDT entry
struct gdt_entry_struct
{
    uint16_t limit;      // Segment limit (lower 16 bits)
    uint16_t base_low;   // Base address (lower 16 bits)
    uint8_t base_middle; // Base address (next 8 bits)
    uint8_t access;      // Access flags
    uint8_t flags;       // Flags (granularity, long mode, etc.)
    uint8_t base_high;   // Base address (final 8 bits)
} __attribute__((packed));

// Pointer structure for the GDT
struct gdt_ptr_struct
{
    uint16_t limit; // Limit of the GDT (size - 1)
    uint64_t base;  // Base address of the GDT in memory
} __attribute__((packed));

// Function prototypes
extern void flush_gdt(void *); // Function to load new GDT
extern void gdt_init();        // Function to initialize GDT

#endif // GDT_H
