#include <kf/game/actor.h>

int main()
{
    KfActorDefinition definition = {};
    const s16 coordinates[3][3] = {{123, -456, 789}, {-234, 567, -890}, {0, -1000, -2200}};
    int slot;

    for (slot = 0; slot < KF_ACTOR_ATTACHMENT_OFFSET_COUNT; slot++) {
        definition.attachment_offsets[slot].x = coordinates[slot][0];
        definition.attachment_offsets[slot].y = coordinates[slot][1];
        definition.attachment_offsets[slot].z = coordinates[slot][2];
    }
    for (slot = 0; slot < KF_ACTOR_ATTACHMENT_OFFSET_COUNT; slot++) {
        if (definition.attachment_offsets[slot].x != coordinates[slot][0]
            || definition.attachment_offsets[slot].y != coordinates[slot][1]
            || definition.attachment_offsets[slot].z != coordinates[slot][2]) {
            return 1;
        }
    }
    if (actor_definition_special_attack_chance(&definition) != 0
        || actor_definition_special_attack_range(&definition) != -1000) {
        return 2;
    }
    definition.attachment_offsets[2].x = 48;
    definition.attachment_offsets[2].y = 3000;
    if (actor_definition_special_attack_chance(&definition) != 48
        || actor_definition_special_attack_range(&definition) != 3000
        || definition.attachment_offsets[2].z != -2200) {
        return 3;
    }
    return 0;
}
