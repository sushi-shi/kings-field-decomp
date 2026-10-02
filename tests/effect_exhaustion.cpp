#include "../src/game/effect_pool.cpp"
#include "../src/game/player_warp.cpp"
#include "../src/game/map_object_pool.cpp"

#include <cassert>
#include <string_view>

KfPlayerState player_state{};
KfMapCollisionGrid map_collision_grid{};
KfMapGrid map_floor_height_grid{};
std::array<SoundRef, KF_GAMEPLAY_SOUND_COUNT> gameplay_sound_refs{};
static kf::InputContext input_context = kf::InputContext::Gameplay;
static unsigned frames, sounds;
struct MissingHazard {};

namespace kf {
InputContext host_set_input_context(InputContext next)
{
    const auto previous = input_context;
    input_context = next;
    return previous;
}
[[noreturn]] void host_fail(const char *message)
{
    assert(std::string_view(message) == "No effect slot available for a map hazard.");
    throw MissingHazard{};
}
}
void display_flip_buffer_index() {}
void render_frame(const VECTOR *, const SVECTOR *)
{
    assert(input_context == kf::InputContext::Scripted);
    ++frames;
    for (const auto &effect : effect_state.records) {
        if (effect.type != KF_EFFECT_SLOT_FREE && effect.kind == KF_EFFECT_KIND_WARP_SHIMMER) {
            assert(effect.direction.vx == 0 && effect.direction.vy == 0);
            assert(effect.direction.vz == 0);
        }
    }
}
KfAudioPlayback audio_playback()
{
    static KfAudioState state{};
    static s32 slot;
    return {state, slot, true};
}
void sound_ref_play(KfAudioPlayback, const SoundRef *, s16) { ++sounds; }
void collision_adjust_cell_occupancy(u16, u16, s32) {}
void map_object_start_action_if_idle(KfMapObject *object, KfMapObjectOperation action)
{
    object->action = action;
}

static void occupy_pool()
{
    effect_state = {};
    for (auto &effect : effect_state.records) {
        effect.type = KF_EFFECT_CLASS_20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER;
        effect.kind = KF_MAGIC_FIRE_BALL;
        effect.position = {123, 456, 789};
    }
}

int main()
{
    for (auto mode : {KF_WARP_SHIMMER_GROW_REMOVE, KF_WARP_SHIMMER_GROW_KEEP, KF_WARP_SHIMMER_SHRINK_REMOVE}) {
        for (unsigned available = 0; available <= KF_CYLINDER_TRANSITION_COUNT; ++available) {
            occupy_pool();
            const auto occupied = effect_state.records.size() - available;
            for (std::size_t i = occupied; i < effect_state.records.size(); ++i)
                effect_state.records[i].type = KF_EFFECT_SLOT_FREE;
            frames = sounds = 0;
            VECTOR position{20000, -1000, 20000};
            player_warp_shimmer(mode, &position);
            assert(input_context == kf::InputContext::Gameplay);
            assert(frames == KF_CYLINDER_TRANSITION_FRAMES + 2 && sounds == 1);
            for (std::size_t i = 0; i < effect_state.records.size(); ++i) {
                const auto &effect = effect_state.records[i];
                if (i < occupied) {
                    assert(effect.kind == KF_MAGIC_FIRE_BALL && effect.type != KF_EFFECT_SLOT_FREE);
                    assert(effect.position.vx == 123 && effect.position.vy == 456 && effect.position.vz == 789);
                } else if (mode == KF_WARP_SHIMMER_GROW_KEEP) {
                    assert(effect.kind == KF_EFFECT_KIND_WARP_SHIMMER && effect.type != KF_EFFECT_SLOT_FREE);
                } else {
                    assert(effect.type == KF_EFFECT_SLOT_FREE);
                }
            }
        }
    }

    for (auto id : {KF_MAP_OBJECT_ORBITING_PROJECTILE, KF_MAP_OBJECT_SHORT_SWING,
                   KF_MAP_OBJECT_LONG_SWING, KF_MAP_OBJECT_EFFECT_SWITCH}) {
        std::array<u8, 21> bytes{};
        bytes[0] = kf_enum_encode<u8>(id);
        bytes[2] = bytes[3] = 10;
        bytes[20] = 255;
        occupy_pool();
        try {
            map_object_pool_load({bytes.data(), bytes.size()});
            assert(false);
        } catch (const MissingHazard &) {}
        for (const auto &effect : effect_state.records)
            assert(effect.kind == KF_MAGIC_FIRE_BALL);

        effect_state.records.back().type = KF_EFFECT_SLOT_FREE;
        map_object_pool_load({bytes.data(), bytes.size()});
        const auto index = map_object_state.objects[0].link.fields.action_parameter.effect_index;
        assert(index == effect_state.records.size() - 1);
        assert(effect_state.records[index].type != KF_EFFECT_SLOT_FREE);
        if (id == KF_MAP_OBJECT_EFFECT_SWITCH) {
            const auto &direction = effect_state.records[index].direction;
            assert(direction.vx == 0 && direction.vy == 0 && direction.vz == 0);
        }
    }
}
