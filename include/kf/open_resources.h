#ifndef KF_OPEN_RESOURCES_H
#define KF_OPEN_RESOURCES_H

/* OPEN.EXE resource loading and its opening-only placement consumer. */

#include <kf/game_map.h>

/* OPEN title-scene entity and its 32-slot runtime pool. */
typedef struct KfOpeningEntity {
    u8 object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    VECTOR position;
    struct KfEulerAngles rotation;
    u16 unknown_1e;
    SVECTOR scale;
} KfOpeningEntity;

typedef struct KfOpeningEntityState {
    KfOpeningEntity entities[32];
    u8 unknown_500[10];
    u16 unknown_control_50a;
    u16 unknown_control_50c;
    u16 unknown_control_50e;
} KfOpeningEntityState;

extern KfOpeningEntityState opening_entity_state;
extern u16 floor_item_count;

extern void opening_entity_pool_reset(void);
extern KfOpeningEntity *opening_entity_find_by_object_id(
    KfOpeningEntity *entities, u8 object_id);
extern void opening_entity_pool_load_placements(
    const KfMapObjectPlacement *placements, s32 base_y);

extern void opening_resources_load_scene0(void);
extern void opening_resources_load_scene1(void);
extern void opening_resources_load_scene3(void);
extern void opening_resources_load_ending(void);
extern void opening_resources_load_ending_entities(void);
extern void opening_resources_load_ending_sequence(void);
#endif
