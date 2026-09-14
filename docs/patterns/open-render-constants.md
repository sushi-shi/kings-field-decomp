# OPEN renderer constants and shared scene model IDs

The later [OPEN model-domain review](open-model-domain.md) types the selector,
identifies the six formerly raw renderer IDs from resources, and refines the
ending disk/starfield names. The account below records the earlier campaign;
the [retained-literal ledger](open-render-literal-ledger.md) tracks current source.

## Function Match Plan

Baseline `cd679e4`, hash-verified OPEN retail and unchanged `probe-gcc257-o2-g0`.
Fourteen function snapshots include all six semantic views, history, adjacent
render helpers, callers and current strict scores. Game scene orchestration,
authored entity models, item descriptors and the ending gradient establish
game ownership; SDK GTE and primitive helpers remain external library calls.

Share the six already evidenced scene-role model names between scene scripts
and the entity renderer. Name per-render yaw step, selected-model depth bias,
ending model depth biases, floor-item brightness and ending-gradient OT slot.
Propagate existing projection, angle-wrap, Q12 and cell-visibility constants.
Preserve storage/signatures, narrowing, branch/call order, return and load
delay slots, switch ownership and ordered relocation targets. IDs with no
scene-role identity remain authored ordinals; no visual/story name is inferred.

Freeze all 112 source/image variants and 484 scores, force the five captured
units, compare isolated before/after runtime sections/symbols/relocations and
resolve every reviewed function's words/referents. Recheck exact functions
against retail and target objects, initialized entity data and the switch
pointer rows. Provide a full literal ledger for the four renderer files;
scene scripts only move already named constants and retain their prior audit.
Run modern checking, inventory, Ruff, existing tests, whitespace and full
`kf build`. No tests or size assertions are added; no new exact result is
expected or banked.

| OPEN VA / bytes | Function | Initial strict % | Evidence and preserved contract |
| --- | --- | ---: | --- |
| 0x800137c4 / 64 | `opening_render_frame` | 100.000000 | Two borrowed SDK-vector pointers; five ordered calls; projection 200 in the SetGeomScreen delay slot. |
| 0x80013804 / 408 | `sprite_add_g4` | 100.000000 | Halfword rectangle plus four byte RGB pointers (fifth argument on caller stack); authored ending gradient, OT index 0x3fef. |
| 0x80014268 / 372 | `opening_scene0_run` | 100.000000 | IDs 11/12 passed to the entity finder; opposite signed yaw updates establish existing scene-role names. |
| 0x800143dc / 384 | `opening_scene1_draw_fade` | 100.000000 | Byte shade and authored panels; unchanged shared-header control. |
| 0x8001455c / 172 | `opening_scene1_run` | 100.000000 | Scene fade/hold caller; unchanged shared-header control. |
| 0x80014608 / 508 | `opening_entity_transition` | 99.921260 | Signed mode and position pointer; pool creation/scaling and frame call; unchanged shared-header control. |
| 0x80014804 / 816 | `opening_scene3_run` | 99.931370 | IDs 13/14 passed to finder and rotated oppositely; propagate existing role names. |
| 0x80014b34 / 756 | `opening_ending_scene_run` | 100.000000 | Reuses the scene-3 pair IDs; scene wave/light/audio sequence remains unchanged. |
| 0x80014e28 / 1944 | `opening_ending_scroll_run` | 97.129630 | IDs 26/27, translation until hide/show swap and later roll; sole G4 caller; propagate existing role names. |
| 0x80018ecc / 552 | `opening_entity_render` | 100.000000 | Byte model ID; signed rotations/scales; 17-entry switch table; yaw increments occur after transform; signed depth bias. |
| 0x800190f4 / 332 | `render_floor_item` | 98.795180 | Byte facing/frame packing and increment/narrow/compare order; unchanged full-module control. |
| 0x80019240 / 664 | `opening_render_entities_and_items` | 98.885544 | Halfword-wrapped window bounds, signed item coordinates and countdowns; tpage member lifetime, named visibility class and RGB180. |
| 0x800194d8 / 72 | `opening_scene0_render_frame` | 100.000000 | Two borrowed SDK-vector pointers; six ordered calls; projection 200. |
| 0x80019520 / 120 | `opening_render_entities` | 100.000000 | 32-slot pool, model ID below 32, signed countdown; unchanged traversal control. |

## Names and units

| Family | Evidence and preserved meaning |
| --- | --- |
| Scene-0 models 11/12 | Finder calls at OPEN `8001428c/800142a0`; the scene subtracts/adds its yaw step respectively. The renderer uses the same IDs for its selected-model depth bias. |
| Scene-3 models 13/14 | Finder calls at OPEN `80014890/800148a4`; opposite yaw updates establish the existing increasing/decreasing role names. The ending scene reuses the pair. |
| Ending models 26/27 | Finder calls at OPEN `800150a4/800150b8`. The first translates in Y until hidden; the second is then shown and rolled. Their shared names express these observed roles without assigning story or visual identities. |
| Render-time yaw step | Models 21/22 add 64 and 23/24 subtract 64 angle units: 5.625 degrees per renderer invocation, wrapping at 4096. `matrix_set_rotation_yxz` runs before these writes, so the new yaw affects a later render. This is not a wall-clock or simulation-tick rate. |
| Selected-model depth | Models 11..14 and authored model 20 use bias -100. Other default-path models use zero. The exact asset identity of model 20 remains unresolved. |
| Ending-model depth | Translating model 26 adds 1000 ordering-table units; rotating model 27 adds 10000. Both use `render_enqueue_unlit_triangles`. These are sorting biases, not world-space distances. |
| Floor-item material | RGB 180 feeds lighting/texture modulation; it is not opacity. Two +Z light coefficients use Q12 unity (4096); their zero axes and translations remain authored matrix data. |
| Cell visibility | Both pool traversals reject `KF_CELL_WINDOW_HIDDEN`; other cell classes remain accepted with the existing unsigned window checks. |
| Projection | OPEN `800137e0` and `800194f4` pass 200 in SetGeomScreen call delay slots; both now reuse `KF_DEFAULT_PROJECTION_DISTANCE`. |
| Ending gradient | The sole G4 caller is the ending scroll at OPEN `800153ec`. The emitter adds byte offset `0xffbc` to the active ordering-table pointer, selecting index `0x3fef` (16367). Its private name describes this use; the original reason for choosing that exact slot remains unknown. |

Six existing scene constants move into `open/resources.h` with shared prefixes;
all nine scene-side uses and six renderer case labels use the shared identities.
Six new private rendering constants name the remaining tuning/placement roles.
The four renderer files contain **110 retained numeric/character occurrences**,
down from 131: **21 inline uses** now have semantic names. The
[complete remainder ledger](open-render-literal-ledger.md) covers all 67
initializer tokens and 43 function tokens. Rectangle/color component indices,
frame/countdown arithmetic, byte narrowing and authored sprite tuples retain
explicit values. Model ordinals 20..25 retain their measured resource identity
and individual behavior explanation until additional asset evidence supports
stronger names.

Moving existing scene definitions changes none of the scene file's **595**
inline numeric/character occurrences. Their prior authored-path/panel/color,
SDK argument, countdown and phase explanations remain in the
[scene audit](source-constant-names.md#later-opening-scenes-and-ending-sequence).
That audit's historical count predates size-assertion removal.

## Switch and initialized-data controls

The retail switch subtracts 11 from the byte model ID and accepts indices
0..16. Its 17 pointer words at OPEN `80012238` decode as follows:

| Model ID | Target | Effect |
| --- | --- | --- |
| 11..14, 20 | `80019000` | Depth bias -100 in the jump delay slot. |
| 15..19 | `800190a4` | Default zero depth. |
| 21..22 | `80019008` | Add yaw step, then wrap/store. |
| 23..24 | `80019014` | Subtract yaw step, then wrap/store. |
| 25 | `8001902c` | Alternate perspective-right projection, zero extra bias. |
| 26 | `80019060` | Bias 1000 in jump delay slot before unlit path. |
| 27 | `80019068` | Bias 10000 before unlit path. |

The semantic navigator still marks the indirect `jr` at `80018ff8` unresolved.
The table interpretation here uses decoded bounds and all 17 retail pointers;
no candidate reference is promoted by a percentage or by the navigator filter.
Both source and target object R_MIPS_32 entries resolve to these retail words.
The floor-item descriptors (84 bytes at `800358e0`) and light matrix (32 bytes
at `800359e4`) also match source/target symbols and retail, preserving the gap
between their owners. Scene-script initialized data remains byte-identical to
the frozen source; this does not claim that every existing scene-data layout
issue is resolved.

## Verification and final verdicts

All 112 independently compiled before/after variants retain runtime sections,
alignment, runtime symbols and ordered relocations. Only the opening-render,
scene-script and entity-render debug-line sections change. All fourteen
reviewed functions retain their initial strict scores: nine exact, five partial.
No function is newly banked.

The reviewed bodies preserve **1791 instruction words**, **166 direct calls**
and **120 ordered address pairs**. The nine exact functions reproduce
all **725 words** against retail and target objects.

| Function | Words / calls / addresses | Final verdict |
| --- | --- | --- |
| `opening_render_frame` | 16 / 5 / 0 | Exact, unchanged |
| `sprite_add_g4` | 102 / 2 / 2 | Exact, unchanged |
| `opening_scene0_run` | 93 / 11 / 10 | Exact, unchanged |
| `opening_scene1_draw_fade` | 96 / 8 / 3 | Exact, unchanged |
| `opening_scene1_run` | 43 / 8 / 1 | Exact, unchanged |
| `opening_entity_transition` | 127 / 2 / 3 | Partial, unchanged |
| `opening_scene3_run` | 204 / 27 / 19 | Partial, unchanged |
| `opening_ending_scene_run` | 189 / 20 / 23 | Exact, unchanged |
| `opening_ending_scroll_run` | 486 / 40 / 31 | Partial, unchanged |
| `opening_entity_render` | 138 / 22 / 6 | Exact, unchanged |
| `render_floor_item` | 82 / 9 / 6 | Partial, unchanged |
| `opening_render_entities_and_items` | 167 / 4 / 15 | Partial, unchanged |
| `opening_scene0_render_frame` | 18 / 6 / 0 | Exact, unchanged |
| `opening_render_entities` | 30 / 2 / 1 | Exact, unchanged |

First raw differences in the unchanged partial functions:

| Function / OPEN site | Candidate | Retail |
| --- | --- | --- |
| `opening_entity_transition`, `80014608` | Stack frame -48 | Stack frame -56 |
| `opening_scene3_run`, `80014804` | Stack frame -96 | Stack frame -112 |

The scene-3 frame gap is not unused capacity in its paired tpage and CLUT
arrays. Doubling both arrays from two to four entries creates the 112-byte
frame, but moves the live CLUT base from stack offset 48 to 56 and its second
entry from 52 to 60; the indexed overlay load likewise moves from 24 to 32.
Retail and the original candidate agree at 48, 52, and 24. Restore the proven
two-entry arrays. Their extent cannot explain the otherwise frame-only residue.
| `opening_ending_scroll_run`, `800150c0` | `sh zero,176(sp)` | `lui a0,0x8003` |
| `render_floor_item`, `80019184` | `beqz v0,800191b8` | `andi v0,v0,0xff` |
| `opening_render_entities_and_items`, `800193f4` | item-window load followed by `nop` | item-window load followed by Z-origin subtraction |

These are observations, not an attribution to a compiler pass. Exact source
structure and the existing signedness/ownership questions remain independent
of this naming pass.

The live comparison isolates one concurrent GAME experiment outside this
commit: `player_move_horizontal` at `800171fc`, whose score changed from
93.538315% to 92.839080% during the audit. Its current source was independently
compiled with both frozen header sets, producing identical runtime contents
and matching the live object. The other 111 live objects and 483 function
scores agree with the frozen controls. The movement experiment is not part
of this OPEN result or commit.

Inventory, Ruff and whitespace checks pass. The full existing test suite ran:
**679 passed, one failed**, out of 680 tests in 89.895 seconds. The sole failure
is `test_player_interaction_campaign_matches_curated_identities`: the concurrent
GAME inventory now declares a word-sized movement heading while its evidence
TSV still declares a halfword heading. Neither file is modified by this
campaign. No tests were added or changed. Modern checking retains the same
**300 error diagnostics** and **65/112** passing variants.

Full `kf build` ran and retains existing data/relink failures: source-data
matches PSX 0/1, GAME 9/42 and OPEN 2/19; target relinks PSX 1/1, GAME 75/77
and OPEN 34/38. There are six conflicting section bases and no artifact
failures. No tooling or flake configuration changed. The ten `unknown_`
source lines and broader constant/type audit remain open.
