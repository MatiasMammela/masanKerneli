#ifndef VMM_H
#define VMM_H

#include "lib.h"
#include "../limine/limine.h"
#define PAGE_SIZE 4096
#define ADDRESS_MASK ((uint64_t)0x000FFFFFFFFFF000)

#define VMM_BLOCK_NULL 0
#define VMM_BLOCK_WRITE (1 << 0)
#define VMM_BLOCK_EXEC (1 << 1)
#define VMM_BLOCK_USER (1 << 2)
#define VMM_TABLE_ENTRY_READ_WRITE (1 << 1)
#define VMM_TABLE_ENTRY_PRESENT (1 << 0)

#define VADDR_TO_INDEX(VADDR, LEVEL) ((((uintptr_t)(VADDR)) >> ((LEVEL) * 9 + 3)) & 0x1FF)

#define ALIGN_TO_PAGE(addr) (((uintptr_t)(addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PTE_BIT_PRESENT 1

struct vmm_block
{
    uintptr_t base;
    size_t length;
    size_t flags;
    struct vmm_block *next;
};
struct addrspace
{
    uint64_t *pml4;
    struct vmm_block *block_head;
};
extern void *vmm_alloc(size_t size, uint8_t flags, struct addrspace *addrspace);
extern void test_vmm_alloc();
extern void vmm_init();
extern void vmm_init_blocks();
extern struct addrspace *create_addrspace();
extern struct addrspace kernel_addrspace;
#endif // VMM_H

// The 4 levels of page directories/tables are:

//     the Page-Map Level-4 Table (PML4),
//     the Page-Directory Pointer Table (PDPR),
//     the Page-Directory Table (PD),
//     and the Page Table (PT).
