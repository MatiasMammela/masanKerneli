#include "pmm.h"
#include "limineRequest.h"
#include "hhdm.h"
static uintptr_t free_block_ptr = 0;

void pmm_init()
{

    struct limine_memmap_entry **entries = r_memmap->entries;
    hhdm_offset = r_hhdm->offset;
    // printf("%lx", offset);
    long int entry_count = r_memmap->entry_count;

    for (long int i = 0; i < entry_count; i++)
    {
        struct limine_memmap_entry *entry = entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            for (uintptr_t addr = entry->base; addr < entry->base + entry->length; addr += PAGE_SIZE)
            {
                pmm_free(addr);
            }
        }
    }
}

uintptr_t pmm_alloc()
{
    if (!free_block_ptr)
    {
        panic();
    }

    uintptr_t block = free_block_ptr;
    free_block_ptr = *(uintptr_t *)(block + hhdm_offset);
    return block;
}

void pmm_free(uintptr_t addr)
{
    if (addr == 0)
    {
        return;
    }

    uintptr_t *block = (uintptr_t *)(addr + hhdm_offset);
    *block = (uintptr_t)free_block_ptr;

    free_block_ptr = addr;
}
