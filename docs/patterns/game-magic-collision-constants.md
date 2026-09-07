# Magic targeting and effect-collision constants

## Function Match Plan

Start at `da55f25`, with hash-validated retail initialized and the unchanged
`probe-gcc257-o2-g0` probe. Review the complete GAME magic and effect-collision
modules. Image-specific address, disassembly/CFG, callers/callees, strings,
match state and source history precede edits. The current effect context,
player casting call, target query and constructor connect this campaign;
SDK bodies remain external. Direct calls are proven and validated referents
retain their existing tier.

| GAME VA / retail bytes | Function | Initial strict % | Evidence / intended change |
| --- | --- | --- | --- |
| 80037850 / 1900 | effect_map_collision | 97.452630 | Attribute scalar selects signed height or one of seven four-halfword rectangles; name shared table capacity, preserve all stored bounds and cover every literal. |
| 8003a244 / 48 | effect_pool_reset | 100 | Forty-eight tagged slots; retain initial loop ordinal with a specific ledger reason. |
| 8003a274 / 44 | magic_load_records | 100 | Copy the existing magic owner by words; retain zero exhaustion and existing sizeof-derived count. |
| 8003a2a0 / 1216 | magic_cast | 98.891450 | Player-view launch offset, cone range, spell-dependent speed, Lightning pitch/countdown/height choices and Fire Wall placement. Rename speed local and reuse the existing player-damage unity multiplier. |
| 8003a760 / 124 | effect_pool_sweep | 100 | Select each live effect's context before dispatch; retain countdown arithmetic and visitation order. |

Preserve the Lightning attribute-minus-one lookup, threshold -4999, signed
height additions, division order, variadic argument positions and all
delay slots. Preserve reversed authored rectangle bounds and the collision
fallback's raw return 1 without inventing a collision identity. No bounds
guard, phase change, signature change or reordered source is planned.

Force affected compiles, compare all 112 frozen before/after units at
identical paths, resolve every reviewed instruction/call/address reference
and require all three exact bodies to remain retail-exact. Record first
divergences for both non-exact bodies. Run inventory, modern checking, Ruff,
existing tests, whitespace and full build before commit. Add no permanent
tests, size assertions, bank entries or toolchain changes.

## Magic launch units and argument meanings

The view-relative launch offset is (-200, 200, 400) world units, rotated by
the player camera transform before adding camera position. The cone query
range is 20000 world units and retains the existing 341-angle-unit aim
tolerance. The query chooses the smallest angular difference, not simply
the nearest actor, and writes distance zero on a miss. These measured roles
do not establish the designers' reasons for their chosen values.

The local formerly named `scale` is now `speed`: it scales a Q12 unit forward
vector into per-update motion. Fire Ball and Light Needle use 600 world
units/update; Lightning and Wind Cutter each use 800. Lightning and Wind
Cutter retain distinct names despite their equal speeds. The direction
components still undergo the helper's fixed-point rounding and halfword
stores. No update frequency or exact Euclidean vector length is assumed.

Untargeted Lightning sets pitch -128 angle units (-11.25 degrees) and a
twenty-update travel countdown. Targeted Lightning derives pitch from the
vertical difference, then replaces distance with its signed quotient by
speed 800. That quotient is passed in the constructor's sixth slot and read
as a halfword countdown. Collision can end travel earlier. The update moves
before decrementing the countdown, so a zero quotient wraps on the first
decrement rather than producing immediate expiry; no minimum is introduced.

The Lightning height decision reads `attribute_table[attribute - 1]`. Retail
GAME 8003a440..8003a44c uses the table address minus two plus twice the byte
attribute; this differs from the collision helper's direct attribute index.
The comparison at 8003a454 is against -4999. Values at least -4999 select
the +3000 world-Y offset, lower values the +5000 offset. The names describe
the comparison and offsets without attributing them to an unproved floor,
room or spell trajectory design. The lookup bias and all signed arithmetic
remain unchanged.

All four constructor calls now reuse `KF_PLAYER_DAMAGE_MULTIPLIER_ONE` for
their first argument, ten in the downstream player's tenths-based multiplier
domain. This is separate from selecting the player-magic power flag and
collision target bits. For Light Needle, the sixth argument is a rotation
pointer; the other projectile call supplies the distance/countdown word.
Both seventh arguments request sound, although Fire Ball's constructor plays
its launch sound unconditionally. Fire Ball and Wind Cutter do not use
that sixth argument as a travel countdown. Fire Wall instead passes the
named root branch role in its sixth slot.

Targeted Fire Wall starts at the selected actor position. Without a target,
it projects 6000 world units, expressed as three map tiles, along camera yaw
and samples the destination floor height. Integer trigonometric rounding,
cell division and the original lack of an extra bounds guard are preserved.

## Collision geometry data

`KF_MAP_CELL_HEIGHT_RECORD_COUNT` names the seven-record capacity in both
definition and shared declaration. The table is the existing 56-byte
load-image owner at GAME 80055ab8. Each record has the ordered signed-halfword
fields `x_min, y_min, x_max, y_max`; all 28 initialized values stay literal
geometry data with a per-field reason in the ledger.

The attribute table's nonnegative entries cover indices 0..6; negative
entries are floor-relative scalar heights. For a rectangle, the orientation
selects local X/Z or its reflection around the tile extent, while vertical
bounds are offsets from the current floor. Both interval tests are inclusive.
Rows 4..6 store y_min=-10000 and y_max=-15000, so their vertical intervals
cannot pass the current ordered comparisons. They remain exactly as stored;
the rectangle branch then falls through to the grid-shape path.

The blocked-cell shape uses repeated one-cell neighbor offsets with its
original short-circuit order. The final target selector dispatches actor,
player or both queries with height zero and separate exclusion flags. When
the low selector bits are zero, retail returns raw 1. It is neither the
no-collision sentinel nor an actor/player class result; its original identity
is unresolved, so the ledger retains it without naming it boolean success.

## Retained literal coverage

The batch removes 22 inline numeric occurrences: magic 30 to 9, collision
44 to 43. Thirteen private casting definitions and one shared table capacity
name the parameters; four call sites reuse the existing multiplier constant.
The complete [two-module ledger](game-magic-collision-literal-ledger.md)
accounts for all 52 retained uses: 28 rectangle data values, 15 collision
body literals and nine across the four magic/reset/load/sweep functions.
Named definitions and retail address claims are excluded. Null/exhaustion
values, direct loop/neighbor arithmetic, sound requests and the biased lookup
have separate explanations rather than generic “magic number” exemptions.

## Verification and verdict

All five reviewed functions retain their strict scores. Their combined 824
candidate instruction words, sixteen calls and 39 ordered address references
are unchanged. The three exact reset/load/sweep functions independently
reproduce their combined 54 retail words and resolved target-object referents.

Effect collision remains 97.452630%, with 469 candidate words versus 475
retail words. Its first difference is GAME 800378e4: candidate `beqz v0`
targets 800379a4, retail targets 800378fc. Magic casting remains 98.891450%,
with 301 candidate words versus 304 retail; its first difference is GAME
8003a2c0, where candidate `bnez v0` targets 8003a740 rather than retail
8003a74c. These preexisting layout/control-flow residues are unchanged and
remain unattributed. Neither function is banked by this campaign.

All 112 frozen before/after units have identical allocated sections, runtime
symbols and ordered relocations, and the live objects agree with that
control. Only the magic module's debug-line metadata changes. All 484
captured strict scores are unchanged. The complete two-module ledger checks
function/line/token/expression multiplicity independently.

Both affected units were forcibly compiled. All 680 repository tests pass
(82.248 seconds), including the existing collision oracle over all seven
rectangles, orientations and inclusive-boundary samples, plus compiled and
retail data-payload checks. Inventory, Ruff and whitespace pass. Modern
checking retains the same 300-error diagnostic multiset and 65/112 passing
variants. Full `kf build` retains existing data/placement failures: data
matches PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77,
OPEN 34/38; six conflicting section bases and zero artifact failures. This
does not claim a passing full build or completion of the broader naming goal.
