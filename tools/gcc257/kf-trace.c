/* Deterministic, read-only JSONL instrumentation for GCC 2.5.7.
   Copyright (C) 2026. SPDX-License-Identifier: GPL-2.0-or-later

   Do not call RTL allocators, recognizers, or optimizer predicates here.
   The serializer deliberately omits private pointers and linked-list edges. */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtl.h"
#include "tree.h"
#include "regs.h"
#include "kf-trace.h"

const char *kf_trace_pass = "expand";
const char *kf_trace_context;
unsigned long kf_trace_expression;
unsigned long kf_trace_sequence;
static FILE *kf_trace_file;
static int kf_trace_initialized;
static const char *kf_source_hash;
static const char *kf_function_filter;
extern int lineno;

static void
json_string (const char *s)
{
  unsigned char c;
  if (!s) { fputs ("null", kf_trace_file); return; }
  fputc ('"', kf_trace_file);
  while ((c = *s++))
    {
      if (c == '"' || c == '\\') fprintf (kf_trace_file, "\\%c", c);
      else if (c < 32 || c >= 127) fprintf (kf_trace_file, "\\u%04x", c);
      else fputc (c, kf_trace_file);
    }
  fputc ('"', kf_trace_file);
}

static const char *
function_name (void)
{
  if (!current_function_decl || !DECL_ASSEMBLER_NAME (current_function_decl))
    return "<translation-unit>";
  return IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (current_function_decl));
}

static int
sha256_p (const char *s)
{
  int i;
  if (!s || strlen (s) != 64) return 0;
  for (i = 0; i < 64; i++)
    if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f')))
      return 0;
  return 1;
}

int
kf_trace_active (void)
{
  if (!kf_trace_initialized)
    {
      const char *path = getenv ("KF_GCC257_TRACE");
      kf_trace_initialized = 1;
      if (path && *path)
        {
          kf_source_hash = getenv ("KF_GCC257_SOURCE_SHA256");
          kf_function_filter = getenv ("KF_GCC257_TRACE_FUNCTION");
          if (!sha256_p (kf_source_hash))
            { fputs ("GCC trace requires KF_GCC257_SOURCE_SHA256\n", stderr); exit (1); }
          kf_trace_file = fopen (path, "w");
          if (!kf_trace_file) { perror ("GCC trace"); exit (1); }
        }
    }
  return kf_trace_file && (!kf_function_filter || !*kf_function_filter
                          || !strcmp (kf_function_filter, function_name ()));
}

static int
insn_uid (rtx x)
{
  if (!x) return 0;
  switch (GET_CODE (x))
    {
    case INSN: case JUMP_INSN: case CALL_INSN: case CODE_LABEL:
    case NOTE: case BARRIER: return INSN_UID (x);
    default: return 0;
    }
}

static void
json_rtx (rtx x)
{
  const char *format;
  int i, j;
  if (!x) { fputs ("null", kf_trace_file); return; }
  fputs ("{\"code\":", kf_trace_file);
  json_string (GET_RTX_NAME (GET_CODE (x)));
  fputs (",\"mode\":", kf_trace_file);
  json_string (GET_MODE_NAME (GET_MODE (x)));
  fprintf (kf_trace_file, ",\"flags\":[%d,%d,%d,%d]",
           x->in_struct, x->volatil, x->unchanging, x->integrated);
  if (insn_uid (x))
    {
      fprintf (kf_trace_file, ",\"uid\":%d", insn_uid (x));
      if (GET_CODE (x) == INSN || GET_CODE (x) == JUMP_INSN || GET_CODE (x) == CALL_INSN)
        {
          fputs (",\"pattern\":", kf_trace_file); json_rtx (PATTERN (x));
          fputs (",\"notes\":", kf_trace_file); json_rtx (REG_NOTES (x));
        }
      else if (GET_CODE (x) == NOTE)
        fprintf (kf_trace_file, ",\"line\":%d", NOTE_LINE_NUMBER (x));
      fputc ('}', kf_trace_file);
      return;
    }
  fputs (",\"ops\":[", kf_trace_file);
  format = GET_RTX_FORMAT (GET_CODE (x));
  for (i = 0; i < GET_RTX_LENGTH (GET_CODE (x)); i++)
    {
      if (i) fputc (',', kf_trace_file);
      switch (format[i])
        {
        case 'e': json_rtx (XEXP (x, i)); break;
        case 'u': fprintf (kf_trace_file, "%d", insn_uid (XEXP (x, i))); break;
        case 's': case 'S': json_string (XSTR (x, i)); break;
        case 'i': case 'n': fprintf (kf_trace_file, "%d", XINT (x, i)); break;
        case 'w': fprintf (kf_trace_file, "%ld", (long) XWINT (x, i)); break;
        case 'E': case 'V':
          fputc ('[', kf_trace_file);
          if (XVEC (x, i))
            for (j = 0; j < XVECLEN (x, i); j++)
              { if (j) fputc (',', kf_trace_file); json_rtx (XVECEXP (x, i, j)); }
          fputc (']', kf_trace_file);
          break;
        default: fputs ("null", kf_trace_file); break;
        }
    }
  fputs ("]}", kf_trace_file);
}

void
kf_trace_event (const char *kind, rtx insn, int pseudo, int hard,
                const char *reason, rtx x, rtx y, int a, int b, int c)
{
  if (!kf_trace_active ()) return;
  fprintf (kf_trace_file, "{\"schema\":1,\"seq\":%lu,\"function\":", ++kf_trace_sequence);
  json_string (function_name ());
  fputs (",\"pass\":", kf_trace_file); json_string (kf_trace_pass);
  fputs (",\"event\":", kf_trace_file); json_string (kind);
  fputs (",\"context\":", kf_trace_file); json_string (kf_trace_context);
  fputs (",\"source_sha256\":", kf_trace_file); json_string (kf_source_hash);
  fputs (",\"compiler_source_sha256\":", kf_trace_file); json_string (KF_COMPILER_SOURCE_SHA256);
  fprintf (kf_trace_file, ",\"uid\":%d,\"pseudo\":%d,\"hard_reg\":%d,"
           "\"expression\":%lu,\"parser_line\":%d,\"reason\":",
           insn_uid (insn), pseudo, hard, kf_trace_expression, lineno);
  json_string (reason);
  fprintf (kf_trace_file, ",\"values\":[%d,%d,%d],\"x\":", a, b, c);
  json_rtx (x);
  fputs (",\"y\":", kf_trace_file); json_rtx (y);
  fputs ("}\n", kf_trace_file);
  if (ferror (kf_trace_file)) { perror ("GCC trace write"); exit (1); }
}

void
kf_trace_snapshot (const char *stage, rtx insns, int allocation)
{
  rtx x;
  int i;
  if (!kf_trace_active ()) return;
  for (x = insns; x; x = NEXT_INSN (x))
    kf_trace_event ("rtl.snapshot", x, -1, -1, stage, x, 0, 0, 0, 0);
  if (allocation)
    for (i = FIRST_PSEUDO_REGISTER; i < max_reg_num (); i++)
      kf_trace_event ("allocation.pseudo", 0, i, reg_renumber[i], stage,
                      regno_reg_rtx[i], 0, reg_n_refs[i],
                      reg_n_calls_crossed[i], reg_live_length[i]);
  kf_trace_event ("stage.end", 0, -1, -1, stage, 0, 0, 0, 0, 0);
  if (fflush (kf_trace_file)) { perror ("GCC trace flush"); exit (1); }
}
