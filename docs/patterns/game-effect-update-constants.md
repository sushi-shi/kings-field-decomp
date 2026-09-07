# Effect-update constants and typed boundaries

## Function Match Plan

Continue from `22a3c57` using the unchanged `probe-gcc257-o2-g0` probe.
Review all eight GAME effect-update helpers, their dispatcher and the radial
player-damage boundary. Saved image-specific disassembly/CFG, callers,
callees, strings, current matches, neighbors and source history precede edits.
Direct calls are proven; validated references retain their existing tier.
SDK matrix, random, trigonometry and audio bodies remain external.

| GAME VA / retail bytes | Function | Initial strict % | Evidence / intended change |
| --- | --- | --- | --- |
| 800166b4 / 304 | player_apply_radial_damage | 100 | Match shared declaration to existing typed definition; preserve unused base-power slot and eleven argument positions. |
| 80037fbc / 36 | effect_magic_power | 100 | Type flag selects player magic or fixed power five; align declaration/inventory with existing int return and effect pointer. |
| 80037fe0 / 696 | effect_projectile_update_3d | 100 | Rotated collision probe, signed pitch acceleration, sound cutoff and upward withdrawal; name parameters without changing yaw-tested negative clamp or zero-crossing order. |
| 80038298 / 608 | effect_projectile_update_2d | 99.934210 | Sine/cosine multiply radius, not speed; name orbit period, collision and sound distances, retain phase masks and signed limit consumption. |
| 800384f8 / 460 | effect_floor_deform_line | 100 | Five seven-byte segments; sound interval starts at progress 3900 and ends at 3900+abs(step), after clamp branches. |
| 800386c4 / 104 | effect_scatter_triple | 100 | Three halfword lanes receive existing [-64,63] jitter; expose existing typed declaration, retain lane indices. |
| 8003872c / 144 | effect_rotate_scale_offset_y | 100 | SDK SVECTOR input/VECTOR output; scale X/Z, force Y zero, rotate around signed-halfword yaw. |
| 800387bc / 248 | effect_spawn_trail_kind13 | 100 | Distance 800 gives Q12 unity scale; typed record pointer and angle; parent X/Z translation only. |
| 800388b4 / 388 | effect_spawn_ground_kind6 | 100 | Trigonometric placement 1500 units from parent, sample floor Y; fourth argument forwards branch role in constructor's sixth slot. |
| 80038a38 / 6156 | effect_update_dispatch | 96.939570 | Supply radius 6500 and inclusive phase limits 40/60; verify every call affected by newly visible prototypes. |

Before commit, force affected compiles and compare all 112 frozen before/after
units at identical paths, including allocated data, runtime symbols and ordered
relocations. Resolve all reviewed instruction words and referents; require
every exact body to remain retail-exact. Review any non-exact change from its
first divergence. Run inventory, modern checking, Ruff, existing tests,
whitespace and full build. Add no size assertions or permanent tests.

## Units and limits

The swing collision probe passes radius 120 world units and the orbit probe
passes radius 150. Swing velocity changes by ten angle units per update,
or about 0.879 degrees per update per update. Withdrawal subtracts 60 world
Y units per update. These names identify measured roles and units; retail
does not reveal why those collision sizes or acceleration were selected.

The dispatcher passes inclusive swing phase limits 40 and 60. Release begins
at phase 10, so an uninterrupted rise visits 31 or 51 phases respectively.
Neither value is a duration in seconds. The negative swing clamp still tests
yaw before assigning pitch, and release still requires a pitch zero crossing.
The orbit helper also receives limit 40, but its active release-request phase
does not itself transition to rising. Sharing a phase-limit constant would
obscure these distinct paths.

Orbit radius 6500 multiplies Q12 sine/cosine; the former name `speed` was
misleading. A full turn takes 64 active updates, advancing 64 angle units
(5.625 degrees) per update. The doubled vertical angle makes two vertical
cycles per horizontal turn; dividing sine by four gives a 1024-world-unit
vertical amplitude. The constructor's signed halfword X/Z centers still
restore 256-unit buckets. No update-frequency assumption converts these to
seconds or real-world speed.

Sound retries accept the lowest quarter of the SDK's 0..32767 random output
range, expressed as `(RAND_MAX + 1) / 4`. This states the threshold without
assuming uniform samples. Swing and orbit playback cutoffs are 3000 and 5000
world units, while both pass 14000 as the attenuation denominator. The audio
helper rejects at the cutoff; 14000 is not the audible radius. The separate
5000-unit orbit reset test measures from `player_state.camera_position`,
while playback uses the audio listener transform. Equal distances do not
establish equal origins, so their constants remain distinct.

Floor sound starts at Q12 progress 3900, about 95.2% of unity. The accepted
interval is `[3900, 3900 + abs(progress_step))`, reached only after the
negative/above-unity clamp branches. A zero step produces an empty interval;
unity itself may enter it. The original threshold tuning is unresolved.
Five authored seven-byte segments supply coordinates, byte-wrapped strides,
cell counts and endpoint heights; all 35 payload bytes remain unchanged.

The trail's distance argument yields unity Q12 scale at 800. This does not
normalize the input direction vector: a direction with length 900 remains
length 900 at unity scale. The helper forces its rotated offset Y to zero
and adds only the parent's X/Z. Ground branches instead place children 1500
world units from the parent in X/Z and sample their floor height. This
spacing is distinct from any equal-valued damage radius.

## Typed boundaries

Shared declarations now expose the helpers' existing `KfEffectRecord *`,
`SVECTOR *`, `VECTOR *`, halfword and word parameter types. Curated signatures
agree with their definitions; no function identity or evidence tier is
promoted. `effect_magic_power` retains its existing C `int` return, replacing
the stale inventory's `u16`/`u8 *` model. Its player-magic load alone does not
prove the historical return spelling. The ground helper's fourth word is
named `branch_role`; it forwards to the constructor's sixth argument, whose
consumer reads a halfword. Its current O32 parameter width remains unchanged.

The player radial-damage declaration retains all eleven argument positions,
including the unused base-power slot. The new declarations let the modern
compiler check these calls and eliminate exactly twenty undeclared-function
errors in `effect_update_dispatch`; they do not introduce blanket casts or
change SDK interfaces.

## Retained literals

This batch removes 25 inline numeric occurrences: 21 in `effect_update.c`
(126 to 105) and four in its dispatcher (261 to 257). The complete
[update ledger](game-effect-update-literal-ledger.md) accounts for all 105
retained occurrences, including 35 authored segment bytes. Representation
masks, coordinate/component array positions, sign boundaries and absent
damage channels retain explicit reasons. Named definitions and retail address
claims are excluded from these counts.

## Verification and verdict

All ten reviewed functions retain the strict scores in the plan. Their 2286
resolved instruction words, 101 calls and 34 ordered address references are
unchanged, including delay slots and raw constants. The eight exact functions
independently reproduce all 595 retail instruction words and the resolved
target-object referents. The orbit helper's first difference remains its
56-byte candidate frame versus 120 retail bytes. The dispatcher still first
differs at GAME 80038a68, `lui s5,0x800a` versus `lui s4,0x800a`. These two
residues remain unattributed; neither function is banked by this campaign.

All 112 frozen units compile at identical before/after paths with unchanged
allocated sections, runtime symbols and ordered relocations. Only update and
dispatcher debug-line metadata changes. Concurrent commit `d7e0260` changed
the renderer separately: actor rendering rose from 91.406010% to 94.751880%,
and map-object rendering from 95.604164% to 100%. Compiling that committed
renderer with both header versions also yields identical runtime contents
and agrees with the live object. The other 482 captured scores are unchanged.

The three affected units were forcibly compiled. All 680 current repository
tests pass (82.863 seconds), including the existing floor-deformation oracle
covering all five segments, clamp/sound boundaries and positive/negative/zero
steps. Inventory, Ruff and whitespace checks pass. Modern checking improves
from 64/112 to 65/112 passing variants and from 320 errors to 300; the exact
diagnostic difference consists of the twenty missing declarations described
above, with no new errors.

Full `kf build` retains its existing data/placement failures: data matches
PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77, OPEN 34/38;
six conflicting section bases and zero artifact failures. This is not a
passing full build. No permanent tests, size assertions, bank entries or
toolchain changes are introduced by this campaign.
