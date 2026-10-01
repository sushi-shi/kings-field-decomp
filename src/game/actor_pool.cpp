#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/lib/null.h>

#include <kf/game/map_data.h>
#include <kf/game/actor.h>
#include <kf/game/game.h>

enum class KfActorPlacementStreamState : s32 {
    KF_ACTOR_PLACEMENTS_READING = 0,
    KF_ACTOR_PLACEMENTS_EXHAUSTED = 1
}; using enum KfActorPlacementStreamState;


kf::FrameTask<void> actor_pool_update(WorldState &world, PlayerContext &player)
{
    for (auto &actor : world.actors.actors) {
        actor_bind_current(world, &actor);
        if (actor.slot_state != KF_ACTOR_SLOT_FREE) {
            auto *nearest = world.party.enabled ? party_nearest_player(world, actor.position) : &player;
            if (!nearest) continue;
            if (world.party.enabled) {
                VECTOR position;
                SVECTOR rotation;
                player_update_transform_snapshot(*nearest, &position, &rotation);
                actor_set_player_transform(world, &position, &rotation);
            }
            actor_update_awareness(world, *nearest);
            if (actor.lifecycle == KF_ACTOR_LIFECYCLE_ACTIVE) {
                auto *target = world.party.enabled ? party_actor_target(world, actor, *nearest) : &player;
                if (world.party.enabled) {
                    VECTOR position;
                    SVECTOR rotation;
                    player_update_transform_snapshot(*target, &position, &rotation);
                    actor_set_player_transform(world, &position, &rotation);
                }
                (co_await actor_update_current_action(world, *target));
            }
        }
    }
    actor_bind_current(world, NULL);
}

s32 actor_random_next(WorldState &world)
{
    if (!world.party.enabled) return kf::random_next();
    if (!world.actors.current) kf::host_fail("Actor random draw without an actor");
    return kf::random_next(world.actors.current->random);
}

void actor_pool_load_placements(WorldState &world, const KfActorPlacement *placements)
{
    KfActorPlacementStreamState stream_state = KF_ACTOR_PLACEMENTS_READING;

    for (auto &actor : world.actors.actors) {
        if (stream_state == KF_ACTOR_PLACEMENTS_EXHAUSTED
            || placements->slot_state == KF_ACTOR_SLOT_FREE) {
            stream_state = KF_ACTOR_PLACEMENTS_EXHAUSTED;
            actor.slot_state = KF_ACTOR_SLOT_FREE;
            actor.lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            continue;
        }
        actor.slot_state = placements->slot_state;
        actor.target_player_slot = no_player;
        actor.random.state = 1u + static_cast<u32>(&actor - world.actors.actors) * 2654435761u + world.epoch;
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
        actor.position.vy = map_floor_height_at_position(world, &actor.position);
        actor.cell_x = actor.tile_x;
        actor.cell_z = actor.tile_z;
        placements++;
    }
}

void actor_definitions_load(WorldState &world, const KfActorDefinitionTable *definitions)
{
    world.actors.definitions = *definitions;
}


void actor_pool_reset_module_state(void)
{
}
