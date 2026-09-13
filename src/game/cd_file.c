#include <kf/null.h>
#include <kf/bool.h>
#include <kf/address.h>
#include <kf/game_cd.h>
#include <psyq/libc.h>
#include <kf/game.h>

enum {
    CD_PATH_READ_ATTEMPTS = 3,
    CD_TABLE_READ_ATTEMPTS = 5
};

DATA(0x80057b3c, 0x5)
char cd_path_prefix[5] = "\\KF\\";
DATA(0x80057b44, 0x3)
char cd_version_suffix[3] = ";1";
DATA(0x80057e80, 0x4)
CdlLOC cd_read_location;

DATA(0x80058080, 0x18)
CdlFILE cd_search_file;

DATA(0x80058098, 0x50)
char cd_path_buffer[KF_CD_PATH_BYTES];

DATA(0x8006b730, 0x640)
KfCdFileEntry cd_file_table[KF_CD_FILE_TABLE_ENTRIES];

/* Loads \KF\<relative_path>;1 into a fresh arena allocation. */
ADDRESS(0x8001acf0, 0x170)
KfResourceLoadResult cd_file_load_allocated(u8 **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded; /* also the rounded sector count; retail keeps both in $s1 */

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        display_show_error_screen(KF_SYSTEM_SCREEN_CD_SEARCH_FAILED);
    }
    if (cd_search_file.size & (KF_CD_SECTOR_BYTES - 1)) {
        loaded = (cd_search_file.size >> KF_CD_SECTOR_SHIFT) + 1;
        cd_search_file.size = loaded << KF_CD_SECTOR_SHIFT;
    }
    *destination = memory_allocate(cd_search_file.size);
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
        display_show_error_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}

/* Loads the file table entry INDEX into a fresh arena allocation. */
ADDRESS(0x8001ae60, 0x13c)
KfResourceLoadResult cd_file_load_table_entry(void **destination, s32 index)
{
    s32 attempt;
    KfBool32 loaded;

    *destination = memory_allocate(cd_file_table[index].size);
    loaded = KF_FALSE;
    CD_LOCATION_COPY(cd_read_location, cd_file_table[index]);
    for (attempt = 0; attempt < CD_TABLE_READ_ATTEMPTS; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, NULL);
        CdRead(cd_file_table[index].size >> KF_CD_SECTOR_SHIFT, *destination, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
            loaded = KF_TRUE;
        }
    }
    if (loaded == KF_FALSE) {
        display_show_error_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}

/* Loads \KF\<relative_path>;1 into DESTINATION. */
ADDRESS(0x8001af9c, 0x164)
KfResourceLoadResult cd_file_load_into(void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded; /* also the rounded sector count; retail keeps both in $s1 */

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == NULL) {
        display_show_error_screen(KF_SYSTEM_SCREEN_CD_SEARCH_FAILED);
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
        CdRead(cd_search_file.size >> KF_CD_SECTOR_SHIFT, destination, CdlModeSpeed);
        while ((result = CdReadSync(KF_CD_READ_POLL, NULL)) > 0) {
        }
        if (result == 0) {
            attempt = KF_CD_READ_STOP_ATTEMPT;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        display_show_error_screen(KF_SYSTEM_SCREEN_CD_READ_FAILED);
    }
    return KF_RESOURCE_LOADED;
}
