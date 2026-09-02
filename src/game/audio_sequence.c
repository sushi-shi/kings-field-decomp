#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * King's Field custom sequence-open family (SEQREAD.C-shaped MIDI player).
 * Two openers allocate a free slot out of the shared open-sequence bitmask and
 * hand the sequence data to the per-track parser.  Both are structurally exact
 * under the gcc257 probe but retain a load-placement residue: retail hoists the
 * opening `DAT_8009a728` load above the frame allocation so the frame fills the
 * load-delay slot, and fills the mask-reload delay with the argument shift,
 * while cc1psx-257 emits `.set noreorder; lw; nop` for the compare-feeding
 * loads (see docs/patterns/source-shapes-gcc257.md).
 */

/* Shared diagnostic string of this opener family in the retail rodata pool;
 * both openers print it when no free sequence slot remains. */
RODATA(0x80013b9c, 0x24)

/* Bitmask of open sequence slots (bit N set == slot N in use); a negative value
 * means the top slot is taken and no further sequence can open. */
extern u32 DAT_8009a728;

extern int printf();

/* Per-track sequence parsers: func_800471a4 loads a single sequence into the
 * slot; func_80046a94 parses track `index` of a multi-track set and returns the
 * number of bytes it consumed from the sequence buffer. */
extern void func_800471a4(s16 slot, s16 vab_id, u8 *sequence);
extern s32 func_80046a94(s16 slot, s32 index, s16 vab_id, u8 *data);

ADDRESS(0x800468d8, 0xb0)
s16 func_800468d8(u8 *sequence, s16 vab_id)
{
    s16 slot;
    s32 index;
    u8 found;

    if ((s32)DAT_8009a728 < 0) {
        printf("Can't Open Sequence data any more\n\n");
        return -1;
    }
    for (index = 0, found = 0; !found; index++) {
        if (((1 << index) & DAT_8009a728) == 0) {
            slot = index;
            found = 1;
        }
    }
    DAT_8009a728 |= 1 << slot;
    func_800471a4(slot, vab_id, sequence);
    return slot;
}

ADDRESS(0x80046988, 0x10c)
s16 func_80046988(u8 *data, s16 vab_id, s16 count)
{
    s16 slot;
    s32 i;
    u8 found;

    if ((s32)DAT_8009a728 < 0) {
        printf("Can't Open Sequence data any more\n\n");
        return -1;
    }
    for (i = 0, found = 0; !found; i++) {
        if (((1 << i) & DAT_8009a728) == 0) {
            slot = i;
            found = 1;
        }
    }
    DAT_8009a728 |= 1 << slot;
    for (i = 0; i < count; i++) {
        data += func_80046a94(slot, i, vab_id, data);
    }
    return slot;
}
