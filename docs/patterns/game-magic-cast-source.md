# GAME magic casting source reconstruction

## Function Match Plan

GAME `0x8003a2a0 magic_cast`, 1216 bytes, `game.magic`, existing
`probe-gcc257-o2-g0`. At `cf01219`, strict objdiff is 97.375000% and the
compiled body is 1204 bytes. The six semantic views, retail switch words,
raw base/target objects, source history, caller, neighbors and shared
interfaces were inspected before editing. The only direct caller is
`player_update` at 0x800193ac; it supplies no arguments and ignores the
result. Keep `void (void)`.

The byte-sized selected spell gates a five-entry switch for IDs 4..8.
The owned RODATA at 0x80012dc0 contains four +0x48 entries and one +0x318
entry (spell 6); the navigator leaves these pointer rows candidate and
the dispatch jr unresolved. The bounded load chain independently supports
those destinations. There are 13 proven direct calls, seven validated
internal jumps and 24 validated HI16/LO16 pairs. No strings are referenced.
Calls comprise rotation-matrix setup, ApplyMatrix, two cone searches, angle
construction, direction construction/scaling, four effect constructors,
rsin and rcos. The function is absent from vendor/FID inventories; Sony
ApplyMatrix/rsin/rcos remain archive-attributed SDK calls, not reconstructed
game bodies.

The current player, actor and collision owners must remain intact. Relevant
referents are selected spell +0x5c, camera position +0xa4..ac, camera rotation
+0xb8..bc, actor player-target +0x2b44, the cell-attribute grid, its signed
height table with the independently supported -2 bias, and floor heights.
Preserve short direction/angles, full-word positions and distance, the
constructor's type-dependent varargs, signed divisions by 800/2000 and
their checked expansions. Constants include offset (-200,200,400), cone
range 20000/tolerance 0x155, threshold -4999, height corrections 3000/5000,
ground-spawn distance 6000, cell size 2000 and height scale 100.

The first raw divergence is the 152-byte compiled frame versus retail's
144 bytes. Retail's projectile distance is at sp+104 and its rotation copy
at sp+112; spell 6 passes a separate distance at sp+120 and builds its
ground position at sp+32, overlapping the mutually exclusive projectile
locals. This is a supported lexical-lifetime question, not permission to add
padding or fabricate an unused local. Neighboring magic-record loading
ends with jr/nop at 0x8003a298/9c; effect-pool sweep starts at 0x8003a760.

First isolate the speed CFG: retail sets 600 in both spell-ID comparison
slots at 0x8003a3cc and 0x8003a404, then applies the spell-4/7 overrides to
800. Existing C assigns 600 separately in three branches, one of which is
dead before an 800 override; it emits 800 in the null-target compare slot
and shares a later 600 tail. Initialize the default once before the target
split and retain only meaningful overrides. Compare from the first real
divergence after the focused build; any subsequent lexical-lifetime or
angle-expression experiment gets its own evidence and verdict.

Require strict canonical comparison, ordered raw referents, all-image
regression checks, full build and existing lint/tests before a commit.
Only a verified strict-100% result is eligible for banking; never weaken
existing data/placement gates to claim closure.

## Speed initialization control

A single pre-split default is hoisted before the matrix calls, adds an s2
save and does not reproduce either retail default-filled comparison slot.
Do not retain that shape. The next control puts the default at the start
of each target-present/target-absent arm, matching the two distinct retail
definitions without changing the spell overrides or any other expression.

The common-default control scored 95.634865%. Arm-local defaults restore
both retail `li s0,600` comparison slots without adding a saved register.
Retain them while checking the independently evidenced stack lifetimes.

## Case-local storage control

Projectile direction, offset, angles, world position, matrix, distance and
target/scale are used only by IDs 4/5/7/8. Its extra rotation copy is used only
in the ID-8 constructor arm. Spell 6 independently needs a cone-search
distance and target, and its no-target arm owns the ground position. Express
those lexical scopes and give the two distance outputs distinct C objects.
Retail proves that the ground position shares sp+32 with projectile-only
storage, whereas the two address-taken distance outputs use sp+104/sp+120.
Do not introduce a union, raw stack offsets or any unused padding to make
the frame match; test whether the natural scopes reproduce the overlap.

The scopes restore retail's 144-byte frame, its saved registers, both
distinct distance locations and the ID-8 rotation at sp+112. They also
restore the first target pointer's a1 role. The ground position now reuses
the projectile world-position slot at sp+56, not retail's sp+32, so lexical
scopes explain part but not all of the local-storage layout. Retain the
supported ownership split without forcing the remaining position offset.

## Flight-time divisor control

At 0x8003a498 retail loads 800 directly into the same s0 subsequently passed
as the direction scale, then divides the distance by s0. Current C divides
by a literal and updates the speed afterward; the probe materializes a
separate v1 divisor and later s0 assignment. Set `scale = 800` before the
division and use `distance / scale`, expressing travel duration in terms of
the actual projectile speed. This is an already live semantic variable,
not an added constant carrier. Leave the angle-height expressions unchanged.

This recovers the retail divisor register and removes the separate post-divide
speed assignment. The remaining aim-height branch is reversed: retail's
`slti -4999; bnez` selects the 5000 correction, falling through to 3000;
current C emits `beqz` and the opposite block order. Test the directly
equivalent `>= -4999` normal-height arm first, with 5000 in its else arm.
Keep the arithmetic expressions otherwise unchanged for this CFG control.

The normal-first conditional restores the retail branch polarity. Both
branches still compile the angle numerator as world_y - (target_y - offset)
instead of retail's (world_y + offset) - target_y. Test a meaningful shared
`aim_y` intermediate: the height branch selects world_y plus 3000 or 5000,
then one angle call subtracts target_y and uses -distance. This models the
retail shared subtraction/call join without distorting types or constants.

That shared-call control moves the actor-height load outside both branches
and delays the angle store until after division, unlike retail. Revert it;
the normal-first two-call source remains the better-supported control-flow
shape. Do not impose artificial dependencies to select the numerator's
registers or association.

The remaining stack question admits one lexical-ownership control: declare
the ground position at the entry of spell 6's case, not inside its no-target
arm. Its contents are still initialized and consumed only when needed.
This tests case-owned versus inner-arm-owned automatic storage, with the
two distinct distances and the projectile-local scope already established.
Keep it only if the raw layout supports retail's sp+32 storage; no other
declaration reordering or type changes are part of this control.

The ground-position scope control is byte-identical to the kept source and
still uses sp+56; it is reverted. Natural case/arm scope recovery is retained,
but does not establish the remaining original automatic-object types or
allocation. No union, enlarged vector, padding, artificial dependency or
dead zero store is added for that residue.

## Kept result and negative controls

| Source control | Strict objdiff | Verdict |
| --- | ---: | --- |
| Original at cf01219 | 97.375000% | Starting point |
| One common speed default | 95.634865% | Rejected: hoisted before calls, extra saved register |
| Target-arm speed defaults | 98.101974% | Kept: both retail default-filled slots |
| Case/ID-8 scoped automatic objects | 98.282900% | Kept: frame, distances, rotation and target register recovered |
| Divide by the live projectile scale | 98.677635% | Kept: retail divisor and speed lifetime |
| Normal-height branch first | 98.875000% | Kept: retail branch polarity |
| Shared aim-height temporary and call | 96.453950% | Rejected: wrong load placement and angle-store schedule |
| Ground position at case entry | 98.875000% | Unchanged; reverted |

The final body is 301 words/1204 bytes versus retail's 304/1216. All 13
ordered direct calls and all 24 ordered HI16/LO16 referents remain intact.
The source has six internal absolute jumps versus retail's seven (67 versus
68 total text relocations). The absent jump belongs to the no-target,
non-spell-4 path at retail +0x14c, where retail also loads camera pitch at
+0x148 and clears the local pitch at +0x150 before the shared final store.
The source reaches the common pitch-copy block directly. This is an open
CFG/store residue, not a missing relocation candidate to manufacture.

Apart from shifted branch destinations, the remaining instruction differences
are those three omitted words, seven numerator instructions at retail
+0x1c0..1e4 (operand registers and positive versus negative corrections),
and five ground-position references at +0x3ac/+0x40c/+0x444/+0x44c/+0x498
(sp+32/36/40 versus sp+56/60/64). The switch table's four +0x48 targets
remain byte-identical; its spell-6 target is +0x30c versus retail +0x318.
The `.rodata` addend mismatch therefore remains visible in the data gate.
This function and its unit remain partial, with no new bank row.

Verification: actual focused compilation and strict canonical report,
raw instruction/relocation/table audit, Ruff, all 591 existing tests and
`git diff --check` pass their respective controls. Only `magic_cast` changes
among all 484 native report rows; the three exact siblings and every other
score remain intact. GAME stays 282/362 exact, OPEN 97/108 and PSX 1/1.
Full `kf build` still exits 1 at the existing data/ownership/placement gates:
source data 7/60 (PSX 0/1, GAME 5/40, OPEN 2/19), complete SDK contributions
4/4, target relink 110/116 (1/1, 75/77, 34/38), no artifact failures or banked
regressions. No SDK source, shared type, inventory, profile or gate changed.

## Exact pitch initialization and height intermediates

### Function Match Plan at `44ab6f9`

The refreshed six GAME semantic views and focused comparison start at
98.891450%, 1204 candidate bytes. Read the current player-update call at
`800193ac`, adjacent record loader/sweep, angle helper, source history,
shared types and the prior controls above. The authentic SVECTOR extent
correction made since the earlier campaign restores the ground-position
stack offsets; no remaining automatic-object layout change is needed.
Keep the no-argument signature, 144-byte frame, thirteen direct calls,
twenty-four address pairs, seven retail internal jumps, seventeen branches,
all delay slots and the five-entry switch contribution. The navigator's
indirect jump and candidate table rows retain their existing evidence tiers.
SDK ApplyMatrix/rsin/rcos stay separately attributed external providers.

Retail's untargeted non-Lightning path loads camera pitch, jumps to the
shared pitch store and clears the local pitch in the jump's delay slot.
Test zero-initialization followed by adding camera pitch to that actual
local. This consumes the initialized value and explains the decoded store;
it introduces no dummy carrier, volatile access or unused object. The
targeted non-Lightning path keeps its direct camera-pitch assignment.

Both Lightning height branches must compute the world height plus the
positive 3000/5000 offset, subtract target height and call the angle helper.
Cross the pitch control with four equivalent signed source expressions:
elevation-minus-target, difference-plus-offset, offset-plus-difference and
negative-target-plus-elevation. Compare the raw operation sequence before
considering any composed control. Retain all per-arm calls, the real speed
divisor, height-table bias and constructor arguments. Only strict 100% after
focused/full verification permits banking and master integration.

### Thirteen JSON states

The eight-state matrix recovers all three missing instructions only with
zero-initialization followed by camera-pitch addition. The original height
expression then scores 99.894740% with the correct 1216-byte body. Exactly
seven arithmetic words remain different: the candidate adjusts target Y
negatively rather than world Y positively. The other expression forms do
not resolve them (98.996710–99.078950% with pitch initialization).

The five-state follow-up composes that initialization with consumed height
intermediates while retaining the angle calls inside their respective
branches. This differs from the earlier rejected shared-call trial.

| Height form | Strict result | Verdict |
| --- | ---: | --- |
| Original complete expression | 99.894740% | Control |
| Branch-local adjusted world height | 100% | Kept |
| Branch-local height, then compound subtraction | 100% | Equivalent exact control |
| Separate branch-local target and world heights | 100% | Equivalent exact control |
| Case-owned adjusted height reused by both branches | 99.835526% | Rejected |

The retained `s32 aim_y` in each branch represents the positively adjusted
world height and is immediately consumed by the existing angle call. No
type, constant, source operation, call topology or storage owner is invented
to obtain the result. The resulting source explains retail under the pinned
probe; it does not prove historical compiler or variable-name attribution.

### Exact verification

Fresh `kf try` gives four identical listings; canonical `kf match` confirms
all four functions are strict **100%**. Independently resolving relocations
reproduces all **358 retail words**, **15 calls** and **27 address
materializations** across the unit. `magic_cast` itself contributes 304
words, thirteen calls and twenty-four addresses. The other three functions
remain exact. A wrong angle-helper address changes exactly one call word.

All five switch rows independently resolve to retail: four point to
`8003a2e8`, and Fire Wall points to `8003a5b8`. The twenty-byte RODATA
contribution now matches too. No target relocation is added, removed or
weakened; the missing internal jump returns through the source correction.

Ruff, whitespace checks and all 713 repository tests pass (107.291 seconds,
nine optional skips). Full `kf build` reports GAME **335/362**, OPEN
**106/108**, PSX **1/1**, and thirteen exact vendored controls. GAME data
matching improves from 10/41 to **11/41** through the repaired switch table.
Other data results remain OPEN 3/19 and PSX 0/1. The build still exits with
existing data ownership/placement and target-relink failures; target relinks
remain GAME 75/77, OPEN 34/38 and PSX 1/1, with zero artifact failures.

Banking selects the four verified functions in `game.magic`. The retained
zero initialization is added to the current literal ledger. Generated batch
results, raw audit and verification logs stay under `build/`; no tooling,
compiler profile or unrelated source is included in the matching commit.
