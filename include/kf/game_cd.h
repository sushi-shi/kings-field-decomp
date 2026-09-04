#ifndef KF_GAME_CD_H
#define KF_GAME_CD_H

/* GAME.EXE additions to the shared CD-file interface. */
#include <kf/cd_file.h>

/*
 * Retail indexes these as 20-byte records from an older CdlFILE layout. The
 * supplied Release 2.5 header's CdlFILE has a 16-byte name and is 24 bytes, so
 * callers cast only after selecting a table entry with the proven 20-byte
 * stride.
 */
extern KfCdFileEntry cd_file_table[80];
extern s32 cd_file_load_table_entry(void **destination, s32 index);

#endif
