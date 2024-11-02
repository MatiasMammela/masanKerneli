#ifndef PMM_H
#define PMM_H
#define PAGE_SIZE 4096
#define BLOCK_AMOUNT 1024
#include "lib.h"
#include "../limine/limine.h"
void pmm_init();
extern uintptr_t pmm_alloc();
extern void pmm_free(uintptr_t addr);
extern void panic();
#endif // PMM_H