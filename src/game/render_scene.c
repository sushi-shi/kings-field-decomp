#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_render.h>
#include <kf/game_state.h>

/* Cull each pool against the active cell window before dispatching its emitter.
 * Effect records remain a temporary view of the shared pool storage.
 */

ADDRESS(0x8001f218, 0x580)
void render_entities(void)
{
    const KfCellWindow *grid = active_cell_window;
    int window_origin_z = (u16)render_state.view_cell.z - grid->origin_z;
    int window_origin_x = (u16)render_state.view_cell.x - grid->origin_x;
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    KfEffectRenderView *sprite;
    s16 i;

    tmd_select(KF_TMD_SLOT_ENTITIES);

    /* Map objects. */
    object = map_object_state.objects;
    for (i = KF_MAP_OBJECT_CAPACITY - 1; i != -1; i--) {
        if (object->object_id < 133) {
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
    for (i = KF_ACTOR_CAPACITY - 1; i != -1; i--, actor++) {
        u8 visible;
        if (actor->lifecycle != 1) {
            continue;
        }
        if (actor->variant == 0) {
            u16 row = actor->cell_z - window_origin_z;
            const KfCellWindow *g = active_cell_window;
            if (row >= g->height) {
                continue;
            }
            {
                u16 col = actor->cell_x - window_origin_x;
                if (col >= g->width) {
                    continue;
                }
                visible = g->cells[row * g->width + col];
            }
        } else {
            u16 dz = actor->cell_z + 12;
            u16 dx;
            if ((u16)(dz - (u16)render_state.view_cell.z) >= 24) {
                continue;
            }
            dx = actor->cell_x + 12;
            visible = (u16)(dx - (u16)render_state.view_cell.x) < 24;
        }
        if (visible != 0) {
            render_actor(actor);
        }
    }

    /* Floor items. */
    SetLightMatrix(&render_light_matrices[1]);
    active_render_blue = 0xb4;
    active_render_green = 0xb4;
    active_render_red = 0xb4;
    active_render_tpage = DAT_8009508e;
    active_render_clut = DAT_8009508c;
    {
        KfFloorItem *items = floor_items;
        for (i = floor_item_count - 1; i != -1; i--) {
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
    SetLightMatrix(&render_light_matrices[2]);
    sprite = (KfEffectRenderView *)effect_pool_records;
    for (i = KF_EFFECT_CAPACITY - 1; i != -1; i--, sprite++) {
        if (sprite->type == KF_EFFECT_SLOT_FREE || sprite->sprite_id == 0xff) {
            continue;
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
    }

    /* Map events. */
    SetLightMatrix(&render_state.light_matrix_copy);
    event = map_event_pool;
    for (i = KF_MAP_EVENT_CAPACITY - 1; i != -1; i--) {
        if (event->state == 1) {
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
