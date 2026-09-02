#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Map-event runtime band 0x80035708..0x80035e14 (GAME.EXE).
 *
 * func_8003596c is the per-frame driver called by game_main_loop: it walks the
 * eight-record map_event_pool, dispatches each active event on its unknown_0e
 * kind (1 -> func_80035708 wander, 2 -> func_800358e0 spinner), advances the
 * image-animation counters, then runs two global countdowns that fire the
 * per-floor ambient scripts. func_80035b5c serialises the live event, actor,
 * and map-object state into the DAT_8009ddb4 world-state block per floor and is
 * invoked on death restart, floor teleport, and from func_80035e14.
 *
 * map_event_pool, current_map_event, DAT_8009ddb0/b2/b4 and the per-floor save
 * records are one contiguous BSS aggregate in the original; func_800358e0 and
 * func_80035b5c reach the pool through a single DAT_8009ddb4 base register,
 * which separate globals cannot reproduce (documented residue).
 */

extern KfPlayerState player_state;
extern KfMapEvent map_event_pool[8];
extern KfMapEvent *current_map_event;
extern KfActorState actor_state;
extern KfMapObjectState map_object_state;
extern SoundRef gameplay_sound_ref_10;

/* Event-animation gate: nonzero three frames in four. */
extern u16 DAT_8009ddb0;
/* Ambient floor-script countdown, reloaded to 10. */
extern u16 DAT_8009ddb2;
/* Start of the persistent world-state block (save_system world_state base). */
extern u32 DAT_8009ddb4;

extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern s16 angle_approach(s16 current, s16 target, s32 step);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern void audio_play_spatial_range(
    const SoundRef *sound, const VECTOR *position, s16 volume,
    s32 max_distance, s32 attenuation_distance);
extern void map_event_set_current(KfMapEvent *event);
extern int rand(void);

/* Per-floor ambient-event scripts dispatched by current_floor. */
extern void func_80033f64(void);
extern void func_800341ec(void);
extern void func_8003425c(void);
extern void func_800342e4(void);
extern void func_800342ec(void);

/* func_8003596c current-floor dispatch jump table (cases 1..5). */
RODATA(0x80012be4, 0x14)

ADDRESS(0x80035708, 0x1d8)
void func_80035708(void)
{
    KfMapEvent *event = current_map_event;
    struct KfVecXZs forward;
    VECTOR point;
    s16 heading;

    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);

    heading = angle_approach(event->rotation, event->rotation_target, 0x46);
    event->rotation = heading;
    angle_to_forward_xz(heading, &forward);
    vector2s_scale_shift11(0x14, (s16 *)&forward);

    point.vx = forward.x + event->reference_x;
    point.vz = forward.z + event->reference_z;

    if (collision_query_world(point.vx, 0xffff, point.vz, event->radius, 0, 0x8040) == (u32)-1) {
        event->reference_x = point.vx;
        event->reference_z = point.vz;
        event->cell_x = point.vx / 2000;
        event->cell_z = point.vz / 2000;
        event->unknown_10 = 0;
        if (event->rotation == event->rotation_target && rand() < 1584) {
            event->rotation_target = rand() >> 3;
        }
    } else {
        if (event->unknown_10 == 0 || event->rotation == event->rotation_target) {
            event->rotation_target = rand() >> 3;
            event->unknown_10 = 1;
        }
    }

    event->rotation_phase = (event->rotation_phase + 110) & 0xfff;
    collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);
}

ADDRESS(0x800358e0, 0x8c)
void func_800358e0(void)
{
    KfMapEvent *event = current_map_event;

    event->rotation_phase = (event->rotation_phase + 200) & 0xfff;

    if (player_state.progress_state.current_floor == 5
            && event == &map_event_pool[0]
            && map_event_pool[0].rotation_phase < 200) {
        audio_play_spatial_range(&gameplay_sound_ref_10,
            (const VECTOR *)&map_event_pool[0].reference_x,
            0x7f, 0x4650, 0xc350);
    }
}

ADDRESS(0x8003596c, 0x1f0)
void func_8003596c(void)
{
    KfMapEvent *event = map_event_pool;
    u16 index = 7;

    do {
        s32 state = event->state;

        if (state == 1) {
            map_event_set_current(event);

            if (event->unknown_0e == state) {
                goto call_wander;
            }
            if (event->unknown_0e == 2) {
                goto call_spinner;
            }
            goto advance_image;
        call_wander:
            func_80035708();
            goto advance_image;
        call_spinner:
            func_800358e0();
        advance_image:
            if (DAT_8009ddb0 == 0 && event->image_delay != 0) {
                event->image_delay--;
                if (event->image_delay == 0) {
                    s32 limit = event->tag.bytes[event->image_index - 1];
                    event->image_dirty++;
                    if (event->image_dirty >= limit) {
                        event->image_dirty = limit;
                    }
                }
            }
        }

        event++;
    } while (index-- != 0);

    {
        u16 *gate = &DAT_8009ddb0;
        u16 current = *gate;

        *gate = current - 1;
        if (current == 0) {
            *gate = 3;
        }
    }

    if (DAT_8009ddb2-- == 0) {
        DAT_8009ddb2 = 10;
        switch (player_state.progress_state.current_floor) {
        case 1:
            func_80033f64();
            break;
        case 2:
            func_800341ec();
            break;
        case 3:
            func_8003425c();
            break;
        case 4:
            func_800342e4();
            break;
        case 5:
            func_800342ec();
            break;
        }
    }
}

ADDRESS(0x80035b5c, 0x2b8)
void func_80035b5c(void)
{
    u8 *base = (u8 *)&DAT_8009ddb4;
    u8 *out;
    u8 *count_slot;
    KfMapEvent *event;
    KfActor *actor;
    KfMapObject *object;
    s32 i;
    s32 active;

    out = base - 1690 + 1700 * player_state.progress_state.current_floor;
    *out++ = 1;

    event = (KfMapEvent *)(base - 556);
    for (i = 0; i < 8; i++, event++) {
        *out++ = event->state;
        *out++ = event->image_limit;
        *out++ = event->image_index;
        *out++ = event->image_dirty;
        *out++ = event->tag.bytes[event->image_index - 1];
        *out++ = event->image_delay;
        *out++ = event->unknown_0d;
    }

    count_slot = out++;
    active = 0;
    actor = &actor_state.actors[0];
    for (i = 0; i < 128; i++, actor++) {
        if (actor->slot_state == 1 || actor->slot_state == 3) {
            active++;
            *out++ = i;
            if (actor->lifecycle == 3) {
                *out++ = 3;
            } else {
                *out++ = 0;
            }
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[0];
    for (i = 0; i < 190; i++, object++) {
        *out++ = object->object_id;
    }

    count_slot = out++;
    active = 0;
    object = &map_object_state.objects[0];
    for (i = 0; i < 160; i++, object++) {
        u8 id = object->object_id;
        u8 behavior;

        if (id == 0xff) {
            continue;
        }

        behavior = map_object_state.definitions[id].behavior_type;
        if ((behavior == 0xff || behavior == 0xd || behavior == 0x40
                || behavior == 0xe || behavior == 0x41)
                && object->action == 0xff) {
            continue;
        }

        active++;
        *out++ = i;
        {
            u8 *link = (u8 *)&object->link;
            s32 k = 7;

            do {
                *out++ = *link++;
            } while (--k != -1);
        }
    }
    *count_slot = active;

    object = &map_object_state.objects[160];
    for (i = 0; i < 10; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = object->link.link_id;
        *out++ = (u8)(*(const u16 *)&object->link >> 8);
    }

    object = &map_object_state.objects[170];
    for (i = 0; i < 20; i++, object++) {
        *out++ = (u8)object->cell_x;
        *out++ = (u8)object->cell_z;
        *out++ = (u8)((u16)object->rotation.y >> 4);
    }
}
