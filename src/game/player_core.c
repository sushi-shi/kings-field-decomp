#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_player.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>


/*
 * Player movement, motion, and interaction run, one contiguous band
 * 0x80016848..0x80017edc (GAME.EXE): vertical motion, view bob, weapon attack,
 * warp helpers, and related per-frame player updates. Assembled from ten
 * address-adjacent single-purpose units; module boundary is WIP.
 */


RODATA(0x80012030, 0x18)

DATA(0x8005581c, 0x10)
char weapon_image_path_template[16] = "WEPON\\WEP00.MIM";

DATA(0x8005582c, 0xa)
KfFloorEntryCell floor_entry_cells[5] = {
    {15, 2}, {29, 56}, {28, 18}, {7, 22}, {39, 69}
};

/* Stores the item id into one of the six armor slots, re-resolves the five armor record pointers, and recalculates. */
ADDRESS(0x80016848, 0x1e8)
void player_set_equipment_slot(u8 item_id, u8 slot)
{
    switch (slot) {
    case 0:
        player_state.equipped_shield_id = item_id;
        break;
    case 1:
        player_state.equipped_head_armor_id = item_id;
        break;
    case 2:
        player_state.equipped_arm_armor_id = item_id;
        break;
    case 3:
        player_state.equipped_leg_armor_id = item_id;
        break;
    case 4:
        player_state.equipped_body_armor_id = item_id;
        break;
    case 5:
        player_state.equipped_accessory_id = item_id;
        break;
    }
    if (player_state.equipped_shield_id != 0xff) {
        player_state.equipped_shield_record = &armor_records[player_state.equipped_shield_id - 13];
    } else {
        player_state.equipped_shield_record = 0;
    }
    if (player_state.equipped_head_armor_id != 0xff) {
        player_state.equipped_head_armor_record =
            &armor_records[player_state.equipped_head_armor_id - 13];
    } else {
        player_state.equipped_head_armor_record = 0;
    }
    if (player_state.equipped_arm_armor_id != 0xff) {
        player_state.equipped_arm_armor_record =
            &armor_records[player_state.equipped_arm_armor_id - 13];
    } else {
        player_state.equipped_arm_armor_record = 0;
    }
    if (player_state.equipped_leg_armor_id != 0xff) {
        player_state.equipped_leg_armor_record =
            &armor_records[player_state.equipped_leg_armor_id - 13];
    } else {
        player_state.equipped_leg_armor_record = 0;
    }
    if (player_state.equipped_body_armor_id != 0xff) {
        player_state.equipped_body_armor_record =
            &armor_records[player_state.equipped_body_armor_id - 13];
    } else {
        player_state.equipped_body_armor_record = 0;
    }
    player_recalculate_combat_stats();
}

/* weapon_image_path_template has decimal weapon-id digits at [9] and [10]. */
ADDRESS(0x80016a30, 0xf4)
void player_equip_weapon(u8 weapon_id)
{
    player_state.attack_charge_state.current = 0;
    player_state.attack_charge_state.committed = 0;
    player_state.weapon_charge_delay = 10;
    player_state.equipped_weapon_id = weapon_id;
    if (weapon_id != 0xff) {
        player_state.equipped_weapon_record = &weapon_records[weapon_id];
        weapon_image_path_template[9] = '0' + weapon_id / 10;
        weapon_image_path_template[10] = '0' + weapon_id % 10;
        if (cd_file_load_into(player_state.weapon_asset_buffer, weapon_image_path_template) != 0) {
            exit(1);
        }
        asset_registry_set(0x14, player_state.weapon_asset_buffer);
    }
    player_state.weapon_attack_phase = -1;
    player_state.weapon_animation_cache = 0;
    player_recalculate_combat_stats();
}

ADDRESS(0x80016b24, 0x9c)
void player_begin_weapon_attack(void)
{
    if (player_state.weapon_attack_phase == -1 && player_state.equipped_weapon_id != 0xff) {
        player_state.weapon_attack_phase = 0;
        sound_ref_play(&player_sound_refs[0], 0x7f);
        player_state.attack_charge_state.committed = player_state.attack_charge_state.current;
        if (player_state.attack_charge_state.current == 5000) {
            player_state.weapon_attack_fully_charged = 1;
        } else {
            player_state.weapon_attack_fully_charged = 0;
        }
        player_state.attack_charge_state.current = 0;
    }
}


/* Psy-Q LIBGTE: RotMatrix(SVECTOR *r, MATRIX *m); ApplyMatrix(MATRIX *m, SVECTOR *v, VECTOR *rv). */

ADDRESS(0x80016bc0, 0x264)
void player_update_weapon_attack(void)
{
    SVECTOR offset;
    SVECTOR rotation;
    VECTOR result;
    MATRIX matrix;
    u16 window;
    s32 actor;

    if (player_state.equipped_weapon_id == 0xff) {
        return;
    }
    if (player_state.weapon_attack_phase != -1) {
        player_state.weapon_attack_phase += 300;
        window = player_state.weapon_attack_phase;
        if (player_state.equipped_weapon_id == 3
                ? (u16)(window - 1000) < 300
                : (u16)(window - 3072) < 300) {
            offset.vx = 0;
            offset.vy = 1000;
            offset.vz = player_state.equipped_weapon_record->attack_z_offset;
            rotation.vx = 0;
            rotation.vy = -player_state.camera_rotation.vy;
            rotation.vz = 0;
            RotMatrix(&rotation, &matrix);
            ApplyMatrix(&matrix, &offset, &result);
            result.vx += player_state.camera_position.vx;
            result.vy += player_state.camera_position.vy;
            result.vz += player_state.camera_position.vz;
            actor = actor_pool_find_overlap(result.vx, result.vy, result.vz, 800, 1000);
            if (actor != -1) {
                actor_apply_damage(
                    actor,
                    player_state.physical_power,
                    player_state.attack_component0,
                    player_state.attack_component1,
                    player_state.attack_component2,
                    player_state.attack_component3,
                    player_state.attack_component4,
                    player_state.attack_charge_state.committed,
                    0x10);
            }
            player_state.attack_charge_state.committed = 0;
            player_state.attack_charge_state.current = 0;
            player_state.weapon_charge_delay = 10;
        }
        if (player_state.weapon_attack_phase >= 4096) {
            player_state.weapon_attack_phase = -1;
        }
    } else if (player_state.weapon_charge_delay == 0) {
        player_state.attack_charge_state.current +=
            fixed6_ratio_step(
                player_state.physical_power,
                player_state.equipped_weapon_record->charge_rate) * 2;
        if (player_state.attack_charge_state.current >= 5001) {
            player_state.attack_charge_state.current = 5000;
        }
    } else {
        player_state.weapon_charge_delay--;
    }
}


ADDRESS(0x80016e24, 0x94)
void game_initialize_session(void)
{
    player_state.camera_rotation.vz = 0;
    player_state.camera_rotation.vy = 0;
    player_state.camera_rotation.vx = 0;
    player_state.camera_position.vx = 0x7918;
    player_state.camera_position.vy = 0;
    player_state.camera_position.vz = 0xfa0;
    player_state.weapon_asset_buffer = memory_allocate(0xc000);
    game_state_initialize();
    player_state.update_state = 0;
    player_state.audio_effects_enabled = 1;
    player_state.audio_music_enabled = 1;
    player_state.hud_gauges_enabled = 1;
    player_state.compass_enabled = 1;
}

ADDRESS(0x80016eb8, 0x30)
void player_clear_motion(void)
{
    player_state.motion_state.yaw_step = 0;
    player_state.motion_state.pitch_step = 0;
    player_state.motion_state.movement_speed = 0;
    player_state.motion_state.forward_velocity = 0;
    player_state.motion_state.strafe_velocity = 0;
}

ADDRESS(0x80016ee8, 0x158)
void player_sync_position_to_map(void)
{
    s32 cell_x = player_state.camera_position.vx / KF_MAP_TILE_SIZE;
    s32 cell_z = player_state.camera_position.vz / KF_MAP_TILE_SIZE;
    s32 floor;
    s32 view_offset;
    s32 floor_height;

    player_state.equipment_effect_ticks = 0;
    player_state.map_cell.x = cell_x;
    player_state.map_cell.z = cell_z;
    floor = map_floor_height_grid[player_state.map_cell.z][player_state.map_cell.x];
    player_state.allow_near_actor_spawn = 1;
    floor_height = -(floor * KF_MAP_HEIGHT_STEP);
    view_offset = player_state.view_bob_offset - 1500;
    player_state.floor_height = floor_height;
    player_state.camera_position.vy = view_offset + floor_height;
    player_clear_motion();
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);
    DAT_80095064 = 0x56;
    player_state.vertical_state = 0;
    player_state.vertical_velocity = 0;
}


/* Psy-Q LIBGTE: rsin, rcos. */
/* Psy-Q LIBGTE: long SquareRoot0(long a); */

ADDRESS(0x80017040, 0xc8)
s32 player_distance_to_point_in_cone(
    const struct KfVec3i *point, s16 facing, s32 max_distance, s32 angle_tolerance)
{
    s32 distance;
    s16 delta;

    distance = player_distance_to_point(point->x, 0xffff, point->z, max_distance, 0);
    if (distance != -1) {
        delta = (vector_xz_to_angle(
                     player_state.camera_position.vx - point->x,
                     point->z - player_state.camera_position.vz)
                 - facing) & 0xfff;
        if (delta > 2048) {
            delta = 0x1000 - delta;
        }
        if (angle_tolerance < delta) {
            distance = -1;
        }
    }
    return distance;
}

/* point_height is reused as the vertical tolerance, as retail keeps it in $a3. */
ADDRESS(0x80017108, 0xf4)
s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height)
{
    s32 dx;
    s32 dz;
    s32 center;
    s32 dy;
    s32 distance;

    dx = player_state.camera_position.vx - point_x;
    if (dx < -max_distance || max_distance < dx) {
        goto out_of_range;
    }
    dz = player_state.camera_position.vz - point_z;
    if (dz < -max_distance || max_distance < dz) {
        goto out_of_range;
    }
    dx >>= 3;
    if (point_y != 0xffff) {
        point_height >>= 1;
        center = point_y - point_height;
        point_height += 850;
        center += 850;
        dy = player_state.floor_height - center;
        if (dy < -point_height || point_height < dy) {
            goto out_of_range;
        }
    }
    dz >>= 3;
    distance = SquareRoot0(dx * dx + dz * dz) << 3;
    if (max_distance < distance) {
        goto out_of_range;
    }
    return distance;
out_of_range:
    return -1;
}

/*
 * Moves the camera by DISTANCE along HEADING: a wall hit slides the target
 * around the collision point once, each axis is accepted only into a
 * passable cell no more than 699 units above the floor, a diagonal cell
 * recentres the position on its wall line, and a blocked corner retries
 * along the wall diagonal. Every exit returns 1.
 */
ADDRESS(0x800171fc, 0x828)
/* The heading arrives already extended; retail never re-extends it. */
s32 player_move_horizontal(s32 heading, s32 distance)
{
    s32 cell_z0 = player_state.map_cell.z;
    s32 cell_x0 = player_state.map_cell.x;
    s32 dz;
    s32 dx;
    s32 new_z;
    s32 new_x;
    s32 angle;
    s32 radius;
    s32 remainder_z;
    s32 remainder_x;
    s32 half;
    s32 step;
    u32 cell_z;
    u32 cell_x;
    s16 delta_x;
    s16 delta_z;
    s16 attempt = 1;
    u8 type;

    dz = (rcos(heading) * distance) >> 12;
    dx = (-rsin(heading) * distance) >> 12;
    new_z = dz + player_state.camera_position.vz;
    new_x = dx + player_state.camera_position.vx;
    for (;;) {
        if (collision_query_world(new_x, player_state.floor_height, new_z, 800, 1700, 2177) == -1) {
            break;
        }
        delta_x = collision_target.position.vx - player_state.camera_position.vx;
        delta_z = collision_target.position.vz - player_state.camera_position.vz;
        angle = vector_xz_to_angle(delta_x, delta_z);
        angle = (angle_mod_delta_le_half_turn(heading, angle) == 0 ? angle + 2112 : angle + 1984)
            & 0xfff;
        radius = collision_target.radius + 900;
        delta_z = (rcos(angle) * radius) >> 12;
        delta_x = (-rsin(angle) * radius) >> 12;
        attempt--;
        new_z = collision_target.position.vz + delta_z;
        dz = new_z - player_state.camera_position.vz;
        new_x = collision_target.position.vx + delta_x;
        dx = new_x - player_state.camera_position.vx;
        if (attempt == -1) {
            return 1;
        }
    }
    cell_z = new_z / KF_MAP_TILE_SIZE;
    if (cell_z < KF_MAP_ROWS && map_collision_grid[cell_z][player_state.map_cell.x] != 0
        && -(map_floor_height_grid[cell_z][player_state.map_cell.x] * KF_MAP_HEIGHT_STEP) - player_state.floor_height
               >= -699) {
        player_state.camera_position.vz = new_z;
        player_state.map_cell.z = cell_z;
    }
    cell_x = new_x / KF_MAP_TILE_SIZE;
    if (cell_x < KF_MAP_COLUMNS && map_collision_grid[player_state.map_cell.z][cell_x] != 0
        && -(map_floor_height_grid[player_state.map_cell.z][cell_x] * KF_MAP_HEIGHT_STEP) - player_state.floor_height
               >= -699) {
        player_state.camera_position.vx = new_x;
        player_state.map_cell.x = cell_x;
    }
    type = map_collision_grid[cell_z0][cell_x0];
    if (type >= 2 && type <= 5) {
        if (player_state.map_cell.x == cell_x0 && player_state.map_cell.z == cell_z0) {
            remainder_z = player_state.camera_position.vz % KF_MAP_TILE_SIZE;
            remainder_x = player_state.camera_position.vx % KF_MAP_TILE_SIZE;
            if (type == 2) {
                if (remainder_x < remainder_z) {
                    half = (remainder_z - remainder_x) / 2;
                    player_state.camera_position.vz -= half;
                    player_state.camera_position.vx += half;
                }
            } else if (type == 3) {
                if (remainder_z + remainder_x >= KF_MAP_TILE_SIZE + 1) {
                    half = (remainder_z + remainder_x - KF_MAP_TILE_SIZE) / 2;
                    player_state.camera_position.vz -= half;
                    player_state.camera_position.vx -= half;
                }
            } else if (type == 4) {
                if (remainder_z < remainder_x) {
                    half = (remainder_x - remainder_z) / 2;
                    player_state.camera_position.vz += half;
                    player_state.camera_position.vx -= half;
                }
            } else if (type == 5) {
                if (remainder_z + remainder_x < KF_MAP_TILE_SIZE) {
                    half = (KF_MAP_TILE_SIZE - (remainder_z + remainder_x)) / 2;
                    player_state.camera_position.vz += half;
                    player_state.camera_position.vx += half;
                }
            }
            player_state.map_cell.z = player_state.camera_position.vz / KF_MAP_TILE_SIZE;
            player_state.map_cell.x = player_state.camera_position.vx / KF_MAP_TILE_SIZE;
        }
        if (map_collision_grid[cell_z][cell_x] == 0) {
            if (dz < 0) {
                dz = -dz;
            }
            if (dx < 0) {
                dx = -dx;
            }
            type = map_collision_grid[player_state.map_cell.z][player_state.map_cell.x];
            if (type == 2) {
                if (dz < dx) {
                    dx = -(distance * 2896) >> 12;
                    dz = dx;
                } else {
                    dx = (distance * 2896) >> 12;
                    dz = dx;
                }
            } else if (type == 3) {
                if (dz < dx) {
                    dz = -(distance * 2896) >> 12;
                    dx = (distance * 2896) >> 12;
                } else {
                    dz = (distance * 2896) >> 12;
                    dx = -(distance * 2896) >> 12;
                }
            } else if (type == 4) {
                if (dx < dz) {
                    dx = -(distance * 2896) >> 12;
                    dz = dx;
                } else {
                    dx = (distance * 2896) >> 12;
                    dz = dx;
                }
            } else if (type == 5) {
                if (dx < dz) {
                    dz = -(distance * 2896) >> 12;
                    dx = (distance * 2896) >> 12;
                } else {
                    dz = (distance * 2896) >> 12;
                    dx = -(distance * 2896) >> 12;
                }
            }
            new_z = dz + player_state.camera_position.vz;
            cell_z = new_z / KF_MAP_TILE_SIZE;
            new_x = dx + player_state.camera_position.vx;
            cell_x = new_x / KF_MAP_TILE_SIZE;
            if (cell_z < KF_MAP_ROWS && cell_x < KF_MAP_COLUMNS && map_collision_grid[cell_z][cell_x] != 0) {
                player_state.camera_position.vz = new_z;
                player_state.camera_position.vx = new_x;
                player_state.map_cell.z = cell_z;
                player_state.map_cell.x = cell_x;
            }
        }
    }
    return 1;
}


ADDRESS(0x80017a24, 0x5c)
void player_update_view_bob(void)
{
    s32 phase;

    if (player_state.vertical_state == 0) {
        phase = (player_state.view_bob_phase + player_state.motion_state.movement_speed * 2)
            & 0xfff;
        player_state.view_bob_phase = phase;
        player_state.view_bob_offset = rsin(phase) >> 6;
    }
}


ADDRESS(0x80017a80, 0x278)
void player_update_vertical_motion(void)
{
    s32 target;
    s32 view_offset;

    target = -(map_floor_height_grid[player_state.map_cell.z][player_state.map_cell.x] * KF_MAP_HEIGHT_STEP);
    if (player_state.update_state != 0xff) {
        if (player_state.floor_height - target < -3000) {
            if (player_state.equipped_leg_armor_id == 0x26
                && map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
                    == 0x5d) {
                goto done;
            }
            player_death_begin();
        } else if (target >= -6999
                   && map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
                       == 0x52) {
            player_death_begin();
        }
    }
    switch (player_state.vertical_state) {
    case 0x10:
    falling:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += 40;
        if (target + 100 < player_state.floor_height) {
            player_state.floor_height = target;
            player_state.vertical_state = 0;
        }
        break;
    case 0x20:
    jumping:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += 5;
        if (player_state.floor_height <= target) {
            player_state.floor_height = target;
            player_state.vertical_state = 0;
        }
        break;
    case 0:
        if (target < player_state.floor_height) {
            player_state.vertical_state = 0x20;
            if ((s16)player_state.motion_state.movement_speed >= 181) {
                player_state.vertical_velocity = -300;
            } else {
                player_state.vertical_velocity = -100;
            }
            goto jumping;
        }
        if (player_state.floor_height < target) {
            player_state.vertical_state = 0x10;
            player_state.vertical_velocity = 0;
            goto falling;
        }
        break;
    }
done:
    view_offset = player_state.view_bob_offset - 1500;
    player_state.camera_position.vy = view_offset + player_state.floor_height;
}


ADDRESS(0x80017cf8, 0x144)
void player_warp_to_floor_entry(void)
{
    VECTOR position;
    const KfFloorEntryCell *entry;
    u8 floor;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(0, &position);
    floor = player_state.progress_state.current_floor;
    entry = &floor_entry_cells[floor - 1];
    player_state.previous_map_cell.x = entry->x;
    player_state.previous_map_cell.z = entry->z;
    player_state.camera_position.vx = player_state.previous_map_cell.x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player_state.camera_position.vx;
    player_state.camera_position.vz = player_state.previous_map_cell.z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player_state.camera_position.vz;
    if (floor == 5 && player_state.map_variant != 1) {
        if (player_state.map_variant == 3) {
            audio_play_current_map_sequence();
        }
        pool_release_all();
        player_state.map_variant = 1;
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    position.vy = player_state.floor_height;
    player_warp_shimmer(1, &position);
}


ADDRESS(0x80017e3c, 0xa0)
void player_update_transform_snapshot(VECTOR *position_out, SVECTOR *rotation_out)
{
    *position_out = player_state.camera_position;
    *rotation_out = player_state.camera_rotation;
    rotation_out->vx += player_state.view_rotation_offset.vx;
    rotation_out->vy += player_state.view_rotation_offset.vy;
    rotation_out->vz += player_state.view_rotation_offset.vz;
}
