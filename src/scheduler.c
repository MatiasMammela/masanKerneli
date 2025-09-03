#include "scheduler.h"

linked_list_t thread_que = LIST_INIT;
thread_t *current_running_thread = NULL;

process_t *create_process(addrspace *addrspace, uint64_t pid)
{
    process_t *new_process = malloc(sizeof(process_t));
    new_process->pid = pid;
    new_process->addrspace = *addrspace;
    new_process->threads = LIST_INIT;
    return new_process;
}

void add_thread_back_to_que(thread_t *thread)
{
    if (thread->status == DEAD)
    {
        return;
    }
    // printf("\nWAITED THREAD %d\n", thread->id);
    thread->status = WAITING;
    list_elem_t *thread_list_elem = &thread->elem;

    // Remove from wherever it is and move to back
    list_move_elem_back(&thread_que, thread_list_elem);
}

void append_thread_to_que(thread_t *thread)
{
    if (thread->status == DEAD)
        return;

    // printf("\nAPPENDED THREAD %d\n", thread->id);
    thread->status = WAITING;
    list_elem_t *thread_list_elem = &thread->elem;
    list_append(&thread_que, thread_list_elem);
}

extern struct thread_t *context_switch(thread_t *old_thread, thread_t *new_thread, uint64_t rsp_offset);
void schedule_threads()
{
    list_elem_t *elem = thread_que.head;
    thread_t *old_thread = current_running_thread;

    // debug_thread_que();

    while (elem != NULL)
    {
        thread_t *candidate = LIST_ENTRY(elem, thread_t, elem);

        if (candidate->status == WAITING)
        {
            current_running_thread = candidate;
            current_running_thread->status = RUNNING;

            if (old_thread && old_thread != current_running_thread && old_thread->status != DEAD)
            {
                context_switch(old_thread, current_running_thread, RSP_OFFSET);
                add_thread_back_to_que(old_thread);
            }

            add_thread_back_to_que(current_running_thread);
            return; // schedule only one thread
        }
        elem = elem->next;
    }

    printf("No WAITING threads found\n");
}

void init_thread(thread_t *prev)
{
    if (!prev || prev->status == DEAD)
        return;

    // Only add if not already in queue. checks if prev not linked
    if (prev->elem.next == NULL && prev->elem.prev == NULL)
    {
        printf("Requeuing thread %d after switch\n", prev->id);
        add_thread_back_to_que(prev);
    }
}

thread_t *create_thread(void (*function)(), process_t *process, uint64_t id)
{
    thread_t *new_thread = malloc(sizeof(thread_t));
    new_thread->process = process;
    new_thread->id = id;
    size_t stack_size = 4096;
    uintptr_t stack_base = vmm_alloc(stack_size, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT, &process->addrspace);
    uintptr_t stack_address = stack_base + stack_size - sizeof(stack);

    stack *end_of_stack = (stack *)stack_address;
    end_of_stack->r12 = 0;
    end_of_stack->r13 = 0;
    end_of_stack->r14 = 0;
    end_of_stack->r15 = 0;
    end_of_stack->rbp = 0;
    end_of_stack->rbx = 0;
    end_of_stack->init = (void *)init_thread;
    end_of_stack->function = function;

    new_thread->rsp = (uintptr_t)end_of_stack;
    new_thread->status = WAITING;

    // Initialize list element pointers to NULL
    new_thread->elem.next = NULL;
    new_thread->elem.prev = NULL;

    return new_thread;
}

void debug_thread_que()
{
    list_elem_t *tmp_thread;
    printf("\nThread queue status:\n");
    LIST_LOOP(&thread_que, tmp_thread)
    {
        thread_t *current_thread = LIST_ENTRY(tmp_thread, thread_t, elem);
        printf("Thread ID %d \n", current_thread->id);
        printf("Stack Pointer %lx \n", current_thread->rsp);
        switch (current_thread->status)
        {
        case WAITING:
            printf("Status WAITING \n");
            break;
        case DEAD:
            printf("Status DEAD \n");
            break;
        case RUNNING:
            printf("Status RUNNING \n");
            break;
        default:
            printf("Status UNKNOWN \n");
            break;
        }
    }
}

void yield()
{
    if (current_running_thread == NULL || current_running_thread->process == NULL)
    {
        printf("Current running thread NULL\n");
        return;
    }
    current_running_thread->status = WAITING;
    // add_thread_back_to_que(current_running_thread);
    schedule_threads();
}

void proc1()
{
    while (1)
    {
        printf("\nProcess 1");
        //    debug_thread_que();
        yield();
    }
}
void proc2()
{
    while (1)
    {
        printf("\nProcess 2");
        //    debug_thread_que();
        yield();
    }
}

void proc3()
{
    while (1)
    {
        printf("\nProcess 3");
        //    debug_thread_que();
        yield();
    }
}

void init_scheduling()
{
    process_t *dummy_process = create_process(kernel_addrspace, 0);
    thread_t *dummy_thread = create_thread(proc1, dummy_process, 0);
    dummy_thread->status = DEAD;

    thread_t *first_thread = LIST_ENTRY(thread_que.head, thread_t, elem);
    current_running_thread = first_thread;
    first_thread->status = RUNNING;

    context_switch(dummy_thread, first_thread, RSP_OFFSET);
}

void scheduler_init()
{
    printf("\nInitializing Scheduler...\n");
    process_t *proc = create_process(kernel_addrspace, 1);
    thread_t *main_thread = create_thread(proc1, proc, 10);
    thread_t *worker_thread = create_thread(proc2, proc, 11);
    thread_t *worker_thread2 = create_thread(proc3, proc, 12);
    append_thread_to_que(main_thread);
    append_thread_to_que(worker_thread);
    append_thread_to_que(worker_thread2);
    init_scheduling();

    for (;;)
    {
        // yield();
    }
}
