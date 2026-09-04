#ifndef KF_ITEM_H
#define KF_ITEM_H

/* Floor-item resource loading shared by GAME and OPEN. */

#include <kf/semantic_types.h>

extern u16 floor_item_count;
extern KfFloorItem floor_items[64];

extern void item_load_floor_placements(KfFloorItemPlacement *placements);

#endif
