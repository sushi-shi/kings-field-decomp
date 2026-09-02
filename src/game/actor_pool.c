#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorState actor_state;

extern void actor_bind_current(KfActor *actor);
extern void actor_update_awareness(void);
extern void actor_update_current_action(void);
extern s32 map_floor_height_at_position(struct KfVec3i *position);

#define ACTOR_SLOT_FREE 0xff
#define MAP_TILE_SIZE 2000

/* Runs awareness and the current action for every occupied actor slot. */
ADDRESS(0x80030818, 0xa8)
void actor_pool_update(void)
{
    KfActor *actor = actor_state.actors;
    u16 count = 0x7f;

    do {
        actor_bind_current(actor);
        if (actor->slot_state != ACTOR_SLOT_FREE) {
            actor_update_awareness();
            if (actor->lifecycle == 1) {
                actor_update_current_action();
            }
        }
        actor++;
    } while (count-- != 0);
    actor_bind_current(0);
}

/*
 * Fills the actor pool from a placement list terminated by a 0xff slot
 * state; the terminator and every slot after it are marked free.  Only the
 * placed slots advance the placement pointer.
 */
ADDRESS(0x800308c0, 0x1ac)
void actor_pool_load_placements(const KfActorPlacement *placements)
{
    s32 finished = 0;
    KfActor *actor = actor_state.actors;
    u16 count = 0x7f;

    do {
        if (finished == 1) {
            /*
             * Reached directly once the terminator has been seen; the
             * terminator slot itself arrives through the goto below and the
             * shared actor increment lives in the loop condition, so neither
             * path advances the placement pointer.
             */
        mark_free:
            actor->slot_state = ACTOR_SLOT_FREE;
            actor->lifecycle = 0;
            continue;
        }
        actor->slot_state = placements->slot_state;
        if (actor->slot_state != ACTOR_SLOT_FREE) {
            actor->definition_id = placements->definition_flags & 0x1f;
            if (placements->definition_flags & 0x20) {
                actor->variant = 1;
            } else {
                actor->variant = 0;
            }
            actor->heading_quadrant = placements->heading_quadrant;
            actor->tile_z = placements->tile_z;
            actor->tile_x = placements->tile_x;
            actor->unknown_07 = placements->unknown_05;
            actor->unknown_09 = placements->unknown_06;
            actor->local_z = placements->local_z;
            actor->local_x = placements->local_x;
            actor->lifecycle = 0;
            actor->position.vz = actor->tile_z * MAP_TILE_SIZE + actor->local_z;
            actor->position.vx = actor->tile_x * MAP_TILE_SIZE + actor->local_x;
            actor->position.vy = map_floor_height_at_position((struct KfVec3i *)&actor->position);
            actor->cell_x = actor->tile_x;
            actor->cell_z = actor->tile_z;
        } else {
            finished = 1;
            goto mark_free;
        }
        placements++;
    } while (actor++, count-- != 0);
}
