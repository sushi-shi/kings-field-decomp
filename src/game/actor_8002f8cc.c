#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x80012524, 0x214)

extern KfActorState actor_state;
extern KfPlayerState player_state;
extern u8 map_cell_attribute_grid[100][100];
extern s16 map_cell_attribute_height_table[284];

extern const SoundRef boss_death_phase_sounds[4];
extern const SoundRef boss_death_loop_sound;
extern u8 boss_defeat_complete;

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);
extern void actor_play_sound_at_phase(const SoundRef *sound, u16 phase);
extern void map_object_pool_trigger_link(u8 link_id);
extern void actor_pool_begin_death_by_definition(u16 definition_id);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
/* Effect spawner called with six to eight arguments; declared without a prototype. */
extern u8 *func_80036f44();
extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern s32 map_floor_height_at_position(const struct KfVec4i *position);
extern s32 vector_xz_to_angle(s32 x, s32 z);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern void vector3i_add_xz(struct KfVec3i *destination, const struct KfVecXZs *delta);
extern s16 angle_approach(s16 current, s16 target, s32 step);
extern s32 actor_move_along_heading(s32 direction, s32 stop_on_collision);
extern void actor_prepare_charge_toward_player(void);
extern void actor_apply_horizontal_movement(void);
extern void actor_update_effect_action(s32 action);
extern void actor_apply_random_movement(s16 step, s16 limit);
extern void actor_select_next_action(s32 player_distance);
extern s32 actor_animation_crossed_phase(const KfActor *actor, u16 phase);
/*
 * These three take halfword arguments that retail passes without any
 * extension, so they are declared without prototypes here.
 */
extern void actor_advance_animation_clamped();
extern void actor_advance_animation_wrapped();
extern void actor_try_attack_player();
extern s32 actor_distance_to_point(
    const KfActor *actor,
    s32 point_x,
    s32 point_y,
    s32 point_z,
    s32 max_distance,
    s32 actor_height,
    s32 point_height);
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
extern void actor_initialize(KfActor *actor);
extern void func_80036d3c(KfActor *actor);
extern void map_object_spawn_effect(
    u8 kind, u8 object_id, const struct KfVec3i *position, s32 y_offset);
extern void map_object_spawn_actor_debris(
    u16 source, const struct KfVec3i *position, s32 y_offset);
extern void player_apply_damage(
    u16 component0,
    u16 component1,
    u16 component2,
    u16 status_effect_flags,
    u16 component3,
    u16 component4,
    u16 scale_q12,
    u16 multiplier_tenths);

/* Boss death: phase sounds, then random effects and a loop sound keyed on the death animation step. */
ADDRESS(0x8002f8cc, 0x1bc)
void actor_update_boss_death_sequence(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    u32 effect_output[2];
    struct KfVec3i position;

    actor_play_sound_at_phase(&boss_death_phase_sounds[1], 500);
    actor_play_sound_at_phase(&boss_death_phase_sounds[2], 1000);
    actor_play_sound_at_phase(&boss_death_phase_sounds[3], 1500);
    if (actor->animation_phase >= 4095) {
        actor->animation_phase = 0xfff;
        actor->action_timer = 0;
        actor->lifecycle = 3;
        boss_defeat_complete = 1;
        map_object_pool_trigger_link(13);
        actor_pool_begin_death_by_definition(0);
        actor_pool_begin_death_by_definition(2);
        actor_pool_begin_death_by_definition(3);
        actor_pool_begin_death_by_definition(4);
    }
    if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 2) == 0) {
        position.x = actor->position.x + (rand() & 0x1fff) - 4096;
        position.z = actor->position.z + (rand() & 0x1fff) - 4096;
        position.y = actor->position.y - (rand() & 0xfff);
        func_80036f44(0, 0x13, 0x2c, &position, effect_output, 0);
        if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 4) == 0) {
            sound_ref_play(&boss_death_loop_sound, 100);
        }
    }
}

/*
 * Runs the bound actor's current action, then its vertical state. The
 * actor leaves the collision cell census while it moves and rejoins at the
 * end; the removal paths return early so a dead actor is not counted again.
 */
ADDRESS(0x8002fa88, 0xd90)
void actor_update_current_action(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    struct KfVecXZs direction;
    struct KfVec3i target;
    s32 result;
    s32 hit;
    s32 floor_height;
    s32 next_y;
    s32 home_x;
    s32 home_z;
    u16 debris;
    u8 attribute;

    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, -1);
    switch (actor->action) {
    case 0:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[0];
            actor->animation_phase = 0;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[0]);
        break;
    case 1:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = rand() >> 3;
        } else if (actor->collision_state == 0 && rand() < 2048) {
            actor->movement_yaw = rand() >> 3;
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 2:
        switch (actor->action_timer) {
        case 0:
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.x - actor->position.x,
                actor_state.player_position.z - actor->position.z);
            break;
        case 1:
            if (actor_move_along_heading(1, 1) != 0) {
                actor->action_timer = (rand() >> 11) + 13;
                goto vertical;
            }
            if (rand() < 4096) {
                actor->movement_yaw = vector_xz_to_angle(
                    actor_state.player_position.x - actor->position.x,
                    actor_state.player_position.z - actor->position.z);
            }
            break;
        default:
            if (actor_move_along_heading(-1, 1) != 0 || actor->action_timer < 11) {
                actor->action_timer = 1;
            } else {
                actor->action_timer--;
            }
            break;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 5:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[3];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[3]);
        actor_play_sound_at_phase(&definition->sounds[1], definition->action_animation_phases[3]);
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.x,
                0xffff,
                actor_state.player_position.z,
                32000,
                0,
                0));
        }
        break;
    case 6:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[4];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[4]);
        actor_play_sound_at_phase(&definition->sounds[2], definition->action_animation_phases[4]);
        if (player_state.progress_state.current_floor == 5 && actor->definition_id == 7) {
            actor_update_boss_death_sequence();
            return;
        }
        if (actor_animation_crossed_phase(actor, 0x800)) {
            debris = ((u32)rand() * definition->unknown_96) >> 15;
            if (debris != 0) {
                map_object_spawn_actor_debris(
                    debris, (struct KfVec3i *)&actor->position, -(definition->collision_height >> 1));
            }
            if (actor->slot_state == 0 || actor->slot_state == 2) {
                if (definition->action_parameters[6] != 0x63 && definition->action_parameters[6] != 0xff
                    && (rand() >> 7) <= definition->action_parameters[7]) {
                    map_object_spawn_effect(
                        1,
                        definition->action_parameters[6],
                        (struct KfVec3i *)&actor->position,
                        -(definition->collision_height >> 1));
                }
            } else if (actor->unknown_09 != 0x63) {
                map_object_spawn_effect(
                    0,
                    actor->unknown_09,
                    (struct KfVec3i *)&actor->position,
                    -(definition->collision_height >> 1));
            }
        }
        if (actor->animation_phase >= 4095) {
            actor->action = 0x7f;
            actor->animation_phase = 0xfff;
            actor->action_timer = 0;
        }
        break;
    case 3:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.x - actor->position.x,
                actor_state.player_position.z - actor->position.z);
        }
        if (actor_move_along_heading(-1, 1) != 0) {
            actor->action = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.x,
                0xffff,
                actor_state.player_position.z,
                32000,
                0,
                0));
        } else {
            actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        }
        break;
    case 4:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[2];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[2]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[2]);
        if (actor->animation_phase >= 2700
            && actor->animation_phase < definition->action_animation_steps[2] + 2700) {
            actor_try_attack_player(0, definition->awareness_distance, 0, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.x,
                0xffff,
                actor_state.player_position.z,
                32000,
                0,
                0));
        }
        break;
    case 127:
        if (player_state.progress_state.current_floor == 4 && actor->definition_id == 5) {
            func_80036d3c(actor);
            actor_initialize(actor);
            return;
        }
        if (actor->action_timer >= 7) {
            if (actor->slot_state == 2) {
                actor->lifecycle = 0;
            } else if (actor->slot_state == 1 || actor->slot_state == 3) {
                actor->lifecycle = 3;
            } else {
                actor->lifecycle = 2;
            }
            return;
        }
        actor->action_timer++;
        break;
    case 16:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[5];
            actor->animation_phase = 0;
            attribute = map_cell_attribute_grid[actor->cell_z][actor->cell_x];
            if (map_cell_attribute_height_table[attribute - 1] > -5000) {
                actor->vertical_state = 4;
                actor->vertical_velocity = -220;
                actor->animation_step = 140;
            } else {
                actor->vertical_state = 4;
                actor->vertical_velocity = -300;
                actor->animation_step = 110;
            }
            actor_prepare_charge_toward_player();
        }
        actor_apply_horizontal_movement();
        actor->animation_phase += actor->animation_step;
        if (actor->animation_phase >= 4095) {
            actor->animation_phase = 0xfff;
        }
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[5]);
        switch (actor->action_timer) {
        case 1:
            if (actor->vertical_velocity >= 0) {
                actor_prepare_charge_toward_player();
                actor->action_timer = 2;
            }
            break;
        case 2:
            if (actor_animation_crossed_phase(actor, 0xd48)) {
                actor_try_attack_player(0, definition->unknown_36, 0, 0x155);
                actor->action_timer = 3;
            }
            break;
        case 3:
            if (actor->vertical_state != 4) {
                actor->vertical_state = 0;
                actor->vertical_velocity = 0;
                result = collision_query_world(
                    actor->position.x,
                    actor->position.y,
                    actor->position.z,
                    definition->collision_radius,
                    definition->collision_height,
                    0x8060);
                if (result != -1 && (result >> 16) == 0xfff1) {
                    actor->vertical_state = 2;
                    actor->vertical_velocity = 0;
                }
                actor->action_timer = 0xff;
                actor_select_next_action(actor_distance_to_point(
                    actor,
                    actor_state.player_position.x,
                    0xffff,
                    actor_state.player_position.z,
                    32000,
                    0,
                    0));
            }
            break;
        }
        break;
    case 17:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[6];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[6]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[6]);
        if (actor->animation_phase >= 3000
            && actor->animation_phase < definition->action_animation_steps[6] + 3000) {
            actor_try_attack_player(0, definition->unknown_36, 0, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.x,
                0xffff,
                actor_state.player_position.z,
                32000,
                0,
                0));
        }
        break;
    case 32:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[1];
            actor->animation_phase = 0;
        }
        result = collision_query_world(
            actor->position.x, 0xffff, actor->position.z, definition->collision_radius, 0, 0x8040);
        if (result != -1) {
            target.x = actor->position.x;
            target.z = actor->position.z;
            angle_to_forward_xz(actor->rotation.y, &direction);
            vector2s_scale_shift11(definition->move_speed, (s16 *)&direction);
            vector3i_add_xz(&target, &direction);
            if (actor_pool_find_overlap(target.x, 0xffff, target.z, definition->collision_radius, 0)
                == -1) {
                actor->position.x = target.x;
                actor->position.z = target.z;
            }
        } else {
            actor->action_timer = 0xff;
            actor_select_next_action(32000);
        }
        break;
    case 18:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[7];
            actor->animation_phase = 0;
            actor->movement_y = 0;
            actor->movement_z = 0;
            actor->movement_x = 0;
            actor->movement_yaw = 0;
            actor->vertical_state = 0;
            actor->vertical_velocity = 0;
        }
        actor_apply_random_movement(1, 100);
        if (rand() < 16384) {
            actor->movement_yaw++;
            if (actor->movement_yaw > 32) {
                actor->movement_yaw = 32;
            }
        } else {
            actor->movement_yaw--;
            if (actor->movement_yaw < -32) {
                actor->movement_yaw = -32;
            }
        }
        actor->rotation.y = (actor->rotation.y + actor->movement_yaw) & 0xfff;
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[7]);
        break;
    case 19:
        actor_update_effect_action(0);
        break;
    case 20:
        actor_update_effect_action(1);
        break;
    case 21:
        actor_update_effect_action(2);
        break;
    case 33:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            /* Retail reads the home position before this branch assigns it. */
            actor->movement_yaw =
                vector_xz_to_angle(home_x - actor->position.x, home_z - actor->position.z);
        } else if (actor->collision_state == 0) {
            home_x = actor->tile_x * 2000 + actor->local_x;
            home_z = actor->tile_z * 2000 + actor->local_z;
            if (actor->position.x - home_x > -200 && actor->position.x - home_x < 200
                && actor->position.z - home_z > -200 && actor->position.z - home_z < 200) {
                actor->movement_yaw = actor->heading_quadrant << 10;
                actor->rotation.y = angle_approach(
                    actor->rotation.y, actor->movement_yaw, definition->turn_rate);
                if (actor->animation_id != definition->action_animations[2]) {
                    break;
                }
                if (actor->movement_yaw == actor->rotation.y
                    && actor_animation_crossed_phase(actor, 0x4b0)) {
                    actor->animation_phase = 0;
                    actor->animation_id = definition->action_animations[2];
                    break;
                }
                actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
                break;
            }
            if (rand() < 2048) {
                actor->movement_yaw = vector_xz_to_angle(
                    home_x - actor->position.x, home_z - actor->position.z);
            }
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 22:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[11];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[11]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[11]);
        if (actor_animation_crossed_phase(actor, 0x8c0) || actor_animation_crossed_phase(actor, 0xa80)
            || actor_animation_crossed_phase(actor, 0xc80)
            || actor_animation_crossed_phase(actor, 0xe00)) {
            actor_try_attack_player(8000, 11000, 0, 0x155);
        }
        if (actor_animation_crossed_phase(actor, 0xdac)) {
            actor_try_attack_player(0, 11000, -512, 0x155);
        }
        if (actor_animation_crossed_phase(actor, 0xed8)) {
            actor_try_attack_player(0, 11000, 512, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.x,
                0xffff,
                actor_state.player_position.z,
                32000,
                0,
                0));
        }
        break;
    }
vertical:
    switch (actor->vertical_state) {
    case 0:
        break;
    case 1:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.y;
        if (next_y > floor_height) {
            goto fall;
        }
    land:
        actor->position.y = floor_height;
        actor->vertical_state = 0;
        actor->vertical_velocity = 0;
        break;
    fall:
        actor->position.y = next_y;
        actor->vertical_velocity += 20;
        break;
    case 2:
    case 3:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.y;
        if (next_y >= floor_height) {
            goto land;
        }
        goto fall;
    case 4:
        next_y = actor->vertical_velocity + actor->position.y;
        hit = collision_query_world(
            actor->position.x,
            next_y,
            actor->position.z,
            definition->collision_radius,
            definition->collision_height,
            0x8060);
        if (hit == -1) {
            goto fall;
        }
        if ((hit >> 16) == 0x80) {
            player_apply_damage(0, 15, 0, 0, 0, 0, 0x1000, 10);
        stagger:
            actor->vertical_state = 4;
            actor->vertical_velocity = -120;
            actor->animation_phase = 0;
        } else if ((hit >> 16) == 1) {
            switch (hit & 0xffff) {
            case 0xfff0:
                floor_height = map_floor_height_at_position(&actor->position);
                goto land;
            case 0xfff1:
                actor->vertical_state = 4;
                actor->vertical_velocity = 100;
                break;
            }
        } else if ((hit >> 16) == 0x10) {
            goto stagger;
        }
        break;
    }
    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, 1);
}
