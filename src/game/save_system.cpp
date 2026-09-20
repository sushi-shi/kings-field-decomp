#include <kf/lib/null.h>
#include <kf/lib/bool.h>
#include <kf/game/graphics.h>
#include <kf/lib/resource_file.h>
#include <kf/game/save.h>
#include <kf/game/player.h>
#include <kf/platform/saves.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/game/game.h>

enum {
    SAVE_MESSAGE_NO_SPACE = 102,
    SAVE_MESSAGE_STALE_CATALOG = 103,
    SAVE_MESSAGE_WRITE_FAILED = 107,
    SAVE_MESSAGE_NO_DATA = 111,
    SAVE_MESSAGE_READ_FAILED = 112,
    MESSAGE_IMAGE_SKIP = 0xff,
    MENU_INPUT_SOUND_VOLUME = 0x40,
    IMAGE_WAIT_INITIAL_BRIGHTNESS = 32,
    IMAGE_WAIT_MAX_BRIGHTNESS = 127
};
char talk_image_path_template[20] = "TALK/C00/T00000.TIM";
KfBool32 menu_load_message_image(s32 message_id);
void screen_show_image_until_input(const char *path);

#include "save_state.inc"

constexpr u8 save_magic[] = {'K', 'F', 'J', 'P', 'S', 'A', 'V', 'E'};
constexpr u32 save_version = 1;
constexpr std::size_t save_header_bytes = 20;
struct SaveCatalogEntry { bool occupied; u32 checksum; };
static SaveCatalogEntry save_catalog[KF_SAVE_SLOT_COUNT];

static bool save_slot_valid(KfSaveSlotId slot) {
    return slot >= KF_SAVE_SLOT_FIRST && slot <= KF_SAVE_SLOT_THIRD;
}
static kf::SaveSlot save_slot_native(KfSaveSlotId slot) {
    return static_cast<kf::SaveSlot>(kf_enum_encode<s16>(slot));
}
static KfSaveSlotSummary save_summary(const SavedGameState &state) {
    const auto &p = state.player;
    return {static_cast<u32>(p.experience), static_cast<KfFloorId>(p.progress_state.current_floor),
        p.vitals.current_hp, p.vitals.maximum_hp, p.vitals.current_mp, p.vitals.maximum_mp,
        KfSaveSlotState::Ready};
}

static kf::SaveFileResult save_read_record(KfSaveSlotId slot, SavedGameState &state, u32 &checksum) {
    u8 data[kf::save_file_capacity];
    std::size_t size;
    const auto result = kf::save_file_read(save_slot_native(slot), data, sizeof data, &size);
    if (result != kf::SaveFileResult::Ok)
        return result;
    if (size < save_header_bytes || std::memcmp(data, save_magic, sizeof save_magic) != 0)
        return kf::SaveFileResult::Invalid;
    SaveCodec io{data, size, sizeof save_magic, SaveCodecMode::Read, true};
    u32 version = 0, payload_size = 0;
    save_field<u32>(io, version);
    save_field<u32>(io, payload_size);
    save_field<u32>(io, checksum);
    if (!io.valid || version != save_version || payload_size != size - save_header_bytes
            || checksum != save_checksum(data + save_header_bytes, payload_size))
        return kf::SaveFileResult::Invalid;
    save_state_fields(io, state);
    return io.valid && io.position == size && save_state_valid(state)
        ? kf::SaveFileResult::Ok : kf::SaveFileResult::Invalid;
}

static KfSaveResult save_failure(kf::SaveFileResult result, bool writing) {
    s32 message = writing ? SAVE_MESSAGE_WRITE_FAILED : SAVE_MESSAGE_READ_FAILED;
    if (result == kf::SaveFileResult::Missing)
        message = SAVE_MESSAGE_NO_DATA;
    else if (result == kf::SaveFileResult::NoSpace)
        message = SAVE_MESSAGE_NO_SPACE;
    std::fprintf(stderr, "%s save failed (%d)\n", writing ? "Write" : "Read", static_cast<int>(result));
    menu_load_message_image(message);
    return result == kf::SaveFileResult::NoSpace ? KF_SAVE_RESULT_NO_SPACE : KF_SAVE_RESULT_FAILED;
}

KfSaveResult save_system_read_catalog(KfSaveSlotSummary *summaries) {
    std::memset(summaries, 0, sizeof(*summaries) * KF_SAVE_SLOT_COUNT);
    std::memset(save_catalog, 0, sizeof save_catalog);
    for (unsigned i = 0; i < KF_SAVE_SLOT_COUNT; ++i) {
        SavedGameState state{};
        u32 checksum = 0;
        const auto slot = static_cast<KfSaveSlotId>(i + 1);
        const auto result = save_read_record(slot, state, checksum);
        if (result == kf::SaveFileResult::Missing)
            continue;
        if (result != kf::SaveFileResult::Ok) {
            summaries[i].state = result == kf::SaveFileResult::Invalid
                ? KfSaveSlotState::Damaged : KfSaveSlotState::Unavailable;
            std::fprintf(stderr, "Save slot %u cannot be read (%d); other slots remain available\n",
                i + 1, static_cast<int>(result));
            continue;
        }
        save_catalog[i] = {true, checksum};
        summaries[i] = save_summary(state);
    }
    return KF_SAVE_RESULT_OK;
}

KfSaveResult save_system_read_slot(KfSaveSlotId slot) {
    if (!save_slot_valid(slot))
        return save_failure(kf::SaveFileResult::Invalid, false);
    SavedGameState state{};
    u32 checksum = 0;
    const auto result = save_read_record(slot, state, checksum);
    if (result != kf::SaveFileResult::Ok)
        return save_failure(result, false);
    const auto &catalog = save_catalog[kf_enum_encode<s16>(slot) - 1];
    if (!catalog.occupied || catalog.checksum != checksum) {
        menu_load_message_image(SAVE_MESSAGE_STALE_CATALOG);
        return KF_SAVE_RESULT_FAILED;
    }
    // No live game state is touched until the complete file is decoded and validated.
    save_state_apply(state);
    return KF_SAVE_RESULT_OK;
}

KfSaveResult save_system_write_slot(KfSaveSlotId slot) {
    if (!save_slot_valid(slot))
        return save_failure(kf::SaveFileResult::Invalid, true);
    SavedGameState state{};
    state.player = player_state;
    state.world = map_runtime_state.world_state;
    std::memcpy(state.stock, item_stock, sizeof state.stock);
    for (unsigned i = 0; i < KF_MAGIC_RECORD_COUNT; ++i)
        state.learned[i] = magic_records[i].learned;
    if (!save_state_valid(state))
        return save_failure(kf::SaveFileResult::Invalid, true);
    u8 data[kf::save_file_capacity];
    SaveCodec payload{data, sizeof data, save_header_bytes, SaveCodecMode::Write, true};
    save_state_fields(payload, state);
    if (!payload.valid)
        return save_failure(kf::SaveFileResult::Invalid, true);
    std::memcpy(data, save_magic, sizeof save_magic);
    SaveCodec header{data, save_header_bytes, sizeof save_magic, SaveCodecMode::Write, true};
    u32 version = save_version;
    u32 payload_size = payload.position - save_header_bytes;
    u32 checksum = save_checksum(data + save_header_bytes, payload_size);
    save_field<u32>(header, version);
    save_field<u32>(header, payload_size);
    save_field<u32>(header, checksum);
    const auto result = kf::save_file_write(save_slot_native(slot), data, payload.position);
    if (result != kf::SaveFileResult::Ok)
        return save_failure(result, true);
    save_catalog[kf_enum_encode<s16>(slot) - 1] = {true, checksum};
    return KF_SAVE_RESULT_OK;
}

void menu_play_input_sound(KfMenuSoundCue cue)
{
    SoundRef sound;

    if (cue == MENU_SOUND_CURSOR) {
        sound.program = 0xe;
        sound.note = 0x44;
    } else if (cue == MENU_SOUND_CONFIRM) {
        sound.program = 0xd;
        sound.note = 0x3c;
    } else {
        sound.program = 0xf;
        sound.note = 0x3f;
    }

    kf::sound_note_play(audio_state.bank, sound.program, sound.note, MENU_INPUT_SOUND_VOLUME, MENU_INPUT_SOUND_VOLUME);
    kf::host_wait_frame();
    kf::sound_note_release(audio_state.bank, sound.program, sound.note);
}

KfBool32 menu_load_message_image(s32 message_id)
{
    char path[16] = "TIM/M000.";
    u8 *buffer;

    if (message_id != MESSAGE_IMAGE_SKIP) {
        RESOURCE_PATH_WRITE_DECIMAL3(&path[5], message_id);
        buffer = game_graphics_runtime.display_state.asset_load_buffer;
        std::size_t image_size;
        if (resource_file_load_into(buffer,
                game_graphics_runtime.display_state.asset_load_capacity, path, &image_size) != KF_RESOURCE_LOADED) {
            return KF_TRUE;
        }
        tim_upload_images(buffer, image_size);
    }
    return KF_FALSE;
}

void screen_show_image_until_input(const char *path)
{
    s32 brightness = IMAGE_WAIT_INITIAL_BRIGHTNESS;
    KfBool8 pressed = KF_FALSE;
    std::size_t image_size;
    if (resource_file_load_into(game_graphics_runtime.display_state.asset_load_buffer,
            game_graphics_runtime.display_state.asset_load_capacity, path, &image_size) != KF_RESOURCE_LOADED) {
        return;
    }
    tim_upload_images(game_graphics_runtime.display_state.asset_load_buffer, image_size);
    const auto input_context = kf::host_set_input_context(kf::InputContext::Menu);
    for (;;) {
        if (brightness < IMAGE_WAIT_MAX_BRIGHTNESS) {
            brightness++;
        }
        display_present_system_screen(brightness);
        kf::host_wait_frame();
        if (pressed == KF_FALSE) {
            if (kf::host_read_buttons() == 0) {
                pressed = KF_TRUE;
            }
        } else if (kf::host_read_buttons() != 0) {
            kf::host_wait_buttons_released();
            break;
        }
    }
    kf::host_set_input_context(input_context);
}

void talk_show_dialogue_page(KfFloorId floor, u8 stage, KfCharacterId character_id, u8 page)
{
    char *directory_character = &talk_image_path_template[6];

    talk_image_path_template[0xc] = kf_enum_encode<s32>(character_id) / 10 + '0';
    directory_character[0] = kf_enum_encode<s32>(character_id) / 10 + '0';
    talk_image_path_template[0xa] = kf_enum_encode<u8>(floor) + '0';
    directory_character[1] = talk_image_path_template[0xd] =
        kf_enum_encode<s32>(character_id) % 10 + '0';
    talk_image_path_template[0xb] = stage + '0';
    talk_image_path_template[0xe] = page + '0';
    screen_show_image_until_input(directory_character - 6);
}


void save_system_reset_module_state(void)
{
    kf::restore_initial_value<talk_image_path_template>();
    kf::restore_initial_value<save_catalog>();
}
