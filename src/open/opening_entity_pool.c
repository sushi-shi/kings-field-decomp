#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_math.h>
#include <kf/open_resources.h>

DATA(0x80049538, 0x510)
KfOpeningEntityState opening_entity_state;

ADDRESS(0x80019994, 0x4c)
void opening_entity_pool_reset(void)
{
    KfOpeningEntity *entity = opening_entity_state.entities;
    u16 remaining = KF_OPENING_ENTITY_CAPACITY - 1;

    do {
        entity->object_id = KF_OPENING_ENTITY_FREE;
        entity++;
    } while (remaining-- != 0);

    opening_entity_state.unknown_control_50e = 0;
    opening_entity_state.unknown_control_50c = 0;
    opening_entity_state.unknown_control_50a = 0;
}

ADDRESS(0x800199e0, 0x44)
KfOpeningEntity *opening_entity_find_by_object_id(
    KfOpeningEntity *entities, KfOpeningModelId object_id)
{
    KfOpeningEntity *entity = entities;

    if (entity->object_id != KF_OPENING_ENTITY_FREE) {
        do {
            if (entity->object_id == object_id)
                return entity;
            entity++;
        } while (entity->object_id != KF_OPENING_ENTITY_FREE);
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
    u16 remaining = KF_OPENING_ENTITY_CAPACITY - 1;

    do {
        if (exhausted == 1) {
            /* The terminator path below shares this inactive-slot store and
             * leaves the placement pointer unchanged. */
        mark_empty:
            entity->object_id = KF_OPENING_ENTITY_FREE;
        } else {
            if (KF_ENUM_DECODE(KfOpeningModelId, placement->object_id) != KF_OPENING_ENTITY_FREE) {
                entity->object_id = KF_ENUM_DECODE(KfOpeningModelId, placement->object_id);
                entity->cell_x = placement->tile_x;
                entity->cell_z = placement->tile_z;
                entity->rotation.z = 0;
                entity->rotation.x = 0;
                entity->rotation.y = placement->yaw & KF_ANGLE_WRAP_MASK;
                entity->position.vx =
                    placement->tile_x * KF_MAP_TILE_SIZE + placement->local_x;
                entity->position.vz =
                    placement->tile_z * KF_MAP_TILE_SIZE + placement->local_z;
                entity->scale.vz = KF_FIXED12_ONE;
                entity->scale.vy = KF_FIXED12_ONE;
                entity->scale.vx = KF_FIXED12_ONE;
                if (base_y == KF_OPENING_ENTITY_FLOOR_HEIGHT) {
                    entity->position.vy = placement->local_y -
                        map_floor_height_grid.cells[placement->tile_z]
                                             [placement->tile_x] *
                            KF_MAP_HEIGHT_STEP;
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
