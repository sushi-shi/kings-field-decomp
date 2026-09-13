#ifndef KF_CD_FILE_H
#define KF_CD_FILE_H

/* King's Field file loading and state at the Psy-Q CD-ROM boundary. */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <psyq/cd.h>

enum {
    KF_CD_SECTOR_BYTES = 0x800,
    KF_CD_SECTOR_WORDS = 512,
    KF_CD_SECTOR_SHIFT = 11,
    KF_CD_PATH_BYTES = 80,
    KF_CD_READ_POLL = 1,
    KF_CD_READ_STOP_ATTEMPT = 100
};

KF_ENUM_BEGIN(KfResourceLoadResult, s32)
    KF_RESOURCE_LOADED = 0,
    KF_RESOURCE_LOAD_FAILED = 1
KF_ENUM_END(KfResourceLoadResult)

/* Simple, side-effect-free record expressions; retain the request track byte. */
#define CD_LOCATION_COPY(destination, source) do { \
    (destination).minute = (source).minute; \
    (destination).second = (source).second; \
    (destination).sector = (source).sector; \
} while (0)

/* Simple pointer and signed ID expressions; write exactly three characters. */
#define CD_PATH_WRITE_DECIMAL3(digits, number) ( \
    (digits)[0] = (number) / 100 + '0', \
    (digits)[1] = ((number) % 100) / 10 + '0', \
    (digits)[2] = ((number) % 100) % 10 + '0')

extern KfResourceLoadResult cd_file_load_allocated(
    u8 **destination, const char *relative_path);
extern KfResourceLoadResult cd_file_load_into(
    void *destination, const char *relative_path);

#endif
