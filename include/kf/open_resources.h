#ifndef KF_OPEN_RESOURCES_H
#define KF_OPEN_RESOURCES_H

/* OPEN.EXE resource loading and its opening-only placement consumer. */

#include <kf/semantic_types.h>

extern KfOpeningEntityState opening_entity_state;

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
