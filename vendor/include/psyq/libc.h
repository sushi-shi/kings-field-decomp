#ifndef KF_PSYQ_LIBC_H
#define KF_PSYQ_LIBC_H

#include <MEMORY.H>
#include <MALLOC.H>
#include <RAND.H>

extern int rand(void);
extern int printf(const char *format, ...);
extern void exit(int status);
extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);

#endif
