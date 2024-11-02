#ifndef LIB_H
#define LIB_H
#include <stdint.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

// Serial print
#define SERIAL_PORT 0x3F8

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
extern void serial_print_hex(unsigned int num);
extern void printf(const char *format, ...);

extern void *memcpy(void *dest, const void *src, size_t n);

extern void *memset(void *s, int c, size_t n);

extern void *memmove(void *dest, const void *src, size_t n);

extern int memcmp(const void *s1, const void *s2, size_t n);

#endif // LIB_H