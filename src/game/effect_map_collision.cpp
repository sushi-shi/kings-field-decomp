#include <kf/lib/map_data.h>
#include <kf/game/collision.h>
#include <kf/game/effect.h>
#include <kf/game/game.h>

KfCellHeightRecord map_cell_height_records[KF_MAP_CELL_HEIGHT_RECORD_COUNT] = {
    {500, -2000, 1500, -1250},
    {1000, -2500, 2000, -1000},
    {0, -2500, 2000, -1000},
    {0, -2500, 1000, -1000},
    {1000, -10000, 2000, -15000},
    {0, -10000, 2000, -15000},
    {0, -10000, 1000, -15000},
};

static u32 effect_query_collision_targets(VECTOR *position, s32 radius, const KfEffectRecord *effect)
{
    switch (effect->type & KF_EFFECT_COLLISION_TARGETS_MASK) {
    default:
        return 1;
    case KF_EFFECT_COLLISION_TARGET_ACTORS:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_PLAYER
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
    case KF_EFFECT_COLLISION_TARGET_PLAYER:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_ACTORS
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
    case KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_MAP_OBJECTS
                | KF_COLLISION_SKIP_MAP_EVENTS);
    }
}

static bool effect_within_height_rectangle(const KfCellHeightRecord *record,
    KfMapOrientation orientation, s32 subx, s32 subz)
{
    s16 coordinate;
    switch (orientation) {
    case KF_MAP_ORIENT_UNROTATED:
        coordinate = subz;
        break;
    case KF_MAP_ORIENT_QUARTER_TURN:
        coordinate = subx;
        break;
    case KF_MAP_ORIENT_HALF_TURN:
        coordinate = KF_MAP_TILE_SIZE - subz;
        break;
    case KF_MAP_ORIENT_THREE_QUARTER_TURN:
        coordinate = KF_MAP_TILE_SIZE - subx;
        break;
    default:
        return false;
    }
    return coordinate >= record->x_min && coordinate <= record->x_max;
}

static inline u32 effect_collision_in_cell(
    VECTOR *position, s32 radius, s16 x, s16 z, s32 subz,
    KfEffectRecord *effect)
{
    KfCellHeightRecord *record;
    s32 subx;
    s32 y;
    s32 floor;
    s32 height;
    KfMapAttribute attr;

    y = position->vy;
    floor = map_floor_height_grid.cells[z][x] * -KF_MAP_HEIGHT_STEP;
    if (floor < y) {
        return KF_COLLISION_TERRAIN;
    }
    attr = map_cell_attribute_grid.cells[z][x];
    if (attr != KF_MAP_ATTRIBUTE_NONE) {
        height = map_cell_attribute_height_table[kf_enum_encode<u8>(attr)];
        if (height < 0) {
            height += floor;
            if (y < height) {
                return KF_COLLISION_TERRAIN;
            }
        } else {
            record = &map_cell_height_records[height];
            if (floor + record->y_min <= y && y <= floor + record->y_max) {
                const KfMapOrientation orientation = map_cell_orientation_grid.cells[z][x];
                subx = position->vx % KF_MAP_TILE_SIZE;
                if (effect_within_height_rectangle(record, orientation, subx, subz)) {
                    return effect_query_collision_targets(position, radius, effect);
                }
            }
        }
    }

    switch (map_collision_grid.cells[z][x]) {
    case KF_MAP_CELL_BLOCKED:
        if ((!MAP_CELL_HAS_FULL_FLOOR(map_collision_grid.cells[z + 1][x])
                || position->vz % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER) &&
            (!MAP_CELL_HAS_FULL_FLOOR(map_collision_grid.cells[z - 1][x])
                || KF_MAP_TILE_CENTER < position->vz % KF_MAP_TILE_SIZE) &&
            (!MAP_CELL_HAS_FULL_FLOOR(map_collision_grid.cells[z][x + 1])
                || position->vx % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER)) {
            if (!MAP_CELL_HAS_FULL_FLOOR(map_collision_grid.cells[z][x - 1])) {
                return KF_COLLISION_TERRAIN;
            }
            if (KF_MAP_TILE_CENTER < position->vx % KF_MAP_TILE_SIZE) {
                return KF_COLLISION_TERRAIN;
            }
        }
        break;
    case KF_MAP_CELL_X_GE_Z:
        if (position->vx % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER
                >= position->vz % KF_MAP_TILE_SIZE) {
            break;
        }
        return KF_COLLISION_TERRAIN;
    case KF_MAP_CELL_SUM_LE_SIZE:
        if ((KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER)
                < position->vx % KF_MAP_TILE_SIZE + position->vz % KF_MAP_TILE_SIZE) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_Z_GE_X:
        if (position->vx % KF_MAP_TILE_SIZE
                > position->vz % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_SUM_GE_SIZE:
        if (position->vx % KF_MAP_TILE_SIZE + position->vz % KF_MAP_TILE_SIZE
                < KF_MAP_TILE_CENTER) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    }

    return effect_query_collision_targets(position, radius, effect);
}

u32 effect_map_collision(VECTOR *position, s32 radius)
{
    KfEffectRecord *effect;
    s16 x;
    s16 z;
    s32 subz;

    x = position->vx / KF_MAP_TILE_SIZE;
    z = position->vz / KF_MAP_TILE_SIZE;
    subz = position->vz % KF_MAP_TILE_SIZE;
    effect = current_effect;
    if (x < 0 || x >= KF_MAP_COLUMNS || z < 0 || z >= KF_MAP_ROWS) {
        return KF_COLLISION_TERRAIN;
    }
    return effect_collision_in_cell(position, radius, x, z, subz, effect);
}


void effect_map_collision_reset_module_state(void)
{
    kf::restore_initial_value<map_cell_height_records>();
}
