#ifndef KF_SEQTRACK_H
#define KF_SEQTRACK_H

#include <kf/game_types.h>

/*
 * King's Field custom music sequence-track driver (built over Psy-Q libsnd's
 * VMANAGER voice helpers).  DAT_800a06e0[sequence] points at that open
 * sequence's array of 0xAC-byte per-track records; DAT_800a0770 counts the
 * open sequences, DAT_800a0778 the tracks per sequence, and DAT_8009a728 is
 * the active-sequence bitmask.  Only the fields the driver touches are named;
 * the rest of the record stays opaque.  Widths follow the retail load/store
 * forms: flags is a 32-bit bitfield, the fade counters at 0x10/0x1c are
 * unsigned (retail divides them with divu), and the periods at 0x14/0x24 are
 * signed halfwords.
 */
typedef struct SeqTrack {
    u32 flags;             /* 0x00 dispatch flags: 0x1/0x2/0x4/0x8/0x10/0x20/0x40/0x80/0x100/0x200 */
    u8 field_04;           /* 0x04 */
    u8 reserved_05;        /* 0x05 */
    s16 field_06;          /* 0x06 fade direction/amount */
    s16 field_08;          /* 0x08 fade step accumulator */
    u8 reserved_0a[0x02];  /* 0x0a */
    s32 field_0c;          /* 0x0c */
    u32 field_10;          /* 0x10 fade countdown */
    s16 field_14;          /* 0x14 fade period */
    u8 reserved_16[0x02];  /* 0x16 */
    s32 field_18;          /* 0x18 */
    u32 field_1c;          /* 0x1c slide countdown */
    u32 field_20;          /* 0x20 slide target */
    s16 field_24;          /* 0x24 slide period */
    u16 volume_left;       /* 0x26 -> SpuVmSetSeqVol arg1 */
    u16 volume_right;      /* 0x28 -> SpuVmSetSeqVol arg2 */
    u16 field_2a;          /* 0x2a <- SpuVmGetSeqVol out (cached left) */
    u16 field_2c;          /* 0x2c <- SpuVmGetSeqVol out (cached right) */
    u8 reserved_2e[0x02];  /* 0x2e */
    u32 field_30;          /* 0x30 */
    u32 field_34;          /* 0x34 */
    u32 field_38;          /* 0x38 */
    u8 reserved_3c[0x17];  /* 0x3c */
    u8 field_53;           /* 0x53 */
    u8 reserved_54[0x03];  /* 0x54 */
    u8 field_57;           /* 0x57 key-state byte */
    u8 reserved_58[0x10];  /* 0x58 */
    u16 field_68;          /* 0x68 */
    u16 field_6a;          /* 0x6a */
    s16 field_6c;          /* 0x6c slide scale */
    u8 reserved_6e[0x24];  /* 0x6e */
    s16 field_92;          /* 0x92 slide increment */
    u16 field_94;          /* 0x94 */
    u8 reserved_96[0x02];  /* 0x96 */
    u32 field_98;          /* 0x98 */
    u32 field_9c;          /* 0x9c */
    u32 field_a0;          /* 0xa0 */
    u32 field_a4;          /* 0xa4 */
    u32 field_a8;          /* 0xa8 slide accumulator */
} SeqTrack;

/* Per-sequence track-record table and driver-wide sequence census (all BSS). */
extern SeqTrack *DAT_800a06e0[];
extern s16 DAT_800a0770; /* open-sequence count */
extern s16 DAT_800a0778; /* tracks per sequence */
extern s32 DAT_8009a728; /* active-sequence bitmask */

/*
 * Psy-Q Release 2.5 LIBSND VMANAGER helpers (vendored; see
 * functions_vendored.tsv).  SpuVmSeqKeyOff reads only the sequence argument;
 * the driver declares it K&R so the sites that also pass a track index match.
 */
extern s32 SpuVmSetSeqVol(s16 sequence, s16 volume_left, s16 volume_right, s16 mode);
extern s32 SpuVmGetSeqVol(s16 sequence, u16 *volume_left, u16 *volume_right);
extern void SpuVmSeqKeyOff();

#endif /* KF_SEQTRACK_H */
