void func_8003b4b8(unsigned char *destination, const unsigned char *source)
{
    int count;

    if (destination != 0) {
        for (count = 7; count >= 0; --count) {
            *destination++ = *source++;
        }
    }
}
