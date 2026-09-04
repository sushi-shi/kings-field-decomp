#ifndef KF_GAME_CD_H
#define KF_GAME_CD_H

/* Shared game-owned state at the Psy-Q CD-ROM boundary. */

#include <kf/semantic_types.h>
#include <kf/psyq_cd.h>

extern CdlFILE cd_search_file;
extern CdlLOC cd_read_location;

/*
 * Retail indexes these as 20-byte records from an older CdlFILE layout. The
 * supplied Release 2.5 header's CdlFILE has a 16-byte name and is 24 bytes, so
 * callers cast only after selecting a table entry with the proven 20-byte
 * stride.
 */
extern KfCdFileEntry cd_file_table[80];

#endif
