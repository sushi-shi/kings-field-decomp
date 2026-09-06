#ifndef KF_GAME_SAVE_H
#define KF_GAME_SAVE_H

/* GAME.EXE memory-card file layouts, shared workspace, and public save API. */

#include <kf/game_types.h>

enum {
    KF_CARD_CHANNEL = 0,
    KF_CARD_STATUS_NOT_STARTED = 0,
    KF_CARD_STATUS_IO_END = 1,
    KF_CARD_STATUS_TIMEOUT = 2,
    KF_CARD_STATUS_NEW_DEVICE = 3,
    KF_CARD_STATUS_ERROR = 4
};

enum {
    KF_SAVE_SLOT_COUNT = 3,
    KF_SAVE_DIRECTORY_ENTRIES = 4,
    KF_SAVE_SLOT_EMPTY = 0,
    KF_SAVE_SLOT_SPARE = 4
};

enum {
    KF_SAVE_ICON_THREE_FRAMES = 0x13
};

/* PlayStation card header with the three icon frames used by this game. */
typedef struct KfPsxSaveHeader {
    u8 magic[2];
    u8 icon_type;
    u8 block_count;
    u8 title[64];
    u8 zero_pad[28];
    u16 clut[16];
    u8 icon_frames[3][128]; /* Each 16-by-16 image has two 4-bit pixels per byte. */
} KfPsxSaveHeader;

/* On-card summaries widen the player's byte/halfword values to words. */
typedef struct KfSaveSlotSummary {
    u32 experience;
    u32 current_floor;
    u32 current_hp;
    u32 maximum_hp;
    u32 current_mp;
    u32 maximum_mp;
} KfSaveSlotSummary;

/* King's Field's 0x80-byte directory appended to the PlayStation header. */
typedef struct KfSaveDirectory {
    u8 slot_ids[KF_SAVE_DIRECTORY_ENTRIES];
    u8 reserved[0x1c];
    KfSaveSlotSummary summaries[KF_SAVE_DIRECTORY_ENTRIES];
} KfSaveDirectory;

/* One standard 0x200-byte PlayStation save header plus the game directory. */
typedef struct KfSaveHeader {
    KfPsxSaveHeader playstation_header;
    KfSaveDirectory directory;
} KfSaveHeader;

/*
 * Serialized slot payload. The read/write pair proves the four copied ranges
 * and their alignment; unresolved ranges deliberately remain byte arrays.
 */
typedef struct KfSavePayload {
    u32 player_state[56];
    u8 unknown_0e0[556];
    u32 world_state[2125];
    u8 item_stock[3][80];
    u8 magic_flags[24];
    u8 unknown_2548[56];
} KfSavePayload;

typedef char check_psx_save_header_size[sizeof(KfPsxSaveHeader) == 0x200 ? 1 : -1];
#define KF_PSX_SAVE_HEADER_OFFSET_CHECK(member, offset) \
    typedef char check_psx_save_header_##member[ \
        ((unsigned long)&((KfPsxSaveHeader *)0)->member == (offset)) ? 1 : -1]
KF_PSX_SAVE_HEADER_OFFSET_CHECK(magic, 0x00);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(icon_type, 0x02);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(block_count, 0x03);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(title, 0x04);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(zero_pad, 0x44);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(clut, 0x60);
KF_PSX_SAVE_HEADER_OFFSET_CHECK(icon_frames, 0x80);
#undef KF_PSX_SAVE_HEADER_OFFSET_CHECK
typedef char check_save_summary_size[sizeof(KfSaveSlotSummary) == 0x18 ? 1 : -1];
#define KF_SAVE_SUMMARY_OFFSET_CHECK(member, offset) \
    typedef char check_save_summary_##member[ \
        ((unsigned long)&((KfSaveSlotSummary *)0)->member == (offset)) ? 1 : -1]
KF_SAVE_SUMMARY_OFFSET_CHECK(experience, 0x00);
KF_SAVE_SUMMARY_OFFSET_CHECK(current_floor, 0x04);
KF_SAVE_SUMMARY_OFFSET_CHECK(current_hp, 0x08);
KF_SAVE_SUMMARY_OFFSET_CHECK(maximum_hp, 0x0c);
KF_SAVE_SUMMARY_OFFSET_CHECK(current_mp, 0x10);
KF_SAVE_SUMMARY_OFFSET_CHECK(maximum_mp, 0x14);
#undef KF_SAVE_SUMMARY_OFFSET_CHECK
typedef char check_save_directory_size[sizeof(KfSaveDirectory) == 0x80 ? 1 : -1];
typedef char check_save_header_size[sizeof(KfSaveHeader) == 0x280 ? 1 : -1];
typedef char check_save_payload_size[sizeof(KfSavePayload) == 0x2580 ? 1 : -1];

extern s32 memory_card_error_event;
extern s32 memory_card_io_end_event;
extern char memory_card_message_path_template[];
extern s32 memory_card_new_device_event;
extern const char memory_card_root_path[];
extern s32 memory_card_timeout_event;
extern KfSaveHeader *save_header_buffer;
extern const char save_main_file_path[];
extern KfSavePayload *save_payload_buffer;
extern const char save_temporary_file_path[];

extern s32 memory_card_check_or_format(s16 allow_format);
extern void memory_card_initialize(void);
extern void memory_card_shutdown_events(void);
extern s32 save_file_cleanup_temporary(void);
extern s32 save_system_read_catalog(KfSaveSlotSummary *summaries);
extern s32 save_system_read_slot(s16 slot_id);
extern s32 save_system_write_slot(s16 slot_id);

#endif
