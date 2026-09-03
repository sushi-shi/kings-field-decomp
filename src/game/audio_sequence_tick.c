#include <kf/address.h>
#include <kf/psyq_audio.h>
#include <kf/seqtrack.h>
#include <kf/game.h>

/*
 * Per-tick sequence driver.  This follows the LIBSND SSCALL topology, but the
 * 0x2f8-byte retail dispatcher does not match the 0x244-byte Release 2.5
 * object, so its provider remains unresolved.  It wraps the update pass in a
 * setjmp/longjmp abort envelope and dispatches per-track control routines from
 * the flag bits.  The row pointer is reloaded on each field access because a
 * store through a track record could alias the pointer table.
 *
 * The abort blocks carry an unattributed residue: retail materializes the
 * handler-slot address (0x8005b274) once and derives the jmp_buf (0x8005b270 =
 * slot - 4) from it, but the curated data model lists the two as separate BSS
 * globals, so this reconstruction materializes each independently.  The
 * dispatch walk is byte-exact.
 */

/* setjmp/longjmp abort envelope and handler-swap state (all BSS/load). */
extern s16 DAT_800a0770;   /* open-sequence count */

/* The vendored libsnd _memcpy at 0x8004a52c; address-suffixed because GAME.EXE
 * links four distinct memcpy bodies. */
extern int setjmp();
extern void longjmp();
extern void SpuVmFlush();

/* Per-track dispatch targets (own module and siblings). */
void audio_sequence_fade_out(s16 sequence, s16 track);
void func_8004ad9c(s16 sequence, s16 track);

ADDRESS(0x8004a55c, 0x2f8)
void SsSeqCalledTbyT(void)
{
    int sequence;
    int track;

    if (DAT_8009a740 != 0x1000) {
        memcpy_8004a52c((void *)DAT_80058010, DAT_80057d08, 8);
        switch (setjmp(DAT_8005b270)) {
        case 0:
            DAT_80058018 = DAT_8005b274;
            DAT_8005b274 = &DAT_80063278;
            longjmp(DAT_8005b270, 1);
            break;
        case 1:
            break;
        case 2:
            return;
        }
    }

    SpuVmFlush();
    for (sequence = 0; sequence < DAT_800a0770; sequence++) {
        if (!(DAT_8009a728 & (1 << sequence))) {
            continue;
        }
        if (DAT_800a0778 <= 0) {
            continue;
        }
        for (track = 0; track < DAT_800a0778; track++) {
            if (DAT_800a06e0[sequence][track].flags & 0x1) {
                Snd_play(sequence, track);
                if (DAT_800a06e0[sequence][track].flags & 0x10) {
                    audio_sequence_fade_in(sequence, track);
                }
            }
            if (DAT_800a06e0[sequence][track].flags & 0x20) {
                audio_sequence_fade_out(sequence, track);
            }
            if (DAT_800a06e0[sequence][track].flags & 0x40) {
                audio_sequence_pitch_slide(sequence, track);
            }
            if (DAT_800a06e0[sequence][track].flags & 0x80) {
                audio_sequence_pitch_slide(sequence, track);
            }
            if (DAT_800a06e0[sequence][track].flags & 0x2) {
                audio_sequence_track_key_off(sequence, track);
            }
            if (DAT_800a06e0[sequence][track].flags & 0x8) {
                func_8004ad9c(sequence, track);
            }
            if (DAT_800a06e0[sequence][track].flags & 0x4) {
                func_8004a3c8(sequence, track);
            }
        }
    }

    if (DAT_8009a740 != 0x1000) {
        memcpy_8004a52c(DAT_80057d08, (void *)DAT_80058010, 8);
        DAT_8005b274 = DAT_80058018;
        longjmp(DAT_8005b270, 2);
    }
}

ADDRESS(0x8004a854, 0x548)
void audio_sequence_fade_out(s16 sequence, s16 track)
{
    u16 vl;
    u16 vr;
    int period;

    DAT_800a06e0[sequence][track].field_10--;
    period = DAT_800a06e0[sequence][track].field_14;
    if (period > 0) {
        if (DAT_800a06e0[sequence][track].field_10 % period != 0) {
            goto tail;
        }
        if (DAT_800a06e0[sequence][track].field_06 > 0) {
            DAT_800a06e0[sequence][track].field_08--;
            if (DAT_800a06e0[sequence][track].field_08 < 0) {
                SpuVmSetSeqVol(sequence, 0, 0, 0);
                goto clear_flag;
            }
            SpuVmGetSeqVol(sequence, &vl, &vr);
            if (vl != 0 && vr != 0) {
                SpuVmSetSeqVol(sequence, vl - 1, vr - 1, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0, 0, 0);
            goto clear_flag;
        }
        if (DAT_800a06e0[sequence][track].field_06 < 0) {
            DAT_800a06e0[sequence][track].field_08++;
            if (DAT_800a06e0[sequence][track].field_08 > 0) {
                SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
                goto clear_flag;
            }
            SpuVmGetSeqVol(sequence, &vl, &vr);
            if (vl + 1 < 128 && vr + 1 < 128) {
                SpuVmSetSeqVol(sequence, vl + 1, vr + 1, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
            goto clear_flag;
        }
        goto refresh;
    } else {
        if (DAT_800a06e0[sequence][track].field_06 > 0) {
            DAT_800a06e0[sequence][track].field_08 += period;
            if (DAT_800a06e0[sequence][track].field_08 < 0) {
                SpuVmSetSeqVol(sequence, 0, 0, 0);
                goto clear_flag;
            }
            SpuVmGetSeqVol(sequence, &vl, &vr);
            period = DAT_800a06e0[sequence][track].field_14;
            if ((int)vl >= -period && (int)vr >= -period) {
                SpuVmSetSeqVol(sequence, vl + period, vr + period, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0, 0, 0);
            goto clear_flag;
        }
        if (DAT_800a06e0[sequence][track].field_06 < 0) {
            DAT_800a06e0[sequence][track].field_08 -= period;
            if (DAT_800a06e0[sequence][track].field_08 > 0) {
                SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
                goto clear_flag;
            }
            SpuVmGetSeqVol(sequence, &vl, &vr);
            period = DAT_800a06e0[sequence][track].field_14;
            if ((int)vl - period < 128 && (int)vr - period < 128) {
                SpuVmSetSeqVol(sequence, vl - period, vr - period, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
            goto clear_flag;
        }
        goto refresh;
    }

clear_flag:
    DAT_800a06e0[sequence][track].flags &= ~0x20u;
refresh:
    if (DAT_800a06e0[sequence][track].field_10 != 0 &&
        DAT_800a06e0[sequence][track].field_08 != 0) {
        goto tail;
    }
    DAT_800a06e0[sequence][track].flags &= ~0x20u;
tail:
    SpuVmGetSeqVol(sequence, &DAT_800a06e0[sequence][track].field_2a,
                   &DAT_800a06e0[sequence][track].field_2c);
}

ADDRESS(0x8004ad9c, 0x68)
void func_8004ad9c(s16 sequence, s16 track)
{
    DAT_800a06e0[sequence][track].field_57 = 1;
    DAT_800a06e0[sequence][track].flags &= ~0x8u;
}
