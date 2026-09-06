# GAME map-object clearing

## Function Match Plan and shared evidence

Campaign starts at `0ab6950`, clean `master`, GAME **272/362** strict-exact.
`kf init` validated all retail hashes. Both functions operate on the same
190-record `KfMapObject` pool and its eight-byte link block; the relationship
comes from field use and callers, not only address proximity. All six required
semantic views were read for each function with `--image game`, including
unfiltered references, plus source history and adjacent functions.

The existing shared layouts are retained: map-object state definitions at
owner+0 and 190 live records at +`0x500`, stride `0x2c`; `KfMapObject.link`
at +`0x20`, size eight; link ID +0, parameter +1, sequence halfword +2,
signed vertical velocity +4 and two unknown bytes +6. The placement loader
copies this complete link block; both clearing functions use the same owner.

Vendor negative controls: neither target appears in the vendored/FID census.
The fixed custom pool/record layout, link policy and reserved-slot counters
exclude a libc or GTE wrapper. Psy-Q `MEMORY.H` was read and supplies the
authentic `extern void *memset()` declaration already imported by the unit.
SDK providers remain separate; no library body is reconstructed or counted.

## GAME 80031c44: clear matching links

`map_object_pool_clear_link`, **132 bytes / 33 words**, `game.map_object`,
starts at **95%** strict objdiff. The unit starts at 5/8 exact.

- Six-block leaf, no frame, calls, strings or indirect transfers. One
  validated HI16/LO16 pair targets `map_object_state+0x500`; a fixed -1280
  adjustment recovers the definitions from that base. No candidate outgoing
  references exist. The input is masked to `u8`; no result escapes.
- The body uses `lbu` for object ID, definition behavior and link ID. At
  `80031c84`, `bnez` on `behavior < 8` goes directly to the link comparison
  at `80031c94`. Otherwise, behavior 8 falls through to that same block and
  other values skip it. Thus retail admits **0 through 8**, not only 8.
  Matching links are set to `0xff`; no other record fields are changed.
- The current C spells `behavior >= 8 && behavior == 8`, incorrectly
  excluding 0..7. Its first divergence is the branch to loop tail instead
  of the link comparison, followed by the prematurely scheduled cursor
  increment. This is a real CFG/predicate error, not a scheduling limit.
- Both derived cursors advance 44 bytes. The loop tests the old `u16`
  countdown from 189; decrement-by-`0xffff` is in the branch delay slot.
  Load delays and the final `jr`/`nop` belong to the function.
- All five proven caller sites were inspected in asm and source: event
  interaction `80034ce0` passes `0x37`; floor restore `8003633c`, `80036360`,
  `800363a4` pass `0x33`, and `8003647c` passes `0x34`. Each immediate is
  produced in the call delay slot; no return is used. Indirect floor-switch
  inventory gaps in the caller are not promoted by this audit.
- The preceding exact trigger helper's complete body and the following
  action dispatcher's entry were reviewed, alongside the original source
  commit `3839393` and the old residue note.

First hypothesis: change only the behavior predicate to
`(behavior < 8 || behavior == 8)`, keeping the separate link-ID condition,
shared definitions pointer, counter, store and interface. Correct its source
comment. Require strict 100%, all 33 words and the two ordered relocations.

## GAME 80030f7c: reset the pool

`map_object_pool_clear`, **96 bytes / 24 words**, `game.map_object_pool`,
starts at **78.25%** strict objdiff. The unit starts at 4/8 exact.

- Three-block leaf, no arguments, return value, calls, strings or indirect
  transfers. Four validated HI16/LO16 pairs target `map_object_state+0x500`
  then the halfword counters for reserved slot bands 180, 170, 160 in order.
  No candidate outgoing references exist.
- A 190-record scan sets object ID +0 and action +`0x28` to `0xff`, then
  clears the complete link block using **two word stores**, at +`0x24`
  followed by +`0x20`. The two cursors advance 44; its old narrowed counter
  and branch-delay decrement are the same form as the link scan. It then
  clears the three sequence halfwords and returns with a final `nop`.
- Existing source writes the same eight bytes through six member stores.
  The first difference is the compiler's derived member cursor at +34
  instead of +32, followed by halfword/byte stores instead of the two words.
  There is no call or relocation discrepancy to work around.
- The sole caller is startup at `80014778`, after actor clear and before
  effect reset, with a `nop` call delay slot and no input/result use. Its
  call site was inspected. Both adjacent functions, the forward collision
  probe and exact definition loader, were read completely; their bodies
  remain outside this edit. Source history includes consolidation `c73e32f`.

First hypothesis: spell the complete zeroing operation as
`memset(&object->link, 0, sizeof(object->link))` after the two `0xff` stores.
Keep all existing ownership, field types, loop and post-loop stores. Inspect
whether the pinned probe expands the fixed-size clear into the two retail
word stores; if it emits a call or another store order, do not force it with
assembly, fabricated carriers, padding or incompatible structure views.

Both results require focused rebuilding, canonical strict matching, raw
instruction/ordered-relocation review and the full build before banking.
All previously exact functions must remain exact; full data/relink closure
is separate unfinished work and its gates must not be weakened.

## Final verdicts and verification

The corrected link predicate is **100% strict objdiff**, up from 95%, after
focused rebuilding and canonical matching. All **33 encoded words** agree,
including the branch to the common link test, delayed cursor increments,
counter decrement and final return delay slot. Both ordered relocations at
function-relative +0/+4 are the same HI16/LO16 pair; the encoded low addend
is `0x500` into `map_object_state`. The enclosing unit is now **6/8 exact**.

The pool-reset `memset` trial emits a real call and 40-byte frame rather than
the retail leaf's two word stores. It is reverted, leaving the original
78.25% reconstruction intact. The complete link type is already shared;
this result does not justify fabricated carriers, a forced call, or a
per-file incompatible word view. Its word-store residue remains unattributed.

The related [forward-probe correction](game-map-forward-probe.md) improves
from 1.8163265% to 93.755104% but is not exact or banked. These are the only
two changed comparisons across all 484 report rows, including 13 vendored
verification controls; the other 482 are unchanged. GAME moves from
**272/362 to 273/362 exact**, OPEN stays 97/108 and PSX stays 1/1. No
previous exact result regresses.

Repository verification: **551 tests pass** (47.675 s), Ruff passes and
`git diff --check` passes. The full `kf build` was run and still exits 1 on
the existing non-code closure gates: source data 11/59, config-provided SDK
data 2/2 and target relink 108/114, with incomplete known-reference ownership.
No gate, data inventory, SDK attribution or toolchain setting was changed.

## Pool-reset continuation at `b19ed81`

The world-collision continuation was verified progress; `master` is clean.
Hash-identical retail and all six GAME semantic views were refreshed for
`80030f7c map_object_pool_clear` (96 bytes, 78.250000%). The current
`game.map_object_pool` unit has five of eight functions exact. Its complete
reset CFG, sole startup caller at `80014778`, both immediate neighbors,
complete placement loader, SDK MEMORY.H, source history and shared link
field consumers were read before this edit. The existing no-argument/void
contract, 190 iterations, four validated address pairs, zero calls/strings,
three sequence halfword stores and both control delay slots are unchanged.

The first real divergence is still the link-derived cursor at object+34
instead of +32, followed by six narrow stores rather than retail's two
word clears. The earlier memset trial is not repeated. The independently
exact placement loader at `80031170..8003117c` copies this complete link
using two aligned `lw` and two `sw` instructions. Its shared source already
expresses that bulk representation through `u32 *` operands to memcpy.
The serializer separately preserves all eight link bytes; individual
consumers prove the actual byte/halfword fields and must keep those types.

First hypothesis: use the same aligned whole-link word view for the reset,
writing word 1 then word 0. This is a bounded bulk operation on the already
owned eight-byte link, not a replacement structure, overlapping global or
per-file incompatible field layout. The runtime pool is word-aligned,
records have 44-byte stride, and link starts at +32; both stores are aligned
and stay within the link. Keep the object-ID/action byte stores, old u16
countdown, cursor advancement and sequence order. Do not infer that the
semantic fields themselves are words or change their shared declarations.

Require a fresh focused compile, strict 100%, all 24 linked instruction
words and all eight ordered relocation entries. Verify the five exact
neighbors and the other images before banking only this function. The
unit's existing RODATA/placement issues remain separate closure work.

The first aligned-block trial is **100% strict objdiff**. All 24 retail
words appear in the focused comparison, including the object+32 derived
cursor, both word clears and the old-counter branch delay slot. The other
seven function scores are unchanged; the unit now has six exact functions.
Add compile-time shared-layout checks for link size eight, object stride
44 and link offsets +32/+12 in runtime/placement records before final
verification. No struct representation or field type is changed.

The layout-check rebuild remains 100%. A separate fresh compile under the
manifest's exact GCC 2.5.7/O2/G0/r2000 profile produces the production
sections byte-for-byte. Resolving each function's real references reproduces
all retail instructions for the new reset and all five exact controls:
collision-edge marker (572 bytes), definition loader (44), placement loader
(1096), distance helper (168) and near-point search (204). Reset's eight
relocation entries are HI/LO pairs at +0/+4, +64/+68, +72/+76 and +80/+84,
materializing `8006ede0`, `80070e96`, `80070e94`, `80070e92` in that order.
The source and target have no call or internal-J relocation in this leaf.
A deliberately shifted pool base fails the raw-byte comparison; addresses
and addends are not masked to obtain exactness.

Removing the reset's 16 excess bytes also removes the previous
placement-loader switch-table displacement. All 84 RODATA pointers now
relink exactly to their retail destinations, and the 24 initialized map-copy
bytes remain exact. The unit's `.data` and `.rodata` comparisons both pass;
its two remaining partial functions do not become exact by this data result.

All 484 reported function rows were compared with the `b19ed81` snapshot;
only the reset changes (78.25% to 100%). GAME increases to **294/362 exact**,
OPEN remains **98/108**, and PSX remains **1/1**: **393/471** eligible total.
All thirteen vendored verification functions remain exact. No source body
from a library, gate relaxation or compiler-profile change is included.

Ruff and `git diff --check` pass; all **649 tests pass** (89.198 seconds).
After the fresh focused build, the full `kf build` runs all three image
checks and still fails on remaining data/ownership/placement work. Source
data improves from 6/60 to **7/60** exact units (GAME 5/40, OPEN 2/19,
PSX 0/1); SDK/config data remains 4/4, target relink remains 110/116 with six
conflicting-section-base findings, and artifact failures remain zero.

`kf bank --function game:0x80030f7c` banks only this verified exact function;
the baseline diff contains one row. The unit's partial copy-region and
forward-probe functions are not banked by this continuation.
