# Effect construction constants and remainder ledger

## Function Match Plan

Starting at `333f135`, review all four GAME effect-pool functions and propagate
shared values through the orbit helper and effect dispatcher. The unchanged
probe is `probe-gcc257-o2-g0`; this is not historical compiler attribution.
Preserve signatures, byte/halfword storage, variadic argument positions,
switch tables, references, delay slots and all phase/rounding behavior.

| GAME VA / retail bytes | Function | Strict % before/after | Snapshot / intended change |
| --- | --- | --- | --- |
| `80036f00 / 68` | `effect_pool_find_free` | 100.000000 | Forty-eight byte-tagged slots, decrementing halfword scan and null miss; ledger only. |
| `80036f44 / 2092` | `effect_pool_construct` | 100.000000 | Kind-minus-four switch and six/seven/eight argument slots; identify lightning impact/radial blast and branch visual children, authored orientation/scales, branch delay, emission depth, orbit quantization and extended sound ranges. |
| `80037770 / 172` | `effect_pool_spawn_typed` | 100.000000 | Specialized floor controller stores live type 0xf0, kind 52 and forward phase; name the complete type value without inventing individual high-bit flags. |
| `8003781c / 52` | `effect_pool_set_current` | 100.000000 | Select current record and derive its magic-row pointer; no literals or type changes. |
| `80038298 / 608` | `effect_projectile_update_2d` | 99.934210 | Signed halfword X/Z centers restore 256-world-unit buckets with <<8; share that quantization width with constructor writes. |
| `80038a38 / 6156` | `effect_update_dispatch` | 96.939570 | Lightning impact emits radial children; ground branches emit visual children; twenty 175-unit emergence steps undo constructor depth 3500. Propagate identities and emergence units. |

Image-specific address, disassembly/CFG, callers/callees, strings and match
snapshots precede editing. Source history and adjacent constructor, selector,
collision and magic helpers constrain ownership. Direct calls remain proven;
decoded references retain their validated tier and switch-table models are
checked against linked words. SDK audio/math implementations remain external.

Force the affected units, compare all 112 frozen before/after objects and
ordered relocations, then resolve every reviewed body. Require all four exact
bodies to remain retail-exact; keep and explain existing non-exact residues.
Run inventory, modern checking, Ruff, the existing suite, whitespace and full
build checks. No new assertions, tests, banking or compiler changes are planned.

## Effect identities

Lightning Bolt's collision or exhausted travel timer creates kind 32 at the
floor under the projectile. The constructor selects a three-frame billboard
sequence; updates at ages 3/5/7 create kind 33, an expanding model that applies
radial damage from Lightning Bolt's magic record. Thus the names distinguish
the impact emitter from its radial blasts. The emitter's age3 also triggers
the second sound slot. Neither name claims the original source symbol or an
independently identified visual shape.

The Fire Wall ground branch creates kind 34 during its sustained phase. Its
own branch applies damage; the child only grows/shrinks Y scale, rises and
rotates before freeing its slot. `GROUND_BRANCH_VISUAL` states that supported
relationship without naming the uninspected model's appearance.

The linked kind-minus-four tables independently reproduce these mappings:

| Kind | Constructor table slot -> destination | Dispatcher table slot -> destination |
| --- | --- | --- |
| 32, Lightning Impact | `80012c98 -> 80037124` | `80012d68 -> 80039974` |
| 33, Lightning Radial Blast | `80012c9c -> 80037170` | `80012d6c -> 80039a30` |
| 34, Ground Branch Visual | `80012ca0 -> 800371dc` | `80012d70 -> 80039df0` |

Input variants 41/42 retain their numeric identities but now assign the named
stored kinds 32/33. They choose alternate rendering resources 19/15; this alone
does not establish an enemy, weapon or color identity. Variants23/24/44 and
unresolved kinds 14/21/22/48 likewise keep explicit per-occurrence reasons in
the ledger. In particular, kind 22's damage path differs from Light Needle
despite similar constructor code. The current dispatcher has no update arm
for kinds 21 or48. Do not turn either absence into invented behavior.

The specialized floor constructor's complete type byte0xf0 is named separately
from kind 52 and phases0/1/2. It marks this live controller for the pool scan;
individual bits are not given unsupported new meanings. The current-record
selector still derives a magic-row pointer from the kind, including kinds
outside the 24 loaded records. No bounds correction is inferred by naming.

## Units and parameter relationships

| Parameter | Value and measured meaning |
| --- | --- |
| Wind Cutter pitch | 850 angle units, 74.70703125 degrees at 4096 units/turn. The exact authored orientation is preserved; no original conversion formula is proved. |
| Ground-branch propagation delay | Six countdown updates before the old value tests zero on the seventh visit. Preserve the postdecrement and completed-timer sentinel. |
| Ground-branch sound volume | 120 in the SDK's0..127 volume range; separate from spatial range and cue-slot index. |
| Emerging initial scale | 1500 Q12 units on all three axes, approximately 0.366 unity. |
| Short swinging hazard scale | 2600 Q12 units, approximately 0.635 unity. Its 2500-world-unit collision probe is an independent dimension. |
| Orbit scale | 2800 Q12 units, approximately 0.684 unity. It does not determine the orbit radius. |
| Homing X/Y scale | Exactly half Q12 unity; Z retains unity. The literal divisor two expresses that geometric ratio. |
| Extended sound cutoff | 20000 world units; the audio helper rejects distances greater than or equal to this bound. |
| Extended sound attenuation distance | 60000 world units; supplies the divisor for Q7 linear attenuation, not a second audible boundary. |

The extended sound pair is private to the constructor. Its five uses cover
Lightning Bolt, lightning impact, Moonlight and the two radial-blast render
variants. The downstream audio calculation is
`((attenuation_distance - distance) << 7) / attenuation_distance`, followed
by volume multiplication and panning. Thus the 20000 cutoff happens while
nominal gain is still about two thirds; do not rename60000 as the audible
range or rewrite the cutoff to make a continuous fade. The original tuning
choice remains unknown.

Emergence begins at phase 100 and moves upward through phase 119 inclusive:
twenty updates of 175 world units undo the constructor's downward3500 offset.
`KF_EFFECT_EMERGE_DEPTH` derives from that phase interval and the shared Y
step. Phase119 still writes the byte wrap value before the common increment;
the launch transition is unchanged. Naming this relationship explains3500
without claiming why175 was selected.

Orbit centers store signed X/Z coordinates shifted right by 8 and restore their
signed halfwords with a left shift by the same amount. They are quantized
256-world-unit buckets. Y uses a direct signed-halfword view. No rounding,
narrowing, signed shift or omitted low bits are changed; this is not a Q8
fractional-coordinate model.

Other authored scales/orientations receive names for their measured roles,
but no original designer rationale is asserted. Render selectors remain
resource ordinals until the corresponding billboard/model asset has an
independent identity; the two namespaces must not be conflated.

## Complete remainder and verification

The [pool ledger](game-effect-pool-literal-ledger.md) covers every retained
occurrence in the complete module: 207 in the general constructor, two in the
free-slot scan, two in the specialized floor constructor, and 15 in existing
alignment/offset witnesses. The current-record selector has none. Total 226,
down from 257. The dispatcher removes seven more inline occurrences and the
orbit helper two, for 40 overall. This per-occurrence ledger supersedes the
pool portion of the earlier [effect literal audit](game-effect-literals.md).
It retains authored resource ordinals, unresolved kind variants, neutral
vectors/scales, positional variadic indices, boolean flags, sound-slot
indices, ordinary loop/pointer tests, and the unexplained Moonlight control
bytes with a specific reason for each use.

All six reviewed functions retain their strict percentages, all 2287 candidate
instruction words, 86 calls and 37 ordered address references. All four pool
functions independently reproduce their combined 596 retail words and agree
with the resolved target objects. The orbit helper's first difference remains
its entry stack allocation, 56 candidate bytes versus 120 retail. The dispatcher
still first differs at GAME 80038a68, `lui s5,0x800a` versus `lui s4,0x800a`.
These unchanged non-exact residues remain unattributed and are not banked.

Frozen before/after builds compile all 112 units at identical paths. Every
runtime/data section, runtime symbol and ordered relocation is identical;
only the pool's debug line metadata changes. The live objects agree with
that control and all 484 captured strict scores are unchanged. The literal
ledger independently checks function/line/token/expression multiplicity.

All three affected units were forcibly compiled. Inventory, Ruff, whitespace
and all 679 repository tests pass (81.600 seconds). Modern checking retains
the same 320 errors and 64/112 passing variants. Full `kf build` retains the
existing data/placement failures: data PSX 0/1, GAME 9/42, OPEN 2/19; target
relinks PSX 1/1, GAME 75/77, OPEN 34/38; six conflicting section bases and zero
artifact failures. These results do not claim a passing full build. This
campaign adds no permanent tests, size assertions, bank entries or tooling.
