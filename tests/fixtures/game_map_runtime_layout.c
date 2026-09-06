#include <kf/game_map.h>

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
