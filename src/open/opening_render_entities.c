#include <kf/address.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>

ADDRESS(0x80019520, 0x78)
void opening_render_entities(void)
{
    KfOpeningEntity *entity;
    s16 remaining;

    tmd_select(KF_TMD_SLOT_ENTITIES);
    entity = opening_entity_state.entities;
    remaining = KF_OPENING_ENTITY_CAPACITY - 1;
    do {
        if (entity->object_id < KF_OPENING_ENTITY_MODEL_LIMIT) {
            opening_entity_render(entity);
        }
        entity++;
    } while (--remaining != -1);
}
