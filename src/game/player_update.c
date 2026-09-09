#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_player.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Flash immediately before/on each one-HP poison tick. */
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

enum {
    DARKNESS_FADE_BITS = 5,
    DARKNESS_FADE_STEPS = 1 << DARKNESS_FADE_BITS,
    PLAYER_NORMAL_MOVEMENT_LIMIT = 180,
    PLAYER_SLOWED_MOVEMENT_LIMIT = PLAYER_NORMAL_MOVEMENT_LIMIT / 5,
    PLAYER_NORMAL_TURN_LIMIT = 28,
    PLAYER_SLOWED_TURN_LIMIT = 5,
    PLAYER_DARKNESS_FOG_NEAR = 5000,
    PLAYER_NORMAL_FOG_NEAR = 11000
};

/* Fractions of the active limit; signed shifts preserve retail truncation. */
enum {
    PLAYER_FORWARD_ACCEL_SHIFT = 2,
    PLAYER_FORWARD_DECEL_SHIFT = 3,
    PLAYER_STRAFE_ACCEL_DECEL_SHIFT = 2,
    PLAYER_YAW_ACCEL_DECEL_SHIFT = 2,
    PLAYER_PITCH_ACCEL = 3,
    PLAYER_PITCH_DECEL = 2,
    PLAYER_PITCH_STEP_LIMIT = 10,
    PLAYER_CAMERA_PITCH_LIMIT = 191
};

/* Delay counts down to one; zero disables the pending sequence. */
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
    PLAYER_WEAPON_MAGIC_TARGET_RANGE = 20000,
    PLAYER_WEAPON_MAGIC_BURST_CONE = 0x155,
    PLAYER_WEAPON_MAGIC_TARGET_CONE = 0x555,
    PLAYER_WEAPON_MAGIC_JITTER_BIAS = 4,
    PLAYER_WEAPON_MAGIC_RANDOM_SHIFT = 9,
    PLAYER_WEAPON_MAGIC_SPEED = 900,
    PLAYER_TRIPLE_FANG_Y_OFFSET = 300,
    PLAYER_TRIPLE_FANG_PITCH_OFFSET = 64
};

DATA(0x80055858, 0x20)
static MATRIX player_darkness_color_matrix = {
    {{666, 233, 1333}, {666, 233, 1333}, {666, 233, 1333}},
    {0, 0, 0}
};

DATA(0x80055878, 0x40)
static SVECTOR player_damage_camera_offsets[KF_ENUM_ENCODE(u8, KF_PLAYER_DAMAGE_FRAME_END)] = {
    {0, 0, 0, 0},
    {-32, 0, -32, 0},
    {-64, 0, -64, 0},
    {-48, 0, -32, 0},
    {-32, 0, 0, 0},
    {-16, 0, 32, 0},
    {0, 0, 64, 0},
    {-16, 0, 32, 0}
};

DATA(0x80057b30, 0x4)
static u32 player_previous_input = 0;

DATA(0x80057e68, 0x4)
static s32 player_movement_velocity_limit;

DATA(0x80057e70, 0x4)
static s32 player_turn_step_limit;

ADDRESS(0x80018880, 0x1a1c)
void player_update(void)
{
    u32 input;
    s32 item;
    s16 forward;
    s16 strafe;
    s32 strafe_sq;
    s32 forward_sq;
    s16 magnitude;
    s16 fade;
    u16 phase;
    s32 cost;
    KF_ENUM_PARAM(KfEffectKind, s32) effect;
    s32 attachment;
    KfMagicRecord *record;
    KfActor *target;
    const VECTOR *origin;
    SVECTOR direction;
    SVECTOR spawn_offset;
    KfRotation effect_rotation;
    VECTOR position;
    MATRIX matrix;
    s32 distance;
    u8 attribute;
    KfMagicId magic_id;

    if (player_state.update_state == KF_PLAYER_UPDATE_DYING) {
        player_death_update();
        return;
    }
    if (player_state.update_state == KF_PLAYER_UPDATE_RECOVERY_FADE) {
        player_death_update_reverse_fade();
        return;
    }
    collision_adjust_cell_occupancy(player_state.motion_state.fields.map_cell.coords.x, player_state.motion_state.fields.map_cell.coords.z, -1);
    input = PadRead(1);
    if (input & PADh) {
        display_show_error_screen(KF_SYSTEM_SCREEN_PAUSE);
    }
    if (input & PADk) {
        input = PADRdown;
    }
    if ((input & PADRdown) && !(player_previous_input & PADRdown)
        && player_state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE) {
        item = menu_enter_mode(KF_MENU_MODE_ROOT);
        if (item >= 0) {
            player_use_item(KF_ENUM_DECODE(KF_ENUM_PROMOTED(KfItemId), item));
        } else if (item == KF_MENU_ROOT_GAME_LOADED) {
            pool_release_all();
            audio_close_vab();
            func_800365f8();
            player_sync_position_to_map();
            player_state.previous_map_cell.coords.x = player_state.motion_state.fields.map_cell.coords.x;
            player_state.previous_map_cell.coords.z = player_state.motion_state.fields.map_cell.coords.z;
            player_equip_weapon(player_state.equipped_weapon_id);
            player_select_magic(player_state.selected_magic_id);
        } else if (item == KF_MENU_ROOT_RETURN_TO_INTRO) {
            game_exit_code = KF_GAME_EXIT_INTRO;
            return;
        }
        player_previous_input = input;
    } else {
        if ((input & PADRright) && !(player_previous_input & PADRright)) {
            map_interaction_dispatch(&player_state.camera_position, &player_state.camera_rotation);
        }
        if (player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) {
            player_movement_velocity_limit = PLAYER_SLOWED_MOVEMENT_LIMIT;
            player_turn_step_limit = PLAYER_SLOWED_TURN_LIMIT;
        } else {
            player_movement_velocity_limit = PLAYER_NORMAL_MOVEMENT_LIMIT;
            player_turn_step_limit = PLAYER_NORMAL_TURN_LIMIT;
        }
        if (input & PADLup) {
            forward = player_state.motion_state.fields.forward_velocity
                + (player_movement_velocity_limit >> PLAYER_FORWARD_ACCEL_SHIFT);
            if (forward > player_movement_velocity_limit) {
                player_state.motion_state.fields.forward_velocity = player_movement_velocity_limit;
            } else {
                player_state.motion_state.fields.forward_velocity = forward;
            }
        } else if (input & PADLdown) {
            forward = player_state.motion_state.fields.forward_velocity
                - (player_movement_velocity_limit >> PLAYER_FORWARD_ACCEL_SHIFT);
            if (forward >= -player_movement_velocity_limit) {
                player_state.motion_state.fields.forward_velocity = forward;
            } else {
                player_state.motion_state.fields.forward_velocity = -player_movement_velocity_limit;
            }
        } else if (player_state.motion_state.fields.forward_velocity > 0) {
            player_state.motion_state.fields.forward_velocity -=
                player_movement_velocity_limit >> PLAYER_FORWARD_DECEL_SHIFT;
            if (player_state.motion_state.fields.forward_velocity < 0) {
                player_state.motion_state.fields.forward_velocity = 0;
            }
        } else if (player_state.motion_state.fields.forward_velocity < 0) {
            player_state.motion_state.fields.forward_velocity +=
                player_movement_velocity_limit >> PLAYER_FORWARD_DECEL_SHIFT;
            if (player_state.motion_state.fields.forward_velocity > 0) {
                player_state.motion_state.fields.forward_velocity = 0;
            }
        }
        if (input & PADl) {
            strafe = player_state.motion_state.fields.strafe_velocity
                + (player_movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT);
            if (strafe > player_movement_velocity_limit) {
                player_state.motion_state.fields.strafe_velocity = player_movement_velocity_limit;
            } else {
                player_state.motion_state.fields.strafe_velocity = strafe;
            }
        } else if (input & PADn) {
            strafe = player_state.motion_state.fields.strafe_velocity
                - (player_movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT);
            if (strafe >= -player_movement_velocity_limit) {
                player_state.motion_state.fields.strafe_velocity = strafe;
            } else {
                player_state.motion_state.fields.strafe_velocity = -player_movement_velocity_limit;
            }
        } else if (player_state.motion_state.fields.strafe_velocity > 0) {
            player_state.motion_state.fields.strafe_velocity -=
                player_movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.strafe_velocity < 0) {
                player_state.motion_state.fields.strafe_velocity = 0;
            }
        } else if (player_state.motion_state.fields.strafe_velocity < 0) {
            player_state.motion_state.fields.strafe_velocity +=
                player_movement_velocity_limit >> PLAYER_STRAFE_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.strafe_velocity > 0) {
                player_state.motion_state.fields.strafe_velocity = 0;
            }
        }
        strafe_sq = player_state.motion_state.fields.strafe_velocity;
        strafe_sq *= strafe_sq;
        forward_sq = player_state.motion_state.fields.forward_velocity;
        forward_sq *= forward_sq;
        magnitude = SquareRoot0(strafe_sq + forward_sq);
        if (magnitude == 0) {
            forward = 0;
            strafe = 0;
        } else {
            strafe = strafe_sq / magnitude;
            if (player_state.motion_state.fields.strafe_velocity < 0) {
                strafe = -(strafe_sq / magnitude);
            }
            forward = forward_sq / magnitude;
            if (player_state.motion_state.fields.forward_velocity < 0) {
                forward = -(forward_sq / magnitude);
            }
        }
        player_state.motion_state.fields.movement_speed = SquareRoot0(strafe * strafe + forward * forward);
        if (forward > 0) {
            player_move_horizontal(player_state.camera_rotation.vy, forward);
        } else if (forward < 0) {
            player_move_horizontal(
                (player_state.camera_rotation.vy + KF_ANGLE_HALF_TURN) & KF_ANGLE_WRAP_MASK, -forward);
        }
        if (strafe > 0) {
            player_move_horizontal(
                (player_state.camera_rotation.vy - KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK, strafe);
        } else if (strafe < 0) {
            player_move_horizontal(
                (player_state.camera_rotation.vy + KF_ANGLE_QUARTER_TURN) & KF_ANGLE_WRAP_MASK, -strafe);
        }
        player_update_view_bob();
        if (input & PADLleft) {
            player_state.motion_state.fields.yaw_step += player_turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.yaw_step > player_turn_step_limit) {
                player_state.motion_state.fields.yaw_step = player_turn_step_limit;
            }
        } else if (input & PADLright) {
            player_state.motion_state.fields.yaw_step -= player_turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.yaw_step < -player_turn_step_limit) {
                player_state.motion_state.fields.yaw_step = -player_turn_step_limit;
            }
        } else if (player_state.motion_state.fields.yaw_step > 0) {
            player_state.motion_state.fields.yaw_step -= player_turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.yaw_step < 0) {
                player_state.motion_state.fields.yaw_step = 0;
            }
        } else if (player_state.motion_state.fields.yaw_step < 0) {
            player_state.motion_state.fields.yaw_step += player_turn_step_limit >> PLAYER_YAW_ACCEL_DECEL_SHIFT;
            if (player_state.motion_state.fields.yaw_step > 0) {
                player_state.motion_state.fields.yaw_step = 0;
            }
        }
        player_state.camera_rotation.vy =
            (player_state.camera_rotation.vy + player_state.motion_state.fields.yaw_step) & KF_ANGLE_WRAP_MASK;
        if (input & PADm) {
            player_state.motion_state.fields.pitch_step += PLAYER_PITCH_ACCEL;
            if (player_state.motion_state.fields.pitch_step >= PLAYER_PITCH_STEP_LIMIT + 1) {
                player_state.motion_state.fields.pitch_step = PLAYER_PITCH_STEP_LIMIT;
            }
        } else if (input & PADo) {
            player_state.motion_state.fields.pitch_step -= PLAYER_PITCH_ACCEL;
            if (player_state.motion_state.fields.pitch_step < -PLAYER_PITCH_STEP_LIMIT) {
                player_state.motion_state.fields.pitch_step = -PLAYER_PITCH_STEP_LIMIT;
            }
        } else if (player_state.motion_state.fields.pitch_step > 0) {
            player_state.motion_state.fields.pitch_step -= PLAYER_PITCH_DECEL;
            if (player_state.motion_state.fields.pitch_step < 0) {
                player_state.motion_state.fields.pitch_step = 0;
            }
        } else if (player_state.motion_state.fields.pitch_step < 0) {
            player_state.motion_state.fields.pitch_step += PLAYER_PITCH_DECEL;
            if (player_state.motion_state.fields.pitch_step > 0) {
                player_state.motion_state.fields.pitch_step = 0;
            }
        }
        if (player_state.motion_state.fields.pitch_step > 0) {
            player_state.camera_rotation.vx += player_state.motion_state.fields.pitch_step;
            if (player_state.camera_rotation.vx >= PLAYER_CAMERA_PITCH_LIMIT + 1) {
                player_state.camera_rotation.vx = PLAYER_CAMERA_PITCH_LIMIT;
            }
        } else if (player_state.motion_state.fields.pitch_step < 0) {
            player_state.camera_rotation.vx += player_state.motion_state.fields.pitch_step;
            if (player_state.camera_rotation.vx < -PLAYER_CAMERA_PITCH_LIMIT) {
                player_state.camera_rotation.vx = -PLAYER_CAMERA_PITCH_LIMIT;
            }
        }
        if ((input & PADRup) && !(player_previous_input & PADRup)) {
            player_begin_weapon_attack();
        }
        if (player_state.equipped_body_armor_id != KF_ITEM_SKULL_ARMOR) {
            if ((input & PADRleft) && !(player_previous_input & PADRleft)) {
                if (player_state.weapon_attack_fully_charged == KF_WEAPON_ATTACK_FULL_CHARGE) {
                    player_state.weapon_attack_fully_charged = KF_WEAPON_ATTACK_NORMAL_CHARGE;
                    switch (player_state.equipped_weapon_id) {
                    case KF_ITEM_FLAME_SWORD:
                        if (player_state.weapon_attack_phase >= PLAYER_WEAPON_MAGIC_PHASE_FIRST
                            && player_state.weapon_attack_phase <= PLAYER_WEAPON_MAGIC_PHASE_LAST) {
                            player_state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                            player_state.weapon_magic_shots_remaining =
                                (PLAYER_WEAPON_MAGIC_PHASE_LAST - player_state.weapon_attack_phase)
                                / KF_WEAPON_ATTACK_PHASE_STEP + 1;
                            goto magic_done;
                        }
                        break;
                    case KF_ITEM_TRIPLE_FANG:
                    case KF_ITEM_MOONLIGHT_SWORD:
                        if (player_state.weapon_attack_phase >= PLAYER_WEAPON_MAGIC_PHASE_FIRST
                            && player_state.weapon_attack_phase <= PLAYER_WEAPON_MAGIC_PHASE_LAST) {
                            player_state.weapon_magic_shots_remaining = 1;
                            player_state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                            goto magic_done;
                        }
                        break;
                    case KF_ITEM_COLICHEMARDE:
                        if (player_state.weapon_attack_phase >= PLAYER_COLICHEMARDE_MAGIC_PHASE_FIRST
                            && player_state.weapon_attack_phase <= PLAYER_COLICHEMARDE_MAGIC_PHASE_LAST) {
                            player_state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                            player_state.weapon_magic_shots_remaining =
                                ((PLAYER_WEAPON_MAGIC_PHASE_LAST - player_state.weapon_attack_phase)
                                 / KF_WEAPON_ATTACK_PHASE_STEP + 1)
                                * PLAYER_COLICHEMARDE_SHOTS_PER_PHASE;
                            goto magic_done;
                        }
                        break;
                    }
                }
                if (player_state.selected_magic_id != KF_MAGIC_NONE
                    && player_state.magic_charge == KF_PLAYER_CHARGE_FULL) {
                    player_state.weapon_magic_delay = 0;
                    player_state.weapon_magic_shots_remaining = 0;
                    if (player_state.equipped_accessory_id == KF_ITEM_WIND_BLADE_BRACELET && player_state.selected_magic_id == KF_MAGIC_WIND_CUTTER) {
                        cost = player_state.selected_magic_record->mp_cost >> 1;
                    } else {
                        cost = player_state.selected_magic_record->mp_cost;
                    }
                    if (player_state.vitals.current_mp >= cost) {
                        player_state.vitals.current_mp -= cost;
                        magic_cast();
                        player_state.magic_charge = 0;
                    }
                }
                player_state.weapon_attack_fully_charged = KF_WEAPON_ATTACK_NORMAL_CHARGE;
            } else {
                magic_id = player_state.selected_magic_id;
                if (magic_id != KF_MAGIC_NONE) {
                    player_state.magic_charge +=
                        fixed6_ratio_step(player_state.magic, player_state.selected_magic_record->charge_rate)
                        * KF_PLAYER_CHARGE_GAIN_MULTIPLIER;
                    if (player_state.magic_charge > KF_PLAYER_CHARGE_FULL) {
                        player_state.magic_charge = KF_PLAYER_CHARGE_FULL;
                    }
                }
            }
        }
    magic_done:
        if (player_state.weapon_magic_delay == PLAYER_WEAPON_MAGIC_READY) {
            if (player_state.weapon_magic_shots_remaining != 0) {
                switch (player_state.equipped_weapon_id) {
                case KF_ITEM_TRIPLE_FANG:
                    if (player_state.physical_power < PLAYER_TRIPLE_FANG_MAGIC_MIN_POWER
                        || player_state.magic < PLAYER_TRIPLE_FANG_MAGIC_MIN_POWER) {
                        goto cancel;
                    }
                    effect = KF_EFFECT_KIND_HOMING_PROJECTILE;
                    record = &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_HOMING_PROJECTILE)];
                    player_state.weapon_magic_delay = PLAYER_TRIPLE_FANG_MAGIC_DELAY;
                    break;
                case KF_ITEM_FLAME_SWORD:
                    if (magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_BALL)].learned == KF_MAGIC_UNLEARNED) {
                        goto cancel;
                    }
                    effect = KF_EFFECT_KIND_FIRE_BALL;
                    record = &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_BALL)];
                    player_state.weapon_magic_delay = PLAYER_FLAME_SWORD_MAGIC_DELAY;
                    break;
                case KF_ITEM_MOONLIGHT_SWORD:
                    if (player_state.physical_power < PLAYER_MOONLIGHT_SWORD_MAGIC_MIN_POWER
                        || player_state.magic < PLAYER_MOONLIGHT_SWORD_MAGIC_MIN_POWER) {
                        goto cancel;
                    }
                    effect = KF_EFFECT_KIND_MOONLIGHT_PROJECTILE;
                    record = &magic_records[KF_ENUM_ENCODE(u8, KF_EFFECT_KIND_RADIAL_BLAST)];
                    player_state.weapon_magic_delay = PLAYER_MOONLIGHT_SWORD_MAGIC_DELAY;
                    break;
                case KF_ITEM_COLICHEMARDE:
                    if (player_state.physical_power < PLAYER_COLICHEMARDE_MAGIC_MIN_POWER
                        || player_state.magic < PLAYER_COLICHEMARDE_MAGIC_MIN_POWER) {
                        goto cancel;
                    }
                    effect = KF_EFFECT_KIND_LIGHT_NEEDLE;
                    record = &magic_records[KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHT_NEEDLE)];
                    player_state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY;
                    break;
                default:
                cancel:
                    player_state.weapon_magic_shots_remaining = 0;
                    player_state.weapon_magic_delay = PLAYER_WEAPON_MAGIC_READY + 1;
                    goto store_input;
                }
                if (player_state.vitals.current_mp >= record->mp_cost) {
                    if (player_state.weapon_magic_shots_remaining == 1) {
                        player_state.vitals.current_mp -= record->mp_cost;
                    }
                    spawn_offset.vx = PLAYER_WEAPON_MAGIC_SPAWN_X;
                    spawn_offset.vy = PLAYER_WEAPON_MAGIC_SPAWN_Y;
                    spawn_offset.vz = PLAYER_WEAPON_MAGIC_SPAWN_Z;
                    effect_rotation.angles.x = -player_state.camera_rotation.vx;
                    effect_rotation.angles.y = player_state.camera_rotation.vy;
                    effect_rotation.angles.z = -player_state.camera_rotation.vz;
                    matrix_set_rotation_yxz(&effect_rotation.angles, &matrix);
                    ApplyMatrix(&matrix, &spawn_offset, &position);
                    position.vx += player_state.camera_position.vx;
                    position.vy += player_state.camera_position.vy;
                    effect_rotation.angles.y = player_state.camera_rotation.vy;
                    position.vz += player_state.camera_position.vz;
                    effect_rotation.angles.x = player_state.camera_rotation.vx;
                    effect_rotation.angles.z = player_state.camera_rotation.vz;
                    origin = &player_state.camera_position;
                    if ((effect == KF_EFFECT_KIND_FIRE_BALL || effect == KF_EFFECT_KIND_LIGHT_NEEDLE)
                        && player_state.weapon_magic_shots_remaining != 1) {
                        actor_state.player_target = actor_pool_find_target_in_cone(
                            origin, player_state.camera_rotation.vy, PLAYER_WEAPON_MAGIC_TARGET_RANGE,
                            PLAYER_WEAPON_MAGIC_BURST_CONE, &distance);
                        effect_rotation.angles.x -= PLAYER_WEAPON_MAGIC_JITTER_BIAS
                            - (rand() >> PLAYER_WEAPON_MAGIC_RANDOM_SHIFT);
                        effect_rotation.angles.y -= PLAYER_WEAPON_MAGIC_JITTER_BIAS
                            - (rand() >> PLAYER_WEAPON_MAGIC_RANDOM_SHIFT);
                        attachment = player_state.weapon_magic_shots_remaining & 1;
                    } else {
                        target = actor_pool_find_target_in_cone(
                            &player_state.camera_position,
                            player_state.camera_rotation.vy, PLAYER_WEAPON_MAGIC_TARGET_RANGE,
                            PLAYER_WEAPON_MAGIC_TARGET_CONE, &distance);
                        actor_state.player_target = target;
                        if (target == 0) {
                            attachment = KF_ENUM_ENCODE(s32, KF_EFFECT_HOMING_WANDER);
                        } else {
                            attachment = target - actor_state.actors;
                        }
                    }
                    pitch_yaw_to_forward_vector(&effect_rotation.angles, &direction);
                    vector3s_scale_shift12(PLAYER_WEAPON_MAGIC_SPEED, &direction);
                    effect_pool_construct(
                        10, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS, effect,
                        &position, &direction, &player_state.camera_rotation, attachment, 1);
                    if (effect == KF_EFFECT_KIND_HOMING_PROJECTILE) {
                        position.vy += PLAYER_TRIPLE_FANG_Y_OFFSET;
                        effect_rotation.angles.y = player_state.camera_rotation.vy;
                        effect_rotation.angles.z = player_state.camera_rotation.vz;
                        effect_rotation.angles.x = player_state.camera_rotation.vx + PLAYER_TRIPLE_FANG_PITCH_OFFSET;
                        effect_pool_construct(
                            10, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                            KF_EFFECT_KIND_HOMING_PROJECTILE, &position, &direction, &effect_rotation.vector, attachment,
                            0);
                        effect_rotation.angles.x -= 2 * PLAYER_TRIPLE_FANG_PITCH_OFFSET;
                        position.vy -= 2 * PLAYER_TRIPLE_FANG_Y_OFFSET;
                        effect_pool_construct(
                            10, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                            KF_EFFECT_KIND_HOMING_PROJECTILE, &position, &direction, &effect_rotation.vector, attachment,
                            0);
                    }
                }
                player_state.weapon_magic_shots_remaining--;
            }
        } else if (player_state.weapon_magic_delay != 0) {
            player_state.weapon_magic_delay--;
        }
    store_input:
        player_previous_input = input;
        player_update_vertical_motion();
    }
    collision_adjust_cell_occupancy(player_state.motion_state.fields.map_cell.coords.x, player_state.motion_state.fields.map_cell.coords.z, 1);
    player_update_weapon_attack();
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    if (player_state.darkness_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!(player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS)
            && player_state.darkness_timer >= DARKNESS_FADE_STEPS + 1) {
            player_state.darkness_timer = DARKNESS_FADE_STEPS;
        }
        player_state.darkness_timer--;
        if (player_state.darkness_timer == KF_PLAYER_STATUS_TIMER_INACTIVE) {
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_DARKNESS;
        } else {
            fade = player_state.darkness_timer
                - (KF_DARKNESS_DURATION_UPDATES - DARKNESS_FADE_STEPS);
            if (fade < 0) {
                fade = DARKNESS_FADE_STEPS - player_state.darkness_timer;
            }
            if (fade >= 0) {
                lighting_set_color_matrix(&player_darkness_color_matrix, color_matrix_table,
                    fade << (KF_FIXED12_BITS - DARKNESS_FADE_BITS));
                fog_interpolate_near(PLAYER_DARKNESS_FOG_NEAR, PLAYER_NORMAL_FOG_NEAR,
                    fade << (KF_FIXED12_BITS - DARKNESS_FADE_BITS));
            } else {
                SetColorMatrix(&player_darkness_color_matrix);
                fog_set_near(PLAYER_DARKNESS_FOG_NEAR);
            }
        }
    } else {
        fog_set_near(PLAYER_NORMAL_FOG_NEAR);
    }
    if (player_state.update_state != KF_PLAYER_UPDATE_NORMAL
        && player_state.update_state != KF_PLAYER_UPDATE_DYING) {
        if (player_state.update_state >= KF_PLAYER_DAMAGE_FRAME_END) {
            player_state.update_state = KF_PLAYER_UPDATE_NORMAL;
            player_state.view_rotation_offset = player_damage_camera_offsets[KF_ENUM_ENCODE(u8, KF_PLAYER_UPDATE_NORMAL)];
            if (player_state.vitals.current_hp == 0) {
                player_death_begin();
            }
        } else {
            player_state.view_rotation_offset = player_damage_camera_offsets[KF_ENUM_ENCODE(u8, player_state.update_state)];
            lighting_set_active_color_matrix(KF_GAME_COLOR_DAMAGE);
            player_state.update_state++;
        }
    }
    if (player_state.equipped_weapon_id != KF_ITEM_NONE) {
        if (player_state.equipped_weapon_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_weapon_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_weapon_record->mp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_weapon_record->mp_regen_interval == 0) {
            player_adjust_mp(1);
        }
    }
    if (player_state.equipped_head_armor_id != KF_ITEM_NONE) {
        if (player_state.equipped_head_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_head_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_head_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_head_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_body_armor_id != KF_ITEM_NONE) {
        if (player_state.equipped_body_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_body_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_body_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_body_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_shield_id != KF_ITEM_NONE) {
        if (player_state.equipped_shield_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_shield_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_shield_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_shield_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_arm_armor_id != KF_ITEM_NONE) {
        if (player_state.equipped_arm_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_arm_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_arm_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_arm_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_leg_armor_id != KF_ITEM_NONE) {
        if (player_state.equipped_leg_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_leg_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_leg_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_leg_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    player_state.equipment_effect_ticks++;
    attribute = map_cell_attribute_grid.cells[player_state.motion_state.fields.map_cell.coords.z][player_state.motion_state.fields.map_cell.coords.x];
    switch (attribute) {
    case KF_MAP_ATTRIBUTE_PITFALL:
        if (player_state.update_state == KF_PLAYER_UPDATE_NORMAL) {
            player_apply_damage(5, 3, 5, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
        }
        break;
    case KF_MAP_ATTRIBUTE_POISON_HOLE:
        player_apply_damage(0, 0, 0, KF_PLAYER_STATUS_POISON, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);
        break;
    }
    if (player_state.slowed_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!(player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED)) {
            player_state.slowed_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_SLOWED;
        } else {
            player_state.slowed_timer--;
            if (player_state.slowed_timer == KF_PLAYER_STATUS_TIMER_INACTIVE) {
                player_state.status_effect_flags &= ~KF_PLAYER_STATUS_SLOWED;
            }
        }
    }
    if (player_state.poison_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!(player_state.status_effect_flags & KF_PLAYER_STATUS_POISON)) {
            player_state.poison_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_POISON;
        } else {
            player_state.poison_timer--;
            if (player_state.poison_timer == KF_PLAYER_STATUS_TIMER_INACTIVE) {
                player_state.status_effect_flags &= ~KF_PLAYER_STATUS_POISON;
            } else {
                phase = player_state.poison_timer % POISON_DAMAGE_INTERVAL_UPDATES;
                if (phase < POISON_FLASH_UPDATES) {
                    lighting_set_active_color_matrix(KF_GAME_COLOR_DAMAGE);
                    if (phase == 0) {
                        player_adjust_hp(-1);
                    }
                }
            }
        }
    }
    if (player_state.curse_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (!(player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE)) {
            player_state.curse_timer = 0;
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_CURSE;
            player_recalculate_combat_stats();
        } else if (player_state.curse_timer == 0) {
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_CURSE;
            player_recalculate_combat_stats();
        } else if (player_state.curse_timer == KF_CURSE_DURATION_UPDATES) {
            player_recalculate_combat_stats();
        }
        player_state.curse_timer--;
    }
    if (player_state.fire_defense_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
        if (player_state.fire_defense_timer == 0) {
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
            player_recalculate_combat_stats();
        } else if (player_state.fire_defense_timer == KF_FIRE_DEFENSE_DURATION_UPDATES) {
            player_recalculate_combat_stats();
        }
        lighting_set_active_color_matrix(KF_GAME_COLOR_DEFENSE_EFFECT);
        player_state.fire_defense_timer--;
    }
    if (player_state.equipped_weapon_id == KF_ITEM_SHADOW_BLADE) {
        lighting_apply_weapon9_environment();
    }
    if (player_state.illusion_staff_timer != KF_ILLUSION_STAFF_INACTIVE) {
        player_state.illusion_staff_timer--;
        lighting_apply_timed_player_effect();
    }
}
