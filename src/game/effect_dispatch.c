#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Dense effect-kind dispatch table for kinds 4 through 52. */
RODATA(0x80012cf8, 0xc4)

DATA(0x80057b88, 0x10)
SVECTOR effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_COUNT] = {
    {0, 2500, 0, 0},
    {0, 4900, 0, 0},
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
    KfEffectRecord *linked_effect;
    KfActor *target;
    const SoundRef *phase_sound;
    VECTOR impact_position;
    u32 collision;
    u16 collision_kind;
    u8 phase;
    u8 kind;
    u32 radius;
    u16 angle;
    u16 distance;
    u16 power;
    u16 next;
    s16 desired_pitch;
    s32 value;

    kind = effect->kind;
    phase = effect->phase;
    radius = 100;

    switch (kind) {
    case KF_EFFECT_KIND_EMERGING_PROJECTILE:
        radius = 150;
        goto shared_projectile;
    case KF_EFFECT_KIND_LIGHTNING_BOLT:
    case KF_EFFECT_KIND_FIRE_BALL:
    case KF_EFFECT_KIND_WIND_CUTTER:
    case KF_EFFECT_KIND_LIGHT_NEEDLE:
    case KF_EFFECT_KIND_SCATTER_PROJECTILE:
    case KF_EFFECT_KIND_DARKNESS_PROJECTILE:
    case KF_EFFECT_KIND_CURSE_PROJECTILE:
    case 14:
    case 22:
shared_projectile:
        if (phase == KF_EFFECT_PROJECTILE_TRAVEL) {
            collision = effect_map_collision(&effect->position, radius);
            if (collision != (u32)KF_COLLISION_NONE) {
                impact_magic = current_effect_magic_record;
                collision_kind = collision >> 16;
                if (kind == KF_EFFECT_KIND_LIGHTNING_BOLT) {
                    goto effect_kind4_impact;
                }
                power = effect_magic_power(effect);
                if (kind != KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                    audio_play_spatial_default_range(
                        &impact_magic->sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                }
                if (collision_kind == (KF_COLLISION_ACTOR >> 16)) {
                    if (kind == 14 || kind == 22 || kind == KF_EFFECT_KIND_WIND_CUTTER) {
                        actor_apply_damage(
                            (u16)collision, power,
                            impact_magic->damage_components[0],
                            impact_magic->damage_components[2],
                            impact_magic->damage_components[1],
                            0, 0, KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                    } else if (kind != KF_EFFECT_KIND_EMERGING_PROJECTILE) {
                        actor_apply_damage(
                            (u16)collision, power,
                            0, 0, 0, impact_magic->damage_components[0],
                            impact_magic->damage_components[1],
                            KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                    }
                    if (kind == KF_EFFECT_KIND_WIND_CUTTER) {
                        goto advance_shared_projectile;
                    }
                } else if (collision_kind == (KF_COLLISION_PLAYER >> 16)) {
                    if (kind == KF_EFFECT_KIND_EMERGING_PROJECTILE || kind == 14 || kind == 22) {
                        player_apply_damage(
                            impact_magic->damage_components[0],
                            impact_magic->damage_components[2],
                            impact_magic->damage_components[1],
                            0, 0, 0, KF_FIXED12_ONE, effect->id);
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
                            0, 0, 0, 0,
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
            effect->position.vx += (s16)effect->direction.words.x;
            effect->position.vy += (s16)effect->direction.words.y;
            effect->position.vz += (s16)effect->direction.words.z;
            if (kind == KF_EFFECT_KIND_FIRE_BALL || kind == KF_EFFECT_KIND_DARKNESS_PROJECTILE) {
                effect->rotation.vz = (effect->rotation.vz + 200) & KF_ANGLE_WRAP_MASK;
                return;
            }
            if (kind == KF_EFFECT_KIND_LIGHTNING_BOLT) {
                s32 remaining;

                effect->rotation.vz = (effect->rotation.vz + 200) & KF_ANGLE_WRAP_MASK;
                remaining = effect->control.frames_remaining - 1;
                effect->control.frames_remaining = remaining;
                if ((u16)remaining == 0) {
effect_kind4_impact:
                    audio_play_spatial_default_range(
                        &magic->sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                    effect->phase = KF_EFFECT_PROJECTILE_DISSIPATE_FIRST;
                    impact_position.vx = effect->position.vx;
                    impact_position.vz = effect->position.vz;
                    impact_position.vy =
                        -(map_floor_height_grid[effect->position.vz / KF_MAP_TILE_SIZE]
                                               [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP);
                    if (effect->base_render_id == 6) {
                        effect_pool_construct(
                            effect->id, effect->type, 0x20,
                            &impact_position, &effect->rotation);
                    } else {
                        effect_pool_construct(
                            effect->id, effect->type, 0x29,
                            &impact_position, &effect->rotation);
                    }
                } else {
                    effect->render_id =
                        effect->base_render_id + ((u8)effect->control.frames_remaining & 1);
                }
                return;
            }
            if (kind == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                SVECTOR scatter;
                s32 pulse_angle;

                if (--effect->control.frames_remaining == 0) {
                    if (effect->propagation.generations_remaining != 0) {
                        next = ((s16)effect->scale_x * 3) >> 2;
                        effect->visual.pulse_base_scale = next;
                        effect->scale_z = next;
                        effect->scale_y = next;
                        effect->scale_x = next;
                        effect->propagation.generations_remaining--;
                        scatter = effect->direction.vector;
                        effect_scatter_triple((u16 *)&scatter);
                        effect->control.frames_remaining = effect->propagation.generations_remaining == 0 ? 15 : 5;
                        effect_pool_construct(
                            effect->id, effect->type, kind, &effect->position, &scatter,
                            effect->propagation.generations_remaining, effect->control.frames_remaining,
                            (s16)effect->scale_x);
                        effect_scatter_triple(&effect->direction.words.x);
                    } else {
                        effect->type = KF_EFFECT_SLOT_FREE;
                    }
                }
                pulse_angle = effect->control.frames_remaining << 9;
                value = rsin(pulse_angle);
                next = effect->visual.pulse_base_scale
                    + ((effect->visual.pulse_base_scale * value) >> 13);
                effect->scale_z = next;
                effect->scale_x = next;
                value = rcos(pulse_angle);
                effect->scale_y = effect->visual.pulse_base_scale
                    + ((effect->visual.pulse_base_scale * value) >> 13);
                return;
            }
            if (kind == KF_EFFECT_KIND_EMERGING_PROJECTILE) {
                effect->rotation.vz = (effect->rotation.vz + 100) & KF_ANGLE_WRAP_MASK;
                return;
            }
            if (kind == 14) {
                if (rand() < 8192 && effect->sound_played == 0) {
                    effect->sound_played = audio_play_spatial_range(
                        &magic->sounds[0], &effect->position, KF_AUDIO_MAX_VOLUME,
                        4000, 12000);
                }
                return;
            }
            if (kind == KF_EFFECT_KIND_LIGHT_NEEDLE || kind == 22) {
                return;
            }
            effect->rotation.vz = (effect->rotation.vz + 600) & KF_ANGLE_WRAP_MASK;
            return;
        }

        if (phase < 5 && kind == KF_EFFECT_KIND_FIRE_BALL) {
            effect->render_id = effect->base_render_id + phase;
        } else if (phase < 10) {
            goto invalidate_and_advance;
        } else if (phase < KF_EFFECT_PROJECTILE_DISSIPATE_END) {
            s32 scale = effect->scale_x - 400;

            effect->scale_x = scale;
            effect->scale_z = scale;
            effect->scale_y = scale;
        } else if (phase == KF_EFFECT_PROJECTILE_DISSIPATE_END) {
            goto invalidate_and_advance;
        } else if (phase < KF_EFFECT_PROJECTILE_FALL) {
            effect->position.vy -= 175;
            if (phase == KF_EFFECT_PROJECTILE_EMERGE_LAST) {
                effect->phase = KF_EFFECT_PROJECTILE_LAUNCH_WRAP;
                phase_sound = &magic->sounds[0];
play_phase_sound:
                audio_play_spatial_default_range(
                    phase_sound, &effect->position, KF_AUDIO_MAX_VOLUME);
            }
        } else if (phase == KF_EFFECT_PROJECTILE_FALL) {
            effect->direction.words.y += 20;
            effect->position.vy += (s16)effect->direction.words.y;
            if (effect->position.vy
                > -(map_floor_height_grid[effect->position.vz / KF_MAP_TILE_SIZE]
                                        [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP)
                    + 3000) {
                effect->type = KF_EFFECT_SLOT_FREE;
            }
            return;
        } else if (phase == KF_EFFECT_PROJECTILE_SHRINK) {
            s32 scale = effect->scale_y - 0x180;

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
        if (phase < 11) {
            if (effect_map_collision(&effect->position, 100) != (u32)KF_COLLISION_NONE) {
                effect->animation_clip = KF_EFFECT_ANIMATION_BILLBOARD;
                effect->base_render_id = KF_EFFECT_RENDER_NONE;
                effect->render_id = KF_EFFECT_RENDER_NONE;
                effect->phase = 20;
                audio_play_spatial_default_range(
                    &magic_records[KF_EFFECT_KIND_RADIAL_BLAST].sounds[1], &effect->position, KF_AUDIO_MAX_VOLUME);
                return;
            }
            effect->position.vx += (s16)effect->direction.words.x;
            effect->position.vy += (s16)effect->direction.words.y;
            effect->position.vz += (s16)effect->direction.words.z;
            effect->scale_z += 0x400;
            if ((s16)effect->scale_z < 0) {
                effect->scale_z = 0x7fff;
            }
            if (phase == 2) {
                effect_spawn_trail_kind13(effect->id, effect, 0x6ee, 4000);
                effect_spawn_trail_kind13(effect->id, effect, -0x6ee, 4000);
                effect_spawn_trail_kind13(effect->id, effect, 0x720, 8000);
                effect_spawn_trail_kind13(effect->id, effect, -0x720, 8000);
            }
            if (phase == 10) {
                return;
            }
        } else {
            if (((phase - 20) & 1) == 0) {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_RADIAL_BLAST,
                    &effect->position, &effect->direction.vector, 1);
            }
            if (phase > 23) {
                goto invalidate_and_return;
            }
        }
        goto advance_effect_phase;

    case KF_EFFECT_KIND_GROUND_TRAIL:
        linked_effect = &effect_pool_records[(u8)effect->control.parent_effect_index];
        collision = effect_map_collision(&effect->position, radius);
        if (collision != (u32)KF_COLLISION_NONE) {
            collision_kind = collision >> 16;
            power = effect_magic_power(effect);
            if (collision_kind == (KF_COLLISION_ACTOR >> 16)) {
                actor_apply_damage(
                    (u16)collision, power, 0, 0, 0,
                    magic->damage_components[0], magic->damage_components[1],
                    KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
            }
        }
        effect->position.vx += (s16)effect->direction.words.x;
        effect->position.vy += (s16)effect->direction.words.y;
        effect->position.vz += (s16)effect->direction.words.z;
        effect->position.vy =
            -(map_floor_height_grid[effect->position.vz / KF_MAP_TILE_SIZE]
                                   [effect->position.vx / KF_MAP_TILE_SIZE] * KF_MAP_HEIGHT_STEP);
        switch (phase) {
        case KF_EFFECT_GROUND_TRAIL_WAIT_FOR_PARENT:
            if (linked_effect->phase > 19) {
                effect->phase = KF_EFFECT_GROUND_TRAIL_SHRINK;
            }
            break;
        case KF_EFFECT_GROUND_TRAIL_SHRINK: {
            s32 scale = effect->scale_x - 400;

            effect->scale_x = scale;
            effect->scale_z = scale;
            effect->scale_y = scale;
            if ((s16)scale <= 0) {
                effect->type = KF_EFFECT_SLOT_FREE;
            }
            break;
        }
        }
        effect->render_id++;
        if (effect->render_id >= effect->base_render_id + 3) {
            effect->render_id = effect->base_render_id;
        }
        break;

    case KF_EFFECT_KIND_RADIAL_BLAST:
        if (++effect->phase < 13) {
            next = effect->scale_x + 0x400;
            effect->scale_x = next;
            effect->scale_z = next;
            effect->scale_y = next;
            radius = phase * 0x14d;
            power = effect_magic_power(effect);
            if (effect->phase & 1) {
                actor_pool_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    radius, KF_FIXED12_ONE, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                player_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    radius, KF_FIXED12_ONE, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->id);
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

        if (phase == 0) {
randomize_kind20:
            effect->direction.words.x =
                (effect->direction.words.x + (rand() >> 6) - 0x80) & KF_ANGLE_WRAP_MASK;
            effect->direction.words.y =
                (effect->direction.words.y + (rand() >> 3) - 0x200) & KF_ANGLE_WRAP_MASK;
        } else if (phase > 4) {
            if ((u8)effect->control.target_mode == KF_EFFECT_HOMING_WANDER) {
                if (rand() < 0xccc) {
                    goto randomize_kind20;
                }
            } else if ((u8)effect->control.target_mode == KF_EFFECT_HOMING_PLAYER) {
                effect->direction.words.y = vector_xz_to_angle(
                    player_state.camera_position.vx - effect->position.vx,
                    effect->position.vz - player_state.camera_position.vz);
                desired_pitch = vector_xz_to_angle(
                    effect->position.vy - 800 - player_state.camera_position.vy,
                    /* Retail reads the shared distance slot before this branch
                     * has initialized it; preserve that original behavior. */
                    -target_distance);
                effect->direction.words.x = -desired_pitch & KF_ANGLE_WRAP_MASK;
            } else {
                target = actor_pool_find_target_in_cone(
                    (const struct KfVec3i *)&effect->position,
                    (s16)effect->rotation.vy, 20000, 0x555, &target_distance);
                if (target != 0) {
                    KfActorDefinition *definition =
                        &actor_state.definitions[target->definition_id];

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
            effect->phase = 20;
        }

        effect->rotation.vx = angle_approach(
            effect->rotation.vx, (s16)effect->direction.words.x, KF_ANGLE_FULL_TURN / 64);
        effect->rotation.vy = angle_approach(
            effect->rotation.vy, (s16)effect->direction.words.y, KF_ANGLE_FULL_TURN / 64);
        local_motion.vx = 0;
        local_motion.vy = 0;
        local_motion.vz = 650;
        matrix_set_rotation_x(effect->rotation.vx, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        local_motion.vx = movement.vx;
        local_motion.vy = movement.vy;
        local_motion.vz = movement.vz;
        matrix_set_rotation_y(effect->rotation.vy, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        effect->position.vx += movement.vx;
        effect->position.vy += movement.vy;
        effect->position.vz += movement.vz;
        effect->phase++;
        effect->rotation.vz = (effect->rotation.vz + 0x100) & KF_ANGLE_WRAP_MASK;
        if (effect_map_collision(&effect->position, radius) != (u32)KF_COLLISION_NONE) {
            if (effect->base_render_id == 0x10) {
                effect_pool_construct(
                    effect->id, effect->type, 0x2c,
                    &effect->position, &effect->direction.vector, 1);
            } else {
                effect_pool_construct(
                    effect->id, effect->type, KF_EFFECT_KIND_RADIAL_BLAST,
                    &effect->position, &effect->direction.vector, 1);
            }
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        break;
    }

    case 32:
        if (phase > 9) {
            goto invalidate_and_advance;
        }
        effect->render_id++;
        if (effect->render_id >= effect->base_render_id + 3) {
            effect->render_id = effect->base_render_id;
        }
        if (phase == 3 || phase == 5 || phase == 7) {
            if (effect->base_render_id == 11) {
                effect_pool_construct(
                    effect->id, effect->type, 0x21,
                    &effect->position, &effect->rotation);
            } else {
                effect_pool_construct(
                    effect->id, effect->type, 0x2a,
                    &effect->position, &effect->rotation);
            }
            if (phase == 3) {
                phase_sound = &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].sounds[1];
                goto play_phase_sound;
            }
        }
        goto advance_effect_phase;

    case 33: {
        struct KfVec3i position;

        if (phase > 7) {
            goto invalidate_and_advance;
        }
        next = effect->scale_x + 0xdff;
        effect->scale_x = next;
        effect->scale_z = next;
        effect->scale_y = next;
        effect->rotation.vy = (effect->rotation.vy + 0x514) & KF_ANGLE_WRAP_MASK;
        if (phase & 1) {
            u32 damage_radius;

            position.x = effect->position.vx;
            position.y = KF_COLLISION_IGNORE_HEIGHT;
            position.z = effect->position.vz;
            damage_radius = phase * 1000;
            power = effect_magic_power(effect);
            actor_pool_apply_radial_damage(
                &position, damage_radius, KF_FIXED12_ONE, power, 0, 0, 0,
                magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].damage_components[0],
                magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
            player_apply_radial_damage(
                &position, damage_radius, KF_FIXED12_ONE, power, 0, 0, 0,
                magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].damage_components[0],
                magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT)].damage_components[1], 5000, effect->id);
        }
        goto advance_effect_phase;
    }

invalidate_and_advance:
        effect->type = KF_EFFECT_SLOT_FREE;
advance_effect_phase:
        effect->phase++;
        break;

    case KF_EFFECT_KIND_GROUND_BRANCH:
        if (phase < 16) {
            if (effect->control.frames_remaining != KF_EFFECT_GROUND_BRANCH_TIMER_DONE) {
                if (effect->control.frames_remaining-- == 0) {
                    switch (effect->propagation.branch) {
                    case KF_EFFECT_GROUND_BRANCH_ROOT:
                        effect_spawn_ground_kind6(
                            effect->id, effect, KF_ANGLE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_QUARTER_TURN);
                        effect_spawn_ground_kind6(
                            effect->id, effect, KF_ANGLE_THREE_QUARTER_TURN,
                            KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN);
                        break;
                    case KF_EFFECT_GROUND_BRANCH_QUARTER_TURN:
                        effect_spawn_ground_kind6(
                            effect->id, effect, KF_ANGLE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_LEAF);
                        break;
                    case KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN:
                        effect_spawn_ground_kind6(
                            effect->id, effect, KF_ANGLE_THREE_QUARTER_TURN, KF_EFFECT_GROUND_BRANCH_LEAF);
                        break;
                    }
                    effect->control.frames_remaining = KF_EFFECT_GROUND_BRANCH_TIMER_DONE;
                }
            }
            effect->scale_y += 0x100;
            if (phase == 15) {
                switch (effect->propagation.branch) {
                case KF_EFFECT_GROUND_BRANCH_ROOT:
                    effect->phase = 0x10;
                    break;
                case KF_EFFECT_GROUND_BRANCH_QUARTER_TURN:
                case KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN:
                    effect->phase = 0x1a;
                    break;
                case KF_EFFECT_GROUND_BRANCH_LEAF:
                    effect->phase = 0x24;
                    break;
                }
            }
        } else if (phase < 48) {
            if ((phase & 3) == 0) {
                VECTOR spawn_position;

                angle = (u32)rand() >> 3;
                distance = ((u32)rand() * 0x145) >> 13;
                spawn_position.vx = effect->position.vx
                    + ((rsin(angle) * distance) >> KF_FIXED12_BITS);
                spawn_position.vz = effect->position.vz
                    + ((rcos(angle) * distance) >> KF_FIXED12_BITS);
                spawn_position.vy = effect->position.vy;
                effect_pool_construct(
                    effect->id, effect->type, 0x22,
                    &spawn_position, &effect->rotation);
                power = effect_magic_power(effect);
                actor_pool_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    1500, KF_FIXED12_ONE, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], KF_ACTOR_DAMAGE_SCALE_ONE, effect->type);
                player_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    1500, KF_FIXED12_ONE, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->id);
            }
        } else if (phase < 64) {
            effect->scale_y -= 0x100;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        effect->rotation.vy = (effect->rotation.vy + 500) & KF_ANGLE_WRAP_MASK;
        effect->phase++;
        break;

    case 34:
        if (phase < 4) {
            effect->scale_y += 4000;
        } else if (phase < 8) {
            effect->scale_y -= 4000;
            effect->position.vy -= 1800;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
        effect->rotation.vy = (effect->rotation.vy + 500) & KF_ANGLE_WRAP_MASK;
        effect->phase++;
        break;

    case KF_EFFECT_KIND_ACTOR_SPAWNER: {
        s32 scale;
        u8 scale_phase;

        if (phase < 17) {
            scale = effect->scale_x + 220;
            scale_phase = effect->phase;
            goto publish_kind9_scale;
        } else if (phase < 41) {
            struct KfVec3i position;

            position.x = effect->position.vx + (s16)effect->direction.words.x;
            position.z = effect->position.vz + (s16)effect->direction.words.z;
            position.y = effect->position.vy;
            value = collision_query_world(
                position.x, position.y, position.z, 1000, 0,
                KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
            if ((phase == 40 && value != KF_COLLISION_NONE) || effect->control.frames_remaining == 0) {
                effect->phase = 0x75;
            } else {
                effect->position.vx = position.x;
                effect->position.vz = position.z;
                effect->control.frames_remaining--;
            }
            if (phase == 40) {
                goto rotate_kind9;
            }
            goto advance_kind9_phase;
        } else if (phase < 149) {
            if (phase == 132) {
                struct KfVec3s actor_rotation;
                struct KfVec3i position;

                position.x = effect->position.vx;
                position.y = effect->position.vy + 800;
                position.z = effect->position.vz;
                actor_rotation.x = 0;
                actor_rotation.z = 0;
                actor_rotation.y = vector_xz_to_angle(
                    player_state.camera_position.vx - position.x,
                    player_state.camera_position.vz - position.z);
                value = rand();
                if (value < 3000) {
                    actor_pool_spawn(2, &position, &actor_rotation);
                } else if (rand() < 3000) {
                    actor_pool_spawn(4, &position, &actor_rotation);
                } else {
                    actor_pool_spawn(0, &position, &actor_rotation);
                }
            }
advance_kind9_phase:
            effect->phase++;
            goto rotate_kind9;
        } else if (phase < 165) {
            scale = effect->scale_x - 220;
            scale_phase = effect->phase;
publish_kind9_scale:
            effect->scale_x = scale;
            effect->scale_z = scale;
            effect->scale_y = scale;
            effect->phase = scale_phase + 1;
        } else {
            effect->type = KF_EFFECT_SLOT_FREE;
        }
rotate_kind9:
        effect->rotation.vy = (effect->rotation.vy + 0x6a4) & KF_ANGLE_WRAP_MASK;
        effect->rotation.vx = (effect->rotation.vx + 0x578) & KF_ANGLE_WRAP_MASK;
        effect->rotation.vz = (effect->rotation.vz + 0x76c) & KF_ANGLE_WRAP_MASK;
        break;
    }

    case KF_EFFECT_KIND_SWINGING_HAZARD_SHORT:
        effect_projectile_update_3d(&effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_SHORT], 0x28);
        break;

    case KF_EFFECT_KIND_SWINGING_HAZARD_LONG:
        effect_projectile_update_3d(&effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_LONG], 0x3c);
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
            effect->direction.words.z += effect->rotation.vz;
            for (column = 0; column < effect->rotation.vy; column++) {
                effect_floor_deform_line(
                    effect->rotation.vx + column,
                    (s16)effect->direction.words.z,
                    -(s16)effect->direction.words.y);
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
            effect->direction.words.z -= effect->rotation.vz;
            for (column = 0; column < effect->rotation.vy; column++) {
                effect_floor_deform_line(
                    effect->rotation.vx + column,
                    (s16)effect->direction.words.z,
                    (s16)effect->direction.words.y);
            }
            break;
        }
        break;
    }

    case KF_EFFECT_KIND_ORBITING_PROJECTILE:
        effect_projectile_update_2d(0x1964, 0x28);
        break;

    default:
        break;
    }
}
