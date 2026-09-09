#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    EFFECT_ORBIT_RADIUS = 6500
};

enum {
    PROJECTILE_COLLISION_RADIUS = 100,
    EMERGING_COLLISION_RADIUS = 150,
    FIRE_DARKNESS_ROLL_STEP = 200,
    LIGHTNING_ROLL_STEP = 200,
    EMERGING_ROLL_STEP = 100,
    PROJECTILE_DEFAULT_ROLL_STEP = 600,
    PROJECTILE_DISSIPATE_SCALE_STEP = 400,
    EMERGING_FALL_ACCELERATION = 20,
    EMERGING_RETIRE_DEPTH = 3000,
    STATUS_PROJECTILE_SHRINK_STEP = 384,
    SCATTER_FINAL_COUNTDOWN = 15,
    SCATTER_BRANCH_COUNTDOWN = 5,
    SCATTER_PULSE_ANGLE_SHIFT = 9,
    MAP_EMITTER_SOUND_RANDOM_CUTOFF = (RAND_MAX + 1) / 4,
    MAP_EMITTER_SOUND_MAX_DISTANCE = 4000,
    MAP_EMITTER_SOUND_ATTENUATION_DISTANCE = 12000
};

enum {
    MOONLIGHT_LENGTH_STEP = KF_FIXED12_ONE / 4,
    MOONLIGHT_LENGTH_MAX = 0x7fff,
    MOONLIGHT_NEAR_TRAIL_ANGLE = 1774,
    MOONLIGHT_FAR_TRAIL_ANGLE = 1824,
    MOONLIGHT_NEAR_TRAIL_DISTANCE = 4000,
    MOONLIGHT_FAR_TRAIL_DISTANCE = 8000,
    GROUND_TRAIL_SHRINK_STEP = 400,
    GROUND_TRAIL_RENDER_FRAME_COUNT = 3
};

enum {
    RADIAL_BLAST_SCALE_STEP = KF_FIXED12_ONE / 4,
    RADIAL_BLAST_RADIUS_STEP = 333,
    EFFECT_PLAYER_RADIAL_SCALE_Q12 = 5000
};

enum {
    HOMING_PITCH_RANDOM_SHIFT = 6,
    HOMING_PITCH_RANDOM_BIAS = 128,
    HOMING_YAW_RANDOM_SHIFT = 3,
    HOMING_YAW_RANDOM_BIAS = 512,
    HOMING_WANDER_RANDOM_CUTOFF = 3276,
    HOMING_PLAYER_AIM_Y_OFFSET = 800,
    HOMING_TARGET_MAX_DISTANCE = 20000,
    HOMING_TARGET_CONE_ANGLE = 1365,
    HOMING_TURN_STEP = KF_ANGLE_FULL_TURN / 64,
    HOMING_FORWARD_STEP = 650,
    HOMING_ROLL_STEP = 256
};

enum {
    LIGHTNING_IMPACT_RENDER_FRAME_COUNT = 3,
    LIGHTNING_BLAST_SCALE_STEP = 3583,
    LIGHTNING_BLAST_YAW_STEP = 1300,
    LIGHTNING_BLAST_RADIUS_STEP = 1000
};

enum {
    GROUND_BRANCH_SCALE_STEP = KF_FIXED12_ONE / 16,
    GROUND_BRANCH_DAMAGE_PERIOD = 4,
    GROUND_BRANCH_DAMAGE_RADIUS = 1500,
    GROUND_BRANCH_YAW_STEP = 500,
    GROUND_VISUAL_ANGLE_RANDOM_SHIFT = 3,
    GROUND_VISUAL_RADIUS_RANDOM_SCALE = 325,
    GROUND_VISUAL_RADIUS_RANDOM_SHIFT = 13
};

enum {
    GROUND_VISUAL_SCALE_STEP = 4000,
    GROUND_VISUAL_RISE_STEP = 1800,
    GROUND_VISUAL_YAW_STEP = 500
};

enum {
    ACTOR_SPAWNER_SCALE_STEP = 220,
    ACTOR_SPAWNER_COLLISION_RADIUS = 1000,
    ACTOR_SPAWNER_CREATE_Y_OFFSET = 800,
    ACTOR_SPAWNER_SELECTION_RANDOM_CUTOFF = 3000,
    ACTOR_SPAWNER_YAW_STEP = 1700,
    ACTOR_SPAWNER_PITCH_STEP = 1400,
    ACTOR_SPAWNER_ROLL_STEP = 1900
};

enum {
    SHORT_SWING_PROBE_LENGTH = 2500,
    LONG_SWING_PROBE_LENGTH = 4900
};

/* Dense effect-kind dispatch table for kinds 4 through 52. */
RODATA(0x80012cf8, 0xc4)

DATA(0x80057b88, 0x10)
SVECTOR effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_COUNT] = {
    {0, SHORT_SWING_PROBE_LENGTH, 0, 0},
    {0, LONG_SWING_PROBE_LENGTH, 0, 0},
};

/*
 * Per-frame behavior dispatcher for the current effect.  The constructor and
 * effect_pool_sweep publish the active record and its magic definition through
 * current_effect/current_effect_magic_record before entering here.
 */
ADDRESS(0x80038a38, 0x180c)
void effect_update_dispatch(void)
{
    KfEffectRecord *effect = current_effect;
    KfMagicRecord *magic = current_effect_magic_record;
    KfMagicRecord *impact_magic;
    KfActor *target;
    const SoundRef *phase_sound;
    VECTOR impact_position;
    u32 collision;
    u16 collision_kind;
    KfEffectPhase phase;
    KfEffectKind kind;
    u32 radius;
    u16 angle;
    u16 distance;
    u16 power;
    u16 next;
    s16 desired_pitch;
    s32 value;

    kind = effect->kind;
    phase = effect->phase;
    radius = PROJECTILE_COLLISION_RADIUS;

    switch (kind) {
    case KF_EFFECT_KIND_EMERGING_PROJECTILE:
        radius = EMERGING_COLLISION_RADIUS;
    case KF_EFFECT_KIND_LIGHTNING_BOLT:
    case KF_EFFECT_KIND_FIRE_BALL:
    case KF_EFFECT_KIND_WIND_CUTTER:
    case KF_EFFECT_KIND_LIGHT_NEEDLE:
    case KF_EFFECT_KIND_SCATTER_PROJECTILE:
    case KF_EFFECT_KIND_DARKNESS_PROJECTILE:
    case KF_EFFECT_KIND_CURSE_PROJECTILE:
    case KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE:
    case KF_EFFECT_KIND_PHYSICAL_PROJECTILE:
        if (phase == KF_EFFECT_PROJECTILE_TRAVEL) {
            collision = effect_map_collision(&effect->position, radius);
            if (collision != KF_COLLISION_NONE) {
                u16 impact_power;

                impact_magic = current_effect_magic_record;
                collision_kind = collision >> KF_COLLISION_KIND_SHIFT;
                if (kind == KF_EFFECT_KIND_LIGHTNING_BOLT) {
                    goto lightning_impact;
                }
                impact_power = effect_magic_power(effect);
                if (kind != KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                    audio_play_spatial_default_range(
                        &impact_magic->sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                }
                if (collision_kind == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                    if (kind == KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE || kind == KF_EFFECT_KIND_PHYSICAL_PROJECTILE || kind == KF_EFFECT_KIND_WIND_CUTTER) {
                        actor_apply_damage(
                            (u16)collision, impact_power,
                            impact_magic->damage_components[0],
                            impact_magic->damage_components[2],
                            impact_magic->damage_components[1],
                            0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                    } else if (kind != KF_EFFECT_KIND_EMERGING_PROJECTILE) {
                        actor_apply_damage(
                            (u16)collision, impact_power,
                            0, 0, 0, impact_magic->damage_components[0],
                            impact_magic->damage_components[1],
                            KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                    }
                    if (kind == KF_EFFECT_KIND_WIND_CUTTER) {
                        goto advance_shared_projectile;
                    }
                } else if (collision_kind == (KF_COLLISION_PLAYER >> KF_COLLISION_KIND_SHIFT)) {
                    if (kind == KF_EFFECT_KIND_EMERGING_PROJECTILE || kind == KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE || kind == KF_EFFECT_KIND_PHYSICAL_PROJECTILE) {
                        player_apply_damage(
                            impact_magic->damage_components[0],
                            impact_magic->damage_components[2],
                            impact_magic->damage_components[1],
                            KF_PLAYER_STATUS_NONE, 0, 0, KF_FIXED12_ONE, effect->id);
                    } else if (kind == KF_EFFECT_KIND_DARKNESS_PROJECTILE) {
                        player_apply_damage(
                            0, 0, 0, KF_PLAYER_STATUS_DARKNESS, 0, 0, KF_FIXED12_ONE, effect->id);
                    } else if (kind == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                        player_apply_damage(
                            0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, KF_FIXED12_ONE, effect->id);
                    } else if (kind == KF_EFFECT_KIND_CURSE_PROJECTILE) {
                        player_apply_damage(0, 0, 0, KF_PLAYER_STATUS_CURSE, 0, 0, KF_FIXED12_ONE, effect->id);
                    } else {
                        player_apply_damage(
                            0, 0, 0, KF_PLAYER_STATUS_NONE,
                            impact_magic->damage_components[0],
                            impact_magic->damage_components[1],
                            KF_FIXED12_ONE, effect->id);
                    }
                    if (kind == KF_EFFECT_KIND_WIND_CUTTER) {
                        goto advance_shared_projectile;
                    }
                }

                if (kind == KF_EFFECT_KIND_EMERGING_PROJECTILE) {
                    effect->direction.words.y = 0;
                    effect->phase = KF_EFFECT_PROJECTILE_FALL;
                } else if (kind == KF_EFFECT_KIND_DARKNESS_PROJECTILE
                           || kind == KF_EFFECT_KIND_CURSE_PROJECTILE) {
                    effect->phase = KF_EFFECT_PROJECTILE_SHRINK;
                } else {
                    effect->phase = KF_EFFECT_PROJECTILE_IMPACT_FIRST;
                }
                return;
            }

advance_shared_projectile:
            addVector(&effect->position, &effect->direction.vector);
            if (kind == KF_EFFECT_KIND_FIRE_BALL || kind == KF_EFFECT_KIND_DARKNESS_PROJECTILE) {
                effect->rotation.vector.vz = (effect->rotation.vector.vz + FIRE_DARKNESS_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
                return;
            }
            if (kind == KF_EFFECT_KIND_LIGHTNING_BOLT) {
                s32 remaining;

                effect->rotation.vector.vz = (effect->rotation.vector.vz + LIGHTNING_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
                remaining = effect->control.frames_remaining - 1;
                effect->control.frames_remaining = remaining;
                if ((u16)remaining == 0) {
lightning_impact:
                    audio_play_spatial_default_range(
                        &magic->sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                    effect->phase = KF_EFFECT_PROJECTILE_DISSIPATE_FIRST;
                    impact_position.vx = effect->position.vx;
                    impact_position.vz = effect->position.vz;
                    impact_position.vy =
                        -(map_floor_height_grid.cells[effect->position.vz / KF_MAP_TILE_SIZE]
                                               [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP);
                    if (effect->base_render_id.billboard == KF_EFFECT_BILLBOARD_LIGHTNING_BOLT) {
                        effect_pool_construct(
                            effect->id, effect->type, KF_EFFECT_KIND_LIGHTNING_IMPACT,
                            &impact_position, &effect->rotation.vector);
                    } else {
                        effect_pool_construct(
                            effect->id, effect->type, KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE,
                            &impact_position, &effect->rotation.vector);
                    }
                } else {
                    effect->render_id.billboard = KF_ENUM_DECODE(KfEffectBillboardId,
                        KF_ENUM_ENCODE(u8, effect->base_render_id.billboard) + (effect->control.frames_remaining & 1));
                }
                return;
            }
            if (kind == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                KfEffectDirection scatter;
                s32 pulse_angle;

                if (--effect->control.frames_remaining == 0) {
                    if (effect->propagation.generations_remaining != 0) {
                        next = ((s16)effect->scale_x * 3) >> 2;
                        effect->visual.pulse_base_scale = next;
                        effect->scale_z = next;
                        effect->scale_y = next;
                        effect->scale_x = next;
                        effect->propagation.generations_remaining--;
                        scatter.vector = effect->direction.vector;
                        effect_scatter_triple(&scatter.words);
                        if (effect->propagation.generations_remaining == 0) {
                            effect->control.frames_remaining = SCATTER_FINAL_COUNTDOWN;
                        } else {
                            effect->control.frames_remaining = SCATTER_BRANCH_COUNTDOWN;
                        }
                        effect_pool_construct(
                            effect->id, effect->type, kind, &effect->position, &scatter.vector,
                            KF_EFFECT_ARGS_SCATTER(effect->propagation.generations_remaining, effect->control.frames_remaining, (s16)effect->scale_x));
                        effect_scatter_triple(&effect->direction.words);
                    } else {
                        effect->type = KF_EFFECT_SLOT_FREE;
                    }
                }
                pulse_angle = effect->control.frames_remaining << SCATTER_PULSE_ANGLE_SHIFT;
                value = rsin(pulse_angle);
                value = (effect->visual.pulse_base_scale * value) >> (KF_FIXED12_BITS + 1);
                next = effect->visual.pulse_base_scale + value;
                effect->scale_z = next;
                effect->scale_x = next;
                value = rcos(pulse_angle);
                effect->scale_y = effect->visual.pulse_base_scale
                    + ((effect->visual.pulse_base_scale * value) >> (KF_FIXED12_BITS + 1));
                return;
            }
            if (kind == KF_EFFECT_KIND_EMERGING_PROJECTILE) {
                effect->rotation.vector.vz = (effect->rotation.vector.vz + EMERGING_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
                return;
            }
            if (kind == KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE) {
                if (rand() < MAP_EMITTER_SOUND_RANDOM_CUTOFF && effect->sound_played == KF_AUDIO_NOT_PLAYED) {
                    effect->sound_played = audio_play_spatial_range(
                        &magic->sounds[0], &effect->position, KF_AUDIO_MAX_VOLUME,
                        MAP_EMITTER_SOUND_MAX_DISTANCE, MAP_EMITTER_SOUND_ATTENUATION_DISTANCE);
                }
                return;
            }
            if (kind == KF_EFFECT_KIND_LIGHT_NEEDLE || kind == KF_EFFECT_KIND_PHYSICAL_PROJECTILE) {
                return;
            }
            effect->rotation.vector.vz = (effect->rotation.vector.vz + PROJECTILE_DEFAULT_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
            return;
        }

        if (phase < KF_EFFECT_FIRE_BALL_IMPACT_END && kind == KF_EFFECT_KIND_FIRE_BALL) {
            effect->render_id.billboard = KF_ENUM_DECODE(KfEffectBillboardId,
                KF_ENUM_ENCODE(u8, effect->base_render_id.billboard) + KF_ENUM_ENCODE(u8, phase));
        } else if (phase < KF_EFFECT_PROJECTILE_IMPACT_END) {
            goto invalidate_and_advance;
        } else if (phase < KF_EFFECT_PROJECTILE_DISSIPATE_END) {
            s32 scale = effect->scale_x - PROJECTILE_DISSIPATE_SCALE_STEP;

            effect->scale_x = scale;
            effect->scale_z = scale;
            effect->scale_y = scale;
        } else if (phase == KF_EFFECT_PROJECTILE_DISSIPATE_END) {
            goto invalidate_and_advance;
        } else if (phase < KF_EFFECT_PROJECTILE_FALL) {
            effect->position.vy -= KF_EFFECT_EMERGE_Y_STEP;
            if (phase == KF_EFFECT_PROJECTILE_EMERGE_LAST) {
                effect->phase = KF_EFFECT_PROJECTILE_LAUNCH_WRAP;
                phase_sound = &magic->sounds[0];
play_phase_sound:
                audio_play_spatial_default_range(
                    phase_sound, &effect->position, KF_AUDIO_MAX_VOLUME);
            }
        } else if (phase == KF_EFFECT_PROJECTILE_FALL) {
            effect->direction.words.y += EMERGING_FALL_ACCELERATION;
            effect->position.vy += effect->direction.vector.vy;
            if (effect->position.vy
                > -(map_floor_height_grid.cells[effect->position.vz / KF_MAP_TILE_SIZE]
                                        [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP)
                    + EMERGING_RETIRE_DEPTH) {
                effect->type = KF_EFFECT_SLOT_FREE;
            }
            return;
        } else if (phase == KF_EFFECT_PROJECTILE_SHRINK) {
            s32 scale = effect->scale_y - STATUS_PROJECTILE_SHRINK_STEP;

            effect->scale_y = scale;
            effect->scale_z = scale;
            if ((s16)effect->scale_y < 0) {
                effect->type = KF_EFFECT_SLOT_FREE;
            }
            return;
        } else {
            goto invalidate_and_advance;
        }
        goto advance_effect_phase;

    case KF_EFFECT_KIND_MOONLIGHT_PROJECTILE:
        if (KF_ENUM_ENCODE(u8, phase) < KF_ENUM_ENCODE(u8, KF_EFFECT_MOONLIGHT_TRAVEL_LAST) + 1) {
            if (effect_map_collision(&effect->position, PROJECTILE_COLLISION_RADIUS) != KF_COLLISION_NONE) {
                effect->animation_clip = KF_ANIMATION_CLIP_NONE;
                effect->base_render_id.model = KF_EFFECT_MODEL_NONE;
                effect->render_id.model = KF_EFFECT_MODEL_NONE;
                effect->phase = KF_EFFECT_MOONLIGHT_IMPACT_FIRST;
                audio_play_spatial_default_range(
                    &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_RADIAL_BLAST)].sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                return;
            }
            addVector(&effect->position, &effect->direction.vector);
            effect->scale_z += MOONLIGHT_LENGTH_STEP;
            if ((s16)effect->scale_z < 0) {
                effect->scale_z = MOONLIGHT_LENGTH_MAX;
            }
            if (phase == KF_EFFECT_MOONLIGHT_TRAIL_EMIT_PHASE) {
                effect_spawn_ground_trail(effect->id, effect, MOONLIGHT_NEAR_TRAIL_ANGLE, MOONLIGHT_NEAR_TRAIL_DISTANCE);
                effect_spawn_ground_trail(effect->id, effect, -MOONLIGHT_NEAR_TRAIL_ANGLE, MOONLIGHT_NEAR_TRAIL_DISTANCE);
                effect_spawn_ground_trail(effect->id, effect, MOONLIGHT_FAR_TRAIL_ANGLE, MOONLIGHT_FAR_TRAIL_DISTANCE);
                effect_spawn_ground_trail(effect->id, effect, -MOONLIGHT_FAR_TRAIL_ANGLE, MOONLIGHT_FAR_TRAIL_DISTANCE);
            }
            if (phase == KF_EFFECT_MOONLIGHT_TRAVEL_LAST) {
                return;
            }
        } else {
            if (((KF_ENUM_ENCODE(u8, phase) - KF_ENUM_ENCODE(u8, KF_EFFECT_MOONLIGHT_IMPACT_FIRST)) & 1) == 0) {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_RADIAL_BLAST,
                    &effect->position, &effect->direction.vector, KF_EFFECT_ARGS_SOUND(KF_EFFECT_SOUND_PLAY));
            }
            if (phase > KF_EFFECT_MOONLIGHT_IMPACT_LAST) {
                goto invalidate_and_return;
            }
        }
        goto advance_effect_phase;

    case KF_EFFECT_KIND_GROUND_TRAIL: {
        KfEffectRecord *linked_effect;
        u32 collision;
        u16 collision_kind;
        u16 power;

        linked_effect = &effect_pool_records[effect->control.parent_effect_index];
        collision = effect_map_collision(&effect->position, radius);
        if (collision != KF_COLLISION_NONE) {
            collision_kind = collision >> KF_COLLISION_KIND_SHIFT;
            power = effect_magic_power(effect);
            if (collision_kind == (KF_COLLISION_ACTOR >> KF_COLLISION_KIND_SHIFT)) {
                actor_apply_damage(
                    (u16)collision, power, 0, 0, 0,
                    magic->damage_components[0], magic->damage_components[1],
                    KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
            }
        }
        addVector(&effect->position, &effect->direction.vector);
        effect->position.vy =
            -(map_floor_height_grid.cells[effect->position.vz / KF_MAP_TILE_SIZE]
                                   [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP);
        switch (phase) {
        case KF_EFFECT_GROUND_TRAIL_WAIT_FOR_PARENT:
            if (KF_ENUM_ENCODE(u8, linked_effect->phase) > KF_ENUM_ENCODE(u8, KF_EFFECT_MOONLIGHT_IMPACT_FIRST) - 1) {
                effect->phase = KF_EFFECT_GROUND_TRAIL_SHRINK;
            }
            break;
        case KF_EFFECT_GROUND_TRAIL_SHRINK: {
            s32 scale = effect->scale_x - GROUND_TRAIL_SHRINK_STEP;

            effect->scale_x = scale;
            effect->scale_y = effect->scale_z = scale;
            if ((s16)scale <= 0) {
                effect->type = KF_EFFECT_SLOT_FREE;
            }
            break;
        }
        }
        effect->render_id.billboard++;
        if (KF_ENUM_ENCODE(u8, effect->render_id.billboard) >= KF_ENUM_ENCODE(u8, effect->base_render_id.billboard) + GROUND_TRAIL_RENDER_FRAME_COUNT) {
            effect->render_id.billboard = effect->base_render_id.billboard;
        }
        break;
    }

    case KF_EFFECT_KIND_RADIAL_BLAST:
        if (++effect->phase < KF_EFFECT_RADIAL_BLAST_PHASE_END) {
            u32 damage_radius;

            effect->scale_y = effect->scale_z =
                effect->scale_x += RADIAL_BLAST_SCALE_STEP;
            damage_radius = KF_ENUM_ENCODE(u8, phase) * RADIAL_BLAST_RADIUS_STEP;
            power = effect_magic_power(effect);
            if (KF_ENUM_ENCODE(u8, effect->phase) & 1) {
                actor_pool_apply_radial_damage(
                    &effect->position,
                    damage_radius, KF_FIXED12_ONE, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                player_apply_radial_damage(
                    &effect->position,
                    damage_radius, KF_FIXED12_ONE, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], EFFECT_PLAYER_RADIAL_SCALE_Q12, effect->id);
            }
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        break;

    case KF_EFFECT_KIND_HOMING_PROJECTILE: {
        SVECTOR local_motion;
        VECTOR movement;
        MATRIX matrix;
        s32 target_distance;

        if (phase == KF_EFFECT_PHASE_INIT) {
randomize_homing_direction:
            effect->direction.words.x =
                (effect->direction.words.x + (rand() >> HOMING_PITCH_RANDOM_SHIFT) - HOMING_PITCH_RANDOM_BIAS) & KF_ANGLE_WRAP_MASK;
            effect->direction.words.y =
                (effect->direction.words.y + (rand() >> HOMING_YAW_RANDOM_SHIFT) - HOMING_YAW_RANDOM_BIAS) & KF_ANGLE_WRAP_MASK;
        } else if (phase > KF_EFFECT_HOMING_INITIAL_PHASE_LAST) {
            if (effect->control.target_mode == KF_EFFECT_HOMING_WANDER) {
                if (rand() < HOMING_WANDER_RANDOM_CUTOFF) {
                    goto randomize_homing_direction;
                }
            } else if (effect->control.target_mode == KF_EFFECT_HOMING_PLAYER) {
                s32 aim_height;

                effect->direction.words.y = vector_xz_to_angle(
                    player_state.camera_position.vx - effect->position.vx,
                    effect->position.vz - player_state.camera_position.vz);
                aim_height = effect->position.vy - HOMING_PLAYER_AIM_Y_OFFSET;
                desired_pitch = vector_xz_to_angle(
                    aim_height - player_state.camera_position.vy,
                    /* Retail reads the shared distance slot before this branch
                     * has initialized it; preserve that original behavior. */
                    -target_distance);
                effect->direction.words.x = -desired_pitch & KF_ANGLE_WRAP_MASK;
            } else {
                target = actor_pool_find_target_in_cone(
                    &effect->position,
                    effect->rotation.vector.vy, HOMING_TARGET_MAX_DISTANCE, HOMING_TARGET_CONE_ANGLE, &target_distance);
                if (target != 0) {
                    KfActorDefinition *definition =
                        &actor_state.definitions.entries[target->definition_id];

                    effect->direction.words.y = vector_xz_to_angle(
                        target->position.vx - effect->position.vx,
                        effect->position.vz - target->position.vz);
                    desired_pitch = vector_xz_to_angle(
                        effect->position.vy
                            - (target->position.vy
                               - (definition->collision_height >> 1)),
                        -target_distance);
                    effect->direction.words.x = -desired_pitch & KF_ANGLE_WRAP_MASK;
                } else {
                    effect->direction.words.x = 0;
                }
            }
            effect->phase = KF_EFFECT_HOMING_TRACKING_PHASE;
        }

        effect->rotation.vector.vx = angle_approach(
            effect->rotation.vector.vx, effect->direction.vector.vx, HOMING_TURN_STEP);
        effect->rotation.vector.vy = angle_approach(
            effect->rotation.vector.vy, effect->direction.vector.vy, HOMING_TURN_STEP);
        local_motion.vy = 0;
        local_motion.vx = 0;
        local_motion.vz = HOMING_FORWARD_STEP;
        matrix_set_rotation_x(effect->rotation.vector.vx, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        copyVector(&local_motion, &movement);
        matrix_set_rotation_y(effect->rotation.vector.vy, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        addVector(&effect->position, &movement);
        effect->phase++;
        effect->rotation.vector.vz = (effect->rotation.vector.vz + HOMING_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
        if (effect_map_collision(&effect->position, radius) != KF_COLLISION_NONE) {
            if (effect->base_render_id.model == KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE) {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_RADIAL_BLAST_ALTERNATE,
                    &effect->position, &effect->direction.vector, KF_EFFECT_ARGS_SOUND(KF_EFFECT_SOUND_PLAY));
            } else {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_RADIAL_BLAST,
                    &effect->position, &effect->direction.vector, KF_EFFECT_ARGS_SOUND(KF_EFFECT_SOUND_PLAY));
            }
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        break;
    }

    case KF_EFFECT_KIND_LIGHTNING_IMPACT:
        if (phase > KF_EFFECT_LIGHTNING_IMPACT_PHASE_LAST) {
            goto invalidate_and_advance;
        }
        effect->render_id.billboard++;
        if (KF_ENUM_ENCODE(u8, effect->render_id.billboard) >= KF_ENUM_ENCODE(u8, effect->base_render_id.billboard) + LIGHTNING_IMPACT_RENDER_FRAME_COUNT) {
            effect->render_id.billboard = effect->base_render_id.billboard;
        }
        if (phase == KF_EFFECT_LIGHTNING_IMPACT_EMIT_FIRST || phase == KF_EFFECT_LIGHTNING_IMPACT_EMIT_SECOND || phase == KF_EFFECT_LIGHTNING_IMPACT_EMIT_LAST) {
            if (effect->base_render_id.billboard == KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT) {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST,
                    &effect->position, &effect->rotation.vector);
            } else {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST_ALTERNATE,
                    &effect->position, &effect->rotation.vector);
            }
            if (phase == KF_EFFECT_LIGHTNING_IMPACT_EMIT_FIRST) {
                phase_sound = &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].sounds[1];
                goto play_phase_sound;
            }
        }
        goto advance_effect_phase;

    case KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST: {
        VECTOR position;

        if (phase > KF_EFFECT_LIGHTNING_BLAST_PHASE_LAST) {
            goto invalidate_and_advance;
        }
        effect->scale_y = effect->scale_z =
            effect->scale_x += LIGHTNING_BLAST_SCALE_STEP;
        effect->rotation.vector.vy = (effect->rotation.vector.vy + LIGHTNING_BLAST_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        if (KF_ENUM_ENCODE(u8, phase) & 1) {
            u32 damage_radius;
            KfMagicRecord *lightning_magic;

            setVector(&position,
                effect->position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                effect->position.vz);
            damage_radius = KF_ENUM_ENCODE(u8, phase) * LIGHTNING_BLAST_RADIUS_STEP;
            power = effect_magic_power(effect);
            lightning_magic = &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)];
            actor_pool_apply_radial_damage(
                &position, damage_radius, KF_FIXED12_ONE, power, 0, 0, 0,
                lightning_magic->damage_components[0],
                lightning_magic->damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
            player_apply_radial_damage(
                &position, damage_radius, KF_FIXED12_ONE, power, 0, 0, 0,
                lightning_magic->damage_components[0],
                lightning_magic->damage_components[1], EFFECT_PLAYER_RADIAL_SCALE_Q12, effect->id);
        }
        goto advance_effect_phase;
    }

invalidate_and_advance:
        effect->type = KF_EFFECT_SLOT_FREE;
advance_effect_phase:
        effect->phase++;
        break;

    case KF_EFFECT_KIND_GROUND_BRANCH:
        if (phase < KF_EFFECT_GROUND_BRANCH_GROW_END) {
            if (effect->control.frames_remaining != KF_EFFECT_GROUND_BRANCH_TIMER_DONE) {
                if (effect->control.frames_remaining-- == 0) {
                    switch (effect->propagation.branch) {
                    case KF_EFFECT_GROUND_BRANCH_ROOT:
                        effect_spawn_ground_branch(
                            effect->id, effect, KF_ANGLE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_QUARTER_TURN);
                        effect_spawn_ground_branch(
                            effect->id, effect, KF_ANGLE_THREE_QUARTER_TURN,
                            KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN);
                        break;
                    case KF_EFFECT_GROUND_BRANCH_QUARTER_TURN:
                        effect_spawn_ground_branch(
                            effect->id, effect, KF_ANGLE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_LEAF);
                        break;
                    case KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN:
                        effect_spawn_ground_branch(
                            effect->id, effect, KF_ANGLE_THREE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_LEAF);
                        break;
                    }
                    effect->control.frames_remaining = KF_EFFECT_GROUND_BRANCH_TIMER_DONE;
                }
            }
            effect->scale_y += GROUND_BRANCH_SCALE_STEP;
            if (KF_ENUM_ENCODE(u8, phase) == KF_ENUM_ENCODE(u8, KF_EFFECT_GROUND_BRANCH_GROW_END) - 1) {
                switch (effect->propagation.branch) {
                case KF_EFFECT_GROUND_BRANCH_ROOT:
                    effect->phase = KF_EFFECT_GROUND_BRANCH_ROOT_HOLD_BASE;
                    break;
                case KF_EFFECT_GROUND_BRANCH_QUARTER_TURN:
                case KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN:
                    effect->phase = KF_EFFECT_GROUND_BRANCH_SIDE_HOLD_BASE;
                    break;
                case KF_EFFECT_GROUND_BRANCH_LEAF:
                    effect->phase = KF_EFFECT_GROUND_BRANCH_LEAF_HOLD_BASE;
                    break;
                }
            }
        } else if (phase < KF_EFFECT_GROUND_BRANCH_SHRINK_FIRST) {
            if ((KF_ENUM_ENCODE(u8, phase) & (GROUND_BRANCH_DAMAGE_PERIOD - 1)) == 0) {
                VECTOR spawn_position;

                angle = (u32)rand() >> GROUND_VISUAL_ANGLE_RANDOM_SHIFT;
                distance = ((u32)rand() * GROUND_VISUAL_RADIUS_RANDOM_SCALE) >> GROUND_VISUAL_RADIUS_RANDOM_SHIFT;
                spawn_position.vx = effect->position.vx
                    + ((rsin(angle) * distance) >> KF_FIXED12_BITS);
                spawn_position.vz = effect->position.vz
                    + ((rcos(angle) * distance) >> KF_FIXED12_BITS);
                spawn_position.vy = effect->position.vy;
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_GROUND_BRANCH_VISUAL,
                    &spawn_position, &effect->rotation.vector);
                power = effect_magic_power(effect);
                actor_pool_apply_radial_damage(
                    &effect->position,
                    GROUND_BRANCH_DAMAGE_RADIUS, KF_FIXED12_ONE, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                player_apply_radial_damage(
                    &effect->position,
                    GROUND_BRANCH_DAMAGE_RADIUS, KF_FIXED12_ONE, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], EFFECT_PLAYER_RADIAL_SCALE_Q12, effect->id);
            }
        } else if (phase < KF_EFFECT_GROUND_BRANCH_PHASE_END) {
            effect->scale_y -= GROUND_BRANCH_SCALE_STEP;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        effect->rotation.vector.vy = (effect->rotation.vector.vy + GROUND_BRANCH_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        effect->phase++;
        break;

    case KF_EFFECT_KIND_GROUND_BRANCH_VISUAL:
        if (phase < KF_EFFECT_GROUND_VISUAL_SHRINK_FIRST) {
            effect->scale_y += GROUND_VISUAL_SCALE_STEP;
        } else if (phase < KF_EFFECT_GROUND_VISUAL_PHASE_END) {
            effect->scale_y -= GROUND_VISUAL_SCALE_STEP;
            effect->position.vy -= GROUND_VISUAL_RISE_STEP;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        effect->rotation.vector.vy = (effect->rotation.vector.vy + GROUND_VISUAL_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        effect->phase++;
        break;

    case KF_EFFECT_KIND_ACTOR_SPAWNER: {
        s32 scale;
        KfEffectPhase scale_phase;

        if (phase < KF_EFFECT_ACTOR_SPAWNER_TRAVEL_FIRST) {
            scale = effect->scale_x + ACTOR_SPAWNER_SCALE_STEP;
            scale_phase = effect->phase;
            goto publish_actor_spawner_scale;
        } else if (KF_ENUM_ENCODE(u8, phase) < KF_ENUM_ENCODE(u8, KF_EFFECT_ACTOR_SPAWNER_TRAVEL_LAST) + 1) {
            VECTOR position;

            position.vx = effect->position.vx + effect->direction.vector.vx;
            position.vz = effect->position.vz + effect->direction.vector.vz;
            position.vy = effect->position.vy;
            value = collision_query_world(
                position.vx, position.vy, position.vz, ACTOR_SPAWNER_COLLISION_RADIUS, 0,
                KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
            if ((phase == KF_EFFECT_ACTOR_SPAWNER_TRAVEL_LAST && value != KF_COLLISION_NONE) || effect->control.frames_remaining == 0) {
                effect->phase = KF_EFFECT_ACTOR_SPAWNER_WAIT_FIRST;
            } else {
                effect->position.vx = position.vx;
                effect->position.vz = position.vz;
                effect->control.frames_remaining--;
            }
            if (phase == KF_EFFECT_ACTOR_SPAWNER_TRAVEL_LAST) {
                goto rotate_actor_spawner;
            }
            goto advance_actor_spawner_phase;
        } else if (phase < KF_EFFECT_ACTOR_SPAWNER_SHRINK_FIRST) {
            if (phase == KF_EFFECT_ACTOR_SPAWNER_CREATE_PHASE) {
                struct KfVec3s actor_rotation;
                VECTOR position;

                setVector(&position,
                    effect->position.vx,
                    effect->position.vy + ACTOR_SPAWNER_CREATE_Y_OFFSET,
                    effect->position.vz);
                actor_rotation.x = 0;
                actor_rotation.z = 0;
                actor_rotation.y = vector_xz_to_angle(
                    player_state.camera_position.vx - position.vx,
                    player_state.camera_position.vz - position.vz);
                value = rand();
                if (value < ACTOR_SPAWNER_SELECTION_RANDOM_CUTOFF) {
                    actor_pool_spawn(2, &position, &actor_rotation);
                } else if (rand() < ACTOR_SPAWNER_SELECTION_RANDOM_CUTOFF) {
                    actor_pool_spawn(4, &position, &actor_rotation);
                } else {
                    actor_pool_spawn(0, &position, &actor_rotation);
                }
            }
advance_actor_spawner_phase:
            effect->phase++;
            goto rotate_actor_spawner;
        } else if (phase < KF_EFFECT_ACTOR_SPAWNER_PHASE_END) {
            scale = effect->scale_x - ACTOR_SPAWNER_SCALE_STEP;
            scale_phase = effect->phase;
publish_actor_spawner_scale:
            effect->scale_x = scale;
            effect->scale_z = scale;
            effect->scale_y = scale;
            scale_phase++;
            effect->phase = scale_phase;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
rotate_actor_spawner:
        effect->rotation.vector.vy = (effect->rotation.vector.vy + ACTOR_SPAWNER_YAW_STEP) & KF_ANGLE_WRAP_MASK;
        effect->rotation.vector.vx = (effect->rotation.vector.vx + ACTOR_SPAWNER_PITCH_STEP) & KF_ANGLE_WRAP_MASK;
        effect->rotation.vector.vz = (effect->rotation.vector.vz + ACTOR_SPAWNER_ROLL_STEP) & KF_ANGLE_WRAP_MASK;
        break;
    }

    case KF_EFFECT_KIND_SWINGING_HAZARD_SHORT:
        effect_projectile_update_3d(&effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_SHORT], KF_EFFECT_SHORT_SWING_PHASE_LIMIT);
        break;

    case KF_EFFECT_KIND_SWINGING_HAZARD_LONG:
        effect_projectile_update_3d(&effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_LONG], KF_EFFECT_LONG_SWING_PHASE_LIMIT);
        break;

    case KF_EFFECT_KIND_FLOOR_DEFORMATION: {
        s16 column;
        s32 remaining;

        switch (phase) {
        case KF_EFFECT_FLOOR_DEFORM_ADVANCE:
            remaining = effect->direction.words.x - 1;
            effect->direction.words.x = remaining;
            if ((s16)remaining == -1) {
                effect->phase = KF_EFFECT_FLOOR_DEFORM_HOLD;
                return;
            }
            effect->direction.words.z += effect->rotation.vector.vz;
            for (column = 0; column < effect->rotation.vector.vy; column++) {
                effect_floor_deform_line(
                    effect->rotation.vector.vx + column,
                    effect->direction.vector.vz,
                    -effect->direction.vector.vy);
            }
            break;
        case KF_EFFECT_FLOOR_DEFORM_HOLD:
            if (--effect->position.vy != -1) {
                return;
            }
            effect->phase = KF_EFFECT_FLOOR_DEFORM_REVERSE;
            effect->direction.words.z = KF_FIXED12_ONE;
            effect->direction.words.x = effect->position.vx;
            break;
        case KF_EFFECT_FLOOR_DEFORM_REVERSE:
            remaining = effect->direction.words.x - 1;
            effect->direction.words.x = remaining;
            if ((s16)remaining == -1) {
invalidate_and_return:
                effect->type = KF_EFFECT_SLOT_FREE;
                return;
            }
            effect->direction.words.z -= effect->rotation.vector.vz;
            for (column = 0; column < effect->rotation.vector.vy; column++) {
                effect_floor_deform_line(
                    effect->rotation.vector.vx + column,
                    effect->direction.vector.vz,
                    effect->direction.vector.vy);
            }
            break;
        }
        break;
    }

    case KF_EFFECT_KIND_ORBITING_PROJECTILE:
        effect_projectile_update_2d(EFFECT_ORBIT_RADIUS, KF_EFFECT_ORBIT_PHASE_LIMIT);
        break;

    default:
        break;
    }
}
