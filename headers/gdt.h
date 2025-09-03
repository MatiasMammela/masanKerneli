#pragma once
#include "lib.h"
#define ENTRY_AMOUNT 5

void gdt_construct_table();
void gdt_load_table();
typedef struct __attribute__((packed)) descriptor_t
{
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
} descriptor_t;

typedef struct __attribute__((packed)) ptr_struct_t
{
    uint16_t limit;
    uint64_t base;
} ptr_struct_t;

void gdt_init();