# Floor-item and map-event byte provenance

This follow-up examines nine entries in the [unknown-field ledger](../unknown-field-review.tsv):
`KfFloorItemPlacement.unknown_03`, `KfFloorItem.unknown_03`,
`KfMapEventDefinition.unknown_0b/unknown_0c`, and
`KfMapEvent.unknown_0c/unknown_0d/unknown_11/unknown_22/unknown_42`.
It follows retail pointer arithmetic,
interior pointers, wide accesses and saved-world transport. All nine names and
layouts remain unchanged: the evidence identifies transport and a possible
out-of-range dialogue alias, but no original gameplay meaning.

These are game placement and runtime records. Their SDK/file-format boundaries
are covered in [the format provenance audit](unknown-field-format-provenance.md).
An SDK object elsewhere in an enclosing owner does not identify these bytes.

## Floor-item source and runtime copy

GAME takes 12-byte placements from chunk 3 of `B1..B5/MIXA.DAT`;
OPEN takes the same grammar from chunk 3 of `B0/MIXA0.`. The resource loaders
cast the chunk payload and pass it to `item_load_floor_placements`, which
expands each source row into a 24-byte `KfFloorItem`.

| Image | Raw byte transport | Runtime owner |
| --- | --- | --- |
| GAME | `lbu` at `0x80020bf4` reads placement +3; `sb` at `0x80020bfc` writes item +3 | 64 items beginning at `0x80095098` |
| OPEN | `lbu` at `0x8001988c` reads placement +3; `sb` at `0x80019894` writes item +3 | 64 items beginning at `0x8006da40` |

The GAME and OPEN traversal loops derive interior item pointers and pass them
to their respective `render_floor_item` bodies. The GAME renderer reads
appearance +2 at `0x8001ee14/0x8001eea0` and frame +20 at
`0x8001ee74/0x8001ee9c`; OPEN reads appearance at
`0x80019178/0x80019204` and frame at `0x800191d8/0x80019200`.
The decoded coordinate accesses cover +4, +8 and +12, while the initial
halfword identifies the sprite. None of these reads covers +3, and the frame
updates do not write it. The enclosing graphics state is cleared during
startup; that whole-owner initialization supplies no behavioral meaning.

The 131 active source placements comprise 117 rows across B1–B5 and 14 OPEN
rows. The byte takes values 8, 28, 32 and 64; every OPEN row has 64. Sprite 0
with appearance 4 occurs with 28, 32 and 64, so the value is not determined by
that sprite/appearance pair. These observations do not establish brightness,
radius or an animation role. No fixed-offset or wide behavioral read of +3
was found along the traced loader/render paths. That is a bounded result,
not a whole-image unused-field proof.

## Event expansion and saved-world transport

GAME chunk 7 of each `B1..B5/MIXA.DAT` contains eight 24-byte event definitions.
`map_resources_load` passes a cast chunk payload to `map_event_pool_load`.
The runtime array begins at `GAME:0x8009db88`, with eight 68-byte records.
During the relevant loader sequence, `s1` is definition +18 and `s0` is
runtime event +30; interpreting the negative offsets against those biased
pointers gives these mappings.

| Source field | Load instruction | Destination field | Store instruction |
| --- | --- | --- | --- |
| definition +0x0b | `0x8003397c: lbu ..., -7(s1)` | event +0x0c | `0x80033984: sb ..., -18(s0)` |
| definition +0x0c | `0x80033988: lbu ..., -6(s1)` | event +0x0d | `0x80033990: sb ..., -17(s0)` |

Both source fields are zero in all 40 resource rows, including inactive rows.
Copying zeros is not evidence that the fields are padding.
`map_world_state_persist` additionally loads event +0x0d at `0x80035bfc`
and stores it at `0x80035c08`; `map_restore_floor_state` restores it with the
store at `0x80035f10`. Each floor's saved-event block has one marker byte
followed by eight seven-byte records: state, stage limit, stage, page,
the indexed last-page value, delay, and this unknown byte. Within the
saved-world region, its offset is `1700 * (floor - 1) + 17 + 7 * event_index`.
The direct saved fields do not include event +0x0c.

The loader's unaligned five-byte dialogue-page copy is a useful negative
control. `lwl` at `0x80033958` uses source +8 and `lwr` at `0x8003395c`
uses source +5: together they produce bytes +5 through +8. The byte load
at `0x80033960` supplies +9. Although an aligned memory fetch underlying
`lwl` can include +0x0b, that byte is discarded by the instruction's lane
selection. It is not a second semantic read of `unknown_0b`.

## Four real indexed aliases

The source expression `event->dialogue_pages.last_page[stage - 1]` becomes
an effective address of `event + stage + 2`. It therefore cannot be audited
as a fixed five-byte member access without examining the stage value.

| GAME function | Raw indexed access | Direction |
| --- | --- | --- |
| `map_event_interact` | `lbu` at `0x80034d04` | read |
| `map_event_pool_update` | `lbu` at `0x80035a34` | read |
| `map_world_state_persist` | `lbu` at `0x80035be4` | read into saved-world storage |
| `map_restore_floor_state` | `sb` at `0x80035ef0` | write from saved-world storage |

Stages 1–5 select event bytes +3 through +7. Stage 10 selects +0x0c and
stage 11 selects +0x0d. The loader initializes stage to 1 (`0x80033a84`).
All 40 resource stage limits are at most 5; the reviewed floor scripts set
limits to 2 or 5. `map_event_refresh_dialogue_stage` reads the limit and
current stage at `0x800337ac/0x800337b0`. Only when the current stage is less
than the limit does it write a new stage, bounded by that limit, at
`0x800337f0` or `0x80033814`. These ordinary updates do not create stages
10 or 11, but the refresh does not repair an already excessive stage.

Save restoration supplies a separate input domain. `save_file_read_slot`
checks directory consistency and read length, then copies the saved-world
payload into `map_runtime_state+0x22c`; the inline word-copy loop begins at
`0x8002c12c`. It does not validate dialogue stages. `map_restore_floor_state`
restores the stage byte before using it in the indexed store above, also
without a stage-range check. Thus crafted or damaged saved-world data can
select +0x0c/+0x0d through these instructions. The final direct restoration
of +0x0d overwrites a preceding stage-11 indexed write to that same byte.
This is an out-of-range alias, not evidence that either unknown field was
intended as an extra dialogue-page entry. No source change is proposed here.

The same effective-address calculation conditionally reaches the other
unresolved runtime-event fields. Stage 15 selects `unknown_11` at +0x11;
stages 32/33 select the individual bytes of `unknown_22` at +0x22/+0x23;
stages 64/65 select the individual bytes of `unknown_42` at +0x42/+0x43.
Each access is one byte, including the restoration store: this does not
establish a halfword operation or a semantic subdivision of either `u16`.
The restored stage is an unchecked byte, and larger indices can leave the
68-byte record. The formula establishes these conditional addresses, not
whole-array bounds safety, execution under normal gameplay, or intended
meanings for the target bytes.

## Interior pointer retained by the render pool

`render_map_event` forms `event +0x3c` at `0x8001f19c` and calls
`render_bind_animated_instance` at `0x8001f1b4`. The binder loads the cache
pointer through that address at `0x80020604`, stores the address itself in
`KfPoolRecord.owner_slot` at `0x80020688`, and writes the allocated cache
pointer back through it at `0x800206b4`. This is a real persistent interior
pointer and must not disappear from a member-name-only census.

The consuming `pool_record_release` loads `owner_slot` at `0x800209f4`
and writes a zero word through it at `0x800209fc`. For the selected event
caller, that store covers exactly event +0x3c through +0x3f, not +0x0c/+0x0d.
Release paths from the binder, `pool_release_all` and `pool_release_stale`
share this operation. The event rotation passed to SDK `RotMatrix` begins
at +0x34, another distinct interior range. These checked paths do not justify
closing other retained pointers or other callers with different owners.

## Audit method and remaining boundaries

A local proposal script built retail CFGs for 933 of 935 inventoried GAME
functions and 663 of 665 OPEN functions. The excluded bodies in each image
are the fragmented SDK `SquareRoot0` and `InvSquareRoot`. It propagated affine
addresses through decoded reachable blocks, direct O32 calls, return values,
stack spills and discovered global pointer stores. The input placement
pointers were seeded at the two loader boundaries; fixed runtime bases were
recognized independently of admitted relocation rows. It reported accesses
in 29 GAME and three OPEN functions, with fixed points after three and two
rounds. These are function counts, not counts of proved complete consumers.

The proposal is not a sound whole-program alias analysis. Function-inventory
gaps, unresolved GP bases, indirect successors, addresses outside the selected
families and undiscovered pointer encodings remain coverage boundaries.
The floor-action switch at `GAME:0x80035668` and floor-ambient switch at
`GAME:0x80035af0` retain unresolved CFG successors. Their bounded floor-index
calculations and table bases (`0x80012bcc`, `0x80012be4`) are visible in
retail; the pointer rows remain **candidate** inventory entries, not promoted
control-flow evidence. The SDK `rand(void)` wrapper jumps to BIOS A0 function
0x2f; a stale pointer in an argument register is not a declared argument, and
the BIOS body was not inspected. Those facts limit absence claims.

The six-view dossiers (`addr`, block disassembly, callers, callees, strings,
strict match) cover 47 selected functions, including source loaders, rendering,
behavior, persistence, pool release and save I/O. The instruction witnesses
above were inspected manually after the proposal, including the unaligned
byte-lane counterexample and the retained `owner_slot` chain. Local generated
artifacts are `build/unknown-family-*-raw-accesses.json`,
`build/unknown-family-dossiers/` and
`build/unknown-family-resource-correlations.json`; they are not curated inputs
or committed results. To reproduce an individual witness, run `kf init` and
then, in `nix develop`, `kf sema --image game disasm FUNCTION --blocks`
(use `--image open` for OPEN). No source, ABI, identity or relocation change
was made for this follow-up.

## Final function verdicts

Every name below has a six-view dossier. Verdicts concern the reviewed byte
families and paths, not complete function reconstruction or a new exact-match
claim. A checked nonoverlap means the inspected access addresses and widths
miss the unknown bytes; it does not close unrelated indirect consumers.

| Image | Function | Bounded verdict |
| --- | --- | --- |
| GAME | `RotMatrix` | SDK boundary: selected event input is rotation +0x34; XYZ halfword reads miss the reviewed unknowns. |
| GAME | `actor_transform_definition5_to6` | Checked nonoverlap: direct event writes disable state byte +0 in two rows; actor transformations are a separate owner. |
| GAME | `audio_play_spatial` | Checked nonoverlap: selected interior input is event reference position +0x24, with three coordinate-word reads. |
| GAME | `collision_query_world` | Checked nonoverlap: selected event collision path reads radius +0x20 and copies reference-position words beginning +0x24. |
| GAME | `game_main_loop` | Whole-owner initialization: startup clears include graphics items and the event runtime region; no behavioral name follows. |
| GAME | `item_load_floor_placements` | Direct transport: placement +3 becomes runtime item +3. |
| GAME | `map_action_script_floor2` | Checked nonoverlap: state byte +0 and dialogue word +8..+0x0b; word mask does not include +0x0c/+0x0d. |
| GAME | `map_action_script_floor3` | Checked nonoverlap: dialogue trigger word +8..+0x0b. |
| GAME | `map_action_script_floor5` | Checked nonoverlap: dialogue trigger word +8..+0x0b. |
| GAME | `map_ambient_script_floor2` | Checked nonoverlap: dialogue stage/page +9/+0x0a and reference-position interior pointer. |
| GAME | `map_event_advance_animation_blocking` | Checked nonoverlap: updates animation phase +0x12 and invokes rendering. |
| GAME | `map_event_distance_to_point` | Checked nonoverlap: reference-position X/Z words +0x24/+0x2c. |
| GAME | `map_event_interact` | Dynamic stage alias: indexed byte read can reach unresolved event lanes for excessive stages; explicit script stage limits remain 2/5. |
| GAME | `map_event_pool_find_overlap` | Checked nonoverlap: state +0, radius halfword +0x20 and the distance helper. |
| GAME | `map_event_pool_find_target_in_cone` | Checked nonoverlap: state +0 and reference-position X/Z words; returns a selected row index. |
| GAME | `map_event_pool_load` | Direct transport: definition +0x0b/+0x0c become event +0x0c/+0x0d; byte-lane analysis excludes a false second +0x0b read. |
| GAME | `map_event_pool_update` | Dynamic stage alias plus open indirect successor: indexed last-page read; floor-ambient switch still has candidate pointer rows. |
| GAME | `map_event_refresh_dialogue_stage` | Stage-domain evidence: bounds newly updated stages by the limit but leaves an already excessive stage unchanged. |
| GAME | `map_event_set_current` | Pointer transport: stores the selected event pointer in the runtime current-event slot for behavior helpers. |
| GAME | `map_event_show_person_image` | Checked nonoverlap: character ID byte +1 selects the image. |
| GAME | `map_event_update_animation_loop` | Checked nonoverlap: phase +0x12, with a distinct reference-position audio pointer. |
| GAME | `map_event_update_wander` | Checked nonoverlap: movement/phase/collision/position/rotation fields; BIOS rand remains an external boundary. |
| GAME | `map_interaction_dispatch` | Event index and behavior/render-field dispatch reviewed; final floor-action switch retains an unresolved successor. |
| GAME | `map_refresh_dialogue_stages` | Pointer transport: checks state +0 and passes each selected row to the stage-refresh helper. |
| GAME | `map_resources_load` | Disc provenance: obtains MIXA chunks and passes cast item/event placement pointers to their loaders. |
| GAME | `map_restore_floor_state` | Direct +0x0d transport and unchecked dynamic byte store; final +0x0d restoration overwrites the stage-11 indexed store. |
| GAME | `map_reveal_fade` | Checked nonoverlap: state +0, reference-position Y +0x28 and rotation Y +0x36. |
| GAME | `map_world_state_persist` | Direct +0x0d transport plus indexed stage alias; stage 11 copies that byte into two saved positions. |
| GAME | `pool_allocate` | Cache-owner provenance: allocator supplies a pool record to the binder; allocation itself gives no field interpretation. |
| GAME | `pool_record_release` | Retained-pointer consumer: selected event owner_slot clears only +0x3c..+0x3f. |
| GAME | `pool_release_all` | Release transport: iterates pool records and invokes the checked owner_slot release operation. |
| GAME | `pool_release_stale` | Release transport: selects stale pool records and invokes the checked owner_slot release operation. |
| GAME | `rand` | SDK/BIOS boundary: declared zero-argument wrapper jumps to BIOS A0/0x2f; BIOS body unexamined. |
| GAME | `render_bind_animated_instance` | Retained interior pointer: reads/writes event cache +0x3c and stores its address in pool owner_slot. |
| GAME | `render_entities` | Pointer transport: derives event/item row pointers; inspected state/cell/item-coordinate reads miss the unknowns. |
| GAME | `render_floor_item` | Checked nonoverlap: sprite, appearance, coordinates and frame accesses miss item +3. |
| GAME | `render_initialize` | Owner initialization: floor-item count/material and graphics setup reviewed; no meaning for an item byte established. |
| GAME | `render_map_event` | Pointer transport: distinct rotation +0x34 and cache +0x3c interior pointers; direct model/animation/position accesses miss unknowns. |
| GAME | `save_file_read_slot` | Save provenance: length/directory checks precede whole-world copy; no dialogue-stage range check. |
| GAME | `save_file_write_slot` | Save transport: serializes the saved-world aggregate; copying it does not identify the unknown byte's role. |
| OPEN | `item_load_floor_placements` | Direct transport: placement +3 becomes runtime item +3. |
| OPEN | `opening_render_entities_and_items` | Pointer transport: derives item row pointers; inspected item X/Z reads miss +3. |
| OPEN | `opening_resources_load_scene0` | Disc provenance: B0/MIXA0. chunk payload is cast and passed to the item loader. |
| OPEN | `opening_run` | Whole-owner initialization: graphics clear at `0x800156ec` includes the item array. |
| OPEN | `rand` | SDK/BIOS boundary: zero-argument random wrapper, with BIOS implementation outside the inspected image. |
| OPEN | `render_floor_item` | Checked nonoverlap: sprite, appearance, coordinates and frame accesses miss item +3. |
| OPEN | `render_initialize` | Owner initialization: floor-item count/material and graphics setup reviewed; no meaning for an item byte established. |
