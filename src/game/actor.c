#include <kf/address.h>
#include <kf/game_actor.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Unresolved flag consulted before damaging definition 7 on floor 5. */
/*
 * Shared pool of 48 sixty-byte effect records whose first byte is 0xff when free.
 */

/*
 * Twenty-five ten-byte action-selection profiles indexed by profile_index in
 * actor_try_select_profiled_action: {far_distance, far_weight, near_distance,
 * middle_weight, near_weight}. Unused rows are zero.
 */
DATA(0x80056080, 0xfa)
KfActorActionProfile actor_action_profiles[25] = {
    {0},
    {0},
    {0},
    {0},
    {12000, 256, 5500, 768, 32},
    {9000, 256, 4000, 768, 48},
    {0},
    {9000, 256, 4000, 768, 48},
    {9000, 256, 4000, 768, 48},
    {10000, 256, 1000, 768, 32},
    {6000, 64, 3000, 768, 48},
    {20000, 64, 5000, 768, 48},
    {6000, 64, 2000, 768, 48},
    {9000, 256, 4000, 768, 48},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {20000, 256, 6000, 1024, 48},
    {0},
    {9000, 256, 4000, 768, 48},
    {12000, 256, 5500, 768, 32},
    {20000, 256, 6000, 1024, 48},
};

/*
 * Boss-death phase sounds {program, tone, note}; the death handler uses phases
 * one through three, and the fourth record shares the trailing filler byte.
 */
DATA(0x8005617c, 0xc)
SoundRef boss_death_phase_sounds[4] = {
    {27, 1, 88},
    {27, 2, 88},
    {27, 3, 88},
    {88, 88, 88},
};

ADDRESS(0x8002ca78, 0x3c)
KfActor *actor_pool_find_free(void)
{
    KfActor *actor = actor_state.actors;
    s32 count = 127;

    do {
        if (actor->slot_state == 0xff) {
            return actor;
        }
        actor++;
    } while (count-- != 0);
    return 0;
}

/* Caller-less wrapper that discards the free slot; its intent is unresolved. */
ADDRESS(0x8002cab4, 0x20)
void func_8002cab4(void)
{
    actor_pool_find_free();
}

ADDRESS(0x8002cad4, 0x70)
void actor_set_player_transform(
    const VECTOR *position,
    const SVECTOR *rotation)
{
    if (position != 0) {
        actor_state.player_position = *position;
    }
    if (rotation != 0) {
        actor_state.player_rotation = *rotation;
    }
}

ADDRESS(0x8002cb44, 0x74)
void actor_update_cell_from_position(KfActor *actor)
{
    actor->cell_x = actor->position.vx / 2000;
    actor->cell_z = actor->position.vz / 2000;
}

ADDRESS(0x8002cbb8, 0x9c)
void actor_set_position(KfActor *actor, const struct KfVec3i *position)
{
    actor->position.vx = position->x;
    actor->position.vz = position->z;
    actor->position.vy = position->y;
    actor->cell_x = position->x / 2000;
    actor->cell_z = position->z / 2000;
}

ADDRESS(0x8002cc54, 0x10)
void actor_set_rotation(
    KfActor *actor,
    s16 x,
    s16 y,
    s16 z)
{
    actor->rotation.x = x;
    actor->rotation.y = y;
    actor->rotation.z = z;
}

ADDRESS(0x8002cc64, 0xc4)
void actor_initialize(KfActor *actor)
{
    actor->lifecycle = 1;
    actor->animation_id = 0;
    actor->animation_phase = 0;
    actor->collision_state = 0;
    actor->vertical_velocity = 0;
    actor->vertical_state = 0;
    actor->action = 0xff;
    actor->action_timer = 0xff;
    actor->health = actor_state.definitions[actor->definition_id].initial_health;
    if (actor->slot_state == 2 || actor->slot_state == 3 || actor->slot_state == 1) {
        actor->rotation.y = actor->heading_quadrant << 10;
    } else {
        actor->rotation.y = rand() >> 3;
    }
    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, 1);
}

/*
 * Both initializers reuse one temporary for the tile index and then the local
 * offset: retail keeps each local-offset load after the multiply chain that
 * still reads the tile value, which a fresh temporary would not reproduce.
 */
ADDRESS(0x8002cd28, 0xa4)
void actor_initialize_current(void)
{
    KfActor *actor = actor_state.current;
    struct KfVec3i position;
    s32 coordinate;
    s32 world;

    coordinate = actor->tile_x;
    world = coordinate * 2000;
    coordinate = actor->local_x;
    position.x = world + coordinate;
    coordinate = actor->tile_z;
    world = coordinate * 2000;
    coordinate = actor->local_z;
    position.z = world + coordinate;
    position.y = map_floor_height_at_position((const VECTOR *)&position);
    actor_set_position(actor, &position);
    actor_set_rotation(actor, 0, 0, 0);
    actor_initialize(actor);
}

ADDRESS(0x8002cdcc, 0xbc)
void actor_initialize_slot(u16 actor_index)
{
    KfActor *actor = &actor_state.actors[actor_index];
    struct KfVec3i position;
    s32 coordinate;
    s32 world;

    actor->lifecycle = 1;
    coordinate = actor->tile_x;
    world = coordinate * 2000;
    coordinate = actor->local_x;
    position.x = world + coordinate;
    coordinate = actor->tile_z;
    world = coordinate * 2000;
    coordinate = actor->local_z;
    position.z = world + coordinate;
    position.y = map_floor_height_at_position((const VECTOR *)&position);
    actor_set_position(actor, &position);
    actor_set_rotation(actor, 0, 0, 0);
    actor_initialize(actor);
}

ADDRESS(0x8002ce88, 0x40)
void actor_pool_clear(void)
{
    KfActor *actor = actor_state.actors;
    u16 index;

    for (index = 0; index < 128; index++, actor++) {
        actor->slot_state = 0xff;
        actor->lifecycle = 0;
        actor->unknown_34 = 0;
    }
}

ADDRESS(0x8002cec8, 0xc)
void actor_set_action(KfActor *actor, u8 action)
{
    actor->action = action;
    actor->action_timer = 0;
}

ADDRESS(0x8002ced4, 0xb0)
void actor_pool_spawn(
    u8 definition_id,
    const struct KfVec3i *position,
    const struct KfVec3s *rotation)
{
    KfActor *actor = actor_state.actors;
    s16 count = 127;

    do {
        if (actor->slot_state == 0xff) {
            goto found;
        }
        actor++;
    } while (--count != -1);
    return;
found:
    actor->definition_id = definition_id;
    actor->slot_state = 0;
    actor->tile_z = 0xff;
    actor->tile_x = 0xff;
    actor->variant = 0;
    actor_set_position(actor, position);
    actor_set_rotation(actor, rotation->x, rotation->y, rotation->z);
    actor_initialize(actor);
    actor_set_action(actor, 2);
}

ADDRESS(0x8002cf84, 0xf4)
void actor_pool_begin_death_by_definition(u16 definition_id)
{
    KfActor *actor = actor_state.actors;
    KfActorDefinition *definition = &actor_state.definitions[definition_id];
    s16 count = 127;

    do {
        if (actor->slot_state != 0xff && actor->definition_id == definition_id) {
            if (actor->lifecycle == 1 && definition->action_animations[KF_ACTOR_ACTION_INDEX(6)] != 0xff) {
                actor_set_action(actor, 6);
            } else {
                actor->lifecycle = 3;
            }
        }
        actor++;
    } while (--count != -1);
}

ADDRESS(0x8002d078, 0xa8)
s32 combat_calculate_damage_component(s32 base_power, s32 attack, s32 defense)
{
    s32 difference;

    if (attack == 0) {
        return 0;
    }
    attack += base_power / 5;
    difference = attack - defense;
    if (difference < 0) {
        difference = 0;
    }
    if (defense == 0) {
        defense = 1;
    }
    return difference + attack * attack / (defense * 2);
}

ADDRESS(0x8002d120, 0x388)
void actor_apply_damage(
    u16 actor_index,
    u16 base_power,
    u16 component0,
    u16 component1,
    u16 component2,
    u16 component3,
    u16 component4,
    u16 scale,
    u16 hit_flags)
{
    KfActor *actor = &actor_state.actors[actor_index];
    KfActorDefinition *definition = &actor_state.definitions[actor->definition_id];
    s32 damage;
    s32 health;
    s32 remaining;

    if (player_state.progress_state.current_floor == 5 && actor->definition_id == 7
        && DAT_8009f846 == 0) {
        return;
    }
    if (actor->health == 0) {
        return;
    }
    if (actor->action == 6 && actor->animation_phase >= 1548) {
        return;
    }
    if (actor->action == 0x7f) {
        return;
    }
    damage = combat_calculate_damage_component(
        base_power * 10, component0 * 10, definition->defenses[0] * 10);
    damage += combat_calculate_damage_component(
        base_power * 10, component1 * 10, definition->defenses[1] * 10);
    damage += combat_calculate_damage_component(
        base_power * 10, component2 * 10, definition->defenses[2] * 10);
    damage += combat_calculate_damage_component(
        base_power * 10, component3 * 10, definition->defenses[3] * 10);
    damage += combat_calculate_damage_component(
        base_power * 10, component4 * 10, definition->defenses[4] * 10);
    damage = (damage + 5) / 10;
    damage = damage * scale / 5000;
    if (damage == 0) {
        return;
    }
    hit_flags &= 0xf0;
    if (actor->health != 0 && hit_flags == 0x10) {
        if (component0 == 0 && component1 == 0 && component2 == 0) {
            player_increment_magic_training();
        } else if (component1 != 0 || component2 != 0) {
            player_increment_physical_power_training();
        }
    }
    if (actor->action == 0x12) {
        actor->vertical_state = 2;
        actor->vertical_velocity = 0;
    }
    health = actor->health;
    remaining = health - damage;
    if (remaining > 0) {
        if (definition->action_animations[KF_ACTOR_ACTION_INDEX(5)] != 0xff) {
            actor_set_action(actor, 5);
        }
    } else {
        remaining = 0;
        if (health != 0 && hit_flags == 0x10) {
            player_add_experience(definition->experience_reward);
        }
        if (definition->action_animations[KF_ACTOR_ACTION_INDEX(6)] != 0xff) {
            actor_set_action(actor, 6);
        }
    }
    actor->health = remaining;
}

ADDRESS(0x8002d4a8, 0x1f8)
void actor_pool_apply_radial_damage(
    const struct KfVec3i *origin,
    u32 radius,
    u16 falloff,
    u16 base_power,
    u16 component0,
    u16 component1,
    u16 component2,
    u16 component3,
    u16 component4,
    u16 scale,
    u16 hit_flags)
{
    s32 falloff_value = falloff;
    s32 remaining = 0x1000 - falloff_value;
    KfActor *actor = actor_state.actors;
    KfActorDefinition *definition;
    s16 index;
    s32 distance;
    u16 ratio;
    u16 weight;
    u32 damage_scale;

    for (index = 0; index < 128; index++, actor++) {
        if (actor->lifecycle != 1) {
            continue;
        }
        if (actor->action == 0x7f) {
            continue;
        }
        if (actor == actor_state.current) {
            continue;
        }
        definition = &actor_state.definitions[actor->definition_id];
        distance = actor_distance_to_point(
            actor,
            origin->x,
            origin->y,
            origin->z,
            radius,
            definition->collision_height,
            radius);
        if (distance == -1) {
            continue;
        }
        if (falloff_value == 0x1000) {
            damage_scale = scale;
        } else {
            ratio = (distance << 12) / radius;
            weight = 0x1000 - ((u32)(ratio * remaining) >> 12);
            damage_scale = (u32)(scale * weight) >> 12;
        }
        actor_apply_damage(
            index,
            base_power,
            component0,
            component1,
            component2,
            component3,
            component4,
            damage_scale,
            hit_flags);
    }
}

ADDRESS(0x8002d6a0, 0x158)
void actor_try_attack_player(
    u16 minimum_distance,
    u16 maximum_distance,
    s16 angle_offset,
    s16 angle_tolerance)
{
    KfActorDefinition *definition = actor_state.current_definition;
    KfActor *actor = actor_state.current;
    s32 distance;
    s32 angle;
    u16 status_effect;

    distance = actor_distance_to_point(
        actor,
        actor_state.player_position.vx,
        actor_state.player_position.vy + 1500,
        actor_state.player_position.vz,
        maximum_distance,
        definition->collision_height,
        1700);
    if (distance == -1) {
        return;
    }
    if (distance < minimum_distance) {
        return;
    }
    angle = vector_xz_to_angle(
        actor_state.player_position.vx - actor->position.vx,
        actor_state.player_position.vz - actor->position.vz);
    if (!angle_within_tolerance(actor->rotation.y + angle_offset, angle, angle_tolerance)) {
        return;
    }
    status_effect = 0;
    if (definition->status_effect_chance != 0
        && (rand() >> 7) < definition->status_effect_chance) {
        status_effect = definition->status_effect;
    }
    player_apply_damage(
        definition->attack_components[0],
        definition->attack_components[1],
        definition->attack_components[2],
        status_effect,
        0,
        0,
        0x1000,
        10);
}

ADDRESS(0x8002d7f8, 0x184)
KfActor *actor_pool_find_target_in_cone(
    const struct KfVec3i *origin,
    s32 facing,
    u32 max_distance,
    s32 angle_tolerance,
    s32 *distance_out)
{
    KfActor *best = 0;
    s16 best_difference = 30000;
    s32 best_distance = 0;
    KfActor *actor = actor_state.actors;
    u16 count = 127;
    s32 distance;
    s32 delta;
    s32 folded;
    s16 difference;

    do {
        if (actor->lifecycle != 1) {
            continue;
        }
        if (actor->action == 0x7f) {
            continue;
        }
        if (actor == actor_state.current) {
            continue;
        }
        distance = actor_distance_to_point(
            actor, origin->x, 0xffff, origin->z, max_distance, 0, 0);
        if (distance == -1) {
            continue;
        }
        delta = vector_xz_to_angle(
            actor->position.vx - origin->x, origin->z - actor->position.vz) - facing;
        delta &= 0xfff;
        folded = delta;
        if (delta > 2048) {
            folded = 0x1000 - delta;
        }
        difference = folded;
        if (angle_tolerance < difference) {
            continue;
        }
        if (difference < best_difference) {
            best_difference = folded;
            best = actor;
            best_distance = distance;
        }
    } while (actor++, count-- != 0);
    *distance_out = best_distance;
    return best;
}

ADDRESS(0x8002d97c, 0xf0)
s32 actor_distance_to_point(
    const KfActor *actor,
    s32 point_x,
    s32 point_y,
    s32 point_z,
    s32 max_distance,
    s32 actor_height,
    s32 point_height)
{
    s32 delta_x = actor->position.vx - point_x;
    s32 delta_z;
    s32 delta_y;
    s32 top;
    s32 distance;

    if (delta_x < -max_distance || max_distance < delta_x) {
        goto out_of_range;
    }
    delta_z = actor->position.vz - point_z;
    if (delta_z < -max_distance || max_distance < delta_z) {
        goto out_of_range;
    }
    delta_x >>= 3;
    if (point_y != 0xffff) {
        actor_height >>= 1;
        point_height >>= 1;
        top = point_y - point_height;
        point_height += actor_height;
        delta_y = (actor->position.vy - actor_height) - top;
        if (delta_y < -point_height) {
            goto out_of_range;
        }
        if (point_height < delta_y) {
            goto out_of_range;
        }
    }
    delta_z >>= 3;
    distance = SquareRoot0(delta_x * delta_x + delta_z * delta_z) << 3;
    if (max_distance < distance) {
        goto out_of_range;
    }
    return distance;
out_of_range:
    return -1;
}

ADDRESS(0x8002da6c, 0x144)
s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height)
{
    KfActor *actor = actor_state.actors;
    KfActorDefinition *definition;
    s16 index;

    for (index = 0; index < 128; index++, actor++) {
        if (actor->lifecycle != 1) {
            continue;
        }
        if (actor->action == 6) {
            continue;
        }
        if (actor->action == 0x7f) {
            continue;
        }
        if (actor == actor_state.current) {
            continue;
        }
        definition = &actor_state.definitions[actor->definition_id];
        if (actor_distance_to_point(
                actor,
                x,
                y,
                z,
                definition->collision_radius + extra_radius,
                definition->collision_height,
                point_height) != -1) {
            return index;
        }
    }
    return -1;
}

ADDRESS(0x8002dbb0, 0x8c)
void actor_bind_current(KfActor *actor)
{
    s32 index;

    actor_state.current = actor;
    index = actor - actor_state.actors;
    actor_state.current_definition = &actor_state.definitions[actor->definition_id];
    actor_state.current_index = index;
    actor_state.current_definition_id = actor->definition_id;
}

ADDRESS(0x8002dc3c, 0x34)
void actor_advance_animation_wrapped(KfActor *actor, s16 delta)
{
    if (delta < 0) {
        actor->animation_step = -delta;
    } else {
        actor->animation_step = delta;
    }
    actor->animation_phase = (actor->animation_phase + delta) & 0xfff;
}

ADDRESS(0x8002dc70, 0x5c)
void actor_advance_animation_clamped(KfActor *actor, s16 delta)
{
    s16 phase;

    if (delta < 0) {
        actor->animation_step = -delta;
    } else {
        actor->animation_step = delta;
    }
    phase = actor->animation_phase + delta;
    actor->animation_phase = phase;
    if (phase >= 4096) {
        actor->animation_phase = 0xfff;
    } else if (phase < 0) {
        actor->animation_phase = 0;
    }
}

ADDRESS(0x8002dccc, 0x30)
s32 actor_animation_crossed_phase(const KfActor *actor, u16 phase)
{
    s32 crossed = 0;

    if (phase < actor->animation_phase) {
        crossed = phase >= actor->animation_phase - actor->animation_step;
    }
    return crossed;
}

ADDRESS(0x8002dcfc, 0x98)
void actor_play_sound_at_phase(const SoundRef *sound, u16 phase)
{
    KfActor *actor = actor_state.current;

    if (!actor_animation_crossed_phase(actor, phase)) {
        return;
    }
    if (player_state.progress_state.current_floor == 5 && actor->definition_id == 7) {
        audio_play_spatial_range(
            sound, &actor->position, 0x7f, 20000, 60000);
    } else {
        audio_play_spatial_default_range(
            sound, &actor->position, 0x7f);
    }
}

ADDRESS(0x8002dd94, 0x120)
u8 actor_try_select_action_distance_facing(
    u8 action,
    s32 distance,
    u16 chance,
    u16 distance_scale)
{
    KfActor *actor = actor_state.current;
    u16 odds = chance;

    if (actor->action == action && actor->action_timer != 0xff) {
        return actor->action;
    }
    if (distance_scale * 4 < distance) {
        if (actor->slot_state == 3) {
            return 0xff;
        }
        odds >>= 4;
    }
    if (distance < distance_scale) {
        odds <<= 2;
    } else if (distance < distance_scale + distance_scale / 2) {
        odds <<= 1;
    } else {
        odds >>= 2;
    }
    if (!((rand() >> 4) < odds)) {
        return 0xff;
    }
    if (rand() < 1638) {
        return action;
    }
    if (angle_within_tolerance(
            actor->rotation.y,
            vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz),
            0x18e)) {
        return action;
    }
    return 0xff;
}

ADDRESS(0x8002deb4, 0x164)
u8 actor_try_select_ground_action(u8 action, s32 distance, u16 chance)
{
    KfActor *actor = actor_state.current;
    u16 odds = chance;
    s32 cell;
    const u8 *row;

    if (actor->action == action && actor->action_timer != 0xff) {
        return actor->action;
    }
    cell = actor->cell_z;
    row = map_floor_height_grid[cell];
    cell = actor->cell_x;
    if (-(row[cell] * 100) != actor->position.vy) {
        return 0xff;
    }
    if (actor_state.player_target == actor) {
        actor_state.player_target = 0;
        return action;
    }
    if (distance > 7000) {
        odds >>= 1;
    } else {
        if (distance < 4001) {
            return 0xff;
        }
        odds <<= 3;
    }
    if (!((rand() >> 4) < odds)) {
        return 0xff;
    }
    if (rand() < 1638) {
        return action;
    }
    if (angle_within_tolerance(
            actor->rotation.y,
            vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz),
            0x18e)) {
        return action;
    }
    return 0xff;
}

ADDRESS(0x8002e018, 0xd8)
u8 actor_try_select_facing_action(u8 action, s32 distance, u16 chance)
{
    KfActor *actor = actor_state.current;
    u16 odds = chance;

    if (actor->action == action && actor->action_timer != 0xff) {
        return actor->action;
    }
    if (distance > 11000) {
        odds >>= 4;
    } else {
        if (distance < 8000) {
            return 0xff;
        }
        odds <<= 2;
    }
    if (!((rand() >> 4) < odds)) {
        return 0xff;
    }
    if (angle_within_tolerance(
            actor->rotation.y,
            vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz),
            0x1c7)) {
        return action;
    }
    return 0xff;
}

ADDRESS(0x8002e0f0, 0x1f8)
u8 actor_try_select_profiled_action(u8 action, s32 distance, u8 profile_index, u16 chance)
{
    u16 profile = profile_index & 0x1f;
    KfActorActionProfile *weights = &actor_action_profiles[profile];
    KfActor *actor = actor_state.current;
    s32 weight;
    s32 odds;
    KfActor *candidate;
    const KfEffectRecord *record;
    s16 index;
    s16 count;

    if (actor->action == action && actor->action_timer != 0xff) {
        return actor->action;
    }
    weight = weights->near_weight;
    if (distance < weights->far_distance) {
        if (distance >= weights->near_distance) {
            weight = weights->middle_weight;
        }
    } else {
        weight = weights->far_weight;
    }
    odds = (chance * weight) >> 8;
    if (!((rand() >> 4) < odds)) {
        return 0xff;
    }
    if (!angle_within_tolerance(
            actor->rotation.y,
            vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz),
            0x155)
        && rand() >= 819) {
        return 0xff;
    }
    if (profile != 9) {
        return action;
    }
    count = 0;
    candidate = actor_state.actors;
    index = 127;
    do {
        if (candidate->slot_state != 0xff && candidate->lifecycle == 1) {
            count++;
        }
        candidate++;
    } while (--index != -1);
    record = effect_pool_records;
    index = 47;
    do {
        if (record->type != 0xff && record->kind == 9) {
            count++;
        }
        record++;
    } while (--index != -1);
    if (count < 2) {
        return action;
    }
    return 0xff;
}
