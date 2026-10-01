#include <kf/platform/prelude.h>
#include <kf/game/actor.h>
#include <kf/game/game.h>
#include <kf/lib/byte_reader.h>
#include <kf/lib/codec.h>
#include <kf/lib/map_data.h>
#include <kf/lib/null.h>

#include <array>

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

void actor_pool_load_placements(KfResourceChunk chunk)
{
    std::array<KfActorPlacementData, KF_ACTOR_CAPACITY> decoded {};
    std::size_t count;
    if (kf_actor_placements_decode({chunk.data, chunk.size},
            {KF_MAP_COLUMNS, KF_ACTOR_DEFINITION_COUNT, KF_MAP_TILE_SIZE}, decoded, count) != KF_CODEC_OK)
        kf::host_fail("Invalid actor placements.");
    for (std::size_t i = 0; i < std::size(actor_state.actors); ++i) {
        auto &actor = actor_state.actors[i];
        if (i >= count) {
            actor.slot_state = KF_ACTOR_SLOT_FREE;
            actor.lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            continue;
        }
        const auto *placements = &decoded[i];
        actor.slot_state = kf_enum_decode<KfActorSlotState>(placements->slot_state);
        actor.definition_id = placements->definition_id;
        if (placements->near_square_culling) {
            actor.culling_mode = KF_ACTOR_CULL_NEAR_SQUARE;
        } else {
            actor.culling_mode = KF_ACTOR_CULL_VISIBILITY_GRID;
        }
        actor.heading_quadrant = kf_enum_decode<KfActorHeadingQuadrant>(placements->heading_quadrant);
        actor.tile_z = placements->tile_z;
        actor.tile_x = placements->tile_x;
        actor.spawn_chance = placements->spawn_chance;
        actor.death_drop_object_id = kf_enum_decode<KfObjectId>(placements->death_drop_object_id);
        actor.local_z = placements->local_z;
        actor.local_x = placements->local_x;
        actor.lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
        actor.position.vz = map_placement_axis_position(actor.tile_z, actor.local_z);
        actor.position.vx = map_placement_axis_position(actor.tile_x, actor.local_x);
        actor.position.vy = map_floor_height_at_position(&actor.position);
        actor.cell_x = actor.tile_x;
        actor.cell_z = actor.tile_z;
    }
}

void actor_definitions_load(KfResourceChunk chunk)
{
    using namespace kf::codec;
    constexpr std::size_t actor_definition_bytes = 152;
    KfActorDefinitionTable definitions {};
    if (decode([&] {
        Reader input({chunk.data, chunk.size});
        for (auto &definition : definitions.entries) {
            Reader record(input.take(actor_definition_bytes));
            definition.pursuit_distance_scale = record.byte();
            definition.model_and_texture = record.byte();
            definition.melee_attack_chance = record.byte();
            definition.status_effect = kf_enum_decode<KfPlayerStatusFlags>(record.byte());
            definition.status_effect_chance = record.byte();
            for (auto &code : definition.action_parameters.effect_codes)
                code = kf_enum_decode<KfActorEffectCode>(record.byte());
            for (auto &chance : definition.action_parameters.effect_chances)
                chance = record.byte();
            definition.action_parameters.drop_object = kf_enum_decode<KfObjectId>(record.byte());
            definition.action_parameters.drop_chance = record.byte();
            definition.move_speed = record.byte();
            for (auto &clip : definition.action_animations)
                clip = kf_enum_decode<KfAnimationClip>(record.byte());
            definition.turn_rate = record.byte();
            for (auto &sound : definition.sounds)
                sound = {record.byte(), record.byte(), record.byte()};
            for (auto &offset : definition.attachment_offsets)
                offset = {record.s16_le(), record.s16_le(), record.s16_le()};
            // Retail reuses the third attachment's x/y for special-attack parameters.
            const auto &third = definition.attachment_offsets.back();
            definition.special_attack_chance = third.x;
            definition.special_attack_range = third.y;
            for (auto &step : definition.action_animation_steps)
                step = record.u16_le();
            for (auto &phase : definition.action_animation_phases)
                phase = record.u16_le();
            definition.collision_radius = record.u16_le();
            definition.collision_height = record.u16_le();
            definition.awareness_distance = record.u16_le();
            definition.initial_health = record.u16_le();
            definition.effect_owner_id = record.u16_le();
            definition.experience_reward = record.u16_le();
            for (auto &component : definition.attack_components)
                component = record.u16_le();
            for (auto &defense : definition.defenses)
                defense = record.u16_le();
            definition.gold_drop_limit = record.u16_le();
        }
    }) != KF_CODEC_OK)
        kf::host_fail("Invalid actor definitions.");
    actor_state.definitions = definitions;
}


void actor_pool_reset_module_state(void)
{
    kf::restore_initial_value<actor_state>();
}
