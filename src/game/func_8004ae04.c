#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Thin sequence-audio entry wrapper.  A Psy-Q 2.60 signature corpus flagged
 * this 0x2c body as LIBSND Snd_play/SsSetNoiseOn, but that was a
 * wildcard-object-signature match (41/44 bytes fixed, two archive candidates).
 * The body tail-calls SeqPlay (0x800476e8), the vendored LIBSND SEQREAD
 * per-track player, so this wrapper is game glue over the sequence engine
 * rather than a vendored library body itself.
 */

extern void SeqPlay(s16 arg0, s16 arg1);

ADDRESS(0x8004ae04, 0x2c)
void func_8004ae04(s16 arg0, s16 arg1)
{
    SeqPlay(arg0, arg1);
}
