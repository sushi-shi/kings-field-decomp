#include <kf/platform/prelude.h>
#include <kf/game/game.h>
#include <kf/lib/map_data.h>

#include <algorithm>
#include <cassert>
#include <cstdio>

void actor_spawn_action_effect(KfActorEffectCode effect_code, KfActorEffectSlot effect_slot);

static void reset()
{
    effect_state = {};
    effect_pool_reset();
    actor_state = {};
    actor_pool_clear();
    player_state = {};
    player_state.camera_position = {10000, -1000, 10000};
    std::ranges::fill(map_collision_grid.linear, KF_MAP_CELL_FLOOR);
    std::ranges::fill(map_cell_attribute_grid.linear, KF_MAP_ATTRIBUTE_NONE);
    std::ranges::fill(map_floor_height_grid.linear, 0);
    std::ranges::fill(map_collision_flag_grid.linear, 0);
}

static void scatter_words()
{
    // Track the BIOS-style random stream independently, retaining its unsigned
    // word arithmetic before comparing the signed velocity used for movement.
    u32 random = 1;
    for (const u16 bits : {0, 1, 0x7fff, 0x8000, 0xffff}) {
        SVECTOR velocity{static_cast<s16>(bits), static_cast<s16>(bits), static_cast<s16>(bits)};
        SVECTOR expected{};
        for (s16 *component : {&expected.vx, &expected.vy, &expected.vz}) {
            random = random * 1103515245u + 12345u;
            const u16 word = bits - 64 + ((random >> 24) & 127);
            *component = static_cast<s16>(word);
        }
        effect_scatter_triple(&velocity);
        assert(velocity.vx == expected.vx && velocity.vy == expected.vy && velocity.vz == expected.vz);
    }
}

static void falling_words()
{
    for (const u16 bits : {0, 0x7fff, 0x8000, 0xffff}) {
        reset();
        auto *effect = effect_spawn_emerging_projectile(0, KF_EFFECT_COLLISION_TARGET_ACTORS,
            {10000, -100000, 10000}, {0, static_cast<s16>(bits), 0});
        const s32 start_y = effect->position.vy;
        effect->phase = KF_EFFECT_PROJECTILE_FALL;
        effect_pool_set_current(effect);
        effect_update_dispatch();
        const s16 velocity = static_cast<s16>(static_cast<u16>(bits + 20));
        assert(effect->direction.vy == velocity && effect->position.vy == start_y + velocity);
    }
}

static void swinging_words()
{
    for (const u16 bits : {0, 0x7fff, 0x8000, 0xffff}) {
        reset();
        auto *effect = effect_spawn_swinging_hazard_short(0, KF_EFFECT_COLLISION_TARGET_ACTORS,
            {10000, -1000, 10000}, {}, {});
        effect->rotation.vx = 100;
        effect->direction.vx = static_cast<s16>(bits);
        effect->sound_played = KF_AUDIO_PLAYED;
        effect_pool_set_current(effect);
        SVECTOR probe{};
        effect_update_swinging_hazard(&probe, KF_EFFECT_SHORT_SWING_PHASE_LIMIT);
        const u16 angular_velocity = bits - 10;
        assert(effect->direction.vx == static_cast<s16>(angular_velocity));
        assert(effect->rotation.vx == static_cast<s16>(100 + angular_velocity));
    }
}

static void orbit_and_branch()
{
    reset();
    auto *orbit = effect_spawn_orbiting_projectile(0, KF_EFFECT_COLLISION_TARGET_ACTORS,
        {10000, -1000, 10000}, {});
    effect_pool_set_current(orbit);
    effect_update_orbiting_projectile(0, KF_EFFECT_ORBIT_PHASE_LIMIT);
    assert(orbit->position.vx == 9984 && orbit->position.vz == 9984 && orbit->position.vy == -1000);

    auto *parent = effect_spawn_fire_wall(0, KF_EFFECT_COLLISION_TARGET_ACTORS,
        {10000, -1000, 10000}, {0, KF_ANGLE_THREE_QUARTER_TURN, 0}, KF_EFFECT_GROUND_BRANCH_LEAF);
    effect_spawn_ground_branch(0, parent, KF_ANGLE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_QUARTER_TURN);
    assert(effect_state.records[2].position.vx == 10000 && effect_state.records[2].position.vz == 11500);
}

static void actor_rotations()
{
    for (const auto kind : {KF_MAGIC_LIGHT_NEEDLE, KF_EFFECT_KIND_PHYSICAL_PROJECTILE,
                           KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE}) {
        reset();
        auto &actor = actor_state.actors[0];
        actor.rotation = {128, 256, 64};
        actor.position = {10000, -1000, 10000};
        actor_state.current = &actor;
        actor_state.current_definition = &actor_state.definitions.entries[0];
        player_state.camera_position = actor_state.player_position = {100000, -1000, 100000};
        actor_spawn_action_effect(kf_enum_decode<KfActorEffectCode>(kf_enum_encode<u8>(kind)),
            KF_ACTOR_EFFECT_SLOT_FIRST);
        const auto &effect = effect_state.records[0];
        assert(effect.type != KF_EFFECT_SLOT_FREE);
        assert(effect.rotation.vy == KF_ANGLE_HALF_TURN - 256);
        assert(effect.rotation.vx == (kind == KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE ? -128 : 0));
        assert(effect.rotation.vz == (kind == KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE ? 64 : 0));
        assert(effect.rotation.pad == 0);
    }
}

static void floor_lifecycle(u16 first, u16 count, s32 sweep, s32 hold)
{
    reset();
    auto *effect = effect_pool_spawn_floor_deformation(first, count, 150, 800, sweep, hold);
    effect_pool_set_current(effect);
    u32 frames = 0;
    while (effect->type != KF_EFFECT_SLOT_FREE && frames < 500) {
        effect_update_dispatch();
        ++frames;
        if (frames == static_cast<u32>(sweep)) {
            assert(effect->floor_deformation.progress == sweep * 150);
            if (first == 0)
                assert(map_floor_height_grid.cells[80][65] == 100 && map_floor_height_grid.cells[54][75] == 0);
            else
                assert(map_floor_height_grid.cells[82][32] == 100 && map_floor_height_grid.cells[93][32] == 100);
        }
    }
    assert(frames == static_cast<u32>(2 * sweep + hold + 3));
    assert(effect->type == KF_EFFECT_SLOT_FREE);
    assert(std::ranges::all_of(map_floor_height_grid.linear, [](u8 height) { return height == 0; }));
}

int main()
{
    static_assert(sizeof(KfEulerAngles) == 6 && sizeof(SVECTOR) == 8);
    scatter_words();
    falling_words();
    swinging_words();
    orbit_and_branch();
    actor_rotations();
    floor_lifecycle(0, 4, 43, 70);
    floor_lifecycle(4, 1, 88, 270);
    std::puts("Effect storage regressions passed.");
}
