# GAME effect halfword scatter

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x800386c4 effect_scatter_triple`, **104 bytes / 26
words**, unit `game.effect_update`, initially **76.923080%** strict objdiff.
Campaign starts at clean `99a1358`, GAME **275/362 exact**. Hash-identical
retail was initialized. All six required semantic views were read with
`--image game`, plus the raw focused comparison, both callers, source
history, shared effect fields and both adjacent retail functions.

- Straight-line, one-block body with a 24-byte frame and saved `$s0/$ra`.
  Three proven `rand` calls at +`0xc/+0x24/+0x3c` are the only three
  relocations. There are no globals, strings, candidate outgoing references
  or indirect transfers in this helper. It returns void.
- The first call captures the input pointer in `$s0` in its delay slot.
  Each call is followed by `lhu` at pointer+0/+2/+4, an arithmetic shift
  of the random word by eight, subtracting 64 from the loaded halfword,
  and adding the random increment. The first two `sh` stores occupy the
  next call's delay slot. The third store precedes the epilogue; the frame
  is released in the final `jr` delay slot. The fourth halfword is untouched.
- Both callers are in `effect_update_dispatch`. At `80038f34`, the caller
  first copies eight bytes from effect+`0x2c` to stack+64 using paired
  unaligned word accesses, passes that stack address in the call slot and
  supplies the modified copy to the effect constructor. At `80038f8c`,
  it passes the live effect's +`0x2c` field family in the slot. Neither
  uses a return value. The surrounding candidate jump/table inventory is
  not promoted by this direct-call audit.
- The source's `u16 *values`, caller's `u16 scatter[4]`, and shared
  `KfEffectRecord.direction_x/y/z` unsigned halfwords agree. The curated
  identity still spells `s16 *arg0`; update only this stale signature
  metadata to `u16 *values`, with the body/caller evidence. Do not create
  a different record layout or widen the six-byte mutation extent.
- Both neighbors, `effect_floor_deform_line` and exact
  `effect_rotate_scale_offset_y`, were inspected completely. Original
  `1914c0e:src/game/effect_update.c` contains the same unsigned signature
  and additive expression; later naming/ownership history was reviewed.
- Vendor negative control: this helper is absent from the SDK/FID census;
  its three-field effect mutation is game policy. `rand` is independently
  vendored as `LIBAPI.LIB:C47`, with a unique fixed-byte Psy-Q signature.
  Its retail trampoline loads BIOS selector 47 and jumps via the A0 vector;
  it is not reconstructed or counted as game work. The existing authentic
  `RAND.H`/project LIBAPI wrapper was read; retain the `int rand(void)` API.

The focused comparison rules out the initially considered call/load-order
issue: those instructions already match. The six differing words simply
reverse `addiu value,-64` and `addu value,random` for each lane. The current
source is `(value + (rand() >> 8)) - 64`; retail independently supports
`(value - 64) + (rand() >> 8)`. First hypothesis: express that association
directly for all three real updates, retaining the existing types, signed
shift, constant, RNG call count/order, final halfword stores and frame.
Require strict 100%, all 26 words and three ordered call relocations. No
fake locals, extra calls, volatile, inline assembly or forced registers.

## Focused trials

The explicit `(value - 64) + random` expression is rejected: the probe
combines the constant with the random increment, preserves `0xffc0` in a
saved register across the calls and grows the frame to 32 bytes. This moves
away from retail's 24-byte frame and three immediate subtractions. No source
fact requires retaining that result merely because the expression reads
like the retail instruction order.

Second bounded hypothesis: express each genuine in-place update as
`values[i] += (rand() >> 8) - 64`. This makes the signed RNG-derived increment
the RHS of the mutation instead of explicitly reassociating an addition
tree. The original mutable halfword, three independent RNG draws and modulo
halfword result are unchanged. Compare the actual subtraction operand,
frame and first divergence; do not add a temporary or forced narrowing.

The compound trial is also rejected. It retains the 24-byte frame but moves
each arithmetic shift before the halfword load and subtracts 64 from the
random increment, not the stored value. Canonical strict objdiff falls to
**75.769230%**. Neither trial reproduces the independently observed retail
association and load order. The original body is restored; the remaining
arithmetic-association residue is unattributed. These controls do not
justify fake temporaries, forced widths or further ungrounded permutations.

Only the independently supported unsigned signature metadata is retained.
The actual C interface and body return to their starting state. No exact
result is claimed and nothing is banked in this scatter audit.

## Final verification

After rebuilding the restored source and running canonical matching, scatter
is again **76.923080%**. All **484 function comparisons**, including 13
vendored controls, are unchanged from `99a1358`; GAME remains **275/362**,
OPEN 97/108 and PSX 1/1 exact. The source file has no retained diff.

The existing repository test run passes **551 tests** (52.723 s), Ruff and
`git diff --check` pass. Full `kf build` on the restored source still exits
1 on unchanged data/ownership/relink gates: source data 11/59, config SDK
data 2/2 and target relink 108/114, with incomplete known-reference ownership.
Only signature metadata and durable evidence are committed; no baseline,
relocation inventory, SDK body or build setting is changed.
