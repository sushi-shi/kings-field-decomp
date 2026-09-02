#include <kf/address.h>
#include <kf/semantic_types.h>
#include <LIBCD.H>
#include <MEMORY.H>

/* Psy-Q Release 2.5 headers declare strcat without a prototype. */
extern char *strcat();
extern void *memory_allocate(s32 size);
/* libcd CdSearchFile body linked without its symbol; see the vendored census. */
extern CdlFILE *func_8003c810(CdlFILE *file, char *path);
/* CD failure handler: 0 after a failed search, 1 after failed reads. */
extern void func_8001b7b0(s32 stage);

extern CdlFILE cd_search_file;
extern char cd_path_buffer[80];
extern KfCdFileEntry cd_file_table[];

DATA(0x80057b3c, 0x5)
char cd_path_prefix[5] = "\\KF\\";
DATA(0x80057b44, 0x3)
char cd_version_suffix[3] = ";1";
DATA(0x80057e80, 0x4)
CdlLOC cd_read_location = {0, 0, 0, 0};

/* Sector size on the CD; sizes are rounded up to whole sectors before reading. */
#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11

/* Loads \KF\<relative_path>;1 into a fresh arena allocation. */
ADDRESS(0x8001acf0, 0x170)
s32 cd_file_load_allocated(void **destination, char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded; /* also the rounded sector count; retail keeps both in $s1 */

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (func_8003c810(&cd_search_file, path) == 0) {
        func_8001b7b0(0);
    }
    if (cd_search_file.size & (CD_SECTOR_SIZE - 1)) {
        loaded = (cd_search_file.size >> CD_SECTOR_SHIFT) + 1;
        cd_search_file.size = loaded << CD_SECTOR_SHIFT;
    }
    *destination = memory_allocate(cd_search_file.size);
    loaded = 0;
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 3; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(cd_search_file.size >> CD_SECTOR_SHIFT, *destination, CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        func_8001b7b0(1);
    }
    return 0;
}

/* Loads the file table entry INDEX into a fresh arena allocation. */
ADDRESS(0x8001ae60, 0x13c)
s32 func_8001ae60(void **destination, s32 index)
{
    s32 attempt;
    s32 loaded;

    *destination = memory_allocate(cd_file_table[index].size);
    loaded = 0;
    cd_read_location.minute = cd_file_table[index].minute;
    cd_read_location.second = cd_file_table[index].second;
    cd_read_location.sector = cd_file_table[index].sector;
    for (attempt = 0; attempt < 5; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(cd_file_table[index].size >> CD_SECTOR_SHIFT, *destination, CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        func_8001b7b0(1);
    }
    return 0;
}

/* Loads \KF\<relative_path>;1 into DESTINATION. */
ADDRESS(0x8001af9c, 0x164)
s32 cd_file_load_into(void *destination, char *relative_path)
{
    char *path = cd_path_buffer;
    s32 attempt;
    s32 loaded; /* also the rounded sector count; retail keeps both in $s1 */

    memcpy(path, cd_path_prefix, sizeof cd_path_prefix);
    strcat(path, relative_path);
    strcat(path, cd_version_suffix);
    if (func_8003c810(&cd_search_file, path) == 0) {
        func_8001b7b0(0);
    }
    if (cd_search_file.size & (CD_SECTOR_SIZE - 1)) {
        loaded = (cd_search_file.size >> CD_SECTOR_SHIFT) + 1;
        cd_search_file.size = loaded << CD_SECTOR_SHIFT;
    }
    loaded = 0;
    cd_read_location.minute = cd_search_file.pos.minute;
    cd_read_location.second = cd_search_file.pos.second;
    cd_read_location.sector = cd_search_file.pos.sector;
    for (attempt = 0; attempt < 3; attempt++) {
        s32 result;

        CdControl(CdlSetloc, (u_char *)&cd_read_location, 0);
        CdRead(cd_search_file.size >> CD_SECTOR_SHIFT, destination, CdlModeSpeed);
        while ((result = CdReadSync(1, 0)) > 0) {
        }
        if (result == 0) {
            attempt = 100;
            loaded = 1;
        }
    }
    if (loaded == 0) {
        func_8001b7b0(1);
    }
    return 0;
}
