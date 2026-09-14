#ifndef KF_GAME_SAVE_H
#define KF_GAME_SAVE_H

#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <kf/lib/map.h>
#include <kf/lib/item.h>
#include <kf/game/magic.h>

enum {
    KF_CARD_CHANNEL = 0
};

enum class KfSaveStatus : s32 {
    KF_CARD_STATUS_NOT_STARTED = 0,
    SAVE_STATUS_OK = 1,
    KF_CARD_STATUS_TIMEOUT = 2,
    KF_CARD_STATUS_NEW_DEVICE = 3,
    KF_CARD_STATUS_ERROR = 4,
    SAVE_STATUS_FORMAT_REQUIRED = KF_CARD_STATUS_NEW_DEVICE,
    SAVE_STATUS_NO_SPACE = 5,
    SAVE_STATUS_FAILED = 6,
    SAVE_STATUS_NO_DATA = 7,
    SAVE_STATUS_FORMAT_CONFIRMATION = 8,
    SAVE_STATUS_09 = 9,
    SAVE_STATUS_10 = 10,
    SAVE_STATUS_FORMAT_FAILED = 11,
    SAVE_STATUS_STALE_CATALOG = 12,
    SAVE_STATUS_READ_FAILED = 13,
    SAVE_STATUS_WRITE_FAILED = 14
}; using enum KfSaveStatus;

enum class KfSaveResult : s32 {
    KF_SAVE_RESULT_FAILED = 0,

    KF_SAVE_RESULT_OK = 1,
    KF_SAVE_RESULT_FORMAT_REQUIRED = 2,
    KF_SAVE_RESULT_NO_SPACE = 3,
    KF_SAVE_RESULT_READ_FAILED = kf_enum_encode<s32>(SAVE_STATUS_READ_FAILED)
}; using enum KfSaveResult;

enum class KfSaveCleanupResult : s32 {
    KF_SAVE_CLEANUP_TEMP_OPEN_FAILED = 0,
    KF_SAVE_CLEANUP_TEMP_OPENED = 1,
    KF_SAVE_CLEANUP_CARD_TIMEOUT = kf_enum_encode<s32>(KF_CARD_STATUS_TIMEOUT),
    KF_SAVE_CLEANUP_CARD_ERROR = kf_enum_encode<s32>(KF_CARD_STATUS_ERROR)
}; using enum KfSaveCleanupResult;

enum class KfCardFormatConfirmation : s16 {
    KF_CARD_FORMAT_UNCONFIRMED = 0,
    KF_CARD_FORMAT_CONFIRMED = 1
}; using enum KfCardFormatConfirmation;

enum {
    KF_SAVE_SLOT_COUNT = 3,
    KF_SAVE_DIRECTORY_ENTRIES = 4
};

enum class KfSaveSlotId : s16 {
    KF_SAVE_SLOT_EMPTY = 0,
    KF_SAVE_SLOT_FIRST = 1,
    KF_SAVE_SLOT_SECOND = 2,
    KF_SAVE_SLOT_THIRD = 3,
    KF_SAVE_SLOT_SPARE = 4
}; using enum KfSaveSlotId;

typedef KfSaveSlotId KfSaveSlotArgument;

enum class KfSaveIconType : u8 {
    KF_SAVE_ICON_THREE_FRAMES = 0x13
}; using enum KfSaveIconType;

enum {
    KF_SAVE_MAGIC_BYTES = 2,
    KF_SAVE_TITLE_BYTES = 64,
    KF_SAVE_ICON_PALETTE_COLORS = 16,
    KF_SAVE_ICON_FRAME_COUNT = 3,
    KF_SAVE_ICON_FRAME_BYTES = 128,
    KF_SAVE_PLAYER_STATE_WORDS = 56
};

typedef struct KfPsxSaveHeader {
    u8 magic[KF_SAVE_MAGIC_BYTES];
    KfSaveIconType icon_type;
    u8 block_count;
    u8 title[KF_SAVE_TITLE_BYTES];
    u8 zero_pad[28];
    u16 clut[KF_SAVE_ICON_PALETTE_COLORS];

    u8 icon_frames[KF_SAVE_ICON_FRAME_COUNT][KF_SAVE_ICON_FRAME_BYTES];
} KfPsxSaveHeader;

typedef struct KfSaveSlotSummary {
    u32 experience;
    KfEnumStorage<KfFloorId, u32> current_floor;
    u32 current_hp;
    u32 maximum_hp;
    u32 current_mp;
    u32 maximum_mp;
} KfSaveSlotSummary;

typedef struct KfSaveDirectory {
    KfEnumStorage<KfSaveSlotId, u8> slot_ids[KF_SAVE_DIRECTORY_ENTRIES];
    u8 reserved[0x1c];
    KfSaveSlotSummary summaries[KF_SAVE_DIRECTORY_ENTRIES];
} KfSaveDirectory;

typedef struct KfSaveHeader {
    KfPsxSaveHeader playstation_header;
    KfSaveDirectory directory;
} KfSaveHeader;

typedef struct KfSavePayload {
    u32 player_state[KF_SAVE_PLAYER_STATE_WORDS];
    u8 unknown_0e0[556];
    KfMapSavedWorld world_state;
    u8 item_stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];
    KfMagicLearningState magic_flags[KF_MAGIC_RECORD_COUNT];
    u8 unknown_2548[56];
} KfSavePayload;

typedef struct KfSaveWorkspace {
    KfSaveHeader header;
    KfSavePayload payload;
} KfSaveWorkspace;

extern char memory_card_message_path_template[];
extern char memory_card_root_path[];
extern KfSaveHeader *save_header_buffer;
extern char save_main_file_path[];
extern KfSavePayload *save_payload_buffer;
extern char save_temporary_file_path[];

extern KfSaveResult memory_card_check_or_format(KfCardFormatConfirmation format_confirmation);
extern void memory_card_initialize(void);
extern void memory_card_shutdown_events(void);
extern KfSaveCleanupResult save_file_cleanup_temporary(void);
extern KfSaveResult save_system_read_catalog(KfSaveSlotSummary *summaries);
extern KfSaveResult save_system_read_slot(KfSaveSlotId slot_id);
extern KfSaveResult save_system_write_slot(KfSaveSlotId slot_id);

#endif
