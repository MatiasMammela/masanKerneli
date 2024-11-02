#include "limineRequest.h"
struct limine_memmap_response *r_memmap = NULL;
struct limine_hhdm_response *r_hhdm = NULL;
struct limine_kernel_address_response *r_kernel_address = NULL;

__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 3,
};
__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 3,
};

__attribute((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3};

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

int request_init()
{

    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        return -1;
    }

    if (hhdm.response == NULL)
    {
        printf("%s", "EE");
        return -1;
    }
    r_hhdm = hhdm.response;

    if (memmap.response == NULL)
    {
        printf("%s", "E");
        return -1;
    }
    r_memmap = memmap.response;

    if (kernel_address.response == NULL)
    {
        printf("%s", "kernel address not found on limineRequests\n");
    }
    r_kernel_address = kernel_address.response;

    return 0;
}