#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_player.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Motion rates count executions of the player update, not elapsed seconds. */
enum {
    PLAYER_FATAL_DROP_DISTANCE = 3000,
    PLAYER_FALL_ACCELERATION = 40,
    PLAYER_FALL_LANDING_OVERSHOOT = 100,
    PLAYER_STEP_UP_ACCELERATION = 5,
    PLAYER_FAST_STEP_MIN_SPEED = 181,
    PLAYER_FAST_STEP_UP_VELOCITY = -300,
    PLAYER_SLOW_STEP_UP_VELOCITY = -100,
    PLAYER_BOB_PHASE_PER_SPEED = 2,
    PLAYER_BOB_SINE_DOWNSHIFT = 6
};

enum {
    PLAYER_WEAPON_CHARGE_DELAY_UPDATES = 10,
    PLAYER_COLICHEMARDE_HIT_PHASE = 1000,
    PLAYER_WEAPON_HIT_PHASE = 3072,
    PLAYER_WEAPON_HIT_WINDOW = 300,
    PLAYER_WEAPON_HIT_Y_OFFSET = 1000,
    PLAYER_WEAPON_HIT_RADIUS = 800,
    PLAYER_WEAPON_HIT_HEIGHT = 1000,
    PLAYER_COLLISION_SLIDE_CLEARANCE = 100,
    PLAYER_COLLISION_DEFLECTION_ANGLE = 64,
    PLAYER_MAX_STEP_RISE = 699,
    PLAYER_DIAGONAL_COMPONENT_Q12 = 2896
};

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
KfFloorEntryCell floor_entry_cells[KF_PLAYER_FLOOR_ENTRY_COUNT] = {
    {15, 2}, {29, 56}, {28, 18}, {7, 22}, {39, 69}
};

/* Stores the item id into one of the six armor slots, re-resolves the five armor record pointers, and recalculates. */
ADDRESS(0x80016848, 0x1e8)
void player_set_equipment_slot(u8 item_id, KfEquipmentSlot slot)
{
    switch (slot) {
    case KF_EQUIPMENT_SLOT_HEAD:
        player_state.equipped_head_armor_id = item_id;
        break;
    case KF_EQUIPMENT_SLOT_BODY:
        player_state.equipped_body_armor_id = item_id;
        break;
    case KF_EQUIPMENT_SLOT_ARM:
        player_state.equipped_arm_armor_id = item_id;
        break;
    case KF_EQUIPMENT_SLOT_LEG:
        player_state.equipped_leg_armor_id = item_id;
        break;
    case KF_EQUIPMENT_SLOT_SHIELD:
        player_state.equipped_shield_id = item_id;
        break;
    case KF_EQUIPMENT_SLOT_ACCESSORY:
        player_state.equipped_accessory_id = item_id;
        break;
    }
    if (player_state.equipped_head_armor_id != KF_ITEM_NONE) {
        player_state.equipped_head_armor_record = &armor_records[player_state.equipped_head_armor_id - KF_ARMOR_ITEM_FIRST];
    } else {
        player_state.equipped_head_armor_record = 0;
    }
    if (player_state.equipped_body_armor_id != KF_ITEM_NONE) {
        player_state.equipped_body_armor_record =
            &armor_records[player_state.equipped_body_armor_id - KF_ARMOR_ITEM_FIRST];
    } else {
        player_state.equipped_body_armor_record = 0;
    }
    if (player_state.equipped_arm_armor_id != KF_ITEM_NONE) {
        player_state.equipped_arm_armor_record =
            &armor_records[player_state.equipped_arm_armor_id - KF_ARMOR_ITEM_FIRST];
    } else {
        player_state.equipped_arm_armor_record = 0;
    }
    if (player_state.equipped_leg_armor_id != KF_ITEM_NONE) {
        player_state.equipped_leg_armor_record =
            &armor_records[player_state.equipped_leg_armor_id - KF_ARMOR_ITEM_FIRST];
    } else {
        player_state.equipped_leg_armor_record = 0;
    }
    if (player_state.equipped_shield_id != KF_ITEM_NONE) {
        player_state.equipped_shield_record =
            &armor_records[player_state.equipped_shield_id - KF_ARMOR_ITEM_FIRST];
    } else {
        player_state.equipped_shield_record = 0;
    }
    player_recalculate_combat_stats();
}

/* weapon_image_path_template has decimal weapon-id digits at [9] and [10]. */
ADDRESS(0x80016a30, 0xf4)
void player_equip_weapon(u8 weapon_id)
{
    player_state.attack_charge_state.current = 0;
    player_state.attack_charge_state.committed = 0;
    player_state.weapon_charge_delay = PLAYER_WEAPON_CHARGE_DELAY_UPDATES;
    player_state.equipped_weapon_id = weapon_id;
    if (weapon_id != KF_ITEM_NONE) {
        player_state.equipped_weapon_record = &weapon_records[weapon_id];
        weapon_image_path_template[9] = '0' + weapon_id / 10;
        weapon_image_path_template[10] = '0' + weapon_id % 10;
        if (cd_file_load_into(player_state.weapon_asset_buffer, weapon_image_path_template) != 0) {
            exit(1);
        }
        asset_registry_set(KF_ASSET_WEAPON, player_state.weapon_asset_buffer);
    }
    player_state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
    player_state.weapon_animation_cache = 0;
    player_recalculate_combat_stats();
}

ADDRESS(0x80016b24, 0x9c)
void player_begin_weapon_attack(void)
{
    if (player_state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE
        && player_state.equipped_weapon_id != KF_ITEM_NONE) {
        player_state.weapon_attack_phase = 0;
        sound_ref_play(&player_sound_refs[KF_PLAYER_SOUND_WEAPON_ATTACK], KF_AUDIO_MAX_VOLUME);
        player_state.attack_charge_state.committed = player_state.attack_charge_state.current;
        if (player_state.attack_charge_state.current == KF_PLAYER_CHARGE_FULL) {
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

    if (player_state.equipped_weapon_id == KF_ITEM_NONE) {
        return;
    }
    if (player_state.weapon_attack_phase != KF_WEAPON_ATTACK_INACTIVE) {
        player_state.weapon_attack_phase += KF_WEAPON_ATTACK_PHASE_STEP;
        window = player_state.weapon_attack_phase;
        if (player_state.equipped_weapon_id == KF_ITEM_COLICHEMARDE
                ? (u16)(window - PLAYER_COLICHEMARDE_HIT_PHASE) < PLAYER_WEAPON_HIT_WINDOW
                : (u16)(window - PLAYER_WEAPON_HIT_PHASE) < PLAYER_WEAPON_HIT_WINDOW) {
            offset.vx = 0;
            offset.vy = PLAYER_WEAPON_HIT_Y_OFFSET;
            offset.vz = player_state.equipped_weapon_record->attack_z_offset;
            rotation.vx = 0;
            rotation.vy = -player_state.camera_rotation.vy;
            rotation.vz = 0;
            RotMatrix(&rotation, &matrix);
            ApplyMatrix(&matrix, &offset, &result);
            result.vx += player_state.camera_position.vx;
            result.vy += player_state.camera_position.vy;
            result.vz += player_state.camera_position.vz;
            actor = actor_pool_find_overlap(result.vx, result.vy, result.vz,
                PLAYER_WEAPON_HIT_RADIUS, PLAYER_WEAPON_HIT_HEIGHT);
            if (actor != -1) {
                actor_apply_damage(
                    actor,
                    player_state.physical_power,
                    player_state.cutting_attack,
                    player_state.striking_attack,
                    player_state.piercing_attack,
                    player_state.holy_attack,
                    player_state.fire_attack,
                    player_state.attack_charge_state.committed,
                    KF_ACTOR_DAMAGE_CREDIT_PLAYER);
            }
            player_state.attack_charge_state.committed = 0;
            player_state.attack_charge_state.current = 0;
            player_state.weapon_charge_delay = PLAYER_WEAPON_CHARGE_DELAY_UPDATES;
        }
        if (player_state.weapon_attack_phase >= KF_WEAPON_ATTACK_PHASE_END) {
            player_state.weapon_attack_phase = KF_WEAPON_ATTACK_INACTIVE;
        }
    } else if (player_state.weapon_charge_delay == 0) {
        player_state.attack_charge_state.current +=
            fixed6_ratio_step(
                player_state.physical_power,
                player_state.equipped_weapon_record->charge_rate) * KF_PLAYER_CHARGE_GAIN_MULTIPLIER;
        if (player_state.attack_charge_state.current >= KF_PLAYER_CHARGE_FULL + 1) {
            player_state.attack_charge_state.current = KF_PLAYER_CHARGE_FULL;
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
    player_state.weapon_asset_buffer = (KfAssetHeader *)memory_allocate(KF_WEAPON_ASSET_BUFFER_BYTES);
    game_state_initialize();
    player_state.update_state = KF_PLAYER_UPDATE_NORMAL;
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
    view_offset = player_state.view_bob_offset - KF_PLAYER_CAMERA_HEIGHT;
    player_state.floor_height = floor_height;
    player_state.camera_position.vy = view_offset + floor_height;
    player_clear_motion();
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);
    hud_brightness = KF_HUD_DEFAULT_BRIGHTNESS;
    player_state.vertical_state = KF_PLAYER_VERTICAL_GROUNDED;
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

    distance = player_distance_to_point(point->x, KF_COLLISION_IGNORE_HEIGHT, point->z, max_distance, 0);
    if (distance != KF_COLLISION_NONE) {
        delta = (vector_xz_to_angle(
                     player_state.camera_position.vx - point->x,
                     point->z - player_state.camera_position.vz)
                 - facing) & KF_ANGLE_WRAP_MASK;
        if (delta > KF_ANGLE_HALF_TURN) {
            delta = KF_ANGLE_FULL_TURN - delta;
        }
        if (angle_tolerance < delta) {
            distance = KF_COLLISION_NONE;
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
    dx >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    if (point_y != KF_COLLISION_IGNORE_HEIGHT) {
        point_height >>= 1;
        center = point_y - point_height;
        point_height += KF_COLLISION_PLAYER_HEIGHT / 2;
        center += KF_COLLISION_PLAYER_HEIGHT / 2;
        dy = player_state.floor_height - center;
        if (dy < -point_height || point_height < dy) {
            goto out_of_range;
        }
    }
    dz >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    distance = SquareRoot0(dx * dx + dz * dz) << KF_LENGTH_SQUARE_DOWNSHIFT;
    if (max_distance < distance) {
        goto out_of_range;
    }
    return distance;
out_of_range:
    return KF_COLLISION_NONE;
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
    SVECTOR delta;
    s16 attempt = 1;
    u8 type;

    dz = (rcos(heading) * distance) >> KF_FIXED12_BITS;
    dx = (-rsin(heading) * distance) >> KF_FIXED12_BITS;
    new_z = dz + player_state.camera_position.vz;
    new_x = dx + player_state.camera_position.vx;
    for (;;) {
        if (collision_query_world(new_x, player_state.floor_height, new_z,
                KF_COLLISION_PLAYER_RADIUS, KF_COLLISION_PLAYER_HEIGHT,
                KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_PLAYER | KF_COLLISION_CAPTURE_TARGET)
            == KF_COLLISION_NONE) {
            break;
        }
        /* The bearing inputs use opposite X/Z subtraction directions. */
        delta.vx = collision_target.position.vx - player_state.camera_position.vx;
        delta.vz = player_state.camera_position.vz - collision_target.position.vz;
        angle = vector_xz_to_angle(delta.vx, delta.vz);
        angle = (angle_mod_delta_le_half_turn(heading, angle) == 0
            ? angle + (KF_ANGLE_HALF_TURN + PLAYER_COLLISION_DEFLECTION_ANGLE)
            : angle + (KF_ANGLE_HALF_TURN - PLAYER_COLLISION_DEFLECTION_ANGLE))
            & KF_ANGLE_WRAP_MASK;
        radius = collision_target.radius
            + (KF_COLLISION_PLAYER_RADIUS + PLAYER_COLLISION_SLIDE_CLEARANCE);
        delta.vz = (rcos(angle) * radius) >> KF_FIXED12_BITS;
        delta.vx = (-rsin(angle) * radius) >> KF_FIXED12_BITS;
        attempt--;
        new_z = collision_target.position.vz + delta.vz;
        dz = new_z - player_state.camera_position.vz;
        new_x = collision_target.position.vx + delta.vx;
        dx = new_x - player_state.camera_position.vx;
        if (attempt == -1) {
            return 1;
        }
    }
    cell_z = new_z / KF_MAP_TILE_SIZE;
    if (cell_z < KF_MAP_ROWS && map_collision_grid[cell_z][player_state.map_cell.x] != KF_MAP_CELL_BLOCKED
        && -(map_floor_height_grid[cell_z][player_state.map_cell.x] * KF_MAP_HEIGHT_STEP) - player_state.floor_height
               >= -PLAYER_MAX_STEP_RISE) {
        player_state.camera_position.vz = new_z;
        player_state.map_cell.z = cell_z;
    }
    cell_x = new_x / KF_MAP_TILE_SIZE;
    if (cell_x < KF_MAP_COLUMNS && map_collision_grid[player_state.map_cell.z][cell_x] != KF_MAP_CELL_BLOCKED
        && -(map_floor_height_grid[player_state.map_cell.z][cell_x] * KF_MAP_HEIGHT_STEP) - player_state.floor_height
               >= -PLAYER_MAX_STEP_RISE) {
        player_state.camera_position.vx = new_x;
        player_state.map_cell.x = cell_x;
    }
    type = map_collision_grid[cell_z0][cell_x0];
    if (type >= KF_MAP_CELL_X_GE_Z && type <= KF_MAP_CELL_SUM_GE_SIZE) {
        if (player_state.map_cell.x == cell_x0 && player_state.map_cell.z == cell_z0) {
            remainder_z = player_state.camera_position.vz % KF_MAP_TILE_SIZE;
            remainder_x = player_state.camera_position.vx % KF_MAP_TILE_SIZE;
            if (type == KF_MAP_CELL_X_GE_Z) {
                if (remainder_x < remainder_z) {
                    half = (remainder_z - remainder_x) / 2;
                    player_state.camera_position.vz -= half;
                    player_state.camera_position.vx += half;
                }
            } else if (type == KF_MAP_CELL_SUM_LE_SIZE) {
                if (remainder_z + remainder_x >= KF_MAP_TILE_SIZE + 1) {
                    half = (remainder_z + remainder_x - KF_MAP_TILE_SIZE) / 2;
                    player_state.camera_position.vz -= half;
                    player_state.camera_position.vx -= half;
                }
            } else if (type == KF_MAP_CELL_Z_GE_X) {
                if (remainder_z < remainder_x) {
                    half = (remainder_x - remainder_z) / 2;
                    player_state.camera_position.vz += half;
                    player_state.camera_position.vx -= half;
                }
            } else if (type == KF_MAP_CELL_SUM_GE_SIZE) {
                if (remainder_z + remainder_x < KF_MAP_TILE_SIZE) {
                    half = (KF_MAP_TILE_SIZE - (remainder_z + remainder_x)) / 2;
                    player_state.camera_position.vz += half;
                    player_state.camera_position.vx += half;
                }
            }
            player_state.map_cell.z = player_state.camera_position.vz / KF_MAP_TILE_SIZE;
            player_state.map_cell.x = player_state.camera_position.vx / KF_MAP_TILE_SIZE;
        }
        if (map_collision_grid[cell_z][cell_x] == KF_MAP_CELL_BLOCKED) {
            if (dz < 0) {
                dz = -dz;
            }
            if (dx < 0) {
                dx = -dx;
            }
            type = map_collision_grid[player_state.map_cell.z][player_state.map_cell.x];
            if (type == KF_MAP_CELL_X_GE_Z) {
                if (dz < dx) {
                    dx = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dz = dx;
                } else {
                    dx = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dz = dx;
                }
            } else if (type == KF_MAP_CELL_SUM_LE_SIZE) {
                if (dz < dx) {
                    dz = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dx = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                } else {
                    dz = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dx = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                }
            } else if (type == KF_MAP_CELL_Z_GE_X) {
                if (dx < dz) {
                    dx = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dz = dx;
                } else {
                    dx = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dz = dx;
                }
            } else if (type == KF_MAP_CELL_SUM_GE_SIZE) {
                if (dx < dz) {
                    dz = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dx = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                } else {
                    dz = (distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                    dx = -(distance * PLAYER_DIAGONAL_COMPONENT_Q12) >> KF_FIXED12_BITS;
                }
            }
            new_z = dz + player_state.camera_position.vz;
            cell_z = new_z / KF_MAP_TILE_SIZE;
            new_x = dx + player_state.camera_position.vx;
            cell_x = new_x / KF_MAP_TILE_SIZE;
            if (cell_z < KF_MAP_ROWS && cell_x < KF_MAP_COLUMNS && map_collision_grid[cell_z][cell_x] != KF_MAP_CELL_BLOCKED) {
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

    if (player_state.vertical_state == KF_PLAYER_VERTICAL_GROUNDED) {
        phase = (player_state.view_bob_phase
            + player_state.motion_state.movement_speed * PLAYER_BOB_PHASE_PER_SPEED)
            & KF_ANGLE_WRAP_MASK;
        player_state.view_bob_phase = phase;
        player_state.view_bob_offset = rsin(phase) >> PLAYER_BOB_SINE_DOWNSHIFT;
    }
}


ADDRESS(0x80017a80, 0x278)
void player_update_vertical_motion(void)
{
    s32 target;
    s32 view_offset;

    target = -(map_floor_height_grid[player_state.map_cell.z][player_state.map_cell.x] * KF_MAP_HEIGHT_STEP);
    if (player_state.update_state != KF_PLAYER_UPDATE_DYING) {
        if (player_state.floor_height - target < -PLAYER_FATAL_DROP_DISTANCE) {
            if (player_state.equipped_leg_armor_id == KF_ITEM_FEATHER_BOOTS
                && map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
                    == KF_MAP_ATTRIBUTE_BOTTOMLESS_PIT) {
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
    case KF_PLAYER_VERTICAL_FALLING:
    falling:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += PLAYER_FALL_ACCELERATION;
        if (target + PLAYER_FALL_LANDING_OVERSHOOT < player_state.floor_height) {
            player_state.floor_height = target;
            player_state.vertical_state = KF_PLAYER_VERTICAL_GROUNDED;
        }
        break;
    case KF_PLAYER_VERTICAL_STEP_UP:
    stepping_up:
        player_state.floor_height += player_state.vertical_velocity;
        player_state.vertical_velocity += PLAYER_STEP_UP_ACCELERATION;
        if (player_state.floor_height <= target) {
            player_state.floor_height = target;
            player_state.vertical_state = KF_PLAYER_VERTICAL_GROUNDED;
        }
        break;
    case KF_PLAYER_VERTICAL_GROUNDED:
        if (target < player_state.floor_height) {
            player_state.vertical_state = KF_PLAYER_VERTICAL_STEP_UP;
            if ((s16)player_state.motion_state.movement_speed >= PLAYER_FAST_STEP_MIN_SPEED) {
                player_state.vertical_velocity = PLAYER_FAST_STEP_UP_VELOCITY;
            } else {
                player_state.vertical_velocity = PLAYER_SLOW_STEP_UP_VELOCITY;
            }
            goto stepping_up;
        }
        if (player_state.floor_height < target) {
            player_state.vertical_state = KF_PLAYER_VERTICAL_FALLING;
            player_state.vertical_velocity = 0;
            goto falling;
        }
        break;
    }
done:
    view_offset = player_state.view_bob_offset - KF_PLAYER_CAMERA_HEIGHT;
    player_state.camera_position.vy = view_offset + player_state.floor_height;
}


ADDRESS(0x80017cf8, 0x144)
void player_warp_to_floor_entry(void)
{
    VECTOR position;
    const KfFloorEntryCell *entry;
    KF_ENUM_STORAGE(KfFloorId, u8) floor;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_GROW_REMOVE, &position);
    floor = player_state.progress_state.current_floor;
    entry = &floor_entry_cells[KF_ENUM_ENCODE(u8, floor) - 1];
    player_state.previous_map_cell.x = entry->x;
    player_state.previous_map_cell.z = entry->z;
    player_state.camera_position.vx = player_state.previous_map_cell.x * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vx = player_state.camera_position.vx;
    player_state.camera_position.vz = player_state.previous_map_cell.z * KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER;
    position.vz = player_state.camera_position.vz;
    if (floor == KF_FLOOR_5 && player_state.map_variant != KF_FLOOR5_ENTRY_VARIANT) {
        if (player_state.map_variant == KF_FLOOR5_ALTERNATE_MUSIC_VARIANT) {
            audio_play_current_map_sequence();
        }
        pool_release_all();
        player_state.map_variant = KF_FLOOR5_ENTRY_VARIANT;
        map_variant_assets_load();
    }
    player_sync_position_to_map();
    position.vy = player_state.floor_height;
    player_warp_shimmer(KF_WARP_SHIMMER_SHRINK_REMOVE, &position);
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
