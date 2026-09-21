#ifndef KF_RESOURCES_H
#define KF_RESOURCES_H

#include <kf/lib/types.h>
#include <kf/platform/host.hpp>
#include <kf/lib/geometry_types.h>

enum {
    KF_RESOURCE_CHUNK_HEADER_BYTES = 4,
    KF_RESOURCE_REUSE_PREFIX_BYTES = 16
};

#define RESOURCE_STREAM_NEXT(stream) \
    ((stream) += *(u32 *)(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES)

struct KfResourceChunk {
    const u8 *data;
    std::size_t size;
};

inline KfResourceChunk resource_chunk_view(const u8 *chunk, const u8 *end)
{
    if (chunk > end || end - chunk < KF_RESOURCE_CHUNK_HEADER_BYTES)
        kf::host_fail("Truncated resource chunk header");
    const std::size_t size = static_cast<u32>(chunk[0]) | (static_cast<u32>(chunk[1]) << 8)
        | (static_cast<u32>(chunk[2]) << 16) | (static_cast<u32>(chunk[3]) << 24);
    if (size > static_cast<std::size_t>(end - chunk - KF_RESOURCE_CHUNK_HEADER_BYTES))
        kf::host_fail("Truncated resource chunk payload");
    return {chunk + KF_RESOURCE_CHUNK_HEADER_BYTES, size};
}

inline u8 *resource_stream_next(u8 *chunk, const u8 *end)
{
    const auto payload = resource_chunk_view(chunk, end);
    return chunk + KF_RESOURCE_CHUNK_HEADER_BYTES + payload.size;
}

extern void tim_upload_images(const u8 *tim_data, std::size_t size);

extern const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, std::size_t word_count);

#endif
