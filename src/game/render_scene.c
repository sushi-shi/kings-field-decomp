#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_render.h>
#include <kf/game_state.h>

enum {
    ACTOR_CULL_SQUARE_HALF_WIDTH = 12,
    ACTOR_CULL_SQUARE_WIDTH = 2 * ACTOR_CULL_SQUARE_HALF_WIDTH,
    FLOOR_ITEM_RENDER_BRIGHTNESS = 180
};

/* Cull each pool against the active cell window before dispatching its emitter. */

ADDRESS(0x8001f218, 0x580)
void render_entities(void)
{
    const KfCellWindow *grid = game_graphics_runtime.active_cell_window;
    u16 window_origin_z = (u16)game_graphics_runtime.render_state.view_cell.z - grid->origin_z;
    u16 window_origin_x = (u16)game_graphics_runtime.render_state.view_cell.x - grid->origin_x;
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    KfEffectRecord *sprite;
    s16 i;
    u16 *active_tpage;
    u16 row;
    u16 col;

    tmd_select(KF_TMD_SLOT_ENTITIES);

    /* Map objects. */
    object = map_object_state.objects;
    for (i = KF_MAP_OBJECT_CAPACITY - 1; i != -1; i--) {
        if (object->object_id < KF_MAP_OBJECT_RENDER_ID_END) {
            const KfCellWindow *g;

            row = object->cell_z - window_origin_z;
            g = game_graphics_runtime.active_cell_window;
            if (row < g->height) {
                col = object->cell_x - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != KF_CELL_WINDOW_HIDDEN) {
                    render_map_object(object);
                }
            }
        }
        object++;
    }

    /* Actors. */
    actor = actor_state.actors;
    for (i = KF_ACTOR_CAPACITY - 1; i != -1; actor++, i--) {
        u8 visible;
        if (actor->lifecycle != KF_ACTOR_LIFECYCLE_ACTIVE) {
            continue;
        }
        if (actor->culling_mode == KF_ACTOR_CULL_VISIBILITY_GRID) {
            const KfCellWindow *g;

            row = actor->cell_z - window_origin_z;
            g = game_graphics_runtime.active_cell_window;
            if (row >= g->height) {
                continue;
            }
            {
                col = actor->cell_x - window_origin_x;
                if (col >= g->width) {
                    continue;
                }
                /* This join also carries the square-culling predicate below. */
                visible = KF_ENUM_ENCODE(u8, g->cells[row * g->width + col]);
            }
        } else {
            row = actor->cell_z + ACTOR_CULL_SQUARE_HALF_WIDTH;
            row -= (u16)game_graphics_runtime.render_state.view_cell.z;
            if (row >= ACTOR_CULL_SQUARE_WIDTH) {
                continue;
            }
            col = actor->cell_x + ACTOR_CULL_SQUARE_HALF_WIDTH;
            col -= (u16)game_graphics_runtime.render_state.view_cell.x;
            visible = col < ACTOR_CULL_SQUARE_WIDTH;
        }
        if (visible != 0) {
            render_actor(actor);
        }
    }

    /* Floor items. */
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_FLOOR_ITEM]);
    active_tpage = &game_graphics_runtime.active_render_tpage;
    game_graphics_runtime.active_render_color.b = FLOOR_ITEM_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.g = FLOOR_ITEM_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.r = FLOOR_ITEM_RENDER_BRIGHTNESS;
    i = game_graphics_runtime.floor_item_count;
    *active_tpage = game_graphics_runtime.floor_item_tpage;
    game_graphics_runtime.active_render_clut = game_graphics_runtime.floor_item_clut;
    {
        KfFloorItem *items = game_graphics_runtime.floor_items;
        for (i--; i != -1; i--) {
            const KfCellWindow *g;

            row = (items->position_z / KF_MAP_TILE_SIZE) - window_origin_z;
            g = game_graphics_runtime.active_cell_window;
            if (row < g->height) {
                col = (items->position_x / KF_MAP_TILE_SIZE) - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != KF_CELL_WINDOW_HIDDEN) {
                    render_floor_item(items);
                }
            }
            items++;
        }
    }

    /* Actor sprites. */
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_EFFECT]);
    sprite = effect_pool_records;
    for (i = KF_EFFECT_CAPACITY - 1; i != -1; sprite++, i--) {
        if (sprite->type == KF_EFFECT_SLOT_FREE || sprite->render_id.model == KF_EFFECT_MODEL_NONE) {
            continue;
        }
        {
            const KfCellWindow *g;

            row = (sprite->position.vz / KF_MAP_TILE_SIZE) - window_origin_z;
            g = game_graphics_runtime.active_cell_window;
            if (row < g->height) {
                col = (sprite->position.vx / KF_MAP_TILE_SIZE) - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != KF_CELL_WINDOW_HIDDEN) {
                    render_actor_sprite(sprite);
                }
            }
        }
    }

    /* Map events. */
    SetLightMatrix(&game_graphics_runtime.render_state.light_matrix_copy);
    event = map_event_pool;
    for (i = KF_MAP_EVENT_CAPACITY - 1; i != -1; i--) {
        if (event->state == KF_MAP_EVENT_ACTIVE) {
            const KfCellWindow *g;

            row = event->cell_z - window_origin_z;
            g = game_graphics_runtime.active_cell_window;
            if (row < g->height) {
                col = event->cell_x - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != KF_CELL_WINDOW_HIDDEN) {
                    render_map_event(event);
                }
            }
        }
        event++;
    }
}
