#include <kf/platform/frame_task.hpp>
#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/platform/prelude.h>
#include <kf/game/audio.h>
#include <kf/game/collision.h>
#include <kf/game/game.h>
#include <kf/game/avatar.h>
#include <kf/game/graphics.h>
#include <kf/game/player.h>
#include <kf/game/session.h>
#include <kf/lib/null.h>
#include <kf/lib/random.h>
#include <kf/platform/input.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static constexpr u16 PLAYER_PITFALL_CUTTING_DAMAGE = 5;
static constexpr u16 PLAYER_PITFALL_STRIKING_DAMAGE = 3;
static constexpr u16 PLAYER_PITFALL_PIERCING_DAMAGE = 5;

enum {
    POISON_DAMAGE_INTERVAL_UPDATES = 20,
    POISON_FLASH_UPDATES = 2
};

enum {
    PLAYER_WEAPON_MAGIC_PHASE_FIRST = 2400,
    PLAYER_WEAPON_MAGIC_PHASE_LAST = 3900,
    PLAYER_COLICHEMARDE_MAGIC_PHASE_FIRST = 900,
    PLAYER_COLICHEMARDE_MAGIC_PHASE_LAST = 2400
};

static constexpr int DARKNESS_FADE_BITS = 5;
static constexpr int DARKNESS_FADE_STEPS = 1 << DARKNESS_FADE_BITS;
static constexpr int PLAYER_NORMAL_MOVEMENT_LIMIT = 180;
static constexpr int PLAYER_SLOWED_MOVEMENT_LIMIT = PLAYER_NORMAL_MOVEMENT_LIMIT / 5;
static constexpr int PLAYER_NORMAL_TURN_LIMIT = 28;
static constexpr int PLAYER_SLOWED_TURN_LIMIT = 5;
static constexpr int PLAYER_DARKNESS_FOG_NEAR = 5000;

enum {
    PLAYER_FORWARD_ACCEL_SHIFT = 2,
    PLAYER_FORWARD_DECEL_SHIFT = 3,
    PLAYER_STRAFE_ACCEL_DECEL_SHIFT = 2,
    PLAYER_YAW_ACCEL_DECEL_SHIFT = 2,
    PLAYER_PITCH_ACCEL = 3,
    PLAYER_PITCH_DECEL = 2,
    PLAYER_PITCH_STEP_LIMIT = 10
};

enum {
    PLAYER_WEAPON_MAGIC_READY = 1,
    PLAYER_TRIPLE_FANG_MAGIC_DELAY = 3,
    PLAYER_FLAME_SWORD_MAGIC_DELAY = 2,
    PLAYER_MOONLIGHT_SWORD_MAGIC_DELAY = 3,
    PLAYER_TRIPLE_FANG_MAGIC_MIN_POWER = 80,
    PLAYER_MOONLIGHT_SWORD_MAGIC_MIN_POWER = 80,
    PLAYER_COLICHEMARDE_MAGIC_MIN_POWER = 60,
    PLAYER_COLICHEMARDE_SHOTS_PER_PHASE = 2,
    PLAYER_WEAPON_MAGIC_SPAWN_X = 200,
    PLAYER_WEAPON_MAGIC_SPAWN_Y = 200,
    PLAYER_WEAPON_MAGIC_SPAWN_Z = 400,
    PLAYER_WEAPON_MAGIC_BURST_CONE = 0x155,
    PLAYER_WEAPON_MAGIC_JITTER_BIAS = 4,
    PLAYER_WEAPON_MAGIC_RANDOM_SHIFT = 9,
    PLAYER_WEAPON_MAGIC_SPEED = 900,
    PLAYER_TRIPLE_FANG_Y_OFFSET = 300,
    PLAYER_TRIPLE_FANG_PITCH_OFFSET = 64
};

static MATRIX player_darkness_color_matrix = {
    .m = {{
        {666, 233, 1333},
        {666, 233, 1333},
        {666, 233, 1333},
    }},
    .t = {},
};

static std::array<SVECTOR, kf_enum_encode<u8>(KF_PLAYER_DAMAGE_FRAME_END)> player_damage_view_rotation_offsets = {
    SVECTOR{0, 0, 0, 0},
    SVECTOR{-32, 0, -32, 0},
    SVECTOR{-64, 0, -64, 0},
    SVECTOR{-48, 0, -32, 0},
    SVECTOR{-32, 0, 0, 0},
    SVECTOR{-16, 0, 32, 0},
    SVECTOR{0, 0, 64, 0},
    SVECTOR{-16, 0, 32, 0}
};




static void player_handle_magic_input(WorldState &world, PlayerContext &player, u32 input)
{
    s32 cost;
    KfEffectKind magic_id;

    if (player.state.equipped_body_armor_id != KF_ITEM_SKULL_ARMOR) {
        if (kf::button_pressed(input, player.previous_input, kf::Button::Magic)) {
            if (player.state.weapon_attack_fully_charged == KF_WEAPON_ATTACK_FULL_CHARGE) {
                player.state.weapon_attack_fully_charged = KF_WEAPON_ATTACK_NORMAL_CHARGE;
                switch (player.state.equipped_weapon_id) {
                default:
                    // Other weapons do not arm a weapon-magic attack.
                    break;
                case KF_ITEM_FLAME_SWORD:
                    if (player.state.weapon_attack_phase >= PLAYER_WEAPON_MAGIC_PHASE_FIRST
                        && player.state.weapon_attack_phase <= PLAYER_WEAPON_MAGIC_PHASE_LAST) {
                        player.state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                        player.state.weapon_magic_shots_remaining =
                            (PLAYER_WEAPON_MAGIC_PHASE_LAST - player.state.weapon_attack_phase)
                            / KF_WEAPON_ATTACK_PHASE_STEP + 1;
                        return;
                    }
                    break;
                case KF_ITEM_TRIPLE_FANG:
                case KF_ITEM_MOONLIGHT_SWORD:
                    if (player.state.weapon_attack_phase >= PLAYER_WEAPON_MAGIC_PHASE_FIRST
                        && player.state.weapon_attack_phase <= PLAYER_WEAPON_MAGIC_PHASE_LAST) {
                        player.state.weapon_magic_shots_remaining = 1;
                        player.state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                        return;
                    }
                    break;
                case KF_ITEM_COLICHEMARDE:
                    if (player.state.weapon_attack_phase >= PLAYER_COLICHEMARDE_MAGIC_PHASE_FIRST
                        && player.state.weapon_attack_phase <= PLAYER_COLICHEMARDE_MAGIC_PHASE_LAST) {
                        player.state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                        player.state.weapon_magic_shots_remaining =
                            ((PLAYER_WEAPON_MAGIC_PHASE_LAST - player.state.weapon_attack_phase)
                             / KF_WEAPON_ATTACK_PHASE_STEP + 1)
                            * PLAYER_COLICHEMARDE_SHOTS_PER_PHASE;
                        return;
                    }
                    break;
                }
            }
            if (player.state.selected_magic_id != KF_MAGIC_NONE
                && player.state.magic_charge == KF_PLAYER_CHARGE_FULL) {
                player.state.weapon_magic_delay = 0;
                player.state.weapon_magic_shots_remaining = 0;
                if (player.state.equipped_accessory_id == KF_ITEM_WIND_BLADE_BRACELET && player.state.selected_magic_id == KF_MAGIC_WIND_CUTTER) {
                    cost = player.state.selected_magic_record->mp_cost >> 1;
                } else {
                    cost = player.state.selected_magic_record->mp_cost;
                }
                if (player.state.vitals.current_mp >= cost) {
                    player.state.vitals.current_mp -= cost;
                    magic_cast(world, player);
                    player.cast_pose_ticks = avatar_cast_ticks;
                    player.state.magic_charge = 0;
                }
            }
            player.state.weapon_attack_fully_charged = KF_WEAPON_ATTACK_NORMAL_CHARGE;
        } else {
            magic_id = player.state.selected_magic_id;
            if (magic_id != KF_MAGIC_NONE) {
                player.state.magic_charge +=
                    fixed6_ratio_step(player.state.magic, player.state.selected_magic_record->charge_rate)
                    * KF_PLAYER_CHARGE_GAIN_MULTIPLIER;
                player.state.magic_charge =
                    std::clamp<s32>(player.state.magic_charge, 0, KF_PLAYER_CHARGE_FULL);
            }
        }
    }
}

static void player_cancel_weapon_magic(PlayerContext &player)
{
    player.state.weapon_magic_shots_remaining = 0;
    player.state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY + 1;
}

static void player_update_weapon_magic(WorldState &world, PlayerContext &player)
{
    KfEffectKind effect;
    KfEffectHomingMode homing_target;
    KfMagicRecord *record;
    KfActor *target;
    const VECTOR *origin;
    SVECTOR direction;
    KfRotation effect_rotation;
    SVECTOR *launch_direction;
    SVECTOR spawn_offset;
    VECTOR position;
    MATRIX matrix;
    s32 distance;

    if (player.state.weapon_magic_delay == PLAYER_WEAPON_MAGIC_READY) {
        if (player.state.weapon_magic_shots_remaining != 0) {
            switch (player.state.equipped_weapon_id) {
            case KF_ITEM_TRIPLE_FANG:
                if (player.state.physical_power < PLAYER_TRIPLE_FANG_MAGIC_MIN_POWER
                    || player.state.magic < PLAYER_TRIPLE_FANG_MAGIC_MIN_POWER) {
                    player_cancel_weapon_magic(player);
                    return;
                }
                effect = KF_EFFECT_KIND_HOMING_PROJECTILE;
                record = &world.effects.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_HOMING_PROJECTILE)];
                player.state.weapon_magic_delay = PLAYER_TRIPLE_FANG_MAGIC_DELAY;
                break;
            case KF_ITEM_FLAME_SWORD:
                if (player.learned_magic[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)] == KF_MAGIC_UNLEARNED) {
                    player_cancel_weapon_magic(player);
                    return;
                }
                effect = KF_MAGIC_FIRE_BALL;
                record = &world.effects.magic.entries[kf_enum_encode<u8>(KF_MAGIC_FIRE_BALL)];
                player.state.weapon_magic_delay = PLAYER_FLAME_SWORD_MAGIC_DELAY;
                break;
            case KF_ITEM_MOONLIGHT_SWORD:
                if (player.state.physical_power < PLAYER_MOONLIGHT_SWORD_MAGIC_MIN_POWER
                    || player.state.magic < PLAYER_MOONLIGHT_SWORD_MAGIC_MIN_POWER) {
                    player_cancel_weapon_magic(player);
                    return;
                }
                effect = KF_EFFECT_KIND_MOONLIGHT_PROJECTILE;
                record = &world.effects.magic.entries[kf_enum_encode<u8>(KF_EFFECT_KIND_RADIAL_BLAST)];
                player.state.weapon_magic_delay = PLAYER_MOONLIGHT_SWORD_MAGIC_DELAY;
                break;
            case KF_ITEM_COLICHEMARDE:
                if (player.state.physical_power < PLAYER_COLICHEMARDE_MAGIC_MIN_POWER
                    || player.state.magic < PLAYER_COLICHEMARDE_MAGIC_MIN_POWER) {
                    player_cancel_weapon_magic(player);
                    return;
                }
                effect = KF_MAGIC_LIGHT_NEEDLE;
                record = &world.effects.magic.entries[kf_enum_encode<u8>(KF_MAGIC_LIGHT_NEEDLE)];
                player.state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                break;
            default:
                player_cancel_weapon_magic(player);
                return;
            }
            if (player.state.vitals.current_mp >= record->mp_cost) {
                if (player.state.weapon_magic_shots_remaining == 1) {
                    player.state.vitals.current_mp -= record->mp_cost;
                }
                spawn_offset = {
                    PLAYER_WEAPON_MAGIC_SPAWN_X,
                    PLAYER_WEAPON_MAGIC_SPAWN_Y,
                    PLAYER_WEAPON_MAGIC_SPAWN_Z};
                effect_rotation.vector = VECTOR{
                    -player.state.camera_rotation.vx,
                    player.state.camera_rotation.vy,
                    -player.state.camera_rotation.vz}.narrowed();
                matrix_set_rotation_yxz(&effect_rotation.angles, &matrix);
                position = kf::matrix_apply_rotation(matrix, spawn_offset);
                position += player.state.camera_position;
                effect_rotation.vector = player.state.camera_rotation;
                origin = &player.state.camera_position;
                if ((effect == KF_MAGIC_FIRE_BALL || effect == KF_MAGIC_LIGHT_NEEDLE)
                    && player.state.weapon_magic_shots_remaining != 1) {
                    world.actors.player_target = actor_pool_find_target_in_cone(world,
                        origin, player.state.camera_rotation.vy, KF_EFFECT_ACTOR_TARGET_MAX_DISTANCE,
                        PLAYER_WEAPON_MAGIC_BURST_CONE, &distance);
                    effect_rotation.angles.x -= PLAYER_WEAPON_MAGIC_JITTER_BIAS
                        - (player_random_next(player) >> PLAYER_WEAPON_MAGIC_RANDOM_SHIFT);
                    effect_rotation.angles.y -= PLAYER_WEAPON_MAGIC_JITTER_BIAS
                        - (player_random_next(player) >> PLAYER_WEAPON_MAGIC_RANDOM_SHIFT);
                    homing_target = kf_enum_decode<KfEffectHomingMode>(player.state.weapon_magic_shots_remaining & 1);
                } else {
                    target = actor_pool_find_target_in_cone(world,
                        &player.state.camera_position,
                        player.state.camera_rotation.vy, KF_EFFECT_ACTOR_TARGET_MAX_DISTANCE,
                        KF_EFFECT_ACTOR_TARGET_WIDE_CONE, &distance);
                    world.actors.player_target = target;
                    if (target == NULL) {
                        homing_target = KF_EFFECT_HOMING_WANDER;
                    } else {
                        homing_target = kf_enum_decode<KfEffectHomingMode>(target - world.actors.actors.data());
                    }
                }
                launch_direction = &direction;
                pitch_yaw_to_forward_vector(&effect_rotation.angles, launch_direction);
                vector3s_scale_shift12(PLAYER_WEAPON_MAGIC_SPEED, launch_direction);
                effect_pool_construct(world, player,
                    KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS, effect,
                    &position, launch_direction, KfEffectHomingArguments{&player.state.camera_rotation, homing_target, KF_EFFECT_SOUND_PLAY});
                if (effect == KF_EFFECT_KIND_HOMING_PROJECTILE) {
                    position.vy += PLAYER_TRIPLE_FANG_Y_OFFSET;
                    effect_rotation.vector = VECTOR{
                        player.state.camera_rotation.vx + PLAYER_TRIPLE_FANG_PITCH_OFFSET,
                        player.state.camera_rotation.vy,
                        player.state.camera_rotation.vz}.narrowed();
                    effect_pool_construct(world, player,
                        KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                        KF_EFFECT_KIND_HOMING_PROJECTILE, &position, launch_direction, KfEffectHomingArguments{&effect_rotation.vector, homing_target, KF_EFFECT_SOUND_SILENT});
                    effect_rotation.angles.x -= 2 * PLAYER_TRIPLE_FANG_PITCH_OFFSET;
                    position.vy -= 2 * PLAYER_TRIPLE_FANG_Y_OFFSET;
                    effect_pool_construct(world, player,
                        KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                        KF_EFFECT_KIND_HOMING_PROJECTILE, &position, launch_direction, KfEffectHomingArguments{&effect_rotation.vector, homing_target, KF_EFFECT_SOUND_SILENT});
                }
            }
            player.state.weapon_magic_shots_remaining--;
        }
    } else if (player.state.weapon_magic_delay != 0) {
        player.state.weapon_magic_delay--;
    }
}

static void player_apply_armor_periodic_effects(PlayerContext &player, const KfArmorRecord &armor)
{
    if (armor.hp_regen_interval != 0
        && player.state.equipment_effect_ticks % armor.hp_regen_interval == 0) {
        player_adjust_hp(player, 1);
    }
    if (armor.hp_drain_interval != 0
        && player.state.equipment_effect_ticks % armor.hp_drain_interval == 0) {
        player_adjust_hp(player, -1);
    }
}

static void player_update_movement_input(WorldState &world, PlayerContext &player, u32 input)
{
    auto &motion = player.state.motion_state;
    s16 forward;
    s16 strafe;
    s32 strafe_sq;
    s32 forward_sq;
    s16 magnitude;

    if ((player.state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
        player.movement_velocity_limit = PLAYER_SLOWED_MOVEMENT_LIMIT;
        player.turn_step_limit = PLAYER_SLOWED_TURN_LIMIT;
    } else {
        player.movement_velocity_limit = PLAYER_NORMAL_MOVEMENT_LIMIT;
        player.turn_step_limit = PLAYER_NORMAL_TURN_LIMIT;
    }
    if (input & kf::Button::Up) {
        forward = motion.forward_velocity
            + (player.movement_velocity_limit >> PLAYER_FORWARD_ACCEL_SHIFT);
        motion.forward_velocity = std::min<s32>(forward, player.movement_velocity_limit);
    } else if (input & kf::Button::Down) {
        forward = motion.forward_velocity
            - (player.movement_velocity_limit >> PLAYER_FORWARD_ACCEL_SHIFT);
        motion.forward_velocity = std::max<s32>(forward, -player.movement_velocity_limit);
    } else if (motion.forward_velocity > 0) {
        motion.forward_velocity -=
            player.movement_velocity_limit >> PLAYER_FORWARD_DECEL_SHIFT;
        motion.forward_velocity = std::max<s32>(motion.forward_velocity, 0);
    } else if (motion.forward_velocity < 0) {
        motion.forward_velocity +=
            player.movement_velocity_limit >> PLAYER_FORWARD_DECEL_SHIFT;
        motion.forward_velocity = std::min<s32>(motion.forward_velocity, 0);
    }
    if (input & kf::Button::StrafeRight) {
        strafe = motion.strafe_velocity
            + (player.movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT);
        motion.strafe_velocity = std::min<s32>(strafe, player.movement_velocity_limit);
    } else if (input & kf::Button::StrafeLeft) {
        strafe = motion.strafe_velocity
            - (player.movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT);
        motion.strafe_velocity = std::max<s32>(strafe, -player.movement_velocity_limit);
    } else if (motion.strafe_velocity > 0) {
        motion.strafe_velocity -=
            player.movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT;
        motion.strafe_velocity = std::max<s32>(motion.strafe_velocity, 0);
    } else if (motion.strafe_velocity < 0) {
        motion.strafe_velocity +=
            player.movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT;
        motion.strafe_velocity = std::min<s32>(motion.strafe_velocity, 0);
    }
    strafe_sq = motion.strafe_velocity;
    strafe_sq *= strafe_sq;
    forward_sq = motion.forward_velocity;
    forward_sq *= forward_sq;
    magnitude = kf::length_square_root(strafe_sq + forward_sq);
    if (magnitude == 0) {
        forward = 0;
        strafe = 0;
    } else {
        strafe = strafe_sq / magnitude;
        if (motion.strafe_velocity < 0) {
            strafe = -(strafe_sq / magnitude);
        }
        forward = forward_sq / magnitude;
        if (motion.forward_velocity < 0) {
            forward = -(forward_sq / magnitude);
        }
    }
    motion.movement_speed = kf::length_square_root(strafe * strafe + forward * forward);
    if (forward > 0) {
        player_move_horizontal(world, player, player.state.camera_rotation.vy, forward);
    } else if (forward < 0) {
        player_move_horizontal(world, player,
            (player.state.camera_rotation.vy + KF_ANGLE_HALF_TURN) & KF_ANGLE_WRAP_MASK, -forward);
    }
    if (strafe > 0) {
        player_move_horizontal(world, player,
            (player.state.camera_rotation.vy - KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK, strafe);
    } else if (strafe < 0) {
        player_move_horizontal(world, player,
            (player.state.camera_rotation.vy + KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK, -strafe);
    }
    player_update_view_bob(player);
}

static void player_update_view_input(PlayerContext &player, u32 input)
{
    auto &motion = player.state.motion_state;

    if (input & kf::Button::Left) {
        motion.yaw_step += player.turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
        motion.yaw_step = std::min<s32>(motion.yaw_step, player.turn_step_limit);
    } else if (input & kf::Button::Right) {
        motion.yaw_step -= player.turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
        motion.yaw_step = std::max<s32>(motion.yaw_step, -player.turn_step_limit);
    } else if (motion.yaw_step > 0) {
        motion.yaw_step -= player.turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
        motion.yaw_step = std::max<s32>(motion.yaw_step, 0);
    } else if (motion.yaw_step < 0) {
        motion.yaw_step += player.turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
        motion.yaw_step = std::min<s32>(motion.yaw_step, 0);
    }
    player.state.camera_rotation.vy =
        (player.state.camera_rotation.vy + motion.yaw_step) & KF_ANGLE_WRAP_MASK;
    if (input & kf::Button::LookDown) {
        motion.pitch_step += PLAYER_PITCH_ACCEL;
        motion.pitch_step = std::min<s32>(motion.pitch_step, PLAYER_PITCH_STEP_LIMIT);
    } else if (input & kf::Button::LookUp) {
        motion.pitch_step -= PLAYER_PITCH_ACCEL;
        motion.pitch_step = std::max<s32>(motion.pitch_step, -PLAYER_PITCH_STEP_LIMIT);
    } else if (motion.pitch_step > 0) {
        motion.pitch_step -= PLAYER_PITCH_DECEL;
        motion.pitch_step = std::max<s32>(motion.pitch_step, 0);
    } else if (motion.pitch_step < 0) {
        motion.pitch_step += PLAYER_PITCH_DECEL;
        motion.pitch_step = std::min<s32>(motion.pitch_step, 0);
    }
    if (motion.pitch_step > 0) {
        player.state.camera_rotation.vx += motion.pitch_step;
        player.state.camera_rotation.vx =
            std::min<s32>(player.state.camera_rotation.vx, KF_PLAYER_CAMERA_PITCH_LIMIT);
    } else if (motion.pitch_step < 0) {
        player.state.camera_rotation.vx += motion.pitch_step;
        player.state.camera_rotation.vx =
            std::max<s32>(player.state.camera_rotation.vx, -KF_PLAYER_CAMERA_PITCH_LIMIT);
    }
}

static void player_update_darkness(PlayerContext &player)
{
    s32 fade;

    if (player.state.darkness_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!((player.state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE)
            && player.state.darkness_timer >= DARKNESS_FADE_STEPS + 1) {
            player.state.darkness_timer = DARKNESS_FADE_STEPS;
        }
        player.state.darkness_timer--;
        if (player.state.darkness_timer == KF_PLAYER_STATUS_TIMER_INACTIVE) {
            player.state.status_effect_flags &= ~KF_PLAYER_STATUS_DARKNESS;
        } else {
            if (!player.local_view || player.prediction) return;
            fade = (s16)(player.state.darkness_timer
                - (KF_DARKNESS_DURATION_UPDATES - DARKNESS_FADE_STEPS));
            if (fade < 0) {
                fade = (s16)(DARKNESS_FADE_STEPS - player.state.darkness_timer);
            }
            if (fade >= 0) {
                lighting_set_color_matrix(game_graphics_runtime.render_state, &player_darkness_color_matrix, color_matrix_table.data(),
                    fade << (KF_FIXED12_BITS - DARKNESS_FADE_BITS));
                fog_interpolate_near(game_graphics_runtime.render_state, PLAYER_DARKNESS_FOG_NEAR, KF_INITIAL_FOG_NEAR_DISTANCE,
                    fade << (KF_FIXED12_BITS - DARKNESS_FADE_BITS));
            } else {
                game_graphics_runtime.render_state.lighting.color_matrix.m = (player_darkness_color_matrix).m;
                fog_set_near(game_graphics_runtime.render_state, PLAYER_DARKNESS_FOG_NEAR);
            }
        }
    } else {
        if (player.local_view && !player.prediction)
            fog_set_near(game_graphics_runtime.render_state, KF_INITIAL_FOG_NEAR_DISTANCE);
    }
}

static void player_update_damage_reaction(PlayerContext &player)
{
    if (player.state.update_state != KF_PLAYER_UPDATE_NORMAL
        && player.state.update_state != KF_PLAYER_UPDATE_DYING) {
        if (player.state.update_state >= KF_PLAYER_DAMAGE_FRAME_END) {
            player.state.update_state = KF_PLAYER_UPDATE_NORMAL;
            player.state.view_rotation_offset = player_damage_view_rotation_offsets[kf_enum_encode<u8>(KF_PLAYER_UPDATE_NORMAL)];
            if (player.state.vitals.current_hp == 0) {
                player_death_begin(player);
            }
        } else {
            player.state.view_rotation_offset = player_damage_view_rotation_offsets[kf_enum_encode<u8>(player.state.update_state)];
            if (player.local_view && !player.prediction)
                lighting_set_active_color_matrix(KF_GAME_COLOR_DAMAGE);
            player.state.update_state++;
        }
    }
}

static void player_update_equipment_effects(PlayerContext &player)
{
    if (player.state.equipped_weapon_id != KF_OBJECT_NONE) {
        if (player.state.equipped_weapon_record->hp_regen_interval != 0
            && player.state.equipment_effect_ticks % player.state.equipped_weapon_record->hp_regen_interval == 0) {
            player_adjust_hp(player, 1);
        }
        if (player.state.equipped_weapon_record->mp_regen_interval != 0
            && player.state.equipment_effect_ticks % player.state.equipped_weapon_record->mp_regen_interval == 0) {
            player_adjust_mp(player, 1);
        }
    }
    if (player.state.equipped_head_armor_id != KF_OBJECT_NONE) {
        player_apply_armor_periodic_effects(player, *player.state.equipped_head_armor_record);
    }
    if (player.state.equipped_body_armor_id != KF_OBJECT_NONE) {
        player_apply_armor_periodic_effects(player, *player.state.equipped_body_armor_record);
    }
    if (player.state.equipped_shield_id != KF_OBJECT_NONE) {
        player_apply_armor_periodic_effects(player, *player.state.equipped_shield_record);
    }
    if (player.state.equipped_arm_armor_id != KF_OBJECT_NONE) {
        player_apply_armor_periodic_effects(player, *player.state.equipped_arm_armor_record);
    }
    if (player.state.equipped_leg_armor_id != KF_OBJECT_NONE) {
        player_apply_armor_periodic_effects(player, *player.state.equipped_leg_armor_record);
    }
    player.state.equipment_effect_ticks++;
}

static void player_apply_current_floor_hazard(WorldState &world, PlayerContext &player)
{
    KfMapAttribute attribute;

    attribute = player_current_map_attribute(world, player);
    switch (attribute) {
    default:
        // Only these two attributes apply contact damage here.
        break;
    case KF_MAP_ATTRIBUTE_PITFALL:
        if (player.state.update_state == KF_PLAYER_UPDATE_NORMAL) {
            player_apply_damage(player, PLAYER_PITFALL_CUTTING_DAMAGE, PLAYER_PITFALL_STRIKING_DAMAGE, PLAYER_PITFALL_PIERCING_DAMAGE, KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
        }
        break;
    case KF_MAP_ATTRIBUTE_POISON_HOLE:
        player_apply_damage(player, 0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
        break;
    }
}

static void player_update_status_effects(PlayerContext &player)
{
    u16 phase;

    if (player.state.slowed_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        do {
            if (!((player.state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE)) {
                player.state.slowed_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
            } else {
                player.state.slowed_timer--;
                if (player.state.slowed_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
                    break;
                }
            }
            player.state.status_effect_flags &= ~KF_PLAYER_STATUS_SLOWED;
        } while (0);
    }
    if (player.state.poison_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!((player.state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE)) {
            player.state.poison_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
        } else {
            player.state.poison_timer--;
        }
        if (player.state.poison_timer == KF_PLAYER_STATUS_TIMER_INACTIVE) {
            player.state.status_effect_flags &= ~KF_PLAYER_STATUS_POISON;
        } else {
            phase = player.state.poison_timer % POISON_DAMAGE_INTERVAL_UPDATES;
            if (phase < POISON_FLASH_UPDATES) {
                if (player.local_view && !player.prediction)
                    lighting_set_active_color_matrix(KF_GAME_COLOR_DAMAGE);
                if (phase == 0) {
                    player_adjust_hp(player, -1);
                }
            }
        }
    }
    if (player.state.curse_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!((player.state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE)) {
            player.state.curse_timer = 0;
        }
        if (player.state.curse_timer == 0) {
            player.state.status_effect_flags &= ~KF_PLAYER_STATUS_CURSE;
            player_recalculate_combat_stats(player);
        } else if (player.state.curse_timer == KF_CURSE_DURATION_UPDATES) {
            player_recalculate_combat_stats(player);
        }
        player.state.curse_timer--;
    }
    if (player.state.fire_defense_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (player.state.fire_defense_timer == 0) {
            player.state.status_effect_flags &= ~KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
            player_recalculate_combat_stats(player);
        } else if (player.state.fire_defense_timer == KF_FIRE_DEFENSE_DURATION_UPDATES) {
            player_recalculate_combat_stats(player);
        }
        if (player.local_view && !player.prediction)
            lighting_set_active_color_matrix(KF_GAME_COLOR_DEFENSE_EFFECT);
        player.state.fire_defense_timer--;
    }
    if (player.local_view && !player.prediction && player.state.equipped_weapon_id == KF_ITEM_SHADOW_BLADE) {
        lighting_apply_shadow_blade_environment();
    }
    if (player.state.illusion_staff_timer != KF_ILLUSION_STAFF_INACTIVE) {
        player.state.illusion_staff_timer--;
        if (player.local_view && !player.prediction)
            lighting_apply_illusion_staff_effect();
    }
}

static kf::FrameTask<void> player_restore_loaded_game(WorldState &world, PlayerContext &player)
{
    animation_cache_release_all();
    audio_close_vab(audio_state);
    (co_await map_load_floor_wrapper(world, player));
    player_sync_position_to_map(world, player);
    player.state.previous_map_cell.x = player.state.motion_state.map_cell.x;
    player.state.previous_map_cell.z = player.state.motion_state.map_cell.z;
    player_equip_weapon(player, player.state.equipped_weapon_id);
    player_select_magic(world, player, player.state.selected_magic_id);
}

kf::FrameTask<void> player_update(WorldState &world, PlayerContext &player, PlayerInput command)
{
    auto &motion = player.state.motion_state;
    u32 input;

    input = command.buttons;
    if (world.party.enabled) {
        input &= ~(kf::Button::Start | kf::Button::Back);
        if (!player.local_view || player.prediction)
            input &= ~(kf::Button::Confirm | kf::Button::Select);
    }
    const auto look = command.look;
    if (player.state.update_state == KF_PLAYER_UPDATE_DYING) {
        (co_await player_death_update(world, player));
        co_return;
    }
    if (player.state.update_state == KF_PLAYER_UPDATE_RECOVERY_FADE) {
        player_death_update_reverse_fade(player);
        co_return;
    }
    if (!world.party.enabled)
        collision_adjust_cell_occupancy(world, motion.map_cell.x, motion.map_cell.z, -1);
    player.state.camera_rotation.vy =
        (player.state.camera_rotation.vy + look.yaw) & KF_ANGLE_WRAP_MASK;
    player.state.camera_rotation.vx = std::clamp<s32>(
        player.state.camera_rotation.vx + look.pitch,
        -KF_PLAYER_CAMERA_PITCH_LIMIT, KF_PLAYER_CAMERA_PITCH_LIMIT);
    if ((input & kf::Button::Select) && !world.party.enabled) {
        (co_await display_show_system_screen(KF_SYSTEM_SCREEN_PAUSE));
    }
    if (input & kf::Button::Start) {
        input = kf::button_mask(kf::Button::Back);
    }
    if (kf::button_pressed(input, player.previous_input, kf::Button::Back)
        && player.state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE) {
        const auto result = co_await menu_open_root(world, player);
        if (const auto *selected = std::get_if<KfObjectId>(&result)) {
            co_await player_use_item(world, player, *selected);
        } else if (std::get<KfMenuAction>(result) == KfMenuAction::GameLoaded) {
            co_await player_restore_loaded_game(world, player);
        } else if (std::get<KfMenuAction>(result) == KfMenuAction::ReturnToIntro) {
            game_result = GameResult::ReturnToIntro;
            co_return;
        }
        player.previous_input = input;
    } else {
        if (kf::button_pressed(input, player.previous_input, kf::Button::Confirm)) {
            (co_await map_interaction_dispatch(world, player, &player.state.camera_position, &player.state.camera_rotation));
        }
        if (world.party.enabled)
            collision_adjust_cell_occupancy(world, motion.map_cell.x, motion.map_cell.z, -1);
        player_update_movement_input(world, player, input);
        player_update_view_input(player, input);
        if (kf::button_pressed(input, player.previous_input, kf::Button::Attack)) {
            player_begin_weapon_attack(player);
        }
        player_handle_magic_input(world, player, input);
        player_update_weapon_magic(world, player);
        player.previous_input = input;
        player_update_vertical_motion(world, player);
        if (world.party.enabled)
            collision_adjust_cell_occupancy(world, motion.map_cell.x, motion.map_cell.z, 1);
    }
    if (!world.party.enabled) {
        collision_adjust_cell_occupancy(world, motion.map_cell.x, motion.map_cell.z, 1);
        player_tick_effects(world, player);
    }
}

void player_tick_effects(WorldState &world, PlayerContext &player)
{
    if (player.cast_pose_ticks) --player.cast_pose_ticks;
    player_update_weapon_attack(world, player);
    if (player.local_view && !player.prediction)
        lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    player_update_darkness(player);
    player_update_damage_reaction(player);
    player_update_equipment_effects(player);
    player_apply_current_floor_hazard(world, player);
    player_update_status_effects(player);
}

void player_update_reset_module_state(void)
{
    kf::restore_initial_value<player_darkness_color_matrix>();
    kf::restore_initial_value<player_damage_view_rotation_offsets>();
}
