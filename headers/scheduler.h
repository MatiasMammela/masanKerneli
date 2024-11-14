#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "lib.h"
#include "vmm.h"

typedef enum
{
    READY,
    RUNNING,
    DEAD
} status_t;
typedef struct process
{
    status_t process_status;
    struct addrspace *addrspace; // Page tables
    struct process *next;        // Link to the next process
    struct thread *thread;       // Thread
    char *name;

} process_t;

struct thread
{
    uintptr_t rsp;
    process_t *process;
};

typedef struct // Helpers struct to write a stack to the addrspace of the process
{
    uint64_t r12, r13, r14, r15, rbp, rbx;
    void (*init)(struct thread *prev);
    void (*function)();
} __attribute__((packed)) stack;

extern void scheduler_init();

#endif // SCHEDULER_H