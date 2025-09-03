#pragma once
#include "lib.h"
#include "spinlock.h"
#include "vmm.h"
#include "list.h"
#define RSP_OFFSET offsetof(thread_t, rsp)

typedef enum
{
    WAITING,
    RUNNING,
    DEAD
} status_t;

typedef struct process_t
{
    list_elem_t elem;
    uint64_t pid;
    spinlock_t lock;
    addrspace addrspace;
    linked_list_t threads;
    status_t status;
} process_t;

typedef struct thread_t
{
    list_elem_t elem;
    uintptr_t rsp;
    uint64_t id;
    status_t status;
    process_t *process;
} thread_t;

typedef struct // Helpers struct to write a stack to the addrspace of the process
{
    uint64_t r12, r13, r14, r15, rbp, rbx;
    void (*init)(struct thread_t *prev);
    void (*function)();
} __attribute__((packed)) stack;

void scheduler_init();
void debug_thread_que();
thread_t *create_thread(void (*function)(), process_t *process, uint64_t id);
void init_thread(thread_t *prev);
void schedule_threads();
void append_thread_to_que(thread_t *thread);
process_t *create_process(addrspace *addrspace, uint64_t pid);
void add_thread_back_to_que(thread_t *thread);
void yield();