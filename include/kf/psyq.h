#ifndef KF_PSYQ_H
#define KF_PSYQ_H

/*
 * Single guarded entry point for the Psy-Q SDK headers, which ship without
 * include guards of their own. Include this instead of the raw SDK headers so
 * a TU pulls each in exactly once and gets the real library prototypes and
 * types rather than hand-rolled externs.
 *
 * LIBSND, LIBCD and LIBSPU are intentionally not pulled in here: they include
 * a lower-case sys header path that does not resolve on a case-sensitive host,
 * so Ss and Cd routines keep explicit declarations. MEMORY.H is left out so
 * memcpy and memset remain GCC builtins.
 */
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>

#endif
