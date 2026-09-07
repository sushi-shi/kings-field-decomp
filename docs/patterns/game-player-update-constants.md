# Player update constants and remainder ledger

## Function Match Plan

Starting at `88d0ad1`, audit the complete GAME.EXE player-update module and
propagate recovered map/effect identities through their constructor, dispatcher,
interaction and rendering consumers. All five units use `probe-gcc257-o2-g0`;
that probe is not historical compiler attribution. Retain signatures, storage
widths, branches, delay slots, ordered calls and referents.

| GAME VA / retail bytes | Function | Strict % before/after | Evidence snapshot / intended change |
| --- | --- | --- | --- |
| `80018880 / 6684` | `player_update` | 96.945540 | Signed halfword motion integration, twelve-bit yaw, pitch step +/-10 and angle +/-191; byte weapon-magic countdown, halfword power gates, spell/effect selection, cone queries and three constructor calls; complete literal audit including initialized data. |
| `80034de4 / 2308` | `map_interaction_dispatch` | 83.436745 | Attribute cases 0x3a/0x3f/0x45 call notification rows 12/23/17; name pitfall, poison hole and hidden door from decoded text. |
| `80036f44 / 2092` | `effect_pool_construct` | 100.000000 | Kind 36 selects model 10, copies/inverts pitch, initializes two unresolved control bytes and plays magic row 18's second sound; name the Moonlight Sword projectile selector only. |
| `80038a38 / 6156` | `effect_update_dispatch` | 96.939570 | Matching kind-36 branch integrates its direction, grows Z scale, creates four trails and transitions on terrain collision to repeated radial blasts; propagate the selector. |
| `8001e5ec / 592` | `render_map_cell` | 96.743240 | Illusion Staff's authored remapping includes hidden-door attribute 0x45 to mesh selector 0x18; propagate only the independently identified attribute. |

Before editing, capture each function's image-specific address, retail CFG and
disassembly, incoming/outgoing references, strings and strict match, plus source
history and adjacent claims. Direct calls are proven; decoded address pairs
retain their validated tier. Switch-table destinations remain curated models;
an unresolved indirect successor is not promoted by this naming pass.

The initialized matrix at GAME `80055858` contains three repeated Q12 rows
`666,233,1333`, zero translation and alignment bytes. The eight SVECTOR rows at
`80055878` reproduce the source camera offsets, including each zero pad. The
input snapshot at `80057b30` begins at zero. Preserve the authored values and
document their roles without inventing original tuning explanations.

Verification will force all five affected units, compare frozen before/after
objects including runtime data and ordered relocations, resolve each reviewed
function against retail, run inventory/Ruff/existing tests/modern checking, and
run the full build. Every function requires a final verdict, including existing
non-exact results. No new size assertions or permanent tests are needed.

## Input and movement units

Forward input accelerates by limit/4 but decelerates by limit/8; strafe uses
limit/4 for both. Yaw also uses a quarter of its own limit for acceleration and
deceleration. These are signed shifts, preserving integer truncation. Normal
forward steps are therefore 45 while its braking steps are 22; slowed values
are 9 and 4. Normal yaw changes by 7 and slowed yaw by 1. Do not replace these
with interpolated fractions or claim that each reaches rest in exactly four
or eight updates.

Pitch step accelerates by 3 angle units/update and brakes by 2, with a signed
limit of +/-10. Camera pitch is limited to +/-191 angle units, approximately
16.787 degrees each way at 4096 units/turn. Retail tests the positive bounds
as >=11 and >=192, then stores 10 and 191; the named limits retain that +1
boundary form. These are observed integration and clamp values, not a recovered
explanation of why the original developers chose them.

## Weapon magic

The input windows remain those in the [player-core review](game-player-core-constants.md).
The byte delay is a countdown: zero disables it, one is ready, and values above
one decrement without casting. Flame Sword stores two after each shot, leaving
one intervening update; Colichemarde remains ready, with two shots per eligible
weapon phase. Triple Fang and Moonlight each start one cast and store three.
Cancellation empties the shot count and stores ready+1. This counter remains an
integer; it is not a discrete lifecycle field.

| Weapon | Requirement | Spawned kind | MP record | Stored delay |
| --- | --- | --- | --- | --- |
| Triple Fang | Physical and magic power each at least 80 | Homing projectile, 20 | 20 | 3 |
| Flame Sword | Fire Ball learned is nonzero | Fire Ball, 5 | 5 | 2 |
| Moonlight Sword | Physical and magic power each at least 80 | Moonlight projectile, 36 | Radial blast, 18 | 3 |
| Colichemarde | Physical and magic power each at least 60 | Light Needle, 8 | 8 | 1 |

Each weapon's power threshold has its own name; equal numbers do not prove a
shared balance rule. Every shot checks available MP, but only the last shot
subtracts the cost. The shots remaining still decrement when MP is insufficient.
The one stored for a full-charge swing is a consumed boolean snapshot, not the
full charge amount. None of these guards or update-order details is changed.

The local spawn offset is (200,200,400), transformed by the camera matrix and
added to camera position; the previous local name `scale` was misleading and
is now `spawn_offset`. The forward direction is scaled to 900 world units.
Triple Fang adds projectiles at Y offsets +/-300 and pitch offsets +/-64
(5.625 degrees). Subtracting twice each offset moves from the positive side to
the negative side while preserving the original mutation order and unchanged
direction argument.

Both target queries admit range 20000. Their angular tolerances are 0x155 and
0x555, approximately 29.97 and 119.97 degrees to either side of facing. The
query selects the smallest angular difference within range, not the nearest
actor. Non-final Fire Ball/Light Needle shots take the narrower query and
independent pitch/yaw jitter. The pinned SDK `RAND.H` defines RAND_MAX=32767:
`angle -= 4 - (rand() >> 9)` therefore adds -4..59 angle units, not symmetric
spread. Preserve the asymmetry and both calls. Their seventh constructor slot
uses shot-count parity: Light Needle consumes it as a launch-sound gate; Fire
Ball ignores it. Other branches pass an actor index or the homing-wander
selector. That mixed argument is not assigned one false enum domain.

The direct Moonlight Sword branch supplies kind 36 at GAME `8001953c`, while
its MP record address at `80019540` is row 18. The retail constructor switch
slot `80012ca8` resolves to `80037548`; dispatcher slot `80012d78` resolves to
`80039250`. Both use the kind-minus-four index. The constructor uses model 10
and row 18's second sound. The dispatcher grows the travelling projectile,
emits four trails, holds travel phase 10, and enters phase 20 on collision;
even phases then spawn radial blasts before expiry beyond 23. This chain
supports the weapon/projectile identity, without identifying the two control
bytes merely initialized to 0xff or inventing a visual shape.

## Map attributes and retained values

The [decoded notification table](game-notification-identities.md) identifies
pitfall (0x3a -> row 12), poison hole (0x3f -> row 23), and hidden door
(0x45 -> row 17). Player contact with the first supplies damage channels
5/3/5 and no status; the second supplies poison and no direct damage. The
hidden-door name also reaches Illusion Staff's map-mesh substitution case.
Neighboring mesh selectors 0x44/0x46 and replacement 0x18 are not renamed from
proximity alone. The previously identified bottomless-pit case stays shared.

The [player-update ledger](game-player-update-literal-ledger.md) covers all
168 retained numeric occurrences: 45 in initialized data and 123 in the body.
The module previously had 227; six further occurrences are named in its four
consumers, for 65 removed overall. Retained constants include authored camera
and color components, zero/sign tests, boolean and one-point vital updates,
index/count corrections, the unresolved mixed constructor id 10, and the
pitfall damage tuple whose individual channel identities remain unproven.
The existing [map-script ledger](game-map-script-literal-ledger.md) is refreshed
to remove all four now-named attribute cases, including the earlier bottomless
pit; it covers 272 remaining occurrences outside named definitions.

Retail PadRead ignores its caller's literal one and consults global
PadIdentifier instead. That ABI argument is retained without naming it as a
controller port. Its old unprototyped C declaration remains a separate modern
C++ checking issue; this constant pass does not change the SDK interface.

## Verification and final verdicts

All five reviewed functions retain their original strict percentages and
resolved bodies: 4442 candidate instruction words, 204 calls and 258 ordered
address references. The constructor remains exactly all 523 retail words and
agrees with its resolved target object. The other four remain non-exact and
are not newly banked. Their first differences are unchanged:

| Function | First differing GAME VA | Candidate / retail |
| --- | --- | --- |
| `player_update` | `80018880` | Stack allocation 216 / 224 bytes |
| `map_interaction_dispatch` | `80034de4` | Stack allocation 80 / 72 bytes |
| `effect_update_dispatch` | `80038a68` | `lui s5,0x800a` / `lui s4,0x800a` |
| `render_map_cell` | `8001e5ec` | Stack allocation 88 / 120 bytes |

These are unchanged, unattributed residues. No type, referent, CFG or delay
slot is distorted to conceal them. Frozen before/after builds of all 112 units
at identical paths preserve every runtime/data section, runtime symbol and
ordered relocation. Only player-update debug line metadata changes. The live
comparison agrees outside the independently edited entity renderer; all five
reviewed functions and their ordered references still match the frozen control.

All affected units were forcibly compiled. Inventory validation, Ruff,
whitespace checks and all 679 repository tests pass (83.974 seconds). The one
existing initializer parser was adjusted to accept a named array bound; its
payload digest, tuple order and SDK-lane checks remain intact. No new test or
size assertion was added by this campaign. The final suite also includes an
independently added graphics-owner test.

Modern checking retains the same 320 errors and 64/112 passing variants.
Full `kf build` still reports existing data and placement failures: data
PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77,
OPEN 34/38; six conflicting section bases and zero artifact failures.
This is not a passing full build. The complete player-update ledger is checked
against each function/line/token/expression occurrence and its multiplicity;
the refreshed map-script ledger's function/token totals also agree exactly.
