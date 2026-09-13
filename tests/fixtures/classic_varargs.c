#include <stdarg.h>

int check_one(int tag, ...)
{
    va_list args;
    int signed_value;
    unsigned int unsigned_value;
    int *pointer;
    unsigned long long wide;
    va_start(args, tag);
    signed_value = va_arg(args, int);
    unsigned_value = va_arg(args, unsigned int);
    pointer = va_arg(args, int *);
    wide = va_arg(args, unsigned long long);
    va_end(args);
    return tag != 1 || signed_value != -7 || unsigned_value != 0x89abcdefU
        || *pointer != 37 || wide != 0x1122334455667788ULL;
}

int check_three(int a, int b, int c, ...)
{
    va_list args;
    int value;
    unsigned long long wide;
    va_start(args, c);
    value = va_arg(args, int);
    wide = va_arg(args, unsigned long long);
    va_end(args);
    return a != 1 || b != 2 || c != 3 || value != 32767
        || wide != 0x8877665544332211ULL;
}

int check_five(int a, int b, int c, int d, int e, ...)
{
    va_list args;
    unsigned long long wide;
    int value;
    va_start(args, e);
    wide = va_arg(args, unsigned long long);
    value = va_arg(args, int);
    va_end(args);
    return a != 1 || b != 2 || c != 3 || d != 4 || e != 5
        || wide != 0x123456789abcdef0ULL || value != -19;
}

int control_entry(void)
{
    signed char small = -7;
    short medium = 32767;
    int pointed = 37;
    return check_one(1, small, 0x89abcdefU, &pointed, 0x1122334455667788ULL)
        | (check_three(1, 2, 3, medium, 0x8877665544332211ULL) << 1)
        | (check_five(1, 2, 3, 4, 5, 0x123456789abcdef0ULL, -19) << 2);
}
