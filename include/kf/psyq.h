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
 * declarations and case-sensitive include shim. MEMORY.H is left out so
 * memcpy and memset remain GCC builtins.
 */
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>

#endif
