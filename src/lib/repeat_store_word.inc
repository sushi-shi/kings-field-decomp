ADDRESS_AT("GAME", 0x80014268, 0x24)
ADDRESS_AT("OPEN", 0x80013734, 0x24)
void repeat_store_word(int *destination, int count, int value)
{
    /* Retail stores to the same word on every iteration. */
    while (count-- != 0) {
        *destination = value;
    }
}
