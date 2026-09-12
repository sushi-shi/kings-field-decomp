#ifndef KF_STDARG_H
#define KF_STDARG_H

/*
 * Variable-argument access for the pinned GCC 2.5.7 probe. The Release 2.5
 * media supply no STDARG.H, and the GCC 2.5.8 va-mips.h macros add
 * alignment arithmetic that retail lacks. These are the classic Unix
 * varargs macros: va_start addresses the word after the last named
 * argument and va_arg loads through the cursor before advancing it, which
 * matches the reviewed retail readers (home-slot reads
 * in menu_enter_mode and notify_enqueue, the cursor walk in
 * format_vsprintf). The header spelling the original used is unresolved.
 * Modern checking compilers use their own builtins so the ABI stays correct
 * off-target and the lvalue cast never reaches them.
 */
/* The pinned preprocessor runs with -undef, so the branch keys on the
 * feature-test operator modern compilers expose regardless of -undef. */
#if defined(__cplusplus) || defined(__has_builtin)
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)
#else
typedef char *va_list;
#define __va_rounded_size(type) \
    (((sizeof (type) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(ap, last) \
    (ap = ((char *) &(last) + __va_rounded_size (last)))
#define va_arg(ap, type) (*((type *) (ap))++)
#define va_end(ap)
#endif

#endif
