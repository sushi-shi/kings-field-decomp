#include <kf/address.h>
#include <kf/game_types.h>

ADDRESS(0x8001b3e4, 0x30)
const u32 *map_resource_copy_words(
    u32 *destination,
    const u32 *source,
    u32 word_count)
{
    while (word_count-- != 0) {
        *destination++ = *source++;
    }
    return source;
}
