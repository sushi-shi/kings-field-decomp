#ifndef KF_NULL_H
#define KF_NULL_H

/* Psy-Q Release 2.5 STDDEF.H's constant, without its C-only wchar_t typedef.
 * The modern C++ view checks the same spellings against a real null pointer. */
#ifndef NULL
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL 0
#endif
#endif

#endif
