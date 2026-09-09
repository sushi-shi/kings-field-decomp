# Compile and link executables with the Psy-Q tools

Inside `nix develop`, after `kf init`, run:

```sh
kf link
```

There is one executable build path:

```text
C source --GCC--> assembly --ASPSX--> Psy-Q objects
SDK startup source --------ASPSX--> overlay startup object
                                      + original SDK .LIB files
                                           |
                                        PSYLINK
                                           |
                                          CPE
                                           |
                                         CPE2X
                                           |
                                          EXE
```

GCC uses each unit's source, includes, defines and compiler flags from the
manifest. ASPSX 1.07 reads that compiler output directly, with DOS text line
endings. PSYLINK 1.17 consumes the resulting LNK objects and the original
Psy-Q library files. CPE2X writes the final PS-X EXE. The build does not change
any bytes in the object, linked CPE or converted EXE.

For PSX the original library inputs are `LIBSN.LIB` and `LIBAPI.LIB`.
The native linker selects their members and resolves their symbols. Its
ordinary command file specifies the program's load origin, input objects,
libraries, BSS groups and entry symbol. It does not place individual game functions or
rewrite their addresses.

GAME and OPEN additionally assemble `src/sdk/overlay_start.s`, a reconstruction
of their identified minimal NONE2 SDK startup family. The report records its
provenance and hashes separately from compiled game units. The entry sets `gp`
to the start of `.sdata` and jumps to `main`; the preceding `__main` stub returns
without initialization. This SDK assembly is excluded from game progress.

The command file places `.sbss` and `.bss` in an uninitialized `bssdata` group.
Their addresses are allocated, but their contents are not load records. GAME's
four BIOS-wrapper spellings resolve through native `ALIAS` directives:
`InitCARD2` → `InitCARD`, `StartCARD2` → `StartCARD`, `StopCARD2` → `StopCARD`,
and `erase` → `delete`. LIBAPI members A74/A75/A76/A69 contain the matching
BIOS B0 selectors `4a/4b/4c/45`; no wrapper instructions are added. Both
directives are documented in the
[PSYLINK manual, chapter 13](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/sdevtc.pdf#page=154)
and tested against the pinned PSYLINK 1.17.

The linker combines each object's code and data and resolves references
between objects. For example, a C call to `Load` refers to a library symbol;
the linker finds the implementation in the SDK and encodes the call target.

CPE2X supplies the executable header and rounds its load area to a 2048-byte
sector boundary. Space after the linked initialized data is **padding**.
PSX has 560 initialized bytes, so its load sector has 1488 padding bytes.
The separate 2048-byte header makes the whole executable 4096 bytes.

## Inputs, outputs and failures

`build/link/psx/` contains compiler output (`U0000.I`, `U0000.S`), the native
object (`U0000.OBJ`), `LINK.LNK`, native tool logs, `PSX.CPE`, `PSX.SYM`,
`PSX.MAP`, `PSX.EXE` and `comparison.json`. The report lists commands and
source, object, library and tool hashes. Every CPE load record is checked
against the converter's unchanged output before the retail comparison.

The verified retail EXE is used **only for comparison**. It does not supply
header bytes, missing initialized data, missing startup objects or tail bytes.
Missing source definitions fail the native link. A failed build removes an
older executable so it cannot be mistaken for the current result.

The custom ELF section rewrite, SDK-to-ELF link adapter, retail-header template,
inventory data fallback and inferred-padding mode have been removed. The
existing ELF-based `kf match` analysis remains separate from `kf link`; its
objects are not executable-build inputs.

`kf link --image game` and `kf link --image open` use the same direct path;
`kf link` attempts all three. A successful exit means native compilation,
assembly, linking and conversion succeeded. `comparison.file_equal` reports
unmasked complete-file equality. Build success alone is not an exactness claim.

All three images now link through the native tools. The missing game storage
is explicitly defined in C, using the shared types and reviewed initializers.
There are no unresolved-symbol fallbacks.

| Image | Native EXE bytes | Retail EXE bytes | Differing file bytes |
| --- | ---: | ---: | ---: |
| PSX | 4,096 | 4,096 | 66 |
| OPEN | 153,600 | 155,648 | 133,372 |
| GAME | 286,720 | 288,768 | 249,268 |

These are complete-file comparisons, including relocated addresses, headers
and padding. Neither overlay is exact or tested as a running game. Native
load-record payload verification covers 560 PSX, 150,405 OPEN and 283,660 GAME
bytes. The next matching work is data ownership/allocation order, section
layout, remaining function residues and SDK/library ordering.

The full PSX header, padding and converter investigation is consolidated in
[the executable byte-provenance record](patterns/executable-byte-provenance.md).

## Layout-tolerant executable comparison

`kf link` also compares moved byte regions and refreshes the second generated
README block, `executable-score`, after linking. To compare existing EXEs and
refresh that block without rebuilding them, run:

```sh
kf link --compare-only
```

Both forms accept `--image psx|game|open`. The README keeps one row per image
and a byte-weighted total for the available images. It reads saved reports
only when their candidate hashes match EXEs still on disk and their retail
hashes match the configured retail identities. Missing, changed or failed
outputs receive no cached score. The executable and function blocks share
the same update lock and preserve each other.

The report is a content heuristic called `unique-byte-islands-v1`, not a
reimplementation of objdiff or a claim of function identity:

1. Find exact 16-byte windows that occur once in each load area. Coalesce
   overlapping windows with the same displacement, then select the longest
   nonoverlapping runs. Each byte can be assigned at most once in each image.
2. Extend selected runs through equal, unused neighboring bytes. Also keep
   exact file-boundary prefixes and suffixes, so repeated padding does not
   require a unique seed.
3. Join adjacent anchors only when they are neighbors in both images. Each
   intervening gap is at most 128 bytes. A bounded byte-sequence comparison
   accepts gaps with at least 60% similarity, or edits of at most 16 bytes
   per side when the two flanking exact runs supply at least 85% similarity
   across that local span. Unchanged bytes inside an accepted gap earn
   credit; its edited, inserted or deleted bytes do not.
4. Sum equal byte pairs across all selected islands. Report similarity as
   `200 * equal_pairs / (retail_load_bytes + candidate_load_bytes)`. All
   unpaired bytes remain in the denominator. The nonzero score uses the
   same formula with zero-valued bytes removed from both counts.

Islands may move or reorder, but no address, opcode, register, immediate,
constant or data byte is masked. A changed pointer still loses byte credit;
similar pointer encodings do not prove the same referent. Short or repetitive
regions without suitable anchors can remain unpaired even when shared.
The algorithm is conservative and deterministic, rather than an optimal
global alignment or an instruction-semantic comparison.

The island scores cover the complete EXE load areas, including sector
padding, and exclude the 2048-byte headers. The separate fixed-offset
comparison still covers the complete files and is the only executable
equality check. Neither island similarity nor island coverage changes
function scores, banking criteria or the full data-layout gates.

Normal links write the detailed score into each image's `comparison.json`
and create `comparison.html`. Comparison-only runs write
`fuzzy-comparison.json` and `fuzzy-comparison.html`, preserving the original
native-build report and executable. The standalone HTML contains a movement
map, filterable islands with both addresses and sizes, and the largest
unanchored regions. The JSON retains all islands, complementary unanchored
ranges, parameters, counts and input hashes. These generated files stay
under `build/link/{psx,game,open}/` and are not committed.

## Overlay link plan and evidence

The initial native runs reported 61 OPEN and 2694 GAME diagnostic references;
these are repeated uses, not counts of distinct missing objects. OPEN lacked ten
game data definitions and the entry symbol. GAME lacked 55 game data definitions,
four SDK name aliases and the entry symbol. Existing C bodies compile and
assemble successfully. Source ownership is WIP; this work does not establish
original translation-unit boundaries or historical link layout.

The first storage pass covers OPEN's already typed BSS owners:

| Image/address | Definition and extent | Source owner and evidence |
| --- | --- | --- |
| OPEN `800377a0` | `cd_search_file`, `CdlFILE`, 24 bytes | `open/resources.c`; `CdSearchFile` destination, sector-rounded size; GAME homolog |
| OPEN `800377b8` | `cd_path_buffer`, 80 chars | `open/resources.c`; prefix/name/suffix assembly, adjacent search result |
| OPEN `800446c8` | `map_collision_flag_grid`, `KfMapGrid`, 10000 bytes | `open/resources.c`; fourth 2500-word MIXA0 grid copy |
| OPEN `80046df8` | `map_cell_orientation_grid`, `KfMapOrientationGrid`, 10000 bytes | Same loader, third grid copy; shared typed grid accessors |
| OPEN `8006e260` | `map_floor_height_grid`, `KfMapGrid`, 10000 bytes | Same loader, second grid copy |
| OPEN `80070978` | `map_collision_grid`, `KfMapCollisionGrid`, 10000 bytes | Same loader, fifth grid copy |
| OPEN `800730a0` | `map_cell_attribute_grid`, `KfMapAttributeGrid`, 10000 bytes | Same loader, first grid copy |
| OPEN `80075898`, GAME `800a0240` | `memory_system_heap_start`, `u8 *`, 4 bytes | Shared `game/memory.c`; captures arena end plus one and feeds `InitHeap` |
| OPEN `8007589c`, GAME `800a0244` | `memory_system_heap_size`, `s32`, 4 bytes | Shared allocator; subtracts heap start from `801f8000`, feeds `InitHeap` |
| OPEN `80075928` | `primitive_allocation_count`, `u32`, 4 bytes | `open/render_init.c`; allocator increments, frame begin resets |

These definitions use existing shared declarations and curated BSS extents;
no function body changed. The rebuilt comparison retained every function score.

The startup evidence is separate from game progress. GAME `8003ac54` and OPEN
`8001aa74` contain the eight-byte `__main` return stub; the following sixteen
bytes set `gp` and tail-jump to `main`, including a `nop` delay slot. The
`functions_vendored.tsv` NONE2 signature identifies this 24-byte SDK assembly
family. The present LIBSN supplies a different, larger startup. The preserved
NONE2 sample and NONE3 object from the later SDK collection also differ and
cannot be substituted. A source reconstruction of the identified minimal SDK
assembly retains symbolic `main` and `.sdata` references and remains excluded
from game matching. The native entry name is `__SN_ENTRY_POINT`, not the
inventory's address label `start`.

### GAME storage pass

Each row adds a definition using the existing shared type, without changing a function body. The two heap words are covered by the shared OPEN pass above. Initialized selectors and named path arrays live in the retail writable-data band; their definitions and shared declarations therefore use writable objects, while function parameters retain const views. Five tail-page words change from load to BSS based on the write-before-read and allocation evidence below.

| GAME address | Object (bytes) | Source owner | Evidence |
| --- | --- | --- | --- |
| `80055af0` | `map_resource_path` (12) | `resources.c` | mutable floor/path buffer; retail B0 followed by backslash and nine zero bytes |
| `80055f68` | `render_light_matrices` (192) | `render_frame.c` | six complete SDK MATRIX objects passed to SetLightMatrix; nine signed halfwords and three translation words each, raw initializers verified |
| `80056034` | `save_main_file_path` (26) | `save_system.c` | named global path in the initialized-data band; literal bytes including NUL verified |
| `80056050` | `save_temporary_file_path` (26) | `save_system.c` | named global path in the initialized-data band; literal bytes including NUL verified |
| `8005606c` | `talk_image_path_template` (20) | `save_system.c` | named global path in the initialized-data band; literal bytes including NUL verified; dialogue patches template in place |
| `80056188` | `gameplay_sound_ref_0` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `8005618b` | `gameplay_sound_ref_1` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `8005618e` | `gameplay_sound_ref_2` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `80056191` | `gameplay_sound_ref_3` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `80056194` | `gameplay_sound_ref_4` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `80056197` | `gameplay_sound_ref_5` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `8005619a` | `gameplay_sound_ref_6` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `8005619d` | `gameplay_sound_ref_7` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `800561a0` | `gameplay_sound_ref_8` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `800561a3` | `gameplay_sound_ref_9` (3) | `map_object.c` | existing curated selector between entries 8 and 10; raw bytes `08 00 2f`; retain the full initialized family even though no compiled caller currently needs entry 9 |
| `800561a6` | `gameplay_sound_ref_10` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `800561a9` | `gameplay_sound_ref_11` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `800561ac` | `gameplay_sound_ref_12` (3) | `map_object.c` | independently addressed packed program/tone/note selector; raw three-byte initializer verified |
| `80057b6c` | `menu_item_model_allocation_pending` (4) | `menu_runtime.c` | native zero word before BSS boundary; load/release functions assign enum states |
| `80057b78` | `memory_card_root_path` (6) | `save_system.c` | named global path in the initialized-data band; literal bytes including NUL verified |
| `80057b80` | `boss_death_loop_sound` (3) | `actor.c` | boss death loop passes this three-byte selector to audio |
| `80057b84` | `audio_voice_slot_index` (4) | `audio.c` | retail starts managed voice ring at index nine |
| `80057e78` | `player_death_saved_fog_near` (4) | `player_death.c` | death entry writes snapshot before forward/reverse fades; eight-byte tail-BSS spacing |
| `80057e90` | `memory_card_io_end_event` (4) | `save_system.c` | memory_card_initialize stores OpenEvent result before first EnableEvent; subsequent uses require initialization; eight-byte tail-BSS spacing |
| `80057e98` | `memory_card_timeout_event` (4) | `save_system.c` | memory_card_initialize stores OpenEvent result before first EnableEvent; subsequent uses require initialization; eight-byte tail-BSS spacing |
| `80057ea0` | `memory_card_new_device_event` (4) | `save_system.c` | memory_card_initialize stores OpenEvent result before first EnableEvent; subsequent uses require initialization; eight-byte tail-BSS spacing |
| `80057ea8` | `memory_card_error_event` (4) | `save_system.c` | memory_card_initialize stores OpenEvent result before first EnableEvent; subsequent uses require initialization; eight-byte tail-BSS spacing |
| `80058060` | `player_death_saved_color_matrix` (32) | `player_death.c` | ReadColorMatrix fills full matrix before fade |
| `80058080` | `cd_search_file` (24) | `cd_file.c` | CdSearchFile destination; exact SDK structure and OPEN homolog |
| `80058098` | `cd_path_buffer` (80) | `cd_file.c` | prefix/name/suffix assembly, adjacent CdlFILE |
| `80059738` | `audio_sequence_table` (344) | `audio.c` | SsSetTableSize workspace for two sequences, one track; OPEN homolog |
| `80065098` | `player_rotation_snapshot` (8) | `player_core.c` | game loop passes snapshot to player_update_transform_snapshot |
| `800650a0` | `player_position_snapshot` (16) | `player_core.c` | same transform snapshot call; full VECTOR width |
| `800650b0` | `player_level_growth_table` (480) | `player_core.c` | common_resources_load copies forty 12-byte growth records |
| `800668d8` | `save_header_buffer` (4) | `save_system.c` | save workspace pointer rebound to arena or stack header |
| `800668e0` | `save_payload_buffer` (4) | `save_system.c` | save workspace pointer at header plus 0x280 |
| `800668e8` | `map_collision_flag_grid` (10000) | `resources.c` | map resource chunk copies 2500 words; shared typed 100x100 accessors |
| `80069018` | `map_cell_orientation_grid` (10000) | `resources.c` | map resource chunk copies 2500 words; shared typed 100x100 accessors |
| `8006b730` | `cd_file_table` (1600) | `cd_file.c` | 80 file records, each old-layout CdlFILE occupies 20 bytes |
| `8006bd98` | `actor_state` (11080) | `actor_pool.c` | definition table, actor pool and current/target pointers share one base |
| `8006e8e0` | `map_object_state` (9640) | `map_object_pool.c` | definition table and pool derived from one base; existing complete-object layout |
| `80070e92` | `map_object_effect_sequence_160` (2) | `map_object_pool.c` | wrapping halfword sequence for the named reserved map-object slots |
| `80070e94` | `map_object_effect_sequence_170` (2) | `map_object_pool.c` | wrapping halfword sequence for the named reserved map-object slots |
| `80070e96` | `map_object_effect_sequence_180` (2) | `map_object_pool.c` | wrapping halfword sequence for the named reserved map-object slots |
| `80095868` | `audio_state` (144) | `audio.c` | voice-ID loop and shared sequence/listener fields prove aggregate |
| `800958f8` | `game_exit_code` (4) | `game.c` | main-loop exit state, copied into the loader argument block |
| `80095900` | `map_floor_height_grid` (10000) | `resources.c` | map resource chunk copies 2500 words; shared typed 100x100 accessors |
| `80098018` | `map_collision_grid` (10000) | `resources.c` | map resource chunk copies 2500 words; shared typed 100x100 accessors |
| `8009a748` | `map_cell_attribute_grid` (10000) | `resources.c` | map resource chunk copies 2500 words; shared typed 100x100 accessors |
| `8009ff10` | `weapon_records` (704) | `equipment.c` | loader copies exactly 176 words |
| `800a01d0` | `collision_target` (32) | `collision.c` | collision query output transform and radius, shared typed object |
| `800a0248` | `armor_records` (1176) | `equipment.c` | loader copies 42 28-byte armor/accessory records |
| `800a0768` | `DAT_800a0768` (4) | `render.c` | render initialization clears the word; retain unresolved semantic identity |
| `800a0780` | `player_state` (224) | `player_core.c` | save payload copies exactly 0xe0 bytes; shared field family |

The map-object startup clear spans sixteen bytes beyond the currently modeled aggregate, into the neighboring counters/gaps. This existing ownership/layout uncertainty is retained; a successful native link does not prove runtime equivalence or original BSS ordering.

The specific gameplay sound selectors belong with `map_object.c` in this WIP
source organization: its update paths directly use seven selector identities,
with the remaining consumers in map scripts, items, effects and warp handling.
The generic `audio.c` API accepts selector pointers and names none of these
constants. The final definitions therefore move to the primary consuming
module. All thirteen packed retail records are retained, including currently
unreferenced selector 9. The probe aligns individual C structs more strongly
than this retail run; whether the original was one array remains unresolved.

## Verification and limits

The full rebuild compared 484 functions, including thirteen source-verified vendor functions. Every score is unchanged from the pre-edit snapshot: 460 remain exactly 100%. The game-only totals remain 340/362 GAME, 106/108 OPEN and 1/1 PSX. No new function was banked.

All 22 newly defined initialized objects match their retail extents and raw bytes in the freshly compiled objects. This includes every sound selector, the six light matrices, path arrays and initialized state words; it does not establish their linked positions.

The full repository suite passes all 739 tests with local retail/build inputs. `ruff check scripts tests`, `git diff --check` and `nix flake check -L` also pass. The isolated flake test run skips 140 checks that require local retail or build artifacts; those checks run in the full local suite.

The executable-island controls cover reordering, small edits, insertions/deletions, duplicate-copy accounting, unrelated content, padding, changed address encodings and malformed EXEs. README controls cover hash freshness, weighted totals, preservation of the function block and repeatable updates. A full native link and subsequent comparison-only pass give identical island reports for all three images; the latter leaves native EXEs and build-provenance reports unchanged. The full build also preserves the generated executable-score block. These integration checks are recorded under `build/link/island-audit/`.

The native executable controls check the minimal startup opcodes and symbolic entry, all four BIOS aliases, initialized zero data, exclusion of an 8192-byte BSS buffer, unchanged original archives and rejection of unresolved references/stale output. None executes a complete game image.

The existing function oracles now explicitly opt into named-object data bindings where merged modules own data from separate retail regions. Each object keeps its compiled extent and initializer; ambiguous section-relative references, unknown objects, wrong extents and overlapping initialized objects are rejected. The returned program records `data_binding=objects`. This isolates function semantics and does not prove section placement. Default oracle linking still requires a consistent section base. This facility is not used by `kf link`, the delinker or the full data-layout checks.

`kf build` still exits unsuccessfully on data layout and incomplete ownership. It reports 0/1 PSX, 3/20 OPEN and 8/44 GAME data-owning units matching; target placement verifies 1/1 PSX, 32/38 OPEN and 63/77 GAME units. Adding complete storage exposes more previously untested allocation-order and section-base differences. Equal bytes or unchanged function scores do not waive these failures. Reports are retained under `build/link/overlay-link-audit/`; native commands and hashes remain in each image's `comparison.json`.
