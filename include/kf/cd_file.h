#ifndef KF_CD_FILE_H
#define KF_CD_FILE_H

/* King's Field file loading and state at the Psy-Q CD-ROM boundary. */

#include <kf/game_types.h>
#include <kf/psyq_cd.h>

extern CdlFILE cd_search_file;
extern CdlLOC cd_read_location;
extern char cd_path_buffer[80];

extern s32 cd_file_load_allocated(
    void **destination, const char *relative_path);
extern s32 cd_file_load_into(
    void *destination, const char *relative_path);

#endif
