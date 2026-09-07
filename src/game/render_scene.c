#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_render.h>
#include <kf/game_state.h>

enum {
    ACTOR_CULL_SQUARE_HALF_WIDTH = 12,
    ACTOR_CULL_SQUARE_WIDTH = 2 * ACTOR_CULL_SQUARE_HALF_WIDTH,
    FLOOR_ITEM_RENDER_BRIGHTNESS = 180
};

/* Cull each pool against the active cell window before dispatching its emitter.
 * Effect records remain a temporary view of the shared pool storage.
 */

ADDRESS(0x8001f218, 0x580)
void render_entities(void)
{
    const KfCellWindow *grid = active_cell_window;
    u16 window_origin_z = (u16)render_state.view_cell.z - grid->origin_z;
    u16 window_origin_x = (u16)render_state.view_cell.x - grid->origin_x;
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    KfEffectRenderView *sprite;
    s16 i;

    tmd_select(KF_TMD_SLOT_ENTITIES);

    /* Map objects. */
    object = map_object_state.objects;
    for (i = KF_MAP_OBJECT_CAPACITY - 1; i != -1; i--) {
        if (object->object_id < KF_MAP_OBJECT_RENDER_ID_END) {
            u16 row = object->cell_z - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = object->cell_x - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_map_object(object);
                }
            }
        }
        object++;
    }

    /* Actors. */
    actor = actor_state.actors;
    for (i = KF_ACTOR_CAPACITY - 1; i != -1; i--) {
        u8 visible;
        if (actor->lifecycle != KF_ACTOR_LIFECYCLE_ACTIVE) {
            goto next_actor;
        }
        if (actor->culling_mode == KF_ACTOR_CULL_VISIBILITY_GRID) {
            u16 row = actor->cell_z - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row >= g->height) {
                goto next_actor;
            }
            {
                u16 col = actor->cell_x - window_origin_x;
                if (col >= g->width) {
                    goto next_actor;
                }
                visible = g->cells[row * g->width + col];
            }
        } else {
            u16 dz = actor->cell_z + ACTOR_CULL_SQUARE_HALF_WIDTH;
            u16 dx;
            if ((u16)(dz - (u16)render_state.view_cell.z) >= ACTOR_CULL_SQUARE_WIDTH) {
                goto next_actor;
            }
            dx = actor->cell_x + ACTOR_CULL_SQUARE_HALF_WIDTH;
            visible = (u16)(dx - (u16)render_state.view_cell.x) < ACTOR_CULL_SQUARE_WIDTH;
        }
        if (visible != 0) {
            render_actor(actor);
        }
next_actor:
        actor++;
    }

    /* Floor items. */
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_FLOOR_ITEM]);
    active_render_blue = FLOOR_ITEM_RENDER_BRIGHTNESS;
    active_render_green = FLOOR_ITEM_RENDER_BRIGHTNESS;
    active_render_red = FLOOR_ITEM_RENDER_BRIGHTNESS;
    i = floor_item_count;
    active_render_tpage = floor_item_tpage;
    active_render_clut = floor_item_clut;
    {
        KfFloorItem *items = floor_items;
        for (i--; i != -1; i--) {
            u16 row = (items->position_z / KF_MAP_TILE_SIZE) - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = (items->position_x / KF_MAP_TILE_SIZE) - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_floor_item(items);
                }
            }
            items++;
        }
    }

    /* Actor sprites. */
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_EFFECT]);
    sprite = (KfEffectRenderView *)effect_pool_records;
    for (i = KF_EFFECT_CAPACITY - 1; i != -1; i--) {
        if (sprite->type == KF_EFFECT_SLOT_FREE || sprite->sprite_id == KF_EFFECT_RENDER_NONE) {
            goto next_sprite;
        }
        {
            u16 row = (*(s32 *)&sprite->position_z / KF_MAP_TILE_SIZE) - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = (*(s32 *)&sprite->position_x / KF_MAP_TILE_SIZE) - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_actor_sprite(sprite);
                }
            }
        }
next_sprite:
        sprite++;
    }

    /* Map events. */
    SetLightMatrix(&render_state.light_matrix_copy);
    event = map_event_pool;
    for (i = KF_MAP_EVENT_CAPACITY - 1; i != -1; i--) {
        if (event->state == KF_MAP_EVENT_ACTIVE) {
            u16 row = event->cell_z - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = event->cell_x - window_origin_x;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_map_event(event);
                }
            }
        }
        event++;
    }
}
