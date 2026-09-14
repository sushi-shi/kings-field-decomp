/* The two entry points retain their image-specific count signedness. */
#ifdef KF_OPEN
ADDRESS_AT("OPEN", 0x80016318, 0x30)
const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count)
#else
ADDRESS_AT("GAME", 0x8001b3e4, 0x30)
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
