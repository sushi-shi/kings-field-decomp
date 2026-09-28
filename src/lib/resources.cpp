#include <kf/platform/prelude.hpp>
#include <kf/lib/resources.h>

const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, std::size_t word_count)
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
