#include <kf/null.h>
#include <kf/bool.h>
#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_map.h>
#include <kf/game_collision.h>
#include <kf/game.h>

enum {
    MAP_EVENT_CONE_INITIAL_BEST_ERROR = 30000
};

ADDRESS(0x8003379c, 0x10)
void map_event_set_current(KfMapEvent *event)
{
    current_map_event = event;
}

ADDRESS(0x800337ac, 0x74)
void map_event_refresh_dialogue_stage(KfMapEvent *event)
{
    if (event->dialogue.fields.stage_limit > event->dialogue.fields.stage) {
        if (KF_ENUM_ENCODE(u8, player_state.progress_state.highest_floor) < event->dialogue.fields.stage_limit) {
            if (event->dialogue.fields.stage != KF_ENUM_ENCODE(u8, player_state.progress_state.highest_floor)) {
                event->dialogue.fields.stage = KF_ENUM_ENCODE(u8, player_state.progress_state.highest_floor);
            reset_dialogue_page:
                event->dialogue.fields.page = KF_DIALOGUE_FIRST_PAGE;
                event->dialogue.fields.page_delay = 0;
            }
        } else if (event->dialogue.fields.stage != event->dialogue.fields.stage_limit) {
            event->dialogue.fields.stage = event->dialogue.fields.stage_limit;
            goto reset_dialogue_page;
        }
    }
}

ADDRESS(0x80033820, 0x98)
void map_event_advance_animation_blocking(KfMapEvent *event, u16 target, s16 step)
{
    while (event->animation_phase < target) {
        event->animation_phase += step;
        render_frame(NULL, NULL);
        frame_pacer_wait();
    }
    event->animation_phase = target;
    render_frame(NULL, NULL);
}

ADDRESS(0x800338b8, 0x22c)
void map_event_pool_load(const KfMapEventDefinition *definitions)
{
    KfBool8 exhausted = KF_FALSE;
    KfMapEvent *event = map_event_pool;
    u16 count = KF_MAP_EVENT_CAPACITY - 1;

    do {
        if (exhausted == KF_TRUE) {
        mark_free:
            event->state = KF_MAP_EVENT_FREE;
        } else {
            event->state = definitions->state;
            if (event->state != KF_MAP_EVENT_FREE) {
                event->character_id = definitions->character_id;
                event->model_index = definitions->model_index;
                event->dialogue_pages = definitions->dialogue_pages;
                event->dialogue.fields.stage_limit = definitions->dialogue_stage_limit;
                event->unknown_0c = definitions->unknown_0b;
                event->unknown_0d = definitions->unknown_0c;
                event->behavior = definitions->behavior;
                event->position_x = definitions->cell_x * KF_MAP_TILE_SIZE + definitions->position_x_offset;
                event->reference_position.vx = event->position_x;
                event->position_z = definitions->cell_z * KF_MAP_TILE_SIZE + definitions->position_z_offset;
                event->reference_position.vz = event->position_z;
                event->cell_x = definitions->cell_x;
                event->cell_z = definitions->cell_z;
                event->radius = definitions->radius;
                event->reference_position.vy =
                    -(map_floor_height_grid.cells[event->cell_z][event->cell_x] * KF_MAP_HEIGHT_STEP);
                event->rotation.vy = definitions->initial_rotation;
                definitions++;
                event->rotation.vz = 0;
                event->rotation.vx = 0;
                event->dialogue.fields.page = KF_DIALOGUE_FIRST_PAGE;
                event->dialogue.fields.stage = KF_DIALOGUE_FIRST_STAGE;
                event->dialogue.fields.page_delay = 0;
                event->animation_clip = KF_ANIMATION_CLIP_FIRST;
                event->animation_phase = 0;
                event->rotation_target = 0;
                event->collision_turn_pending = KF_MAP_EVENT_COLLISION_TURN_NONE;
                collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);
            } else {
                exhausted = KF_TRUE;
                goto mark_free;
            }
        }
        event++;
    } while (count-- != 0);
}

ADDRESS(0x80033ae4, 0xa8)
s32 map_event_distance_to_point(
    const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance)
{
    s32 delta_x = event->reference_position.vx - point_x;
    s32 delta_z;
    s32 distance;

    if (delta_x >= -max_distance && delta_x <= max_distance) {
        delta_z = event->reference_position.vz - point_z;
        if (delta_z >= -max_distance && delta_z <= max_distance) {
            delta_x >>= KF_LENGTH_SQUARE_DOWNSHIFT;
            delta_z >>= KF_LENGTH_SQUARE_DOWNSHIFT;
            distance = SquareRoot0(delta_x * delta_x + delta_z * delta_z)
                << KF_LENGTH_SQUARE_DOWNSHIFT;
            if (distance <= max_distance) {
                return distance;
            }
        }
    }
    return -1;
}

ADDRESS(0x80033b8c, 0x144)
KfMapEvent *map_event_pool_find_target_in_cone(
    const VECTOR *origin,
    s16 facing,
    s32 max_distance,
    s32 angle_tolerance,
    s32 *distance_out)
{
    KfMapEvent *found = NULL;
    s16 best_angle = MAP_EVENT_CONE_INITIAL_BEST_ERROR;
    s32 found_distance = 0;
    KfMapEvent *event = map_event_pool;
    u16 count = KF_MAP_EVENT_CAPACITY - 1;
    s32 distance;
    s16 angle;
    s16 folded;

    do {
        if (event->state != KF_MAP_EVENT_ACTIVE) {
            continue;
        }
        distance = map_event_distance_to_point(event, origin->vx, origin->vz, max_distance);
        if (distance == -1) {
            continue;
        }
        angle = vector_xz_to_angle(
            event->reference_position.vx - origin->vx, origin->vz - event->reference_position.vz) - facing;
        angle &= KF_ANGLE_WRAP_MASK;
        folded = angle;
        if (angle >= KF_ANGLE_HALF_TURN + 1) {
            folded = KF_ANGLE_FULL_TURN - angle;
        }
        if (angle_tolerance < folded) {
            continue;
        }
        if (folded < best_angle) {
            best_angle = folded;
            found = event;
            found_distance = distance;
        }
    } while (event++, count-- != 0);
    *distance_out = found_distance;
    return found;
}

ADDRESS(0x80033cd0, 0xb0)
s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding)
{
    KfMapEvent *event = map_event_pool;
    s16 index = 0;

    do {
        if (event->state == KF_MAP_EVENT_ACTIVE
            && map_event_distance_to_point(
                   event, point_x, point_z, event->radius + radius_padding) != -1) {
            return index;
        }
        index++;
        event++;
    } while (index < KF_MAP_EVENT_CAPACITY);
    return -1;
}
