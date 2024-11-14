#include "scheduler.h"
#include "heap.h"

// Order of scheduling
//  current_process => ready_queue_head => ready_queue_head->next .....

process_t *current_process = NULL;
process_t *ready_queue_head = NULL;

extern struct thread *context_switch(struct thread *old_context, struct thread *new_context);

void debug_scheduler()
{
    if (current_process != NULL)
    {
        printf("\nCurrent Process %s\n", current_process->name);
        printf("rsp %lx\n", &current_process->thread);
        printf("status  %d \n", current_process->process_status);
    }
    process_t *ptr = ready_queue_head;
    while (ptr != NULL)
    {
        printf("\nProcess %s\n", ptr->name);
        printf("rsp %lx \n", &ptr->thread);
        printf("status  %d \n", ptr->process_status);
        ptr = ptr->next;
    }
}

void add_process_to_ready_queue(process_t *process)
{
    if (process->process_status == DEAD) // Drop of dead processes
    {
        return;
    }

    if (ready_queue_head == NULL) // If ready_queue_head is NULL set new head
    {
        ready_queue_head = process;
        printf("\n New ready_queue_head set at add_process_to_ready_queue \n");
    }
    else
    {
        process_t *temp = ready_queue_head;
        while (temp->next != NULL) // Move to the end of the ready_queue_head
        {
            temp = temp->next;
        }
        temp->next = process; // Place the process to the end of the list
        printf("\n New ready_queue_head->next set at add_process_to_ready_queue \n");
    }

    process->process_status = READY; // Set the Process as Ready

    process->next = NULL;
}

void schedule()
{
    printf("\n Started to schedule \n ");
    if (ready_queue_head == NULL)
    {
        printf("\n Ready queue head null \n");
        return;
    }

    if (current_process->process_status != RUNNING)
    {

        struct thread *dummy_thread = malloc(sizeof(struct thread));
        memset(dummy_thread, 0, sizeof(sizeof(struct thread)));
        dummy_thread->process = NULL;

        process_t *old_process = current_process; // Todo current process should stay NULL before the first schedule is done
        // Todo Replace the old process with a dummy process. Prevent the dummy process from getting to the add_process_to_ready_queue by setting the state of it to destroy
        process_t *new_process = ready_queue_head;

        printf("\nSwitching process %lx  %s To\n", old_process->thread, old_process->name);
        printf("%lx  %s\n", new_process->thread, new_process->name);

        // Move the ready_queue head forwards
        ready_queue_head = ready_queue_head->next;
        current_process = new_process;
        current_process->process_status = RUNNING;
        struct thread *old_thread = context_switch(old_process->thread, new_process->thread);

        add_process_to_ready_queue(old_thread->process);

        debug_scheduler();
    }
}
void init_scheduling()
{

    process_t *dummy_process = malloc(sizeof(process_t));
    struct thread *dummy_thread = malloc(sizeof(struct thread));
    dummy_process->thread = dummy_thread;
    dummy_thread->process = dummy_process;
    dummy_process->process_status = DEAD;

    process_t *new_process = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    current_process = new_process;
    struct thread *old_thread = context_switch(dummy_process->thread, new_process->thread);
}
void yield()
{
    if (current_process == NULL || ready_queue_head == NULL)
    {
        printf("\n Current process is NULL or ready_queue_head is NULL. Cant yield\n");
        return;
    }

    current_process->process_status = READY;
    printf("\nYielding\n");
    schedule();
}

void task3()
{

    printf("\n Task1 Running ..\n");
    while (1)
    {
        yield();
    }
}

void task2()
{
    printf("\n Task2 Running ..\n");
    while (1)
    {
        yield();
    }
}

void task1()
{

    printf("\n Task1 Running ..\n");
    while (1)
    {
        yield();
    }
}

void init_thread(struct thread *prev)
{
    printf("\n Context switched \n");

    add_process_to_ready_queue(prev->process);
}

process_t *create_process(void (*function)(), char *name)
{

    process_t *new_process = malloc(sizeof(process_t));
    new_process->name = name;
    size_t stack_size = 4096;

    void *stack_base = vmm_alloc(stack_size, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT, &kernel_addrspace);

    uintptr_t stack_address = (uintptr_t)stack_base + stack_size - sizeof(stack);

    stack *end_of_stack = (stack *)stack_address;

    end_of_stack->r12 = 0;
    end_of_stack->r13 = 0;
    end_of_stack->r14 = 0;
    end_of_stack->r15 = 0;
    end_of_stack->rbp = 0;
    end_of_stack->rbx = 0;
    end_of_stack->init = init_thread;
    end_of_stack->function = function;
    new_process->thread = malloc(sizeof(struct thread));

    new_process->thread->rsp = (uint64_t)stack_address;

    // Create a circular pointer so the thread can its own process
    new_process->thread->process = new_process;
    return new_process;
}

void scheduler_init()
{

    process_t *task1_proc = create_process(task1, "Task1");
    add_process_to_ready_queue(task1_proc);

    process_t *task2_proc = create_process(task2, "Task2");
    add_process_to_ready_queue(task2_proc);

    process_t *task3_proc = create_process(task3, "Task3");
    add_process_to_ready_queue(task3_proc);

    debug_scheduler();
    init_scheduling();
}