#include <kf/lib/resources.h>
#ifndef KF_OPEN
#include <kf/game/resources.h>
#endif

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

#include <kf/renderer/renderer.hpp>
#include <kf/platform/host.hpp>

void tim_upload_images(const u8 *tim_data, std::size_t size)
{
    if (!kf::texture_store_upload_tim(&kf::host_renderer()->textures, tim_data, size))
        kf::host_fail("Invalid TIM resource or texture allocation failure.");
}
