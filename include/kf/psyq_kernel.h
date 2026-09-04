#ifndef KF_PSYQ_KERNEL_H
#define KF_PSYQ_KERNEL_H

/*
 * Psy-Q Release 2.5 kernel, controller, and BIOS file interfaces. KERNEL.H
 * and SYS/FILE.H provide the vendor constants and layouts, but this release
 * ships no LIBAPI header with callable declarations. Keep the missing BIOS
 * entry points here so game sources do not reproduce private extern blocks.
 */

#include <KERNEL.H>
#include <SYS/FILE.H>

/* KERNEL.H hides its C layouts unless the original Psy-Q driver defines
 * LANGUAGE_C; the pinned native preprocessor does not. */
#ifndef LANGUAGE_C
struct EXEC {
    unsigned long pc0;
    unsigned long gp0;
    unsigned long t_addr;
    unsigned long t_size;
    unsigned long d_addr;
    unsigned long d_size;
    unsigned long b_addr;
    unsigned long b_size;
    unsigned long s_addr;
    unsigned long s_size;
    unsigned long sp;
    unsigned long fp;
    unsigned long gp;
    unsigned long ret;
    unsigned long base;
};
#endif

extern long OpenEvent(
    unsigned long descriptor, long spec, long mode, void (*handler)(void));
extern long EnableEvent(long event);
extern long TestEvent(long event);
extern long UnDeliverEvent(long event);
extern long CloseEvent(long event);

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void ResetCallback(void);

extern void InitHeap(void *head, long size);
extern void InitCARD2(long pad_enable);
extern void StartCARD2(void);
extern void StopCARD2(void);
extern void _bu_init(void);
extern void _card_auto(long enable);
extern long _card_info(long channel);
extern void _new_card(void);

extern unsigned long PAD_init2(unsigned long mode, unsigned long *pad_state);
extern void PAD_dr(void);
extern void StopPAD2(void);

extern long open(const char *name, long mode);
extern long close(long file);
extern long lseek(long file, long offset, long origin);
extern long read(long file, void *buffer, long length);
extern long write(long file, const void *buffer, long length);
extern long erase(const char *name);
extern long format(const char *device);

extern long _96_init(void);
extern long _96_remove(void);
extern long Load(const char *name, struct EXEC *header);
extern long Exec(struct EXEC *header, long argc, char **argv);

#endif
