#ifndef KF_PSYQ_H
#define KF_PSYQ_H

/*
 * Single guarded entry point for the Psy-Q SDK headers, which ship without
 * include guards of their own. Include this instead of the raw SDK headers so
 * a TU pulls each in exactly once and gets the real library prototypes and
 * types rather than hand-rolled externs.
 *
 * LIBSND and LIBCD stay in kf/psyq_audio.h so their declarations are limited
 * to audio/CD users. KERNEL and the BIOS file interfaces stay in
 * kf/psyq_kernel.h, which also supplies the release's missing LIBAPI
 * declarations and case-sensitive include shim. C runtime declarations stay
 * in kf/psyq_libc.h so only their callers include MEMORY.H and MALLOC.H.
 */
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>

/* Release 2.5's LIBGPU.H spells this SetPolyGT; PRIM.OBJ exports SetPolyGT3. */
extern void SetPolyGT3(POLY_GT3 *primitive);

/* REG.OBJ exports this helper, but Release 2.5's LIBGTE.H omits it.
 * Retail writes only *depth; GAME and OPEN pass an unused second pointer. */
extern void ReadSZ2(long *depth, long *unused_depth);

#endif
