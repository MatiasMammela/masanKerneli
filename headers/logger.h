#pragma once
#include "lib.h"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
typedef enum log_lvl
{
    NOTICE,
    WARNING,
    PANIC
} log_lvl;

static inline const char *
get_log_color(log_lvl lvl)
{
    switch (lvl)
    {
    case NOTICE:
        return GREEN;
        break;
    case WARNING:
        return YELLOW;
        break;
    case PANIC:
        return RED;
        break;
    default:
        return "";
        break;
    }
}

static inline void logger(log_lvl lvl, const char *msg)
{
    printf("\n%s[KERNEL]\033[0m%s", get_log_color(lvl), msg);
}