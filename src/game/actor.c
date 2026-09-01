#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorTable actor_table;

extern struct KfVec4i actor_player_position;
extern struct KfVec4s actor_player_rotation;
extern KfActor *current_actor;

extern KfPlayerProgressState player_progress_state;
/* Unresolved flag consulted before damaging definition 7 on floor 5. */
extern u8 DAT_8009f846;

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);
/* Unresolved map-cell helpers: floor height for a position, and cell marking. */
extern s32 func_8001a44c(struct KfVec3i *position);
extern void func_8001a4e8(s32 cell_x, s32 cell_z, char value);
extern void player_increment_physical_power_training(void);
extern void player_increment_magic_training(void);
extern void player_add_experience(s16 amount);

ADDRESS(0x8002ca78, 0x3c)
KfActor *actor_pool_find_free(void)
{
    KfActor *actor = actor_table.actors;
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
    const struct KfVec4i *position,
    const struct KfVec4s *rotation)
{
    if (position != 0) {
        actor_player_position = *position;
    }
    if (rotation != 0) {
        actor_player_rotation = *rotation;
    }
}

ADDRESS(0x8002cb44, 0x74)
void actor_update_cell_from_position(KfActor *actor)
{
    actor->cell_x = actor->position.x / 2000;
    actor->cell_z = actor->position.z / 2000;
}

ADDRESS(0x8002cbb8, 0x9c)
void actor_set_position(KfActor *actor, const struct KfVec3i *position)
{
    actor->position.x = position->x;
    actor->position.z = position->z;
    actor->position.y = position->y;
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
    actor->health = actor_table.definitions[actor->definition_id].initial_health;
    if (actor->slot_state == 2 || actor->slot_state == 3 || actor->slot_state == 1) {
        actor->rotation.y = actor->heading_quadrant << 10;
    } else {
        actor->rotation.y = rand() >> 3;
    }
    func_8001a4e8(actor->cell_x, actor->cell_z, 1);
}

/*
 * Both initializers reuse one temporary for the tile index and then the local
 * offset: retail keeps each local-offset load after the multiply chain that
 * still reads the tile value, which a fresh temporary would not reproduce.
 */
ADDRESS(0x8002cd28, 0xa4)
void actor_initialize_current(void)
{
    KfActor *actor = current_actor;
    struct KfVec3i position;
    s32 coordinate;
    s32 world;

    coordinate = actor->tile_z;
    world = coordinate * 2000;
    coordinate = actor->local_z;
    position.x = world + coordinate;
    coordinate = actor->tile_x;
    world = coordinate * 2000;
    coordinate = actor->local_x;
    position.z = world + coordinate;
    position.y = func_8001a44c(&position);
    actor_set_position(actor, &position);
    actor_set_rotation(actor, 0, 0, 0);
    actor_initialize(actor);
}

ADDRESS(0x8002cdcc, 0xbc)
void actor_initialize_slot(u16 actor_index)
{
    KfActor *actor = &actor_table.actors[actor_index];
    struct KfVec3i position;
    s32 coordinate;
    s32 world;

    actor->lifecycle = 1;
    coordinate = actor->tile_z;
    world = coordinate * 2000;
    coordinate = actor->local_z;
    position.x = world + coordinate;
    coordinate = actor->tile_x;
    world = coordinate * 2000;
    coordinate = actor->local_x;
    position.z = world + coordinate;
    position.y = func_8001a44c(&position);
    actor_set_position(actor, &position);
    actor_set_rotation(actor, 0, 0, 0);
    actor_initialize(actor);
}

ADDRESS(0x8002ce88, 0x40)
void actor_pool_clear(void)
{
    KfActor *actor = actor_table.actors;
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
    KfActor *actor = actor_table.actors;
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
    actor->tile_x = 0xff;
    actor->tile_z = 0xff;
    actor->variant = 0;
    actor_set_position(actor, position);
    actor_set_rotation(actor, rotation->x, rotation->y, rotation->z);
    actor_initialize(actor);
    actor_set_action(actor, 2);
}

ADDRESS(0x8002cf84, 0xf4)
void actor_pool_begin_death_by_definition(u16 definition_id)
{
    KfActor *actor = actor_table.actors;
    KfActorDefinition *definition = &actor_table.definitions[definition_id];
    s16 count = 127;

    do {
        if (actor->slot_state != 0xff && actor->definition_id == definition_id) {
            if (actor->lifecycle == 1 && definition->death_action != 0xff) {
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
    KfActor *actor = &actor_table.actors[actor_index];
    KfActorDefinition *definition = &actor_table.definitions[actor->definition_id];
    s32 damage;
    s32 health;
    s32 remaining;

    if (player_progress_state.current_floor == 5 && actor->definition_id == 7
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
        if (definition->hit_action != 0xff) {
            actor_set_action(actor, 5);
        }
    } else {
        remaining = 0;
        if (health != 0 && hit_flags == 0x10) {
            player_add_experience(definition->experience_reward);
        }
        if (definition->death_action != 0xff) {
            actor_set_action(actor, 6);
        }
    }
    actor->health = remaining;
}
