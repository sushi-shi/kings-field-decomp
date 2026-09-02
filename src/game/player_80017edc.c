#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x80012048, 0x130)

extern KfPlayerState player_state;
extern KfMapObjectState map_object_state;
extern KfEffectRecord DAT_8009d040[];
/* Per-id byte table shared with the save system; entry 57 counts the lamp charges. */
extern u8 DAT_800652a8[240];
extern const SoundRef gameplay_sound_ref_7;
extern const SoundRef gameplay_sound_ref_8;
extern const SoundRef gameplay_sound_ref_12;
/* Psy-Q LIBGTE: rsin, rcos. */
extern void func_8001fa44(s32 arg0);
/* Effect spawner variant called with six arguments; declared without a prototype. */
extern u8 *func_80037770();
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern s32 map_object_pool_find_interaction_from(s32 start_index, s32 x, s32 z, s32 extra_radius);
extern void map_object_pool_trigger_link(u8 link_id);
extern int angle_within_tolerance(int lhs, int rhs, s16 range);
extern KfActor *actor_pool_find_target_in_cone( const struct KfVec3i *origin, s32 facing, u32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern KfMapEvent *map_event_pool_find_target_in_cone( const struct KfVec3i *origin, s16 facing, s32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern void player_warp_to_floor_entry(void);
extern void player_increment_magic_training(void);
/* Image paths whose decimal digits are patched before display. */
extern char enemy_info_image_path_template[13];
extern char person_image_path_template[14];
extern void func_8001fde4(s32 first, s32 second);
extern void screen_show_image_until_input(const char *path);

ADDRESS(0x80017edc, 0xc8)
void actor_show_info_image(const KfActor *actor)
{
    func_8001fde4(0, 0);
    func_8001fde4(0, 0);
    enemy_info_image_path_template[3] = '0' + player_state.progress_state.current_floor;
    enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;
    enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;
    screen_show_image_until_input(enemy_info_image_path_template);
}

ADDRESS(0x80017fa4, 0xb0)
void map_event_show_person_image(const KfMapEvent *event)
{
    func_8001fde4(0, 0);
    func_8001fde4(0, 0);
    person_image_path_template[8] = '0' + event->kind / 10;
    person_image_path_template[9] = '0' + event->kind % 10;
    screen_show_image_until_input(person_image_path_template);
}

/*
 * Uses the selected item: keys open the door in reach whose link matches,
 * crystals fire the matching lever, the flask scans the effect pool before
 * spawning its floor-specific effect, and the remaining ids warp, refill
 * the lamp, show the target's picture, or train magic.
 */
ADDRESS(0x80018054, 0x45c)
void player_use_item(u8 item_id)
{
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    KfEffectRecord *record;
    s32 distance;
    s32 index;
    s32 reach_x;
    s32 reach_z;
    s16 slot;
    u8 used = 0;

    reach_x = player_state.camera_position.vx - ((rsin(player_state.camera_rotation.vy) * 1000) >> 12);
    reach_z = player_state.camera_position.vz + ((rcos(player_state.camera_rotation.vy) * 1000) >> 12);
    index = 0;
    switch (item_id) {
    case 53:
    case 54:
    case 60:
    case 74:
        for (;;) {
            index = map_object_pool_find_interaction_from(index, reach_x, reach_z, 800);
            if (index == -1) {
                break;
            }
            object = &map_object_state.objects[index];
            switch (object->object_id) {
            case 81:
            case 83:
            case 85:
            case 89:
            case 117:
            case 118:
            case 119:
            case 120:
            case 121:
            case 122:
                if (object->link.link_id == 0xff) {
                    func_8001fa44(0x12);
                } else if (object->object_id != 89
                           || angle_within_tolerance(
                               player_state.camera_rotation.vy, 0x800 - object->rotation.y, 0x155)) {
                    used = 1;
                    if (object->link.link_id == item_id) {
                        object->link.link_id = 0xff;
                        sound_ref_play(&gameplay_sound_ref_12, 0x6e);
                        if (object->object_id == 89) {
                            sound_ref_play(&gameplay_sound_ref_7, 0x7f);
                        }
                    } else {
                        func_8001fa44(4);
                    }
                }
                break;
            }
            index++;
        }
        /* fallthrough */
    case 56:
    case 63:
    case 64:
    case 68:
    case 69:
        for (;;) {
            index = map_object_pool_find_interaction_from(index, reach_x, reach_z, 800);
            if (index == -1) {
                break;
            }
            object = &map_object_state.objects[index];
            if (object->object_id == item_id) {
                if (object->link.link_id == 0xff) {
                    func_8001fa44(0x12);
                } else {
                    DAT_800652a8[object->object_id] = 0;
                    used = 1;
                    map_object_pool_trigger_link(object->link.link_id);
                    object->link.link_id = 0xff;
                }
            }
            index++;
        }
        break;
    case 62:
        record = DAT_8009d040;
        for (slot = 47; slot != -1; slot--, record++) {
            if (record->unknown_00[0] == 0xff) {
                continue;
            }
            if (record->unknown_00[1] == 0x34) {
                goto done;
            }
        }
        if (player_state.progress_state.current_floor == 2) {
            func_80037770(0, 4, 0x96, 0x320, 0x2b, 0x46);
        } else if (player_state.progress_state.current_floor == 3) {
            func_80037770(4, 1, 0x96, 0x320, 0x58, 0x10e);
        } else {
            break;
        }
        sound_ref_play(&gameplay_sound_ref_8, 0x7f);
        used = 1;
        break;
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        used = 1;
        break;
    case 58:
        player_warp_to_floor_entry();
        return;
    case 57:
        player_state.light_effect_timer = 1000;
        if (DAT_800652a8[57] != 0) {
            DAT_800652a8[57]--;
        }
        return;
    case 59:
        actor = actor_pool_find_target_in_cone(
            (struct KfVec3i *)&player_state.camera_position,
            player_state.camera_rotation.vy,
            6000,
            0x155,
            &distance);
        if (actor != 0) {
            actor_show_info_image(actor);
            return;
        }
        event = map_event_pool_find_target_in_cone(
            (struct KfVec3i *)&player_state.camera_position,
            player_state.camera_rotation.vy,
            6000,
            0x155,
            &distance);
        if (event == 0) {
            break;
        }
        map_event_show_person_image(event);
        return;
    case 42:
        player_state.magic_training += 100;
        used = 1;
        player_increment_magic_training();
        break;
    }
done:
    if (!used) {
        func_8001fa44(0x12);
    }
}
