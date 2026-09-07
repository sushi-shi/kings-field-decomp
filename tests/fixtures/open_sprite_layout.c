#include <kf/item.h>
#include <kf/render_types.h>

#ifndef EXPECTED_SPRITE_SIZE
#define EXPECTED_SPRITE_SIZE 12
#endif

#define FIELD_OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, offset) \
    typedef char type##_##field##_offset[FIELD_OFFSET(type, field) == offset ? 1 : -1]

typedef char sprite_size[sizeof(KfSpriteQuad) == EXPECTED_SPRITE_SIZE ? 1 : -1];
CHECK_OFFSET(KfSpriteQuad, u, 0);
CHECK_OFFSET(KfSpriteQuad, v, 1);
CHECK_OFFSET(KfSpriteQuad, u_span, 2);
CHECK_OFFSET(KfSpriteQuad, v_span, 3);
CHECK_OFFSET(KfSpriteQuad, x, 4);
CHECK_OFFSET(KfSpriteQuad, y, 6);
CHECK_OFFSET(KfSpriteQuad, w, 8);
CHECK_OFFSET(KfSpriteQuad, h, 10);

typedef char floor_item_size[sizeof(KfFloorItem) == 24 ? 1 : -1];
CHECK_OFFSET(KfFloorItem, base_sprite_index, 0);
CHECK_OFFSET(KfFloorItem, facing_and_frame_count, 2);
CHECK_OFFSET(KfFloorItem, position_x, 4);
CHECK_OFFSET(KfFloorItem, position_y, 8);
CHECK_OFFSET(KfFloorItem, position_z, 12);
CHECK_OFFSET(KfFloorItem, animation_frame, 20);
