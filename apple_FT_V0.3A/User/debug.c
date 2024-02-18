#include "bsp.h"
#include "debug.h"




void debug(const char *fmt, ...)
{
    //printf(fmt, ##__VA_ARGS__)
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
