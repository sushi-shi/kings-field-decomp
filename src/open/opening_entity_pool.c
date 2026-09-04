#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/open_resources.h>

DATA(0x80049538, 0x500)
KfOpeningEntity opening_entities[32];

ADDRESS(0x80019a24, 0x180)
void opening_entity_pool_load_placements(
    const KfMapObjectPlacement *placements, s32 base_y)
{
    u16 exhausted = 0;
    const KfMapObjectPlacement *placement = placements;
    KfOpeningEntity *entity = opening_entities;
    u16 remaining = 31;

    do {
        if (exhausted == 1) {
            /* The terminator path below shares this inactive-slot store and
             * leaves the placement pointer unchanged. */
        mark_empty:
            entity->object_id = 0xff;
        } else {
            if (placement->object_id != 0xff) {
                entity->object_id = placement->object_id;
                entity->cell_x = placement->tile_x;
                entity->cell_z = placement->tile_z;
                entity->rotation.z = 0;
                entity->rotation.x = 0;
                entity->rotation.y = placement->yaw & 0xfff;
                entity->position_x =
                    placement->tile_x * 2000 + placement->local_x;
                entity->position_z =
                    placement->tile_z * 2000 + placement->local_z;
                entity->scale.vz = 0x1000;
                entity->scale.vy = 0x1000;
                entity->scale.vx = 0x1000;
                if (base_y == 0) {
                    entity->position_y = placement->local_y -
                        map_floor_height_grid[placement->tile_z]
                                             [placement->tile_x] *
                            100;
                } else {
                    entity->position_y = base_y + placement->local_y;
                }
                placement++;
            } else {
                exhausted = 1;
                goto mark_empty;
            }
        }
        entity++;
    } while (remaining-- != 0);
}
