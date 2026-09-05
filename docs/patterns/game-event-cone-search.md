# GAME map-event cone search

Final result: strict **100%**, with the original `s16 facing` interface
unchanged. The initial wider-parameter hypothesis below was rejected after
recovering the short angle locals; it is recorded as an experiment, not as
the final ABI conclusion.

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x80033b8c map_event_pool_find_target_in_cone`, 324 bytes,
`src/game/map_event.c` / `game.map_event`; initial strict objdiff 87.530860%.
GAME starts at 269/362 exact. Retail hashes were validated with `kf init`,
and all six matcher semantic views were inspected with `--image game`.

- The sole decoded caller is `player_use_item` (`0x80018054`), call site
  `0x80018434`. It first tries the analogous actor selector at `0x80018408`.
  Both calls receive camera position, signed yaw loaded by `lh` from
  `player_state+0xba`, range 6000, tolerance `0x155` in the call delay slot,
  and a fifth argument pointing to the caller's stack distance word. The
  event result is null-tested and passed to `map_event_show_person_image`.
- Retail preserves the incoming facing word with `move s7,a1` at
  `0x80033bc4`; no halfword conversion occurs before subtracting it from the
  angle at `0x80033c30`. The old signature inferred `s16` from the caller's
  storage width, although `lh` already supplies its promoted signed value.
  The actor counterpart also preserves its incoming register unchanged.
  This initially suggested an `s32 facing` hypothesis, but the modulo-4096
  operation does not uniquely establish the original source type from
  linked bytes alone. The later local-width trial resolves the mismatch
  without retaining that parameter change.
- A 64-byte frame saves `$ra` and `$s0` through `$s8`. The selector scans
  eight `KfMapEvent` records with a 68-byte stride and a `u16` countdown
  starting at seven. It filters `state == 1`, rejects distance -1, folds
  the 12-bit angle about 2048, and selects the strictly smallest signed
  halfword error within a signed word tolerance. Initial best error is
  30000; initial result and distance are zero.
- Two proven calls target `map_event_distance_to_point` (`0x80033ae4`)
  and `vector_xz_to_angle` (`0x80014fb8`). Their delay slots set the event
  pointer and Z difference, respectively. The sole validated address pair
  targets `map_runtime_state`, whose first member owns all eight events.
  No strings, candidate references or indirect control transfers occur.
- Retail copies the masked angle to a separate folded value in the branch
  delay slot at `0x80033c40`; the upper-half case assigns `4096 - angle`
  to that folded value. Only then is a signed halfword comparison view
  formed at `0x80033c4c`. The existing actor selector has this same data
  flow; the event source currently overwrites the angle before copying it.
- The bottom branch tests the old narrowed counter and decrements in its
  delay slot. After the loop, the fifth argument is loaded from frame+80,
  the winning distance is stored, and the pointer is returned. The `jr`
  delay slot releases all 64 frame bytes.
- Both neighbors were inspected: the exact distance helper (`0xa8` bytes)
  and exact overlap query (`0x80033cd0`, `0xb0` bytes). The actor counterpart
  (`0x8002d7f8`, `0x184` bytes), its source, shared layouts, caller and source
  history were reviewed. No owner or unit consolidation is proposed.
- Vendor negative control: the target and its two direct callees are
  game-owned event/angle policy, absent from the vendored/FID inventories.
  The distance helper calls independently vendored Psy-Q `SquareRoot0`;
  the supplied `LIBGTE.H` retains its real `long(long)` declaration. No SDK
  body, macro, signature or game-progress denominator is changed.

First hypothesis: change only the facing parameter to `s32`, consistently
in the definition, shared header and curated identity/evidence. Rebuild
the event unit and exact caller. Then inspect the next real divergence
before testing the separately evidenced wrapped/folded angle data flow.
Require strict 100%, all 81 encoded words and ordered relocations for closure.

## Focused results

The full-word parameter removes the extra sign-extension pair, and the
item-use caller remains exact (all three functions in its unit). The event
selector is not yet exact: the probe exchanges which limit is held in a
register versus the frame, still initializes the pool cursor before the
selection accumulators, and still overwrites the wrapped angle before its
folded copy. The observed intermediate instruction similarity is lower;
this does not refute the independently supported parameter width.

Next focused hypothesis: retain the wrapped angle and introduce its actual
folded value before the 2048 branch, just as the decoded branch delay slot
and actor counterpart do. Preserve the explicit signed-halfword comparison
view and assign the winning best angle from the folded word. This is a
data-flow reconstruction, not a register assignment or profile change.

The separate folded-value trial is byte-identical to the width-only trial;
the probe still merges those values. The next independently evidenced
source hypothesis is the actor selector's early-rejection loop: all retail
state/distance/tolerance rejection branches target the same record-advance
tail. Express those guards as `continue`, keep only the winning comparison
nested, and advance the event in the common bottom expression before the
existing postdecrement test. This changes no admitted records or comparisons.

The early-rejection form also emits identical words. One remaining source
fact from the shared actor algorithm is that the helper result is used
directly to initialize the signed heading difference, which is then masked;
it is not first assigned as an absolute heading and then overwritten by a
combined subtraction/mask. Retail's separate `subu` and `andi` stages support
testing that difference-first expression without changing the arithmetic.

That expression also leaves the words unchanged. Retail and the actor source
both initialize the best-result pointer, best-error sentinel and best-distance
before obtaining the traversal cursor. The event source instead initializes
the cursor first, matching the probe's differing order. The next focused
source correction places those active initializers in their evidenced order;
no inactive declaration, artificial live range or extra local is added.

The initializer correction reproduces that setup sequence. The next width
hypothesis models the folded result itself as `s16`, eliminating the separate
word-sized folded value and halfword comparison copy. Retail explicitly forms
a signed halfword at the fold join, compares that value to the tolerance and
best error, and copies its low halfword into the new best error. The wrapped
angle remains a word; its 0..4095 mask and fold bound the result to 0..2048.

The signed-halfword folded value removes the extra copy at the fold join.
The remaining wrapped value is likewise bounded to 0..4095 before use; the
linked register width alone does not prove a 32-bit source local. A bounded
type hypothesis will retain that masked angle as `s16` too, leaving the
full-word facing contract and all arithmetic constants unchanged.

Both short locals now reproduce the angle calculation. This supplies an
important correction to the initial ABI hypothesis: absence of an argument
extension does not distinguish a word parameter from a short parameter when
all high bits are discarded through the recovered short-angle computation.
The only caller already passes signed-halfword yaw. Recheck that original
`s16` signature with these recovered locals before retaining any interface
widening; the first width-only trial was not unique evidence of an `s32` ABI.

## Final verdict

The original `s16 facing` signature plus the two signed-halfword locals
reproduces every instruction. The kept source initializes the best-result
state before the cursor and uses the reviewed common record-advance tail.
Its algorithm, limits, eight-record extent, data owner and caller interface
are unchanged. No forced registers, fake locals, inline assembly or compiler
profile changes were used.

`kf match --unit game.map_event` rebuilt the canonical object and produced
strict **100%** for the 324-byte function. Raw objdump comparison confirms
all **81 words**, including every branch, load/call/return delay slot and
constant. The four ordered relocation entries also agree exactly:

| Relative offset | Kind | Target |
| --- | --- | --- |
| `0x48` | `R_MIPS_HI16` | `map_runtime_state` |
| `0x4c` | `R_MIPS_LO16` | `map_runtime_state` |
| `0x70` | `R_MIPS_26` | `map_event_distance_to_point` |
| `0x9c` | `R_MIPS_26` | `vector_xz_to_angle` |

GAME advances from 269 to **270/362 exact**, leaving 92 partial functions.
The event unit advances from 5 to **6/7 exact**; its image refresher remains
unchanged at 70.793106%. All other 483 function report rows across the three
images retain their sizes and scores, including the exact item-use caller
and its two image-display helpers. Only this selector is selected for banking.

Ruff and all 551 repository tests pass, as does `git diff --check`. Full
`kf build` was run and still fails the existing data-comparison,
known-reference ownership and section-placement gates: 11/59 source data
owners pass and six target units have conflicting section bases. These
failures were not weakened or misreported as successful closure.

The reusable observation is narrower than a compiler-mechanism claim:
full-register subtraction with no input extension is compatible with a
short parameter when the recovered intermediate types discard unused high
bits. Audit those intermediate widths and the caller before widening an
interface. The analogous actor selector has the same fold-copy residue,
but requires its own complete caller audit before changing its types.
