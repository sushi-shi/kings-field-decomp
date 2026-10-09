const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, u32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}
