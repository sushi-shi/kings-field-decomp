#if defined(KF_AUDIT_GAME)

#include <kf/platform/prelude.h>
#define game_main_loop audit_original_game_main_loop
#define player_update audit_selected_player_update
#define game_initialize_session audit_initialize_session
#include KF_AUDIT_SOURCE
#undef game_initialize_session
#undef player_update
#undef game_main_loop
void player_update();
void game_initialize_session();
static unsigned audit_entries;
unsigned audit_frames;
void game_main_loop()
{
    ++audit_entries;
    audit_frames = 0;
    if (audit_entries == 7) {
        std::fprintf(stderr, "AUDIT complete: six GAME entries, five floors, save round trips\n");
        kf::host_shutdown();
        std::exit(0);
    }
    if (memory_arena.start || std::strcmp(map_resource_path, "B0/") != 0)
        kf::host_fail("Audit: module state not restored");
    audit_original_game_main_loop();
}
void audit_initialize_session()
{
    game_initialize_session();
    const auto floor = kf_enum_decode<KfFloorId>(audit_entries == 6 ? 1 : audit_entries);
    player_state.progress_state.current_floor = floor;
    player_state.progress_state.highest_floor = floor;
    player_state.map_variant =
        floor == KF_FLOOR_5 ? KF_FLOOR5_ENTRY_VARIANT : KF_MAP_VARIANT_DEFAULT;
}
void audit_selected_player_update()
{
    if (audit_entries == 1 && audit_frames == 8 && std::getenv("KF_AUDIT_KEYS")) {
        const auto objects_before = map_object_state;
        const auto notifications_before = game_graphics_runtime.notification_state;
        KfNotificationId messages_before[KF_NOTIFICATION_CAPACITY];
        std::memcpy(messages_before, game_graphics_runtime.notification_message_ids, sizeof messages_before);
        const auto reach = vector_yaw_probe_xz(player_state.camera_position,
            player_state.camera_rotation.vy, MAP_INTERACTION_PROBE_DISTANCE);
        for (const auto key : {KF_ITEM_KEY_OF_THE_DEAD, KF_ITEM_RAITO_FAMILY_KEY,
                              KF_ITEM_DUNGEON_KEY, KF_ITEM_SORCERER_KEY}) {
            for (auto &object : map_object_state.objects)
                object.object_id = KF_OBJECT_NONE;
            player_use_item(key);
            // Exercise a hit at the end of the pool as well as an empty search.
            auto &lock = map_object_state.objects[KF_MAP_OBJECT_CAPACITY - 1];
            lock = {};
            lock.object_id = KF_MAP_OBJECT_FLAT_WOODEN_LID;
            lock.position = {reach.x, 0, reach.z};
            lock.link.fields.link_id = kf_enum_encode<u8>(key);
            player_use_item(key);
            if (lock.link.fields.link_id != KF_MAP_LINK_NONE)
                kf::host_fail("Audit: matching key did not unlock container");
            const auto wrong_key = key == KF_ITEM_KEY_OF_THE_DEAD
                ? KF_ITEM_RAITO_FAMILY_KEY : KF_ITEM_KEY_OF_THE_DEAD;
            lock.link.fields.link_id = kf_enum_encode<u8>(wrong_key);
            player_use_item(key);
            if (lock.link.fields.link_id != kf_enum_encode<u8>(wrong_key))
                kf::host_fail("Audit: wrong key unlocked container");
        }
        map_object_state = objects_before;
        game_graphics_runtime.notification_state = notifications_before;
        std::memcpy(game_graphics_runtime.notification_message_ids, messages_before, sizeof messages_before);
        std::fprintf(stderr, "AUDIT keys: all four keys handled empty, matching and wrong locks\n");
    }
    if (audit_frames == 8 && std::getenv("KF_AUDIT_LANGUAGE_SWITCH")) {
        const auto initial = kf::game_language();
        const auto alternate = initial == kf::Language::Japanese ? kf::Language::English : kf::Language::Japanese;
        const auto player_before = player_state;
        const auto actors_before = actor_state;
        const auto objects_before = map_object_state;
        constexpr auto texture_words = kf::texture_store_width * kf::texture_store_height;
        auto *textures = kf::host_renderer()->textures.words;
        auto *before = static_cast<u16 *>(std::malloc(texture_words * sizeof(u16)));
        if (!before) std::exit(3);
        std::memcpy(before, textures, texture_words * sizeof(u16));
        if (!kf::language_request(alternate) || !game_apply_language() ||
            kf::game_language() != alternate ||
            std::memcmp(before, textures, texture_words * sizeof(u16)) == 0)
            kf::host_fail("Audit: language switch did not replace text graphics");
        if (!kf::language_request(initial) || !game_apply_language() ||
            std::memcmp(before, textures, texture_words * sizeof(u16)) != 0)
            kf::host_fail("Audit: language round trip changed other live textures");
        std::free(before);
        if (std::memcmp(&player_before, &player_state, sizeof player_state) ||
            std::memcmp(&actors_before, &actor_state, sizeof actor_state) ||
            std::memcmp(&objects_before, &map_object_state, sizeof map_object_state))
            kf::host_fail("Audit: language switch changed world state");
        std::fprintf(stderr, "AUDIT language round trip entry=%u preserved world and textures\n", audit_entries);
    }
    player_update();
    if (++audit_frames != (std::getenv("KF_AUDIT_MOVEMENT") ? 96u : 16u))
        return;
    map_world_state_persist();
    if (save_system_write_slot(KF_SAVE_SLOT_FIRST) != KF_SAVE_RESULT_OK)
        kf::host_fail("Audit: save write failed");
    u8 bytes[kf::save_file_capacity];
    std::size_t size = 0;
    if (kf::save_file_read(kf::SaveSlot::First, bytes, sizeof bytes, &size) !=
        kf::SaveFileResult::Ok)
        kf::host_fail("Audit: saved bytes unavailable");
    u32 hash = 2166136261u;
    for (std::size_t i = 0; i < size; ++i)
        hash = (hash ^ bytes[i]) * 16777619u;
    std::fprintf(stderr, "AUDIT entry=%u floor=%u save=%zu hash=%08x hp=%u xyz=%d,%d,%d\n",
                 audit_entries, kf_enum_encode<unsigned>(player_state.progress_state.current_floor),
                 size, hash, player_state.vitals.current_hp, player_state.camera_position.vx,
                 player_state.camera_position.vy, player_state.camera_position.vz);
    char path[512];
    std::snprintf(path, sizeof path, "%s/entry-%u.kfs", std::getenv("KF_AUDIT_OUTPUT"),
                  audit_entries);
    FILE *file = std::fopen(path, "wb");
    if (!file)
        std::exit(3);
    std::fwrite(bytes, 1, size, file);
    std::fclose(file);
    const u32 gold = player_state.gold;
    KfSaveSlotSummary slots[KF_SAVE_SLOT_COUNT];
    if (save_system_read_catalog(slots) != KF_SAVE_RESULT_OK)
        kf::host_fail("Audit: save catalog failed");
    player_state.gold = gold + 123;
    if (save_system_read_slot(KF_SAVE_SLOT_FIRST) != KF_SAVE_RESULT_OK || player_state.gold != gold)
        kf::host_fail("Audit: save restoration failed");
    game_result = GameResult::ReturnToIntro;
}

#elif defined(KF_AUDIT_OPENING)

#include <kf/platform/prelude.h>
#define opening_poll_input audit_original_opening_poll_input
#include KF_AUDIT_SOURCE
#undef opening_poll_input
void opening_poll_input()
{
    audit_original_opening_poll_input();
    opening_input_action = KF_OPENING_INPUT_SKIP;
}

#elif defined(KF_AUDIT_INPUT)

#include <kf/platform/prelude.h>
extern unsigned audit_frames;
namespace kf
{
u32 audit_read_buttons()
{
    const auto frame = ::audit_frames;
    if (frame >= 16 && frame < 28)
        return static_cast<u32>(Button::Up);
    if (frame >= 28 && frame < 36)
        return static_cast<u32>(Button::Right);
    if (frame >= 36 && frame < 48)
        return static_cast<u32>(Button::StrafeLeft);
    if (frame == 48)
        return static_cast<u32>(Button::Attack);
    if (frame >= 81)
        return static_cast<u32>(Button::Down) | static_cast<u32>(Button::LookUp);
    return 0;
}
} // namespace kf
#define host_read_buttons audit_read_buttons
#include KF_AUDIT_SOURCE
#undef host_read_buttons

#endif
