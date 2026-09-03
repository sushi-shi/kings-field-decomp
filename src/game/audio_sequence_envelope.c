#include <kf/address.h>
#include <kf/seqtrack.h>

/*
 * Sequence-track envelope/control routines dispatched per tick by
 * SsSeqCalledTbyT from the per-track flag bits.  audio_sequence_fade_in runs the
 * volume fade-in state machine (flag 0x10); audio_sequence_track_key_off keys one track off
 * (flag 0x2); func_8004b3f4 clears the key-state byte and flag 0x100;
 * audio_sequence_pitch_slide advances the pitch/portamento slide (flags 0x40/0x80).  All
 * index _ss_score[sequence][track]; the row pointer is reloaded from the
 * global on each field access because a store through the record could alias
 * the pointer table.
 */

/* Per-open-sequence tick divisor consumed by the slide integrator (BSS). */
extern s32 DAT_8009ff00;

ADDRESS(0x8004ae30, 0x530)
void audio_sequence_fade_in(s16 sequence, s16 track)
{
    u16 vl;
    u16 vr;
    int period;

    _ss_score[sequence][track].field_10--;
    period = _ss_score[sequence][track].field_14;
    if (period > 0) {
        if (_ss_score[sequence][track].field_10 % period != 0) {
            goto tail;
        }
        if (_ss_score[sequence][track].field_06 > 0) {
            _ss_score[sequence][track].field_08--;
            if (_ss_score[sequence][track].field_08 < 0) {
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
        if (_ss_score[sequence][track].field_06 < 0) {
            _ss_score[sequence][track].field_08++;
            if (_ss_score[sequence][track].field_08 > 0) {
                SpuVmSetSeqVol(sequence, 0, 0, 0);
                goto clear_flag;
            }
            SpuVmGetSeqVol(sequence, &vl, &vr);
            if ((int)vl - 1 >= 0 && (int)vr - 1 >= 0) {
                SpuVmSetSeqVol(sequence, vl - 1, vr - 1, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0, 0, 0);
            goto clear_flag;
        }
        goto refresh;
    } else {
        if (_ss_score[sequence][track].field_06 > 0) {
            _ss_score[sequence][track].field_08 += period;
            SpuVmGetSeqVol(sequence, &vl, &vr);
            if (_ss_score[sequence][track].field_08 < 0) {
                SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
                goto clear_flag;
            }
            period = _ss_score[sequence][track].field_14;
            if ((int)vl - period < 128 && (int)vr - period < 128) {
                SpuVmSetSeqVol(sequence, vl - period, vr - period, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0x7f, 0x7f, 0);
            goto clear_flag;
        }
        if (_ss_score[sequence][track].field_06 < 0) {
            _ss_score[sequence][track].field_08 -= period;
            SpuVmGetSeqVol(sequence, &vl, &vr);
            if (_ss_score[sequence][track].field_08 > 0) {
                SpuVmSetSeqVol(sequence, 0, 0, 0);
                goto clear_flag;
            }
            period = _ss_score[sequence][track].field_14;
            if ((int)vl >= -period && (int)vr >= -period) {
                SpuVmSetSeqVol(sequence, vl + period, vr + period, 0);
                goto refresh;
            }
            SpuVmSetSeqVol(sequence, 0, 0, 0);
            goto clear_flag;
        }
        goto refresh;
    }

clear_flag:
    _ss_score[sequence][track].flags &= ~0x10u;
refresh:
    if (_ss_score[sequence][track].field_10 != 0 &&
        _ss_score[sequence][track].field_08 != 0) {
        goto tail;
    }
    _ss_score[sequence][track].flags &= ~0x10u;
tail:
    SpuVmGetSeqVol(sequence, &_ss_score[sequence][track].field_2a,
                   &_ss_score[sequence][track].field_2c);
}

ADDRESS(0x8004b360, 0x94)
void audio_sequence_track_key_off(s16 sequence, s16 track)
{
    SpuVmSeqKeyOff(sequence, track);
    _ss_score[sequence][track].field_57 = 0;
    _ss_score[sequence][track].flags &= ~0x2u;
}

ADDRESS(0x8004b3f4, 0x64)
void func_8004b3f4(s16 sequence, s16 track)
{
    _ss_score[sequence][track].field_57 = 0;
    _ss_score[sequence][track].flags &= ~0x100u;
}

ADDRESS(0x8004b458, 0x24c)
void audio_sequence_pitch_slide(s16 sequence, s16 track)
{
    _ss_score[sequence][track].field_1c--;
    if (_ss_score[sequence][track].field_24 > 0) {
        if (_ss_score[sequence][track].field_1c %
                _ss_score[sequence][track].field_24 != 0) {
            return;
        }
        if (_ss_score[sequence][track].field_a8 >
                _ss_score[sequence][track].field_20) {
            _ss_score[sequence][track].field_a8--;
        } else if (_ss_score[sequence][track].field_a8 <
                _ss_score[sequence][track].field_20) {
            _ss_score[sequence][track].field_a8++;
        }
    } else {
        if (_ss_score[sequence][track].field_a8 >
                _ss_score[sequence][track].field_20) {
            _ss_score[sequence][track].field_a8 +=
                _ss_score[sequence][track].field_24;
            if (_ss_score[sequence][track].field_a8 <
                    _ss_score[sequence][track].field_20) {
                _ss_score[sequence][track].field_a8 =
                    _ss_score[sequence][track].field_20;
            }
        } else if (_ss_score[sequence][track].field_a8 <
                _ss_score[sequence][track].field_20) {
            _ss_score[sequence][track].field_a8 -=
                _ss_score[sequence][track].field_24;
            if (_ss_score[sequence][track].field_a8 >
                    _ss_score[sequence][track].field_20) {
                _ss_score[sequence][track].field_a8 =
                    _ss_score[sequence][track].field_20;
            }
        }
    }

    _ss_score[sequence][track].field_92 =
        _ss_score[sequence][track].field_6c *
        _ss_score[sequence][track].field_a8 * 10 / (DAT_8009ff00 * 60);
    if (_ss_score[sequence][track].field_92 <= 0) {
        _ss_score[sequence][track].field_92 = 1;
    }
    if (_ss_score[sequence][track].field_1c == 0 ||
        _ss_score[sequence][track].field_a8 ==
            _ss_score[sequence][track].field_20) {
        _ss_score[sequence][track].flags &= ~0x40u;
        _ss_score[sequence][track].flags &= ~0x80u;
    }
}
