#ifndef KF_RESOURCES_H
#define KF_RESOURCES_H

/* Resource-stream helpers shared by the GAME and OPEN overlays. */

#include <kf/game_types.h>
#include <kf/psyq.h>

extern void tim_upload_images(u_long *tim_data);
extern const u32 *resource_stream_copy_words(
    u32 *destination, const u32 *source, s32 word_count);

#endif
