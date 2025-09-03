#pragma once
#include "lib.h"
#include "list.h"
#include "logger.h"
#include "heap.h"
#include "hhdm.h"

#define VADDR_TO_INDEX(VADDR, LEVEL) ((((uintptr_t)(VADDR)) >> ((LEVEL) * 9 + 3)) & 0x1FF)
#define VMM_TABLE_ENTRY_PRESENT (1 << 0)
#define VMM_TABLE_ENTRY_READ_WRITE (1 << 1)
#define ADDRESS_MASK ((uint64_t)0x000FFFFFFFFFF000)
#define ALIGN_TO_PAGE(addr) (((uintptr_t)(addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
struct vmm_block
{
    list_elem_t elem;
    uintptr_t base;
    size_t length;
    size_t flags;
};

typedef struct addrspace
{
    uint64_t *pml4;
    linked_list_t vmm_map;
} addrspace;

extern addrspace *kernel_addrspace;
uintptr_t vmm_alloc(size_t size, uint8_t flags, struct addrspace *addrspace);
struct addrspace *new_addrspace();
void vmm_init();

extern __attribute__((noreturn)) void panic();
extern void hcf(void);
