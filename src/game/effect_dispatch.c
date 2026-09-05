#include <kf/address.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Dense effect-kind dispatch table for kinds 4 through 52. */
RODATA(0x80012cf8, 0xc4)

DATA(0x80057b88, 0x10)
SVECTOR effect_projectile_velocities[2] = {
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
    KfEffectRecord *linked_effect;
    KfActor *target;
    struct KfVec3i position;
    VECTOR movement;
    struct KfVec3s actor_rotation;
    SVECTOR local_motion;
    MATRIX matrix;
    u16 scatter[4];
    u32 collision;
    u32 phase;
    u32 kind;
    u32 radius;
    u32 angle;
    u32 distance;
    u16 power;
    u16 next;
    s16 prior;
    s16 desired_pitch;
    s32 target_distance;
    s32 cell_x;
    s32 cell_z;
    s32 count;
    s32 value;

    kind = effect->kind;
    phase = effect->unknown_07;

    switch (kind) {
    case 13:
        radius = 150;
        goto shared_projectile;
    case 4:
    case 5:
    case 7:
    case 8:
    case 10:
    case 11:
    case 12:
    case 14:
    case 22:
        radius = 100;
shared_projectile:
        if (phase == 0) {
            collision = effect_map_collision(&effect->position, radius);
            if (collision != (u32)-1) {
                if (kind == 4) {
                    goto effect_kind4_impact;
                }
                power = effect_magic_power(effect);
                if (kind != 10) {
                    audio_play_spatial_default_range(
                        &magic->sounds[1], &effect->position, 0x7f);
                }
                if ((collision >> 16) == 0x10) {
                    if (kind == 14 || kind == 22 || kind == 7) {
                        actor_apply_damage(
                            collision, power,
                            magic->damage_components[0],
                            magic->damage_components[2],
                            magic->damage_components[1],
                            0, 0, 5000, effect->type);
                    } else if (kind != 13) {
                        actor_apply_damage(
                            collision, power,
                            0, magic->damage_components[0], 0,
                            magic->damage_components[1], 0,
                            5000, effect->type);
                    }
                    if (kind == 7) {
                        goto advance_shared_projectile;
                    }
                } else if ((collision >> 16) == 0x80) {
                    if (kind == 13 || kind == 14 || kind == 22) {
                        player_apply_damage(
                            magic->damage_components[0],
                            magic->damage_components[2],
                            magic->damage_components[1],
                            0, 0, 0, 0x1000, effect->id);
                    } else if (kind == 11) {
                        player_apply_damage(0, 0, 0, 2, 0, 0, 0x1000, effect->id);
                    } else if (kind == 10) {
                        player_apply_damage(0, 0, 0, 4, 0, 0, 0x1000, effect->id);
                    } else if (kind == 12) {
                        player_apply_damage(0, 0, 0, 1, 0, 0, 0x1000, effect->id);
                    } else {
                        player_apply_damage(
                            0, 0, 0, 0,
                            magic->damage_components[0],
                            magic->damage_components[1],
                            0x1000, effect->id);
                    }
                }

                if (kind == 13) {
                    effect->direction_y = 0;
                    effect->unknown_07 = 0x78;
                } else if (kind == 11 || kind == 12) {
                    effect->unknown_07 = 0x79;
                } else {
                    effect->unknown_07 = 1;
                }
                return;
            }

advance_shared_projectile:
            effect->position.vx += (s16)effect->direction_x;
            effect->position.vy += (s16)effect->direction_y;
            effect->position.vz += (s16)effect->direction_z;
            if (kind == 5 || kind == 11) {
                effect->rotation_z = (effect->rotation_z + 200) & 0xfff;
                return;
            }
            if (kind == 4) {
                effect->rotation_z = (effect->rotation_z + 200) & 0xfff;
                effect->unknown_38--;
                if ((s16)effect->unknown_38 != 0) {
                    effect->unknown_03 =
                        effect->unknown_02 + ((u8)effect->unknown_38 & 1);
                    return;
                }
effect_kind4_impact:
                audio_play_spatial_default_range(
                    &magic->sounds[1], &effect->position, 0x7f);
                position.x = effect->position.vx;
                position.z = effect->position.vz;
                cell_x = position.x / 2000;
                cell_z = position.z / 2000;
                position.y = -(map_floor_height_grid[cell_z][cell_x] * 100);
                effect->unknown_07 = 0x32;
                effect_pool_construct(
                    effect->id, effect->type,
                    effect->unknown_02 == 6 ? 0x20 : 0x29,
                    &position, &effect->rotation_x);
                return;
            }
            if (kind == 10) {
                prior = effect->unknown_38;
                effect->unknown_38 = prior - 1;
                if ((s16)(prior - 1) == 0) {
                    if ((s16)effect->unknown_3a == 0) {
                        effect->type = 0xff;
                    } else {
                        next = (effect->scale_x * 3) >> 2;
                        effect->unknown_08 = next;
                        effect->scale_x = next;
                        effect->scale_y = next;
                        effect->scale_z = next;
                        effect->unknown_3a--;
                        scatter[0] = effect->direction_x;
                        scatter[1] = effect->direction_y;
                        scatter[2] = effect->direction_z;
                        scatter[3] = effect->unknown_32;
                        effect_scatter_triple(scatter);
                        effect->unknown_38 = effect->unknown_3a == 0 ? 15 : 5;
                        effect_pool_construct(
                            effect->id, effect->type, 10, &effect->position,
                            scatter, effect->unknown_3a, effect->unknown_38,
                            (s16)effect->scale_x);
                        effect_scatter_triple(&effect->direction_x);
                    }
                }
                value = rsin(effect->unknown_38 << 9);
                next = effect->unknown_08
                    + ((effect->unknown_08 * value) >> 13);
                effect->scale_x = next;
                effect->scale_z = next;
                value = rcos(effect->unknown_38 << 9);
                effect->scale_y = effect->unknown_08
                    + ((effect->unknown_08 * value) >> 13);
                return;
            }
            if (kind == 13) {
                effect->rotation_z = (effect->rotation_z + 100) & 0xfff;
                return;
            }
            if (kind == 14) {
                if (rand() < 8192 && effect->unknown_05 == 0) {
                    effect->unknown_05 = audio_play_spatial_range(
                        &magic->sounds[0], &effect->position, 0x7f,
                        4000, 12000);
                }
                return;
            }
            if (kind == 8 || kind == 22) {
                return;
            }
            effect->rotation_z = (effect->rotation_z + 600) & 0xfff;
            return;
        }

        if (phase < 5 && kind == 5) {
            effect->unknown_03 = effect->unknown_02 + phase;
        } else if (phase < 10) {
            effect->type = 0xff;
        } else if (phase < 60) {
            effect->scale_x -= 400;
            effect->scale_y = effect->scale_x;
            effect->scale_z = effect->scale_x;
        } else if (phase == 60) {
            effect->type = 0xff;
        } else if (phase < 120) {
            effect->position.vy -= 175;
            if (phase == 119) {
                effect->unknown_07 = 0xff;
                audio_play_spatial_default_range(
                    &magic->sounds[0], &effect->position, 0x7f);
            }
        } else if (phase == 120) {
            effect->direction_y += 20;
            effect->position.vy += (s16)effect->direction_y;
            cell_x = effect->position.vx / 2000;
            cell_z = effect->position.vz / 2000;
            if (effect->position.vy
                > -(map_floor_height_grid[cell_z][cell_x] * 100) + 3000) {
                effect->type = 0xff;
            }
        } else if (phase == 121) {
            effect->scale_y -= 0x180;
            effect->scale_z = effect->scale_y;
            if ((s16)effect->scale_y < 0) {
                effect->type = 0xff;
            }
        } else {
            effect->type = 0xff;
        }
        effect->unknown_07++;
        break;

    case 6:
        if (phase < 16) {
            if ((s16)effect->unknown_38 != 0xff) {
                prior = effect->unknown_38;
                effect->unknown_38 = prior - 1;
                if (prior == 0) {
                    if (effect->unknown_3a == 0) {
                        effect_spawn_ground_kind6(
                            effect->id, effect, 0x400, 1);
                        effect_spawn_ground_kind6(
                            effect->id, effect, 0xc00, 2);
                    } else if (effect->unknown_3a == 1) {
                        effect_spawn_ground_kind6(
                            effect->id, effect, 0x400, 0xff);
                    } else if (effect->unknown_3a == 2) {
                        effect_spawn_ground_kind6(
                            effect->id, effect, 0xc00, 0xff);
                    }
                    effect->unknown_38 = 0xff;
                }
            }
            effect->scale_y += 0x100;
            if (phase == 15) {
                if (effect->unknown_3a == 0) {
                    effect->unknown_07 = 0x10;
                } else if (effect->unknown_3a < 3) {
                    effect->unknown_07 = 0x1a;
                } else if (effect->unknown_3a == 0xff) {
                    effect->unknown_07 = 0x24;
                }
            }
        } else if (phase < 48) {
            if ((phase & 3) == 0) {
                angle = rand() >> 3;
                distance = (rand() * 0x145) >> 13;
                position.x = effect->position.vx
                    + ((rsin(angle) * distance) >> 12);
                position.y = effect->position.vy;
                position.z = effect->position.vz
                    + ((rcos(angle) * distance) >> 12);
                effect_pool_construct(
                    effect->id, effect->type, 0x22,
                    &position, &effect->rotation_x);
                power = effect_magic_power(effect);
                actor_pool_apply_radial_damage(
                    &position, 1500, 0x1000, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->type);
                player_apply_radial_damage(
                    &position, 1500, 0x1000, power, 0, 0, 0,
                    magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->id);
            }
        } else if (phase < 64) {
            effect->scale_y -= 0x100;
        } else {
            effect->type = 0xff;
        }
        effect->rotation_y = (effect->rotation_y + 500) & 0xfff;
        effect->unknown_07++;
        break;

    case 9:
        if (phase < 17) {
            next = effect->scale_x + 220;
            effect->scale_x = next;
            effect->scale_y = next;
            effect->scale_z = next;
            effect->unknown_07++;
        } else if (phase < 41) {
            position.x = effect->position.vx + (s16)effect->direction_x;
            position.y = effect->position.vy + (s16)effect->direction_y;
            position.z = effect->position.vz + (s16)effect->direction_z;
            value = collision_query_world(
                position.x, position.y, position.z, 1000, 0, 0x60);
            if ((phase == 40 && value != -1) || (s16)effect->unknown_38 == 0) {
                effect->unknown_07 = 0x75;
            } else {
                effect->position.vx = position.x;
                effect->position.vy = position.y;
                effect->position.vz = position.z;
                effect->unknown_38--;
                if (phase != 40) {
                    effect->unknown_07++;
                }
            }
        } else if (phase < 149) {
            if (phase == 132) {
                position.x = effect->position.vx;
                position.y = effect->position.vy + 800;
                position.z = effect->position.vz;
                actor_rotation.x = 0;
                actor_rotation.y = vector_xz_to_angle(
                    player_state.camera_position.vx - position.x,
                    player_state.camera_position.vz - position.z);
                actor_rotation.z = 0;
                value = rand();
                if (value < 3000) {
                    count = 2;
                } else if (rand() < 3000) {
                    count = 4;
                } else {
                    count = 0;
                }
                actor_pool_spawn(count, &position, &actor_rotation);
            }
            effect->unknown_07++;
        } else if (phase < 165) {
            next = effect->scale_x - 220;
            effect->scale_x = next;
            effect->scale_y = next;
            effect->scale_z = next;
            effect->unknown_07++;
        } else {
            effect->type = 0xff;
        }
        effect->rotation_x = (effect->rotation_x + 0x578) & 0xfff;
        effect->rotation_y = (effect->rotation_y + 0x6a4) & 0xfff;
        effect->rotation_z = (effect->rotation_z + 0x76c) & 0xfff;
        break;

    case 15:
        effect_projectile_update_3d(&effect_projectile_velocities[0], 0x28);
        break;

    case 16:
        effect_projectile_update_3d(&effect_projectile_velocities[1], 0x3c);
        break;

    case 17:
        effect_projectile_update_2d(0x1964, 0x28);
        break;

    case 18:
        effect->unknown_07++;
        if (effect->unknown_07 < 13) {
            next = effect->scale_x + 0x400;
            effect->scale_x = next;
            effect->scale_y = next;
            effect->scale_z = next;
            power = effect_magic_power(effect);
            if (effect->unknown_07 & 1) {
                actor_pool_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    effect->unknown_07 * 0x14d, 0x1000, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->type);
                player_apply_radial_damage(
                    (const struct KfVec3i *)&effect->position,
                    effect->unknown_07 * 0x14d, 0x1000, power,
                    0, 0, 0, magic->damage_components[0],
                    magic->damage_components[1], 5000, effect->id);
            }
        } else {
            effect->type = 0xff;
        }
        break;

    case 19:
        collision = effect_map_collision(&effect->position, 100);
        if (collision != (u32)-1 && (collision >> 16) == 0x10) {
            power = effect_magic_power(effect);
            actor_apply_damage(
                collision, power, 0, 0, 0,
                magic->damage_components[0], magic->damage_components[1],
                5000, effect->type);
        }
        linked_effect = &effect_pool_records[(u8)effect->unknown_38];
        effect->position.vx += (s16)effect->direction_x;
        effect->position.vy += (s16)effect->direction_y;
        effect->position.vz += (s16)effect->direction_z;
        cell_x = effect->position.vx / 2000;
        cell_z = effect->position.vz / 2000;
        effect->position.vy = -(map_floor_height_grid[cell_z][cell_x] * 100);
        if (phase == 0) {
            if (linked_effect->unknown_07 > 19) {
                effect->unknown_07 = 1;
            }
        } else if (phase == 1) {
            next = effect->scale_x - 400;
            effect->scale_x = next;
            effect->scale_y = next;
            effect->scale_z = next;
            if ((s16)next <= 0) {
                effect->type = 0xff;
            }
        }
        effect->unknown_03++;
        if (effect->unknown_03 >= effect->unknown_02 + 3) {
            effect->unknown_03 = effect->unknown_02;
        }
        break;

    case 20:
        if (phase == 0) {
randomize_kind20:
            effect->direction_x =
                (effect->direction_x + (rand() >> 6) - 0x80) & 0xfff;
            effect->direction_y =
                (effect->direction_y + (rand() >> 3) - 0x200) & 0xfff;
        } else if (phase > 4) {
            if ((s16)effect->unknown_38 == -1) {
                if (rand() < 0xccc) {
                    goto randomize_kind20;
                }
            } else if ((s16)effect->unknown_38 == -2) {
                effect->direction_y = vector_xz_to_angle(
                    player_state.camera_position.vx - effect->position.vx,
                    effect->position.vz - player_state.camera_position.vz);
                desired_pitch = vector_xz_to_angle(
                    effect->position.vy - 800 - player_state.camera_position.vy,
                    /* Retail reads the shared distance slot before this branch
                     * has initialized it; preserve that original behavior. */
                    -target_distance);
                effect->direction_x = -desired_pitch & 0xfff;
            } else {
                target = actor_pool_find_target_in_cone(
                    (const struct KfVec3i *)&effect->position,
                    (s16)effect->rotation_y, 20000, 0x555, &target_distance);
                if (target == 0) {
                    effect->direction_x = 0;
                } else {
                    effect->direction_y = vector_xz_to_angle(
                        target->position.vx - effect->position.vx,
                        effect->position.vz - target->position.vz);
                    desired_pitch = vector_xz_to_angle(
                        effect->position.vy
                            - (target->position.vy
                               - (actor_state.definitions[target->definition_id]
                                      .collision_radius >> 1)),
                        -target_distance);
                    effect->direction_x = -desired_pitch & 0xfff;
                }
            }
            effect->unknown_07 = 20;
        }

        effect->rotation_x = angle_approach(
            effect->rotation_x, effect->direction_x, 0x40);
        effect->rotation_y = angle_approach(
            effect->rotation_y, effect->direction_y, 0x40);
        local_motion.vx = 0;
        local_motion.vy = 0;
        local_motion.vz = 650;
        local_motion.pad = 0;
        matrix_set_rotation_x(effect->rotation_x, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        local_motion.vx = movement.vx;
        local_motion.vy = movement.vy;
        local_motion.vz = movement.vz;
        matrix_set_rotation_y(effect->rotation_y, &matrix);
        ApplyMatrix(&matrix, &local_motion, &movement);
        effect->position.vx += movement.vx;
        effect->position.vy += movement.vy;
        effect->position.vz += movement.vz;
        effect->unknown_07++;
        effect->rotation_z = (effect->rotation_z + 0x100) & 0xfff;
        if (effect_map_collision(&effect->position, 100) != (u32)-1) {
            effect_pool_construct(
                effect->id, effect->type,
                effect->unknown_02 == 0x10 ? 0x2c : 0x12,
                &effect->position, &effect->direction_x, 1);
            effect->type = 0xff;
        }
        break;

    case 32:
        if (phase > 9) {
            effect->type = 0xff;
            break;
        }
        effect->unknown_03++;
        if (effect->unknown_03 >= effect->unknown_02 + 3) {
            effect->unknown_03 = effect->unknown_02;
        }
        if (phase == 3 || phase == 5 || phase == 7) {
            effect_pool_construct(
                effect->id, effect->type,
                effect->unknown_02 == 11 ? 0x21 : 0x2a,
                &effect->position, &effect->rotation_x);
            if (phase == 3) {
                audio_play_spatial_default_range(
                    &magic_records[4].sounds[1], &effect->position, 0x7f);
            }
        }
        effect->unknown_07++;
        break;

    case 33:
        if (phase > 7) {
            effect->type = 0xff;
            break;
        }
        next = effect->scale_x + 0xdff;
        effect->scale_x = next;
        effect->scale_y = next;
        effect->scale_z = next;
        effect->rotation_y = (effect->rotation_y + 0x514) & 0xfff;
        if (phase & 1) {
            position.x = effect->position.vx;
            position.y = -1;
            position.z = effect->position.vz;
            power = effect_magic_power(effect);
            actor_pool_apply_radial_damage(
                &position, phase * 1000, 0x1000, power, 0, 0, 0,
                magic_records[4].damage_components[0],
                magic_records[4].damage_components[1], 5000, effect->type);
            player_apply_radial_damage(
                &position, phase * 1000, 0x1000, power, 0, 0, 0,
                magic_records[4].damage_components[0],
                magic_records[4].damage_components[1], 5000, effect->id);
        }
        effect->unknown_07++;
        break;

    case 34:
        if (phase < 4) {
            effect->scale_y += 4000;
        } else if (phase < 8) {
            effect->scale_y -= 4000;
            effect->position.vy -= 1800;
        } else {
            effect->type = 0xff;
        }
        effect->rotation_y = (effect->rotation_y + 500) & 0xfff;
        effect->unknown_07++;
        break;

    case 36:
        if (phase < 11) {
            if (effect_map_collision(&effect->position, 100) != (u32)-1) {
                effect->unknown_04 = 0xff;
                effect->unknown_02 = 0xff;
                effect->unknown_03 = 0xff;
                effect->unknown_07 = 20;
                audio_play_spatial_default_range(
                    &magic_records[18].sounds[1], &effect->position, 0x7f);
                return;
            }
            effect->position.vx += (s16)effect->direction_x;
            effect->position.vy += (s16)effect->direction_y;
            effect->position.vz += (s16)effect->direction_z;
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
                    effect->id, effect->type, 0x12,
                    &effect->position, &effect->direction_x, 1);
            }
            if (phase > 23) {
                effect->type = 0xff;
                break;
            }
        }
        effect->unknown_07++;
        break;

    case 52:
        if (phase == 0) {
            prior = effect->direction_x;
            effect->direction_x = prior - 1;
            if ((s16)(prior - 1) != -1) {
                effect->direction_z += effect->rotation_z;
                for (count = 0; count < (s16)effect->rotation_y; count++) {
                    effect_floor_deform_line(
                        (s16)effect->rotation_x + count,
                        (s16)effect->direction_z,
                        -(s16)effect->direction_y);
                }
                return;
            }
            effect->unknown_07 = 1;
        } else if (phase == 1) {
            prior = effect->position.vy;
            effect->position.vy = prior - 1;
            if (prior - 1 != -1) {
                return;
            }
            effect->unknown_07 = 2;
            effect->direction_z = 0x1000;
            effect->direction_x = effect->position.vx;
        } else if (phase == 2) {
            prior = effect->direction_x;
            effect->direction_x = prior - 1;
            if ((s16)(prior - 1) != -1) {
                effect->direction_z -= effect->rotation_z;
                for (count = 0; count < (s16)effect->rotation_y; count++) {
                    effect_floor_deform_line(
                        (s16)effect->rotation_x + count,
                        (s16)effect->direction_z,
                        (s16)effect->direction_y);
                }
                return;
            }
            effect->type = 0xff;
        }
        break;

    default:
        break;
    }
}
