#include "lib.h"

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}
void serial_write(char c)
{
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0)
    {
    } // Wait for the Transmit Holding Register (THR) to be empty
    outb(SERIAL_PORT, c); // Send the character to the serial port
}

static char *reverse_string(char *str, size_t length)
{
    size_t start = 0;
    size_t end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        ++start;
        --end;
    }
    return str;
}

char *itoa(int num, char *str, int base)
{
    static char digits[] = "0123456789abcdef";
    char buffer[33]; // Enough to hold the largest number in binary
    int i = 0;

    if (base < 2 || base > 16)
    {
        return NULL; // Unsupported base
    }

    // Handle 0 explicitly
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers if base is 10
    if (num < 0 && base == 10)
    {
        str[i++] = '-';
        num = -num;
    }

    // Convert number to string
    while (num != 0)
    {
        buffer[i++] = digits[num % base];
        num /= base;
    }

    // Add null terminator
    buffer[i] = '\0';

    // Reverse the string
    reverse_string(buffer, i);

    // Copy to the output string
    for (int j = 0; j < i; ++j)
    {
        str[j] = buffer[j];
    }

    str[i] = '\0';
    return str;
}

void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format == '%')
        {
            format++; // Move to the next character after '%'

            switch (*format)
            {
            case 'd':
            {
                int num = va_arg(args, int);
                char buffer[20];
                memset(buffer, 0, sizeof(buffer));
                itoa(num, buffer, 10);
                for (char *p = buffer; *p; p++) // Print until null terminator
                {
                    serial_write(*p);
                }
                break;
            }
            case 's':
            {
                const char *str = va_arg(args, const char *);
                while (*str)
                {
                    serial_write(*str++);
                }
                break;
            }
            case 'x': // Handle 32-bit hexadecimal
            {
                unsigned int num = va_arg(args, unsigned int);
                char buffer[20];
                memset(buffer, 0, sizeof(buffer));
                char *ptr = buffer + sizeof(buffer) - 1;
                *ptr = '\0'; // Null-terminate the string

                // Convert number to hex string
                do
                {
                    --ptr;
                    *ptr = "0123456789ABCDEF"[num & 0xF];
                    num >>= 4;
                } while (num > 0);

                for (char *p = ptr; *p; p++) // Print until null terminator
                {
                    serial_write(*p);
                }
                break;
            }
            case 'l': // Handle 64-bit hexadecimal (%lx)
            {
                format++; // Check if next char is 'x'
                if (*format == 'x')
                {
                    unsigned long num = va_arg(args, unsigned long);
                    char buffer[30];
                    memset(buffer, 0, sizeof(buffer));
                    char *ptr = buffer + sizeof(buffer) - 1;
                    *ptr = '\0'; // Null-terminate the string

                    // Convert number to hex string
                    do
                    {
                        --ptr;
                        *ptr = "0123456789ABCDEF"[num & 0xF];
                        num >>= 4;
                    } while (num > 0);

                    for (char *p = ptr; *p; p++) // Print until null terminator
                    {
                        serial_write(*p);
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        else
        {
            serial_write(*format); // Print the character as is
        }
        format++;
    }

    va_end(args);
}