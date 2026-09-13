#ifndef KF_GAME_CD_H
#define KF_GAME_CD_H

#include <kf/cd_file.h>

enum {
    KF_CD_FILE_TABLE_ENTRIES = 80,
    KF_CD_FILE_NAME_BYTES = 12
};

extern CdlLOC cd_read_location;
extern CdlFILE cd_search_file;
extern char cd_path_buffer[KF_CD_PATH_BYTES];

typedef struct KfCdFileEntry {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
    u32 size;
    u8 name[KF_CD_FILE_NAME_BYTES];
} KfCdFileEntry;

extern KfCdFileEntry cd_file_table[KF_CD_FILE_TABLE_ENTRIES];
extern KfResourceLoadResult cd_file_load_table_entry(u8 **destination, s32 index);

#endif
