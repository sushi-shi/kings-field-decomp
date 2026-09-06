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

DATA(0x80055858, 0x20)
static MATRIX player_status_effect1_color_matrix = {
    {{666, 233, 1333}, {666, 233, 1333}, {666, 233, 1333}},
    {0, 0, 0}
};

DATA(0x80055878, 0x40)
static SVECTOR player_damage_camera_offsets[8] = {
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
    s32 effect;
    s32 attachment;
    KfMagicRecord *record;
    KfActor *target;
    const struct KfVec3i *origin;
    struct KfVec3s direction;
    SVECTOR scale;
    struct KfEulerAngles angles;
    VECTOR position;
    MATRIX matrix;
    s32 distance;
    u8 attribute;
    u8 magic_id;

    if (player_state.update_state == KF_PLAYER_UPDATE_DYING) {
        player_death_update();
        return;
    }
    if (player_state.update_state == KF_PLAYER_UPDATE_RECOVERY_FADE) {
        player_death_update_reverse_fade();
        return;
    }
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, -1);
    input = PadRead(1);
    if (input & PADh) {
        display_show_error_screen(KF_SYSTEM_SCREEN_PAUSE);
    }
    if (input & PADk) {
        input = PADRdown;
    }
    if ((input & PADRdown) && !(player_previous_input & PADRdown) && player_state.weapon_attack_phase == -1) {
        item = menu_enter_mode(0);
        if (item >= 0) {
            player_use_item(item);
        } else if (item == -3) {
            pool_release_all();
            audio_close_vab();
            func_800365f8();
            player_sync_position_to_map();
            player_state.previous_map_cell.x = player_state.map_cell.x;
            player_state.previous_map_cell.z = player_state.map_cell.z;
            player_equip_weapon(player_state.equipped_weapon_id);
            player_select_magic(player_state.selected_magic_id);
        } else if (item == -2) {
            game_exit_code = KF_OPEN_MODE_INTRO;
            return;
        }
        player_previous_input = input;
    } else {
        if ((input & PADRright) && !(player_previous_input & PADRright)) {
            map_interaction_dispatch(&player_state.camera_position, &player_state.camera_rotation);
        }
        if (player_state.status_effect_flags & 0x8) {
            player_movement_velocity_limit = 36;
            player_turn_step_limit = 5;
        } else {
            player_movement_velocity_limit = 180;
            player_turn_step_limit = 28;
        }
        if (input & PADLup) {
            forward = player_state.motion_state.forward_velocity + (player_movement_velocity_limit >> 2);
            if (forward > player_movement_velocity_limit) {
                player_state.motion_state.forward_velocity = player_movement_velocity_limit;
            } else {
                player_state.motion_state.forward_velocity = forward;
            }
        } else if (input & PADLdown) {
            forward = player_state.motion_state.forward_velocity - (player_movement_velocity_limit >> 2);
            if (forward >= -player_movement_velocity_limit) {
                player_state.motion_state.forward_velocity = forward;
            } else {
                player_state.motion_state.forward_velocity = -player_movement_velocity_limit;
            }
        } else if (player_state.motion_state.forward_velocity > 0) {
            player_state.motion_state.forward_velocity -= player_movement_velocity_limit >> 3;
            if (player_state.motion_state.forward_velocity < 0) {
                player_state.motion_state.forward_velocity = 0;
            }
        } else if (player_state.motion_state.forward_velocity < 0) {
            player_state.motion_state.forward_velocity += player_movement_velocity_limit >> 3;
            if (player_state.motion_state.forward_velocity > 0) {
                player_state.motion_state.forward_velocity = 0;
            }
        }
        if (input & PADl) {
            strafe = player_state.motion_state.strafe_velocity + (player_movement_velocity_limit >> 2);
            if (strafe > player_movement_velocity_limit) {
                player_state.motion_state.strafe_velocity = player_movement_velocity_limit;
            } else {
                player_state.motion_state.strafe_velocity = strafe;
            }
        } else if (input & PADn) {
            strafe = player_state.motion_state.strafe_velocity - (player_movement_velocity_limit >> 2);
            if (strafe >= -player_movement_velocity_limit) {
                player_state.motion_state.strafe_velocity = strafe;
            } else {
                player_state.motion_state.strafe_velocity = -player_movement_velocity_limit;
            }
        } else if (player_state.motion_state.strafe_velocity > 0) {
            player_state.motion_state.strafe_velocity -= player_movement_velocity_limit >> 2;
            if (player_state.motion_state.strafe_velocity < 0) {
                player_state.motion_state.strafe_velocity = 0;
            }
        } else if (player_state.motion_state.strafe_velocity < 0) {
            player_state.motion_state.strafe_velocity += player_movement_velocity_limit >> 2;
            if (player_state.motion_state.strafe_velocity > 0) {
                player_state.motion_state.strafe_velocity = 0;
            }
        }
        strafe_sq = player_state.motion_state.strafe_velocity;
        strafe_sq *= strafe_sq;
        forward_sq = player_state.motion_state.forward_velocity;
        forward_sq *= forward_sq;
        magnitude = SquareRoot0(strafe_sq + forward_sq);
        if (magnitude == 0) {
            forward = 0;
            strafe = 0;
        } else {
            strafe = strafe_sq / magnitude;
            if (player_state.motion_state.strafe_velocity < 0) {
                strafe = -(strafe_sq / magnitude);
            }
            forward = forward_sq / magnitude;
            if (player_state.motion_state.forward_velocity < 0) {
                forward = -(forward_sq / magnitude);
            }
        }
        player_state.motion_state.movement_speed = SquareRoot0(strafe * strafe + forward * forward);
        if (forward > 0) {
            player_move_horizontal(player_state.camera_rotation.vy, forward);
        } else if (forward < 0) {
            player_move_horizontal((player_state.camera_rotation.vy + 0x800) & 0xfff, -forward);
        }
        if (strafe > 0) {
            player_move_horizontal((player_state.camera_rotation.vy - 0x400) & 0xfff, strafe);
        } else if (strafe < 0) {
            player_move_horizontal((player_state.camera_rotation.vy + 0x400) & 0xfff, -strafe);
        }
        player_update_view_bob();
        if (input & PADLleft) {
            player_state.motion_state.yaw_step += player_turn_step_limit >> 2;
            if (player_state.motion_state.yaw_step > player_turn_step_limit) {
                player_state.motion_state.yaw_step = player_turn_step_limit;
            }
        } else if (input & PADLright) {
            player_state.motion_state.yaw_step -= player_turn_step_limit >> 2;
            if (player_state.motion_state.yaw_step < -player_turn_step_limit) {
                player_state.motion_state.yaw_step = -player_turn_step_limit;
            }
        } else if (player_state.motion_state.yaw_step > 0) {
            player_state.motion_state.yaw_step -= player_turn_step_limit >> 2;
            if (player_state.motion_state.yaw_step < 0) {
                player_state.motion_state.yaw_step = 0;
            }
        } else if (player_state.motion_state.yaw_step < 0) {
            player_state.motion_state.yaw_step += player_turn_step_limit >> 2;
            if (player_state.motion_state.yaw_step > 0) {
                player_state.motion_state.yaw_step = 0;
            }
        }
        player_state.camera_rotation.vy =
            (player_state.camera_rotation.vy + player_state.motion_state.yaw_step) & 0xfff;
        if (input & PADm) {
            player_state.motion_state.pitch_step += 3;
            if (player_state.motion_state.pitch_step >= 11) {
                player_state.motion_state.pitch_step = 10;
            }
        } else if (input & PADo) {
            player_state.motion_state.pitch_step -= 3;
            if (player_state.motion_state.pitch_step < -10) {
                player_state.motion_state.pitch_step = -10;
            }
        } else if (player_state.motion_state.pitch_step > 0) {
            player_state.motion_state.pitch_step -= 2;
            if (player_state.motion_state.pitch_step < 0) {
                player_state.motion_state.pitch_step = 0;
            }
        } else if (player_state.motion_state.pitch_step < 0) {
            player_state.motion_state.pitch_step += 2;
            if (player_state.motion_state.pitch_step > 0) {
                player_state.motion_state.pitch_step = 0;
            }
        }
        if (player_state.motion_state.pitch_step > 0) {
            player_state.camera_rotation.vx += player_state.motion_state.pitch_step;
            if (player_state.camera_rotation.vx >= 192) {
                player_state.camera_rotation.vx = 191;
            }
        } else if (player_state.motion_state.pitch_step < 0) {
            player_state.camera_rotation.vx += player_state.motion_state.pitch_step;
            if (player_state.camera_rotation.vx < -191) {
                player_state.camera_rotation.vx = -191;
            }
        }
        if ((input & PADRup) && !(player_previous_input & PADRup)) {
            player_begin_weapon_attack();
        }
        if (player_state.equipped_head_armor_id != 0x17) {
            if ((input & PADRleft) && !(player_previous_input & PADRleft)) {
                if (player_state.weapon_attack_fully_charged == 1) {
                    player_state.weapon_attack_fully_charged = 0;
                    switch (player_state.equipped_weapon_id) {
                    case 8:
                        if (player_state.weapon_attack_phase >= 2400 && player_state.weapon_attack_phase <= 3900) {
                            player_state.weapon_magic_delay = 1;
                            player_state.weapon_magic_shots_remaining = (3900 - player_state.weapon_attack_phase) / 300 + 1;
                            goto magic_done;
                        }
                        break;
                    case 7:
                    case 11:
                        if (player_state.weapon_attack_phase >= 2400 && player_state.weapon_attack_phase <= 3900) {
                            player_state.weapon_magic_shots_remaining = 1;
                            player_state.weapon_magic_delay = 1;
                            goto magic_done;
                        }
                        break;
                    case 3:
                        if (player_state.weapon_attack_phase >= 900 && player_state.weapon_attack_phase <= 2400) {
                            player_state.weapon_magic_delay = 1;
                            player_state.weapon_magic_shots_remaining = ((3900 - player_state.weapon_attack_phase) / 300 + 1) * 2;
                            goto magic_done;
                        }
                        break;
                    }
                }
                if (player_state.selected_magic_id != 0xff && player_state.magic_charge == 5000) {
                    player_state.weapon_magic_delay = 0;
                    player_state.weapon_magic_shots_remaining = 0;
                    if (player_state.equipped_accessory_id == 50 && player_state.selected_magic_id == 7) {
                        cost = player_state.selected_magic_record->mp_cost >> 1;
                    } else {
                        cost = player_state.selected_magic_record->mp_cost;
                    }
                    if (player_state.vitals.current_mp >= cost) {
                        player_state.vitals.current_mp -= cost;
                        magic_cast();
                        player_state.magic_charge = 0;
                        player_state.weapon_attack_fully_charged = 0;
                        goto magic_done;
                    }
                }
                magic_id = player_state.selected_magic_id;
                if (magic_id != 0xff) {
                    player_state.magic_charge +=
                        fixed6_ratio_step(player_state.magic, player_state.selected_magic_record->charge_rate) * 2;
                    if (player_state.magic_charge > 5000) {
                        player_state.magic_charge = 5000;
                    }
                }
            } else {
                player_state.magic_charge +=
                    fixed6_ratio_step(player_state.magic, player_state.selected_magic_record->charge_rate) * 2;
                if (player_state.magic_charge > 5000) {
                    player_state.magic_charge = 5000;
                }
            }
        }
    magic_done:
        if (player_state.weapon_magic_delay == 1) {
            if (player_state.weapon_magic_shots_remaining != 0) {
                switch (player_state.equipped_weapon_id) {
                case 7:
                    if (player_state.physical_power < 80 || player_state.magic < 80) {
                        goto cancel;
                    }
                    effect = 20;
                    record = &magic_records[20];
                    player_state.weapon_magic_delay = 3;
                    break;
                case 8:
                    if (magic_records[5].learned == 0) {
                        goto cancel;
                    }
                    effect = 5;
                    record = &magic_records[5];
                    player_state.weapon_magic_delay = 2;
                    break;
                case 11:
                    if (player_state.physical_power < 80 || player_state.magic < 80) {
                        goto cancel;
                    }
                    effect = 36;
                    record = &magic_records[18];
                    player_state.weapon_magic_delay = 3;
                    break;
                case 3:
                    if (player_state.physical_power < 60 || player_state.magic < 60) {
                        goto cancel;
                    }
                    effect = 8;
                    record = &magic_records[8];
                    player_state.weapon_magic_delay = 1;
                    break;
                default:
                cancel:
                    player_state.weapon_magic_shots_remaining = 0;
                    player_state.weapon_magic_delay = 2;
                    goto store_input;
                }
                if (player_state.vitals.current_mp >= record->mp_cost) {
                    if (player_state.weapon_magic_shots_remaining == 1) {
                        player_state.vitals.current_mp -= record->mp_cost;
                    }
                    scale.vx = 200;
                    scale.vy = 200;
                    scale.vz = 400;
                    angles.x = -player_state.camera_rotation.vx;
                    angles.y = player_state.camera_rotation.vy;
                    angles.z = -player_state.camera_rotation.vz;
                    matrix_set_rotation_yxz(&angles, &matrix);
                    ApplyMatrix(&matrix, &scale, &position);
                    position.vx += player_state.camera_position.vx;
                    position.vy += player_state.camera_position.vy;
                    angles.y = player_state.camera_rotation.vy;
                    position.vz += player_state.camera_position.vz;
                    angles.x = player_state.camera_rotation.vx;
                    angles.z = player_state.camera_rotation.vz;
                    origin = (const struct KfVec3i *)&player_state.camera_position;
                    if ((effect == 5 || effect == 8) && player_state.weapon_magic_shots_remaining != 1) {
                        actor_state.player_target = actor_pool_find_target_in_cone(
                            origin, player_state.camera_rotation.vy, 20000, 0x155, &distance);
                        angles.x -= 4 - (rand() >> 9);
                        angles.y -= 4 - (rand() >> 9);
                        attachment = player_state.weapon_magic_shots_remaining & 1;
                    } else {
                        target = actor_pool_find_target_in_cone(
                            (const struct KfVec3i *)&player_state.camera_position,
                            player_state.camera_rotation.vy, 20000, 0x555, &distance);
                        actor_state.player_target = target;
                        if (target == 0) {
                            attachment = 0xff;
                        } else {
                            attachment = target - actor_state.actors;
                        }
                    }
                    pitch_yaw_to_forward_vector((const struct KfPitchYaw *)&angles, &direction);
                    vector3s_scale_shift12(900, &direction);
                    effect_pool_construct(10, 17, effect, &position, &direction,
                                  &player_state.camera_rotation, attachment, 1);
                    if (effect == 20) {
                        position.vy += 300;
                        angles.y = player_state.camera_rotation.vy;
                        angles.z = player_state.camera_rotation.vz;
                        angles.x = player_state.camera_rotation.vx + 64;
                        effect_pool_construct(10, 17, 20, &position, &direction, &angles, attachment, 0);
                        angles.x -= 128;
                        position.vy -= 600;
                        effect_pool_construct(10, 17, 20, &position, &direction, &angles, attachment, 0);
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
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);
    player_update_weapon_attack();
    lighting_set_active_color_matrix(KF_GAME_COLOR_DEFAULT);
    if (player_state.status_effect1_timer != -1) {
        if (!(player_state.status_effect_flags & 0x2) && player_state.status_effect1_timer >= 33) {
            player_state.status_effect1_timer = 32;
        }
        player_state.status_effect1_timer--;
        if (player_state.status_effect1_timer == -1) {
            player_state.status_effect_flags &= ~0x2;
        } else {
            fade = player_state.status_effect1_timer - 968;
            if (fade < 0) {
                fade = 32 - player_state.status_effect1_timer;
            }
            if (fade >= 0) {
                lighting_set_color_matrix(&player_status_effect1_color_matrix, color_matrix_table, fade << 7);
                fog_interpolate_near(5000, 11000, fade << 7);
            } else {
                SetColorMatrix(&player_status_effect1_color_matrix);
                fog_set_near(5000);
            }
        }
    } else {
        fog_set_near(11000);
    }
    if (player_state.update_state != KF_PLAYER_UPDATE_NORMAL
        && player_state.update_state != KF_PLAYER_UPDATE_DYING) {
        if (player_state.update_state >= KF_PLAYER_DAMAGE_FRAME_END) {
            player_state.update_state = KF_PLAYER_UPDATE_NORMAL;
            player_state.view_rotation_offset = player_damage_camera_offsets[0];
            if (player_state.vitals.current_hp == 0) {
                player_death_begin();
            }
        } else {
            player_state.view_rotation_offset = player_damage_camera_offsets[player_state.update_state];
            lighting_set_active_color_matrix(KF_GAME_COLOR_DAMAGE);
            player_state.update_state++;
        }
    }
    if (player_state.equipped_weapon_id != 0xff) {
        if (player_state.equipped_weapon_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_weapon_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_weapon_record->mp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_weapon_record->mp_regen_interval == 0) {
            player_adjust_mp(1);
        }
    }
    if (player_state.equipped_shield_id != 0xff) {
        if (player_state.equipped_shield_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_shield_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_shield_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_shield_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_head_armor_id != 0xff) {
        if (player_state.equipped_head_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_head_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_head_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_head_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_body_armor_id != 0xff) {
        if (player_state.equipped_body_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_body_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_body_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_body_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_arm_armor_id != 0xff) {
        if (player_state.equipped_arm_armor_record->hp_regen_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_arm_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_arm_armor_record->hp_drain_interval != 0
            && player_state.equipment_effect_ticks % player_state.equipped_arm_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_leg_armor_id != 0xff) {
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
    attribute = map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x];
    switch (attribute) {
    case 0x3a:
        if (player_state.update_state == KF_PLAYER_UPDATE_NORMAL) {
            player_apply_damage(5, 3, 5, 0, 0, 0, 0x1000, 10);
        }
        break;
    case 0x3f:
        player_apply_damage(0, 0, 0, 4, 0, 0, 0x1000, 10);
        break;
    }
    if (player_state.status_effect3_timer != -1) {
        if (!(player_state.status_effect_flags & 0x8)) {
            player_state.status_effect3_timer = -1;
            player_state.status_effect_flags &= ~0x8;
        } else {
            player_state.status_effect3_timer--;
            if (player_state.status_effect3_timer == -1) {
                player_state.status_effect_flags &= ~0x8;
            }
        }
    }
    if (player_state.poison_timer != KF_POISON_TIMER_INACTIVE) {
        if (!(player_state.status_effect_flags & KF_PLAYER_STATUS_POISON)) {
            player_state.poison_timer = KF_POISON_TIMER_INACTIVE;
            player_state.status_effect_flags &= ~KF_PLAYER_STATUS_POISON;
        } else {
            player_state.poison_timer--;
            if (player_state.poison_timer == KF_POISON_TIMER_INACTIVE) {
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
    if (player_state.status_effect0_timer != -1) {
        if (!(player_state.status_effect_flags & 0x1)) {
            player_state.status_effect0_timer = 0;
            player_state.status_effect_flags &= ~0x1;
            player_recalculate_combat_stats();
        } else if (player_state.status_effect0_timer == 0) {
            player_state.status_effect_flags &= ~0x1;
            player_recalculate_combat_stats();
        } else if (player_state.status_effect0_timer == 600) {
            player_recalculate_combat_stats();
        }
        player_state.status_effect0_timer--;
    }
    if (player_state.status_effect4_timer != -1) {
        if (player_state.status_effect4_timer == 0) {
            player_state.status_effect_flags &= ~0x10;
            player_recalculate_combat_stats();
        } else if (player_state.status_effect4_timer == 500) {
            player_recalculate_combat_stats();
        }
        lighting_set_active_color_matrix(KF_GAME_COLOR_DEFENSE_EFFECT);
        player_state.status_effect4_timer--;
    }
    if (player_state.equipped_weapon_id == 9) {
        lighting_apply_weapon9_environment();
    }
    if (player_state.light_effect_timer != -1) {
        player_state.light_effect_timer--;
        lighting_apply_timed_player_effect();
    }
}
