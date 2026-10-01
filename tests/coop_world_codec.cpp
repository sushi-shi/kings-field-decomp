#include <kf/net/world.h>
#include <kf/lib/avatar.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <source_location>
#include <type_traits>

namespace {
void require(bool value, const char *message,
    std::source_location where = std::source_location::current())
{
    if (!value) {
        std::fprintf(stderr, "%s:%u: %s\n", where.file_name(), where.line(), message);
        std::abort();
    }
}

// Malformed fixtures must bypass validation. Share field order, but call only
// the public codec APIs under test; the golden header and optional wire corpus
// also allow comparison with an independently linked implementation.
#include "../src/net/world_fields.h"

template<class IO> void fields(IO &io, const KfNetWorldHeader &v)
{
    io.value(u32(0x3153464b)); io.value(u16(11)); io.value(v.full);
    io.value(v.epoch); io.value(v.tick); io.value(v.floor); io.value(v.variant);
}
template<class IO> void fields(IO &io, const KfNetWorldMember &v)
{
    io.value(v.presence); io.value(v.character_id); io.value(v.generation);
    io.value(v.acknowledged_input); io.value(v.quest_rewards); io.value(v.connected);
    io.value(v.avatar); io.value(v.loot_claims);
    if (v.presence >= 2) io.value(v.player);
}
template<class IO> void fields(IO &io, const KfNetWorld &v)
{
    io.value(v.header); io.value(v.quest_rewards); io.value(v.members); io.value(v.actors);
    io.value(v.action_animations); io.value(v.effects); io.value(v.objects); io.value(v.events);
    io.value(v.gold_drop_sequence); io.value(v.definition_drop_sequence); io.value(v.placement_drop_sequence);
    io.value(v.dialogue_advance_gate); io.value(v.ambient_script_countdown);
    for (const auto &floor : v.floors) { io.value(floor.script); if (v.header.full) io.value(floor.records); }
    io.value(v.sound_sequence); io.value(v.sounds);
    io.grid(v.collision_flags); io.grid(v.cell_orientation); io.grid(v.floor_height);
    io.grid(v.collision); io.grid(v.cell_attribute); io.value(v.random_state); io.value(v.story);
}
struct FixtureWriter {
    std::vector<u8> bytes;
    template<class T> void value(const T &v) {
        if constexpr (std::is_integral_v<T>) {
            auto bits = static_cast<std::make_unsigned_t<T>>(v);
            for (unsigned i = 0; i < sizeof(T); ++i) bytes.push_back(static_cast<u8>(bits >> (i * 8)));
        } else if constexpr (std::is_array_v<T>) {
            for (const auto &item : v) value(item);
        } else fields(*this, v);
    }
    void grid(std::span<const u8> cells) {
        while (!cells.empty()) {
            std::size_t count = 1;
            while (count < cells.size() && count < UINT16_MAX && cells[count] == cells[0]) ++count;
            value(static_cast<u16>(count)); value(cells[0]); cells = cells.subspan(count);
        }
    }
};
std::vector<u8> unchecked(const KfNetWorld &world)
{ FixtureWriter writer; writer.value(world); return std::move(writer.bytes); }

FILE *corpus = nullptr;
unsigned accepted = 0, rejected = 0;
std::size_t corpus_bytes = 0;
std::uint64_t corpus_hash = 14695981039346656037ull;
void record_corpus(std::span<const u8> bytes)
{
    for (u8 byte : bytes) { corpus_hash ^= byte; corpus_hash *= 1099511628211ull; }
    corpus_bytes += bytes.size();
    if (corpus) require(std::fwrite(bytes.data(), 1, bytes.size(), corpus) == bytes.size(), "write wire corpus");
}
struct Fixture {
    std::unique_ptr<KfNetWorld> world = std::make_unique<KfNetWorld>();
    std::unique_ptr<KfNetWorld> restored = std::make_unique<KfNetWorld>();
    KfNetWorldLimits limits {};
    Fixture() {
        world->header = {7, 0x12345678, 1, 1, 0};
        for (auto &a : world->actors) a.slot_state = 255;
        for (auto &e : world->effects) e.slot_type = 255;
        for (auto &o : world->objects) { o.generation = 1; o.object_id = 255; }
        for (auto &e : world->events) e.state = 255;
        std::fill(std::begin(limits.asset_clips), std::end(limits.asset_clips), -1);
        std::fill(std::begin(limits.object_operations), std::end(limits.object_operations), 255);
    }
    void player() {
        world->members[0].presence = 2;
        auto &p = world->members[0].player;
        p.progress_state_level = 1;
        p.progress_state_current_floor = world->header.floor;
        p.progress_state_highest_floor = world->header.floor;
        p.map_variant = world->header.variant;
        p.vitals_maximum_hp = p.vitals_maximum_mp = 100;
        p.equipped_head_armor_id = p.equipped_body_armor_id = p.equipped_shield_id = 255;
        p.equipped_arm_armor_id = p.equipped_leg_armor_id = 255;
    }
    std::vector<u8> accept(std::source_location where = std::source_location::current()) {
        const auto expected = unchecked(*world);
        std::vector<u8> bytes(KF_NET_TRANSFER_LIMIT, 0xa5);
        std::size_t size = 0;
        require(kf_net_world_encode(world.get(), &limits, bytes.data(), bytes.size(), &size) == KF_CODEC_OK,
            "valid world rejected by encoder", where);
        bytes.resize(size);
        require(bytes == expected, "wire field order or run encoding changed", where);
        require(kf_net_world_decode(bytes.data(), bytes.size(), &limits, restored.get()) == KF_CODEC_OK,
            "valid world rejected by decoder", where);
        require(unchecked(*restored) == bytes, "decoded world lost wire values", where);
        FixtureWriter length; length.value(static_cast<u32>(bytes.size()));
        record_corpus(length.bytes); record_corpus(bytes);
        ++accepted;
        return bytes;
    }
    void reject_bytes(std::span<const u8> bytes,
        std::source_location where = std::source_location::current()) {
        std::memset(restored.get(), 0xa5, sizeof(*restored));
        std::vector<u8> before(sizeof(*restored));
        std::memcpy(before.data(), restored.get(), before.size());
        require(kf_net_world_decode(bytes.data(), bytes.size(), &limits, restored.get()) == KF_CODEC_INVALID,
            "malformed world accepted", where);
        require(std::memcmp(before.data(), restored.get(), before.size()) == 0,
            "rejected world partially modified caller output", where);
        ++rejected;
    }
    void reject(std::source_location where = std::source_location::current()) {
        reject_bytes(unchecked(*world), where);
        std::vector<u8> bytes(KF_NET_TRANSFER_LIMIT, 0xa5);
        std::size_t size = 1234;
        require(kf_net_world_encode(world.get(), &limits, bytes.data(), bytes.size(), &size) == KF_CODEC_INVALID,
            "invalid semantic world accepted by encoder", where);
        require(size == 1234 && std::all_of(bytes.begin(), bytes.end(), [](u8 b) { return b == 0xa5; }),
            "rejected world partially modified encoded output", where);
    }
};

void avatar_bounds()
{
    Fixture f;
    for (u8 full : {0, 1}) {
        f.world->header.full = full;
        f.world->members[0].avatar = 43; f.accept();
        require(f.restored->members[0].avatar == 43, "appended avatar lost");
        f.world->members[0].avatar = KF_AVATAR_SLOTS; f.reject();
    }
}
void wire_and_runs()
{
    Fixture f; auto &w = *f.world;
    w.floor_height[9999] = 42; w.actors[0].random_state = 0x98765432;
    w.effects[0].age = UINT32_MAX; w.random_state = 0xabcdef01; w.gold_drop_sequence = 65535;
    std::fill(std::begin(w.members[0].character_id), std::end(w.members[0].character_id), 0xa5);
    auto bytes = f.accept();
    constexpr u8 header[] = {'K', 'F', 'S', '1', 11, 0, 1, 7, 0, 0, 0, 0x78, 0x56, 0x34, 0x12, 1, 0, 0, 0, 0};
    require(std::equal(std::begin(header), std::end(header), bytes.begin()), "known world header changed");
    require(f.restored->floor_height[9999] == 42 && f.restored->actors[0].random_state == 0x98765432 &&
        f.restored->effects[0].age == UINT32_MAX && f.restored->random_state == 0xabcdef01, "wide values lost");
    auto invalid = bytes; invalid[4] = 9; f.reject_bytes(invalid);
    std::copy(std::begin(w.members[0].character_id), std::end(w.members[0].character_id), w.members[1].character_id);
    f.reject(); std::fill(std::begin(w.members[1].character_id), std::end(w.members[1].character_id), 0);
    for (std::size_t length : {std::size_t(0), std::size_t(19), std::size_t(20), bytes.size() - 1})
        f.reject_bytes(std::span(bytes).first(length));
    invalid = bytes;
    const auto last_run = invalid.size() - 35 - 3;
    invalid[last_run] = invalid[last_run + 1] = 0; f.reject_bytes(invalid);
    invalid[last_run] = 10001 & 255; invalid[last_run + 1] = 10001 >> 8; f.reject_bytes(invalid);
    invalid = bytes; invalid.push_back(0); f.reject_bytes(invalid);
    std::vector<u8> short_output(bytes.size() - 1, 0xa5); std::size_t written = 1234;
    require(kf_net_world_encode(&w, &f.limits, short_output.data(), short_output.size(), &written) == KF_CODEC_OUTPUT_FULL &&
        written == 1234 && std::all_of(short_output.begin(), short_output.end(), [](u8 b) { return b == 0xa5; }),
        "undersized world output was partially written");
    w.header.full = 0; const auto fast = f.accept();
    require(bytes.size() - fast.size() == 5 * 1690, "full/fast persisted floor difference changed");
}
void grid_bounds()
{
    Fixture f; auto &w = *f.world;
    for (u8 attribute : {1, 68, 100}) {
        w.cell_attribute[123] = attribute;
        for (u8 rotation = 1; rotation <= 4; ++rotation) {
            w.cell_orientation[123] = rotation;
            for (u8 collision = 0; collision <= 6; ++collision) { w.collision[123] = collision; f.accept(); }
        }
        for (u8 rotation : {0, 5, 255}) { w.cell_orientation[123] = rotation; f.reject(); }
    }
    for (u8 attribute : {0, 101, 255}) { w.cell_attribute[123] = attribute; f.accept(); }
    for (u8 collision : {7, 128, 255}) { w.collision[123] = collision; f.reject(); }
}
void sound_bounds()
{
    Fixture f; auto &w = *f.world;
    for (u32 latest : {1u, 31u, 32u, 37u, UINT32_MAX}) {
        w.sound_sequence = latest;
        for (auto &s : w.sounds) s = {};
        for (u32 age = 0; age < std::min<u32>(latest, KF_WORLD_SOUNDS); ++age) {
            auto sequence = latest - age;
            w.sounds[sequence % KF_WORLD_SOUNDS] = {sequence, 100000, -1700, 100000, 1, 2, 60, 127, 16000, 28000};
        }
        f.accept(); require(f.restored->sound_sequence == latest, "sound sequence changed");
        auto &s = w.sounds[latest % KF_WORLD_SOUNDS]; const auto valid = s;
        s.sequence = 0; f.reject(); s = valid;
        s.sequence = latest - 1; f.reject(); s = valid;
        s.x = INT32_MAX; f.reject(); s = valid;
        s.y = INT32_MIN; f.reject(); s = valid;
        s.tone = 16; f.reject(); s = valid;
        s.program = 128; f.reject(); s = valid;
        s.note = 128; f.reject(); s = valid;
        s.volume = 128; f.reject(); s = valid;
        s.max_distance = 60001; f.reject(); s = valid;
        s.attenuation_distance = 0; f.reject(); s = valid;
        s.attenuation_distance = 100; f.reject(); s = valid;
    }
}
void header_bounds()
{
    Fixture f; const auto valid = f.accept();
    for (s32 floor = 0; floor <= 6; ++floor) for (u8 variant = 0; variant <= 4; ++variant) {
        auto bytes = valid; bytes[15] = floor; bytes[19] = variant;
        bool available = floor == 5 ? variant >= 1 && variant <= 3 : floor >= 1 && floor <= 4 && variant == 0;
        KfNetWorldHeader out {91, 92, 93, 94, 95};
        require((kf_net_world_info(bytes.data(), bytes.size(), &out) == KF_CODEC_OK) == available, "floor/variant header bound changed");
        if (available) {
            f.world->header.floor = floor; f.world->header.variant = variant; f.accept();
        } else {
            require(out.epoch == 91 && out.tick == 92 && out.floor == 93 && out.full == 94 && out.variant == 95,
                "invalid header partially published");
            f.reject_bytes(bytes);
        }
    }
}
void cast_bounds()
{
    Fixture f; f.player();
    for (u8 full : {0, 1}) for (u8 ticks : {0, 1, 6, 12, 13, 255}) {
        f.world->header.full = full; f.world->members[0].player.cast_pose_ticks = ticks;
        if (ticks <= 12) { f.accept(); require(f.restored->members[0].player.cast_pose_ticks == ticks, "cast pose changed"); }
        else f.reject();
    }
}
void story_bounds()
{
    Fixture f; auto &w = *f.world; w.header.floor = 5; w.header.variant = 1; f.player();
    auto &s = w.story;
    s = {2, 0, 255, 0, 180, 1, 1000, -1700, 1000, 0, 0, 0, 0, 0};
    auto bytes = f.accept();
    for (auto [offset, value] : {std::pair{0, 5}, {1, 4}, {2, 48}, {3, 255}, {4, 255}, {5, 179}, {6, 1}, {7, 0}, {14, 127}, {18, 127}, {22, 127}}) {
        auto invalid = bytes; invalid[bytes.size() - 31 + offset] = value; f.reject_bytes(invalid);
    }
    s.tick = 361; w.objects[180].object_id = 11; f.accept();
    s.effect = 0; f.reject(); s.effect = 255;
    w.objects[180].object_id = 10; f.reject(); w.objects[180].object_id = 11;
    s.generation = 2; f.reject();
    s.kind = 3; s.tick = 0; s.page = 1; s.ready_mask = 1; bytes = f.accept();
    for (auto [offset, value] : {std::pair{2, 0}, {3, 1}, {29, 3}, {30, 16}}) {
        auto invalid = bytes; invalid[bytes.size() - 31 + offset] = value; f.reject_bytes(invalid);
    }
    s.page = 0; f.reject(); s.ready_mask = 0; s.tick = 1; f.accept(); s.tick = 2; f.reject();
    s.kind = 4; s.tick = 0; s.ready_mask = 1; f.reject(); w.floors[4].script[3] = 1; bytes = f.accept();
    for (auto [offset, value] : {std::pair{1, 1}, {2, 0}, {3, 1}, {29, 1}, {30, 0}, {30, 16}}) {
        auto invalid = bytes; invalid[bytes.size() - 31 + offset] = value; f.reject_bytes(invalid);
    }
    w.header.floor = 2; w.header.variant = 0; w.members[0].player.map_variant = 0;
    w.members[0].player.progress_state_current_floor = 2; f.reject();
    w.header.floor = w.members[0].player.progress_state_current_floor = 1; f.accept();
}
void combat_bounds()
{
    Fixture f; auto &w = *f.world; std::fill(std::begin(f.limits.asset_clips), std::end(f.limits.asset_clips), 1);
    auto &a = w.actors[0]; a.slot_state = 1; a.lifecycle = 1; a.death_drop_object_id = 99; f.accept();
    a.death_drop_object_id = 160; f.reject(); a.death_drop_object_id = 255;
    a.action = 16; w.action_animations[0][5] = 255; f.reject(); w.action_animations[0][5] = 0; f.accept();
    a.slot_state = 255; auto &e = w.effects[0]; e.slot_type = 0; e.kind = 5; e.animation_clip = 255;
    e.base_render_id = e.render_id = 21; f.reject();
    e.base_render_id = e.render_id = e.animation_clip = 0; f.reject(); e.animation_clip = 255; f.accept();
    constexpr unsigned billboards[][3] = {{4,17,2}, {5,0,5}, {7,5,1}, {10,9,1}, {11,8,1}, {12,10,1}, {19,14,3}, {32,19,3}};
    for (const auto &b : billboards) {
        e.kind = b[0]; e.base_render_id = b[1];
        for (unsigned frame = 0; frame < b[2]; ++frame) { e.render_id = b[1] + frame; f.accept(); }
        e.base_render_id = 255; f.reject();
    }
}
void coordinate_bounds()
{
    Fixture f; auto &w = *f.world; f.player();
    std::fill(std::begin(f.limits.asset_clips), std::end(f.limits.asset_clips), 0);
    auto &a = w.actors[0]; a.slot_state = 1;
    auto &o = w.objects[0]; o.object_id = 0;
    auto &n = w.events[0]; n.state = 1; n.dialogue_stage = 1; n.character_id = 1;
    auto &e = w.effects[0]; e.slot_type = 0; e.kind = 5; e.animation_clip = 255;
    auto &p = w.members[0].player;
    for (u8 full : {0, 1}) {
        w.header.full = full; f.accept();
        for (s32 bad : {INT32_MIN, -1, 200000, INT32_MAX}) {
            for (s32 *field : {&a.position_vx, &o.position_vz, &n.reference_position_vx}) {
                *field = bad; f.reject(); *field = 0;
            }
        }
        for (s32 bad : {INT32_MIN, INT32_MAX}) {
            for (s32 *field : {&p.camera_position_vy, &p.foot_height, &a.position_vy, &e.position_vz, &n.home_x}) {
                *field = bad; f.reject(); *field = 0;
            }
        }
        a.tile_x = 100; f.reject(); a.tile_x = 0; a.local_z = -1; f.reject(); a.local_z = 0;
        a.slot_state = 0; a.tile_x = a.tile_z = 255; e.position_vz = 200001; f.accept();
        a.slot_state = 1; a.tile_x = a.tile_z = 0; e.position_vz = 0;
        e.kind = 52; e.position_vy = INT32_MIN; f.reject(); e.position_vy = -1; f.accept();
        e.position_vy = 0; e.kind = 5;
    }
}
void index_and_saved_link_bounds()
{
    Fixture f; auto &w = *f.world; auto &m = w.members[0];
    m.presence = 5; f.reject(); m.presence = 0;
    w.quest_rewards = 32; f.reject(); w.quest_rewards = 1;
    m.quest_rewards = 2; f.reject(); m.quest_rewards = 1;
    m.loot_claims[4][189] = 16; f.reject(); m.loot_claims[4][189] = 15;
    auto &a = w.actors[0]; a.slot_state = 0; a.definition_id = 12; f.reject(); a.definition_id = 0;
    f.limits.asset_clips[0] = 2; a.animation_clip = 2; f.reject(); a.animation_clip = 1; f.accept();
    w.action_animations[0][5] = 2; f.reject(); w.action_animations[0][5] = 0;
    auto &o = w.objects[0]; o.object_id = 0; o.action = 81; o.link_words[0] = 48 << 8; f.reject();
    o.link_words[0] = 47 << 8; f.accept();
    auto &e = w.effects[0]; e.slot_type = 0; e.animation_clip = 255; e.render_id = 22; f.reject(); e.slot_type = 255;
    auto &floor = w.floors[0].records; floor[0] = 1;
    for (unsigned i = 0; i < 8; ++i) floor[1 + i * 7] = 255;
    std::fill(floor + 58, floor + 248, 255); floor[58] = 0;
    floor[248] = 1; floor[249] = 0; floor[251] = 48; f.limits.object_operations[0] = 81; f.reject();
    floor[251] = 47; f.accept();
}
void preflight_before_floor_load()
{
    Fixture f;
    f.world->header.floor = 5; f.world->header.variant = 1;
    const auto valid = unchecked(*f.world);
    const auto preflight = [](std::span<const u8> bytes, bool accepted) {
        KfNetWorldHeader header {91, 92, 93, 94, 95};
        const auto status = kf_net_world_preflight(bytes.data(), bytes.size(), &header);
        require(status == (accepted ? KF_CODEC_OK : KF_CODEC_INVALID), "unexpected full-snapshot preflight result");
        if (accepted) require(header.floor == 5 && header.variant == 1 && header.epoch == 7 && header.full == 1,
            "preflight published wrong floor metadata");
        else require(header.epoch == 91 && header.tick == 92 && header.floor == 93 && header.full == 94 && header.variant == 95,
            "rejected preflight published a floor before full validation");
    };
    preflight(valid, true);
    // A legal destination in a header is not permission to change live assets.
    KfNetWorldHeader inspected {};
    require(kf_net_world_info(valid.data(), 20, &inspected) == KF_CODEC_OK && inspected.floor == 5,
        "header fixture is not independently valid");
    for (std::size_t length : {std::size_t(0), std::size_t(19), std::size_t(20), valid.size() - 1})
        preflight(std::span(valid).first(length), false);
    auto invalid = valid; invalid.push_back(0); preflight(invalid, false);
    invalid = valid; invalid[invalid.size() - 38] = invalid[invalid.size() - 37] = 0; preflight(invalid, false);
    f.world->members[0].presence = 5; preflight(unchecked(*f.world), false); f.world->members[0].presence = 0;
    f.world->cell_attribute[123] = 1; preflight(unchecked(*f.world), false); f.world->cell_attribute[123] = 0;
    auto &actor = f.world->actors[0]; actor.slot_state = 0; actor.definition_id = 12;
    preflight(unchecked(*f.world), false); actor.definition_id = 0;
    // Clip existence needs the destination's resources. Preflight permits this
    // bounded reference, while the resource-aware decoder must still reject it.
    actor.animation_clip = 7;
    const auto unavailable = unchecked(*f.world); preflight(unavailable, true);
    f.limits.asset_clips[0] = 1; f.reject_bytes(unavailable);
    actor.slot_state = 255;
    auto &effect = f.world->effects[0]; effect.slot_type = 0; effect.render_id = 18; effect.animation_clip = 0;
    preflight(unchecked(*f.world), false); effect.slot_type = 255;
    auto &event = f.world->events[0]; event.state = 1; event.character_id = 1; event.dialogue_stage = 1; event.model_index = 38;
    preflight(unchecked(*f.world), false); event.state = 255;
    f.world->story.kind = 5; preflight(unchecked(*f.world), false);
}
}

int main(int argc, char **argv)
{
    require(argc == 1 || argc == 2, "usage: coop-world-codec-test [wire-corpus-output]");
    if (argc == 2) { corpus = std::fopen(argv[1], "wb"); require(corpus, "cannot open wire corpus output"); }
    avatar_bounds(); wire_and_runs(); grid_bounds(); sound_bounds(); header_bounds();
    cast_bounds(); story_bounds(); combat_bounds(); coordinate_bounds(); index_and_saved_link_bounds();
    preflight_before_floor_load();
    // Protocol 15/schema 11 golden corpus, verified against the prior encoder.
    // Keep this independent of the field-order helper used for malformed input.
    require(corpus_bytes == 5127900 && corpus_hash == 0xf12e8a3672b68a3full,
        "world wire corpus changed");
    if (corpus) require(std::fclose(corpus) == 0, "cannot finish wire corpus output");
    std::printf("World codec: %u valid and %u malformed fixtures passed\n", accepted, rejected);
}
