#include <kf/address.h>
#include <kf/game_save.h>
#include <kf/game_player.h>
#include <kf/psyq_audio.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Jump tables and string literals of this unit in the retail data region. */
RODATA(0x8001235c, 0x178)

/* Memory-card create requests carry the block count in the high halfword. */
#define SAVE_FILE_BLOCKS 5
/* Shift-JIS card title; retail keeps it as a literal in this unit's read-only data. */
#define SAVE_TITLE_TEXT \
    "\201@\201@\201@\201@\201@\201@\201\203\201\203\201@\201@\202j\202h\202m\202f" \
    "\201f\202r\201@\202e\202h\202d\202k\202c\201@\201@\201\204\201\204"

/*
 * Serialized game state. The 0xe0 bytes from player_state.experience onward, the
 * two unresolved blocks, and the first byte of each 20-byte magic record are
 * copied verbatim; the retail object boundaries inside them are still open.
 */

void memory_card_clear_events(void);
s32 memory_card_wait_event(void);
void memory_card_undeliver_events(void);
s32 memory_card_format(void);
s32 save_file_write_slot(s16 slot_id);
s32 save_system_read_header(void);
s32 save_file_read_header(void);
s32 save_file_read_slot(s16 slot_id);
void save_file_initialize_buffers(void);
s32 memory_card_show_status_message(s16 status);
s32 menu_load_message_image(s32 message_id);
void screen_show_image_until_input(const char *path);

ADDRESS(0x8002b078, 0xd8)
s32 save_system_read_catalog(KfSaveSlotSummary *summaries)
{
    s32 result;
    s32 index;
    KfSaveHeader *header;

    /* Retail clears 0x24 bytes although three summaries span 0x48. */
    memset(summaries, 0, 0x24);
    result = save_system_read_header();
    if (result == 1) {
        header = save_header_buffer;
        for (index = 0; index < 4; index++) {
            u8 slot = header->directory.slot_ids[index];

            if (slot != 0 && slot != 4) {
                s32 entry = slot - 1;

                summaries[entry].fields[0] = header->directory.summaries[index].fields[0];
                summaries[entry].fields[1] = header->directory.summaries[index].fields[1];
                summaries[entry].fields[2] = header->directory.summaries[index].fields[2];
                summaries[entry].fields[3] = header->directory.summaries[index].fields[3];
                summaries[entry].fields[4] = header->directory.summaries[index].fields[4];
                summaries[entry].fields[5] = header->directory.summaries[index].fields[5];
            }
        }
    }
    return result;
}

ADDRESS(0x8002b150, 0x84)
void menu_play_input_sound(s32 cue)
{
    SoundRef sound;

    if (cue == 0) {
        sound.program = 0xe;
        sound.note = 0x44;
    } else if (cue == 1) {
        sound.program = 0xd;
        sound.note = 0x3c;
    } else {
        sound.program = 0xf;
        sound.note = 0x3f;
    }
    SsVoKeyOn(sound.program, sound.note << 8, 0x40, 0x40);
    VSync(0);
    SsVoKeyOff(sound.program, sound.note << 8);
}

ADDRESS(0x8002b1d4, 0x100)
void memory_card_initialize(void)
{
    u8 buffer[0x80];

    memset(buffer, 0xff, sizeof(buffer));
    memory_card_io_end_event = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    memory_card_timeout_event = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    memory_card_new_device_event = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    memory_card_error_event = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    EnableEvent(memory_card_io_end_event);
    EnableEvent(memory_card_timeout_event);
    EnableEvent(memory_card_new_device_event);
    EnableEvent(memory_card_error_event);
    StartCARD2();
    _bu_init();
    _card_auto(0);
}

ADDRESS(0x8002b2d4, 0x60)
void memory_card_shutdown_events(void)
{
    StopCARD2();
    CloseEvent(memory_card_io_end_event);
    CloseEvent(memory_card_timeout_event);
    CloseEvent(memory_card_new_device_event);
    CloseEvent(memory_card_error_event);
}

ADDRESS(0x8002b334, 0x38)
s32 memory_card_begin_status_check(void)
{
    memory_card_clear_events();
    if (_card_info(0) != 0) {
        return memory_card_wait_event();
    }
    return 0;
}

ADDRESS(0x8002b36c, 0x58)
void memory_card_clear_events(void)
{
    TestEvent(memory_card_io_end_event);
    TestEvent(memory_card_timeout_event);
    TestEvent(memory_card_new_device_event);
    TestEvent(memory_card_error_event);
}

ADDRESS(0x8002b3c4, 0xbc)
s32 memory_card_wait_event(void)
{
    for (;;) {
        if (TestEvent(memory_card_io_end_event) == 1) {
            memory_card_undeliver_events();
            return 1;
        }
        if (TestEvent(memory_card_timeout_event) == 1) {
            memory_card_undeliver_events();
            return 2;
        }
        if (TestEvent(memory_card_new_device_event) == 1) {
            memory_card_undeliver_events();
            return 3;
        }
        if (TestEvent(memory_card_error_event) == 1) {
            memory_card_undeliver_events();
            return 4;
        }
    }
}

ADDRESS(0x8002b480, 0x58)
void memory_card_undeliver_events(void)
{
    UnDeliverEvent(memory_card_io_end_event);
    UnDeliverEvent(memory_card_timeout_event);
    UnDeliverEvent(memory_card_new_device_event);
    UnDeliverEvent(memory_card_error_event);
}

ADDRESS(0x8002b4d8, 0xf8)
s32 memory_card_check_or_format(s16 allow_format)
{
    s32 status;
    s32 result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == 3) {
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        status = memory_card_format();
    } else if (status == 1) {
        if (allow_format == 0) {
            status = 8;
        } else {
            status = memory_card_format();
        }
    }
    if (status != 1) {
        memory_card_show_status_message(status);
    }
    result = status;
    switch (result) {
    case 0:
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
    case 11:
        result = 0;
        break;
    case 1:
        result = 1;
        break;
    case 8:
        result = 2;
        break;
    }
    return result;
}

ADDRESS(0x8002b5d0, 0x78)
s32 memory_card_format(void)
{
    s32 attempt = 0;
    s32 formatted;
    s32 status;

    do {
        memory_card_clear_events();
        formatted = format(memory_card_root_path);
        status = memory_card_begin_status_check();
        if (formatted == 1) {
            break;
        }
        attempt++;
    } while (attempt < 5);
    if (formatted != 1) {
        return 0xb;
    }
    return status;
}

ADDRESS(0x8002b648, 0xf4)
s32 save_system_write_slot(s16 slot_id)
{
    s32 status;
    s32 result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == 3) {
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        status = 1;
    }
    if (status == 1) {
        status = save_file_write_slot(slot_id);
    }
    if (status != 1) {
        memory_card_show_status_message(status);
    }
    result = status;
    switch (result) {
    case 0:
    case 2:
    case 4:
    case 14:
        result = 0;
        break;
    case 1:
        result = 1;
        break;
    case 3:
        result = 2;
        break;
    case 5:
        result = 3;
        break;
    }
    return result;
}

ADDRESS(0x8002b73c, 0x4f4)
s32 save_file_write_slot(s16 slot_id)
{
    s32 file;
    s32 index;
    s32 written;
    s32 entry;
    s32 previous;
    s32 offset;
    s32 record;
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
                return 3;
            }
            return 5;
        }
        save_file_initialize_buffers();
    }
    entry = -1;
    for (index = 0; index < 4; index++) {
        if (save_header_buffer->directory.slot_ids[index] == 4) {
            entry = index;
            break;
        }
    }
    if (entry == -1) {
        for (index = 0; index < 4; index++) {
            if (save_header_buffer->directory.slot_ids[index] == 0) {
                entry = index;
                break;
            }
        }
    }
    previous = -1;
    for (index = 0; index < 4; index++) {
        if (save_header_buffer->directory.slot_ids[index] == slot_id) {
            previous = index;
            break;
        }
    }
    memcpy(save_payload_buffer->player_state, &player_state.experience,
           sizeof(save_payload_buffer->player_state));
    memcpy(save_payload_buffer->world_state, &map_world_state_base,
           sizeof(save_payload_buffer->world_state));
    memcpy(save_payload_buffer->item_stock, item_stock,
           sizeof(save_payload_buffer->item_stock));
    for (index = 0, record = 0; index < 24; index++) {
        save_payload_buffer->magic_flags[index] = magic_records[index].learned;
        record += 20;
    }
    memory_card_clear_events();
    file = open(save_main_file_path, O_WRONLY);
    if (file == -1) {
        return 0xe;
    }
    offset = payload_size * entry;
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, header_size + offset, 0);
        memory_card_clear_events();
        written = write(file, save_payload_buffer, payload_size);
        if (written == payload_size) {
            break;
        }
        index++;
    } while (index < 5);
    close(file);
    if (written != payload_size) {
        return 0xe;
    }
    save_header_buffer->directory.slot_ids[entry] = slot_id;
    save_header_buffer->directory.slot_ids[previous] = 4;
    save_header_buffer->directory.summaries[entry].fields[0] = player_state.experience;
    save_header_buffer->directory.summaries[entry].fields[1] = player_state.progress_state.current_floor;
    save_header_buffer->directory.summaries[entry].fields[2] = player_state.vitals.current_hp;
    save_header_buffer->directory.summaries[entry].fields[3] = player_state.vitals.maximum_hp;
    save_header_buffer->directory.summaries[entry].fields[4] = player_state.vitals.current_mp;
    save_header_buffer->directory.summaries[entry].fields[5] = player_state.vitals.maximum_mp;
    memory_card_clear_events();
    file = open(save_main_file_path, O_WRONLY);
    if (file == -1) {
        return 0xe;
    }
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, 0);
        memory_card_clear_events();
        written = write(file, save_header_buffer, header_size);
        if (written == header_size) {
            break;
        }
        index++;
    } while (index < 5);
    close(file);
    if (written != header_size) {
        return 0xe;
    }
    return 1;
}

ADDRESS(0x8002bc30, 0xd8)
s32 save_system_read_header(void)
{
    s32 status;
    s32 result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == 3) {
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        memory_card_clear_events();
        _new_card();
        memory_card_begin_status_check();
        status = 1;
    }
    if (status == 1) {
        status = save_file_read_header();
    }
    if (status != 1) {
        memory_card_show_status_message(status);
    }
    result = status;
    switch (result) {
    case 0:
    case 2:
    case 4:
        result = 0;
        break;
    case 1:
    case 3:
    case 7:
        result = 1;
        break;
    }
    return result;
}

ADDRESS(0x8002bd08, 0xdc)
s32 save_file_read_header(void)
{
    s32 file;
    s32 attempt;
    s32 count;
    s32 length;

    memset(save_header_buffer, 0, sizeof(KfSaveHeader));
    length = sizeof(KfSaveHeader);
    memory_card_clear_events();
    file = open(save_main_file_path, O_RDONLY);
    if (file == -1) {
        return 7;
    }
    attempt = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, 0);
        memory_card_clear_events();
        count = read(file, save_header_buffer, length);
        if (count == length) {
            break;
        }
        attempt++;
    } while (attempt < 5);
    close(file);
    if (count != length) {
        return 0xd;
    }
    return 1;
}

ADDRESS(0x8002bde4, 0xcc)
s32 save_system_read_slot(s16 slot_id)
{
    s32 status;
    s32 result;

    memory_card_clear_events();
    status = memory_card_begin_status_check();
    if (status == 3) {
        save_system_read_header();
        memory_card_show_status_message(0xc);
        return 0;
    }
    if (status == 1) {
        status = save_file_read_slot(slot_id);
    }
    if (status != 1) {
        memory_card_show_status_message(status);
    }
    result = status;
    switch (result) {
    case 0:
    case 2:
    case 3:
    case 4:
    case 7:
    case 12:
        result = 0;
        break;
    case 1:
        result = 1;
        break;
    }
    return result;
}

ADDRESS(0x8002beb0, 0x3cc)
s32 save_file_read_slot(s16 slot_id)
{
    KfSaveHeader header;
    s32 file;
    s32 index;
    s32 count;
    s32 entry;
    s32 offset;
    s32 record;
    s32 payload_size;
    s32 header_size;
    u8 *weapon_asset_buffer;
    struct KfPoolRecord *saved_weapon_animation_cache;

    payload_size = sizeof(KfSavePayload);
    header_size = sizeof(KfSaveHeader);
    entry = -1;
    for (index = 0; index < 4; index++) {
        if (save_header_buffer->directory.slot_ids[index] == slot_id) {
            entry = index;
            break;
        }
    }
    if (entry == -1) {
        return 7;
    }
    memory_card_clear_events();
    file = open(save_main_file_path, O_RDONLY);
    if (file == -1) {
        return 7;
    }
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, 0, 0);
        memory_card_clear_events();
        count = read(file, &header, header_size);
        if (count == header_size) {
            break;
        }
        index++;
    } while (index < 5);
    if (count != header_size) {
        close(file);
        return 0xd;
    }
    for (index = 0; index < 4; index++) {
        if (save_header_buffer->directory.summaries[index].fields[0]
                != header.directory.summaries[index].fields[0]
            || save_header_buffer->directory.summaries[index].fields[1]
                != header.directory.summaries[index].fields[1]
            || save_header_buffer->directory.summaries[index].fields[2]
                != header.directory.summaries[index].fields[2]
            || save_header_buffer->directory.summaries[index].fields[3]
                != header.directory.summaries[index].fields[3]
            || save_header_buffer->directory.summaries[index].fields[4]
                != header.directory.summaries[index].fields[4]
            || save_header_buffer->directory.summaries[index].fields[5]
                != header.directory.summaries[index].fields[5]) {
            close(file);
            return 0xc;
        }
    }
    offset = payload_size * entry;
    index = 0;
    do {
        memory_card_clear_events();
        lseek(file, header_size + offset, 0);
        memory_card_clear_events();
        count = read(file, save_payload_buffer, payload_size);
        if (count == payload_size) {
            break;
        }
        index++;
    } while (index < 5);
    close(file);
    if (count != payload_size) {
        return 0xd;
    }
    weapon_asset_buffer = player_state.weapon_asset_buffer;
    saved_weapon_animation_cache = player_state.weapon_animation_cache;
    memcpy(&player_state.experience, save_payload_buffer->player_state,
           sizeof(save_payload_buffer->player_state));
    memcpy(&map_world_state_base, save_payload_buffer->world_state,
           sizeof(save_payload_buffer->world_state));
    memcpy(item_stock, save_payload_buffer->item_stock,
           sizeof(save_payload_buffer->item_stock));
    for (index = 0, record = 0; index < 24; index++) {
        magic_records[index].learned = save_payload_buffer->magic_flags[index];
        record += 20;
    }
    player_state.weapon_asset_buffer = weapon_asset_buffer;
    player_state.weapon_animation_cache = saved_weapon_animation_cache;
    return 1;
}

ADDRESS(0x8002c27c, 0x68)
s32 save_workspace_allocate(void)
{
    save_header_buffer = memory_allocate(0x2800);
    if (save_header_buffer == 0) {
        return -1;
    }
    save_payload_buffer = (KfSavePayload *)(save_header_buffer + 1);
    memset(save_header_buffer, 0, sizeof(KfSaveHeader));
    memset(save_payload_buffer, 0, sizeof(KfSavePayload));
    return 0;
}

ADDRESS(0x8002c2e4, 0x20)
void save_workspace_release(void)
{
    memory_release_last();
}

ADDRESS(0x8002c304, 0x20c)
void save_file_initialize_buffers(void)
{
    u8 image[0x800];

    memset(save_header_buffer, 0, sizeof(KfSaveHeader));
    save_header_buffer->playstation_header[0] = 'S';
    save_header_buffer->playstation_header[1] = 'C';
    save_header_buffer->playstation_header[2] = 0x13;
    save_header_buffer->playstation_header[3] = SAVE_FILE_BLOCKS;
    memcpy(&save_header_buffer->playstation_header[4], SAVE_TITLE_TEXT, sizeof(SAVE_TITLE_TEXT));
    cd_file_load_into(image, "TIM\\ICO1.TIM");
    memcpy(&save_header_buffer->playstation_header[0x60], &image[0x14], 0x20);
    memcpy(&save_header_buffer->playstation_header[0x80], &image[0x40], 0x80);
    cd_file_load_into(image, "TIM\\ICO2.TIM");
    memcpy(&save_header_buffer->playstation_header[0x100], &image[0x40], 0x80);
    cd_file_load_into(image, "TIM\\ICO3.TIM");
    memcpy(&save_header_buffer->playstation_header[0x180], &image[0x40], 0x80);
    memset(save_payload_buffer, 0, sizeof(KfSavePayload));
}

ADDRESS(0x8002c510, 0xd0)
s32 memory_card_show_status_message(s16 status)
{
    s16 message = status;
    s32 result;

    switch (status) {
    case 1:
        message = -1;
        break;
    case 2:
        message = 0x65;
        break;
    case 3:
        message = 0x6d;
        break;
    case 0:
    case 4:
        message = 0x71;
        break;
    case 5:
        message = 0x66;
        break;
    case 6:
        message = 0x72;
        break;
    case 7:
        message = 0x6f;
        break;
    case 8:
        message = 0x73;
        break;
    case 9:
    case 10:
        message = 0x6e;
        break;
    case 11:
        message = 0x6c;
        break;
    case 12:
        message = 0x67;
        break;
    case 13:
        message = 0x70;
        break;
    case 14:
        message = 0x6b;
        break;
    }
    result = menu_load_message_image(message);
    if (result != 1) {
        return result;
    }
    return -1;
}

ADDRESS(0x8002c5e0, 0x12c)
s32 menu_load_message_image(s32 message_id)
{
    char path[16] = "TIM\\M000.";
    void *buffer;
    s32 remainder;

    if (message_id != 0xff) {
        remainder = message_id % 100;
        path[5] = message_id / 100 + '0';
        path[6] = remainder / 10 + '0';
        path[7] = remainder % 10 + '0';
        buffer = display_state.primitive_buffer->cursor;
        if (cd_file_load_into(buffer, path) != 0) {
            return 1;
        }
        tim_upload_images(buffer);
    }
    return 0;
}

ADDRESS(0x8002c70c, 0x88)
s32 save_file_cleanup_temporary(void)
{
    s32 status;
    s32 file;

    if (_card_info(0) == 0) {
        return 4;
    }
    status = memory_card_wait_event();
    if (status == 1 || status == 3) {
        file = open(save_temporary_file_path, O_CREAT);
        close(file);
        erase(save_temporary_file_path);
        return file != -1;
    }
    return status;
}

ADDRESS(0x8002c794, 0x240)
void screen_show_image_until_input(const char *path)
{
    POLY_FT4 polygon;
    s32 brightness = 0x20;
    u8 pressed = 0;
    s32 index;

    DrawSync(0);
    SetPolyFT4(&polygon);
    SetSemiTrans(&polygon, 1);
    polygon.x0 = 0x20;
    polygon.y0 = 0x70;
    polygon.x1 = 0x120;
    polygon.y1 = 0x70;
    polygon.x2 = 0x20;
    polygon.y2 = 0xf0;
    polygon.x3 = 0x120;
    polygon.y3 = 0xf0;
    polygon.u0 = 0;
    polygon.v0 = 0;
    polygon.u1 = 0xff;
    polygon.v1 = 0;
    polygon.u2 = 0;
    polygon.v2 = 0x80;
    polygon.u3 = 0xff;
    polygon.v3 = 0x80;
    polygon.clut = GetClut(0, 0x1f5);
    polygon.tpage = GetTPage(0, 0, 0x3c0, 0x100);
    if (cd_file_load_into(display_state.asset_load_buffer, path) != 0) {
        return;
    }
    tim_upload_images(display_state.asset_load_buffer);
    index = display_state.buffer_index == 0;
    display_draw_environments[index].isbg = 0;
    display_draw_environments[index].dfe = 0;
    PutDrawEnv(&display_draw_environments[index]);
    display_state.ordering_table = display_state.ordering_tables[index].entries;
    for (;;) {
        if (brightness < 127) {
            brightness++;
        }
        polygon.r0 = brightness;
        polygon.g0 = brightness;
        polygon.b0 = brightness;
        ClearOTagR(display_state.ordering_table, 0x4000);
        AddPrim(display_state.ordering_table, &polygon);
        DrawSync(0);
        DrawOTag(&display_state.ordering_table[0x3fff]);
        if (pressed == 0) {
            if (PadRead(1) == 0) {
                pressed = 1;
            }
        } else if (PadRead(1) != 0) {
            while (PadRead(1) != 0) {
            }
            break;
        }
    }
    display_draw_environments[index].isbg = 1;
    display_draw_environments[index].dfe = 1;
    DrawSync(0);
}

ADDRESS(0x8002c9d4, 0xa4)
void talk_show_indexed_image(u8 prefix_digit, u8 index_digit, s32 group_id, u8 frame_digit)
{
    s32 tens = group_id / 10;
    s32 ones = group_id % 10;

    char *directory_group = &talk_image_path_template[6];

    talk_image_path_template[0xa] = prefix_digit + '0';
    talk_image_path_template[0xb] = index_digit + '0';
    talk_image_path_template[0xe] = frame_digit + '0';
    directory_group[0] = talk_image_path_template[0xc] = tens + '0';
    directory_group[1] = talk_image_path_template[0xd] = ones + '0';
    screen_show_image_until_input(talk_image_path_template);
}
