#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>


/* Progress-flag block; individual indices unresolved. */
extern u8 DAT_800652a8[240];
/* Cutscene-active flag. */
/* Special-event trigger latch raised by func_800346a8. */
/* Loaded camera-path point table (points begin at +8); extent unresolved. */
/* Full-screen image path template "KAN\B0\K000.TIM". */

/* Object-behaviour dispatch followed by the current-floor action dispatch. */
RODATA(0x80012a7c, 0x164)

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
void map_action_script_floor5(void)
{
    if ((*(u32 *)&map_event_pool[1].image_limit & 0xffffff00) == 0x28010500) {
        func_800346a8();
        DAT_8009f845 = 1;
    }
}

ADDRESS(0x80034a80, 0x2d4)
void map_event_interact(KfMapEvent *event)
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
void map_show_screen_image(s32 group, s32 index)
{
    DAT_80056238[8] = group + '0';
    DAT_80056238[5] = player_state.progress_state.current_floor + '0';
    DAT_80056238[9] = index / 10 + '0';
    DAT_80056238[10] = index % 10 + '0';
    screen_show_image_until_input(DAT_80056238);
}

ADDRESS(0x80034de4, 0x904)
void map_interaction_dispatch(const VECTOR *position, SVECTOR *rotation)
{
    s32 sound_x;
    s32 sound_z;
    s32 index;
    s32 slot;
    s32 item_index;
    s32 result;
    s32 neighbor_index;
    u16 saved_pitch;
    u8 found_item;
    u8 *item_id;
    KfMapEvent *event;
    KfMapObject *object;
    KfMapObject *neighbor;
    KfMapObjectDefinition *definition;
    KfMapObjectDefinition *neighbor_definition;

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
    if (notification_effect_phase == 0) {
        index = map_event_pool_find_overlap(sound_x, sound_z, 0x320);
        if (index != -1) {
            event = &map_event_pool[index];
            switch (event->unknown_0e) {
            case 0:
                event->rotation_phase = 0;
                event->unknown_0f = 0;
                map_event_advance_rotation_blocking(event, 0x800, 0xc8);
                audio_play_map_sequence(2);
                map_event_interact(event);
                menu_enter_mode(2, event->kind);
                audio_play_current_map_sequence();
                map_event_advance_rotation_blocking(event, 0xfff, 0xc8);
                event->unknown_0f = 0;
                event->rotation_phase = 0;
                player_clear_motion();
                break;
            case 1:
                map_event_interact(event);
                player_clear_motion();
                break;
            case 2:
                map_event_advance_rotation_blocking(event, 0xfff, 0x190);
                result = asset_registry_entries[event->variant]->animation_data < 2;
                if (result == 0) {
                    event->rotation_phase = 0;
                    event->unknown_0f = 1;
                    map_event_advance_rotation_blocking(event, 0x800, 0xc8);
                }
                map_event_interact(event);
                if (result == 0) {
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
        switch (definition->behavior_type) {
        case 0:
        case 1:
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, 0x155)
                && !angle_within_tolerance(
                    rotation->vy, object->rotation.y + 0x800, 0x155)) {
                break;
            }
            if (object->link.link_id != 0xff && definition->behavior_type == 0) {
                goto notify_default;
            }

            neighbor_index = 0;
            for (;;) {
                neighbor_index = map_object_pool_find_interaction_from(
                    neighbor_index, object->position_x, object->position_z, 6000);
                if (neighbor_index == -1) {
                    object->link.action_parameter = 0xff;
                    break;
                }
                if (neighbor_index != index) {
                    neighbor = &map_object_state.objects[neighbor_index];
                    neighbor_definition =
                        &map_object_state.definitions[neighbor->object_id];
                    if (neighbor_definition->behavior_type < 2) {
                        if (neighbor->link.link_id != 0xff
                            && neighbor_definition->behavior_type == 0) {
                            goto notify_default;
                        }
                        map_object_start_action_if_idle(
                            neighbor, neighbor_definition->behavior_type);
                        object->link.action_parameter = neighbor_index;
                        neighbor->link.action_parameter = index;
                        break;
                    }
                }
                neighbor_index++;
            }
            map_object_start_action_if_idle(object, definition->behavior_type);
            continue;

        case 2:
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, 0x155)
                && !angle_within_tolerance(
                    rotation->vy, object->rotation.y + 0x800, 0x155)) {
                break;
            }
            if (object->action != 0xff) {
                break;
            }
            if (object->link.link_id != 0xff) {
                goto notify_default;
            }
            map_object_start_action_if_idle(object, 2);
            continue;

        case 8:
            if (object->link.link_id != 0xff) {
                notify_enqueue(object->link.unknown_06[0]);
                continue;
            }
            if (!angle_within_tolerance(rotation->vy, object->rotation.y, 0x200)) {
                break;
            }

            item_index = 3;
            while (object->link.action_parameter == 0xff) {
                item_index--;
                if ((s16)item_index == -1) {
                    goto notify_default;
                }
            }

            audio_play_spatial_default_range(
                &gameplay_sound_ref_2, (const VECTOR *)&object->position_x, 0x7f);
            saved_pitch = rotation->vx;
            while (object->rotation.x >= -0x3ff) {
                if ((u16)(rotation->vx - 0xbf) >= 0x742) {
                    rotation->vx += 0x10;
                }
                object->rotation.x -= 0x20;
                render_frame(position, rotation);
                frame_pacer_wait();
            }

            item_index = 3;
            item_id = &object->link.action_parameter;
            for (;;) {
                if (*item_id != 0xff) {
                    result = menu_enter_mode(1, *item_id);
                    if (result == 0) {
                        *item_id = 0xff;
                    } else if (result == 2) {
                        notify_enqueue(0x10);
                    }
                }
                item_index--;
                if ((s16)item_index == -1) {
                    break;
                }
                item_id++;
            }
            object->rotation.x = 0;
            rotation->vx = saved_pitch;
            break;

        case 9:
            item_id = &object->link.link_id;
            found_item = 0;
            item_index = 3;
            for (;;) {
                if (*item_id != 0xff) {
                    found_item = 1;
                    result = menu_enter_mode(1, *item_id);
                    if (result == 0) {
                        *item_id = 0xff;
                    } else if (result == 2) {
                        notify_enqueue(0x10);
                    }
                }
                item_index--;
                if ((s16)item_index == -1) {
                    break;
                }
                item_id++;
            }
            if (found_item == 0) {
                goto notify_default;
            }
            continue;

        case 11:
            if (object->link.link_id != 0xff) {
                goto notify_default;
            }
            player_restore_vitals_with_color_cycle();
            continue;

        case 13:
            if (notification_effect_phase != 0) {
                break;
            }
            if (object->object_id == 0x82) {
                result = 0;
            } else {
                if (object->object_id != 0x83) {
                    return;
                }
                result = 1;
            }
            map_show_screen_image(result, object->link.link_id);
            player_clear_motion();
            continue;

        case 14:
            map_world_state_persist();
            menu_save_confirm();
            continue;

        case 0x40:
            result = menu_enter_mode(1, object->object_id);
            if (result == 0) {
                object->object_id = 0xff;
            } else if (result == 2) {
                notify_enqueue(0x10);
                continue;
            }
            break;

        case 0x41:
            result = object->link.link_id | object->link.action_parameter << 8;
            notify_enqueue(0x13, result);
            player_state.gold += result;
            object->object_id = 0xff;
            break;

        case 0x53:
            if (object->link.link_id == 0xff) {
                goto notify_default;
            }
            object->action_timer = 1;
            break;

        default:
notify_default:
            notify_enqueue(object->link.unknown_06[1]);
            break;
        }
    }

    switch (player_state.progress_state.current_floor) {
    case 1:
        map_action_script_floor1();
        break;
    case 2:
        map_action_script_floor2();
        break;
    case 3:
        map_action_script_floor3();
        break;
    case 4:
        map_action_script_floor4();
        break;
    case 5:
        map_action_script_floor5();
        break;
    default:
        break;
    }
}
