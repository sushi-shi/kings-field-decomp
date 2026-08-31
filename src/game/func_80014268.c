void func_80014268(volatile int *destination, int count, int value)
{
    while (count-- != 0) {
        *destination = value;
    }
}
