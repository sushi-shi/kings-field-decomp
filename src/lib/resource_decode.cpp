#include <kf/lib/codec.h>
#include <kf/game/actor.h>
#include <kf/lib/byte_reader.h>

#include <algorithm>
#include <bitset>

namespace {
using namespace kf::codec;

Reader table(Bytes bytes, std::size_t at, std::size_t count, std::size_t width,
    std::source_location location = std::source_location::current())
{
    return Reader(slice(bytes, at, product(count, width, location), location));
}

void cell(u8 z, u8 x, KfPlacementLimits limits)
{
    require(z < limits.map_side && x < limits.map_side, "placement cell exceeds map bounds");
}

void actor_position(u8 tile, s16 local, KfPlacementLimits limits)
{
    const std::int64_t position = std::int64_t(tile) * limits.tile_size + local;
    const std::uint64_t extent = std::uint64_t(limits.map_side) * limits.tile_size;
    require(position >= 0 && std::uint64_t(position) < extent, "actor position exceeds map bounds");
}

KfActorPlacementData actor(Reader input, KfPlacementLimits limits)
{
    KfActorPlacementData result {};
    result.slot_state = kf_enum_decode<KfActorSlotState>(input.byte());
    const auto definition = input.byte();
    result.definition_id = field<0, 5>(definition);
    result.near_square_culling = field<5, 1>(definition);
    result.heading_quadrant = kf_enum_decode<KfActorHeadingQuadrant>(input.byte());
    result.tile_z = input.byte();
    result.tile_x = input.byte();
    result.spawn_chance = input.byte();
    result.death_drop_object_id = kf_enum_decode<KfObjectId>(input.byte());
    input.skip(3);
    result.local_z = input.s16_le();
    result.local_x = input.s16_le();
    input.skip(2);
    cell(result.tile_z, result.tile_x, limits);
    actor_position(result.tile_z, result.local_z, limits);
    actor_position(result.tile_x, result.local_x, limits);
    require(result.slot_state <= KF_ACTOR_SLOT_HOMEBOUND && result.definition_id < limits.definitions &&
        kf_enum_encode<u8>(result.heading_quadrant) <= 3, "invalid actor placement");
    return result;
}

KfObjectPlacementData object(Reader input, KfPlacementLimits limits)
{
    KfObjectPlacementData result {};
    result.object_id = input.byte();
    input.skip(1);
    result.tile_z = input.byte();
    result.tile_x = input.byte();
    result.yaw = input.u16_le();
    result.local_z = input.s16_le();
    result.local_x = input.s16_le();
    result.local_y = input.s16_le();
    for (auto &byte : result.link)
        byte = input.byte();
    cell(result.tile_z, result.tile_x, limits);
    require(result.object_id < limits.definitions, "object definition exceeds available definitions");
    return result;
}

KfEventPlacementData event(Reader input, KfPlacementLimits limits)
{
    KfEventPlacementData result {};
    result.state = kf_enum_decode<KfMapEventState>(input.byte());
    result.character_id = kf_enum_decode<KfCharacterId>(input.byte());
    result.model_index = input.byte();
    result.cell_z = input.byte();
    result.cell_x = input.byte();
    std::ranges::copy(input.take(result.dialogue_pages.size()), result.dialogue_pages.begin());
    result.dialogue_stage_limit = input.byte();
    result.unknown_0b = input.byte();
    result.unknown_0c = input.byte();
    result.behavior = kf_enum_decode<KfMapEventBehavior>(input.byte());
    result.position_z_offset = input.s16_le();
    result.position_x_offset = input.s16_le();
    result.initial_rotation = input.u16_le();
    result.radius = input.u16_le();
    input.skip(2);
    cell(result.cell_z, result.cell_x, limits);
    require((result.state == KF_MAP_EVENT_INACTIVE || result.state == KF_MAP_EVENT_ACTIVE || result.state == KF_MAP_EVENT_DISABLED) &&
        result.model_index < limits.definitions && result.dialogue_stage_limit <= 5 &&
        result.behavior <= KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP, "invalid event placement");
    return result;
}

template<class T, class Decode>
std::size_t placements(Bytes bytes, KfPlacementLimits limits, std::span<T> output,
    std::size_t record_size, Decode record)
{
    Reader input(bytes);
    std::size_t count = 0;
    for (auto &slot : output) {
        auto peek = input;
        if (peek.byte() == 255)
            return count;
        slot = record(Reader(input.take(record_size)), limits);
        ++count;
    }
    return count;
}
} // namespace

KfAsset::KfAsset(std::span<const u8> bytes)
{
    Reader input(bytes);
    const auto encoded_size = input.u32_le();
    clip_count_ = input.u32_le();
    tmd_offset_ = input.u32_le();
    morph_table_ = input.u32_le();
    clip_table_ = input.u32_le();
    require(encoded_size >= 20 && encoded_size <= bytes.size() &&
        tmd_offset_ >= 20 && tmd_offset_ <= encoded_size && clip_count_ <= 256,
        "invalid asset header");
    bytes_ = bytes.first(encoded_size);
}

KfAnimationData KfAsset::animation(u32 vertex_count) const
{
    const auto bytes = bytes_;
    KfAnimationData result;
    result.vertex_count = vertex_count;
    if (clip_count_ == 0) {
        return result;
    }
    require(vertex_count != 0, "animated asset has no vertices");
    auto clips = table(bytes, clip_table_, clip_count_, 4);
    // Morph IDs are 16-bit on disc. Decode each referenced morph once.
    std::bitset<65536> used;
    while (clips.remaining()) {
        auto clip = reader_at(bytes, clips.u32_le());
        const auto count = clip.u16_le();
        clip.skip(2);
        require(count != 0, "animation clip has no keyframes");
        Reader offsets(clip.take(product(count, 4)));
        result.clips.push_back({result.keyframes.size(), count});
        while (offsets.remaining()) {
            auto frame = reader_at(bytes, offsets.u32_le());
            const auto reverse = frame.u16_le(), duration = frame.u16_le();
            const auto rest = frame.u16_le(), morph_count = frame.u16_le();
            Reader indices(frame.take(product(morph_count, 2)));
            // Shared offsets must not amplify a small input into unbounded storage/work.
            require(result.keyframes.size() < bytes.size() &&
                morph_count <= bytes.size() - result.indices.size(), "animation keyframes exceed input budget");
            result.keyframes.push_back({reverse, duration, rest, result.indices.size(), morph_count});
            used.set(rest);
            while (indices.remaining()) {
                const auto id = indices.u16_le();
                used.set(id);
                result.indices.push_back(id);
            }
        }
    }
    // Slice the table tail first, so 32-bit targets cannot wrap base + id * 4.
    auto morph_table = reader_at(bytes, morph_table_);
    for (std::size_t id = 0; id < used.size(); ++id) {
        if (!used.test(id))
            continue;
        auto entry = morph_table;
        entry.skip(id * 4);
        auto morph = reader_at(bytes, entry.u32_le());
        morph.skip(4);
        const auto base = morph.u32_le(), count = morph.u32_le();
        require(base <= vertex_count && count <= vertex_count - base, "morph vertices exceed model bounds");
        Reader deltas(morph.take(product(count, 8)));
        require(count <= bytes.size() - result.deltas.size(), "animation deltas exceed input budget");
        result.morphs.resize(id + 1);
        result.morphs[id] = {base, result.deltas.size(), count};
        while (deltas.remaining()) {
            result.deltas.push_back({deltas.s16_le(), deltas.s16_le(), deltas.s16_le()});
            deltas.skip(2);
        }
    }
    return result;
}

std::size_t kf_actor_placements_decode(std::span<const u8> bytes, KfPlacementLimits limits,
    std::span<KfActorPlacementData> output)
{
    return placements(bytes, limits, output, 16, actor);
}

std::size_t kf_object_placements_decode(std::span<const u8> bytes, KfPlacementLimits limits,
    std::span<KfObjectPlacementData> output)
{
    return placements(bytes, limits, output, 20, object);
}

std::size_t kf_event_placements_decode(std::span<const u8> bytes, KfPlacementLimits limits,
    std::span<KfEventPlacementData> output)
{
    return placements(bytes, limits, output, 24, event);
}
