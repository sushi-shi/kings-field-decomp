typedef signed short s16;

int func_80014f6c(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & 0xfff;

    return range < delta && 0x1000 - range <= delta;
}
