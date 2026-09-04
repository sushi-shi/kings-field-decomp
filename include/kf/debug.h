#ifndef KF_DEBUG_H
#define KF_DEBUG_H

/* No-op diagnostic sink shared by GAME.EXE and OPEN.EXE debug callers. */
extern void debug_printf_sink(const char *format, ...);

#endif
