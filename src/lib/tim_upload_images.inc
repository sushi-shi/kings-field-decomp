#include <kf/renderer/renderer.hpp>
#include <kf/platform/host.hpp>

void tim_upload_images(const u8 *tim_data, std::size_t size)
{
    if (!kf::texture_store_upload_tim(&kf::host_renderer()->textures, tim_data, size))
        kf::host_fail("Invalid TIM resource or texture allocation failure.");
}
