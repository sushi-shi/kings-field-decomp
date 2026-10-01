#include <kf/game/world.h>
#include <kf/game/party_runtime.h>
#include <kf/game/graphics.h>

#include <kf/lib/map_data.h>
#include <kf/game/render.h>
#include <kf/game/state.h>
#include <kf/game/player_actions.h>
#include <kf/platform/prelude.h>

enum {
    ACTOR_CULL_SQUARE_HALF_WIDTH = 12,
    ACTOR_CULL_SQUARE_WIDTH = 2 * ACTOR_CULL_SQUARE_HALF_WIDTH
};

struct RenderCellOrigin {
    u16 x;
    u16 z;
};

static bool render_cell_is_visible(s32 cell_x, s32 cell_z, RenderCellOrigin origin)
{
    // Narrow before checking bounds, including cells before the window origin.
    const u16 row = cell_z - origin.z;
    const auto *grid = game_graphics_runtime.active_cell_window;
    if (row >= grid->height) {
        return false;
    }
    const u16 col = cell_x - origin.x;
    return col < grid->width && grid->cells[row * grid->width + col] != KF_CELL_WINDOW_HIDDEN;
}

static bool render_actor_is_visible(const KfActor &actor, RenderCellOrigin origin)
{
    if (actor.culling_mode == KF_ACTOR_CULL_VISIBILITY_GRID) {
        return render_cell_is_visible(actor.cell_x, actor.cell_z, origin);
    }

    u16 row = actor.cell_z + ACTOR_CULL_SQUARE_HALF_WIDTH;
    row -= game_graphics_runtime.render_state.view_cell.z;
    if (row >= ACTOR_CULL_SQUARE_WIDTH) {
        return false;
    }
    u16 col = actor.cell_x + ACTOR_CULL_SQUARE_HALF_WIDTH;
    col -= game_graphics_runtime.render_state.view_cell.x;
    return col < ACTOR_CULL_SQUARE_WIDTH;
}

void presentation_advance_floor_items()
{
    const auto *grid = game_graphics_runtime.active_cell_window;
    if (!grid) return;
    const RenderCellOrigin origin {
        static_cast<u16>(game_graphics_runtime.render_state.view_cell.x - grid->origin_x),
        static_cast<u16>(game_graphics_runtime.render_state.view_cell.z - grid->origin_z)};
    for (s16 index = 0; index < game_graphics_runtime.floor_item_count; ++index) {
        auto &item = game_graphics_runtime.floor_items[index];
        if (render_cell_is_visible(item.position_x / KF_MAP_TILE_SIZE,
                                  item.position_z / KF_MAP_TILE_SIZE, origin))
            floor_item_advance_frame(&item);
    }
}

void render_entities(WorldState &world)
{
    const auto *grid = game_graphics_runtime.active_cell_window;
    const u16 window_origin_z = game_graphics_runtime.render_state.view_cell.z - grid->origin_z;
    const u16 window_origin_x = game_graphics_runtime.render_state.view_cell.x - grid->origin_x;
    const RenderCellOrigin origin = {window_origin_x, window_origin_z};

    tmd_select(tmd_context(), KF_TMD_SLOT_ENTITIES);

    for (auto &object : world.objects.objects) {
        const auto index = &object - world.objects.objects.data();
        auto pose = party_present_entity(world.presentation ? &world.presentation->objects[index] : nullptr,
            party_object_pose(object));
        if (pose.active && (render_cell_is_visible(object.cell_x, object.cell_z, origin) ||
            (world.presentation && render_cell_is_visible(pose.position.vx / KF_MAP_TILE_SIZE, pose.position.vz / KF_MAP_TILE_SIZE, origin)))) {
            if (!player_loot_visible(world, world.party.local_slot, index)) continue;
            const auto *actions = world.party.members[world.party.local_slot].player.actions;
            if (world.party.enabled && actions && actions->container == index && actions->container_generation == object.generation) {
                pose.rotation.vx = actions->container_pitch;
            }
            render_map_object(world, &object, pose);
        }
    }

    for (auto &actor : world.actors.actors) {
        const auto index = &actor - world.actors.actors.data();
        const auto pose = party_present_entity(world.presentation ? &world.presentation->actors[index] : nullptr,
            party_actor_pose(actor));
        if (pose.active && (render_actor_is_visible(actor, origin) ||
            (world.presentation && render_cell_is_visible(pose.position.vx / KF_MAP_TILE_SIZE, pose.position.vz / KF_MAP_TILE_SIZE, origin)))) {
            render_actor(world, &actor, pose);
        }
    }

    game_graphics_runtime.active_render_color.b = KF_FLOOR_ITEM_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.g = KF_FLOOR_ITEM_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.r = KF_FLOOR_ITEM_RENDER_BRIGHTNESS;
    const s16 item_count = game_graphics_runtime.floor_item_count;
    game_graphics_runtime.active_render_material = game_graphics_runtime.floor_item_material;
    auto *items = game_graphics_runtime.floor_items.data();
    for (s16 index = 0; index < item_count; index++) {
        auto &item = items[index];
        if (render_cell_is_visible(
                item.position_x / KF_MAP_TILE_SIZE, item.position_z / KF_MAP_TILE_SIZE, origin)) {
            render_floor_item(game_graphics_runtime.render_state, floor_item_sprites,
                render_enqueue_sprite, &item, &render_light_matrices[KF_RENDER_LIGHT_FLOOR_ITEM]);
        }
    }

    for (auto &effect : world.effects.records) {
        if (effect.type == KF_EFFECT_SLOT_FREE) {
            continue;
        }
        const auto index = &effect - world.effects.records.data();
        const auto pose = party_present_entity(world.presentation ? &world.presentation->effects[index] : nullptr,
            party_effect_pose(effect));
        if (render_cell_is_visible(pose.position.vx / KF_MAP_TILE_SIZE, pose.position.vz / KF_MAP_TILE_SIZE, origin)) {
            PartyEffectAppearance appearance;
            if (party_present_effect(world.presentation ? &world.presentation->effect_visuals[index] : nullptr,
                    effect, appearance))
                render_effect(&effect, &render_light_matrices[KF_RENDER_LIGHT_EFFECT], pose, appearance);
        }
    }

    for (auto &event : world.map.events) {
        const auto index = &event - world.map.events.data();
        const auto pose = party_present_entity(world.presentation ? &world.presentation->events[index] : nullptr,
            party_event_pose(event));
        if (pose.active && (render_cell_is_visible(event.cell_x, event.cell_z, origin) ||
            (world.presentation && render_cell_is_visible(pose.position.vx / KF_MAP_TILE_SIZE, pose.position.vz / KF_MAP_TILE_SIZE, origin)))) {
            render_map_event(&event, &game_graphics_runtime.map_event_light_matrix, pose);
        }
    }
}
