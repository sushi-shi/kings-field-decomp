#ifndef KF_PSYQ_CD_H
#define KF_PSYQ_CD_H

/* Guarded entry point for the unguarded Psy-Q Release 2.5 LIBCD.H. */

#include <kf/psyq.h>
#include <LIBCD.H>

/* CDREAD.OBJ exports this status poll, omitted by Release 2.5 LIBCD.H. */
extern int CdReadSync(int mode, unsigned char *result);

#endif
