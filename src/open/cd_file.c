#include <kf/address.h>
#include <kf/cd_file.h>
#include <kf/memory.h>
#include <kf/psyq_libc.h>

DATA(0x800372dc, 0x5)
char cd_path_prefix[5] = "\\KF\\";
DATA(0x800372e4, 0x3)
char cd_version_suffix[3] = ";1";

#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11

/* Loads \KF\<relative_path>;1 into a fresh arena allocation. */
ADDRESS(0x80016014, 0x148)
s32 cd_file_load_allocated(
    void **destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return 1;
    }
    if ((cd_search_file.size & (CD_SECTOR_SIZE - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    *destination = memory_allocate(cd_search_file.size);
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 100; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> CD_SECTOR_SHIFT,
            *destination,
            CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
        }
    }
    return 0;
}

/* Loads \KF\<relative_path>;1 into caller-owned storage. */
ADDRESS(0x8001615c, 0x13c)
s32 cd_file_load_into(
    void *destination, const char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (CdSearchFile(&cd_search_file, path) == 0) {
        return 1;
    }
    if ((cd_search_file.size & (CD_SECTOR_SIZE - 1)) != 0) {
        cd_search_file.size =
            ((cd_search_file.size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 100; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(
            cd_search_file.size >> CD_SECTOR_SHIFT,
            destination,
            CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
        }
    }
    return 0;
}
