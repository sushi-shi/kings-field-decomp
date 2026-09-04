#include <kf/address.h>
#include <kf/game_render.h>
#include <kf/game.h>

/*
 * Per-frame entity dispatcher.  It sweeps the map-object, actor, floor-item,
 * actor-sprite, and map-event pools, culls each entry against the visible map
 * cell window selected through active_cell_window, and hands survivors to
 * their emitter.
 * A dedicated light matrix is installed before the floor-item, actor-sprite,
 * and map-event passes.
 *
 * WIP: the owner of the cell-window table and floor-item render descriptor is
 * unresolved.  The descriptor block precedes floor_items in one object, so
 * floor_items is reached as a byte view offset from the descriptor base (the
 * shared-symbol addend the original produced); the actor-sprite pool stays a
 * byte view.
 *
 * Structurally exact (frame, control flow, cull arithmetic, call set, and
 * referents all match), but the seven-register global allocation differs in
 * numbering and one setup subtract lands directly in its saved register rather
 * than via a temp -- an unattributed register-allocation/scheduling residue.
 */

ADDRESS(0x8001f218, 0x580)
void render_entities(void)
{
    const KfCellWindow *grid = active_cell_window;
    int s6 = (u16)render_state.view_cell.z - grid->origin_z;
    int s5 = (u16)render_state.view_cell.x - grid->origin_x;
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    KfEffectRenderView *sprite;
    s16 i;

    tmd_select(1);

    /* Map objects. */
    object = map_object_state.objects;
    for (i = 189; i != -1; i--) {
        if (object->object_id < 133) {
            u16 row = object->cell_z - s6;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = object->cell_x - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_map_object(object);
                }
            }
        }
        object++;
    }

    /* Actors. */
    actor = actor_state.actors;
    for (i = 127; i != -1; i--, actor++) {
        u8 visible;
        if (actor->lifecycle != 1) {
            continue;
        }
        if (actor->variant == 0) {
            u16 row = actor->cell_z - s6;
            const KfCellWindow *g = active_cell_window;
            if (row >= g->height) {
                continue;
            }
            {
                u16 col = actor->cell_x - s5;
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
    DAT_8009505e = 0xb4;
    DAT_8009505d = 0xb4;
    DAT_8009505c = 0xb4;
    DAT_8009505a = DAT_8009508e;
    DAT_80095058 = DAT_8009508c;
    {
        KfFloorItem *items = (KfFloorItem *)((char *)&DAT_8009505a + 62);
        for (i = floor_item_count - 1; i != -1; i--) {
            u16 row = (items->position_z / 2000) - s6;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = (items->position_x / 2000) - s5;
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
    for (i = 47; i != -1; i--) {
        if (sprite->unknown_00[0] == 0xff || sprite->sprite_id == 0xff) {
            break;
        }
        {
            u16 row = (*(s32 *)&sprite->position_z / 2000) - s6;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = (*(s32 *)&sprite->position_x / 2000) - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_actor_sprite(sprite);
                }
            }
        }
        sprite++;
    }

    /* Map events. */
    SetLightMatrix(&render_state.light_matrix_copy);
    event = map_event_pool;
    for (i = 7; i != -1; i--) {
        if (event->state == 1) {
            u16 row = event->cell_z - s6;
            const KfCellWindow *g = active_cell_window;
            if (row < g->height) {
                u16 col = event->cell_x - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    render_map_event(event);
                }
            }
        }
        event++;
    }
}
