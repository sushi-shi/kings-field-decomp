#ifndef KF_PSYQ_LIBC_H
#define KF_PSYQ_LIBC_H

/*
 * Psy-Q Release 2.5 C runtime interfaces. MEMORY.H and MALLOC.H preserve the
 * declarations supplied with the pinned SDK. RAND.H supplies RAND_MAX but no
 * callable declaration; the remaining LIBAPI entry points used by the game
 * are also absent from this release's headers.
 */
#include <MEMORY.H>
#include <MALLOC.H>
#include <RAND.H>

extern int rand(void);
extern int printf(const char *format, ...);
extern void exit(int status);
extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);

#endif
