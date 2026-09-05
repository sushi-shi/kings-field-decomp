# GAME map-script data ownership

## Function Match Plan

Baseline `30448fb`; all selected units use `probe-gcc257-o2-g0`. This remains
a compiler probe, not historical attribution. Retail was freshly hash-validated.
The pre-edit objects/reports are saved locally under
`build/map-script-baseline-phK9BB`. Required six-view dossiers are
`build/map-script-data-evidence.log` and `build/map-script-owner-evidence.log`.
The camera/constructor/caller disassembly is recorded separately under
`build/map-script-data-callers.log` and `build/map-script-camera-segment.log`.
All were read before editing. These generated files are not committed inputs.

The old coverage gaps are not complete objects. Split GAME `800561c8/70`
into eight still-uninterpreted bytes, a two-record camera path, a VECTOR and
a MATRIX. The mutable `KAN\\B0\\K000.TIM` buffer at `80056238` occupies 16 bytes,
including its NUL at `80056247`; that NUL is not the start of effect data.
The following gap contains a separate MATRIX at `80056248` and five seven-byte
floor-deformation records at `80056268`. Preserve the unexplained byte at
`8005628b` (retail `64`) and the following library-adjacent range; do not enlarge
the floor table to consume them or manufacture an initialized zero pad.

| GAME VA / size | Proposed private owner | Evidence |
| --- | --- | --- |
| 561d0 / 38 | map_floor5_camera_path[2] | `camera_path_begin` argument at 34704/08; 28-byte stride and x=-1 termination in camera_path_compute_segment. One position/rotation/speed point and a complete sentinel record. |
| 56208 / 10 | map_floor1_sound_position | VECTOR argument at 33f8c/90 to spatial sound; three 32-bit coordinates plus authentic SDK vector lane. |
| 56218 / 20 | map_reveal_light_matrix | MATRIX argument at 344d8/dc to matrix_interpolate; nine signed halfwords, SDK alignment/translation lanes. |
| 56238 / 10 | map_screen_image_path[16] | Bytes 5,8,9,10 are edited; full base is passed to screen_show_image_until_input via a0-5 in the call delay slot. Retail byte 15 is NUL. |
| 56248 / 20 | actor_transform_color_matrix | Both fade loops pass this exact MATRIX at 36d74/78 and 36dcc/dd0. Independent from the filename and floor records. |
| 56268 / 23 | floor_deform_segments[5] | Seven-byte stride at 3852c/30; unsigned loads of fields 0..6. Player item 62 constructs segment ranges (0,4) and (4,1), passed through effect-pool fields to the two dispatcher calls. |

Use existing SDK VECTOR/MATRIX and KfCameraPathPoint types, not size-compatible
substitutes. Introduce KfFloorDeformSegment in the effect header: unsigned-byte
column, row, column/row step, cell count, starting and ending height. A step of
255 is added as an unsigned byte and coordinates are masked modulo 256 in
retail; do not silently change those loads to signed bytes. Objects live in
their consumer modules without public externs when private ownership is kept.

Test consolidation of `game.map_floor_scripts` and `game.map_interaction`:
the contiguous 33ee4..356e8 run contains all five ambient scripts, all five
action scripts and their dispatcher/helpers. Proven calls connect the halves;
their private data is interleaved in one contiguous 120-byte run, and their
RODATA claims are adjacent (12a54/28 then 12a7c/164). These are independent
ownership clues beyond address proximity. Preserve every exact function and
literal/switch referent; the original filename/boundary remains WIP. A failed
consolidation probe is evidence, not permission to waive a regression.

## Per-function snapshots

All entries are GAME policy, absent from the curated vendored inventory;
existing Psy-Q sound/GTE/SDK callees remain library boundaries. Prior negative
controls include `game_semantic_player_warp.tsv`,
`game_semantic_menu_interaction_api.tsv` and the map/effect reconstruction
history. No library-shaped body is reconstructed and no candidate switch-table
pointer or SDK reference is promoted. Existing signatures, calls, raw branch
and return delay slots remain unchanged except the explicitly described
floor-progress correction. The two RODATA path literals stay source literals.

| VA / extent | Function | Before % | Individual evidence / hypothesis | Final verdict |
| --- | --- | ---: | --- | --- |
| 33ee4 / 80 | actor_pool_find_at_tile | 100 | u8 x/z masked at entry; 128 records, 72-byte stride, signed-halfword index/-1 result. Ambient and restore callers; no calls/strings. Move unchanged. | Exact retained: 100% |
| 33f64 / 288 | map_ambient_script_floor1 | 100 | No args/result, event-update caller; world flags and unsigned cell bounds, actor initialization/object lookup. Replace only the spatial-sound VECTOR cast/base. | Exact retained: 100% |
| 341ec / 70 | map_ambient_script_floor2 | 100 | No args/result; event bytes gate rand<4000 and spatial sound. Move unchanged with its sound referent. | Exact retained: 100% |
| 3425c / 88 | map_ambient_script_floor3 | 100 | No args/result; tile checks, vitals restore and two magic bytes/notification. Move unchanged. | Exact retained: 100% |
| 342e4 / 8 | map_ambient_script_floor4 | 100 | No args/result; admitted event-update call, jr ra/nop. Move unchanged. | Exact retained: 100% |
| 342ec / f4 | map_ambient_script_floor5 | 100 | No args/result; event-update call, tile/yaw bounds, two TALK/C17 TIM literals and actor/copy-region writes. Preserve literal order and call delays. | Exact retained: 100% |
| 343e0 / 58 | map_action_script_floor1 | 100 | No args/result; dispatcher call, item 56 and world flag gate copy-region and SoundRef calls. Move unchanged. | Exact retained: 100% |
| 34438 / 184 | map_reveal_fade | 100 | No args/result, action-floor2 caller; saved full MATRIX, blend 0..4096 by128 then reverse by256, threshold1025, rotation/position stores. Name the private light MATRIX; preserve all lighting/render calls. | Exact retained: 100% |
| 345bc / 54 | map_action_script_floor2 | 100 | No args/result; event word & ffffff00 equals28010200, state1 -> reveal. Move unchanged. | Exact retained: 100% |
| 34610 / 90 | map_action_script_floor3 | 100 | No args/result; item50 and event word28010300 gate two learned bytes and notifications. Move unchanged. | Exact retained: 100% |
| 346a0 / 8 | map_action_script_floor4 | 100 | No args/result; dispatcher call, jr ra/nop. Move unchanged. | Exact retained: 100% |
| 346a8 / 38c | map_floor5_transition_cutscene | 100 | No args/result; floor5 action caller; bank-0 item10 removal, camera begin/step, occupancy, effect/render loops. Typed camera-path argument replaces gap+8 cast only. | Exact retained: 100% |
| 34a34 / 4c | map_action_script_floor5 | 100 | No args/result; event word28010500 gates transition and world byte6801. Move unchanged. | Exact retained: 100% |
| 34a80 / 2d4 | map_event_interact | 100 | Existing KfMapEvent* source argument; three dispatcher calls; item trades, notification and dialog/event refresh. Move without signature/CFG changes. | Exact retained: 100% |
| 34d54 / 90 | map_show_screen_image | 79.25 | Existing s32 group/index source; caller supplies group0/1 and unsigned item byte. Signed div10 produces quotient and remainder; four byte stores then screen call. Own/name complete writable path, retain code-generation residue. | Non-exact, score unchanged |
| 34de4 / 904 | map_interaction_dispatch | 83.01213 | VECTOR*/SVECTOR* from player-update caller; 56 direct calls, two indirect switches, event/object interaction and all five action calls. Preserve body and unresolved pointer evidence during consolidation. | Non-exact, score unchanged |
| 36d3c / f4 | actor_transform_definition5_to6 | 100 | KfActor* a0 from floor4/definition5 caller; two 65-step loops with +/-40 Y and +/-64 yaw, definition6, map event state3. Replace only private MATRIX identity. | Exact retained: 100% |
| 384f8 / 1cc | effect_floor_deform_line | 84.92174 | Three s32 arguments from dispatcher kind52; seven unsigned fields, 4096 clamp,3900 sound threshold, grid bytes. First name/type the true table; separately move progress_start += progress_step before the clamp: retail 385a0 executes it in every iteration's branch delay slot. | Non-exact: 86.704346%, corrected CFG |

## Effect ownership revision (before the split probe)

The first delink rejects the new floor table and existing projectile vectors
in one `.data`: `56268/23` ends at `5628b`, while the vectors start at `57b88`.
The intervening 6,397 bytes are not compiler alignment padding. The original
helper reconstruction (`1914c0e`) ended at `38a38`; the dispatcher and its
RODATA/vectors were appended in `4628a89`. Restore that coherent eight-helper
boundary and give the dispatcher its own unit, without changing either profile
or function bodies. This is a testable working ownership model, not proof of an
original filename. An original `.data`/small-data split remains an alternative
requiring toolchain evidence; do not assert it or manufacture section placement.

The full eight-function six-view dossier `build/effect-data-owner-evidence.log`
was read with source/history and all constraining caller instructions. None is
vendored: these are effect-record policies calling SDK math/audio providers.
All helpers are reached by the dispatcher except rotate-scale, reached by the
trail helper. The dispatcher has one proven caller, effect_pool_sweep, with no
arguments. Its 49 switch pointers and unresolved indirect jump are not promoted.

| VA / extent | Function | Before % | Individual evidence / split hypothesis | Final verdict |
| --- | --- | ---: | --- | --- |
| 37fbc / 24 | effect_magic_power | 100 | Record pointer in a0 at all five calls; byte type mask16, player magic lhu or5; leaf jr/nop. Keep helper. | Exact retained: 100% |
| 37fe0 / 2b8 | effect_projectile_update_3d | 96.75288 | Two dispatcher calls pass the two velocity pointers and limits40/60; frame144, ten calls, projectile/impact/pitch CFG. Keep source signature and body. | Non-exact, score unchanged |
| 38298 / 260 | effect_projectile_update_2d | 99.93421 | Dispatcher passes speed6500/limit40; frame120, nine calls, signed velocity and unsigned life loads. Keep helper unchanged. | Non-exact, score unchanged |
| 386c4 / 68 | effect_scatter_triple | 76.92308 | Dispatcher passes stack triple/current direction; three rand calls, lhu/sh at0/2/4, frame24. No signedness/codegen change. | Non-exact, score unchanged |
| 3872c / 90 | effect_rotate_scale_offset_y | 100 | Trail caller supplies two pointers, sign-extended angle and scale; lh offsets0/4, multiply/shift12, RotMatrix/ApplyMatrix, frame80. Keep helper. | Exact retained: 100% |
| 387bc / f8 | effect_spawn_trail_kind13 | 100 | Four dispatcher calls, byte ID and angles +/-1774,+/-1824 with distances4000/8000. Divide by800 and record stride60; frame56, rotate/construct calls. Keep helper. | Exact retained: 100% |
| 388b4 / 184 | effect_spawn_ground_kind6 | 100 | Three calls pass angles1024/3072 and selectors1/2/255; frame64, sine/cosine, divide2000/grid/construct. Keep helper. | Exact retained: 100% |
| 38a38 / 180c | effect_update_dispatch | 37.918777 | No-arg sweep caller, frame168,70 direct calls, switch4..52. Move whole body and its RODATA12cf8/c4 and vectors57b88/10 unchanged. | Non-exact, score unchanged |

## Verification plan

First rebuild/delink the ownership model without the progress fix and compare
all function scores/ordered referents. Then apply the independently proven CFG
correction and compare from the first real divergence. Check complete source
DATA sizes using independent compiler queries, complete initialized bytes,
section placement and all ten data HI16/LO16 pairs. Add bounded retail controls
for the mutable filename and all five floor lines, including progress crossing
0/3900/4096 and both step signs; do not boot the game. Run focused matches,
full build, lint/tests/diff checks and flake checks for any tooling changes.
Keep strict data failures and unrelated banked functions intact.

## Final verdict

The map consolidation and effect helper/dispatcher split are retained as working
ownership, with original boundaries still unproved. There remain 112 source TUs
and four independent SDK data contributions. No function, vendor classification,
profile, banked baseline, alignment override or relocation evidence level is
removed or relaxed. Per-function final verdicts appear in both tables above.

All six initializers occupy their complete 187 retail bytes, including the
camera sentinel's full record and the filename NUL. Source and target agree on
each private symbol's size, offset and binding; the pinned compiler independently
measures every one of the 107 DATA claims correctly. The entire map-script
`.data` is 120 bytes and passes strict comparison **including retail placement**.
The actor MATRIX (32 bytes) and floor table (35 bytes) have equal full payloads
but fail real source alignment: both retail bases are 8 modulo16 while their
emitted sections require16. These are not strict data matches. The separate
16-byte projectile-vector contribution remains unchanged and also fails source
alignment; the target can independently relink all four affected units.

The map `.rodata` remains non-exact: its first switch row at +28h encodes
`.text+1524` in the target versus `.text+1514` in source, and the actual source
section alignment8 conflicts with retail base12a54. No table row is adjusted
to fit the non-exact dispatcher. The actor and effect dispatcher retain their
pre-existing switch-addend residues. Removing the helper prefix from the effect
dispatcher legitimately rebases both sides' switch offsets; it does not make
their different destinations exact.

The floor correction advances progress on every cell, before clamping. Retail
and compiled code both execute that addition in the BGEZ delay slot. Its native
score moves **84.921740 -> 86.704346%**; it remains non-exact. The first remaining
raw difference is the 80-byte compiled frame versus72 retail. The compiled
body is440 bytes versus460 retail, with different count-loop selection, field
load order and register/frame scheduling. The sound call, unsigned seven-byte
record view, grid referent, thresholds0/3900/4096 and arithmetic meaning remain
intact. No compiler-backend cause or whole-function closure is claimed.

Seven new tests cover the six owners and all324 bytes of their surrounding
census partition, complete source/target payloads, independent sizes/private
bindings, strict placement rejection, and twelve reviewed HI/LO round-trips
(the ten recovered-object references plus the two unchanged projectile rows).
The neighboring SDK reference at3adf8 stays candidate. Bounded retail controls
exercise250 floor-line scenarios (all five records, ten starting progress
values, five positive/negative/zero steps) and90 path scenarios (five floors,
two groups, nine unsigned-byte indices). They verify the whole guarded grid,
ordered sound coordinates/arguments, whole filename plus NUL, and untouched
neighbor/table bytes. Only individual retail bodies execute with explicit
service hooks; no source placement restriction is bypassed to run a candidate.

All484 function-score rows were compared with the pre-edit snapshot: only the
floor helper changes. All **360/471 exact game functions** and thirteen exact
vendor controls remain exact. The108 unrelated source units retain every
`.text/.rodata/.data/.sdata` byte and ordered relocation. Before the separate
progress correction, the ownership/split build retained all484 scores unchanged.

Source/header `DAT_` occurrences fall **100 -> 84**. Curated unresolved identity
names remain2749: the newly resolved gap/string bindings had no identity rows.
GAME reached ownership changes83 ->88 source ranges and356 ->354 config-only
ranges (merging two RODATA owners affects the range count). Unmatched reached
config ranges across images fall **622 ->620**. These are range counts, not a
proof that all computed or indirect references have been found.

Strict whole-source-data units change **7/60 ->6/60** (GAME4/40, OPEN2/19,
PSX0/1). The former passing floor-script literal unit is now part of the merged
map unit, whose switch data remains non-exact; the split effect helper is not
a new passing unit. Independent SDK contributions remain4/4. Target relink stays
110/116, with the same six other conflicting-section-base units. The full build
runs without a compiler or delinker error and remains red on the strict data,
reachability and placement gates. Full reachable-byte and linked-image equality
are still unproved.

Verification logs are `build/map-script-*`: forced focused compiles for all four
affected units, fresh native reports, whole-corpus comparison, full build,
repository lint/tests and diff checks. All609 local tests pass without skips;
Ruff and diff checks pass. `nix flake check -L` passes, including609 tests
with112 expected local-artifact/tool skips and the native compiler/objdiff
controls. Nothing is newly banked.
