#ifndef KF_PSYQ_H
#define KF_PSYQ_H

#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>

extern void SetPolyGT3(POLY_GT3 *primitive);

extern void ReadSZ2(long *depth, long *unused_depth);

#endif
