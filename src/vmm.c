#include "vmm.h"
#include "hhdm.h"
struct addrspace *kernel_addrspace = NULL;
extern uint8_t kernel_size[];

uint64_t *get_next_table(uint64_t *table, uint64_t lvl_index, uint8_t flags)
{

    if (!(table[lvl_index] & VMM_TABLE_ENTRY_PRESENT))
    {

        uintptr_t new_table = pmm_alloc(PAGE_SIZE);
        memset((void *)(new_table + hhdm_offset), 0, PAGE_SIZE);
        table[lvl_index] = new_table | flags;
    }

    return (uint64_t *)PHYS_TO_VIRT(table[lvl_index] & ADDRESS_MASK);
}
void vmm_map_page(uint64_t *pml4, uintptr_t vaddr, uintptr_t paddr, uint8_t flags)
{
    // printf("[vmm] Mapping single page: vaddr=0x%lx paddr=0x%lx flags=0x%x\n", vaddr, paddr, flags);
    uint64_t lvl4_index = VADDR_TO_INDEX(vaddr, 4);
    uint64_t lvl3_index = VADDR_TO_INDEX(vaddr, 3);
    uint64_t lvl2_index = VADDR_TO_INDEX(vaddr, 2);
    uint64_t lvl1_index = VADDR_TO_INDEX(vaddr, 1);
    uint64_t *lvl3_table = get_next_table(pml4, lvl4_index, flags);
    uint64_t *lvl2_table = get_next_table(lvl3_table, lvl3_index, flags);
    uint64_t *lvl1_table = get_next_table(lvl2_table, lvl2_index, flags);
    lvl1_table[lvl1_index] = paddr | flags;
}

uint64_t vmm_unmap_page(uint64_t *pml4, void *vaddr)
{
    uint64_t lvl4_index = VADDR_TO_INDEX(vaddr, 4);
    uint64_t lvl3_index = VADDR_TO_INDEX(vaddr, 3);
    uint64_t lvl2_index = VADDR_TO_INDEX(vaddr, 2);
    uint64_t lvl1_index = VADDR_TO_INDEX(vaddr, 1);

    uint64_t *lvl3_table = get_next_table(pml4, lvl4_index, 0);
    uint64_t *lvl2_table = get_next_table(lvl3_table, lvl3_index, 0);
    uint64_t *lvl1_table = get_next_table(lvl2_table, lvl2_index, 0);

    uint64_t phys_addr = lvl1_table[lvl1_index] & ADDRESS_MASK;

    lvl1_table[lvl1_index] = 0;
    asm volatile(
        "invlpg (%0)"
        :
        : "r"(phys_addr)
        : "memory");
    return phys_addr;
}
void vmm_free(uintptr_t addr, size_t size, struct addrspace *addrspace)
{
    list_elem_t *prev = addrspace->vmm_map.head;
    list_elem_t *current = addrspace->vmm_map.head->next;
    LIST_LOOP(&addrspace->vmm_map, current)
    {
        struct vmm_block *prev_block = (struct vmm_block *)prev;
        struct vmm_block *current_block = (struct vmm_block *)current;
        struct vmm_block *next_block = (struct vmm_block *)current->next;
        if (current_block->base == addr)
        {

            // printf("We found the block, now free it!\n");
            for (size_t i = 0; i < current_block->length; i += PAGE_SIZE)
            {
                uintptr_t vaddr = (current_block->base + i);
                uintptr_t phys_addr = vmm_unmap_page(addrspace->pml4, (void *)vaddr);

                if (phys_addr != 0)
                {
                    pmm_free(phys_addr, PAGE_SIZE);
                }
            }

            list_delete(&addrspace->vmm_map, current);
            pmm_free((uintptr_t)current_block, PAGE_SIZE);
        }
        prev = current;
    }
}
struct addrspace *new_addrspace()
{
    struct addrspace *new_addrspace = malloc(sizeof(struct addrspace));
    // printf("[vmm] new_addrspace: allocated addrspace! %lx\n", new_addrspace);

    new_addrspace->pml4 = (uint64_t *)PHYS_TO_VIRT((pmm_alloc(PAGE_SIZE)));
    // printf("[vmm] new_addrspace: pml4 allocated at %lx\n", new_addrspace->pml4);

    if (!new_addrspace->pml4)
    {
        printf("[vmm] Error: pml4 allocation failed!\n");
        return NULL;
    }

    memset(new_addrspace->pml4, 0, PAGE_SIZE);

    new_addrspace->vmm_map = LIST_INIT;
    // printf("[vmm] new_addrspace: vmm_map initialized\n");

    struct vmm_block *block_head = (struct vmm_block *)PHYS_TO_VIRT(pmm_alloc(PAGE_SIZE));
    if (!block_head)
    {
        printf("[vmm] Error: block_head allocation failed!\n");
        return NULL;
    }
    // printf("[vmm] new_addrspace: block_head allocated at %lx\n", block_head);

    block_head->base = ALIGN_TO_PAGE(hhdm_offset);
    block_head->length = PAGE_SIZE;
    block_head->flags = VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT;

    struct vmm_block *terminator_block = (struct vmm_block *)PHYS_TO_VIRT(pmm_alloc(PAGE_SIZE));
    if (!terminator_block)
    {
        printf("[vmm] Error: terminator_block allocation failed!\n");
        return NULL;
    }
    // printf("[vmm] new_addrspace: terminator_block allocated at %lx\n", terminator_block);

    terminator_block->base = 0xFFFFFFFFFFFFF000;
    terminator_block->length = PAGE_SIZE;
    terminator_block->flags = 0;

    list_append(&new_addrspace->vmm_map, (list_elem_t *)block_head);
    // printf("[vmm] new_addrspace: appended block_head to vmm_map\n");

    list_append(&new_addrspace->vmm_map, (list_elem_t *)terminator_block);

    // printf("[vmm] new_addrspace: appended terminator_block to vmm_map\n");

    return new_addrspace;
}

uintptr_t vmm_alloc(size_t size, uint8_t flags, struct addrspace *addrspace)
{
    size = ALIGN_TO_PAGE(size);

    list_elem_t *prev = addrspace->vmm_map.head;
    list_elem_t *current = addrspace->vmm_map.head->next;
    uintptr_t addr = 0;

    LIST_LOOP(&addrspace->vmm_map, current)
    {
        struct vmm_block *prev_block = (struct vmm_block *)prev;
        struct vmm_block *current_block = (struct vmm_block *)current;

        uintptr_t gap_start = ALIGN_TO_PAGE(prev_block->base + prev_block->length);
        uintptr_t gap_end = current_block->base;

        if (gap_end > gap_start && gap_end - gap_start >= size)
        {
            addr = gap_start;
            break;
        }

        prev = current;
    }

    if (addr == 0)
    {
        struct vmm_block *last_block = (struct vmm_block *)prev;
        addr = ALIGN_TO_PAGE(last_block->base + last_block->length);
    }

    struct vmm_block *new_block = (struct vmm_block *)PHYS_TO_VIRT(pmm_alloc(PAGE_SIZE));
    new_block->base = addr;
    new_block->length = size;
    new_block->flags = flags;

    list_insert_after(&addrspace->vmm_map, prev, &new_block->elem);

    for (uintptr_t offset = 0; offset < size; offset += PAGE_SIZE)
    {
        uintptr_t phys = pmm_alloc(PAGE_SIZE);
        vmm_map_page(addrspace->pml4, addr + offset, phys, flags);
    }

    // printf("[vmm] Allocated %d bytes at %lx, inserted into vmm_map\n", size, addr);
    return addr;
}

void vmm_print_addrspace(struct addrspace *addrspace)
{
    printf("Printing ADDRESS-SPACE\n");
    printf("\nPML4: %lx\n", addrspace->pml4);
    struct vmm_block *head = (struct vmm_block *)addrspace->vmm_map.head;
    struct vmm_block *tail = (struct vmm_block *)addrspace->vmm_map.tail;
    printf("Head: %lx\n", head->base);

    list_elem_t *tmp = addrspace->vmm_map.head->next;
    while (tmp && tmp != addrspace->vmm_map.tail)
    {
        struct vmm_block *block = (struct vmm_block *)tmp;
        printf("%lx\n", block->base);
        tmp = tmp->next;
    }

    printf("Tail: %lx\n", tail->base);
}

// void vmm_print_addrspace(struct addrspace *addrspace)
// {
//     list_print(&addrspace->vmm_map);
// }

void vmm_map_region(struct addrspace *addrspace, uintptr_t virt, uintptr_t phys, size_t size, size_t flags)
{
    // printf("[vmm] Mapping region: PML4=0x%lx virt=0x%lx phys=0x%lx size=0x%lx flags=0x%lx\n", addrspace->pml4, virt, phys, size, flags);
    for (uintptr_t offset = 0; offset < size; offset += PAGE_SIZE)
    {
        vmm_map_page(addrspace->pml4, virt + offset, phys + offset, flags);
    }
}

void vmm_enable_paging(struct addrspace *addrspace)
{
    uintptr_t virt_addr = (uintptr_t)addrspace->pml4;

    // Convert virtual PML4 address to physical before loading CR3
    uintptr_t phys_addr = VIRT_TO_PHYS(virt_addr);

    if (phys_addr % PAGE_SIZE != 0)
    {
        printf("[vmm] PML4 physical address not aligned: %lx\n", phys_addr);
        return;
    }

    // printf("[vmm] Enabling paging...\n");
    // printf("PML4 Virtual Address: %lx\n", virt_addr);
    // printf("PML4 Physical Address: %lx\n", phys_addr);

    asm volatile(
        "movq %0, %%cr3"
        :
        : "r"(phys_addr)
        : "memory");
}

void vmm_init()
{
    // printf("\nCreating a new addrspace!\n");
    kernel_addrspace = new_addrspace();
    list_elem_t *tmp;

    long int entry_count = r_memmap->entry_count;
    for (long i = 0; i < r_memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = r_memmap->entries[i];
        uintptr_t phys = entry->base;
        uintptr_t virt = PHYS_TO_VIRT(entry->base);
        size_t region_length = entry->length;
        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
        {
            vmm_map_region(kernel_addrspace, virt, phys, region_length, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
        }
    }
    for (size_t i = 0; i < ((size_t)kernel_size); i += PAGE_SIZE)
    {
        vmm_map_page(kernel_addrspace->pml4, (uintptr_t)r_kernel_address->virtual_base + i, (uintptr_t)r_kernel_address->physical_base + i, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
    }
    // vmm_print_addrspace(kernel_addrspace);
    //  printf("\naddrspace created\n");
    vmm_enable_paging(kernel_addrspace);
}
