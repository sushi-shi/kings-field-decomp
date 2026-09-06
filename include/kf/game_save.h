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

/* Six on-card summary words; their individual meanings remain unresolved. */
typedef struct KfSaveSlotSummary {
    u32 fields[6];
} KfSaveSlotSummary;

/* King's Field's 0x80-byte directory appended to the PlayStation header. */
typedef struct KfSaveDirectory {
    u8 slot_ids[4];
    u8 reserved[0x1c];
    KfSaveSlotSummary summaries[4];
} KfSaveDirectory;

/* One standard 0x200-byte PlayStation save header plus the game directory. */
typedef struct KfSaveHeader {
    u8 playstation_header[0x200];
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
