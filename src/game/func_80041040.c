void func_80041040(
    unsigned char *destination,
    const unsigned char *source,
    unsigned int count)
{
    while (count-- != 0) {
        *destination++ = *source++;
    }
}
