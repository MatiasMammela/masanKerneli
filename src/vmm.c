#include "vmm.h"
#include "pmm.h"
#include "limineRequest.h"
#include "hhdm.h"
#include "heap.h"
extern uint8_t kernel_size[];
struct addrspace kernel_addrspace;

struct addrspace test;
void enable_paging(void *val)
{
    uintptr_t physical_address = (uintptr_t)val - hhdm_offset;

    if (physical_address % PAGE_SIZE != 0)
    {
        printf("Pages are not aligned\n");
        return;
    }

    // printf("Enabling paging...\n");
    //  printf("Virtual Address: %lx\n", (uintptr_t)val);
    //  printf("Physical Address: %lx\n", physical_address);

    asm volatile(
        "movq %0, %%cr3"
        :
        : "r"((uintptr_t)physical_address)
        : "memory");
    // printf("Paging enabled with CR3 set to: %lx\n", physical_address);
}

struct addrspace *create_addrspace()
{
    struct addrspace *new_addrspace = malloc(sizeof(struct addrspace));

    new_addrspace->pml4 = (uint64_t *)(pmm_alloc() + hhdm_offset);
    memset(new_addrspace->pml4, 0, PAGE_SIZE);

    // Create the initial block head
    struct vmm_block *block_head = PHYS_TO_VIRT(pmm_alloc());
    block_head->base = ALIGN_TO_PAGE(hhdm_offset);
    block_head->length = PAGE_SIZE;
    block_head->flags = VMM_BLOCK_EXEC | VMM_BLOCK_WRITE;

    struct vmm_block *terminator_block = PHYS_TO_VIRT(pmm_alloc());
    terminator_block->base = 0xFFFFFFFFFFFFF000;
    terminator_block->length = PAGE_SIZE;
    terminator_block->flags = 0;
    terminator_block->next = NULL;

    // Link the blocks
    block_head->next = terminator_block;
    new_addrspace->block_head = block_head;

    return new_addrspace;
}
uint64_t *get_next_table(uint64_t *table, uint64_t lvl_index, uint8_t flags)
{

    if (!(table[lvl_index] & PTE_BIT_PRESENT))
    {

        uintptr_t new_table = pmm_alloc();
        memset((void *)(new_table + hhdm_offset), 0, PAGE_SIZE);
        table[lvl_index] = new_table | flags;
    }

    return (uint64_t *)PHYS_TO_VIRT(table[lvl_index] & ADDRESS_MASK);
}

void vmm_map(uint64_t *pml4, void *vaddr, uintptr_t paddr, uint8_t flags)
{

    uint64_t lvl4_index = VADDR_TO_INDEX(vaddr, 4);
    uint64_t lvl3_index = VADDR_TO_INDEX(vaddr, 3);
    uint64_t lvl2_index = VADDR_TO_INDEX(vaddr, 2);
    uint64_t lvl1_index = VADDR_TO_INDEX(vaddr, 1);
    uint64_t *lvl3_table = get_next_table(pml4, lvl4_index, flags);
    uint64_t *lvl2_table = get_next_table(lvl3_table, lvl3_index, flags);
    uint64_t *lvl1_table = get_next_table(lvl2_table, lvl2_index, flags);

    lvl1_table[lvl1_index] = paddr | flags;
}

void *vmm_alloc(size_t size, uint8_t flags, struct addrspace *addrspace)
{
    size = ALIGN_TO_PAGE(size);

    struct vmm_block *previous_block = addrspace->block_head;
    struct vmm_block *current_block = addrspace->block_head->next;

    while (current_block != NULL)
    {

        if (previous_block->base + previous_block->length + size <= current_block->base) // If we found a gap big enough
        {

            struct vmm_block *new_block = PHYS_TO_VIRT(pmm_alloc()); // Create a new block

            new_block->base = previous_block->base + previous_block->length; // new base of the block will be previous block base + length
            new_block->length = size;
            new_block->flags = flags;

            previous_block->next = new_block; // Set the new block as the next of the previous block
            new_block->next = current_block;

            for (size_t i = 0; i < size; i += PAGE_SIZE) // divide the size needed to pages
            {
                uintptr_t phys_addr = (uintptr_t)pmm_alloc(); // For every page allocate physical memory for the size of page

                vmm_map(addrspace->pml4, (void *)((uintptr_t)new_block->base + i),
                        phys_addr, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT); // Map the newly created pages to vmm map
            }

            return (void *)new_block->base; // return the base of the new block we created
        }

        previous_block = current_block;
        current_block = current_block->next; // Move forward the block chain
    }
    // if no free space were found
    printf("\nPanic at vmm_alloc\n ");
    panic();
    __builtin_unreachable();
}

uint64_t vmm_unmap(uint64_t *pml4, void *vaddr)
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

void vmm_free(void *addr, struct addrspace *addrspace)
{
    struct vmm_block *previous_block = addrspace->block_head;
    struct vmm_block *current_block = addrspace->block_head->next;

    while (current_block != NULL)
    {
        if (current_block->base == (uintptr_t)addr) // if we found the block that has the base address of address given as a parameter
        {
            struct vmm_block *next_block = current_block->next; // Find the block that is +1 from the block we want to free

            // free the block
            for (size_t i = 0; i < current_block->length; i += PAGE_SIZE)
            {
                void *vaddr = (void *)(current_block->base + i);
                uint64_t phys_addr = vmm_unmap(addrspace->pml4, vaddr);

                if (phys_addr != 0)
                {
                    pmm_free(phys_addr);
                }
            }

            previous_block->next = next_block; // Close the gap made by the freed block
            pmm_free((uintptr_t)current_block - hhdm_offset);
            return;
        }

        previous_block = current_block; // While block is null go forwards the block chain
        current_block = current_block->next;
    }

    printf("\nPanic at vmm_free\n ");
    panic();
}

void test_vmm_alloc()
{

    // Allocate memory blocks
    size_t block_size = PAGE_SIZE;
    void *addr1 = vmm_alloc(block_size, VMM_TABLE_ENTRY_READ_WRITE, &kernel_addrspace);
    printf("Allocated block 1  %lx\n", addr1);

    void *addr2 = vmm_alloc(block_size, VMM_TABLE_ENTRY_READ_WRITE, &kernel_addrspace);
    printf("Allocated block 2  %lx\n", addr2);

    void *addr3 = vmm_alloc(block_size, VMM_TABLE_ENTRY_READ_WRITE, &kernel_addrspace);
    printf("Allocated block 3  %lx\n", addr3);

    void *addr4 = vmm_alloc(block_size, VMM_TABLE_ENTRY_READ_WRITE, &kernel_addrspace);
    printf("Allocated block 4  %lx\n", addr4);

    // Free memory blocks
    vmm_free(addr1, &kernel_addrspace);
    printf("\nFreed block 1  %lx\n", addr1);

    vmm_free(addr2, &kernel_addrspace);
    printf("Freed block 2  %lx\n", addr2);

    vmm_free(addr3, &kernel_addrspace);
    printf("Freed block 3 %lx\n", addr3);

    vmm_free(addr4, &kernel_addrspace);
    printf("Freed block 4  %lx\n", addr4);

    void *addr5 = vmm_alloc(block_size, VMM_TABLE_ENTRY_READ_WRITE, &kernel_addrspace);
    printf("Allocated block   %lx\n", addr5);
}
void vmm_init_blocks()
{

    kernel_addrspace.block_head = PHYS_TO_VIRT(pmm_alloc());
    kernel_addrspace.block_head->base = ALIGN_TO_PAGE(hhdm_offset);
    kernel_addrspace.block_head->length = ALIGN_TO_PAGE(sizeof(hhdm_offset));
    kernel_addrspace.block_head->flags = VMM_BLOCK_EXEC | VMM_BLOCK_WRITE;

    struct vmm_block *kernel_block = PHYS_TO_VIRT(pmm_alloc());
    kernel_block->base = r_kernel_address->virtual_base;
    kernel_block->length = (size_t)kernel_size;
    kernel_block->flags = VMM_BLOCK_EXEC | VMM_BLOCK_WRITE;

    kernel_addrspace.block_head->next = kernel_block;
}

void vmm_init()
{

    kernel_addrspace.pml4 = (uint64_t *)(pmm_alloc() + hhdm_offset); // pml4 to the hhdm_offset

    memset(kernel_addrspace.pml4, 0, PAGE_SIZE);

    for (unsigned int i = 0; i < r_memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = r_memmap->entries[i];
        for (size_t j = 0; j < (entry->length + PAGE_SIZE); j += PAGE_SIZE)
        {
            if (entry->length < PAGE_SIZE)
            {
                break;
            }
            vmm_map(kernel_addrspace.pml4, (void *)PHYS_TO_VIRT(entry->base + j), entry->base + j, PTE_BIT_PRESENT | VMM_TABLE_ENTRY_READ_WRITE);
        }
    }

    for (size_t i = 0; i < ((size_t)kernel_size); i += PAGE_SIZE)
    {
        vmm_map(kernel_addrspace.pml4, (void *)r_kernel_address->virtual_base + i, r_kernel_address->physical_base + i, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
    }

    enable_paging(kernel_addrspace.pml4);
    vmm_init_blocks();
}
