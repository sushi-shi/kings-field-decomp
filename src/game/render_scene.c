#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Per-frame entity dispatcher.  It sweeps the map-object, actor, floor-item,
 * actor-sprite, and map-event pools, culls each entry against the visible map
 * cell window pointed to by DAT_80095860, and hands survivors to their emitter.
 * A dedicated light matrix is installed before the floor-item, actor-sprite,
 * and map-event passes.
 *
 * WIP: the cell window and the floor-item render descriptor are unresolved.
 * The descriptor block precedes floor_items in one object, so floor_items is
 * reached as a byte view offset from the descriptor base (the shared-symbol
 * addend the original produced); the actor-sprite pool stays a byte view.
 *
 * Structurally exact (frame, control flow, cull arithmetic, call set, and
 * referents all match), but the seven-register global allocation differs in
 * numbering and one setup subtract lands directly in its saved register rather
 * than via a temp -- an unattributed register-allocation/scheduling residue.
 */

extern KfRenderState render_state;
extern KfMapObjectState map_object_state;
extern KfActorState actor_state;
extern MATRIX render_light_matrices[6];

/* Visible map cell window: dimensions, window origin, then the cell flags. */
typedef struct KfCellWindow {
    u16 width;
    u16 height;
    u16 origin_x;
    u16 origin_z;
    u8 cells[1];
} KfCellWindow;

extern KfCellWindow *DAT_80095860;

/* Floor-item render descriptor, laid out just below floor_items. */
extern u16 DAT_80095058;
extern u16 DAT_8009505a;
extern u8 DAT_8009505c;
extern u8 DAT_8009505d;
extern u8 DAT_8009505e;
extern u16 DAT_8009508c;
extern u16 DAT_8009508e;
extern u16 floor_item_count;

extern u8 DAT_8009d040[]; /* actor-sprite pool, 60-byte stride */
extern KfMapEvent map_event_pool[8];

extern void tmd_select(u16 slot);
extern void func_8001ebb8(KfMapObject *object);
extern void func_8001e9a4(KfActor *actor);
extern void func_8001ed90(KfFloorItem *item);
extern void func_8001eedc(u8 *sprite);
extern void func_8001f0c4(KfMapEvent *event);

ADDRESS(0x8001f218, 0x580)
void render_entities(void)
{
    KfCellWindow *grid = DAT_80095860;
    int s6 = (u16)render_state.view_cell.z - grid->origin_z;
    int s5 = (u16)render_state.view_cell.x - grid->origin_x;
    KfMapObject *object;
    KfActor *actor;
    KfMapEvent *event;
    u8 *sprite;
    s16 i;

    tmd_select(1);

    /* Map objects. */
    object = map_object_state.objects;
    for (i = 189; i != -1; i--) {
        if (object->object_id < 133) {
            u16 row = object->cell_z - s6;
            KfCellWindow *g = DAT_80095860;
            if (row < g->height) {
                u16 col = object->cell_x - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    func_8001ebb8(object);
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
            KfCellWindow *g = DAT_80095860;
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
            func_8001e9a4(actor);
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
            KfCellWindow *g = DAT_80095860;
            if (row < g->height) {
                u16 col = (items->position_x / 2000) - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    func_8001ed90(items);
                }
            }
            items++;
        }
    }

    /* Actor sprites. */
    SetLightMatrix(&render_light_matrices[2]);
    sprite = DAT_8009d040;
    for (i = 47; i != -1; i--) {
        if (sprite[0] == 0xff || sprite[3] == 0xff) {
            break;
        }
        {
            u16 row = (*(s32 *)(sprite + 20) / 2000) - s6;
            KfCellWindow *g = DAT_80095860;
            if (row < g->height) {
                u16 col = (*(s32 *)(sprite + 12) / 2000) - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    func_8001eedc(sprite);
                }
            }
        }
        sprite += 60;
    }

    /* Map events. */
    SetLightMatrix(&render_state.light_matrix_copy);
    event = map_event_pool;
    for (i = 7; i != -1; i--) {
        if (event->state == 1) {
            u16 row = event->cell_z - s6;
            KfCellWindow *g = DAT_80095860;
            if (row < g->height) {
                u16 col = event->cell_x - s5;
                if (col < g->width && g->cells[row * g->width + col] != 0) {
                    func_8001f0c4(event);
                }
            }
        }
        event++;
    }
}
