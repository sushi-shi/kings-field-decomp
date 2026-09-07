#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Distances are world units; motion is per action update. */
enum {
    MAP_DOOR_INTERACTION_LOCAL_Z = 550,
    MAP_DROP_TIP_ID_END = 43,
    MAP_DROP_SPIN_ID_END = 48,
    MAP_DROP_BOUNCE_ID_END = 65,
    MAP_DROP_RANDOM_YAW_SHIFT = 3,
    MAP_GOLD_DROP_SCATTER_RADIUS = 600,
    MAP_GOLD_DROP_INITIAL_VELOCITY_Y = -120,
    MAP_DOOR_HOLD_FIRST = 250,
    MAP_DOOR_CLOSE_FIRST = 300,
    MAP_SWING_DOOR_OPEN_UPDATES = 32,
    MAP_SWING_DOOR_YAW_STEP = KF_ANGLE_QUARTER_TURN / MAP_SWING_DOOR_OPEN_UPDATES,
    MAP_SWING_DOOR_CLOSE_END = MAP_DOOR_CLOSE_FIRST + MAP_SWING_DOOR_OPEN_UPDATES,
    MAP_LIFT_DOOR_OPEN_UPDATES = 41,
    MAP_LIFT_DOOR_Y_STEP = 60,
    MAP_LIFT_DOOR_CLOSE_END = MAP_DOOR_CLOSE_FIRST + MAP_LIFT_DOOR_OPEN_UPDATES,
    MAP_DROP_TIP_GRAVITY = 20,
    MAP_DROP_TIP_INITIAL_ANGULAR_VELOCITY = 16,
    MAP_DROP_TIP_ANGULAR_ACCELERATION = 16,
    MAP_DROP_SPIN_Y_STEP = 20,
    MAP_DROP_SPIN_YAW_STEP = 256,
    MAP_DROP_BOUNCE_PITCH_STEP = 160,
    MAP_DROP_BOUNCE_GRAVITY = 30,
    MAP_DROP_BOUNCE_STOP_VELOCITY = 120,
    MAP_EMITTER_PLAYER_RANGE = 30000,
    MAP_EMITTER_COUNTDOWN_RANDOM_SHIFT = 12,
    MAP_EMITTER_COUNTDOWN_BASE = 10,
    MAP_BOSS_EMITTER_COUNTDOWN_BASE = 20,
    MAP_EMITTER_VELOCITY_SHIFT = 10,
    MAP_FIRE_BALL_EMITTER_VELOCITY_SHIFT = 7,
    MAP_FIRE_BALL_EMITTER_Y_OFFSET = -1400,
    MAP_WIND_CUTTER_EMITTER_Y_OFFSET = 600,
    MAP_BOSS_EMITTER_X_OFFSET = 1100,
    MAP_BOSS_EMITTER_Z_OFFSET = 1000,
    MAP_BOSS_EMITTER_Y_OFFSET = -1000,
    MAP_BOSS_EMITTER_SOUND_RANDOM_LIMIT = (RAND_MAX + 1) / 8,
    MAP_EFFECT_SWITCH_PHASE_STEP = 128,
    MAP_REVEAL_SETTLE_END = 6,
    MAP_REVEAL_LIFT = KF_MAP_OBJECT_REVEAL_DEPTH
        + (MAP_REVEAL_SETTLE_END - 1) * KF_MAP_OBJECT_REVEAL_SETTLE_STEP,
    MAP_RESTORE_POINT_YAW_STEP = 8
};

/* Preserve unsigned multiply/shift before narrowing to the direction halfword. */
#define MAP_EMITTER_VELOCITY_NUMERATOR 175u
#define MAP_FIRE_BALL_EMITTER_VELOCITY_NUMERATOR 25u
#define MAP_BOSS_EMITTER_VELOCITY_NUMERATOR 225u

RODATA(0x80012888, 0x18c)

/*
 * Finds the first object from START_INDEX within its padded interaction
 * radius. Hinged-door leaves transform the query point by their respective
 * rotated local offsets before testing it against the object's position.
 */
ADDRESS(0x800315c4, 0x1c0)
s32 map_object_pool_find_interaction_from(s32 start_index, s32 x, s32 z, s32 extra_radius)
{
    KfMapObject *object = &map_object_state.objects[start_index];
    s16 index = start_index;
    KfMapObjectDefinition *definition;
    SVECTOR offset;
    VECTOR point;
    MATRIX matrix;

    for (; index < KF_MAP_OBJECT_CAPACITY; index++, object++) {
        if (object->object_id == KF_MAP_OBJECT_FREE) {
            continue;
        }
        definition = &map_object_state.definitions[object->object_id];
        if (definition->behavior_type == KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR) {
            offset.vx = -KF_MAP_TILE_SIZE;
            offset.vy = 0;
            offset.vz = MAP_DOOR_INTERACTION_LOCAL_Z;
            matrix_set_rotation_y(object->rotation.y, &matrix);
            ApplyMatrix(&matrix, &offset, &point);
            point.vx += x;
            point.vz += z;
            if (map_object_distance_to_point(
                    object, point.vx, point.vz, definition->interaction_radius + extra_radius)
                != -1) {
                return index;
            }
        } else if (definition->behavior_type == KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_PARTNER) {
            offset.vx = KF_MAP_TILE_SIZE;
            offset.vy = 0;
            offset.vz = MAP_DOOR_INTERACTION_LOCAL_Z;
            matrix_set_rotation_y(object->rotation.y, &matrix);
            ApplyMatrix(&matrix, &offset, &point);
            point.vx += x;
            point.vz += z;
            if (map_object_distance_to_point(
                    object, point.vx, point.vz, definition->interaction_radius + extra_radius)
                != -1) {
                return index;
            }
        } else if (map_object_distance_to_point(
                       object, x, z, definition->interaction_radius + extra_radius)
                   != -1) {
            return index;
        }
    }
    return -1;
}

ADDRESS(0x80031784, 0x20)
void map_object_start_action_if_idle(KfMapObject *object, u8 action)
{
    if (object->action == KF_MAP_OBJECT_ACTION_IDLE) {
        object->action = action;
        object->action_timer = 0;
    }
}

ADDRESS(0x800317a4, 0x90)
KfMapObject *map_object_effect_pool_acquire(u16 first_index, u16 count, u16 sequence)
{
    KfMapObject *object = &map_object_state.objects[first_index];
    KfMapObject *oldest = 0;
    s32 oldest_age = 0;
    s32 age;

    do {
        if (object->object_id == KF_MAP_OBJECT_FREE) {
            return object;
        }
        age = sequence - object->link.spawn_sequence;
        if (age < 0) {
            age += 0x10000;
        }
        if (oldest_age < age) {
            oldest = object;
            oldest_age = age;
        }
        object++;
    } while (--count != 0);
    return oldest;
}

/*
 * Spawns an effect object of OBJECT_ID at POSITION (raised by Y_OFFSET) in
 * the 170.. or 180.. effect range, then starts its action by id band.
 */
ADDRESS(0x80031834, 0x194)
void map_object_spawn_effect(u8 kind, u8 object_id, const struct KfVec3i *position, s32 y_offset)
{
    u16 *sequence;
    u16 first_index;
    KfMapObject *object;

    if (kind == KF_MAP_OBJECT_DROP_FROM_PLACEMENT) {
        sequence = &map_object_effect_sequence_180;
        first_index = KF_MAP_OBJECT_PLACEMENT_DROP_FIRST;
    } else {
        first_index = KF_MAP_OBJECT_DEFINITION_DROP_FIRST;
        sequence = &map_object_effect_sequence_170;
    }
    object = map_object_effect_pool_acquire(first_index, KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, *sequence);
    object->link.spawn_sequence = (*sequence)++;
    object->object_id = object_id;
    object->position_x = position->x;
    object->position_y = y_offset + position->y;
    object->position_z = position->z;
    object->cell_x = object->position_x / KF_MAP_TILE_SIZE;
    object->cell_z = object->position_z / KF_MAP_TILE_SIZE;
    object->rotation.z = 0;
    object->rotation.x = 0;
    object->rotation.y = rand() >> MAP_DROP_RANDOM_YAW_SHIFT;
    object->action = KF_MAP_OBJECT_ACTION_IDLE;
    if (object_id < MAP_DROP_TIP_ID_END) {
        map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_FALL_AND_TIP);
        object->link.vertical_velocity = 0;
    } else if (object_id < MAP_DROP_SPIN_ID_END) {
        map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_FALL_AND_SPIN);
        object->link.vertical_velocity = 0;
    } else if (object_id < MAP_DROP_BOUNCE_ID_END) {
        map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_BOUNCE);
        object->link.vertical_velocity = 0;
    }
}

/* Spawns debris object 39 for SOURCE at a random bearing 600 units from POSITION. */
ADDRESS(0x800319c8, 0x18c)
void map_object_spawn_actor_debris(u16 source, const struct KfVec3i *position, s32 y_offset)
{
    KfMapObject *object;
    u16 *sequence;
    u16 angle;

    sequence = &map_object_effect_sequence_160;
    object = map_object_effect_pool_acquire(KF_MAP_OBJECT_GOLD_DROP_FIRST, KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY, *sequence);
    object->link.spawn_sequence = (*sequence)++;
    object->object_id = 39;
    /* The debris keeps its source in the link id and action parameter bytes. */
    *(u16 *)&object->link.link_id = source;
    angle = (u32)rand() >> MAP_DROP_RANDOM_YAW_SHIFT;
    object->position_x = ((rsin(angle) * MAP_GOLD_DROP_SCATTER_RADIUS) >> KF_FIXED12_BITS) + position->x;
    object->position_y = y_offset + position->y;
    object->position_z = ((rcos(angle) * MAP_GOLD_DROP_SCATTER_RADIUS) >> KF_FIXED12_BITS) + position->z;
    object->cell_x = object->position_x / KF_MAP_TILE_SIZE;
    object->cell_z = object->position_z / KF_MAP_TILE_SIZE;
    object->rotation.z = 0;
    object->rotation.x = 0;
    object->rotation.y = rand() >> MAP_DROP_RANDOM_YAW_SHIFT;
    object->action = KF_MAP_OBJECT_ACTION_IDLE;
    map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_BOUNCE);
    object->link.vertical_velocity = MAP_GOLD_DROP_INITIAL_VELOCITY_Y;
}

/*
 * Fires LINK_ID: objects in action 11 or 80..82 linked to it drop the link,
 * and idle objects of behaviour types below 8 with that (128..) link start.
 */
ADDRESS(0x80031b54, 0xf0)
void map_object_pool_trigger_link(u8 link_id)
{
    KfMapObject *object = map_object_state.objects;
    u16 count = KF_MAP_OBJECT_CAPACITY - 1;

    do {
        switch (object->action) {
        case KF_MAP_OBJECT_ACTION_ENABLE_RESTORE_POINT:
        case KF_MAP_OBJECT_ACTION_PROJECTILE_EMITTER:
        case KF_MAP_OBJECT_ACTION_RELEASE_ORBIT_OR_SHORT_SWING:
        case KF_MAP_OBJECT_ACTION_RELEASE_LONG_SWING:
            if (object->link.link_id == link_id) {
                object->link.link_id = KF_MAP_LINK_NONE;
            }
            break;
        default:
            if (map_object_state.definitions[object->object_id].behavior_type < KF_MAP_OBJECT_BEHAVIOR_HINGED_CONTAINER
                && !(object->link.link_id < KF_MAP_LINK_REUSABLE_FIRST) && object->link.link_id == link_id) {
                map_object_start_action_if_idle(
                    object, map_object_state.definitions[object->object_id].behavior_type);
            }
            break;
        }
        object++;
    } while (count-- != 0);
}

/* Drops LINK_ID from every behaviour-type-0..8 object that carries it. */
ADDRESS(0x80031c44, 0x84)
void map_object_pool_clear_link(u8 link_id)
{
    KfMapObject *object = map_object_state.objects;
    u16 count = KF_MAP_OBJECT_CAPACITY - 1;
    KfMapObjectDefinition *definitions = map_object_state.definitions;

    do {
        if ((definitions[object->object_id].behavior_type < KF_MAP_OBJECT_BEHAVIOR_HINGED_CONTAINER
             || definitions[object->object_id].behavior_type == KF_MAP_OBJECT_BEHAVIOR_HINGED_CONTAINER)
            && object->link.link_id == link_id) {
            object->link.link_id = KF_MAP_LINK_NONE;
        }
        object++;
    } while (count-- != 0);
}

/*
 * Advances every map object's action: swinging and sliding doors, falling
 * and tipping debris, effect emitters keyed on the object id, levers that
 * sweep an effect record's angle and fire their link, and the floor
 * specific stair and bridge events.
 */
ADDRESS(0x80031cc8, 0xc18)
void map_object_pool_update(void)
{
    KfMapObject *object = map_object_state.objects;
    KfMapObject *pair;
    KfEffectRecord *record;
    const SoundRef *sound;
    u8 *counter;
    struct KfVec3s direction;
    struct KfVec3i point;
    s16 count;
    u16 timer;
    u16 elapsed;
    s32 floor;
    s32 tilt;

    for (count = KF_MAP_OBJECT_CAPACITY; count != 0; object++, count--) {
        if (object->action == KF_MAP_OBJECT_ACTION_IDLE) {
            continue;
        }
        switch (object->action) {
        case KF_MAP_OBJECT_ACTION_SWING_DOOR:
            timer = object->action_timer;
            if (object->link.action_parameter != KF_MAP_OBJECT_PARAMETER_NONE) {
                pair = &map_object_state.objects[object->link.action_parameter];
            } else {
                pair = 0;
            }
            object->action_timer++;
            if (timer < MAP_SWING_DOOR_OPEN_UPDATES) {
                object->rotation.y += MAP_SWING_DOOR_YAW_STEP;
                if (pair != 0) {
                    pair->rotation.y -= MAP_SWING_DOOR_YAW_STEP;
                }
                if (timer == 0) {
                    if (object->object_id == 0x77) {
                        sound = &gameplay_sound_ref_1;
                    } else {
                        sound = &gameplay_sound_ref_7;
                    }
                    audio_play_spatial_default_range(
                        sound, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                if (timer == MAP_SWING_DOOR_OPEN_UPDATES - 1) {
                    map_object_mark_collision_edge(object, KF_MAP_CELL_FLOOR, object->rotation.y - KF_ANGLE_QUARTER_TURN);
                    object->action_timer = MAP_DOOR_HOLD_FIRST;
                }
            } else if (timer >= MAP_DOOR_CLOSE_FIRST) {
                if (timer >= MAP_SWING_DOOR_CLOSE_END) {
                    goto finish;
                }
                if (timer == MAP_DOOR_CLOSE_FIRST) {
                    if (map_object_probe_forward(object, object->rotation.y - KF_ANGLE_QUARTER_TURN) != -1) {
                        object->action_timer = MAP_DOOR_CLOSE_FIRST;
                        break;
                    }
                    map_object_mark_collision_edge(object, KF_MAP_CELL_BLOCKED, object->rotation.y - KF_ANGLE_QUARTER_TURN);
                    if (object->object_id == 0x77) {
                        sound = &gameplay_sound_ref_1;
                    } else {
                        sound = &gameplay_sound_ref_7;
                    }
                    audio_play_spatial_default_range(
                        sound, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                object->rotation.y -= MAP_SWING_DOOR_YAW_STEP;
                if (pair != 0) {
                    pair->rotation.y += MAP_SWING_DOOR_YAW_STEP;
                }
            }
            break;
        case KF_MAP_OBJECT_ACTION_LIFT_DOOR:
            elapsed = object->action_timer++;
            if (elapsed < MAP_LIFT_DOOR_OPEN_UPDATES) {
                object->position_y -= MAP_LIFT_DOOR_Y_STEP;
                if (elapsed == 0) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_0, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                if (elapsed == MAP_LIFT_DOOR_OPEN_UPDATES - 1) {
                    map_object_mark_collision_edge(object, KF_MAP_CELL_FLOOR, object->rotation.y);
                    object->action_timer = MAP_DOOR_HOLD_FIRST;
                }
            } else if (elapsed >= MAP_DOOR_CLOSE_FIRST) {
                if (elapsed >= MAP_LIFT_DOOR_CLOSE_END) {
                    goto finish;
                }
                if (elapsed == MAP_DOOR_CLOSE_FIRST) {
                    if (map_object_probe_forward(object, object->rotation.y) != -1) {
                        object->action_timer = MAP_DOOR_CLOSE_FIRST;
                        break;
                    }
                    map_object_mark_collision_edge(object, KF_MAP_CELL_BLOCKED, object->rotation.y);
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_0, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                object->position_y += MAP_LIFT_DOOR_Y_STEP;
            }
            break;
        case KF_MAP_OBJECT_ACTION_FALL_AND_TIP:
            if (object->action_timer == 0) {
                s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

                object->position_y += object->link.vertical_velocity;
                object->link.vertical_velocity += MAP_DROP_TIP_GRAVITY;
                if (object->position_y < -(attribute * KF_MAP_HEIGHT_STEP)) {
                    break;
                }
                object->position_y = -(attribute * KF_MAP_HEIGHT_STEP);
                object->link.vertical_velocity = MAP_DROP_TIP_INITIAL_ANGULAR_VELOCITY;
                object->action_timer = 1;
            } else {
                object->rotation.x += object->link.vertical_velocity;
                object->link.vertical_velocity += MAP_DROP_TIP_ANGULAR_ACCELERATION;
                if (object->rotation.x >= KF_ANGLE_QUARTER_TURN) {
                    object->rotation.x = KF_ANGLE_QUARTER_TURN;
                finish:
                    object->action = KF_MAP_OBJECT_ACTION_IDLE;
                }
            }
            break;
        case KF_MAP_OBJECT_ACTION_FALL_AND_SPIN: {
            s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

            object->position_y += MAP_DROP_SPIN_Y_STEP;
            object->rotation.y = (object->rotation.y + MAP_DROP_SPIN_YAW_STEP) & KF_ANGLE_WRAP_MASK;
            if (object->position_y < -(attribute * KF_MAP_HEIGHT_STEP)) {
                break;
            }
            object->position_y = -(attribute * KF_MAP_HEIGHT_STEP);
            object->action_timer = 1;
            goto finish;
        }
        case KF_MAP_OBJECT_ACTION_BOUNCE: {
            s32 attribute = map_floor_height_grid[object->cell_z][object->cell_x];

            object->position_y += object->link.vertical_velocity;
            floor = -(attribute * KF_MAP_HEIGHT_STEP);
            tilt = object->rotation.x;
            if (object->action_timer == 0) {
                object->rotation.x = (tilt + MAP_DROP_BOUNCE_PITCH_STEP) & KF_ANGLE_WRAP_MASK;
            } else {
                object->rotation.x = (tilt - MAP_DROP_BOUNCE_PITCH_STEP) & KF_ANGLE_WRAP_MASK;
            }
            object->link.vertical_velocity += MAP_DROP_BOUNCE_GRAVITY;
            if (object->position_y < floor) {
                break;
            }
            object->position_y = floor;
            if (object->link.vertical_velocity < MAP_DROP_BOUNCE_STOP_VELOCITY) {
                object->rotation.x = 0;
                goto finish;
            }
            object->link.vertical_velocity = -(object->link.vertical_velocity >> 1);
            object->action_timer = object->action_timer == 0;
            break;
        }
        case KF_MAP_OBJECT_ACTION_PROJECTILE_EMITTER:
            if (object->link.link_id == KF_MAP_LINK_NONE) {
                break;
            }
            if (object->action_timer == 0) {
                if (player_distance_to_point(
                        object->position_x, KF_COLLISION_IGNORE_HEIGHT, object->position_z, MAP_EMITTER_PLAYER_RANGE, 0)
                    == -1) {
                    break;
                }
                switch (object->object_id) {
            case 137:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * MAP_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                direction.z = (-rcos(object->rotation.y) * MAP_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                effect_pool_construct(
                    *(u8 *)&object->link.spawn_sequence,
                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    0xe,
                    &object->position_x,
                    &direction,
                    &object->rotation);
                object->action_timer = (rand() >> MAP_EMITTER_COUNTDOWN_RANDOM_SHIFT) + MAP_EMITTER_COUNTDOWN_BASE;
                break;
            case 124:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * MAP_FIRE_BALL_EMITTER_VELOCITY_NUMERATOR) >> MAP_FIRE_BALL_EMITTER_VELOCITY_SHIFT;
                direction.z = (-rcos(object->rotation.y) * MAP_FIRE_BALL_EMITTER_VELOCITY_NUMERATOR) >> MAP_FIRE_BALL_EMITTER_VELOCITY_SHIFT;
                point.x = object->position_x;
                point.z = object->position_z;
                point.y = object->position_y + MAP_FIRE_BALL_EMITTER_Y_OFFSET;
                effect_pool_construct(
                    *(u8 *)&object->link.spawn_sequence,
                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    KF_EFFECT_KIND_FIRE_BALL,
                    &point,
                    &direction,
                    &object->rotation);
                object->action_timer = (rand() >> MAP_EMITTER_COUNTDOWN_RANDOM_SHIFT) + MAP_EMITTER_COUNTDOWN_BASE;
                break;
            case 125:
                direction.y = 0;
                direction.x = (rsin(object->rotation.y) * MAP_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                direction.z = (-rcos(object->rotation.y) * MAP_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                point.x = object->position_x;
                point.z = object->position_z;
                point.y = object->position_y + MAP_WIND_CUTTER_EMITTER_Y_OFFSET;
                effect_pool_construct(
                    *(u8 *)&object->link.spawn_sequence,
                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    KF_EFFECT_KIND_WIND_CUTTER,
                    &point,
                    &direction,
                    &object->rotation,
                    1);
                object->action_timer = (rand() >> MAP_EMITTER_COUNTDOWN_RANDOM_SHIFT) + MAP_EMITTER_COUNTDOWN_BASE;
                break;
            case 115:
                if (map_floor5_script.boss_encounter_started == KF_MAP_SCRIPT_UNSET) {
                    break;
                }
                direction.y = 0;
                direction.x = (rsin(object->rotation.y + KF_ANGLE_QUARTER_TURN) * MAP_BOSS_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                direction.z = (-rcos(object->rotation.y + KF_ANGLE_QUARTER_TURN) * MAP_BOSS_EMITTER_VELOCITY_NUMERATOR) >> MAP_EMITTER_VELOCITY_SHIFT;
                switch (object->rotation.y) {
                case 0:
                    point.x = object->position_x + MAP_BOSS_EMITTER_X_OFFSET;
                    point.z = object->position_z + MAP_BOSS_EMITTER_Z_OFFSET;
                    break;
                case KF_ANGLE_HALF_TURN:
                    point.x = object->position_x - MAP_BOSS_EMITTER_X_OFFSET;
                    point.z = object->position_z - MAP_BOSS_EMITTER_Z_OFFSET;
                    break;
                }
                point.y = object->position_y + MAP_BOSS_EMITTER_Y_OFFSET;
                effect_pool_construct(
                    *(u8 *)&object->link.spawn_sequence,
                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    KF_EFFECT_KIND_WIND_CUTTER,
                    &point,
                    &direction,
                    &object->rotation,
                    rand() < MAP_BOSS_EMITTER_SOUND_RANDOM_LIMIT);
                object->action_timer = (rand() >> MAP_EMITTER_COUNTDOWN_RANDOM_SHIFT) + MAP_BOSS_EMITTER_COUNTDOWN_BASE;
                break;
            }
            } else {
                object->action_timer--;
            }
            break;
        case KF_MAP_OBJECT_ACTION_RELEASE_ORBIT_OR_SHORT_SWING:
        case KF_MAP_OBJECT_ACTION_RELEASE_LONG_SWING:
            if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {
                effect_pool_records[object->link.action_parameter].phase = KF_EFFECT_HAZARD_RELEASE_REQUEST;
                object->action_timer = 1;
            }
            break;
        case KF_MAP_OBJECT_ACTION_EFFECT_SWITCH:
            if (object->action_timer == KF_MAP_OBJECT_SWITCH_DISABLED) {
                break;
            }
            if (object->link.link_id == KF_MAP_LINK_NONE) {
                effect_pool_records[object->link.action_parameter].visual.animation_phase = (KF_FIXED12_ONE - 1);
                object->action_timer = KF_MAP_OBJECT_SWITCH_DISABLED;
                break;
            }
            if (object->action_timer == KF_MAP_OBJECT_SWITCH_FORWARD) {
                record = &effect_pool_records[object->link.action_parameter];
                if (record->visual.animation_phase == 0) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_3, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                record->visual.animation_phase += MAP_EFFECT_SWITCH_PHASE_STEP;
                if (record->visual.animation_phase >= KF_FIXED12_ONE) {
                    record->visual.animation_phase = (KF_FIXED12_ONE - 1);
                    map_object_pool_trigger_link(object->link.link_id);
                    if (object->link.link_id >= KF_MAP_LINK_REUSABLE_FIRST) {
                        object->action_timer = KF_MAP_OBJECT_SWITCH_REVERSE;
                    } else {
                        object->link.link_id = KF_MAP_LINK_NONE;
                    }
                }
            } else if (object->action_timer == KF_MAP_OBJECT_SWITCH_REVERSE) {
                record = &effect_pool_records[object->link.action_parameter];
                if (record->visual.animation_phase == (KF_FIXED12_ONE - 1)) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_3, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                }
                record->visual.animation_phase -= MAP_EFFECT_SWITCH_PHASE_STEP;
                if (record->visual.animation_phase > KF_FIXED12_ONE) {
                    record->visual.animation_phase = 0;
                    object->action_timer = KF_MAP_OBJECT_SWITCH_READY;
                }
            }
            break;
        case KF_MAP_OBJECT_ACTION_COPY_REGION:
            if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {
                if (object->link.action_parameter != KF_MAP_OBJECT_PARAMETER_NONE) {
                    map_apply_copy_region(object->link.action_parameter);
                }
                object->action_timer = 1;
            }
            break;
        case KF_MAP_OBJECT_ACTION_REVEAL_MAP_PIECE:
            if (object->link.link_id != KF_MAP_LINK_NONE) {
                break;
            }
            if (object->action_timer == 0) {
                object->position_y -= MAP_REVEAL_LIFT;
                object->action_timer = 1;
            } else if (object->action_timer < MAP_REVEAL_SETTLE_END) {
                object->position_y += KF_MAP_OBJECT_REVEAL_SETTLE_STEP;
                object->action_timer++;
            } else if (object->action_timer == MAP_REVEAL_SETTLE_END) {
                if (player_state.progress_state.current_floor == 3) {
                    audio_play_spatial_default_range(
                        &gameplay_sound_ref_11, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                    counter = &map_floor3_script.revealed_piece_count;
                    if (*counter != KF_MAP_FLOOR3_REQUIRED_REVEALS) {
                        (*counter)++;
                        if (*counter >= KF_MAP_FLOOR3_REQUIRED_REVEALS) {
                            map_apply_copy_region(2);
                            map_apply_copy_region(3);
                            sound_ref_play(&gameplay_sound_ref_7, KF_AUDIO_MAX_VOLUME);
                            *counter = KF_MAP_FLOOR3_REQUIRED_REVEALS;
                        }
                    }
                } else if (player_state.progress_state.current_floor == 1) {
                    if (map_floor1_script.revival_enabled == KF_MAP_SCRIPT_UNSET) {
                        audio_play_spatial_default_range(
                            &gameplay_sound_ref_5, (VECTOR *)&object->position_x, KF_AUDIO_MAX_VOLUME);
                        map_floor1_script.revival_enabled = KF_MAP_SCRIPT_SET;
                    }
                }
                object->action_timer++;
            }
            break;
        case KF_MAP_OBJECT_ACTION_ENABLE_RESTORE_POINT:
            if (object->link.link_id == KF_MAP_LINK_NONE) {
                object->object_id = 0x7b;
                object->rotation.y = (object->rotation.y + MAP_RESTORE_POINT_YAW_STEP) & KF_ANGLE_WRAP_MASK;
            }
            break;
        }
    }
}
