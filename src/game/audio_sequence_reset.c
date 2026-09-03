#include <kf/address.h>
#include <kf/seqtrack.h>

/*
 * Sequence teardown: mute the sequence, key its voices off, drop it from the
 * active-sequence mask, and reinitialize every track record to its idle state
 * (full 0x7f/0x7f volume, cleared fade/slide counters).  func_8004b6e0 and
 * func_8004b848 are byte-identical bodies; the driver keeps two entry points.
 * The track base is reloaded from the global on every field store because a
 * store through the record could alias the pointer table.
 */

ADDRESS(0x8004b6e0, 0x168)
void func_8004b6e0(s16 sequence)
{
    int track;

    SpuVmSetSeqVol(sequence, 0, 0, 1);
    SpuVmSeqKeyOff(sequence);
    DAT_8009a728 &= ~(1 << sequence);
    for (track = 0; track < DAT_800a0778; track++) {
        DAT_800a06e0[sequence][track].flags = 0;
        DAT_800a06e0[sequence][track].field_04 = 0xff;
        DAT_800a06e0[sequence][track].field_06 = 0;
        DAT_800a06e0[sequence][track].field_08 = 0;
        DAT_800a06e0[sequence][track].field_0c = 0;
        DAT_800a06e0[sequence][track].field_10 = 0;
        DAT_800a06e0[sequence][track].field_14 = 0;
        DAT_800a06e0[sequence][track].field_20 = 0;
        DAT_800a06e0[sequence][track].field_1c = 0;
        DAT_800a06e0[sequence][track].field_18 = 0;
        DAT_800a06e0[sequence][track].field_24 = 0;
        DAT_800a06e0[sequence][track].volume_left = 0x7f;
        DAT_800a06e0[sequence][track].volume_right = 0x7f;
    }
}

ADDRESS(0x8004b848, 0x168)
void func_8004b848(s16 sequence)
{
    int track;

    SpuVmSetSeqVol(sequence, 0, 0, 1);
    SpuVmSeqKeyOff(sequence);
    DAT_8009a728 &= ~(1 << sequence);
    for (track = 0; track < DAT_800a0778; track++) {
        DAT_800a06e0[sequence][track].flags = 0;
        DAT_800a06e0[sequence][track].field_04 = 0xff;
        DAT_800a06e0[sequence][track].field_06 = 0;
        DAT_800a06e0[sequence][track].field_08 = 0;
        DAT_800a06e0[sequence][track].field_0c = 0;
        DAT_800a06e0[sequence][track].field_10 = 0;
        DAT_800a06e0[sequence][track].field_14 = 0;
        DAT_800a06e0[sequence][track].field_20 = 0;
        DAT_800a06e0[sequence][track].field_1c = 0;
        DAT_800a06e0[sequence][track].field_18 = 0;
        DAT_800a06e0[sequence][track].field_24 = 0;
        DAT_800a06e0[sequence][track].volume_left = 0x7f;
        DAT_800a06e0[sequence][track].volume_right = 0x7f;
    }
}
