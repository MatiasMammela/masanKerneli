#pragma once
typedef struct spinlock_t
{
    volatile bool locked;
} spinlock_t;

static inline void accuire_spinlock(spinlock_t *lock)
{
    while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE))
    {
        asm("pause");
    }
}

static inline void release_spinlock(spinlock_t *lock)
{
    __atomic_clear(&lock->locked, __ATOMIC_RELEASE);
}