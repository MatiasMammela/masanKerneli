#pragma once
#include "lib.h"
#include "list.h"
#include "limine_requests.h"
#include "hhdm.h"
#include "logger.h"
#define PAGE_SIZE 4096
uintptr_t pmm_alloc(size_t size);
void pmm_init();
void pmm_free_internal(uintptr_t base, size_t length);
void pmm_print_regions();
void pmm_free(uintptr_t addr, size_t length);

typedef struct free_region
{
    list_elem_t elem; // ! KEEP THE BYTE OFFSET OF ELEM AS 0
    uintptr_t base;
    size_t length;
    size_t pool_index;
} free_region_t;
extern linked_list_t pmm_map;