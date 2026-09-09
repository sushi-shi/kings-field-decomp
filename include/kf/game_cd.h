#ifndef KF_GAME_CD_H
#define KF_GAME_CD_H

/* GAME.EXE additions to the shared CD-file interface. */
#include <kf/cd_file.h>

enum {
    KF_CD_FILE_TABLE_ENTRIES = 80,
    KF_CD_FILE_NAME_BYTES = 12
};

/* Shared by the normal CD loaders and GAME's fatal-error screen loader. */
extern CdlLOC cd_read_location;

/*
 * GAME's file table uses the older 20-byte CdlFILE layout with a 12-byte
 * name, rather than the 24-byte type in the supplied Release 2.5 header.
 */
typedef struct KfCdFileEntry {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
    u32 size;
    u8 name[KF_CD_FILE_NAME_BYTES];
} KfCdFileEntry;

/*
 * Retail indexes these as 20-byte records from an older CdlFILE layout. The
 * supplied Release 2.5 header's CdlFILE has a 16-byte name and is 24 bytes, so
 * callers cast only after selecting a table entry with the proven 20-byte
 * stride.
 */
extern KfCdFileEntry cd_file_table[KF_CD_FILE_TABLE_ENTRIES];
extern KfResourceLoadResult cd_file_load_table_entry(void **destination, s32 index);

#endif
