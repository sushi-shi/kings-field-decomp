#ifndef KF_GAME_SAVE_H
#define KF_GAME_SAVE_H

/* GAME.EXE memory-card file layouts, shared workspace, and public save API. */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <kf/game_map.h>
#include <kf/item.h>
#include <kf/magic.h>

enum {
    KF_CARD_CHANNEL = 0
};

/* Card events and file-policy statuses share this internal channel. */
KF_ENUM_BEGIN(KfSaveStatus, s32)
    KF_CARD_STATUS_NOT_STARTED = 0,
    KF_CARD_STATUS_IO_END = 1,
    KF_CARD_STATUS_TIMEOUT = 2,
    KF_CARD_STATUS_NEW_DEVICE = 3,
    KF_CARD_STATUS_ERROR = 4,
    SAVE_STATUS_OK = KF_CARD_STATUS_IO_END,
    SAVE_STATUS_FORMAT_REQUIRED = KF_CARD_STATUS_NEW_DEVICE,
    SAVE_STATUS_NO_SPACE = 5,
    SAVE_STATUS_FAILED = 6,
    SAVE_STATUS_NO_DATA = 7,
    SAVE_STATUS_FORMAT_CONFIRMATION = 8,
    SAVE_STATUS_FORMAT_FAILED = 11,
    SAVE_STATUS_STALE_CATALOG = 12,
    SAVE_STATUS_READ_FAILED = 13,
    SAVE_STATUS_WRITE_FAILED = 14
KF_ENUM_END(KfSaveStatus)

/* Menu mappings retain unlisted encoded statuses, including read failure. */
KF_ENUM_BEGIN(KfSaveResult, s32)
    KF_SAVE_RESULT_FAILED = 0,
    /* Header reads also accept an empty catalogue with this result. */
    KF_SAVE_RESULT_OK = 1,
    KF_SAVE_RESULT_FORMAT_REQUIRED = 2,
    KF_SAVE_RESULT_FORMAT_CONFIRMATION = 2,
    KF_SAVE_RESULT_NO_SPACE = 3,
    KF_SAVE_RESULT_READ_FAILED = KF_ENUM_ENCODE(s32, SAVE_STATUS_READ_FAILED)
KF_ENUM_END(KfSaveResult)

/* The cleanup path reports open success, not the result of close or erase. */
KF_ENUM_BEGIN(KfSaveCleanupResult, s32)
    KF_SAVE_CLEANUP_TEMP_OPEN_FAILED = 0,
    KF_SAVE_CLEANUP_TEMP_OPENED = 1,
    KF_SAVE_CLEANUP_CARD_TIMEOUT = KF_ENUM_ENCODE(s32, KF_CARD_STATUS_TIMEOUT),
    KF_SAVE_CLEANUP_CARD_ERROR = KF_ENUM_ENCODE(s32, KF_CARD_STATUS_ERROR)
KF_ENUM_END(KfSaveCleanupResult)

/* Checked on the I/O-end path; new-device handling has its own format path. */
KF_ENUM_BEGIN(KfCardFormatConfirmation, s16)
    KF_CARD_FORMAT_UNCONFIRMED = 0,
    KF_CARD_FORMAT_CONFIRMED = 1
KF_ENUM_END(KfCardFormatConfirmation)

enum {
    KF_SAVE_SLOT_COUNT = 3,
    KF_SAVE_DIRECTORY_ENTRIES = 4
};

/* Directory tags use bytes; read/write APIs take signed-halfword slot IDs. */
KF_ENUM_BEGIN(KfSaveSlotId, s16)
    KF_SAVE_SLOT_EMPTY = 0,
    KF_SAVE_SLOT_FIRST = 1,
    KF_SAVE_SLOT_SECOND = 2,
    KF_SAVE_SLOT_THIRD = 3,
    KF_SAVE_SLOT_SPARE = 4
KF_ENUM_END(KfSaveSlotId)

/* MIPS promotes the signed-halfword formal before converting call arguments. */
typedef KF_ENUM_PROMOTED(KfSaveSlotId) KfSaveSlotArgument;

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
    KF_ENUM_STORAGE(KfFloorId, u32) current_floor;
    u32 current_hp;
    u32 maximum_hp;
    u32 current_mp;
    u32 maximum_mp;
} KfSaveSlotSummary;

/* King's Field's 0x80-byte directory appended to the PlayStation header. */
typedef struct KfSaveDirectory {
    KF_ENUM_STORAGE(KfSaveSlotId, u8) slot_ids[KF_SAVE_DIRECTORY_ENTRIES];
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
    KfMapSavedWorld world_state;
    u8 item_stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];
    KfMagicLearningState magic_flags[KF_MAGIC_RECORD_COUNT];
    u8 unknown_2548[56];
} KfSavePayload;

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

extern KfSaveResult memory_card_check_or_format(KfCardFormatConfirmation confirmation);
extern void memory_card_initialize(void);
extern void memory_card_shutdown_events(void);
extern KfSaveCleanupResult save_file_cleanup_temporary(void);
extern KfSaveResult save_system_read_catalog(KfSaveSlotSummary *summaries);
extern KfSaveResult save_system_read_slot(KfSaveSlotId slot_id);
extern KfSaveResult save_system_write_slot(KfSaveSlotId slot_id);

#endif
