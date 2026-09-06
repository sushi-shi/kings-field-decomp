#ifndef KF_CD_FILE_H
#define KF_CD_FILE_H

/* King's Field file loading and state at the Psy-Q CD-ROM boundary. */

#include <kf/game_types.h>
#include <kf/psyq_cd.h>

enum {
    KF_CD_SECTOR_BYTES = 0x800,
    KF_CD_SECTOR_SHIFT = 11,
    KF_CD_PATH_BYTES = 80,
    KF_CD_READ_POLL = 1,
    KF_CD_READ_STOP_ATTEMPT = 100
};

extern CdlFILE cd_search_file;
extern char cd_path_buffer[KF_CD_PATH_BYTES];

extern s32 cd_file_load_allocated(
    void **destination, const char *relative_path);
extern s32 cd_file_load_into(
    void *destination, const char *relative_path);

#endif
