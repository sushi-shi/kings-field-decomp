#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern s32 cd_read(s32 mode, u32 loc, void *buffer);
extern void getpos(u32 loc, void *out);
extern int printf(const char *fmt, ...);
extern void *memcpy(void *dst, const void *src, u32 n);

/* Cached directory index, its file count, and the verbose-log flag. */
extern s32 DAT_80057ee8;
extern s32 DAT_80057ee0;
extern s32 DAT_80057ba4;

/* Directory location table (0x2c-byte records), CD sector buffer, and the
 * parsed file-record table it fills (0x14-byte records). */
extern u8 DAT_80059bf4[];
extern u8 DAT_8005a4d8[];
extern u8 DAT_800598f8[];

/* Verbose-log format strings owned by this unit. */
RODATA(0x8001318c, 0x78)

/*
 * Cache one CD directory: when the requested directory differs from the cached
 * one, read its sector, walk the ISO directory records (each record's byte 0 is
 * its length; 0 ends the run), and for every entry convert the extent LBA to an
 * MSF position via getpos, store the size, and copy the file name into the
 * parsed table.  Returns 0xffffffff when the sector read fails, otherwise 1.
 */
ADDRESS(0x8003ce2c, 0x224)
u32 func_8003ce2c(s32 dir_index)
{
    u8 *src;
    u8 *dst;
    s32 count;
    u32 lba;
    u32 size;
    u8 name_len;

    if (dir_index != DAT_80057ee8) {
        if (cd_read(1, *(u32 *)(DAT_80059bf4 + dir_index * 0x2c), DAT_8005a4d8) != 1) {
            if (DAT_80057ba4 == 0) {
                return 0xffffffff;
            }
            printf("CD_cachefile: dir not found\n");
            return 0xffffffff;
        }

        count = 0;
        if (DAT_80057ba4 != 0) {
            printf("CD_cachefile: searching...\n");
        }
        src = DAT_8005a4d8;
        dst = DAT_800598f8;
        while (*src != 0) {
            lba = *(u32 *)(src + 2);
            size = *(u32 *)(src + 0xa);
            *(u32 *)(dst + 4) = size;
            getpos(lba, dst);
            name_len = src[0x20];
            memcpy(dst + 8, src + 0x21, name_len);
            dst[name_len + 8] = 0;
            count++;
            if (DAT_80057ba4 != 0) {
                printf("\t(%02x:%02x:%02x) %8d %s\n", dst[0], dst[1], dst[2],
                       *(u32 *)(dst + 4), dst + 8);
            }
            dst += 0x14;
            src += *src;
            if (src > DAT_8005a4d8 + 0x7ff || dst > DAT_800598f8 + 0x31f) {
                break;
            }
        }

        if (DAT_80057ba4 == 0) {
            DAT_80057ee0 = count;
            DAT_80057ee8 = dir_index;
            return 1;
        }
        DAT_80057ee0 = count;
        DAT_80057ee8 = dir_index;
        printf("CD_cachefile: %d files found\n", count);
    }
    return 1;
}
