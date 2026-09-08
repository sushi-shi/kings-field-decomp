# GAME effect/map collision

The later [GCC trace revisit](game-effect-collision-traces.md) recovers the
oriented coordinate's register/conversion sequence through the lower-bound
operand order. The bounds return and final flag/default sequence remain open.

## Function Match Plan at `51ebe75`

GAME `80037850 effect_map_collision` owns 1900 bytes (`0x76c`) and six
switch words at `80012ce0` in `game.effect_map_collision`. It starts at
strict 74.821050%, with a 1600-byte compiled body under the unchanged
`probe-gcc257-o2-g0` profile. Retail hashes, all six semantic views, the
complete disassembly/CFG, six caller windows, world-query callee, adjacent
current-record/power helpers, source history and shared data types were read.
The [collision-data dossier](../../config/evidence/game_collision_attribute_data.md)
already proves the seven typed rectangles and corrected orientation policy.

The interface is VECTOR pointer plus a word radius, returning an encoded
word result. Two projectile helpers and four dispatcher arms pass stack/live
effect positions with radii 100/120/150. All compare the result against -1;
some then inspect its high halfword. Thus the source comment describing
0x10000 as "no collision" is unsupported: retain the literal geometry-rejection
result without changing the callers' actual -1 convention. The source and
shared declaration already use the appropriate VECTOR/s32 interface; the
curated candidate signature is stale. No SDK body is being reconstructed:
the function has no vendor census entry and implements game-grid/attribute/
effect-class policy, with one external game world-query call and no strings.

Retail has a 32-byte frame, saves only ra, and shares its return at `80037fac`
including the `jr` delay-slot frame restoration. Fourteen signed divisions
retain the pinned assembler's checked expansion. Fifteen internal absolute
jumps remain candidate solely on their unreviewed instruction-word channel.
Check every numeric target, source opcode, function bound and delay slot,
then review only those fifteen rows without altering their encoded referents.
The twelve existing address pairs are validated; their ordered targets include
the real grid+100/-100 biases. The bounded byte switch has six entries:
0 -> `80037af0`, 1 -> `80037f30`, 2 -> `80037d2c`, 3 -> `80037dac`,
4 -> `80037e30`, 5 -> `80037eb4`. All land inside the body outside delay slots.
The navigator's unresolved switch edge does not make those arms dead.

First rebuild after relocation review without source changes. Then follow
these independently visible source facts, compiling after each correction:

- Retail sign-extends both cell quotients before indexing and repeats their
  extension at neighbor accesses. The current source retains full-word x/z
  and a cached linear cell index. Recover signed-halfword coordinate locals,
  unsigned bounds comparisons, and the two-dimensional grid accesses.
- The entry captures current_effect before the bounds tests and uses that
  pointer after the geometry checks. Recover that genuine pointer snapshot.
- The selected rectangle is a single pointer formed inside the nonnegative
  attribute-height arm, not the whole array base retained before the guard.
  Its oriented coordinate is narrowed to s16 after selecting 1/2/3/4.
- Geometry shape 4 evaluates the X remainder before the Z remainder;
  current source states the reverse operand order. Preserve the mathematical
  comparison and all inclusive thresholds while restoring the input order.
- The final class dispatch has the ordinary three-case switch layout and
  case-local world-query argument setup. Preserve the observed fallback
  results and complete O32 arguments rather than relying on call counts.

Keep the literal rectangles, byte grids, divisors/thresholds, all early
returns and the current-effect type field unchanged. No forced registers,
padding, fake locals, inline assembly or compiler changes are permitted.
Require strict 100%, complete raw words and ordered physical referents for
closure; keep every already exact GAME/OPEN function and run the full build.

## Reviewed target and first source trial

All 28 text-reference rows validate with the shared safe rules, including
the fifteen newly reviewed internal jumps. No opcode, target, original
channel or delay-slot byte is changed. The six switch entries were checked
against the bounded dispatch and complete body. The C source was compiled
unchanged after this target-only step before starting source corrections.

First change only x/z to s16 and spell their bounds as `(u16)x > 99u`
and `(u16)z > 99u`. Retail masks the low halfword for unsigned bounds, then
sign-extends it for grid indexing. Keep the existing cached linear index
temporarily so its independent address-expression correction is measurable.

Relocation review alone moves strict comparison to 74.842100%. The signed
coordinate correction gives 74.557890%, but restores the two unsigned bounds
instructions and signed quotient extensions. Keep that evidenced correction
while removing the independently unsupported cached linear index. Spell the
grid and four neighbors directly as `[z][x]`, `[z+1][x]`, `[z-1][x]`,
`[z][x+1]`, `[z][x-1]`, retaining short-circuit order and all comparisons.

Direct grid indexing raises strict comparison to 89.644210%, restores the
32-byte frame and the repeated signed-coordinate/biased-grid calculations.
The first non-frame differences now include the late current-effect load
and premature rectangle-array binding. Next capture the existing effect
pointer after the initial coordinate/remainder computations, before bounds,
and consume that snapshot in the final type dispatch as retail does.

The early effect snapshot gives 90.136840% and moves the actual pointer load
to its observed lifetime. Next bind `record = &map_cell_height_records[height]`
inside the nonnegative-height arm and use this one selected object for its
four bounds. This removes the premature full-array base and repeated record
selection visible in the candidate, without changing any rectangle or test.

The selected rectangle pointer gives 91.263160% and places its address pair
inside the observed nonnegative-height guard. Next distinguish the two
word-sized remainders from the signed-halfword oriented coordinate. The retail
keeps both MFHI results and selects a separate value in each orientation arm,
then sign-extends that selected value once before comparing rectangle X bounds.
Remove the premature remainder casts; signed remainder by 2000 already fits
in a halfword. Give the selected result its genuine s16 local and keep the
four mappings and default path unchanged.

The separate oriented halfword gives 91.010530%. The entry through both
divisions and the current-effect snapshot now agrees word-for-word, but the
first divergence remains the bounds rejection join; later orientation arms
still have a different layout. Keep the evidenced selected-value width and
next restore shape 4's X-before-Z remainder order. The two mathematical
comparisons are equivalent; retail computes X first and adds 1000 to Z, while
the previous source computes Z first and merges its tail with shape 2.

Shape 4's ordered expression gives 92.616844% and restores separate shape
2/4 tails and the retail arithmetic in both. The entry join remains the first
divergence. Next replace the hand-transcribed final comparison tree with the
observed three-case switch and case-local calls. Each reachable arm still
executes exactly one world query with its original flags, and masked kind 0
still returns 1 explicitly; do not introduce an undefined return just to
reproduce the incidental register value on an impossible kind-above-3 path.

The final switch gives 94.905266%, restoring its signed dispatch, branch
polarity and case-local height/flag setup. A four-word default/third-case
schedule difference remains; the source keeps the defined kind-0 result.
Return to the first divergence: the two cell bounds checks currently share
their rejection jump with the negative-height arm, while retail has a local
bounds rejection block. Test ordinary signed range predicates on the already
proved s16 coordinates (`x < 0 || x >= 100`, likewise z), which express the
same accepted 0..99 values as the unsigned halfword range test.

The signed bounds spelling is byte-identical at 94.905266%; keep the clearer
natural predicates. For the orientation discrepancy, distinguish selecting
a word value from narrowing it at the rectangle comparison: retail selects
into v0, then shifts into v1 before the first bound load. Test a word
coordinate with the evidenced s16 conversion at its two bound consumers;
the halfword-local form selects directly into v1 and delays its shift until
after the load. Neither spelling changes the oriented coordinate's value.

The word-local conversion-at-use trial gives 94.863160% but does not restore
the retail selected-value schedule and changes earlier live-value registers;
restore the direct halfword local. Next recover the decoded backward join:
the common rectangle-X check lies before the out-of-line orientation 2/3
assignment arms, which both jump back to it. Put that real shared check in
the first orientation arm, with an explicit `rectangle_span` label reached
by the other three mappings. This changes the check's ownership/layout,
not its two inclusive predicates or the default grid fallback.

The explicit span join gives 97.389470% and restores the two out-of-line
backward assignment arms plus the common check's exit jump. The remaining
first divergence is still the bounds/negative-height return sharing. Retail
updates the loaded signed height in place by adding floor before comparing
world Y. Test that actual conversion from relative to absolute height as
`height += floor; if (y < height)`, leaving the nonnegative record index and
all other bounds untouched.

Updating the relative height in place gives 97.452630% and recovers the
retail height load/update/index register sequence. Now that the actual
backward span join is present, recheck the earlier word-selection/halfword-
consumer hypothesis on this corrected CFG. This is a conditioned source
conversion control, not a search over unrelated types or local ordering.

The conditioned cast-at-consumer form gives 96.768420% and repeats the
earlier unwanted record-pointer/selected-value change. Reject that form.
The distinct remaining source question is the X-remainder's destructive
orientation: use subx as the oriented word (unchanged for orientation 2,
replaced by Z or the reflected axis in the other cases), then assign the
real halfword coordinate once at the backward join. Retail's word selection
and subsequent halfword view are separate operations; this control models
both instead of replacing a halfword local with repeated consumer casts.

That axis-update trial gives 96.147370% and does not explain the retail
conversion sequence; restore the separate halfword selection at 97.452630%.
No unsupported local/register ordering trial is retained. Correct the stale
curated interface to the already declared VECTOR pointer/word radius and
correct the C comment's mistaken no-collision sentinel.

## Kept verdict and verification

**Partial, 74.821050% -> 97.452630%; not banked.** The fresh compiled body
is 1876 bytes versus retail 1900, with the correct 32-byte frame. The retained
source recovers signed cell widths, repeated two-dimensional neighbor
accesses, the early effect snapshot, selected rectangle, absolute height
update, backward span join, ordered diagonal calculation and three-case
query dispatch. The original TU boundary and historical compiler attribution
remain unproved. This is custom game collision policy, not vendored progress.

The first raw mismatch is still `+0x94`: the compiled bounds branch reaches
the negative-height rejection jump, whereas retail reaches its own local
bounds rejection block at `+0xac`. Remaining differences are concrete:

- Bounds rejection lacks retail's separate two-instruction jump/constant
  block; the negative-height branch also has a different delay-slot fill.
- Orientation arms now have the correct backward shared-check topology,
  but select the short in v1 rather than retail's v0, and the first bound
  load precedes its shift instead of following it.
- Shape 2 shares the final shape-5 rejection branch instead of keeping its
  own branch/return sequence. This accounts for another three instructions.
- The final switch's defined default and third-case flag setup differ by
  one instruction and delay-slot placement. Do not erase the default solely
  to imitate an incidental return-register value.

These differences account for the six-word extent deficit; branch and
switch-table destinations move accordingly. Do not call this complete CFG
agreement or an attributed compiler wall.

A separately fresh source compile equals the production object's sections.
Resolving every physical text relocation without masking confirms all twelve
ordered data addresses and the sole call to `8001a5ac`, including both
collision-grid biases (`8009807c`, `80097fb4`). The compiled body has 39 text
relocations (12 HI/LO pairs, 14 internal J, one JAL) versus retail's 40
(15 internal J). The reviewed delinked target restores **all 475 retail
instruction words** and all six switch-table words exactly. Both source and
target preserve the complete 56-byte initialized rectangle array. Source
switch entries remain displaced; this is not a data-section exact claim.

The five existing collision controls pass, including 2352 rectangle/
orientation/boundary combinations on each side and all three hooked query
flag cases. Ruff passes; all **649 repository tests pass** (85.772 seconds).
The full `kf build` runs and remains red on existing whole-repository data,
ownership and placement work: 6/60 source-data units exact, 4/4 independent
SDK/config-data contributions exact, 110/116 target units relink, six
conflicting-section-base findings, and zero artifact failures. The collision
unit still has its known data-alignment issue and non-exact switch addends.

All 484 reported function rows were compared with the pre-campaign snapshot;
only this function's score changes. Exact counts remain GAME 293/362,
OPEN 98/108 and PSX 1/1 (392/471 eligible total); all 13 vendored verification
functions remain exact. No banked match regresses and no baseline is changed.

## Return-form continuation at `a0ffa9b`

The previous campaign is verified progress, not an exact closure. The retail
hashes and six image-qualified views were refreshed at strict 97.452630%.
No source, claim, data, relocation or compiler change intervened. The 1900-byte
body still has the same six direct callers, one game-query callee, no strings,
twelve address pairs, fifteen reviewed internal jumps and shared 32-byte
return frame described above. Caller/constructor source review supplies no
reason to replace the API or manufacture a new result on an unknown path.

First source hypothesis: represent the encoded return value as one real
result local initialized to the geometry-rejection code. Each rejection
exits through the shared result return, while effect-type cases assign their
world-query result and type zero assigns 1. This tests return-value ownership
and the decoded shared epilogue, preserving all defined values, call count,
query arguments, divisors, inclusive thresholds and selected-coordinate
semantics. Do not add an unused result carrier or drop the default. Compare
the first bounds branch after a focused fresh compile; keep only source
supported by the raw paths, with all previous exact functions preserved.

The shared-result form gives 95.684210%, introduces a long-lived rejection
value in t6, and branches directly to a trailing move into v0. It does not
recover the retail local bounds return and changes the entry remainder and
effect-pointer roles. Reject this hypothesis and restore the committed
97.452630% source. The result-variable ownership does not explain the missing
return block; no artificial liveness or undefined-default trial is justified.

## Entry-guard scope control at `4eb958c`

Fresh hash validation, all six GAME views, all six caller windows, both
adjacent helpers, the world-query source/ABI, shared typed geometry and source
history confirm the same 1900-byte retail / 1876-byte candidate at strict
97.452630%. The 32-byte frame, one query call, twelve address pairs, fifteen
retail internal jumps, fourteen checked signed divisions and one return remain
the contract. The grid dispatch is bounded to six raw table entries; its
unresolved indirect edge still prevents a complete navigator reachability
claim. This game-specific geometry/active-effect policy is not SDK code.

The first bounds branch at +`0x94` reaches the local rejection at retail
+`0xac`; the candidate reaches its negative-height rejection at +`0x154`.
Both return `KF_COLLISION_TERRAIN`, so this is different block sharing, not
evidence of a missing collision result. Retail's negative-height branch
prepares that result in its delay slot, leaving the following return jump's
slot empty; the candidate prepares the result in the shared jump's slot.
The two diagonal X/Z cases also differ in rejection-tail sharing.

Test one positive in-bounds scope containing the entire existing cell query,
with the bounds-failure return after that scope. This preserves short-circuit
X-before-Z evaluation, accepted signed-halfword coordinates 0..99, and every
body statement and return, but distinguishes the entry guard's source scope
from the existing early rejection. It does not repeat the already negative
signed-versus-unsigned predicate or long-lived result-variable controls.
Do not introduce a result carrier, extra memory access, padding, assembler,
undefined default or compiler change. Inspect the first raw branch and all
numeric referents after a fresh full-unit trial; reject the spelling if it
does not explain the separately emitted bounds rejection.

The positive scope does not recover the entry rejection: both range failures
now branch to a new trailing constant-return block. Although it restores
retail's separate shape-2 exit, the added trailing jump/constant explains the
same overall 1900-byte extent without making the body exact. Do not retain
the positive scope or equate its 126 blocks/78 branches with matching paths.

The remaining return-sharing question is specifically the negative-height
arm. Retail's passing height test branches directly to `grid_shape`; its
failure falls through to a local return jump. On the original source base,
test that explicit passing continuation (`if (y >= height) goto grid_shape;`)
followed by the unchanged terrain return. This preserves the signed test and
all possible outcomes, does not add a result local, and distinguishes that
existing shared continuation from implicit completion of nested `if` scopes.
Check whether it explains both the negative-height delay slot and the entry
bounds return before pursuing any later register/conversion symptoms.

The explicit negative-height continuation emits the original listing and
retains the same first branch. Neither entry-control trial is kept. The
positive-scope control did independently expose the missing diagonal case's
separate success/rejection sequence. Isolate that evidence on the original
base: in `KF_MAP_CELL_X_GE_Z`, spell the successful `X remainder + 1000 >=
Z remainder` path as an early switch `break`, followed by the unchanged
terrain return. Retail has precisely that branch to the post-switch query
with a local rejection fallthrough. Keep X-before-Z remainder evaluation,
signed comparisons, inclusive boundary and all other cases unchanged; no
new comparison value or shared return carrier is introduced.

The isolated early `break` recovers retail's five-instruction case exit:
signed comparison, successful branch to the post-switch query, terrain value
in its delay slot, and a separate rejection jump with an empty slot. The
candidate grows from 1876 to 1888 bytes. An independent complete-word audit
proves that the old two-word shared-tail jump is the only replaced sequence;
all other instructions agree with the original after translating destinations
for the added twelve bytes. All twelve ordered numeric data targets, the sole
query call and all 56 rectangle bytes remain unchanged. Keep only this local
case correction, not either entry-scope experiment.

The retail body SHA-256 is
`5734e37be2f919af2508a63eb7cb9f7e507ce3a3a4bb989c0db8ac7ea0d316c7`.
Unadjusted aligned mismatches rise from 413 to 418 as subsequent instructions
move; that count does not falsify the recovered source/control-flow fact.
The first bounds rejection remains non-exact, as do the orientation value's
register/conversion sequence and final class-default/flag setup. No exactness
or banking claim follows from this partial correction.

One bounded tail control remains independent of those entry experiments:
move the final switch's defined `return 1` fallback after the switch, keeping
the three case-local query returns. This keeps the zero-selector result and
all other inputs defined; it does not imitate the incidental register value
on the impossible above-three selector path. It tests whether ownership of
the fallback by the switch explains the premature third-case flag setup and
the extra default-slot constant. Use the retained diagonal-case source as
the base, and reject the tail spelling if it supplies no raw correction.

The fallback-after-switch trial emits identical allocated section bytes,
section sizes/alignment and ordered symbolic relocations to the retained
diagonal-case trial. It does not explain the third-case flag setup. Reject
the spelling and retain the explicit switch default. Object-wide differences
are source/debug metadata, not executable differences; raw symbol indices
alone are not a stable relocation comparison across these source files.

### Retained correction and verification

**Partial, 97.452630% -> 98.221054%; not banked.** The fresh canonical
object's allocated sections and ordered symbolic relocations equal the
independently compiled and completely audited diagonal-case trial. The
1900-byte retail body is still twelve bytes longer than the reconstruction.
The two-word bounds-return block, orientation conversion sequence and final
flag/default setup remain concrete differences; no compiler limitation is
inferred from them.

The resumed full `kf build` checks all three images and retains the known
non-green data/ownership/placement findings: PSX data 0/1 and target relink
1/1, GAME data 9/42 and relink 75/77, OPEN data 2/19 and relink 34/38.
There are no artifact failures. Ruff and `git diff --check` pass, and all
680 repository tests pass (84.364 seconds).

Comparison of all 484 reported function rows with the pre-control snapshot
finds only this function's score change. Exact counts remain GAME 313/362,
OPEN 98/108 and PSX 1/1; all thirteen vendored verification functions stay
exact. There are still 59 non-exact GAME/OPEN functions. No banked function,
baseline, unrelated source or configuration is changed by this correction.

## Remaining-CFG controls at `5f3eebf`

The 98.221054% source was refreshed with all six image-qualified views. A
positive conjunction branching to an in-bounds label and the equivalent pair
of nested positive guards both normalize to the existing bounds CFG: the two
failures still share the later negative-height return. They do not emit the
retail local bounds-rejection block and are not retained.

Retail also selects the oriented rectangle coordinate as a word before its
single halfword conversion. Modeling that as distinct `s32` selection and
`s16` conversion locals changes register allocation from the entry onward and
worsens the paired listing to 49.6%; it does not recover the local sequence.
The direct signed-halfword selection remains canonical. These controls leave
the existing twelve-byte deficit and one missing known return frontier
unattributed; no source, compiler profile, target referent or bank entry is
changed.
