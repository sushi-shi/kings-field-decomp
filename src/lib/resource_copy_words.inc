#ifdef KF_OPEN

const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count)
#else

const u32 *map_resource_copy_words(
    u32 *destination,
    const u32 *source,
    u32 word_count)
#endif
{
    u32 *out = destination;

    while (word_count-- != 0) {
        *out++ = *source++;
    }
    return source;
}
