

#include "lib.h"
#include "list.h"
#include "pmm.h"
#include "limine_requests.h"
#include "vmm.h"
#include "gdt.h"
#include "scheduler.h"
#include "idt.h"
void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}
__attribute__((noreturn)) void panic()
{
    logger(PANIC, "Kernel panic :]");
    hcf();
    __builtin_unreachable();
}

void kmain(void)
{
    logger(NOTICE, " START!");
    request_init();
    logger(NOTICE, " LIMINE_REQUESTS INIT");
    pmm_init();
    heap_init();
    logger(NOTICE, " PMM INIT");
    vmm_init();
    logger(NOTICE, " VMM INIT");
    gdt_init();
    logger(NOTICE, " GDT INIT");
    idt_init();
    logger(NOTICE, "IDT INIT");

    logger(NOTICE, " SCHEDULING INIT");
    scheduler_init();

    logger(NOTICE, " END!\n");
    hcf();
}
