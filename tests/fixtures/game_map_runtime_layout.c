#include <kf/game_map.h>
#include <kf/game_actor.h>
#include <kf/game_equipment.h>
#include <kf/magic.h>

#ifndef EXPECTED_MAP_RUNTIME_SIZE
#define EXPECTED_MAP_RUNTIME_SIZE 0x2360
#endif

#define FIELD_OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, offset) \
    typedef char type##_##field##_offset[FIELD_OFFSET(type, field) == offset ? 1 : -1]

typedef char map_runtime_size[
    sizeof(KfMapRuntimeState) == EXPECTED_MAP_RUNTIME_SIZE ? 1 : -1];
typedef char world_state_size[
    sizeof(((KfMapRuntimeState *)0)->world_state) == 0x2134 ? 1 : -1];
typedef char map_runtime_alignment[__alignof__(KfMapRuntimeState) == 4 ? 1 : -1];
CHECK_OFFSET(KfMapRuntimeState, events, 0x000);
CHECK_OFFSET(KfMapRuntimeState, current_event, 0x220);
CHECK_OFFSET(KfMapRuntimeState, variant_asset_buffer, 0x224);
CHECK_OFFSET(KfMapRuntimeState, dialogue_advance_gate, 0x228);
CHECK_OFFSET(KfMapRuntimeState, ambient_script_countdown, 0x22a);
CHECK_OFFSET(KfMapRuntimeState, world_state, 0x22c);

/* Canonical owners retain complete extents without alternate copy arrays. */
#define CHECK_SIZE(type, size) typedef char type##_size[sizeof(type) == size ? 1 : -1]
CHECK_SIZE(KfWeaponTable, 704);
CHECK_SIZE(KfArmorTable, 1176);
CHECK_SIZE(KfActorDefinitionTable, 1824);
CHECK_SIZE(KfMapObjectDefinitionTable, 1280);
CHECK_SIZE(KfMagicTable, 480);
CHECK_SIZE(KfMapGrid, 10000);
CHECK_SIZE(KfMapAttributeGrid, 10000);
CHECK_SIZE(KfMapCollisionGrid, 10000);
CHECK_SIZE(KfMapOrientationGrid, 10000);
CHECK_SIZE(KfMapSavedWorld, 8500);
CHECK_SIZE(KfMapSavedFloor, 1700);
CHECK_OFFSET(KfWeaponTable, entries, 0);
CHECK_OFFSET(KfArmorTable, entries, 0);
CHECK_OFFSET(KfActorDefinitionTable, entries, 0);
CHECK_OFFSET(KfMapObjectDefinitionTable, entries, 0);
CHECK_OFFSET(KfMagicTable, entries, 0);
CHECK_OFFSET(KfMapGrid, cells, 0);
CHECK_OFFSET(KfMapAttributeGrid, cells, 0);
CHECK_OFFSET(KfMapCollisionGrid, cells, 0);
CHECK_OFFSET(KfMapOrientationGrid, cells, 0);
CHECK_OFFSET(KfMapSavedWorld, floors, 0);
