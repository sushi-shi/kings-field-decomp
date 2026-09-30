#ifndef KF_CODEC_H
#define KF_CODEC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#ifdef __cplusplus
enum class KfCodecResult : int32_t {
    KF_CODEC_OK = 0,
    KF_CODEC_END = 1,
    KF_CODEC_INVALID = 2,
    KF_CODEC_OUTPUT_FULL = 3
};
using enum KfCodecResult;
#else
typedef enum KfCodecResult {
    KF_CODEC_OK = 0,
    KF_CODEC_END = 1,
    KF_CODEC_INVALID = 2,
    KF_CODEC_OUTPUT_FULL = 3
} KfCodecResult;
#endif // __cplusplus
typedef struct KfTimInfo {
    uint32_t mode, width, height, encoded_bytes;
    int32_t image_x, image_y, palette_x, palette_y;
} KfTimInfo;
/* Buffers are caller-owned and disjoint; offsets are byte offsets. */
KfCodecResult kf_tim_info(const uint8_t *bytes, size_t length, size_t offset, KfTimInfo *info);
KfCodecResult kf_tim_rgba(const uint8_t *bytes, size_t length, size_t offset, uint32_t palette_row,
                          uint8_t *rgba, size_t capacity);
// Compose authored TIM rectangles into a temporary 1024x512 word image for
// material conversion. STP is retained; this is not runtime emulated VRAM.
KfCodecResult kf_tim_compose(const uint8_t *bytes, size_t length, uint16_t *words, size_t capacity);

typedef struct KfAssetInfo {
    uint32_t encoded_bytes, tmd_offset, clip_count;
} KfAssetInfo;
typedef struct KfAnimationClipData {
    size_t first_keyframe, keyframe_count;
} KfAnimationClipData;
typedef struct KfAnimationKeyframe {
    uint16_t reverse, duration, rest_morph;
    size_t first_morph, morph_count;
} KfAnimationKeyframe;
typedef struct KfAnimationMorph {
    uint32_t base_vertex;
    size_t first_delta, delta_count;
} KfAnimationMorph;
typedef struct KfAnimationDelta {
    int16_t x, y, z;
} KfAnimationDelta;
typedef struct KfAnimationSizes {
    size_t clips, keyframes, morphs, indices, deltas;
} KfAnimationSizes;
typedef struct KfAnimationOutput {
    KfAnimationClipData *clips;
    KfAnimationKeyframe *keyframes;
    KfAnimationMorph *morphs;
    uint16_t *indices;
    KfAnimationDelta *deltas;
    KfAnimationSizes capacity;
} KfAnimationOutput;
KfCodecResult kf_asset_info(const uint8_t *bytes, size_t length, KfAssetInfo *info);
// Measure validates every referenced record. Decode copies into disjoint,
// initialized caller-owned buffers; no output retains a pointer into the encoded resource.
KfCodecResult kf_animation_measure(const uint8_t *bytes, size_t length,
    uint32_t vertex_count, KfAnimationSizes *sizes);
KfCodecResult kf_animation_decode(const uint8_t *bytes, size_t length,
    uint32_t vertex_count, KfAnimationOutput *output);

typedef struct KfPlacementLimits {
    uint32_t map_side, definitions, tile_size;
} KfPlacementLimits;
typedef struct KfActorPlacementData {
    uint8_t slot_state, definition_id, near_square_culling, heading_quadrant;
    uint8_t tile_z, tile_x, spawn_chance, death_drop_object_id;
    int16_t local_z, local_x;
} KfActorPlacementData;
typedef struct KfObjectPlacementData {
    uint8_t object_id, tile_z, tile_x;
    uint16_t yaw;
    int16_t local_z, local_x, local_y;
    uint32_t link[2];
} KfObjectPlacementData;
typedef struct KfEventPlacementData {
    uint8_t state, character_id, model_index, cell_z, cell_x;
    uint8_t dialogue_pages[5], dialogue_stage_limit, unknown_0b, unknown_0c, behavior;
    int16_t position_z_offset, position_x_offset;
    uint16_t initial_rotation, radius;
} KfEventPlacementData;
// Decoded lists have explicit counts; the on-disc sentinel is not returned.
KfCodecResult kf_actor_placements_decode(const uint8_t *bytes, size_t length,
    KfPlacementLimits limits, KfActorPlacementData *output, size_t capacity, size_t *count);
KfCodecResult kf_object_placements_decode(const uint8_t *bytes, size_t length,
    KfPlacementLimits limits, KfObjectPlacementData *output, size_t capacity, size_t *count);
KfCodecResult kf_event_placements_decode(const uint8_t *bytes, size_t length,
    KfPlacementLimits limits, KfEventPlacementData *output, size_t capacity, size_t *count);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // KF_CODEC_H
