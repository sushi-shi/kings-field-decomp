extern "C" {
#ifndef KF_PSYQ_LIBC_H
#define KF_PSYQ_LIBC_H

#define memcpy memcpy_unprototyped
#define memset memset_unprototyped
#define malloc malloc_unprototyped
#define free free_unprototyped
#include <MEMORY.H>
#include <MALLOC.H>
#undef memcpy
#undef memset
#undef malloc
#undef free
#include <RAND.H>

extern int rand(void);
extern int printf(const char *format, ...);
extern void exit(int status);
extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);

extern void *memcpy(void *destination, const void *source, unsigned long size);
extern void *memset(void *destination, int value, unsigned long size);
extern void *malloc(unsigned long size);
extern void free(void *allocation);

#endif

}
