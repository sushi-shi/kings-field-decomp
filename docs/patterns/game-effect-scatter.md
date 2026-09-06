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

## Function Match Plan at `3b1c834`

Revisit the 104-byte helper after the map-copy closure, with **394/471**
eligible functions exact. Retail hashes, all six semantic views, both
call-site windows, the complete two neighbors, the vendored RNG trampoline,
SDK RAND.H, shared direction fields and original source history were checked
again before editing. The source still has the original arithmetic and
scores **76.923080%**; no data, signature, owner, relocation or compiler
profile needs changing. The profile is `probe-gcc257-o2-g0`, still a probe.

The halfword fields now live in `KfEffectDirection.words`; the copied caller
uses the aliasing SDK SVECTOR view of the same eight bytes. Both calls supply
the same six mutable bytes as in the original audit. The fourth halfword is
still untouched. `rand` remains the separately attributed LIBAPI C47 BIOS
trampoline at `8005049c`, with selector 47 and vector A0; no provider body is
reconstructed here.

A fresh focused compile repeats the three pairs of differing instructions:
source adds the shifted random value before subtracting 64; retail subtracts
64 from the loaded halfword and then adds the shifted random value. Both
preserve `rand; lhu; sra` and the three calls at +0xc/+0x24/+0x3c. Test the
RNG-first source expression `(rand() >> 8) + (values[i] - 64)`. This differs
from the previously rejected value-first association and compound assignment.
It retains the observed call-before-load and value-side bias without adding
temporaries, narrowing casts, calls or register constraints. Operand order
and the subtraction source must be verified from raw MIPS, not inferred from
the source expression's algebraic equivalence.

The unit has six strictly exact siblings (the focused text display calls
the exact floor helper non-exact only because source uses `.data` and target
uses its private datum name). The 2D projectile helper is still 99.934210%
with different stack-frame operands. Preserve all seven other function
scores and the exact 35-byte floor-deformation initializer. Require native
100%, all 26 relocated retail words and three ordered numeric RNG targets
before banking. Compare all 484 score rows and run focused/full builds,
Ruff, repository tests and diff checks before commit.

The RNG-first expression is rejected: it also preserves `0xffc0` in s1,
grows the frame to 32 bytes and biases the random value instead of the loaded
halfword. Its body is 116 bytes, versus retail 104. Reordering an algebraic
expression does not establish retail's value lifetimes.

Next test the two actual signed-word intermediates seen in retail: capture
the `int rand(void)` result, then compute the promoted halfword minus 64,
then add the shifted random result and store the low halfword. Explicitly
capturing these used values separates the call from the arithmetic tree and
preserves a signed intermediate capable of representing values -64..65471;
there is no forced u16 intermediate narrowing. Retain the three draws and
six-byte mutation extent, and check that these locals disappear into the
observed v0/v1 dataflow with no additional frame or saved-register carrier.

The signed-intermediate form recovers all three load/shift/subtract/add
sequences, the 24-byte frame and the 104-byte body. Its remaining six
differences are the add-result and store-source register: candidate writes
the sum to v0 and stores v0; retail accumulates into v1 and stores v1.
Test the observed in-place accumulation explicitly as
`centered += random >> 8; values[i] = centered;`, retaining the same genuine
signed intermediate instead of creating a separate final addition value.

The in-place signed accumulation reproduces every normalized scatter
instruction and relocation line. It recovers the three
`addiu v1,v1,-64; addu v1,v1,v0` pairs and their original stores, keeping
the 24-byte frame and 104-byte body. Retain it subject to native scoring,
fresh raw relink and regression checks. These are the actual RNG return and
biased halfword value, not synthetic register carriers; no historical
compiler mechanism or unique original variable spelling is inferred.

### Strict and raw verification

Canonical objdiff confirms **76.923080% -> 100%** for scatter. A separate
fresh compile matches the production object's sections. Relinking its real
relocations to the curated numeric addresses reproduces all **26 retail
words**, including the final return delay slot. Its three physical
`R_MIPS_26` entries are at +0xc/+0x24/+0x3c, all targeting `8005049c`;
there are no data references. Delinking and relinking the target independently
also reproduces all 26 words. A wrong-RNG-target control (+4) fails raw
equality, so target bits are not being masked away.

All six previously exact siblings are independently raw-exact, and the
35-byte floor-segment initializer remains byte-identical on both sides.
The 2D projectile's existing frame-operand differences are unchanged; this
unit is now **7/8** strictly exact. The focused textual floor-helper diff
is still only `.data` versus its named static datum, not a raw mismatch.

All **484** score rows were compared with the `3b1c834` snapshot: only
scatter changed. Eligible exact counts rise **394/471 -> 395/471** and
GAME **295/362 -> 296/362**; OPEN stays 98/108, PSX 1/1. The restored
map forward probe remains 93.755104%, with no source diff. Bank only
`GAME.EXE:800386c4`; no other baseline, profile or inventory is changed.

Ruff, `git diff --check` and all **649 repository tests** pass (93.194 s).
The affected unit was rebuilt and freshly compiled independently; full
`kf build` still exits 1 on the pre-existing data/ownership/relink gates.
Source data remains **7/60** (GAME 5/40, OPEN 2/19, PSX 0/1), config SDK
data **4/4**, and target relink **110/116** (GAME 75/77, OPEN 34/38,
PSX 1/1), with zero artifact failures and six conflicting-section-base
units. This function closure does not waive those full-image failures.
