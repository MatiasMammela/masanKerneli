#ifndef HHDM_H
#define HHDM_H
#include "limineRequest.h"
#include <stdint.h>
extern uint64_t hhdm_offset;
#define PHYS_TO_VIRT(ADDR) ((void *)((uintptr_t)(ADDR) + (uintptr_t)hhdm_offset))

#endif // HHDM_H