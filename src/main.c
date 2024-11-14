#include "lib.h"
#include "../limine/limine.h"
#include "limineRequest.h"
#include "pmm.h"
#include "vmm.h"
#include "gdt.h"
#include "idt.h"
#include "hhdm.h"
#include "heap.h"
#include "scheduler.h"
uint64_t hhdm_offset = 0;
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}
__attribute__((noreturn)) void panic()
{
    printf("\nFuck\n");
    hcf();
    __builtin_unreachable();
}

void kmain(void)
{

    if (request_init() != 0)
    {
        hcf();
    }

    hhdm_offset = r_hhdm->offset;

    pmm_init();

    vmm_init();

    gdt_init();

    idt_init();

    // test_vmm_alloc();
    heap_init();
    // test_heap();
    scheduler_init();
    //  debug_scheduler();

    printf("END \n");
    hcf();
}