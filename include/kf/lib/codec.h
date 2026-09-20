#ifndef KF_CODEC_H
#define KF_CODEC_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
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
#endif
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
#ifdef __cplusplus
}
#endif
#endif
