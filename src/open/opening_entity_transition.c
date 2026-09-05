#include <kf/address.h>
#include <kf/open_entity_transition.h>
#include <kf/open_opening_render.h>
#include <kf/open_resources.h>

typedef char KfOpeningEntitySizeCheck[
    sizeof(KfOpeningEntity) == 0x28 ? 1 : -1];
typedef char KfOpeningEntityPositionOffsetCheck[
    (u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];

ADDRESS(0x80014608, 0x1fc)
void opening_entity_transition(s16 mode, const VECTOR *position)
{
    KfOpeningEntity *entity;
    VECTOR position_snapshot;
    s16 entity_index;
    s16 frame;
    s16 initial_scale_y;
    s16 scale_step;

    switch (mode) {
    case 0:
        initial_scale_y = 0;
        scale_step = 0x100;
        break;
    case 1:
        goto deactivate;
    case 2:
    case 3:
        initial_scale_y = 0x2000;
        scale_step = -0x100;
        break;
    }

    entity = &opening_entity_state.entities[24];
    entity_index = 3;
    /* Retail retains these stack coordinates without a subsequent consumer. */
    position_snapshot.vx = position->vx;
    position_snapshot.vz = position->vz;
    position_snapshot.vy = position->vy;
    do {
        entity->object_id = 0x13;
        entity->position = *position;
        entity->rotation.z = 0;
        entity->rotation.y = 0;
        entity->rotation.x = 0;
        entity->scale.vz = 0x1000;
        entity->scale.vx = 0x1000;
        entity->scale.vy = initial_scale_y;
        entity++;
        entity_index--;
    } while (entity_index != -1);

    if (mode == 3) {
        return;
    }

    frame = 0;
    do {
        entity = &opening_entity_state.entities[24];
        entity_index = 0;
        do {
            if ((entity_index << 3) < frame) {
                u16 scale_y = entity->scale.vy;

                if (scale_y < 0x2001) {
                    entity->scale.vy = scale_step + scale_y;
                }
            }
            entity_index++;
            entity->rotation.y = (entity->rotation.y + 0x200) & 0xfff;
            entity++;
        } while (entity_index < 4);
        opening_render_frame(0, 0);
        VSync(0);
        frame++;
    } while (frame < 48);

    if (mode == 0) {
        return;
    }

deactivate:
    entity = &opening_entity_state.entities[24];
    entity_index = 3;
    do {
        entity->object_id = 0xff;
        entity++;
        entity_index--;
    } while (entity_index != -1);
}
