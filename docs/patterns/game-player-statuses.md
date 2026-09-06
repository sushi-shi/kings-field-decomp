# GAME remaining player statuses

## Function Match Plan

Start at 9a0c365. Name the remaining four numbered player timers, their
single-bit flags and the fire-defense application helper. The retail menu
atlas gives glyph c8=呪 (curse) for mask 1 and c7=暗 (darkness) for mask 2.
Mask 8 selects glyph c9 and directly lowers movement/turn limits; `slowed`
names that proved behavior without claiming immobilization. Mask 16 supplies
+10 to the recently identified fire_defense field. It has no corresponding
entry in the four-condition text/HUD priority chain.

Use the existing player aggregate's signed halfwords: curse +48, darkness
+4a, slowed +4e, fire-defense +50. Rename private MATRIX at GAME 80055858 to
player_darkness_color_matrix, preserving its 32 bytes, ownership and private
scope. Rename GAME 80018858 to player_apply_fire_defense_boost with its
unchanged void(void) interface; preserve its unit/claim and the caller's
redundant flag set. Shared function/data inventories must use these names.

Fresh image-qualified disassembly/CFG, xrefs, callees, strings, match state
and source history cover the nine functions below. Existing full player,
poison, menu and equipment dossiers supply the adjacent functions and SDK
boundaries; the small application leaf and its direct call at 80023600 are
checked explicitly. These are game status policies and UI/lighting consumers,
not vendored bodies. The RNG, matrix and pad functions remain SDK boundaries.

| GAME address | Function | Starting strict score | Evidence / preserved behavior |
| --- | --- | --- | --- |
| 800151cc | game_state_initialize | 100% | All four signed timers start at -1; preserve initialization order. |
| 80015714 | player_recalculate_combat_stats | 100% | Curse subtracts 20 physical power and clamps at zero; mask 16 adds 10 fire defense. |
| 80016324 | player_apply_damage | 100% | Sets curse/darkness/slowed flags and timers; accessory 49 blocks darkness. |
| 80018858 | player_apply_fire_defense_boost | 100% | Ten-word leaf stores timer 500, sets mask 16, and returns with its nop slot. |
| 80018880 | player_update | 96.945540% | Movement limits, darkness fade/fog and four distinct countdown lifecycles. |
| 8001fde4 | render_frame | 100% | Existing HUD priority masks 1,2,4,8. |
| 8002317c | menu_magic_panel | 95.897590% | Spell selection 2 sets mask 16 and calls the helper; other cure masks stay literal. |
| 80025f38 | menu_draw_stats_header | 97.686110% | Status-mask to authored-glyph mapping. |
| 800264d8 | menu_draw_status_details | 95.110700% | Same mapping; preserve every numeric field and label. |

Introduce constants for duration/refresh values, curse penalty, fire-defense
bonus, movement/turn limits, darkness fade intervals and fog endpoints. Share
-1 as KF_PLAYER_STATUS_TIMER_INACTIVE, including poison's existing uses.
Do not generalize the different timer CFGs or reinterpret their endpoints.
Rebuild all affected units. Require all 484 function scores unchanged and
all object sections unchanged except debug line tables and symbol-name
metadata explained exactly by the two function/global substitutions. Normalize
only those names in the strict report; keep all symbol values, sizes,
bindings, sections, relocation kinds, indices and numeric targets unchanged.
Require raw retail equality for the five exact affected functions and compare
both darkness references and its initializer. Keep every bank score/hash;
only the already banked leaf's displayed name changes. Run lint, repository
tests, diff checks and full build before committing.

## Meanings, units and timing

The [retail menu atlas](game-menu-glyph-render.md#retail-glyph-format-and-asset-evidence)
is decoded from hash-checked MIX.TIM using the STAT.DAT glyph descriptor.
Both text panels select c8 for mask 1, c7 for mask 2, 88 for poison and c9
for mask 8. Curse also has the independent physical-power penalty, and
darkness changes the color matrix and fog. The slowed name is based on the
actual movement controls; the authored glyph does not justify saying the
player cannot move.

| Constant / value | Meaning and reason |
| --- | --- |
| Status masks 1,2,8,16 | Independent curse, darkness, slowed and fire-defense flags; use 1 shifted by the observed bit index. Poison remains mask 4. |
| Inactive timer -1 | Signed sentinel used by these status blocks and poison; distinct from active zero endpoints. |
| Curse timer 600 | Initial countdown and first-update stat-recalculation marker. Its tuning rationale is unknown. |
| Darkness timer 1000 | Initial countdown; the initial fade uses 32 timer values, while the return visits 32 down to 0 (33 values / 32 intervals). Total duration tuning is unknown. |
| Darkness refresh minimum 970 | Reapplication raises a smaller active timer to this value; -1 takes the full 1000 path. Preserve the existing threshold, whose exact tuning is unknown. |
| Slowed timer 300 | Initial countdown, decremented until -1. Its duration tuning is unknown. |
| Fire-defense timer 500 | Initial countdown and first-update stat-recalculation marker. Its duration tuning is unknown. |
| Curse penalty 20 | Physical-power subtraction, clamped to zero. Magnitude tuning is unknown. |
| Fire-defense bonus 10 | Added to the named fire_defense field, independently of other defense components. Magnitude tuning is unknown. |
| Normal movement limit 180; slowed 36 | The slowed limit is exactly one fifth of normal. Movement acceleration still uses the existing limit/4 step. The base limit and slowdown ratio are observed tuning, not recovered design intent. |
| Normal turn limit 28; slowed 5 | Separate limits; preserve 5, which is not the exact 28/5 ratio. Both are tuned values. |
| Fade bits 5 / steps 32 | 32 interpolation intervals. Q12 factor uses shift 12-5=7, so each fade step is 128/4096. |
| Darkness threshold 33 | Fade length plus one: clearing the flag above the ending fade clamps timer to 32 before decrement. |
| Darkness offset 968 | Initial 1000 minus fade length 32; no independent threshold is invented. |
| Fog near endpoints 5000 and 11000 | Existing darkness and normal inputs to the fog helpers, interpolated by the same Q12 fade. Their exact tuning is unknown. |
| Matrix coefficients 666,233,1333 repeated in three rows | Authored SDK MATRIX coefficients, preserved as data; original selection is unknown. Its three translation zeros are an untranslated color transform. |
| Glyph indices c8,c7,c9 | Authored atlas indices; keep them literal rather than invent another character encoding. |
| Loop/condition zero, decrement one | Empty/inactive tests, clamps and single countdown updates. These arithmetic identities remain literal. |
| Cure masks 3, 0xc and other composite masks | Existing retained-bit policies, including their upper-bit behavior. Do not replace them with one status's complement. |
| Accessory 49 and spell selection 2 | Existing item/spell identifiers establishing darkness immunity and fire-defense application. This pass does not invent their retail names. |

Darkness decrements before computing the fade. From 1000, timer 999 gives
31/32 normal color, timer 968 gives the darkness matrix, the middle holds
that matrix, and timer 32 begins the return fade at factor zero. The return includes 33 values from 32 through zero across 32 intervals.
Timer zero gives the full normal matrix; the next update reaches -1 and clears the flag.
An externally cleared flag with timer at least 33 first clamps to 32, then
decrements to 31, preserving a gradual return rather than an immediate jump.
The initial 1000 and refresh 970 paths remain distinct.

Curse recalculates at its initial 600 value, on expiry at zero, and when
another operation clears the flag; the latter path sets zero before the
unconditional decrement to -1. Slowed decrements through zero and clears at
-1; its movement limits were already selected earlier in that update.
Fire defense recalculates at 500 and zero, applies its lighting preset and
then decrements. That timer block does not test the flag, so a separately
cleared flag does not automatically cancel the timer. Keep this behavior.
These counts describe status-block executions, not assumed seconds or display
frame rates; early exits, reapplication and other state changes matter.

## Final verdict

All nine function scores in the plan remain unchanged, as do the other 475
rows. All 112 source objects preserve every section after exact normalization
of player_apply_fire_defense_boost and player_darkness_color_matrix. Only
player-death/player-update debug line tables otherwise differ; the leaf's
.debug_str changes solely by its function name. Symbol values, sizes,
bindings and section membership are unchanged. The complete strict report
also agrees after exactly those two symbol substitutions.

The source-value control preserves every statement, literal value, type and
width across seven C files, the shared header and two existing tests after
the declared names and constant expressions are normalized. Four field rows
change names/evidence with all layouts and ordering preserved. Exactly one
function identity and one data identity/census row change; their addresses,
extents, types, linkage and ABI remain intact. All relocation rows and bank
scores/hashes agree after only the intended target/name substitutions.

Independent raw comparisons reproduce 1,266 retail words: initialization
185, combat stats 517, damage application 228, fire-defense application 10
and frame rendering 326. Their ordered call/data counts remain 3/77,
3/130, 6/24, 0/3 and 20/64. Both darkness matrix initializers retain all
32 bytes. The partial update/menu bodies preserve every instruction and
relocation; no non-exact result is called exact.

Ruff and diff checks pass; all 651 tests pass in 82.585 seconds with nine
skips on the final fade-step naming. Full `kf build` retains the existing
ownership/placement failures: 7/60 source-data units exact; target relink
PSX 1/1, GAME 75/77 and OPEN 34/38; two GAME/four OPEN section-base conflicts;
zero artifact failures. Post-full-build section, score, raw, source-value
and curated-identity controls pass. No new result is banked, and no tooling
implementation or flake changes. The earlier test pass before the final
FADE_STEPS wording also passed, but is not used as the final verification.
