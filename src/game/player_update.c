#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfActorState actor_state;
extern KfMagicRecord magic_records[24];
extern u8 map_cell_attribute_grid[100][100];
extern struct KfMatrix color_matrix_table[7];
extern struct KfMatrix DAT_80055858;
extern struct KfVec4s DAT_80055878[8];
extern u32 DAT_80057b30;
extern s32 DAT_80057e68;
extern s32 DAT_80057e70;
extern u32 game_exit_code;

extern u32 func_8005012c(s32 mode);
extern void func_8001b7b0(char object);
extern s32 func_80036e38(s32 arg0);
extern void func_80020a2c(void);
extern void func_800365f8(void);
extern void func_80034de4();
extern void func_8003a2a0(void);
extern void func_8001bab8(s32 arg0);
extern KfEffectRecord *func_80036f44();
extern s32 rand(void);
extern s32 SquareRoot0(s32 value);
extern struct KfVec4i *ApplyMatrix(
    struct KfMatrix *matrix, struct KfVec4s *vector, struct KfVec4i *result);
extern void SetColorMatrix(struct KfMatrix *matrix);
extern void matrix_set_rotation_yxz(
    const struct KfEulerAngles *angles, struct KfMatrix *matrix);
extern void pitch_yaw_to_forward_vector(
    const struct KfPitchYaw *angles, struct KfVec3s *direction);
extern void vector3s_scale_shift12(s16 scale, s16 *vector);
extern s32 fixed6_ratio_step(s32 value, s32 span);
extern KfActor *actor_pool_find_target_in_cone(
    const struct KfVec3i *origin, s32 facing, u32 max_distance,
    s32 angle_tolerance, s32 *distance_out);
extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern void audio_close_vab(void);
extern void lighting_set_color_matrix(
    const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);
extern void fog_interpolate_near(s32 start, s32 end, s32 ratio);
extern void fog_set_near(s32 distance);
extern void lighting_apply_weapon9_environment(void);
extern void lighting_apply_timed_player_effect(void);
extern void player_death_update(void);
extern void player_death_update_reverse_fade(void);
extern void player_death_begin(void);
extern void player_use_item();
extern void player_sync_position_to_map(void);
extern void player_equip_weapon(u8 weapon_id);
extern void player_select_magic(u8 magic_id);
extern s32 player_move_horizontal(s32 heading, s32 distance);
extern void player_update_view_bob(void);
extern void player_update_vertical_motion(void);
extern void player_begin_weapon_attack(void);
extern void player_update_weapon_attack(void);
extern void player_adjust_hp(s32 delta);
extern void player_adjust_mp(s32 delta);
extern void player_recalculate_combat_stats(void);
extern void player_apply_damage(
    u16 component0, u16 component1, u16 component2, u16 status_effect_flags,
    u16 component3, u16 component4, u16 scale_q12, u16 multiplier_tenths);

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
    struct KfVec4s scale;
    struct KfEulerAngles angles;
    struct KfVec4i position;
    struct KfMatrix matrix;
    s32 distance;
    u8 attribute;
    u8 magic_id;

    if (player_state.update_state == 0xff) {
        player_death_update();
        return;
    }
    if (player_state.update_state == 0xfe) {
        player_death_update_reverse_fade();
        return;
    }
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, -1);
    input = func_8005012c(1);
    if (input & 0x800) {
        func_8001b7b0(3);
    }
    if (input & 0x100) {
        input = 0x40;
    }
    if ((input & 0x40) && !(DAT_80057b30 & 0x40) && player_state.weapon_attack_phase == -1) {
        item = func_80036e38(0);
        if (item >= 0) {
            player_use_item(item);
        } else if (item == -3) {
            func_80020a2c();
            audio_close_vab();
            func_800365f8();
            player_sync_position_to_map();
            player_state.previous_map_cell.x = player_state.map_cell.x;
            player_state.previous_map_cell.z = player_state.map_cell.z;
            player_equip_weapon(player_state.equipped_weapon_id);
            player_select_magic(player_state.selected_magic_id);
        } else if (item == -2) {
            game_exit_code = 1;
            return;
        }
        DAT_80057b30 = input;
    } else {
        if ((input & 0x20) && !(DAT_80057b30 & 0x20)) {
            func_80034de4(&player_state.camera_position, &player_state.camera_rotation);
        }
        if (player_state.status_effect_flags & 0x8) {
            DAT_80057e68 = 36;
            DAT_80057e70 = 5;
        } else {
            DAT_80057e68 = 180;
            DAT_80057e70 = 28;
        }
        if (input & 0x1000) {
            forward = player_state.motion_state.forward_velocity + (DAT_80057e68 >> 2);
            if (forward > DAT_80057e68) {
                player_state.motion_state.forward_velocity = DAT_80057e68;
            } else {
                player_state.motion_state.forward_velocity = forward;
            }
        } else if (input & 0x4000) {
            forward = player_state.motion_state.forward_velocity - (DAT_80057e68 >> 2);
            if (forward >= -DAT_80057e68) {
                player_state.motion_state.forward_velocity = forward;
            } else {
                player_state.motion_state.forward_velocity = -DAT_80057e68;
            }
        } else if (player_state.motion_state.forward_velocity > 0) {
            player_state.motion_state.forward_velocity -= DAT_80057e68 >> 3;
            if (player_state.motion_state.forward_velocity < 0) {
                player_state.motion_state.forward_velocity = 0;
            }
        } else if (player_state.motion_state.forward_velocity < 0) {
            player_state.motion_state.forward_velocity += DAT_80057e68 >> 3;
            if (player_state.motion_state.forward_velocity > 0) {
                player_state.motion_state.forward_velocity = 0;
            }
        }
        if (input & 0x8) {
            strafe = player_state.motion_state.strafe_velocity + (DAT_80057e68 >> 2);
            if (strafe > DAT_80057e68) {
                player_state.motion_state.strafe_velocity = DAT_80057e68;
            } else {
                player_state.motion_state.strafe_velocity = strafe;
            }
        } else if (input & 0x4) {
            strafe = player_state.motion_state.strafe_velocity - (DAT_80057e68 >> 2);
            if (strafe >= -DAT_80057e68) {
                player_state.motion_state.strafe_velocity = strafe;
            } else {
                player_state.motion_state.strafe_velocity = -DAT_80057e68;
            }
        } else if (player_state.motion_state.strafe_velocity > 0) {
            player_state.motion_state.strafe_velocity -= DAT_80057e68 >> 2;
            if (player_state.motion_state.strafe_velocity < 0) {
                player_state.motion_state.strafe_velocity = 0;
            }
        } else if (player_state.motion_state.strafe_velocity < 0) {
            player_state.motion_state.strafe_velocity += DAT_80057e68 >> 2;
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
            player_move_horizontal(player_state.camera_rotation.y, forward);
        } else if (forward < 0) {
            player_move_horizontal((player_state.camera_rotation.y + 0x800) & 0xfff, -forward);
        }
        if (strafe > 0) {
            player_move_horizontal((player_state.camera_rotation.y - 0x400) & 0xfff, strafe);
        } else if (strafe < 0) {
            player_move_horizontal((player_state.camera_rotation.y + 0x400) & 0xfff, -strafe);
        }
        player_update_view_bob();
        if (input & 0x8000) {
            player_state.motion_state.yaw_step += DAT_80057e70 >> 2;
            if (player_state.motion_state.yaw_step > DAT_80057e70) {
                player_state.motion_state.yaw_step = DAT_80057e70;
            }
        } else if (input & 0x2000) {
            player_state.motion_state.yaw_step -= DAT_80057e70 >> 2;
            if (player_state.motion_state.yaw_step < -DAT_80057e70) {
                player_state.motion_state.yaw_step = -DAT_80057e70;
            }
        } else if (player_state.motion_state.yaw_step > 0) {
            player_state.motion_state.yaw_step -= DAT_80057e70 >> 2;
            if (player_state.motion_state.yaw_step < 0) {
                player_state.motion_state.yaw_step = 0;
            }
        } else if (player_state.motion_state.yaw_step < 0) {
            player_state.motion_state.yaw_step += DAT_80057e70 >> 2;
            if (player_state.motion_state.yaw_step > 0) {
                player_state.motion_state.yaw_step = 0;
            }
        }
        player_state.camera_rotation.y =
            (player_state.camera_rotation.y + player_state.motion_state.yaw_step) & 0xfff;
        if (input & 0x2) {
            player_state.motion_state.pitch_step += 3;
            if (player_state.motion_state.pitch_step >= 11) {
                player_state.motion_state.pitch_step = 10;
            }
        } else if (input & 0x1) {
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
            player_state.camera_rotation.x += player_state.motion_state.pitch_step;
            if (player_state.camera_rotation.x >= 192) {
                player_state.camera_rotation.x = 191;
            }
        } else if (player_state.motion_state.pitch_step < 0) {
            player_state.camera_rotation.x += player_state.motion_state.pitch_step;
            if (player_state.camera_rotation.x < -191) {
                player_state.camera_rotation.x = -191;
            }
        }
        if ((input & 0x10) && !(DAT_80057b30 & 0x10)) {
            player_begin_weapon_attack();
        }
        if (player_state.equipped_head_armor_id != 0x17) {
            if ((input & 0x80) && !(DAT_80057b30 & 0x80)) {
                if (player_state.weapon_attack_fully_charged == 1) {
                    player_state.weapon_attack_fully_charged = 0;
                    switch (player_state.equipped_weapon_id) {
                    case 8:
                        if (player_state.weapon_attack_phase >= 2400 && player_state.weapon_attack_phase <= 3900) {
                            player_state.unknown_79 = 1;
                            player_state.unknown_78 = (3900 - player_state.weapon_attack_phase) / 300 + 1;
                            goto magic_done;
                        }
                        break;
                    case 7:
                    case 11:
                        if (player_state.weapon_attack_phase >= 2400 && player_state.weapon_attack_phase <= 3900) {
                            player_state.unknown_78 = 1;
                            player_state.unknown_79 = 1;
                            goto magic_done;
                        }
                        break;
                    case 3:
                        if (player_state.weapon_attack_phase >= 900 && player_state.weapon_attack_phase <= 2400) {
                            player_state.unknown_79 = 1;
                            player_state.unknown_78 = ((3900 - player_state.weapon_attack_phase) / 300 + 1) * 2;
                            goto magic_done;
                        }
                        break;
                    }
                }
                if (player_state.selected_magic_id != 0xff && player_state.magic_charge == 5000) {
                    player_state.unknown_79 = 0;
                    player_state.unknown_78 = 0;
                    if (player_state.equipped_accessory_id == 50 && player_state.selected_magic_id == 7) {
                        cost = player_state.selected_magic_record->mp_cost >> 1;
                    } else {
                        cost = player_state.selected_magic_record->mp_cost;
                    }
                    if (player_state.vitals.current_mp >= cost) {
                        player_state.vitals.current_mp -= cost;
                        func_8003a2a0();
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
        if (player_state.unknown_79 == 1) {
            if (player_state.unknown_78 != 0) {
                switch (player_state.equipped_weapon_id) {
                case 7:
                    if (player_state.physical_power < 80 || player_state.magic < 80) {
                        goto cancel;
                    }
                    effect = 20;
                    record = &magic_records[20];
                    player_state.unknown_79 = 3;
                    break;
                case 8:
                    if (magic_records[5].learned == 0) {
                        goto cancel;
                    }
                    effect = 5;
                    record = &magic_records[5];
                    player_state.unknown_79 = 2;
                    break;
                case 11:
                    if (player_state.physical_power < 80 || player_state.magic < 80) {
                        goto cancel;
                    }
                    effect = 36;
                    record = &magic_records[18];
                    player_state.unknown_79 = 3;
                    break;
                case 3:
                    if (player_state.physical_power < 60 || player_state.magic < 60) {
                        goto cancel;
                    }
                    effect = 8;
                    record = &magic_records[8];
                    player_state.unknown_79 = 1;
                    break;
                default:
                cancel:
                    player_state.unknown_78 = 0;
                    player_state.unknown_79 = 2;
                    goto store_input;
                }
                if (player_state.vitals.current_mp >= record->mp_cost) {
                    if (player_state.unknown_78 == 1) {
                        player_state.vitals.current_mp -= record->mp_cost;
                    }
                    scale.x = 200;
                    scale.y = 200;
                    scale.z = 400;
                    angles.x = -player_state.camera_rotation.x;
                    angles.y = player_state.camera_rotation.y;
                    angles.z = -player_state.camera_rotation.z;
                    matrix_set_rotation_yxz(&angles, &matrix);
                    ApplyMatrix(&matrix, &scale, &position);
                    position.x += player_state.camera_position.x;
                    position.y += player_state.camera_position.y;
                    angles.y = player_state.camera_rotation.y;
                    position.z += player_state.camera_position.z;
                    angles.x = player_state.camera_rotation.x;
                    angles.z = player_state.camera_rotation.z;
                    origin = (const struct KfVec3i *)&player_state.camera_position;
                    if ((effect == 5 || effect == 8) && player_state.unknown_78 != 1) {
                        actor_state.player_target = actor_pool_find_target_in_cone(
                            origin, player_state.camera_rotation.y, 20000, 0x155, &distance);
                        angles.x -= 4 - (rand() >> 9);
                        angles.y -= 4 - (rand() >> 9);
                        attachment = player_state.unknown_78 & 1;
                    } else {
                        target = actor_pool_find_target_in_cone(
                            (const struct KfVec3i *)&player_state.camera_position,
                            player_state.camera_rotation.y, 20000, 0x555, &distance);
                        actor_state.player_target = target;
                        if (target == 0) {
                            attachment = 0xff;
                        } else {
                            attachment = target - actor_state.actors;
                        }
                    }
                    pitch_yaw_to_forward_vector((const struct KfPitchYaw *)&angles, &direction);
                    vector3s_scale_shift12(900, (s16 *)&direction);
                    func_80036f44(10, 17, effect, &position, &direction,
                                  &player_state.camera_rotation, attachment, 1);
                    if (effect == 20) {
                        position.y += 300;
                        angles.y = player_state.camera_rotation.y;
                        angles.z = player_state.camera_rotation.z;
                        angles.x = player_state.camera_rotation.x + 64;
                        func_80036f44(10, 17, 20, &position, &direction, &angles, attachment, 0);
                        angles.x -= 128;
                        position.y -= 600;
                        func_80036f44(10, 17, 20, &position, &direction, &angles, attachment, 0);
                    }
                }
                player_state.unknown_78--;
            }
        } else if (player_state.unknown_79 != 0) {
            player_state.unknown_79--;
        }
    store_input:
        DAT_80057b30 = input;
        player_update_vertical_motion();
    }
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);
    player_update_weapon_attack();
    func_8001bab8(0);
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
                if (fade >= 0) {
                    lighting_set_color_matrix(&DAT_80055858, color_matrix_table, fade << 7);
                    fog_interpolate_near(5000, 11000, fade << 7);
                } else {
                    fog_set_near(11000);
                }
            } else {
                SetColorMatrix(&DAT_80055858);
                fog_set_near(5000);
            }
        }
    }
    if (player_state.update_state != 0 && player_state.update_state != 0xff) {
        if (player_state.update_state >= 8) {
            player_state.update_state = 0;
            player_state.view_rotation_offset = DAT_80055878[0];
            if (player_state.vitals.current_hp == 0) {
                player_death_begin();
            }
        } else {
            player_state.view_rotation_offset = DAT_80055878[player_state.update_state];
            func_8001bab8(1);
            player_state.update_state++;
        }
    }
    if (player_state.equipped_weapon_id != 0xff) {
        if (player_state.equipped_weapon_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_weapon_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_weapon_record->mp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_weapon_record->mp_regen_interval == 0) {
            player_adjust_mp(1);
        }
    }
    if (player_state.equipped_shield_id != 0xff) {
        if (player_state.equipped_shield_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_shield_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_shield_record->hp_drain_interval != 0
            && player_state.unknown_58 % player_state.equipped_shield_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_head_armor_id != 0xff) {
        if (player_state.equipped_head_armor_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_head_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_head_armor_record->hp_drain_interval != 0
            && player_state.unknown_58 % player_state.equipped_head_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_body_armor_id != 0xff) {
        if (player_state.equipped_body_armor_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_body_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_body_armor_record->hp_drain_interval != 0
            && player_state.unknown_58 % player_state.equipped_body_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_arm_armor_id != 0xff) {
        if (player_state.equipped_arm_armor_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_arm_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_arm_armor_record->hp_drain_interval != 0
            && player_state.unknown_58 % player_state.equipped_arm_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    if (player_state.equipped_leg_armor_id != 0xff) {
        if (player_state.equipped_leg_armor_record->hp_regen_interval != 0
            && player_state.unknown_58 % player_state.equipped_leg_armor_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (player_state.equipped_leg_armor_record->hp_drain_interval != 0
            && player_state.unknown_58 % player_state.equipped_leg_armor_record->hp_drain_interval == 0) {
            player_adjust_hp(-1);
        }
    }
    player_state.unknown_58++;
    attribute = map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x];
    switch (attribute) {
    case 0x3a:
        if (player_state.update_state == 0) {
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
    if (player_state.status_effect2_timer != -1) {
        if (!(player_state.status_effect_flags & 0x4)) {
            player_state.status_effect2_timer = -1;
            player_state.status_effect_flags &= ~0x4;
        } else {
            player_state.status_effect2_timer--;
            if (player_state.status_effect2_timer == -1) {
                player_state.status_effect_flags &= ~0x4;
            } else {
                phase = player_state.status_effect2_timer % 20;
                if (phase < 2) {
                    func_8001bab8(1);
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
        func_8001bab8(2);
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
