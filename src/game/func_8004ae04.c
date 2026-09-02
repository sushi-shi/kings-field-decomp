#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Thin sequence-audio entry wrapper.  A Psy-Q 2.60 signature corpus flagged
 * this 0x2c body as LIBSND Snd_play/SsSetNoiseOn, but that was a
 * wildcard-object-signature match (41/44 bytes fixed, two archive candidates):
 * the body tail-calls the game routine func_800476e8, part of the
 * DAT_800a06e0 sequence-audio family, so it is game glue rather than a vendored
 * library body.  Reclassified out of functions_vendored.tsv accordingly.
 */

extern void func_800476e8(s16 arg0, s16 arg1);

ADDRESS(0x8004ae04, 0x2c)
void func_8004ae04(s16 arg0, s16 arg1)
{
    func_800476e8(arg0, arg1);
}
