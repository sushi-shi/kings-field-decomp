#include <kf/address.h>
#include <kf/game_types.h>

/*
 * King's Field custom SEQ track parser (built over Psy-Q libsnd's SEQREAD
 * engine).  The openers in audio_sequence.c allocate a slot out of the shared
 * bitmask and hand the sequence buffer to one of these routines:
 *
 *   audio_sequence_load_track  - a single-track sequence.  Validates the
 *       "SEQp" header/version (printing FromSoftware's own diagnostics), then
 *       initialises track 0 of DAT_800a06e0[slot].
 *   audio_sequence_parse_track - one track of a multi-track set.  Initialises
 *       DAT_800a06e0[slot][index] and returns the number of bytes this track
 *       occupies (fixed header fields + the track's declared data length) so
 *       the multi-track opener can advance to the next track.
 *
 * Both seed the 0xAC-byte score record, read the resolution/tempo header from
 * the buffer, convert the MIDI microseconds-per-quarter tempo into beats per
 * minute (60000000 / tempo, rounded), and derive the engine tick counts at
 * 0x90/0x92 from the resolution, the BPM and the engine rate DAT_8009ff00.
 * Only ReadDeltaValue (the vendored SEQREAD variable-length delta reader) is
 * called into the engine; every field write lands in the libsnd _ss_score
 * record the game shares with the engine.
 *
 * The record pointer is reloaded from DAT_800a06e0 before every statement
 * because a store through the record could alias the pointer table; the source
 * is old-style (K&R) so the multi-track opener can pass its int loop counter
 * as the `index` argument without a narrowing cast at the call site while the
 * body still treats it as a signed 16-bit track index.
 *
 * audio_sequence_load_track is exact.  audio_sequence_parse_track is
 * structurally exact (same field writes, call set, constants and control
 * flow) but retains a gcc-2.5.7 register-allocation residue: retail keeps the
 * per-track `index * 0xac` byte offset in $a1 across the header reads while
 * cc1psx-257 assigns it to $a0, and that choice re-colours the interleaved
 * cursor loads through the resolution/tempo section.  The 1D loader has no
 * such offset, which is why it reaches exact under the same source shape.
 */
typedef struct SeqTrack {
    u32 flags;             /* 0x00 dispatch flags */
    u8 reserved_04[0x26];  /* 0x04 */
    s16 field_2a;          /* 0x2a cached left volume (0x7f) */
    s16 field_2c;          /* 0x2c cached right volume (0x7f) */
    u8 reserved_2e[0x02];  /* 0x2e */
    u8 *field_30;          /* 0x30 read cursor into the sequence buffer */
    u8 *field_34;          /* 0x34 saved cursor (event base) */
    u8 *field_38;          /* 0x38 saved cursor (loop base) */
    u8 field_3c;           /* 0x3c */
    u8 reserved_3d;        /* 0x3d */
    u8 field_3e;           /* 0x3e */
    u8 field_3f;           /* 0x3f */
    u8 field_40;           /* 0x40 */
    u8 field_41;           /* 0x41 */
    u8 field_42;           /* 0x42 */
    u8 field_43[0x10];     /* 0x43 per-voice byte (default 0x40) */
    u8 field_53;           /* 0x53 */
    u8 field_54;           /* 0x54 */
    u8 field_55;           /* 0x55 */
    u8 field_56;           /* 0x56 */
    u8 field_57;           /* 0x57 key-state byte */
    u8 field_58[0x10];     /* 0x58 per-voice index (0..15) */
    u16 field_68;          /* 0x68 */
    s16 field_6a;          /* 0x6a */
    s16 field_6c;          /* 0x6c resolution (ticks per quarter) */
    s16 field_6e;          /* 0x6e vab id */
    s16 field_70[0x10];    /* 0x70 per-voice halfword (default 0x7f) */
    s16 field_90;          /* 0x90 tick count / -1 */
    s16 field_92;          /* 0x92 tick count */
    s16 field_94;          /* 0x94 tick count copy */
    u8 reserved_96[0x02];  /* 0x96 */
    u32 field_98;          /* 0x98 initial delta */
    u32 field_9c;          /* 0x9c */
    s32 field_a0;          /* 0xa0 tempo -> beats per minute */
    u32 field_a4;          /* 0xa4 initial delta copy */
    u32 field_a8;          /* 0xa8 beats-per-minute copy */
} SeqTrack;

/* Per-sequence libsnd _ss_score record table (BSS; owned elsewhere). */
extern SeqTrack *DAT_800a06e0[];

/* Engine tick rate used to scale tempo into per-tick counts (BSS). */
extern u32 DAT_8009ff00;

/* Vendored Psy-Q libsnd SEQREAD variable-length delta reader. */
extern s32 ReadDeltaValue(s16 seq, s16 track);
extern int printf();

#define TRK  DAT_800a06e0[slot]
#define TRKI DAT_800a06e0[slot][index]

/* The two SEQ-validation diagnostics this unit prints (retail rodata pool). */
RODATA(0x80013bc0, 0x38)

ADDRESS(0x80046a94, 0x710)
s32 audio_sequence_parse_track(slot, index, vab_id, data)
short slot, index, vab_id;
unsigned char *data;
{
    int i;
    int tempo;
    int consumed;
    int size;
    int b0, b1, b2, b3, b4;

    consumed = 0;

    TRKI.field_6c = 0;
    TRKI.field_3f = 0;
    TRKI.field_40 = 0;
    TRKI.field_55 = 0;
    TRKI.field_41 = 0;
    TRKI.field_42 = 0;
    TRKI.field_56 = 0;
    TRKI.field_3e = 0;
    TRKI.field_98 = 0;
    TRKI.field_9c = 0;
    TRKI.field_a0 = 0;
    TRKI.field_94 = 0;
    TRKI.field_6a = 0;
    TRKI.field_57 = 0;
    TRKI.field_a4 = 0;
    TRKI.field_53 = 0;
    TRKI.field_54 = 0;
    TRKI.field_3c = 0;
    TRKI.field_6e = vab_id;

    for (i = 0; i < 16; i++) {
        TRKI.field_58[i] = i;
        TRKI.field_43[i] = 0x40;
        TRKI.field_70[i] = 0x7f;
    }

    TRKI.field_90 = 1;
    TRKI.field_2a = 0x7f;
    TRKI.field_2c = 0x7f;
    TRKI.field_30 = data;
    if (index == 0) {
        TRKI.field_30 += 8;
        consumed += 8;
    } else {
        TRKI.field_30 += 2;
        consumed += 2;
    }

    b0 = *TRKI.field_30++;
    b1 = *TRKI.field_30++;
    TRKI.field_6c = b1 | (b0 << 8);
    b2 = *TRKI.field_30++;
    b3 = *TRKI.field_30++;
    b4 = *TRKI.field_30++;
    TRKI.field_a0 = (b2 << 16) | (b3 << 8) | b4;
    consumed += 5;

    tempo = TRKI.field_a0;
    if (tempo / 2 < 60000000 % tempo)
        TRKI.field_a0 = 60000000 / tempo + 1;
    else
        TRKI.field_a0 = 60000000 / tempo;
    TRKI.field_a8 = TRKI.field_a0;

    TRKI.field_30 += 2;
    b0 = *TRKI.field_30++;
    b1 = *TRKI.field_30++;
    b2 = *TRKI.field_30++;
    b3 = *TRKI.field_30++;
    size = (b0 << 24) + (b1 << 16) + (b2 << 8) + b3;
    TRKI.field_98 = ReadDeltaValue(slot, index);
    consumed += 6;

    TRKI.field_a4 = TRKI.field_98;
    TRKI.field_34 = TRKI.field_30;
    TRKI.field_38 = TRKI.field_30;

    if (TRKI.field_6c * TRKI.field_a0 * 10 < DAT_8009ff00 * 60) {
        TRKI.field_90 = DAT_8009ff00 * 600 / (TRKI.field_6c * TRKI.field_a0);
        TRKI.field_92 = TRKI.field_90;
    } else {
        TRKI.field_90 = -1;
        TRKI.field_92 = TRKI.field_6c * TRKI.field_a0 * 10 / (DAT_8009ff00 * 60);
        if (DAT_8009ff00 * 30 < TRKI.field_6c * TRKI.field_a0 * 10 % (DAT_8009ff00 * 60))
            TRKI.field_92 += 1;
    }
    TRKI.field_94 = TRKI.field_92;

    return consumed + size;
}

ADDRESS(0x800471a4, 0x544)
void audio_sequence_load_track(slot, vab_id, sequence)
short slot, vab_id;
unsigned char *sequence;
{
    int i;
    int tempo;
    int first;
    int b0, b1, b2, b3, b4;

    TRK->field_6c = 0;
    TRK->field_3f = 0;
    TRK->field_40 = 0;
    TRK->field_55 = 0;
    TRK->field_41 = 0;
    TRK->field_42 = 0;
    TRK->field_56 = 0;
    TRK->field_3e = 0;
    TRK->field_98 = 0;
    TRK->field_9c = 0;
    TRK->field_a0 = 0;
    TRK->field_94 = 0;
    TRK->field_6a = 0;
    TRK->field_57 = 0;
    TRK->field_a4 = 0;
    TRK->field_53 = 0;
    TRK->field_54 = 0;
    TRK->field_3c = 0;
    TRK->field_6e = vab_id;

    for (i = 0; i < 16; i++) {
        TRK->field_58[i] = i;
        TRK->field_43[i] = 0x40;
        TRK->field_70[i] = 0x7f;
    }

    TRK->field_90 = 1;
    TRK->field_2a = 0x7f;
    TRK->field_2c = 0x7f;
    TRK->field_30 = sequence;

    first = *TRK->field_30;
    if (first == 0x53 || first == 0x70) {
        TRK->field_30 += 7;
        if (*TRK->field_30++ != 1) {
            printf("This is not SEQ Data.\n");
            return;
        }
    } else {
        printf("This is an old SEQ Data Format.\n");
    }

    b0 = *TRK->field_30++;
    b1 = *TRK->field_30++;
    TRK->field_6c = b1 | (b0 << 8);
    b2 = *TRK->field_30++;
    b3 = *TRK->field_30++;
    b4 = *TRK->field_30++;
    TRK->field_a0 = (b2 << 16) | (b3 << 8) | b4;

    tempo = TRK->field_a0;
    if (tempo / 2 < 60000000 % tempo)
        TRK->field_a0 = 60000000 / tempo + 1;
    else
        TRK->field_a0 = 60000000 / tempo;
    TRK->field_a8 = TRK->field_a0;

    TRK->field_30 += 2;
    TRK->field_98 = ReadDeltaValue(slot, 0);
    TRK->field_a4 = TRK->field_98;
    TRK->field_34 = TRK->field_30;
    TRK->field_38 = TRK->field_30;

    if (TRK->field_6c * TRK->field_a0 * 10 < DAT_8009ff00 * 60) {
        TRK->field_90 = DAT_8009ff00 * 600 / (TRK->field_6c * TRK->field_a0);
        TRK->field_92 = TRK->field_90;
    } else {
        TRK->field_90 = -1;
        TRK->field_92 = TRK->field_6c * TRK->field_a0 * 10 / (DAT_8009ff00 * 60);
        if (DAT_8009ff00 * 30 < TRK->field_6c * TRK->field_a0 * 10 % (DAT_8009ff00 * 60))
            TRK->field_92 += 1;
    }
    TRK->field_94 = TRK->field_92;
}
