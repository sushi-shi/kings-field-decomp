#ifndef KF_PSYQ_H
#define KF_PSYQ_H

/*
 * Single guarded entry point for the Psy-Q SDK headers, which ship without
 * include guards of their own. Include this instead of the raw <SYS/TYPES.H>
 * / <LIBGTE.H> / <LIBGPU.H> so a translation unit pulls each in exactly once.
 */
#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>

#endif
