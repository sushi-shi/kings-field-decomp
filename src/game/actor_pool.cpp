#include <kf/lib/null.h>

#include <kf/lib/map_data.h>
#include <kf/game/actor.h>
#include <kf/game/game.h>

enum class KfActorPlacementStreamState : s32 {
    KF_ACTOR_PLACEMENTS_READING = 0,
    KF_ACTOR_PLACEMENTS_EXHAUSTED = 1
}; using enum KfActorPlacementStreamState;

KfActorState actor_state;

void actor_pool_update(void)
{
    for (auto &actor : actor_state.actors) {
        actor_bind_current(&actor);
        if (actor.slot_state != KF_ACTOR_SLOT_FREE) {
            actor_update_awareness();
            if (actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE) {
                actor_update_current_action();
            }
        }
    }
    actor_bind_current(NULL);
}

void actor_pool_load_placements(const KfActorPlacement *placements)
{
    KfActorPlacementStreamState stream_state = KF_ACTOR_PLACEMENTS_READING;

    for (auto &actor : actor_state.actors) {
        if (stream_state == KF_ACTOR_PLACEMENTS_EXHAUSTED
            || placements->slot_state == KF_ACTOR_SLOT_FREE) {
            stream_state = KF_ACTOR_PLACEMENTS_EXHAUSTED;
            actor.slot_state = KF_ACTOR_SLOT_FREE;
            actor.lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            continue;
        }
        actor.slot_state = placements->slot_state;
        actor.definition_id = placements->definition_flags & KF_ACTOR_PLACEMENT_DEFINITION_MASK;
        if (placements->definition_flags & KF_ACTOR_PLACEMENT_NEAR_SQUARE_CULLING) {
            actor.culling_mode = KF_ACTOR_CULL_NEAR_SQUARE;
        } else {
            actor.culling_mode = KF_ACTOR_CULL_VISIBILITY_GRID;
        }
        actor.heading_quadrant = placements->heading_quadrant;
        actor.tile_z = placements->tile_z;
        actor.tile_x = placements->tile_x;
        actor.spawn_chance = placements->spawn_chance;
        actor.death_drop_object_id = placements->death_drop_object_id;
        actor.local_z = placements->local_z;
        actor.local_x = placements->local_x;
        actor.lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
        actor.position.vz = map_placement_axis_position(actor.tile_z, actor.local_z);
        actor.position.vx = map_placement_axis_position(actor.tile_x, actor.local_x);
        actor.position.vy = map_floor_height_at_position(&actor.position);
        actor.cell_x = actor.tile_x;
        actor.cell_z = actor.tile_z;
        placements++;
    }
}

void actor_definitions_load(const KfActorDefinitionTable *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)&actor_state.definitions;
    s32 count = sizeof actor_state.definitions / sizeof *source;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}


void actor_pool_reset_module_state(void)
{
    kf::restore_initial_value<actor_state>();
}
