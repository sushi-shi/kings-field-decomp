#include <kf/net/world.h>
#include <kf/lib/avatar.h>
#include <kf/lib/byte_reader.h>

#include <algorithm>
#include <bit>
#include <memory>

namespace {
void require(bool value) { kf::codec::require(value, "invalid world snapshot"); }
template<class T> bool aligned(const T *value)
{ return value && reinterpret_cast<std::uintptr_t>(value) % alignof(T) == 0; }
template<class Function> KfCodecResult checked(Function function)
{
    try { function(); return KF_CODEC_OK; }
    catch (const kf::codec::Error &error) { return error.result; }
}
bool optional_index(u8 id, u8 count) { return id == 255 || id < count; }
bool position(s32 x, s32 z) { return x >= 0 && x < 200000 && z >= 0 && z < 200000; }
bool coordinate(s32 v) { return v >= -1000000 && v <= 1000000; }
void header_valid(const KfNetWorldHeader &h)
{
    require(h.full <= 1 && h.epoch && ((h.floor >= 1 && h.floor <= 4 && h.variant == 0) ||
        (h.floor == 5 && h.variant >= 1 && h.variant <= 3)));
}

#include "world_fields.h"

template<class IO, class T> requires std::is_same_v<std::remove_const_t<T>, KfNetWorldHeader>
void fields(IO &io, T &v)
{
    u32 magic = 0x3153464b; u16 version = 11;
    io.value(magic); io.value(version);
    require(magic == 0x3153464b && version == 11);
    io.value(v.full); io.value(v.epoch); io.value(v.tick); io.value(v.floor); io.value(v.variant);
    header_valid(v);
}
template<class IO, class T> requires std::is_same_v<std::remove_const_t<T>, KfNetWorldMember>
void fields(IO &io, T &v)
{
    io.value(v.presence); io.value(v.character_id); io.value(v.generation); io.value(v.acknowledged_input);
    io.value(v.quest_rewards); io.value(v.connected); io.value(v.avatar); io.value(v.loot_claims);
    require(v.presence <= 4 && v.connected <= 1);
    if (v.presence >= 2) io.value(v.player);
}
template<class IO, class T> requires std::is_same_v<std::remove_const_t<T>, KfNetWorld>
void fields(IO &io, T &v)
{
    io.value(v.header); io.value(v.quest_rewards); io.value(v.members); io.value(v.actors);
    io.value(v.action_animations); io.value(v.effects); io.value(v.objects); io.value(v.events);
    io.value(v.gold_drop_sequence); io.value(v.definition_drop_sequence); io.value(v.placement_drop_sequence);
    io.value(v.dialogue_advance_gate); io.value(v.ambient_script_countdown);
    for (auto &floor : v.floors) { io.value(floor.script); if (v.header.full) io.value(floor.records); }
    io.value(v.sound_sequence); io.value(v.sounds);
    io.grid(v.collision_flags); io.grid(v.cell_orientation); io.grid(v.floor_height);
    io.grid(v.collision); io.grid(v.cell_attribute); io.value(v.random_state); io.value(v.story);
}

class WorldReader {
public:
    explicit WorldReader(std::span<const u8> bytes) : reader_(bytes) { require(bytes.size() <= KF_NET_TRANSFER_LIMIT); }
    template<class T> void value(T &v) {
        if constexpr (std::is_integral_v<T>) {
            using U = std::make_unsigned_t<T>;
            U bits = 0; unsigned shift = 0;
            for (u8 byte : reader_.take(sizeof(T))) { bits |= U(byte) << shift; shift += 8; }
            v = std::bit_cast<T>(bits);
        } else if constexpr (std::is_array_v<T>) {
            for (auto &element : v) value(element);
        } else fields(*this, v);
    }
    void grid(std::span<u8> cells) {
        std::size_t at = 0;
        while (at < cells.size()) {
            auto count = reader_.u16_le(); auto byte = reader_.byte();
            require(count && count <= cells.size() - at);
            std::fill_n(cells.begin() + at, count, byte); at += count;
        }
    }
    bool complete() const { return reader_.remaining() == 0; }
private:
    kf::codec::Reader reader_;
};
class WorldWriter {
public:
    std::vector<u8> bytes;
    template<class T> void value(const T &v) {
        if constexpr (std::is_integral_v<T>) {
            kf::codec::output_fits(sizeof(T) <= KF_NET_TRANSFER_LIMIT - bytes.size());
            auto bits = static_cast<std::make_unsigned_t<T>>(v);
            for (unsigned i = 0; i < sizeof(T); ++i) bytes.push_back(static_cast<u8>(bits >> (8 * i)));
        } else if constexpr (std::is_array_v<T>) {
            for (const auto &element : v) value(element);
        } else fields(*this, v);
    }
    void grid(std::span<const u8> cells) {
        std::size_t at = 0;
        while (at < cells.size()) {
            u16 count = 1; auto byte = cells[at];
            while (count < UINT16_MAX && count < cells.size() - at && cells[at + count] == byte) ++count;
            value(count); value(byte); at += count;
        }
    }
};

bool clip_valid(const KfNetWorldLimits &limits, std::size_t asset, u8 clip)
{
    return asset < std::size(limits.asset_clips) &&
        (limits.asset_clips[asset] == 0 || (limits.asset_clips[asset] > 0 && clip < limits.asset_clips[asset]));
}
int actor_action_clip(u8 action)
{
    if (action == 0) return 0;
    if ((action >= 1 && action <= 3) || action == 32 || action == 33) return 1;
    if (action >= 4 && action <= 6) return action - 2;
    if (action >= 16 && action <= 22) return action - 11;
    return -1;
}
int billboard_frames(u8 kind)
{
    if (kind == 4) return 2;
    if (kind == 5) return 5;
    if (kind == 7 || (kind >= 10 && kind <= 12)) return 1;
    if (kind == 19 || kind == 32) return 3;
    return 0;
}
bool link_valid(u8 operation, std::span<const u8> bytes)
{
    if (operation <= 1) return optional_index(bytes[1], 190);
    if (operation == 10) return optional_index(bytes[1], 5);
    if (operation >= 81 && operation <= 83) return bytes[1] < 48;
    if (operation == 8) return std::all_of(bytes.begin() + 1, bytes.begin() + 5, [](u8 id) { return optional_index(id, 80); });
    if (operation == 9) return std::all_of(bytes.begin(), bytes.begin() + 4, [](u8 id) { return optional_index(id, 80); });
    return true;
}
bool event_state(u8 v) { return v == 0 || v == 1 || v == 3 || v == 255; }
void saved_floor_valid(std::span<const u8> bytes, const KfNetWorldLimits *limits)
{
    kf::codec::Reader r(bytes);
    const auto present = r.byte();
    if (!present) return;
    require(present == 1);
    for (unsigned i = 0; i < 8; ++i) {
        auto e = r.take(7);
        require(event_state(e[0]) && e[1] <= 5 && e[2] <= 5 && (e[0] == 255 || e[2] != 0));
    }
    auto actors = r.byte(); require(actors <= 128);
    for (unsigned i = 0; i < actors; ++i) { auto a = r.take(2); require(a[0] < 128 && (a[1] == 0 || a[1] == 3)); }
    auto ids = r.take(190);
    for (unsigned i = 0; i < ids.size(); ++i)
        require(optional_index(ids[i], 160) && (ids[i] == 255 || i < 160 || (i < 170 ? ids[i] == 39 : ids[i] < 80)));
    auto objects = r.byte(); require(objects <= 160);
    for (unsigned i = 0; i < objects; ++i) {
        auto index = r.byte(); require(index < 160); auto id = ids[index];
        require(id < KF_WORLD_OBJECT_DEFINITIONS);
        auto link = r.take(8);
        if (limits) require(link_valid(limits->object_operations[id], link));
    }
    for (unsigned group = 0; group < 3; ++group) for (unsigned i = 0; i < 10; ++i) {
        require(r.byte() < 100 && r.byte() < 100); r.skip(group == 0 ? 2 : 1);
    }
}
void validate_party(const KfNetWorld &world)
{
    header_valid(world.header);
    for (unsigned slot = 0; slot < KF_WORLD_SOUNDS; ++slot) {
        const auto &s = world.sounds[slot];
        const auto age = (world.sound_sequence % KF_WORLD_SOUNDS + KF_WORLD_SOUNDS - slot) % KF_WORLD_SOUNDS;
        const auto expected = world.sound_sequence > age ? world.sound_sequence - age : 0;
        require(s.sequence == expected);
        if (!expected) continue;
        require(s.program <= 127 && s.tone <= 15 && s.note <= 127 && (s.program || s.tone || s.note) &&
            s.volume >= 1 && s.volume <= 127 && s.max_distance >= 1 && s.max_distance <= 60000 &&
            s.attenuation_distance >= s.max_distance && s.attenuation_distance <= 60000 &&
            coordinate(s.x) && coordinate(s.y) && coordinate(s.z));
    }
    const auto &s = world.story;
    require(s.kind <= 4);
    if (s.kind == 4) require((world.header.floor == 1 || world.header.floor == 5) && world.floors[4].script[3] == 1 &&
        s.initiator == 0 && s.effect == 255 && s.page == 0 && s.tick == 0 && s.ready_mask < 16 && (s.ready_mask & 1));
    else if (s.kind == 3) {
        require(world.header.floor == 5 && s.page <= 2 && s.ready_mask < 16 && s.effect == 255);
        require(s.page == 0 ? s.tick < 2 && s.ready_mask == 0 : s.tick == 0);
    } else require(s.page == 0 && s.ready_mask == 0);
    if (s.kind && s.effect != 255) {
        require(s.kind == 2 && s.tick >= 361 && s.tick < 373 && s.effect < 48);
        const auto &e = world.effects[s.effect];
        require(e.slot_type == 0 && e.kind == 18 && e.phase == s.tick - 360);
    }
    if (s.kind) {
        require(s.initiator < 4 && world.members[s.initiator].presence >= 2 && position(s.camera_x, s.camera_z) && coordinate(s.camera_y));
        if (s.kind == 1) require(world.header.floor == 2 && s.tick < 50);
        else if (s.kind == 2) {
            require(world.header.floor == 5 && s.tick <= 620 && s.object >= 180 && s.object < 190 && s.generation);
            const auto &o = world.objects[s.object];
            require(o.generation == s.generation && o.object_id == (s.tick <= 100 ? 255 : s.tick <= 360 ? 10 : 11));
        }
    }
    require((world.quest_rewards & ~KF_WORLD_QUEST_REWARD_MASK) == 0);
    for (unsigned slot = 0; slot < 4; ++slot) {
        const auto &m = world.members[slot];
        if (std::any_of(std::begin(m.character_id), std::end(m.character_id), [](u8 b) { return b != 0; }))
            for (unsigned prior = 0; prior < slot; ++prior)
                require(!std::equal(std::begin(m.character_id), std::end(m.character_id), world.members[prior].character_id));
        require((m.quest_rewards & ~world.quest_rewards) == 0 && m.presence <= 4 && m.connected <= 1 && m.avatar < KF_AVATAR_SLOTS);
        for (const auto &claims : m.loot_claims) for (auto mask : claims) require(mask < 16);
        if (m.presence < 2) continue;
        const auto &p = m.player;
        require(p.party_slot == slot && p.cast_pose_ticks <= KF_WORLD_CAST_POSE_TICKS && p.progress_state_level &&
            p.progress_state_current_floor == world.header.floor && p.map_variant == world.header.variant &&
            p.progress_state_highest_floor >= 1 && p.progress_state_highest_floor <= 5 && p.vitals_maximum_hp && p.vitals_maximum_mp &&
            p.vitals_current_hp <= p.vitals_maximum_hp && p.vitals_current_mp <= p.vitals_maximum_mp &&
            optional_index(p.equipped_weapon_id, 16) && optional_index(p.selected_magic_id, 9) && optional_index(p.equipped_accessory_id, 80) &&
            position(p.camera_position_vx, p.camera_position_vz) && coordinate(p.camera_position_vy) && coordinate(p.foot_height) &&
            p.motion_state_map_cell_x < 100 && p.motion_state_map_cell_z < 100);
        for (auto learned : p.learned_magic) require(learned <= 1);
        for (auto id : {p.equipped_head_armor_id, p.equipped_body_armor_id, p.equipped_shield_id, p.equipped_arm_armor_id, p.equipped_leg_armor_id})
            require(id == 255 || (id >= 13 && id < 55));
    }
}
void validate(const KfNetWorld &world, const KfNetWorldLimits *limits)
{
    validate_party(world);
    for (unsigned i = 0; i < std::size(world.collision); ++i) {
        auto a = world.cell_attribute[i], o = world.cell_orientation[i];
        require(a < 1 || a > 100 || (o >= 1 && o <= 4)); require(world.collision[i] <= 6);
    }
    for (const auto &a : world.actors) {
        if (a.slot_state == 255) continue;
        require(a.slot_state <= 3 && a.definition_id < 12 && a.lifecycle <= 3 && a.cell_x < 100 && a.cell_z < 100 &&
            position(a.position_vx, a.position_vz) && coordinate(a.position_vy) && optional_index(a.target_player_slot, 4) &&
            a.culling_mode <= 1 && a.heading_quadrant <= 3 && a.vertical_state <= 4 && a.collision_state <= 2 &&
            optional_index(a.death_drop_object_id, 160) && (a.action <= 6 || (a.action >= 16 && a.action <= 22) ||
                a.action == 32 || a.action == 33 || a.action == 127 || a.action == 255));
        if (a.slot_state) require(a.tile_x < 100 && a.tile_z < 100 && position(s32(a.tile_x) * 2000 + a.local_x, s32(a.tile_z) * 2000 + a.local_z));
        if (!limits) continue;
        auto asset = limits->actor_assets[a.definition_id]; require(clip_valid(*limits, asset, a.animation_clip));
        if (a.lifecycle == 1 && a.action_progress == 0) {
            const int clip = actor_action_clip(a.action);
            if (clip >= 0) require(clip_valid(*limits, asset, world.action_animations[a.definition_id][clip]));
        }
    }
    if (limits) for (unsigned definition = 0; definition < std::size(world.action_animations); ++definition) {
        auto asset = limits->actor_assets[definition];
        const bool absent = asset >= std::size(limits->asset_clips) || limits->asset_clips[asset] < 0;
        for (auto clip : world.action_animations[definition]) require(clip == 255 || (absent ? clip == 0 : clip_valid(*limits, asset, clip)));
    }
    for (const auto &e : world.effects) {
        if (e.slot_type == 255) continue;
        require((e.kind <= 24 || (e.kind >= 32 && e.kind <= 34) || e.kind == 36 || e.kind == 41 || e.kind == 42 ||
            e.kind == 44 || e.kind == 48 || e.kind == 52) && optional_index(e.owner_player_slot, 4) && optional_index(e.target_player_slot, 4));
        if (e.kind == 19) require(e.control < 48);
        if (int frames = billboard_frames(e.kind)) require(e.animation_clip == 255 && e.base_render_id <= 22 - frames &&
            e.render_id >= e.base_render_id && e.render_id < e.base_render_id + frames);
        if (e.kind == 52) {
            require(e.rotation_vector_vx >= 0 && e.rotation_vector_vy >= 0 && s32(e.rotation_vector_vx) + e.rotation_vector_vy <= 5);
            require(e.position_vx >= 0 && e.position_vx <= 32767 && e.position_vy >= -1);
        } else require(coordinate(e.position_vx) && coordinate(e.position_vy) && coordinate(e.position_vz));
        if (e.render_id != 255) {
            if (e.animation_clip == 255) require(e.render_id < 22 && optional_index(e.base_render_id, 22));
            else {
                require(std::size_t(e.render_id) + 30 < KF_WORLD_ASSETS);
                if (limits) require(clip_valid(*limits, std::size_t(e.render_id) + 30, e.animation_clip));
            }
        }
    }
    for (const auto &o : world.objects) {
        if (o.object_id == 255) continue;
        require(o.generation && o.object_id < 160 && o.cell_x < 100 && o.cell_z < 100 && position(o.position_vx, o.position_vz) && coordinate(o.position_vy));
        auto operation = limits ? limits->object_operations[o.object_id] : 255;
        if (operation == 0 || operation == 2 || operation == 3) require(o.cell_x >= 1 && o.cell_x < 99 && o.cell_z >= 1 && o.cell_z < 99);
        std::array<u8, 8> link;
        for (unsigned i = 0; i < 8; ++i) link[i] = static_cast<u8>(o.link_words[i / 4] >> ((i % 4) * 8));
        require(link_valid(o.action, link) && link_valid(operation, link));
    }
    for (const auto &e : world.events) {
        require(event_state(e.state)); if (e.state != 1) continue;
        require(e.cell_x < 100 && e.cell_z < 100 && position(e.reference_position_vx, e.reference_position_vz) &&
            coordinate(e.reference_position_vy) && position(e.home_x, e.home_z) && e.dialogue_stage >= 1 &&
            e.dialogue_stage <= 5 && e.dialogue_stage_limit <= 5 && e.behavior <= 2 && std::size_t(e.model_index) + 10 < KF_WORLD_ASSETS &&
            (!limits || clip_valid(*limits, std::size_t(e.model_index) + 10, e.animation_clip)));
        if (e.behavior == 0) require(e.character_id >= 1 && e.character_id <= 2);
    }
    if (world.header.full) for (const auto &floor : world.floors) saved_floor_valid(floor.records, limits);
}
}

KfCodecResult kf_net_world_info(const u8 *data, std::size_t size, KfNetWorldHeader *out)
{
    if (!data || !aligned(out) || size > KF_NET_TRANSFER_LIMIT) return KF_CODEC_INVALID;
    return checked([&] { WorldReader r({data, size}); KfNetWorldHeader h {}; r.value(h); *out = h; });
}
KfCodecResult kf_net_world_preflight(const u8 *data, std::size_t size, KfNetWorldHeader *out)
{
    if (!data || !aligned(out) || size > KF_NET_TRANSFER_LIMIT) return KF_CODEC_INVALID;
    return checked([&] {
        auto scratch = std::make_unique<KfNetWorld>();
        WorldReader r({data, size}); r.value(*scratch); require(r.complete());
        validate(*scratch, nullptr);
        *out = scratch->header;
    });
}
KfCodecResult kf_net_world_decode(const u8 *data, std::size_t size, const KfNetWorldLimits *limits, KfNetWorld *out)
{
    if (!data || !aligned(limits) || !aligned(out) || size > KF_NET_TRANSFER_LIMIT) return KF_CODEC_INVALID;
    return checked([&] {
        // Allocate directly on the heap; never place a world on the browser stack.
        auto scratch = std::make_unique<KfNetWorld>();
        WorldReader r({data, size}); r.value(*scratch); require(r.complete()); validate(*scratch, limits);
        *out = *scratch;
    });
}
KfCodecResult kf_net_world_encode(const KfNetWorld *input, const KfNetWorldLimits *limits,
    u8 *data, std::size_t capacity, std::size_t *written)
{
    if (!aligned(input) || !aligned(limits) || !data || !aligned(written) || capacity > PTRDIFF_MAX) return KF_CODEC_INVALID;
    return checked([&] {
        validate(*input, limits); WorldWriter w; w.value(*input);
        kf::codec::output_fits(w.bytes.size() <= capacity);
        std::copy(w.bytes.begin(), w.bytes.end(), data); *written = w.bytes.size();
    });
}
KfCodecResult kf_net_world_summary(const u8 *data, std::size_t size, KfNetWorldSummary *out)
{
    if (!data || !aligned(out) || size > KF_NET_TRANSFER_LIMIT) return KF_CODEC_INVALID;
    return checked([&] {
        auto scratch = std::make_unique<KfNetWorld>(); WorldReader r({data, size}); r.value(*scratch);
        require(r.complete() && scratch->header.full == 1); validate_party(*scratch);
        const auto &host = scratch->members[0];
        require(host.presence >= 2 && std::any_of(std::begin(host.character_id), std::end(host.character_id), [](u8 b) { return b != 0; }));
        const auto &p = host.player; require(p.experience >= 0);
        KfNetWorldSummary summary {};
        summary.experience = p.experience; summary.hp = p.vitals_current_hp; summary.maximum_hp = p.vitals_maximum_hp;
        summary.mp = p.vitals_current_mp; summary.maximum_mp = p.vitals_maximum_mp;
        summary.floor = scratch->header.floor; summary.level = p.progress_state_level;
        std::copy(std::begin(host.character_id), std::end(host.character_id), summary.owner); *out = summary;
    });
}
