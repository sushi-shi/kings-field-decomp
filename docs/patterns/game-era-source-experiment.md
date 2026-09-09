# GAME period-style source experiment

All 21 non-exact GAME functions from `bec4cb62` received multiple ordinary C
alternatives in a separate worktree. The fixed `probe-gcc257-o2-g0` profile
produced **two newly discovered exact functions** (`player_add_experience` and
`display_initialize`) and reproduced the **pre-existing exact statistics-header
candidate**. The isolated branch moves GAME from **341/362 to 344/362 exact**;
18 functions remain non-exact. This is source evidence, not historical compiler
attribution or an exhaustive search of possible source programs.

There are 133 alternative/comparison attempts: 132 compiled and were compared,
including the saved main-worktree header candidate; one price-table attempt
failed compilation. Each of the 21 original sources was also compiled as a
fresh control, for 154 compilation attempts in total. No declaration, register,
or Cartesian permutation search was used.

## Function Match Plan and evidence contract

The campaign plan preceded the first candidate edit. Freeze the complete
non-exact list, then inspect each GAME disassembly/CFG, callers/callees,
references, strings, neighbors, history and strict baseline before its batch.
Keep signatures, shared fixed-width objects, signed rounding, calls, constants,
return behavior and data ownership. Try authentic SDK macros, typed object and
table cursors, whole-object copies, normal loops, phase selection and meaningful
shared calculations. Follow useful intermediate ideas through explicit
combinations even when an individual form lowers the score.

Every attempt compiles its complete unit into a separate directory. Resolve
HI16/LO16 and jump relocations against curated image-specific owners, verify
that every delinked target function resolves to the actual GAME retail bytes,
and compare the candidate's raw instructions, calls and ordered materialized
referents. All 153 successfully compiled objects passed the unchanged-sibling
control. First divergence and unequal aligned-word counts remain observations,
not closure masks. Strict objdiff 100% plus fully resolved retail equality is
required for an exact verdict.

The six batches cover initialization/path helpers; level growth/rendering/item
panels; asset loading/quad panels/serialization; movement/effect collision;
large player/map/effect dispatchers; and focused level/glyph follow-ups. The
relatedness comes from actual shared objects, calls and SDK usage, not guessed
original TU boundaries. No inventories, unit boundaries, compiler flags or
vendored bodies are changed. Sony SDK calls and existing exact sibling bodies
are controls and do not add game-progress credit.

## Kept source and interpretation

`player_add_experience` uses its existing `const KfPlayerLevelGrowth *growth`
for the final table row as well as indexed normal levels. HP, MP and experience
extension values read the previous row through `growth[-1]`. This closes all
548 bytes, all three calls and the ordered referents. It follows the real
program operation: normal table lookup, then extrapolation from the last pair
of rows. Separate `last` and `previous` pointers were a productive 92.036500%
intermediate; naming the three deltas reached 98.357666%.

`display_initialize` replaces the six RGB field statements with two SDK
`setRGB0` invocations. This closes all 332 bytes. The supplied `LIBGPU.H` bears
version 3.51 and date 1994-11-21, SHA-256
`a33286ee6a0a41835a83d3a0cb053d6808b9772671d3d5f6c48c0267928a9a3b`.
The actual macro is one comma expression. This repeats the source-shape lesson
from [the OPEN experiment](open-era-source-experiment.md); it does not prove
which SDK revision originally built either image.

`menu_draw_stats_header` reproduces the saved main-worktree candidate: a real
`s32 row_step = STATS_HEADER_ROW_STEP` supplies all 14 line advances. The full
1440 bytes match, including the retail transition from immediate spacing to a
shared saved value. Its origin is explicitly separate from the two discoveries.

`map_world_state_persist` keeps an independently justified, byte-identical
cleanup: select the owning floor's `records` member directly. Its score stays
97.528730%; this is not banked as exact. The higher fuzzy item-detail candidates
remain experimental because the original two-word scheduling discrepancy is
smaller and more localized than their new raw differences.

## Per-function result

All addresses below are in GAME.EXE. “Best” includes the unchanged control;
“tries” counts successfully compiled alternatives/comparisons, excluding that
control. Scores are strict objdiff percentages rounded only for this table.
Each evidence snapshot also records the signature, full claim extent, shared
unit/source owner, reference evidence tiers and history.

| Function / VA | Tries | Before | Best | Kept | Verdict |
|---|---:|---:|---:|---:|---|
| `player_add_experience` / `0x80016058` | 12 | 88.824814 | 100.000000 | 100.000000 | new exact |
| `player_move_horizontal` / `0x800171fc` | 6 | 96.568960 | 96.568960 | 96.568960 | no gain |
| `player_update` / `0x80018880` | 7 | 99.476960 | 99.476960 | 99.476960 | no gain |
| `display_initialize` / `0x8001bb94` | 5 | 98.421684 | 100.000000 | 100.000000 | new exact |
| `render_map_cell` / `0x8001e5ec` | 5 | 99.878380 | 99.878380 | 99.878380 | no gain |
| `render_entities` / `0x8001f218` | 5 | 99.517044 | 99.517044 | 99.517044 | no gain |
| `item_load_database` / `0x80020cfc` | 6 | 99.746666 | 99.746666 | 99.746666 | no gain |
| `menu_status_panel` / `0x8002430c` | 7 | 99.962170 | 99.962170 | 99.962170 | no gain |
| `menu_draw_stats_header` / `0x80025f38` | 5 | 97.977776 | 100.000000 | 100.000000 | pre-existing exact reproduced |
| `menu_item_model_preview` / `0x800279c4` | 5 | 98.181816 | 98.181816 | 98.181816 | no gain |
| `menu_draw_item_detail` / `0x80027b7c` | 9 | 97.814210 | 98.579230 | 97.814210 | fuzzy gain rejected |
| `menu_draw_item_name_frame` / `0x800292f8` | 8 | 99.570850 | 99.570850 | 99.570850 | no gain |
| `menu_draw_window_backdrop` / `0x8002a510` | 6 | 99.971760 | 99.971760 | 99.971760 | no gain |
| `talk_show_dialogue_page` / `0x8002c9d4` | 5 | 98.780490 | 98.780490 | 98.780490 | no gain |
| `map_object_spawn_effect` / `0x80031834` | 5 | 94.504950 | 94.504950 | 94.504950 | no gain |
| `map_show_screen_image` / `0x80034d54` | 5 | 88.888885 | 88.888885 | 88.888885 | no gain |
| `map_interaction_dispatch` / `0x80034de4` | 7 | 99.202774 | 99.202774 | 99.202774 | no gain |
| `map_world_state_persist` / `0x80035b5c` | 6 | 97.528730 | 97.528730 | 97.528730 | same bytes; typed owner |
| `effect_map_collision` / `0x80037850` | 6 | 99.873690 | 99.873690 | 99.873690 | no gain |
| `effect_projectile_update_2d` / `0x80038298` | 5 | 99.934210 | 99.934210 | 99.934210 | no gain |
| `effect_update_dispatch` / `0x80038a38` | 7 | 99.827810 | 99.827810 | 99.827810 | no gain |

**01. `player_add_experience`** — Exact with one growth-row cursor reused in both level ranges; the last row and `growth[-1]` supply extension increments. Twelve alternatives include the productive two-pointer intermediate (92.036500%) and explicit deltas (98.357666%).

**02. `player_move_horizontal`** — No gain. Tried typed position access, collision-while and retry-for loops, if/else bearing adjustment, absolute values and diagonal chains. Baseline remains 40 bytes longer, with an extra angle move, different register use and later address calculations. Ordered materialized references still differ; no compiler mechanism is claimed.

**03. `player_update`** — No gain. Individual and combined SDK spawn/vector operations preserve baseline bytes. Structured slowed expiry, shared MP cost and normalized movement components change code without improving it. Baseline has an eight-byte smaller frame and corresponding local offsets; all 77 differing words are retained as unresolved evidence.

**04. `display_initialize`** — Exact with two authentic `setRGB0` macro invocations on the owned DRAWENV array. The direct arrays and chained dithering assignment stay. The macro is the single comma-expression form from the supplied Sony header, not hand-written instruction steering.

**05. `render_map_cell`** — No gain. SDK versus field assignments, orientation switch, attribute if/else, phase-bit selection and a view-matrix pointer do not close the body. Baseline has 18 stack/frame operand differences, with a 32-byte smaller frame; no padding is added.

**06. `render_entities`** — No gain. Pool cursors/countdowns, direct texture-page access, grouped colour channels and combined wrapped culling expressions do not close the 30 register-word differences. Call order, complete body size and ordered referents agree.

**07. `item_load_database`** — No gain. Decimal numbering forms, digit characters, sector alignment and a typed CD-entry cursor do not improve the baseline. Its 16 differing words begin in filename arithmetic at +0x4bc; calls and ordered referents agree.

**08. `menu_status_panel`** — No gain. SDK quad helpers and ordinary settling/input phases retain or worsen the score. Baseline differs in 16 frame/save operands, with a 64-byte smaller frame. The complete SDK packet types are preserved.

**09. `menu_draw_stats_header`** — Exact pre-existing comparison. The main-worktree candidate saved before this experiment gives one meaningful row spacing value to all 14 line advances. The other four independently written alternatives do not improve the baseline. This is reproduced evidence, not a new discovery attributed to this campaign.

**10. `menu_item_model_preview`** — No gain. Matrix initialization and glyph-copy variants leave the best body with three reordered address-calculation instructions at +0xc4..+0xcc; complete size, calls and ordered referents agree.

**11. `menu_draw_item_detail`** — Fuzzy gain only; original retained. Indexed/plain glyph-string copies reach 98.579230%, but turn a two-word scheduling swap at +0xec/+0xf4 into 13 unequal aligned words starting +0xd4. Row assignment, actual glyph cursors/countdowns and whole price-table selection do not close it. The first price-table spelling failed compilation and was corrected in a separate recorded attempt.

**12. `menu_draw_item_name_frame`** — No gain. SDK XY/UV/rectangle macros, packet cursors, grouped emission and row-copy forms do not improve the baseline. It has a 64-byte smaller frame plus three reordered address-calculation words, 15 differing words in all.

**13. `menu_draw_window_backdrop`** — No gain. SDK quad forms, a real packet pointer and ordinary primitive emission leave the 12 frame/save differences unresolved. Baseline frame is 64 bytes smaller; no fictitious local is introduced.

**14. `talk_show_dialogue_page`** — No gain. Decimal digit locals, separate stores and pathname cursors retain or worsen the eight register-word differences. Calls, ordered referents and the complete 164-byte extent agree.

**15. `map_object_spawn_effect`** — No gain. SDK vector setup, coordinate grouping, sequence publication and angle chaining do not close the four-byte-shorter body. Baseline has a different saved-register set and sequence-count schedule, while calls and ordered referents agree.

**16. `map_show_screen_image`** — No gain. Direct pathname access, decimal expressions/characters and real string cursors do not close the four-byte-longer body. The first difference is remainder register use at +0x30; floor/path address order and a load-delay nop also differ.

**17. `map_interaction_dispatch`** — No gain. Counted item/search loops, direct unsigned pitch interval, gold addition, positive animation eligibility, pickup if/else and image selection do not improve the baseline. It is 12 bytes shorter, with different register use and countdown/branch joins. The retail first-item-only precheck is preserved.

**18. `map_world_state_persist`** — Byte-identical source cleanup retained. Use `world_state.floors[current_floor - 1].records` instead of constructing a pointer before the owning byte array. Five serialization/count/cursor alternatives do not improve the score. All original 84 unequal aligned words, starting in object serialization at +0x16c, remain; the body is four bytes longer.

**19. `effect_map_collision`** — No gain. Result initialization, direct case returns, explicit zero case, unsigned cell guards and merging the inline helper do not close the single differing word at +0x718: retail nop versus an extra fallback result load in an unreachable masked-switch path. No return value is invented to eliminate it.

**20. `effect_projectile_update_2d`** — No gain. Authentic byte phase, shared collision kind, one orbit angle, a real distance VECTOR and a position pointer do not close the ten frame/save differences. Baseline frame is 64 bytes smaller; trigonometric call order is preserved.

**21. `effect_update_dispatch`** — No gain. Homing SDK operations individually and together, lightning countdown and shared scale publication do not close the baseline 42 register-word differences. Calls, complete size and ordered referents agree. The existing uninitialized homing-distance read is preserved rather than repaired as unrelated gameplay work.

## Verification

The normal full build freshly compiled all four affected units. Focused
semantic matches report the three exact rows at 100%; a separate audit of the
canonical built objects verifies equality to the reviewed trial objects and
unchanged sibling instructions/referents. `map_world_state_persist` retains its
complete original instruction stream.

The existing graphics-owner integration test was updated to require complete
retail equality for the initializer rather than its obsolete partial residue.
Its production-owner control now requires raw equality for all three newly
exact rows, retaining wrong-referent negative controls. Correct initializer
size also restores the later switch-table addends in `game.render`: all 116
rodata bytes and relocation rows match. A second existing test now verifies
that payload equality while still rejecting invalid section placement.

Verification used `nix develop` after hash-checked `kf init`. The canonical
four-unit audit checks 38 complete functions. The fresh GAME report, excluding seven exact vendored verification rows,
contains 344/362 exact game rows and confirms unchanged scores for every one of
the other 18 frozen non-exact functions. Only the three selected exact rows
were banked, with staged source inputs, using:

```sh
kf bank --function game:0x80016058 --function game:0x8001bb94 --function game:0x80025f38
```

`ruff check scripts tests` and `git diff --check` pass. The complete final repository run,
`python3 -m unittest discover -s tests -v`, passes all 739 tests with 9 skips.
The skipped optional integration checks report their missing local driver or
emulator prerequisites in the test log.

Full `kf build` runs all image checks and returns nonzero on the pre-existing
data/ownership/placement blockers. The baseline and final counts agree:
PSX data 0/1 and target relink 1/1; GAME data 8/44 and target relink 63/77;
OPEN data 3/20 and target relink 32/38. GAME render's switch-table addend mismatch
is fixed, but its rodata placement and BSS extent/ownership still fail. No data
or whole-program closure is claimed. OPEN stays 107/108 exact and PSX 1/1;
the overall exact count moves 449/471 to 452/471.

## Master integration

The experiment was merged against master `c19d9d05`. Master already contained
the same display initializer fix from the Sony-helper campaign, so it is kept
once together with that campaign's broader SDK changes and equivalent strict
rodata checks. The added growth and statistics-header matches move the clean
master tree from 342/362 to 344/362 exact GAME functions. This does not include
other uncommitted experiments in the main worktree.

All 38 functions in the four affected units are rechecked against the reviewed
experiment objects. The three selected functions remain strict 100%, and the
world-state cleanup keeps its original bytes. The merged tree receives a fresh
full build, lint, full repository tests and diff checks. The merged-tree suite
passes all 739 tests with 9 skips; lint and diff checks pass. Existing whole-
program data/placement failures remain outside this source experiment.

## Attempt ledger

Generated full sources, source diffs, reasons, objects, scores, relocation
listings, resolved raw differences and sibling audits remain in the experiment
worktree under `build/era/trials/<id>/`. Original six-view dossiers and histories
are under `build/era/evidence/<function>/`; original complete unit sources are
under `build/era/baselines/`. These generated products are not committed.
`runner.py` and the six explicit batch scripts preserve the local replay.

“Same” means the fully resolved candidate equals its original compiled control;
“changed” does not claim a semantic correction. The two background emission
loops also changed the static ordered call list by sharing call sites and were
rejected; they are not call-list matches. Every successful row retained
all sibling controls. The ledger keeps failed and negative experiments visible.

| Attempt | Strict % | Bytes | First raw difference | Aligned unequal words | Result | Source idea |
|---|---:|---:|---|---:|---|---|
| `01-clamp-values` | 80.065700 | 576 | 0x4 | 132 | changed | Express the four existing saturations as selected values. |
| `01-do-levels` | 88.824814 | 528 | 0x4 | 125 | same | Test a guarded do/while for processing one or more earned levels. |
| `01-extension-deltas` | 98.357666 | 544 | 0x58 | 75 | changed | Name the three real growth differences used for levels beyond the authored table. |
| `01-growth-cursor-reuse` | 100.000000 | 548 | — | 0 | exact | Use the same real growth-row pointer in both level ranges; the prior row supplies extension deltas. |
| `01-guarded-loop` | 88.824814 | 528 | 0x4 | 125 | same | Use an explicit early-exit level-up loop. |
| `01-indexed-growth` | 79.817520 | 576 | 0x4 | 127 | changed | Use direct indexing for every normal growth-table field. |
| `01-last-and-do` | 92.036500 | 544 | 0x4 | 120 | changed | Follow the positive table-cursor result through a guarded do/while level loop. |
| `01-last-and-indexed` | 83.029200 | 592 | 0x4 | 130 | changed | Combine the final-row pointers with direct indexing for normal levels. |
| `01-last-and-post-level` | 89.372260 | 556 | 0x4 | 137 | changed | Combine the final-row pointers with postincrement capture of the previous level. |
| `01-last-growth` | 92.036500 | 544 | 0x4 | 120 | changed | Name the final two growth rows that define post-table level increments. |
| `01-player-pointer` | 87.291970 | 524 | 0x4 | 132 | changed | Use one typed pointer for the player object throughout level advancement. |
| `01-post-level` | 86.160580 | 540 | 0x4 | 128 | changed | Capture the old level with postincrement after the maximum-level guard. |
| `02-absolute-distances` | 94.155174 | 2124 | 0x2c | 463 | changed | Use conditional absolute values for the two actual displacement magnitudes. |
| `02-angle-branches` | 96.344826 | 2132 | 0x2c | 467 | changed | Express the two deflection bearings as ordinary if/else, followed by angle wrapping. |
| `02-collision-while` | 96.568960 | 2128 | 0x2c | 468 | same | Make the collision query the retry loop condition, preserving the two-hit early return. |
| `02-diagonal-chain` | 96.568960 | 2128 | 0x2c | 468 | same | Assign equal diagonal components together, preserving negation before the signed shift. |
| `02-position-pointer` | 92.350580 | 2080 | 0x30 | 419 | changed | Use one typed pointer for the camera position read and updated throughout collision sliding. |
| `02-retry-for` | 95.199234 | 2112 | 0x2c | 459 | changed | Put the retry countdown in the for loop; the second collision still returns before applying grid movement. |
| `03-magic-cost` | 98.600240 | 6660 | 0x0 | 960 | changed | Read the real MP cost once, then apply the accessory discount when eligible. |
| `03-normalized-components` | 99.414120 | 6684 | 0x0 | 93 | changed | Compute each full-width normalized component once before signed selection and s16 storage. |
| `03-rotation-vector` | 99.476960 | 6684 | 0x0 | 77 | same | Use the authentic vector view and SDK copyVector for the three camera rotation components. |
| `03-sdk-spawn-sequence` | 99.476960 | 6684 | 0x0 | 77 | same | Compose the three supported SDK macro forms across the complete weapon spawn sequence. |
| `03-slowed-structured` | 99.053860 | 6680 | 0x0 | 227 | changed | Express slowed expiry with a shared test after selecting reset or decrement, without an interior goto. |
| `03-spawn-vector` | 99.476960 | 6684 | 0x0 | 77 | same | Initialize the actual weapon spawn SVECTOR through the SDK setVector macro. |
| `03-world-vector` | 99.476960 | 6684 | 0x0 | 77 | same | Use the SDK addVector form to translate the weapon spawn position into world coordinates. |
| `04-draw-array` | 85.072290 | 344 | 0x0 | 58 | changed | Use one actual draw-environment array pointer for setup and all field writes. |
| `04-draw-array-rgb` | 85.072290 | 344 | 0x0 | 58 | changed | Use one actual draw-environment array pointer for setup and all field writes. |
| `04-dtd` | 93.518074 | 336 | 0x98 | 18 | changed | Separate the two DTD assignments, preserving their C evaluation order. |
| `04-rgb` | 100.000000 | 332 | — | 0 | exact | Use authentic LIBGPU setRGB0 expressions for both DRAWENV backgrounds. |
| `04-rgb-dtd` | 94.903620 | 332 | 0x98 | 8 | changed | Compose real RGB macros and separate DTD initialization. |
| `05-coordinate-statements` | 99.878380 | 592 | 0x0 | 18 | same | Spell the three cell-relative position components as ordinary field assignments. |
| `05-orientation-switch` | 90.013510 | 624 | 0x0 | 65 | changed | Use an ordinary switch for quadrant-dependent origin correction. |
| `05-phase-bit` | 98.790540 | 588 | 0x0 | 130 | changed | Express the two-on/two-off remap phase as its actual bit test (mask 3, active 2). |
| `05-remap-if` | 91.506760 | 564 | 0x0 | 128 | changed | Use three explicit attribute tests for the authored mesh replacements. |
| `05-view-matrix` | 92.878380 | 588 | 0x0 | 132 | changed | Name the one real view matrix shared by the three SDK transform calls. |
| `06-color-chain` | 99.517044 | 1408 | 0x5c | 30 | same | Group the equal CVECTOR channels with a chained assignment in the same order. |
| `06-combined-square` | 97.806816 | 1420 | 0x0 | 341 | changed | Compute each square-culling coordinate as one expression; final u16 wrap is unchanged. |
| `06-direct-tpage` | 97.718750 | 1408 | 0x5c | 47 | changed | Use the owned active texture page directly instead of a one-use interior pointer. |
| `06-floor-countdown` | 97.732956 | 1396 | 0x5c | 211 | changed | Use a classic postdecrement countdown for floor items, including the empty pool. |
| `06-object-for-cursor` | 99.517044 | 1408 | 0x5c | 30 | same | Move the live object-cursor advance into the for-loop update. |
| `07-destructive-digits` | 99.746666 | 1500 | 0x4bc | 16 | same | Reuse the actual number as its remaining decimal digits after extracting hundreds. |
| `07-digit-characters` | 98.770670 | 1500 | 0x4ec | 10 | changed | Construct the actual output characters, then place them in the filename template. |
| `07-file-cursor` | 98.888000 | 1492 | 0x4b0 | 48 | changed | Walk the real 20-byte CD file entries with a typed cursor, preserving the SDK boundary cast. |
| `07-inline-number` | 99.746666 | 1500 | 0x4bc | 16 | same | Use the one-based item index directly in the filename digit expressions. |
| `07-number-late` | 99.746666 | 1500 | 0x4bc | 16 | same | Compute the file number after assigning its directory group. |
| `07-sector-rounding` | 96.874664 | 1488 | 0x4ac | 28 | changed | Round the u32 file size with the usual sector-alignment expression; unsigned wrap is preserved. |
| `08-direct-tpage` | 97.900710 | 1692 | 0x0 | 331 | changed | Pass the actual texture-page field directly at all four helper calls. |
| `08-frame-for` | 99.489365 | 1692 | 0x0 | 19 | changed | Express the frame driver as an initialized infinite for loop. |
| `08-input-direct` | 99.962170 | 1692 | 0x0 | 16 | same | Use the single pad-read result directly in the active-input test. |
| `08-settling-else` | 99.806145 | 1692 | 0x0 | 19 | changed | Group settling and pad release in one phase, then handle active input in else. |
| `08-settling-post` | 98.482270 | 1696 | 0x0 | 39 | changed | Advance the bounded settling frame counter with postincrement, preserving pad release on the third frame. |
| `08-uv-helper-macro` | 99.962170 | 1692 | 0x0 | 16 | same | Use the SDK UV macro inside the actual inlined backdrop helper. |
| `08-xy-helper-macro` | 99.962170 | 1692 | 0x0 | 16 | same | Use the SDK XY macro inside the actual inlined backdrop helper. |
| `09-blank-loop` | 92.588890 | 1428 | 0x0 | 344 | changed | Initialize the five status glyph slots with a fixed-length loop. |
| `09-class-if` | 97.977776 | 1428 | 0x0 | 360 | same | Use an ordinary ascending three-tier if/else for the displayed class. |
| `09-existing-row-step` | 100.000000 | 1440 | — | 0 | exact | Pre-existing main-worktree comparison: one shared s32 row_step used by every line advance. |
| `09-glyph-cursor` | 95.319440 | 1436 | 0x20 | 216 | changed | Use a typed pointer for the glyph workspace throughout label construction. |
| `09-status-post` | 97.977776 | 1428 | 0x0 | 360 | same | Use postdecrement while filling status glyphs from right to left. |
| `10-copy-cursor` | 97.818184 | 440 | 0xc4 | 9 | changed | Use the source glyph pointer as the actual copy cursor. |
| `10-copy-do` | 98.181816 | 440 | 0xc4 | 3 | same | Use a do/while for the known nonempty fixed-length name row. |
| `10-copy-struct` | 55.745453 | 600 | 0x18 | 104 | changed | Copy the complete shared glyph-row object using C structure assignment. |
| `10-early-return` | 98.181816 | 440 | 0xc4 | 3 | same | Use an early return for the empty inventory slot. |
| `10-matrix-chains` | 98.181816 | 440 | 0xc4 | 3 | same | Initialize the two equal light rows and the zero row with chained assignments in original store order. |
| `11-copy-do` | 97.814210 | 732 | 0xec | 2 | same | Use do/while for the nonempty ten-glyph row, preserving the current workspace view. |
| `11-glyph-countdown` | 96.098360 | 728 | 0x28 | 119 | changed | Use two actual glyph cursors and a simple count of characters remaining. |
| `11-glyph-cursor` | 96.715840 | 732 | 0xd4 | 6 | changed | Use two actual glyph cursors without walking across the position fields. |
| `11-glyph-row-copy` | 73.398910 | 892 | 0x28 | 170 | changed | Copy the complete supported MenuGlyphRow object with a structure assignment. |
| `11-indexed-copy` | 98.579230 | 732 | 0xd4 | 12 | changed | Use conventional indexed copy through the shared glyph-row fields. |
| `11-plain-glyph-countdown` | 96.098360 | 728 | 0x28 | 119 | changed | Combine the ordinary glyph-string type with its real character countdown. |
| `11-plain-row-copy` | 73.398910 | 892 | 0x28 | 170 | changed | Combine the ordinary glyph-string object with complete row assignment. |
| `11-price-table` | — | — | — | — | compile failure | Select the whole price table once, then index the item and shop column. |
| `11-price-table-fixed` | 91.442620 | 732 | 0x20 | 22 | changed | Rerun table selection with the actual KF_ITEM_SHOP_COUNT declaration after the first spelling failed compilation. |
| `11-simple-glyph-string` | 98.579230 | 732 | 0xd4 | 12 | changed | Use the ordinary MenuGlyphString object and indexed copy, as the neighboring preview does. |
| `12-background-loop` | 92.823880 | 1868 | 0x0 | 69 | changed | Link the four background primitives in their original descending order with a loop. |
| `12-copy-do` | 99.570850 | 1976 | 0x0 | 15 | same | Use do/while for the fixed-length name copy. |
| `12-copy-struct` | 89.785420 | 2132 | 0x0 | 489 | changed | Use a complete shared glyph-row assignment for the item name. |
| `12-packet-pointer` | 88.153850 | 1808 | 0x0 | 377 | changed | Use the newly allocated packet pointer locally while filling each complete quad. |
| `12-rectangle-macro` | 99.570850 | 1976 | 0x0 | 15 | same | Use the SDK rectangle helper for quad positions defined by one width and height. |
| `12-uv-macro` | 99.570850 | 1976 | 0x0 | 15 | same | Use authentic SDK setUV4 for each reflected texture quad. |
| `12-xy-macro` | 99.570850 | 1976 | 0x0 | 15 | same | Use authentic SDK setXY4 for each quad, keeping all corner expressions. |
| `12-xy-uv-macros` | 99.570850 | 1976 | 0x0 | 15 | same | Compose SDK position and texture-coordinate macros on the same packets. |
| `13-background-loop` | 92.129410 | 1592 | 0x0 | 66 | changed | Link the four background primitives in their original descending order with a loop. |
| `13-packet-pointer` | 86.701180 | 1532 | 0x0 | 374 | changed | Use the newly allocated packet pointer locally while filling each complete quad. |
| `13-rectangle-macro` | 99.971760 | 1700 | 0x0 | 12 | same | Use the SDK rectangle helper for quad positions defined by one width and height. |
| `13-uv-macro` | 99.971760 | 1700 | 0x0 | 12 | same | Use authentic SDK setUV4 for each reflected texture quad. |
| `13-xy-macro` | 99.971760 | 1700 | 0x0 | 12 | same | Use authentic SDK setXY4 for each quad, keeping all corner expressions. |
| `13-xy-uv-macros` | 99.971760 | 1700 | 0x0 | 12 | same | Compose SDK position and texture-coordinate macros on the same packets. |
| `14-characters` | 94.512190 | 164 | 0x60 | 7 | changed | Give the two reused filename characters their actual char type, including ASCII offset. |
| `14-cursor` | 79.853660 | 160 | 0x30 | 27 | changed | Advance the directory cursor across its two adjacent character slots. |
| `14-direct` | 84.829270 | 164 | 0x3c | 16 | changed | Write the two directory characters through the owning path array. |
| `14-inline-digits` | 84.829270 | 164 | 0x3c | 16 | changed | Write quotient and remainder expressions directly at their use sites. |
| `14-separate` | 98.780490 | 164 | 0x30 | 8 | same | Separate filename and directory digit writes in the same store order. |
| `15-axis-cells` | 84.445540 | 384 | 0x4 | 100 | changed | Complete each horizontal axis and its tile coordinate together, as retail division order suggests. |
| `15-copy-position` | 89.950490 | 408 | 0x4 | 90 | changed | Copy all three coordinates with SDK copyVector, then apply the vertical offset. |
| `15-position-macro` | 94.504950 | 400 | 0x4 | 55 | same | Use SDK setVector for the three live position components. |
| `15-sequence` | 94.396040 | 412 | 0x20 | 88 | changed | Separate assigning the spawn sequence from advancing the pool sequence. |
| `15-zero-angles` | 94.504950 | 400 | 0x4 | 55 | same | Use a chained assignment for the two zero rotation components in their existing store order. |
| `16-characters` | 83.611115 | 148 | 0x30 | 20 | changed | Construct actual output characters before assigning the filename slots. |
| `16-cursor` | 72.055560 | 140 | 0x30 | 22 | changed | Traverse the three adjacent filename characters with a real cursor. |
| `16-digits` | 88.888885 | 148 | 0x30 | 20 | same | Compute quotient and remainder together before filling the path fields. |
| `16-direct` | 79.250000 | 148 | 0x30 | 22 | changed | Use the owning path array for the floor character as well as the filename. |
| `16-direct-digits` | 79.250000 | 148 | 0x30 | 22 | changed | Compose quotient/remainder locals with direct path fields. |
| `17-gold-add` | 99.185440 | 2296 | 0x2c | 166 | changed | Add the actual notified gold amount directly to the player balance. |
| `17-has-talk-animation` | 98.812830 | 2300 | 0x2c | 388 | changed | Represent the saved animation count as a positive has-second-animation condition. |
| `17-image-selection` | 98.213170 | 2300 | 0x2c | 155 | changed | Select the two supported image groups with a switch, retaining the unsupported-object return. |
| `17-neighbor-for` | 99.202774 | 2296 | 0x2c | 166 | same | Put the paired-door search cursor advance in its for-loop header. |
| `17-pickup-for` | 98.740036 | 2288 | 0x264 | 251 | changed | Use ordinary counted pointer loops for both four-item containers; keep the peculiar first-item-only precheck. |
| `17-pickup-results` | 96.915080 | 2280 | 0x2c | 274 | changed | Handle the two significant pickup results with if/else in both container loops. |
| `17-pitch-range` | 98.128250 | 2276 | 0x2c | 303 | changed | Write the unsigned wraparound pitch interval directly, using the original u16 narrowing. |
| `18-actor-state-value` | 90.028730 | 704 | 0x3c | 134 | changed | Select the serialized lifecycle byte before advancing the output cursor. |
| `18-definitions-direct` | 96.867810 | 704 | 0x174 | 74 | changed | Index the owning object-definition array directly at the behavior lookup. |
| `18-link-countdown` | 92.040230 | 700 | 0xc | 132 | changed | Use a standard byte-count countdown for the eight-byte object link. |
| `18-link-indexed` | 93.201150 | 704 | 0xbc | 88 | changed | Copy the complete link bytes with conventional indexing and one output advance. |
| `18-offset-first` | 96.189650 | 700 | 0x10 | 92 | changed | Calculate the zero-based floor offset before adding it to the world byte array. |
| `18-typed-floor` | 97.528730 | 700 | 0x16c | 84 | same | Select the typed saved-floor record array directly, avoiding a pointer before the owned array. |
| `19-default-at-entry` | 98.084210 | 1904 | 0x40 | 44 | changed | Initialize the helper result once at entry, retaining all geometry and target cases. |
| `19-default-before-switch` | 97.926315 | 1908 | 0xac | 38 | changed | Give the collision result its fallback before selecting the target-specific query. |
| `19-explicit-no-target` | 98.877890 | 1892 | 0xac | 38 | changed | Spell out the zero target-mask case, which is the only reachable fallback after masking with three. |
| `19-return-cases` | 97.831580 | 1904 | 0x94 | 420 | changed | Return target query results directly from their switch cases; retain the earlier geometry return label. |
| `19-single-function` | 99.063156 | 1892 | 0x94 | 419 | changed | Keep cell validation and geometry in one ordinary function instead of a private inline helper. |
| `19-unsigned-cell-guard` | 99.873690 | 1900 | 0x718 | 1 | same | Use unsigned bounds checks on the already narrowed signed cell coordinates. |
| `20-collision-kind` | 99.934210 | 608 | 0x0 | 10 | same | Name the collision category once for the actor/player damage dispatch. |
| `20-distance-vector` | 97.960526 | 620 | 0x0 | 56 | changed | Represent the actual three-dimensional listener displacement as a VECTOR used by the distance calculation. |
| `20-orbit-angle` | 93.973690 | 612 | 0x0 | 68 | changed | Snapshot the orbit angle used by all three trigonometric calls, preserving their order. |
| `20-phase-byte` | 99.934210 | 608 | 0x0 | 10 | same | Keep the local phase at the authentic byte field width and omit redundant explicit masks. |
| `20-position-pointer` | 93.828950 | 620 | 0x0 | 151 | changed | Use a typed cursor for the effect position shared by movement, collision, sound and distance. |
| `21-homing-add` | 99.827810 | 6156 | 0x38 | 42 | same | Use SDK addVector for the rotated homing displacement applied to the world position. |
| `21-homing-copy` | 99.827810 | 6156 | 0x38 | 42 | same | Use SDK copyVector for the actual VECTOR-to-SVECTOR coordinate narrowing between rotations. |
| `21-homing-sdk` | 99.826510 | 6156 | 0x38 | 44 | changed | Compose the three SDK vector operations through the complete homing transform. |
| `21-homing-vector` | 99.826510 | 6156 | 0x38 | 44 | changed | Initialize the real local homing displacement through SDK setVector. |
| `21-lightning-countdown` | 99.827810 | 6156 | 0x38 | 42 | same | Use a direct predecrement of the actual u16 lightning lifetime field. |
| `21-scatter-scale-chain` | 99.825210 | 6156 | 0x38 | 46 | changed | Group the four equal scatter scales as a normal chained assignment. |
| `21-spawner-publish` | 99.827810 | 6156 | 0x38 | 42 | same | Publish the shared actor-spawner scale and byte phase in ordinary grouped assignments. |
