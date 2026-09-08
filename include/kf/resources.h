#ifndef KF_RESOURCES_H
#define KF_RESOURCES_H

/* Resource-stream helpers shared by the GAME and OPEN overlays. */

#include <kf/game_types.h>
#include <kf/psyq.h>

enum {
    KF_RESOURCE_CHUNK_HEADER_BYTES = 4,
    KF_RESOURCE_REUSE_PREFIX_BYTES = 16
};

/* One allocation slot with generic output and typed resource views. */
typedef union KfResourcePointer {
    void *storage;
    u8 *bytes;
    u_long *tim_data;
} KfResourcePointer;

typedef char check_resource_pointer_size[sizeof(KfResourcePointer) == 4 ? 1 : -1];

extern void tim_upload_images(u_long *tim_data);
extern const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count);

#endif
