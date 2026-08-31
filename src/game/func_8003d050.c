void func_8003d050(
    unsigned char *destination,
    const unsigned char *source,
    unsigned int count)
{
    while (count-- != 0) {
        *destination++ = *source++;
    }
}
