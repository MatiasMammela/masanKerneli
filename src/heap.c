#include "heap.h"
#include "vmm.h"

heap_t *heap = NULL; // Define the heap here

void heap_init()
{
    // Allocate memory for the heap structure itself
    heap = (heap_t *)vmm_alloc(sizeof(heap_t), VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT, &kernel_addrspace);
    if (heap == NULL)
    {
        printf("Failed to initialize heap\n");
        return;
    }

    // Set up the total size of the heap (e.g., 1MB)
    heap->total_size = 1024 * 1024;

    // Allocate a memory block for the first block within the heap (1MB for the entire heap)
    heap->first_block = (heap_block *)vmm_alloc(heap->total_size, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT, &kernel_addrspace);
    if (heap->first_block == NULL)
    {
        printf("Failed to allocate first block for the heap\n");
        return;
    }

    // Initialize the first block's properties
    heap->first_block->base = (uint64_t)heap->first_block + sizeof(heap_block);
    heap->first_block->length = heap->total_size - sizeof(heap_block);
    heap->first_block->status = FREE;
    heap->first_block->next = NULL;
    heap->first_block->prev = NULL;
    // printf("Heap initialized with base address %lx\n", (uint64_t)heap->first_block->base);
}

void *malloc(size_t size)
{
    // Align size to a multiple of 16 bytes for alignment purposes
    size = (size + 15) & ~15;

    heap_block *current = heap->first_block;

    // Traverse the heap to find a free block of sufficient size
    while (current)
    {
        if (current->status == FREE && current->length >= size)
        {
            // If the block is larger than requested size, split it
            if (current->length > size + sizeof(heap_block))
            {
                // Create a new block for the remaining free space
                heap_block *new_block = (heap_block *)(current->base + size);
                new_block->base = current->base + size + sizeof(heap_block);
                new_block->length = current->length - size - sizeof(heap_block);
                new_block->status = FREE;
                new_block->next = current->next;
                new_block->prev = current;

                if (current->next)
                {
                    current->next->prev = new_block;
                }

                // Update the current block
                current->length = size;
                current->status = ALLOCATED;
                current->next = new_block;
            }
            else
            {
                // Otherwise, allocate the entire block
                current->status = ALLOCATED;
            }
            return (void *)(current->base);
        }
        current = current->next;
    }

    // If no suitable block was found, expand the heap
    printf("Not enough space! Attempting to allocate an additional %lx bytes...\n", size);
    heap_block *new_block = (heap_block *)vmm_alloc(size + sizeof(heap_block), VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT, &kernel_addrspace);
    if (!new_block)
    {
        return NULL;
    }

    // Initialize the new block
    new_block->base = (uint64_t)new_block + sizeof(heap_block);
    new_block->length = size; // This block is the new allocation
    new_block->status = ALLOCATED;
    new_block->next = NULL;

    // Attach the new block to the end of the heap list
    heap_block *last = heap->first_block;
    while (last->next)
    {
        last = last->next;
    }
    last->next = new_block;
    new_block->prev = last;

    return (void *)(new_block->base);
}
void free(void *addr)
{
    heap_block *current = heap->first_block;
    while (current)
    {
        if ((void *)current->base == addr)
        {
            current->status = FREE;

            // Attempt to coalesce with next block if it's also free
            if (current->next && current->next->status == FREE)
            {
                current->length += current->next->length + sizeof(heap_block);
                current->next = current->next->next;
            }

            return; // Free completed
        }
        current = current->next;
    }
}

void test_heap()
{
    printf("Testing heap allocation and free functions:\n");

    // Allocate several blocks of varying sizes
    void *block1 = malloc(128);
    printf("Allocated 128 bytes at %lx\n", block1);

    void *block2 = malloc(256);
    printf("Allocated 256 bytes at %lx\n", block2);

    void *block3 = malloc(512);
    printf("Allocated 512 bytes at %lx\n", block3);

    void *block4 = malloc(128);
    printf("Allocated 128 bytes at %lx\n", block4);

    // Check pointers to ensure allocations succeeded
    if (!block1 || !block2 || !block3 || !block4)
    {
        printf("Allocation failed for one or more blocks\n");
        return;
    }

    // Free some blocks
    printf("Freeing block1 at %lx\n", block1);
    free(block1);

    printf("Freeing block3 at %lx\n", block3);
    free(block3);

    // Reallocate blocks to see if freed space is reused
    void *block5 = malloc(64); // This should fit in the space freed by block1
    printf("Allocated 64 bytes at %lx (should reuse block1's space if available)\n", block5);

    void *block6 = malloc(1024 * 1024); // This might reuse block3’s space
    printf("Allocated 1MB bytes at %lx (should reuse block3's space if available)\n", block6);

    // Final allocations to see if they fit into the free space correctly
    void *block7 = malloc(128);
    printf("Allocated 128 bytes at %lx\n", block7);

    // Free remaining blocks to check cleanup
    printf("Freeing block2 at %lx\n", block2);
    free(block2);

    printf("Freeing block4 at %lx\n", block4);
    free(block4);

    printf("Freeing block5 at %lx\n", block5);
    free(block5);

    printf("Freeing block6 at %lx\n", block6);
    free(block6);

    printf("Freeing block7 at %lx\n", block7);
    free(block7);

    printf("Heap allocation and free tests completed.\n");
}
