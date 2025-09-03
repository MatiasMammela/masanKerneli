#ifndef HEAP_H
#define HEAP_H
#include "lib.h"
#include "pmm.h"

typedef struct heap_block
{
    uint64_t base;           // Base address of the block
    size_t length;           // Length of the block
    struct heap_block *next; // Pointer to the next block in the list
    struct heap_block *prev; // Pointer to the previous block in the list
    enum
    {
        FREE,
        ALLOCATED
    } status; // Status of the block
} heap_block;

typedef struct heap
{
    heap_block *first_block; // Pointer to the first block in the linked list
    size_t total_size;       // Total size of the heap
} heap_t;
extern heap_t *heap;
void heap_init();
void *malloc(size_t size);
void free(void *ptr);
void test_heap();
#endif // HEAP_H