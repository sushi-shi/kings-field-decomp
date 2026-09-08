/* Observational hooks for the Decompals GCC 2.5.7 PSX probe (GPL-2.0-or-later). */
#ifndef KF_TRACE_H
#define KF_TRACE_H
extern const char *kf_trace_pass;
extern const char *kf_trace_context;
extern unsigned long kf_trace_expression;
extern unsigned long kf_trace_sequence;
extern int kf_trace_active (void);
extern void kf_trace_event (const char *, rtx, int, int, const char *,
                            rtx, rtx, int, int, int);
extern void kf_trace_snapshot (const char *, rtx, int);
#define KF_TRACE(kind, insn, pseudo, hard, reason, x, y, a, b, c) \
  do { if (kf_trace_active ()) \
    kf_trace_event (kind, insn, pseudo, hard, reason, x, y, a, b, c); \
  } while (0)
#endif
