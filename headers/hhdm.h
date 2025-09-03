#ifndef HHDM_H
#define HHDM_H
#include "limine_requests.h"
#include <stdint.h>
extern uint64_t hhdm_offset;
#define PHYS_TO_VIRT(ADDR) ((uintptr_t)((uintptr_t)(ADDR) + (uintptr_t)hhdm_offset))
#define VIRT_TO_PHYS(ADDR) ((uintptr_t)((uintptr_t)(ADDR) - (uintptr_t)hhdm_offset))
#endif // HHDM_H