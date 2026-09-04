#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/open_resources.h>

DATA(0x80049538, 0x510)
KfOpeningEntityState opening_entity_state;

ADDRESS(0x80019994, 0x4c)
void opening_entity_pool_reset(void)
{
    KfOpeningEntity *entity = opening_entity_state.entities;
    u16 remaining = 31;

    do {
        entity->object_id = 0xff;
        entity++;
    } while (remaining-- != 0);

    opening_entity_state.unknown_control_50e = 0;
    opening_entity_state.unknown_control_50c = 0;
    opening_entity_state.unknown_control_50a = 0;
}

ADDRESS(0x800199e0, 0x44)
KfOpeningEntity *opening_entity_find_by_object_id(
    KfOpeningEntity *entities, u8 object_id)
{
    KfOpeningEntity *entity = entities;

    if (entity->object_id != 0xff) {
        do {
            if (entity->object_id == object_id)
                return entity;
            entity++;
        } while (entity->object_id != 0xff);
    }

    return 0;
}

ADDRESS(0x80019a24, 0x180)
void opening_entity_pool_load_placements(
    const KfMapObjectPlacement *placements, s32 base_y)
{
    u16 exhausted = 0;
    const KfMapObjectPlacement *placement = placements;
    KfOpeningEntity *entity = opening_entity_state.entities;
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
                entity->position.vx =
                    placement->tile_x * 2000 + placement->local_x;
                entity->position.vz =
                    placement->tile_z * 2000 + placement->local_z;
                entity->scale.vz = 0x1000;
                entity->scale.vy = 0x1000;
                entity->scale.vx = 0x1000;
                if (base_y == 0) {
                    entity->position.vy = placement->local_y -
                        map_floor_height_grid[placement->tile_z]
                                             [placement->tile_x] *
                            100;
                } else {
                    entity->position.vy = base_y + placement->local_y;
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
