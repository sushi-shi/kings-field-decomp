#ifndef KF_CODEC_H
#define KF_CODEC_H

#include <kf/lib/byte_reader.h>
#include <kf/lib/types.h>

#include <array>
#include <optional>
#include <span>
#include <utility>
#include <vector>

struct KfTimInfo {
    u32 mode, width, height, encoded_bytes;
    s32 image_x, image_y, palette_x, palette_y;
};
// Parsed views borrow immutable input; output storage must be disjoint.
class KfTimImage {
public:
    static std::optional<KfTimImage> read(kf::codec::Reader &input);
    static std::optional<KfTimImage> parse(std::span<const u8> bytes, std::size_t offset = 0);
    KfTimInfo info() const;
    void rgba(u32 palette_row, std::span<u8> output) const;
private:
    enum class PixelFormat : u8 { Indexed4, Indexed8, Direct16, Direct24 };
    struct Block {
        s16 x, y;
        u16 width, height;
        std::span<const u8> pixels;
    };
    KfTimImage() = default;
    static Block read_block(kf::codec::Reader &input);
    static void validate_rectangle(const Block &block);
    static void copy_block(const Block &block, std::span<u16> words);
    std::pair<u32, u32> dimensions() const;
    PixelFormat format_;
    std::optional<Block> clut_;
    Block pixels_;
    u32 encoded_bytes_;
    friend void kf_tim_compose(std::span<const u8>, std::span<u16>);
};
// Compose authored TIM rectangles into a temporary 1024x512 word image for
// material conversion. STP is retained; this is not runtime emulated VRAM.
void kf_tim_compose(std::span<const u8> bytes, std::span<u16> words);

struct KfAnimationClipData {
    std::size_t first_keyframe, keyframe_count;
};
struct KfAnimationKeyframe {
    u16 reverse, duration, rest_morph;
    std::size_t first_morph, morph_count;
};
struct KfAnimationMorph {
    u32 base_vertex;
    std::size_t first_delta, delta_count;
};
struct KfAnimationDelta {
    s16 x, y, z;
};
struct KfAnimationData {
    std::vector<KfAnimationClipData> clips;
    std::vector<KfAnimationKeyframe> keyframes;
    std::vector<KfAnimationMorph> morphs;
    std::vector<u16> indices;
    std::vector<KfAnimationDelta> deltas;
    u32 vertex_count = 0;
};
// A validated borrowed resource view. The backing bytes must outlive the view.
class KfAsset {
public:
    explicit KfAsset(std::span<const u8> bytes);
    std::size_t encoded_size() const { return bytes_.size(); }
    u32 tmd_offset() const { return tmd_offset_; }
    std::span<const u8> tmd() const { return bytes_.subspan(tmd_offset_); }
    bool animated() const { return clip_count_ != 0; }
    KfAnimationData animation(u32 vertex_count) const;
private:
    std::span<const u8> bytes_;
    u32 tmd_offset_, clip_count_, morph_table_, clip_table_;
};

enum class KfActorSlotState : u8;
enum class KfActorHeadingQuadrant : u8;
enum class KfObjectId : u8;
enum class KfMapEventState : u8;
enum class KfCharacterId : u8;
enum class KfMapEventBehavior : u8;

struct KfPlacementLimits {
    u32 map_side, definitions, tile_size;
};
struct KfActorPlacementData {
    KfActorSlotState slot_state;
    u8 definition_id;
    bool near_square_culling;
    KfActorHeadingQuadrant heading_quadrant;
    u8 tile_z, tile_x, spawn_chance;
    KfObjectId death_drop_object_id;
    s16 local_z, local_x;
};
struct KfObjectPlacementData {
    u8 object_id, tile_z, tile_x;
    u16 yaw;
    s16 local_z, local_x, local_y;
    std::array<u32, 2> link;
};
struct KfEventPlacementData {
    KfMapEventState state;
    KfCharacterId character_id;
    u8 model_index, cell_z, cell_x;
    std::array<u8, 5> dialogue_pages;
    u8 dialogue_stage_limit, unknown_0b, unknown_0c;
    KfMapEventBehavior behavior;
    s16 position_z_offset, position_x_offset;
    u16 initial_rotation, radius;
};
// Decoded lists have explicit counts; the on-disc sentinel is not returned.
std::size_t kf_actor_placements_decode(std::span<const u8> bytes,
    KfPlacementLimits limits, std::span<KfActorPlacementData> output);
std::size_t kf_object_placements_decode(std::span<const u8> bytes,
    KfPlacementLimits limits, std::span<KfObjectPlacementData> output);
std::size_t kf_event_placements_decode(std::span<const u8> bytes,
    KfPlacementLimits limits, std::span<KfEventPlacementData> output);

#endif // KF_CODEC_H
