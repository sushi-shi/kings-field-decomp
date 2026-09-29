const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, u32 word_count)
{
    u32 *out = destination;

    while (word_count-- != 0) {
        *out++ = *source++;
    }
    return source;
}
