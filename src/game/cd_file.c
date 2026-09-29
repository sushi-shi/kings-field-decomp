#include <kf/lib/null.h>
#include <kf/lib/bool.h>

#include <kf/game/cd.h>
#include <psyq/libc.h>
#include <kf/game/game.h>

enum {
    CD_PATH_READ_ATTEMPTS = 3,
    CD_TABLE_READ_ATTEMPTS = 5
};

char cd_path_prefix[5] = "\\KF\\";

char cd_version_suffix[3] = ";1";

CdlLOC cd_read_location;

CdlFILE cd_search_file;

char cd_path_buffer[KF_CD_PATH_BYTES];

KfCdFileEntry cd_file_table[KF_CD_FILE_TABLE_ENTRIES];

KfResourceLoadResult cd_file_load_allocated(u8 **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded;

    memcpy((void *)path, (const void *)cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        display_show_system_screen(KF_SYSTEM_SCREEN_CD_SEARCH_FAILED);
    }
    if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {
        loaded = (cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1;
        cd_search_file.size = loaded << KF_CD_SECTOR_SHIFT;
    }
    *destination = (u8 *)memory_allocate(cd_search_file.size);
    loaded = 0;
    CD_LOCATION_COPY(cd_read_location, cd_search_file.pos);
    for (attempt = 0; attempt < CD_PATH_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(cd_search_file.size >> KF_CD_SECTOR_SHIFT, (u_long *)*destination, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        display_show_system_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}

KfResourceLoadResult cd_file_load_table_entry(u8 **destination, s32 index)
{
    s32 attempt;
    KfBool32 loaded;

    *destination = (u8 *)memory_allocate(cd_file_table[index].size);
    loaded = KF_FALSE;
    CD_LOCATION_COPY(cd_read_location, cd_file_table[index]);
    for (attempt = 0; attempt < CD_TABLE_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(cd_file_table[index].size >> KF_CD_SECTOR_SHIFT, (u_long *)*destination, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
            loaded = KF_TRUE;
        }
    }
    if (loaded == KF_FALSE) {
        display_show_system_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}

KfResourceLoadResult cd_file_load_into(void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded;

    memcpy((void *)path, (const void *)cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        display_show_system_screen(KF_SYSTEM_SCREEN_CD_SEARCH_FAILED);
    }
    if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {
        loaded = (cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1;
        cd_search_file.size = loaded << KF_CD_SECTOR_SHIFT;
    }
    loaded = 0;
    CD_LOCATION_COPY(cd_read_location, cd_search_file.pos);
    for (attempt = 0; attempt < CD_PATH_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(cd_search_file.size >> KF_CD_SECTOR_SHIFT, (u_long *)destination, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        display_show_system_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}
