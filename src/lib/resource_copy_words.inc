/* The count's signedness is not observable: the loop only tests count-- != 0. */
ADDRESS_AT("GAME", 0x8001b3e4, 0x30)
ADDRESS_AT("OPEN", 0x80016318, 0x30)
const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, u32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}
