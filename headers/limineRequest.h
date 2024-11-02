#ifndef LIMINEREQUEST_H
#define LIMINEREQUEST_H
#include "../limine/limine.h"
#include "lib.h"
extern struct limine_memmap_response *r_memmap;
extern struct limine_hhdm_response *r_hhdm;
extern struct limine_kernel_address_response *r_kernel_address;
extern int request_init();
#endif // LIMINEREQUEST_H