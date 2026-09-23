#include <kf/lib/null.h>
#include <kf/lib/bool.h>
#include <kf/game/graphics.h>

#include <kf/lib/cd_file.h>
#include <kf/game/save.h>
#include <kf/game/player.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    SAVE_MESSAGE_NO_CARD = 101,
    SAVE_MESSAGE_NO_SPACE = 102,
    SAVE_MESSAGE_KEEP_CARD_INSERTED = 103,
    SAVE_MESSAGE_WRITE_FAILED = 107,
    SAVE_MESSAGE_FORMAT_FAILED = 108,
    SAVE_MESSAGE_FORMAT_REQUIRED = 109,
    SAVE_MESSAGE_CARD_UNUSABLE = 110,
    SAVE_MESSAGE_NO_DATA = 111,
    SAVE_MESSAGE_READ_FAILED = 112,
    SAVE_MESSAGE_SYSTEM_ERROR = 113,
    SAVE_MESSAGE_FAILED = 114,
    SAVE_MESSAGE_FORMAT_CONFIRMATION = 115,
    MESSAGE_IMAGE_SKIP = 0xff
};

enum {
    SAVE_FILE_IO_ATTEMPTS = 5,
    CARD_FORMAT_ATTEMPTS = 5,
    SAVE_DIRECTORY_ENTRY_NOT_FOUND = -1
};

enum {
    MENU_INPUT_SOUND_VOLUME = 0x40
};

enum {
    IMAGE_WAIT_INITIAL_BRIGHTNESS = 32,
    IMAGE_WAIT_MAX_BRIGHTNESS = 127
};

enum {
    SAVE_ICON_TIM_CLUT_OFFSET = 0x14,
    SAVE_ICON_TIM_PIXELS_OFFSET = 0x40
};

char save_main_file_path[26] = "bu00:BISLPS-00017KF      ";

char save_temporary_file_path[26] = "bu00:BISLPS-00017KFTMP   ";

char talk_image_path_template[20] = "TALK\\C00\\T00000.TIM";

char memory_card_root_path[6] = "bu00:";

static s32 memory_card_io_end_event;

static s32 memory_card_timeout_event;

static s32 memory_card_new_device_event;

static s32 memory_card_error_event;

KfSaveHeader *save_header_buffer;

KfSavePayload *save_payload_buffer;

#define SAVE_FILE_BLOCKS 5

#define SAVE_TITLE_TEXT \
    "\201@\201@\201@\201@\201@\201@\201\203\201\203\201@\201@\202j\202h\202m\202f" \
    "\201f\202r\201@\202e\202h\202d\202k\202c\201@\201@\201\204\201\204"

void memory_card_clear_events(void);
KfSaveStatus memory_card_begin_status_check(void);
KfSaveStatus memory_card_wait_event(void);
void memory_card_undeliver_events(void);
KfSaveStatus memory_card_format(void);
KfSaveStatus save_file_write_slot(KfSaveSlotId slot_id);
KfSaveResult save_system_read_header(void);
KfSaveStatus save_file_read_header(void);
KfSaveStatus save_file_read_slot(KfSaveSlotId slot_id);
void save_file_initialize_buffers(void);
s32 memory_card_show_status_message(s16 status);
KfBool32 menu_load_message_image(s32 message_id);
void screen_show_image_until_input(const char *path);

static inline void memory_card_acknowledge_new_device(void)
{
    memory_card_clear_events();
    _new_card();
    memory_card_begin_status_check();
    memory_card_clear_events();
    _new_card();
    memory_card_begin_status_check();
}

#define SAVE_SUMMARY_COPY(to, from) ( \
    (to).experience = (from).experience, \
    (to).current_floor = (from).current_floor, \
    (to).current_hp = (from).current_hp, \
    (to).maximum_hp = (from).maximum_hp, \
    (to).current_mp = (from).current_mp, \
    (to).maximum_mp = (from).maximum_mp)

#define SAVE_SUMMARY_EQUAL(a, b) ( \
    (a).experience == (b).experience && \
    (a).current_floor == (b).current_floor && \
    (a).current_hp == (b).current_hp && \
    (a).maximum_hp == (b).maximum_hp && \
    (a).current_mp == (b).current_mp && \
    (a).maximum_mp == (b).maximum_mp)

KfSaveResult save_system_read_catalog(KfSaveSlotSummary *summaries)
{
    KfSaveResult result;
    s32 index;
    KfSaveHeader *header;

    memset((void *)summaries, 0, 0x24);
    result = save_system_read_header();
    if (result == KF_SAVE_RESULT_OK) {
        header = save_header_buffer;
        for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
            u8 slot = header->directory.slot_ids[index];

            if (slot != KF_SAVE_SLOT_EMPTY && slot != KF_SAVE_SLOT_SPARE) {
                s32 entry = ((u8)(slot)) - ((s16)(KF_SAVE_SLOT_FIRST));

                SAVE_SUMMARY_COPY(summaries[entry], header->directory.summaries[index]);
            }
        }
    }
    return result;
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

    SsVoKeyOn(sound.program,
        sound.note << KF_SOUND_PACKED_NOTE_SHIFT,
        MENU_INPUT_SOUND_VOLUME,
        MENU_INPUT_SOUND_VOLUME);
    VSync(0);
    SsVoKeyOff(sound.program, sound.note << KF_SOUND_PACKED_NOTE_SHIFT);
}

void memory_card_initialize(void)
{
    u8 buffer[0x80];

    memset((void *)buffer, 0xff, sizeof(buffer));
    memory_card_io_end_event = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    memory_card_timeout_event = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    memory_card_new_device_event = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    memory_card_error_event = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    EnableEvent(memory_card_io_end_event);
    EnableEvent(memory_card_timeout_event);
    EnableEvent(memory_card_new_device_event);
    EnableEvent(memory_card_error_event);
    StartCARD2();
    _bu_init();
    _card_auto(0);
}

void memory_card_shutdown_events(void)
{
    StopCARD2();
    CloseEvent(memory_card_io_end_event);
    CloseEvent(memory_card_timeout_event);
    CloseEvent(memory_card_new_device_event);
    CloseEvent(memory_card_error_event);
}

KfSaveStatus memory_card_begin_status_check(void)
{
    memory_card_clear_events();
    if (_card_info(KF_CARD_CHANNEL) != 0) {
        return memory_card_wait_event();
    }
    return KF_CARD_STATUS_NOT_STARTED;
}

void memory_card_clear_events(void)
{
    TestEvent(memory_card_io_end_event);
    TestEvent(memory_card_timeout_event);
    TestEvent(memory_card_new_device_event);
    TestEvent(memory_card_error_event);
}

KfSaveStatus memory_card_wait_event(void)
{
    for (;;) {
        if (TestEvent(memory_card_io_end_event) == 1) {
            memory_card_undeliver_events();
            return SAVE_STATUS_OK;
        }
        if (TestEvent(memory_card_timeout_event) == 1) {
            memory_card_undeliver_events();
            return KF_CARD_STATUS_TIMEOUT;
        }
        if (TestEvent(memory_card_new_device_event) == 1) {
            memory_card_undeliver_events();
            return KF_CARD_STATUS_NEW_DEVICE;
        }
        if (TestEvent(memory_card_error_event) == 1) {
            memory_card_undeliver_events();
            return KF_CARD_STATUS_ERROR;
        }
    }
}

void memory_card_undeliver_events(void)
{
    UnDeliverEvent(memory_card_io_end_event);
    UnDeliverEvent(memory_card_timeout_event);
    UnDeliverEvent(memory_card_new_device_event);
    UnDeliverEvent(memory_card_error_event);
}

KfSaveResult memory_card_check_or_format(KfCardFormatConfirmation format_confirmation)
{
    KfSaveStatus status;
    KfSaveResult result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == KF_CARD_STATUS_NEW_DEVICE) {
        memory_card_acknowledge_new_device();
        status = memory_card_format();
    } else if (status == SAVE_STATUS_OK) {
        if (format_confirmation == KF_CARD_FORMAT_UNCONFIRMED) {
            status = SAVE_STATUS_FORMAT_CONFIRMATION;
        } else {
            status = memory_card_format();
        }
    }
    if (status != SAVE_STATUS_OK) {
        memory_card_show_status_message(status);
    }
    result = ((KfSaveResult)(((s32)(status))));
    switch (status) {
    case KF_CARD_STATUS_NOT_STARTED:
    case KF_CARD_STATUS_TIMEOUT:
    case KF_CARD_STATUS_NEW_DEVICE:
    case KF_CARD_STATUS_ERROR:
    case SAVE_STATUS_09:
    case SAVE_STATUS_10:
    case SAVE_STATUS_FORMAT_FAILED:
        result = KF_SAVE_RESULT_FAILED;
        break;
    case SAVE_STATUS_OK:
        result = KF_SAVE_RESULT_OK;
        break;
    case SAVE_STATUS_FORMAT_CONFIRMATION:
        result = KF_SAVE_RESULT_FORMAT_REQUIRED;
        break;
    }
    return result;
}

KfSaveStatus memory_card_format(void)
{
    s32 attempt = 0;
    s32 formatted;
    KfSaveStatus status;

    do {
        memory_card_clear_events();
        formatted = format(memory_card_root_path);
        status = memory_card_begin_status_check();
        if (formatted == 1) {
            break;
        }
        attempt++;
    } while (attempt < CARD_FORMAT_ATTEMPTS);
    if (formatted != 1) {
        return SAVE_STATUS_FORMAT_FAILED;
    }
    return status;
}

KfSaveResult save_system_write_slot(KfSaveSlotId slot_id)
{
    KfSaveStatus status;
    KfSaveResult result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == KF_CARD_STATUS_NEW_DEVICE) {
        memory_card_acknowledge_new_device();
        status = SAVE_STATUS_OK;
    }
    if (status == SAVE_STATUS_OK) {
        status = save_file_write_slot(slot_id);
    }
    if (status != SAVE_STATUS_OK) {
        memory_card_show_status_message(status);
    }
    result = ((KfSaveResult)(((s32)(status))));
    switch (status) {
    case KF_CARD_STATUS_NOT_STARTED:
    case KF_CARD_STATUS_TIMEOUT:
    case KF_CARD_STATUS_ERROR:
    case SAVE_STATUS_WRITE_FAILED:
        result = KF_SAVE_RESULT_FAILED;
        break;
    case SAVE_STATUS_OK:
        result = KF_SAVE_RESULT_OK;
        break;
    case SAVE_STATUS_FORMAT_REQUIRED:
        result = KF_SAVE_RESULT_FORMAT_REQUIRED;
        break;
    case SAVE_STATUS_NO_SPACE:
        result = KF_SAVE_RESULT_NO_SPACE;
        break;
    }
    return result;
}

KfSaveStatus save_file_write_slot(KfSaveSlotId slot_id)
{
    s32 file;
    s32 index;
    s32 written;
    s32 entry;
    s32 previous;
    s32 offset;
    s32 payload_size;
    s32 header_size;

    memory_card_clear_events();
    file = open(save_main_file_path, O_RDONLY);
    close(file);
    payload_size = sizeof(KfSavePayload);
    header_size = sizeof(KfSaveHeader);
    if (file == -1) {
        memory_card_clear_events();
        file = open(save_main_file_path, O_CREAT | (SAVE_FILE_BLOCKS << 16));
        close(file);
        if (file == -1) {
            file = open(save_temporary_file_path, O_CREAT);
            close(file);
            erase(save_temporary_file_path);
            if (file == -1) {
                return SAVE_STATUS_FORMAT_REQUIRED;
            }
            return SAVE_STATUS_NO_SPACE;
        }
        save_file_initialize_buffers();
    }
    entry = SAVE_DIRECTORY_ENTRY_NOT_FOUND;
    for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
        if (save_header_buffer->directory.slot_ids[index] == KF_SAVE_SLOT_SPARE) {
            entry = index;
            break;
        }
    }
    if (entry == SAVE_DIRECTORY_ENTRY_NOT_FOUND) {
        for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
            if (save_header_buffer->directory.slot_ids[index] == KF_SAVE_SLOT_EMPTY) {
                entry = index;
                break;
            }
        }
    }
    previous = SAVE_DIRECTORY_ENTRY_NOT_FOUND;
    for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
        if (save_header_buffer->directory.slot_ids[index] == slot_id) {
            previous = index;
            break;
        }
    }
    memcpy((void *)save_payload_buffer->player_state, (const void *)&player_state.experience,
           sizeof(save_payload_buffer->player_state));
    memcpy((void *)&save_payload_buffer->world_state, (const void *)&map_runtime_state.world_state,
           sizeof(save_payload_buffer->world_state));
    memcpy((void *)save_payload_buffer->item_stock, (const void *)item_stock,
           sizeof(save_payload_buffer->item_stock));
    for (index = 0; index < KF_MAGIC_RECORD_COUNT; index++) {
        save_payload_buffer->magic_flags[index] = effect_state.magic.entries[index].learned;
    }
    memory_card_clear_events();
    file = open(save_main_file_path, O_WRONLY);
    if (file == -1) {
        return SAVE_STATUS_WRITE_FAILED;
    }
    offset = payload_size * entry;
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, header_size + offset, SEEK_SET);
        memory_card_clear_events();
        written = write(file, (const void *)save_payload_buffer, payload_size);
        if (written == payload_size) {
            break;
        }
        index++;
    } while (index < SAVE_FILE_IO_ATTEMPTS);
    close(file);
    if (written != payload_size) {
        return SAVE_STATUS_WRITE_FAILED;
    }
    save_header_buffer->directory.slot_ids[entry] = slot_id;
    save_header_buffer->directory.slot_ids[previous] = KF_SAVE_SLOT_SPARE;
    save_header_buffer->directory.summaries[entry].experience = player_state.experience;
    save_header_buffer->directory.summaries[entry].current_floor =
        player_state.progress_state.current_floor;
    save_header_buffer->directory.summaries[entry].current_hp = player_state.vitals.current_hp;
    save_header_buffer->directory.summaries[entry].maximum_hp = player_state.vitals.maximum_hp;
    save_header_buffer->directory.summaries[entry].current_mp = player_state.vitals.current_mp;
    save_header_buffer->directory.summaries[entry].maximum_mp = player_state.vitals.maximum_mp;
    memory_card_clear_events();
    file = open(save_main_file_path, O_WRONLY);
    if (file == -1) {
        return SAVE_STATUS_WRITE_FAILED;
    }
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, SEEK_SET);
        memory_card_clear_events();
        written = write(file, (const void *)save_header_buffer, header_size);
        if (written == header_size) {
            break;
        }
        index++;
    } while (index < SAVE_FILE_IO_ATTEMPTS);
    close(file);
    if (written != header_size) {
        return SAVE_STATUS_WRITE_FAILED;
    }
    return SAVE_STATUS_OK;
}

KfSaveResult save_system_read_header(void)
{
    KfSaveStatus status;
    KfSaveResult result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == KF_CARD_STATUS_NEW_DEVICE) {
        memory_card_acknowledge_new_device();
        status = SAVE_STATUS_OK;
    }
    if (status == SAVE_STATUS_OK) {
        status = save_file_read_header();
    }
    if (status != SAVE_STATUS_OK) {
        memory_card_show_status_message(status);
    }
    result = ((KfSaveResult)(((s32)(status))));
    switch (status) {
    case KF_CARD_STATUS_NOT_STARTED:
    case KF_CARD_STATUS_TIMEOUT:
    case KF_CARD_STATUS_ERROR:
        result = KF_SAVE_RESULT_FAILED;
        break;
    case SAVE_STATUS_OK:
    case KF_CARD_STATUS_NEW_DEVICE:
    case SAVE_STATUS_NO_DATA:
        result = KF_SAVE_RESULT_OK;
        break;
    }
    return result;
}

KfSaveStatus save_file_read_header(void)
{
    s32 file;
    s32 attempt;
    s32 count;
    s32 length;

    memset((void *)save_header_buffer, 0, sizeof(KfSaveHeader));
    length = sizeof(KfSaveHeader);
    memory_card_clear_events();
    file = open(save_main_file_path, O_RDONLY);
    if (file == -1) {
        return SAVE_STATUS_NO_DATA;
    }
    attempt = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, SEEK_SET);
        memory_card_clear_events();
        count = read(file, (void *)save_header_buffer, length);
        if (count == length) {
            break;
        }
        attempt++;
    } while (attempt < SAVE_FILE_IO_ATTEMPTS);
    close(file);
    if (count != length) {
        return SAVE_STATUS_READ_FAILED;
    }
    return SAVE_STATUS_OK;
}

KfSaveResult save_system_read_slot(KfSaveSlotId slot_id)
{
    KfSaveStatus status;
    KfSaveResult result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == KF_CARD_STATUS_NEW_DEVICE) {
        save_system_read_header();
        memory_card_show_status_message(SAVE_STATUS_STALE_CATALOG);
        return KF_SAVE_RESULT_FAILED;
    }
    if (status == SAVE_STATUS_OK) {
        status = save_file_read_slot(slot_id);
    }
    if (status != SAVE_STATUS_OK) {
        memory_card_show_status_message(status);
    }
    result = ((KfSaveResult)(((s32)(status))));
    switch (status) {
    case KF_CARD_STATUS_NOT_STARTED:
    case KF_CARD_STATUS_TIMEOUT:
    case KF_CARD_STATUS_NEW_DEVICE:
    case KF_CARD_STATUS_ERROR:
    case SAVE_STATUS_NO_DATA:
    case SAVE_STATUS_STALE_CATALOG:
        result = KF_SAVE_RESULT_FAILED;
        break;
    case SAVE_STATUS_OK:
        result = KF_SAVE_RESULT_OK;
        break;
    }
    return result;
}

KfSaveStatus save_file_read_slot(KfSaveSlotId slot_id)
{
    KfSaveHeader header;
    s32 file;
    s32 index;
    s32 count;
    s32 entry;
    s32 offset;
    s32 payload_size;
    s32 header_size;
    KfAssetHeader *weapon_asset_buffer;
    struct KfPoolRecord *saved_weapon_animation_cache;

    payload_size = sizeof(KfSavePayload);
    header_size = sizeof(KfSaveHeader);
    entry = SAVE_DIRECTORY_ENTRY_NOT_FOUND;
    for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
        if (save_header_buffer->directory.slot_ids[index] == slot_id) {
            entry = index;
            break;
        }
    }
    if (entry == SAVE_DIRECTORY_ENTRY_NOT_FOUND) {
        return SAVE_STATUS_NO_DATA;
    }
    memory_card_clear_events();
    file = open(save_main_file_path, O_RDONLY);
    if (file == -1) {
        return SAVE_STATUS_NO_DATA;
    }
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, SEEK_SET);
        memory_card_clear_events();
        count = read(file, (void *)&header, header_size);
        if (count == header_size) {
            break;
        }
        index++;
    } while (index < SAVE_FILE_IO_ATTEMPTS);
    if (count != header_size) {
        close(file);
        return SAVE_STATUS_READ_FAILED;
    }
    for (index = 0; index < KF_SAVE_DIRECTORY_ENTRIES; index++) {
        if (!SAVE_SUMMARY_EQUAL(save_header_buffer->directory.summaries[index],
                header.directory.summaries[index])) {
            close(file);
            return SAVE_STATUS_STALE_CATALOG;
        }
    }
    offset = payload_size * entry;
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, header_size + offset, SEEK_SET);
        memory_card_clear_events();
        count = read(file, (void *)save_payload_buffer, payload_size);
        if (count == payload_size) {
            break;
        }
        index++;
    } while (index < SAVE_FILE_IO_ATTEMPTS);
    close(file);
    if (count != payload_size) {
        return SAVE_STATUS_READ_FAILED;
    }
    weapon_asset_buffer = player_state.weapon_asset_buffer;
    saved_weapon_animation_cache = player_state.weapon_animation_cache;
    memcpy((void *)&player_state.experience, (const void *)save_payload_buffer->player_state,
           sizeof(save_payload_buffer->player_state));
    memcpy((void *)&map_runtime_state.world_state, (const void *)&save_payload_buffer->world_state,
           sizeof(save_payload_buffer->world_state));
    memcpy((void *)item_stock, (const void *)save_payload_buffer->item_stock,
           sizeof(save_payload_buffer->item_stock));
    for (index = 0; index < KF_MAGIC_RECORD_COUNT; index++) {
        effect_state.magic.entries[index].learned = save_payload_buffer->magic_flags[index];
    }
    player_state.weapon_asset_buffer = weapon_asset_buffer;
    player_state.weapon_animation_cache = saved_weapon_animation_cache;
    return SAVE_STATUS_OK;
}

s32 save_workspace_allocate(void)
{
    KfSaveWorkspace *workspace = (KfSaveWorkspace *)memory_allocate(sizeof(KfSaveWorkspace));

    save_header_buffer = workspace != NULL ? &workspace->header : NULL;
    if (save_header_buffer == NULL) {
        return -1;
    }
    save_payload_buffer = &workspace->payload;
    memset((void *)save_header_buffer, 0, sizeof(KfSaveHeader));
    memset((void *)save_payload_buffer, 0, sizeof(KfSavePayload));
    return 0;
}

void save_workspace_release(void)
{
    memory_release_last();
}

void save_file_initialize_buffers(void)
{
    u8 image[KF_CD_SECTOR_BYTES];

    memset((void *)save_header_buffer, 0, sizeof(KfSaveHeader));
    save_header_buffer->playstation_header.magic[0] = 'S';
    save_header_buffer->playstation_header.magic[1] = 'C';
    save_header_buffer->playstation_header.icon_type = KF_SAVE_ICON_THREE_FRAMES;
    save_header_buffer->playstation_header.block_count = SAVE_FILE_BLOCKS;
    memcpy((void *)save_header_buffer->playstation_header.title,
        (const void *)SAVE_TITLE_TEXT,
        sizeof(SAVE_TITLE_TEXT));
    cd_file_load_into((void *)image, "TIM\\ICO1.TIM");
    memcpy((void *)save_header_buffer->playstation_header.clut, (const void *)(&image[SAVE_ICON_TIM_CLUT_OFFSET]),
           sizeof(save_header_buffer->playstation_header.clut));
    memcpy((void *)(save_header_buffer->playstation_header.icon_frames[0]),
        (const void *)(&image[SAVE_ICON_TIM_PIXELS_OFFSET]),
        sizeof(save_header_buffer->playstation_header.icon_frames[0]));
    cd_file_load_into((void *)image, "TIM\\ICO2.TIM");
    memcpy((void *)(save_header_buffer->playstation_header.icon_frames[1]),
        (const void *)(&image[SAVE_ICON_TIM_PIXELS_OFFSET]),
        sizeof(save_header_buffer->playstation_header.icon_frames[1]));
    cd_file_load_into((void *)image, "TIM\\ICO3.TIM");
    memcpy((void *)(save_header_buffer->playstation_header.icon_frames[2]),
        (const void *)(&image[SAVE_ICON_TIM_PIXELS_OFFSET]),
        sizeof(save_header_buffer->playstation_header.icon_frames[2]));
    memset((void *)save_payload_buffer, 0, sizeof(KfSavePayload));
}

s32 memory_card_show_status_message(s16 status)
{
    s16 status_value = status;
    s16 message = ((s16)(status_value));
    KfBool32 result;

    switch (status_value) {
    case SAVE_STATUS_OK:
        message = -1;
        break;
    case KF_CARD_STATUS_TIMEOUT:
        message = SAVE_MESSAGE_NO_CARD;
        break;
    case SAVE_STATUS_FORMAT_REQUIRED:
        message = SAVE_MESSAGE_FORMAT_REQUIRED;
        break;
    case KF_CARD_STATUS_NOT_STARTED:
    case KF_CARD_STATUS_ERROR:
        message = SAVE_MESSAGE_SYSTEM_ERROR;
        break;
    case SAVE_STATUS_NO_SPACE:
        message = SAVE_MESSAGE_NO_SPACE;
        break;
    case SAVE_STATUS_FAILED:
        message = SAVE_MESSAGE_FAILED;
        break;
    case SAVE_STATUS_NO_DATA:
        message = SAVE_MESSAGE_NO_DATA;
        break;
    case SAVE_STATUS_FORMAT_CONFIRMATION:
        message = SAVE_MESSAGE_FORMAT_CONFIRMATION;
        break;
    case SAVE_STATUS_09:
    case SAVE_STATUS_10:
        message = SAVE_MESSAGE_CARD_UNUSABLE;
        break;
    case SAVE_STATUS_FORMAT_FAILED:
        message = SAVE_MESSAGE_FORMAT_FAILED;
        break;
    case SAVE_STATUS_STALE_CATALOG:
        message = SAVE_MESSAGE_KEEP_CARD_INSERTED;
        break;
    case SAVE_STATUS_READ_FAILED:
        message = SAVE_MESSAGE_READ_FAILED;
        break;
    case SAVE_STATUS_WRITE_FAILED:
        message = SAVE_MESSAGE_WRITE_FAILED;
        break;
    }
    result = menu_load_message_image(message);
    if (result == KF_TRUE) {
        return -1;
    }

}

KfBool32 menu_load_message_image(s32 message_id)
{
    char path[16] = "TIM\\M000.";
    u8 *buffer;

    if (message_id != MESSAGE_IMAGE_SKIP) {
        CD_PATH_WRITE_DECIMAL3(&path[5], message_id);
        buffer = game_graphics_runtime.display_state.primitive_buffer->cursor;
        if (cd_file_load_into((void *)buffer, path) != KF_RESOURCE_LOADED) {
            return KF_TRUE;
        }
        tim_upload_images(buffer);
    }
    return KF_FALSE;
}

KfSaveCleanupResult save_file_cleanup_temporary(void)
{
    KfSaveStatus status;
    s32 file;

    if (_card_info(KF_CARD_CHANNEL) == 0) {
        return KF_SAVE_CLEANUP_CARD_ERROR;
    }
    status = memory_card_wait_event();
    if (status == SAVE_STATUS_OK || status == KF_CARD_STATUS_NEW_DEVICE) {
        file = open(save_temporary_file_path, O_CREAT);
        close(file);
        erase(save_temporary_file_path);
        return ((KfSaveCleanupResult)(file != -1));
    }
    return ((KfSaveCleanupResult)(((s32)(status))));
}

void screen_show_image_until_input(const char *path)
{
    POLY_FT4 polygon;
    s32 brightness = IMAGE_WAIT_INITIAL_BRIGHTNESS;
    KfBool8 pressed = KF_FALSE;
    s32 index;

    DrawSync(0);
    SetPolyFT4(&polygon);
    SetSemiTrans((void *)&polygon, 1);
    setXY4(&polygon,
        KF_SYSTEM_SCREEN_LEFT, KF_SYSTEM_SCREEN_TOP,
        KF_SYSTEM_SCREEN_RIGHT, KF_SYSTEM_SCREEN_TOP,
        KF_SYSTEM_SCREEN_LEFT, KF_SYSTEM_SCREEN_BOTTOM,
        KF_SYSTEM_SCREEN_RIGHT, KF_SYSTEM_SCREEN_BOTTOM);
    setUVWH(&polygon, 0, 0, KF_SYSTEM_SCREEN_U_SPAN, KF_SYSTEM_SCREEN_V_SPAN);
    polygon.clut = GetClut(0, KF_SYSTEM_SCREEN_CLUT_Y);
    polygon.tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_SYSTEM_SCREEN_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    if (cd_file_load_into((void *)game_graphics_runtime.display_state.asset_load_buffer, path) != KF_RESOURCE_LOADED) {
        return;
    }
    tim_upload_images(game_graphics_runtime.display_state.asset_load_buffer);
    index = game_graphics_runtime.display_state.buffer_index == KF_DISPLAY_BUFFER_FIRST;
    game_graphics_runtime.display_draw_environments[index].isbg = 0;
    game_graphics_runtime.display_draw_environments[index].dfe = 0;
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[index]);
    game_graphics_runtime.display_state.ordering_table
        = game_graphics_runtime.display_state.ordering_tables[index].entries;
    for (;;) {
        if (brightness < IMAGE_WAIT_MAX_BRIGHTNESS) {
            brightness++;
        }
        setRGB0(&polygon, brightness, brightness, brightness);
        ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
        AddPrim((void *)game_graphics_runtime.display_state.ordering_table, (void *)&polygon);
        DrawSync(0);
        DrawOTag(&game_graphics_runtime.display_state.ordering_table[KF_ORDERING_TABLE_LENGTH - 1]);
        if (pressed == KF_FALSE) {
            if (PadRead(1) == 0) {
                pressed = KF_TRUE;
            }
        } else if (PadRead(1) != 0) {
            while (PadRead(1) != 0) {
            }
            break;
        }
    }
    game_graphics_runtime.display_draw_environments[index].isbg = 1;
    game_graphics_runtime.display_draw_environments[index].dfe = 1;
    DrawSync(0);
}

void talk_show_dialogue_page(u8 floor,
    u8 stage,
    s32 character_id,
    u8 page)
{
    char *directory_character = &talk_image_path_template[6];

    talk_image_path_template[0xc] = ((s32)(character_id)) / 10 + '0';
    directory_character[0] = ((s32)(character_id)) / 10 + '0';
    talk_image_path_template[0xa] = ((u8)(floor)) + '0';
    directory_character[1] = talk_image_path_template[0xd] =
        ((s32)(character_id)) % 10 + '0';
    talk_image_path_template[0xb] = stage + '0';
    talk_image_path_template[0xe] = page + '0';
    screen_show_image_until_input(directory_character - 6);
}
