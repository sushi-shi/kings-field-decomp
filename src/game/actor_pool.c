#include <kf/null.h>
#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_actor.h>
#include <kf/game.h>

KF_ENUM_BEGIN(KfActorPlacementStreamState, s32)
    KF_ACTOR_PLACEMENTS_READING = 0,
    KF_ACTOR_PLACEMENTS_EXHAUSTED = 1
KF_ENUM_END(KfActorPlacementStreamState)

/* Runs awareness and the current action for every occupied actor slot. */
DATA(0x8006bd98, 0x2b48)
KfActorState actor_state;

ADDRESS(0x80030818, 0xa8)
void actor_pool_update(void)
{
    KfActor *actor = actor_state.actors;
    u16 count = KF_ACTOR_CAPACITY - 1;

    do {
        actor_bind_current(actor);
        if (actor->slot_state != KF_ACTOR_SLOT_FREE) {
            actor_update_awareness();
            if (actor->lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE) {
                actor_update_current_action();
            }
        }
        actor++;
    } while (count-- != 0);
    actor_bind_current(NULL);
}

/*
 * Fills the actor pool from a placement list terminated by a 0xff slot
 * state; the terminator and every slot after it are marked free.  Only the
 * placed slots advance the placement pointer.
 */
ADDRESS(0x800308c0, 0x1ac)
void actor_pool_load_placements(const KfActorPlacement *placements)
{
    KfActorPlacementStreamState stream_state = KF_ACTOR_PLACEMENTS_READING;
    KfActor *actor = actor_state.actors;
    u16 count = KF_ACTOR_CAPACITY - 1;

    do {
        if (stream_state == KF_ACTOR_PLACEMENTS_EXHAUSTED) {
            /*
             * Reached directly once the terminator has been seen; the
             * terminator slot itself arrives through the goto below and the
             * shared actor increment lives in the loop condition, so neither
             * path advances the placement pointer.
             */
        mark_free:
            actor->slot_state = KF_ACTOR_SLOT_FREE;
            actor->lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            continue;
        }
        actor->slot_state = placements->slot_state;
        if (actor->slot_state != KF_ACTOR_SLOT_FREE) {
            actor->definition_id = placements->definition_flags & KF_ACTOR_PLACEMENT_DEFINITION_MASK;
            if (placements->definition_flags & KF_ACTOR_PLACEMENT_NEAR_SQUARE_CULLING) {
                actor->culling_mode = KF_ACTOR_CULL_NEAR_SQUARE;
            } else {
                actor->culling_mode = KF_ACTOR_CULL_VISIBILITY_GRID;
            }
            actor->heading_quadrant = placements->heading_quadrant;
            actor->tile_z = placements->tile_z;
            actor->tile_x = placements->tile_x;
            actor->spawn_chance = placements->spawn_chance;
            actor->death_drop_object_id = placements->death_drop_object_id;
            actor->local_z = placements->local_z;
            actor->local_x = placements->local_x;
            actor->lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            actor->position.vz = actor->tile_z * KF_MAP_TILE_SIZE + actor->local_z;
            actor->position.vx = actor->tile_x * KF_MAP_TILE_SIZE + actor->local_x;
            actor->position.vy = map_floor_height_at_position(&actor->position);
            actor->cell_x = actor->tile_x;
            actor->cell_z = actor->tile_z;
        } else {
            stream_state = KF_ACTOR_PLACEMENTS_EXHAUSTED;
            goto mark_free;
        }
        placements++;
    } while (actor++, count-- != 0);
}

ADDRESS(0x80030a6c, 0x2c)
void actor_definitions_load(const KfActorDefinitionTable *definitions)
{
    const u32 *source = definitions->words;
    u32 *destination = actor_state.definitions.words;
    s32 count = sizeof actor_state.definitions / sizeof *source;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
