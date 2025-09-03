#include "pmm.h"
#include "hhdm.h"

#define MAX_FREE_REGIONS 1024
static free_region_t free_region_pool[MAX_FREE_REGIONS];
static size_t free_region_count = 0;
linked_list_t pmm_map = LIST_INIT;

free_region_t *allocate_region()
{
    if (free_region_count >= MAX_FREE_REGIONS)
        return NULL;

    free_region_t *region = &free_region_pool[free_region_count];
    region->pool_index = free_region_count;
    free_region_count++;
    return region;
}
void free_region(free_region_t *region)
{
    if (!region)
    {
        return;
    }
    size_t i = region->pool_index;
    if (i < free_region_count - 1)
    {
        free_region_pool[i] = free_region_pool[free_region_count - 1];
        free_region_pool[i].pool_index = i;
    }
    free_region_count--;
}

void pmm_free_internal(uintptr_t base, size_t length)
{
    free_region_t *region = allocate_region();
    region->base = base;
    region->length = length;
    region->elem = ELEM_INIT;
    list_append(&pmm_map, &region->elem);
}

void pmm_free(uintptr_t addr, size_t length)
{
    free_region_t *new_region = allocate_region();
    new_region->base = addr;
    new_region->length = length;
    uintptr_t new_region_start = addr;
    uintptr_t new_region_end = addr + length;

    // Empty list! add new region
    if (pmm_map.head == NULL)
    {
        logger(WARNING, "Head was null at pmm_free\n");
        list_append(&pmm_map, &new_region->elem);
        return;
    }

    free_region_t *first = (free_region_t *)pmm_map.head;

    // New region is before the head
    if (new_region_end < first->base)
    {
        // printf("\nRegion found before the head!\n");

        // If contiguous with head, merge
        if (new_region_end == first->base)
        {
            first->base = new_region->base;
            first->length += new_region->length;
            // Free the new_region metadata since merged
            free_region(new_region);
        }
        else
        {
            // Insert new_region before head
            list_insert_before(&pmm_map, &first->elem, &new_region->elem);
        }
        return;
    }

    // Middle of the list
    list_elem_t *tmp = pmm_map.head->next;
    free_region_t *prev = (free_region_t *)pmm_map.head;

    while (tmp != NULL)
    {
        free_region_t *next = (free_region_t *)tmp;

        // printf("\nSearching region\n");
        // printf("Prev region end: %lx \n", prev->base + prev->length);
        // printf("Current Region %lx - %lx \n", new_region_start, new_region_end);
        // printf("Next region start: %lx \n", next->base);

        // No blocks touch
        if (new_region_start > prev->base + prev->length &&
            new_region_end < next->base)
        {
            // Insert new_region between prev and next
            list_insert_after(&pmm_map, &prev->elem, &new_region->elem);
            return;
        }

        // All 3 of the blocks touch
        else if (new_region_start == prev->base + prev->length &&
                 new_region_end == next->base)
        {
            // Merge prev, new_region and next into one block
            prev->length += new_region->length + next->length;
            list_delete(&pmm_map, &next->elem);
            free_region(new_region);
            return;
        }

        // Merge with prev if start = prev end
        else if (new_region_start == prev->base + prev->length)
        {
            prev->length += new_region->length;
            free_region(new_region);
            return;
        }

        // Merge with next if end = next base
        else if (new_region_end == next->base)
        {
            next->base = new_region_start;
            next->length += new_region->length;
            free_region(new_region);
            return;
        }

        prev = next;
        tmp = tmp->next;
    }

    // Append at the tail
    if (new_region_start >= prev->base + prev->length)
    {
        // printf("\nRegion found after the tail!\n");

        if (new_region_start == prev->base + prev->length)
        {
            // Merge with prev
            prev->length += new_region->length;
            free_region(new_region);
        }
        else
        {
            // Append new region at tail
            list_append(&pmm_map, &new_region->elem);
        }
        return;
    }
    else
    {
        logger(WARNING, "Overlapping or invalid regions at pmm_free\n");
        free_region(new_region);
    }
}

uintptr_t pmm_alloc(size_t size)
{
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    list_elem_t *tmp = pmm_map.head;
    while (tmp != NULL)
    {
        free_region_t *region = (free_region_t *)tmp;
        if (region->length >= size)
        {
            uintptr_t allocated_base = region->base;

            region->base += size;
            region->length -= size;

            if (region->length == 0) // If region has length of 0 delete it
            {
                list_delete(&pmm_map, &region->elem);
            }

            return allocated_base;
        }
        tmp = tmp->next;
    }
    return 0;
}
void pmm_print_regions()
{
    list_elem_t *tmp;
    printf("Printing PMM_REGIONS\n");
    LIST_LOOP(&pmm_map, tmp)
    {
        free_region_t *region = (free_region_t *)tmp;
        printf("Free Region: base = 0x%lx, length = %d bytes \n",
               region->base,
               region->length);
    }
}
void pmm_init()
{
    hhdm_offset = r_hhdm->offset;
    long int entry_count = r_memmap->entry_count;
    for (long i = 0; i < r_memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = r_memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            uintptr_t base = (uintptr_t)(entry->base);
            size_t length = entry->length;
            pmm_free_internal(base, length);
        }
    }
    // pmm_print_regions();
    // uintptr_t ptr = pmm_alloc(PAGE_SIZE);
    // uintptr_t ptr2 = pmm_alloc(PAGE_SIZE * 8);
    // uintptr_t ptr3 = pmm_alloc(PAGE_SIZE * 8);
    // uintptr_t ptr6 = pmm_alloc(PAGE_SIZE * 8);
    // pmm_print_regions();
    // pmm_free(ptr, PAGE_SIZE);
    // pmm_free(ptr2, PAGE_SIZE * 8);
    // pmm_free(ptr3, PAGE_SIZE * 8);
    // pmm_free(ptr6, PAGE_SIZE * 8);
    // pmm_print_regions();
    // pmm_free(ptr2, PAGE_SIZE);
    // pmm_print_regions();
}