#include <kf/open/opening_render.h>
#include <kf/open/render.h>
#include <kf/open/resources.h>

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
