#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfMapEvent map_event_pool[8];
extern KfMapObjectState map_object_state;
extern KfMagicRecord magic_records[24];
extern u8 map_floor_height_grid[100][100];
extern u8 map_cell_attribute_grid[100][100];
extern u16 map_object_effect_sequence_180;
extern SoundRef gameplay_sound_ref_2;

/* Progress-flag block; individual indices unresolved. */
extern u8 DAT_800652a8[240];
/* Cutscene-active flag. */
extern u8 DAT_80095088;
/* Special-event trigger latch raised by func_800346a8. */
extern u8 DAT_8009f845;
/* Loaded camera-path point table (points begin at +8); extent unresolved. */
extern u8 DAT_800561c8[0x70];
/* Full-screen image path template "KAN\B0\K000.TIM". */
extern char DAT_80056238[16];

extern void player_equip_weapon(u8 weapon_id);
extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points);
extern void camera_path_step(KfCameraPathState *path, s32 y_offset);
extern KfMapObject *map_object_effect_pool_acquire(u16 first_index, u16 count, u16 sequence);
extern void map_object_start_action_if_idle(KfMapObject *object, u8 action);
extern void map_object_pool_clear_link(u8 link_id);
extern s32 map_object_pool_find_interaction_from(
    s16 start_index, s32 point_x, s32 point_z, s32 radius_padding);
extern void talk_show_indexed_image(u8 prefix_digit, u8 index_digit, s32 group_id, u8 frame_digit);
extern void screen_show_image_until_input(const char *path);
extern void map_event_refresh_image_for_progress(KfMapEvent *event);
extern void map_event_advance_rotation_blocking(KfMapEvent *event, u16 target, s16 step);
extern s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding);
extern void audio_play_map_sequence(u8 sequence_id);
extern void audio_play_current_map_sequence(void);
extern void audio_play_spatial_default_range(const SoundRef *sound, const VECTOR *position, s16 volume);
extern void player_clear_motion(void);
extern void player_restore_vitals_with_color_cycle(void);
extern int angle_within_tolerance(s32 angle, s32 target, s16 tolerance);
extern void frame_pacer_wait(void);

/* Unprototyped helpers: called with varying arities/argument types. */
extern void render_frame();
extern void notify_enqueue(s32 arg0);
extern u32 func_80036e38(s32 arg0, u8 arg1);
extern u8 *effect_pool_construct();
extern void effect_pool_sweep(void);
extern void func_80035b5c(void);
extern void menu_save_confirm(void);
extern void func_800343e0(void);
extern void func_800345bc(void);
extern void func_80034610(void);
extern void func_800346a0(void);

ADDRESS(0x800346a8, 0x38c)
void func_800346a8(void)
{
    MATRIX color_matrix;
    KfCameraPathState path;
    KfMapObject *effect;
    s32 aux[2];
    VECTOR spawn;
    s32 grid_height;
    s32 spin;
    s32 hold;
    s32 stage;

    if (player_state.equipped_weapon_id == 10) {
        player_equip_weapon(0xff);
    }
    DAT_800652a8[0xa] = 0;
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, -1);

    camera_path_begin(&path, (const KfCameraPathPoint *)&DAT_800561c8[8]);
    for (;;) {
        camera_path_step(&path, 0);
        if (path.frames_remaining == -1) {
            break;
        }
        render_frame(&path.position, &path.rotation);
    }

    player_state.camera_position = path.position;
    player_state.camera_rotation = path.rotation;
    player_state.map_cell.x = player_state.camera_position.vx / 2000;
    player_state.map_cell.z = player_state.camera_position.vz / 2000;
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);

    ReadColorMatrix(&color_matrix);
    effect = map_object_effect_pool_acquire(180, 10, map_object_effect_sequence_180);
    effect->object_id = 10;
    effect->cell_x = 85;
    effect->cell_z = 40;
    effect->position_x = effect->cell_x * 2000 + 1000;
    effect->position_z = effect->cell_z * 2000 + 1000;
    grid_height = map_floor_height_grid[effect->cell_z][effect->cell_x];
    effect->rotation.z = 0;
    effect->rotation.x = 0;
    effect->rotation.y = 0x800;
    effect->action = 0xff;
    effect->position_y = -(grid_height * 100) - 1300;

    spin = 0;
    hold = 0;
    stage = 0;
    for (;;) {
        switch (stage) {
        case 0:
            effect->rotation.y += spin;
            if (hold != 0) {
                hold -= 1;
                if (hold == 1) {
                    stage = 1;
                } else if (hold == 0x14) {
                    spawn = *(VECTOR *)&effect->position_x;
                    spawn.vy -= 600;
                    effect_pool_construct(0, 0x13, 0x12, &spawn, aux, 1);
                    effect->object_id = 0xb;
                }
            } else if (spin < 240) {
                spin += 1;
            } else {
                hold = 0x28;
            }
            break;
        case 1:
            effect->rotation.y += spin;
            if (spin > 0) {
                spin -= 1;
            } else {
                map_object_start_action_if_idle(effect, 0x60);
                effect->link.vertical_velocity = 0;
                goto done;
            }
            break;
        default:
            break;
        }
        effect_pool_sweep();
        render_frame(0, 0);
    }
done:
    ;
}

ADDRESS(0x80034a34, 0x4c)
void func_80034a34(void)
{
    if ((*(u32 *)&map_event_pool[1].image_limit & 0xffffff00) == 0x28010500) {
        func_800346a8();
        DAT_8009f845 = 1;
    }
}

ADDRESS(0x80034a80, 0x2d4)
void func_80034a80(KfMapEvent *event)
{
    switch (event->kind) {
    case 3:
        if (DAT_800652a8[0x34] != 0 && map_event_pool[2].image_index == 1
            && map_event_pool[2].image_dirty < 3) {
            DAT_800652a8[0x35] = 1;
            map_event_pool[2].tag.bytes[0] = 7;
            DAT_800652a8[0x34]--;
            talk_show_indexed_image(player_state.progress_state.current_floor,
                                    event->image_index, event->kind, 3);
            map_event_pool[2].image_dirty = 4;
            map_event_pool[2].image_delay = 0;
            map_event_pool[2].image_limit = 5;
            map_event_refresh_image_for_progress(&map_event_pool[2]);
            return;
        }
        break;
    case 8:
        if (DAT_800652a8[0x3b] != 0 && map_event_pool[2].image_index == 2
            && map_event_pool[2].image_dirty < 2) {
            *(u8 *)&magic_records[0] = 1;
            DAT_800652a8[0x3b]--;
            notify_enqueue(1);
            map_event_pool[2].tag.bytes[1] = 7;
            talk_show_indexed_image(player_state.progress_state.current_floor,
                                    event->image_index, event->kind, 2);
            map_event_pool[2].image_dirty = 3;
            map_event_pool[2].image_delay = 0;
            map_event_pool[2].image_limit = 5;
            map_event_refresh_image_for_progress(&map_event_pool[2]);
            return;
        }
        break;
    case 7:
        if (DAT_800652a8[0x2f] != 0 && map_event_pool[1].image_index == 2
            && map_event_pool[1].image_dirty < 2) {
            DAT_800652a8[0x3e] = 1;
            map_event_pool[1].tag.bytes[1] = 5;
            DAT_800652a8[0x2f]--;
            talk_show_indexed_image(player_state.progress_state.current_floor,
                                    event->image_index, event->kind, 2);
            map_event_pool[1].image_dirty = 3;
            map_event_pool[1].image_delay = 0;
            map_event_pool[1].image_limit = 2;
            map_event_refresh_image_for_progress(&map_event_pool[1]);
            return;
        }
        break;
    case 12:
        map_object_pool_clear_link(0x37);
        break;
    default:
        break;
    }

    if (event->image_limit != 0) {
        if (event->tag.bytes[event->image_index - 1] != 0) {
            talk_show_indexed_image(player_state.progress_state.current_floor,
                                    event->image_index, event->kind, event->image_dirty);
            if (event->image_delay == 0) {
                event->image_delay = 0x28;
            }
        }
    }
}

ADDRESS(0x80034d54, 0x90)
void func_80034d54(s32 group, s32 index)
{
    DAT_80056238[8] = group + '0';
    DAT_80056238[5] = player_state.progress_state.current_floor + '0';
    DAT_80056238[9] = index / 10 + '0';
    DAT_80056238[10] = index % 10 + '0';
    screen_show_image_until_input(DAT_80056238);
}

ADDRESS(0x80034de4, 0x904)
void func_80034de4(const VECTOR *position, SVECTOR *rotation)
{
    s32 sound_x;
    s32 sound_z;
    s32 index;
    s32 slot;
    KfMapEvent *event;
    KfMapObject *object;
    KfMapObjectDefinition *definition;

    sound_x = position->vx - (rsin(rotation->vy) * 1500 >> 12);
    sound_z = position->vz + (rcos(rotation->vy) * 1500 >> 12);
    switch (map_cell_attribute_grid[sound_z / 2000][sound_x / 2000]) {
    case 0x3a:
        notify_enqueue(0xc);
        break;
    case 0x3f:
        notify_enqueue(0x17);
        break;
    case 0x5d:
        notify_enqueue(0x18);
        break;
    case 0x45:
        notify_enqueue(0x11);
        break;
    default:
        break;
    }

    sound_x = position->vx - (rsin(rotation->vy) * 1000 >> 12);
    sound_z = position->vz + (rcos(rotation->vy) * 1000 >> 12);
    if (DAT_80095088 == 0) {
        index = map_event_pool_find_overlap(sound_x, sound_z, 0x320);
        if (index != -1) {
            event = &map_event_pool[index];
            switch (event->unknown_0e) {
            case 0:
                event->rotation_phase = 0;
                event->unknown_0f = 0;
                map_event_advance_rotation_blocking(event, 0x800, 0xc8);
                audio_play_map_sequence(2);
                func_80034a80(event);
                func_80036e38(2, event->kind);
                audio_play_current_map_sequence();
                map_event_advance_rotation_blocking(event, 0xfff, 0xc8);
                event->unknown_0f = 0;
                event->rotation_phase = 0;
                player_clear_motion();
                break;
            case 1:
                func_80034a80(event);
                player_clear_motion();
                break;
            case 2:
                map_event_advance_rotation_blocking(event, 0xfff, 0x190);
                func_80034a80(event);
                if (event->unknown_0f == 0) {
                    map_event_advance_rotation_blocking(event, 0xfff, 0xc8);
                }
                event->unknown_0f = 0;
                event->rotation_phase = 0;
                player_clear_motion();
                break;
            default:
                break;
            }
        }
        return;
    }

    for (slot = 0;; slot++) {
        index = map_object_pool_find_interaction_from(slot, sound_x, sound_z, 0x320);
        if (index == -1) {
            break;
        }
        object = &map_object_state.objects[index];
        definition = &map_object_state.definitions[object->object_id];
        if (definition->behavior_type > 0x53) {
            notify_enqueue(object->unknown_29);
            continue;
        }
        switch (definition->behavior_type) {
        case 8:
            if (object->link.link_id == 0xff
                && angle_within_tolerance(rotation->vy, object->rotation.y, 0x200)) {
                if (object->link.action_parameter == 0xff) {
                    audio_play_spatial_default_range(&gameplay_sound_ref_2,
                                                     (const VECTOR *)&object->position_x, 0x7f);
                }
                notify_enqueue(object->action);
            }
            break;
        default:
            notify_enqueue(object->unknown_29);
            break;
        }
    }

    switch (player_state.progress_state.current_floor) {
    case 1:
        func_800343e0();
        break;
    case 2:
        func_800345bc();
        break;
    case 3:
        func_80034610();
        break;
    case 4:
        func_800346a0();
        break;
    case 5:
        func_80034a34();
        break;
    default:
        break;
    }
}
