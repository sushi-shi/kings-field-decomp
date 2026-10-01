#include "../src/game/actor_pool.cpp"

#include <cassert>
#include <vector>

struct InvalidDefinitions {};
namespace kf {
[[noreturn]] void host_fail(const char *) { throw InvalidDefinitions {}; }
}

int main()
{
    constexpr std::size_t record_bytes = 152;
    // An unaligned table with a trailing unused definition, as allowed by MIXA.
    std::vector<u8> bytes(1 + record_bytes * (KF_ACTOR_DEFINITION_COUNT + 1));
    auto *table = bytes.data() + 1;
    for (u8 i = 0; i < KF_ACTOR_DEFINITION_COUNT; ++i) {
        auto *record = table + i * record_bytes;
        record[0] = i;
        record[7] = 56; // Paired homing projectile in the third action slot.
        record[10] = 70;
        record[24] = 3;
        record[40] = 10;
        record[46] = 20;
        // Third attachment (0, -1000, -2200), from the floor-five boss record.
        record[54] = 0x18; record[55] = 0xfc;
        record[56] = 0x68; record[57] = 0xf7;
        record[58] = 0x34; record[59] = 0x12;
        record[90] = 0x78; record[91] = 0x56;
        record[128] = 0xbc; record[129] = 0x9a;
        record[150] = 0xf0; record[151] = 0xde;
    }
    actor_definitions_load({table, bytes.size() - 1});
    for (u8 i = 0; i < KF_ACTOR_DEFINITION_COUNT; ++i) {
        const auto &definition = actor_state.definitions.entries[i];
        assert(definition.pursuit_distance_scale == i);
        assert(definition.action_parameters.effect_codes[2] == kf_enum_decode<KfActorEffectCode>(56));
        assert(definition.action_parameters.effect_chances[2] == 70);
        assert(definition.action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] == KF_ANIMATION_CLIP_FOURTH);
        assert(definition.attachment_offsets[0].x == 10);
        assert(definition.attachment_offsets[1].x == 20);
        const auto &third = definition.attachment_offsets[2];
        assert(third.x == 0 && third.y == -1000 && third.z == -2200);
        assert(definition.special_attack_chance == 0);
        assert(definition.special_attack_range == -1000);
        assert(definition.action_animation_steps[0] == 0x1234);
        assert(definition.action_animation_phases[0] == 0x5678);
        assert(definition.initial_health == 0x9abc);
        assert(definition.gold_drop_limit == 0xdef0);
    }
    // Reject a missing final byte without partially replacing the live table.
    table[0] = 99;
    try {
        actor_definitions_load({table, record_bytes * KF_ACTOR_DEFINITION_COUNT - 1});
        assert(false);
    } catch (const InvalidDefinitions &) {
        assert(actor_state.definitions.entries[0].pursuit_distance_scale == 0);
    }
}
