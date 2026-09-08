# Effect dispatcher constants and remainder ledger

## Function Match Plan

Start at `29d1ddb`, with freshly initialized hash-validated retail and the
unchanged `probe-gcc257-o2-g0` probe. The campaign covers GAME
`effect_update_dispatch`, VA 80038a38, retail body 6156 bytes, current strict
match 96.939570%. Its shared effect/magic records, constructor, update helpers,
collision and damage callees establish the related family. The eight-byte
probe vectors are the dispatcher's existing 16-byte data claim; its existing
49-entry kind-minus-four switch table is the 196-byte rodata claim.

Fresh address, disassembly/CFG, caller/callee, string and match snapshots,
adjacent pool/helper bodies and source history precede editing. Direct calls
are proven; validated referents remain validated. Switch reachability gaps in
the semantic model are not promoted by source interpretation. No SDK bodies
or new ownership claims are introduced.

Name the remaining tuning parameters and phase boundaries across the whole
dispatcher: projectile collision/spin/retirement, scatter generation/pulse,
Moonlight/trail coordination, ordinary and lightning radial expansion,
homing randomness/steering, ground-branch propagation/visuals and actor-spawner
timing/geometry. Name the two authored swing-probe lengths. Keep distinct
units and equal-valued parameters with different consumers separate. Preserve
array/resource ordinals without independent identities and document every
retained occurrence, including initializers.

The raw body distinguishes old and preincremented radial phases, stores
branch hold counters before their common increment, and performs two separate
conditional actor-selection random draws. Preserve those orders, unsigned
storage, signed scale tests, short-circuit RNG order and all delay slots.
The player-homing arm's preexisting uninitialized distance read is retained;
this campaign does not invent its value or historical source explanation.

Force compilation, compare all 112 frozen before/after units at the same
paths, and resolve every dispatcher word/call/address referent. Inspect the
first remaining difference from retail and require no banked regression.
Run inventory, modern checking, Ruff, existing repository tests, whitespace
and full build before commit. No new tests, size assertions, bank entries,
toolchain changes or synthetic matching code are planned.

## Named units

The names describe decoded roles and units, not recovered original symbols
or design intent. No retail evidence explains why the designers chose the
particular collision sizes, speeds, offsets, spread or timings. Angle steps
use 4096 units per turn; scale steps use Q12 unless explicitly stated.
An update is one dispatcher invocation, without assuming a frame rate.

| Consumer | Named values and measured meaning |
| --- | --- |
| Projectile collision | Default radius 100 world units, shared by Moonlight; emerging radius 150. These arguments reach the collision helper after its separate map-geometry checks. |
| Projectile roll | Fire Ball/Darkness and Lightning each add 200 angle units (17.578125 degrees) per movement update; emerging adds 100 (8.7890625 degrees), the default spinning path 600 (52.734375 degrees). Equal steps on distinct paths retain distinct names. |
| Impact/dissipation | Fire Ball impact ends at exclusive phase 5; the shared invalidation interval ends at 10. The later dissipation interval subtracts 400 Q12 scale units. Lightning's ordinary 50..59 interval performs ten subtractions, leaving 96 from unity, then retires at 60. |
| Emerging retirement | Falling adds 20 world units/update to signed-halfword velocity before moving, and retires below floor+3000. Before wrapping, n steps from zero velocity displace by 10*n*(n+1); no fixed fall duration is implied. |
| Status projectile | Shrinking subtracts 384 Q12 units from Y/Z while leaving X unchanged; a negative signed-halfword Y scale retires it. Starting at unity, eleven steps cross that bound. |
| Scatter | Intermediate and final generation countdowns are 5/15 updates. A shift of 9 advances the pulse argument by 512 angle units per countdown step, eight samples per turn. The extra shift beyond Q12 halves pulse amplitude. The three-quarters descendant scale stays explicit arithmetic. |
| Kind 14 sound | RNG cutoff 8192 selects the lowest quarter of SDK outputs, playback cutoff 4000 world units and attenuation distance 12000. The kind itself remains unidentified; the names preserve its numeric owner rather than inventing a spell. |
| Moonlight | Travel/hold last phase 10, trail emission phase 2, impact first phase 20 and last retained phase 23. Z-scale grows by Q12/4 and clamps at signed-halfword maximum 32767. Near/far trail arguments are angles 1774/1824 (155.91796875/160.3125 degrees), supplied with both signs, and distance parameters 4000/8000. |
| Ground trail | Parent impact threshold is the same Moonlight phase 20. Shrink step 400 Q12 units and a three-entry render cycle belong to this child, separately from shared projectile dissipation. |
| Ordinary radial blast | Exclusive preincremented phase limit 13; Q12/4 scale growth and radius old_phase*333 world units. Player radial damage scale 5000 is Q12, about 1.220703125, unlike the actor API's separately named unity value 5000. |
| Homing randomization | Pitch `(rand>>6)-128` yields -128..383; yaw `(rand>>3)-512` yields -512..3583 before angular wrap. Wander cutoff 3276 is explicit, without assuming uniform or independent random outputs. |
| Homing steering | Initial phase last 4, tracking counter assignment 20; player aim Y offset 800 world units, actor cone range 20000 and tolerance 1365 angle units (119.970703125 degrees). Pitch/yaw approach limit is 4096/64 per update; forward motion 650 world units and roll step 256 angle units. |
| Lightning impact | Last phase 9, three-entry render cycle, child emissions at phases 3/5/7; phase 3 also plays sound. |
| Lightning radial blast | Last phase 7; Q12 growth 3583 per update, yaw step 1300 angle units and radius old_phase*1000 world units. The irregular 3583 is preserved exactly, not rounded to a convenient fraction. |
| Ground branch | Growth ends at 16, shrink begins at 48 and lifetime ends at 64; Q12/16 Y-scale step, damage period four updates, damage radius 1500 world units and yaw step 500 angle units. Hold bases 16/26/36 are assigned before the common increment. |
| Ground visual placement | Angle uses the top twelve bits of a 15-bit RNG output. Radius uses `(rand*325)>>13`, equivalently `floor(rand*1300/32768)`, producing 0..1299 world units. The named multiplier/shift retain the exact evaluated expression. |
| Ground visual motion | Four growth updates then four shrink updates, Q12 scale step 4000; shrinking also rises 1800 world units per update. Yaw step 500 remains distinct from the parent branch's equal step. |
| Actor spawner | Grow before phase 17, travel through phase 40, waiting assignment 117, create at 132, shrink from 149 and retire at 165. Q12 scale step 220, movement collision radius 1000, actor Y offset +800, random selection cutoff 3000. Yaw/pitch/roll steps are 1700/1400/1900 angle units. |
| Swing probes | Local Y offsets 2500/4900 world units become named short/long probe lengths. They remain independent of the constructor's render scales; local X/Z and SDK pad halfwords stay zero. |

## Counter and call ordering

The Moonlight impact test emits on even offsets before checking whether the
old phase exceeds 23. Thus ordinary progression emits at 20, 22 **and 24**;
the final call occurs on the retirement update. Phase 10 holds while moving
until collision sets phase 20. A trail compares its indexed parent's phase
against the same threshold; this campaign adds no new parent-liveness rule.

Ordinary radial damage tests the incremented phase but computes radius from
the saved old phase. Twelve growth updates are accepted, with damage on new
phases 1/3/5/7/9/11 using radii 0/666/1332/1998/2664/3330. Lightning radial
damage instead tests the old odd phase and ignores height. These paths share
the player's Q12 multiplier but retain different phase and radius rules.

At ground-branch growth phase 15, root/side/leaf roles assign hold bases
16/26/36, then the common increment leaves 17/27/37 for the next invocation.
The bases are not the first observed hold phases. Damage/visual emission
occurs at phases divisible by four below 48, so the remaining active windows
differ by role. Shrinking occurs at 48..63. Even retirement still advances
phase and yaw on the branch and visual paths.

The actor spawner initially grows seventeen times to Q12 scale 3740 and
later shrinks sixteen times to 220 before retirement; no symmetric fade is
invented. Before the final travel phase, a zero movement counter assigns 117
then the common increment leaves 118. At old phase 40, collision or exhaustion
assigns 117 and the special hold branch skips that increment. Otherwise phase
40 continues movement and decrements the counter while remaining phase 40.
Creation at 132 remains independent of those two waiting-entry values.

Actor selection makes one random draw for definition slot 2, a second draw
only if the first fails for slot 4, otherwise selects slot 0. These are local
floor-definition ordinals, not established global creature identities. Kind
14 also preserves its random-call order: `rand()` executes before checking
the sound-acceptance flag, even if sound is already accepted. Homing wander
can jump back to randomization and bypass the assignment of phase 20; the
ordinary tracking path assigns 20 then increments to 21. No uniform RNG,
new bounds guard, initialized distance or simplified lifecycle is assumed.

## Retained literal coverage

This campaign introduces 89 private named definitions and removes 104 inline
numeric occurrences, from 257 to 153. The complete
[remainder ledger](game-effect-dispatch-literal-ledger.md) covers six probe
initializer zeros and 147 function uses, with exact multiplicity by
function/line/token/expression. Named definitions and retail claims are
excluded. Retained values describe representation widths, direct fractions,
sign/exhaustion boundaries, resource ordinals, positional damage/sound slots
and boolean constructor options. Every retained unidentified kind or asset
has its specific known behavior and unresolved identity recorded.

## Verification and verdict

The dispatcher remains at strict 96.939570%. All 1539 resolved instruction
words, 69 calls and 21 ordered address references are unchanged, including
the named raw constants and delay slots. The first retail divergence remains
GAME 80038a68: candidate `lui s5,0x800a` versus retail `lui s4,0x800a`. This
existing residue remains unattributed; the function is not banked.

All 112 frozen before/after units have identical allocated sections, runtime
symbols and ordered relocations. Only the dispatcher's debug-line metadata
changes. Live objects agree with that control, and all 484 captured strict
scores are unchanged. The per-occurrence ledger independently checks source
coverage and multiplicity.

After forcing the affected compile, all 680 repository tests pass (83.290
seconds), together with inventory, Ruff and whitespace checks. Modern type
checking remains at 65/112 passing variants and 300 errors; the diagnostic
multiset is unchanged. Full `kf build` retains the existing data/placement
failures: data matches PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1,
GAME 75/77, OPEN 34/38; six conflicting section bases, zero artifact failures.
This does not claim a passing full build or completion of the wider naming
goal. No permanent tests or size assertions were added.

## Semantic label follow-up

Plan for `effect_update_dispatch` (GAME.EXE `0x80038a38`, extent `0x180c`):
replace the four remaining numeric effect labels with lightning-impact and
actor-spawner operation names. The current cases, callers and cached retail
CFG support those identities; the spawner scale, age and rotation blocks are
described above. This changes label spellings only, retaining every jump target,
expression and literal. Post-edit compiler/match verification remains deferred
with the naming pass.
