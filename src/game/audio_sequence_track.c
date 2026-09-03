#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * King's Field custom sequence player: per-track voice control.
 *
 * DAT_800a06e0[sequence] points at that open sequence's array of 0xAC-byte
 * track records (see audio_sequence.c for the openers that allocate them).
 * These routines toggle the per-track control flags at offset 0 and drive the
 * shared voice-parameter helper SpuVmSetSeqVol (left/right volume, capped at
 * 0x7f) and the LIBSND VMANAGER key-off primitive SpuVmSeqKeyOff.
 *
 * The row pointer is reloaded from the global before every statement because a
 * store through the record could alias the pointer table; the compiler still
 * shares the record address within a single read-modify-write statement.  Only
 * the fields these routines touch are named; the rest stay opaque.  flags bits
 * seen here: 0x1/0x2/0x4/0x8/0x100/0x200.
 *
 * func_8004a344 (1D, no track index) is exact.  The four 2D routines carry a
 * load-placement residue on the 0xAC (172-byte) record index: retail schedules
 * the penultimate strength-reduction step (`subu`) of `track * 0xAC` into the
 * load-delay slot of the row-pointer `lw`, which also frees the sign-extended
 * `track` register for reuse as the byte offset; cc1psx-257 (`-mcpu=r2000`)
 * emits that `subu` ahead of the load and fills the delay with the final
 * `sll`, so the row-pointer/offset register pair is swapped throughout.
 * Structurally exact; source form and `-mcpu=r3000` were both ruled out (the
 * latter regresses banked units).  See docs/patterns/source-shapes-gcc257.md.
 */
typedef struct SeqTrack {
    u32 flags;            /* 0x00 */
    u8 reserved_04[0x22]; /* 0x04 */
    u16 volume_left;      /* 0x26 -> SpuVmSetSeqVol arg1 */
    u16 volume_right;     /* 0x28 -> SpuVmSetSeqVol arg2 */
    u8 reserved_2a[0x06]; /* 0x2a */
    u32 field_30;         /* 0x30 */
    u32 field_34;         /* 0x34 */
    u32 field_38;         /* 0x38 */
    u8 reserved_3c[0x17]; /* 0x3c */
    u8 field_53;          /* 0x53 */
    u8 reserved_54[0x03]; /* 0x54 */
    u8 field_57;          /* 0x57 key-state byte */
    u8 reserved_58[0x10]; /* 0x58 */
    u16 field_68;         /* 0x68 */
    u16 field_6a;         /* 0x6a */
    u8 reserved_6c[0x26]; /* 0x6c */
    u16 field_92;         /* 0x92 */
    u16 field_94;         /* 0x94 */
    u8 reserved_96[0x02]; /* 0x96 */
    u32 field_98;         /* 0x98 */
    u32 field_9c;         /* 0x9c */
    u32 field_a0;         /* 0xa0 */
    u32 field_a4;         /* 0xa4 */
    u32 field_a8;         /* 0xa8 */
} SeqTrack;

typedef char SeqTrack_size_is_0xac[(sizeof(SeqTrack) == 0xac) ? 1 : -1];

extern SeqTrack *DAT_800a06e0[];

/* Psy-Q Release 2.5 LIBSND VMANAGER per-sequence volume setter (now vendored). */
extern s32 SpuVmSetSeqVol(s16 sequence, s16 volume_left, s16 volume_right, s16 mode);
/* Psy-Q Release 2.5 LIBSND VMANAGER key-off primitive. */
extern void SpuVmSeqKeyOff(s16 sequence, s16 track);

ADDRESS(0x8004a128, 0xf4)
void func_8004a128(s16 sequence, s16 track, u16 arg2)
{
    DAT_800a06e0[sequence][track].flags &= ~0x200u;
    DAT_800a06e0[sequence][track].flags &= ~0x4u;
    DAT_800a06e0[sequence][track].flags |= 0x1u;
    DAT_800a06e0[sequence][track].field_68 = arg2;
    DAT_800a06e0[sequence][track].field_6a = 0;
    DAT_800a06e0[sequence][track].field_30 = DAT_800a06e0[sequence][track].field_34;
    SpuVmSetSeqVol(sequence, DAT_800a06e0[sequence]->volume_left,
                  DAT_800a06e0[sequence]->volume_right, 0);
}

ADDRESS(0x8004a21c, 0x128)
void func_8004a21c(s16 sequence, s16 track, u8 mode, u16 arg3)
{
    DAT_800a06e0[sequence][track].flags &= ~0x200u;
    DAT_800a06e0[sequence][track].flags &= ~0x4u;
    DAT_800a06e0[sequence][track].field_68 = arg3;
    if (mode == 1) {
        DAT_800a06e0[sequence][track].flags |= 0x1u;
        DAT_800a06e0[sequence][track].field_6a = 0;
        DAT_800a06e0[sequence][track].field_57 = 1;
        SpuVmSetSeqVol(sequence, DAT_800a06e0[sequence][track].volume_left,
                      DAT_800a06e0[sequence][track].volume_right, 0);
    } else if (mode == 0) {
        DAT_800a06e0[sequence][track].flags |= 0x2u;
    }
}

ADDRESS(0x8004a344, 0x30)
void func_8004a344(s16 sequence)
{
    DAT_800a06e0[sequence]->flags |= 0x4u;
}

ADDRESS(0x8004a374, 0x54)
void func_8004a374(s16 sequence, s16 track)
{
    DAT_800a06e0[sequence][track].flags |= 0x4u;
}

ADDRESS(0x8004a3c8, 0x164)
void func_8004a3c8(s16 sequence, s16 track)
{
    SpuVmSeqKeyOff(sequence, track);
    DAT_800a06e0[sequence][track].field_57 = 0;
    DAT_800a06e0[sequence][track].field_9c = 0;
    DAT_800a06e0[sequence][track].field_a4 = DAT_800a06e0[sequence][track].field_98;
    DAT_800a06e0[sequence][track].field_a8 = DAT_800a06e0[sequence][track].field_a0;
    DAT_800a06e0[sequence][track].field_92 = DAT_800a06e0[sequence][track].field_94;
    DAT_800a06e0[sequence][track].field_30 = DAT_800a06e0[sequence][track].field_34;
    DAT_800a06e0[sequence][track].field_38 = DAT_800a06e0[sequence][track].field_34;
    DAT_800a06e0[sequence][track].field_53 = 0;
    DAT_800a06e0[sequence][track].flags &= ~0x1u;
    DAT_800a06e0[sequence][track].flags &= ~0x2u;
    DAT_800a06e0[sequence][track].flags &= ~0x8u;
}
