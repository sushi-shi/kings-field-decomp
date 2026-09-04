#include <kf/address.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>

ADDRESS(0x80019520, 0x78)
void opening_render_entities(void)
{
    KfOpeningEntity *entity;
    s16 remaining;

    tmd_select(1);
    entity = opening_entity_state.entities;
    remaining = 31;
    do {
        if (entity->object_id < 32) {
            opening_entity_render(entity);
        }
        entity++;
    } while (--remaining != -1);
}
