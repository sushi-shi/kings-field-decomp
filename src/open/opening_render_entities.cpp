#include <kf/open/opening_render.h>
#include <kf/open/render.h>
#include <kf/open/resources.h>

void opening_render_entities(void)
{
    tmd_select(tmd_context(), KF_TMD_SLOT_ENTITIES);
    for (auto &entity : opening_entity_state.entities) {
        if (entity.object_id < KF_OPENING_ENTITY_MODEL_LIMIT) {
            opening_entity_render(&entity);
        }
    }
}
