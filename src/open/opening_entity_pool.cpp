#include <kf/lib/null.h>
#include <kf/lib/bool.h>

#include <kf/lib/map_data.h>
#include <kf/lib/math.h>
#include <kf/open/resources.h>

KfOpeningEntityState opening_entity_state;

void opening_entity_pool_reset(void)
{
    for (auto &entity : opening_entity_state.entities) {
        entity.object_id = KF_OPENING_ENTITY_FREE;
    }

    opening_entity_state.unknown_control_50e = 0;
    opening_entity_state.unknown_control_50c = 0;
    opening_entity_state.unknown_control_50a = 0;
}

KfOpeningEntity *opening_entity_find_by_object_id(
    KfOpeningEntity *entities, KfOpeningModelId object_id)
{
    for (auto *entity = entities; entity->object_id != KF_OPENING_ENTITY_FREE; entity++) {
        if (entity->object_id == object_id) {
            return entity;
        }
    }
    return NULL;
}

void opening_entity_pool_load_placements(
    const KfMapObjectPlacement *placements, s32 base_y)
{
    KfBool16 exhausted = false;
    const KfMapObjectPlacement *placement = placements;

    for (auto &entity : opening_entity_state.entities) {
        if (exhausted == true
            || kf_enum_decode<KfOpeningModelId>(placement->object_id) == KF_OPENING_ENTITY_FREE) {
            exhausted = true;
            entity.object_id = KF_OPENING_ENTITY_FREE;
        } else {
            entity.object_id = kf_enum_decode<KfOpeningModelId>(placement->object_id);
            entity.cell_x = placement->tile_x;
            entity.cell_z = placement->tile_z;
            entity.rotation.z = 0;
            entity.rotation.x = 0;
            entity.rotation.y = placement->yaw & KF_ANGLE_WRAP_MASK;
            entity.position.vx =
                map_placement_axis_position(placement->tile_x, placement->local_x);
            entity.position.vz =
                map_placement_axis_position(placement->tile_z, placement->local_z);
            entity.scale = {KF_FIXED12_ONE, KF_FIXED12_ONE, KF_FIXED12_ONE};
            if (base_y == KF_OPENING_ENTITY_FLOOR_HEIGHT) {
                entity.position.vy = placement->local_y -
                    map_floor_height_grid.cells[placement->tile_z]
                                         [placement->tile_x] *
                        KF_MAP_HEIGHT_STEP;
            } else {
                entity.position.vy = base_y + placement->local_y;
            }
            placement++;
        }
    }
}


void opening_entity_pool_reset_module_state(void)
{
    kf::restore_initial_value<opening_entity_state>();
}
