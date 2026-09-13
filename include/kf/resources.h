#ifndef KF_RESOURCES_H
#define KF_RESOURCES_H

#include <kf/game_types.h>
#include <psyq/sdk.h>

enum {
    KF_RESOURCE_CHUNK_HEADER_BYTES = 4,
    KF_RESOURCE_REUSE_PREFIX_BYTES = 16
};

#define RESOURCE_STREAM_NEXT(stream) \
    ((stream) += *(u32 *)(stream) + KF_RESOURCE_CHUNK_HEADER_BYTES)

extern void tim_upload_images(u8 *tim_data);

extern const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count);

#endif
