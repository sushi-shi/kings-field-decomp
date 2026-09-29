extern "C" {
#ifndef KF_PSYQ_H
#define KF_PSYQ_H

#include <sys/types.h>
#include <LIBGTE.H>
#define AddPrim AddPrim_unprototyped
#define DrawOTag DrawOTag_unprototyped
#define SetSemiTrans SetSemiTrans_unprototyped
#include <LIBGPU.H>
#undef AddPrim
#undef DrawOTag
#undef SetSemiTrans
#include <LIBETC.H>

extern void SetPolyGT3(POLY_GT3 *primitive);

extern void ReadSZ2(long *depth, long *unused_depth);

extern void AddPrim(void *ordering_table_entry, void *primitive);
extern void DrawOTag(u_long *ordering_table);
extern void SetSemiTrans(void *primitive, int enabled);

#endif

}
