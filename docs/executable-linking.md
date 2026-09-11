# Source-to-EXE build

Inside `nix develop`, run:

```sh
kf build
```

`kf link` is an alias for the same executable builder. There is one candidate
artifact chain:

```text
C source -> CPPPSX/CC1PSX -> assembly -> ASPSX -> Psy-Q OBJ
Psy-Q OBJ + original SDK OBJ/LIB -> PSYLINK -> CPE -> CPE2X -> PS-X EXE
```

The EXE is the unchanged output of CPE2X. Retail bytes are comparison inputs
only. The build does not synthesize object sections, force addresses from
`DATA()` claims, rewrite the CPE, patch the EXE, or copy missing retail bytes.
A failed phase removes the stale EXE.

The pinned Release 2.5 media supplies PSYLINK 1.17, CPE2X, headers, libraries,
and the exact overlay `NONE2.OBJ` startup. Its ASPSX executables require the
original software key, so the active chain uses the separately hash-pinned
ASPSX 1.07 binary. The build report records every tool and input hash. This is
a reproducible source-to-EXE chain, while exact historical compiler and
assembler attribution remains open.

Overlay startup uses `BSS_START` and `BSS_END` from zero-byte boundary
declarations in `config/link/overlay_bounds.asm`. A separately pinned native
ASMPSX 2.34 assembles those declarations because the C assembler lacks named
sections and the Release 2.5 macro assembler requires a software key. PSYLINK
places the labels around `.bss`; their object contains no instructions or
storage. Startup derives its word count and heap size from RAM/stack settings.

Derived ELF objects, delinked retail modules, objdiff projects, and semantic
reports are analysis views. Source compilation in `kf analyze` and `kf try`
uses the same CPPPSX/CC1PSX/ASPSX implementation as `kf build`. The reader
translates the resulting native LNK objects for objdiff; no GNU assembler
recompiles game source and no ELF view is an input to PSYLINK.

ASPSX serialises a reference to a datum defined in the same object as a
section-relative expression, so a biased reference the compiler wrote as
`floor_entry_cells-2` reaches the object as `.data+14` and no longer names
the symbol the retail relocation names. The view restores the compiler's
spelling from the `symbol+addend` operands of the assembler source
(`data_referent_spellings`): when a section-relative patch equals one of
those spellings for a datum in that section, the relocation names that
datum with the signed addend. Bytes and addresses are unchanged; only the
referent name is, which is what the strict comparison keys on.

The shared probe enables native compiler/assembler debug metadata for private
symbols and function records. The source assembly passes unchanged except for
DOS line endings. Native COMMON reservations remain unplaced in the ELF view;
only PSYLINK assigns their executable addresses.

Current source-to-EXE status is distinct from exactness and playability:

| Property | Current status |
| --- | --- |
| PSYLINK emits PSX, GAME, and OPEN | Active through `kf build` |
| Candidate artifact path | Direct source-to-EXE with no output rewriting |
| Historical toolchain identity | Open; the usable ASPSX is separately sourced |
| Byte-identical retail images | Not achieved; SDK-owned code/data, placement, and other closure gaps remain |
| Boot to the game loading screen | Demonstrated by the smoke test below |
| Playable runtime | Current symbolic startup not run; the earlier saved GAME build had the heap overlap described below |

An SDK mismatch does not by itself prove that an executable is unplayable: a
different library revision may preserve the public API and runtime behavior.
It also cannot establish complete playability, because private object layouts,
data, callbacks, and initialization behavior can differ even when linking
succeeds. A full playability claim still requires testing input, audio, map
transitions, combat, saves, and a representative playthrough. The original
hash-verified retail disc remains the runtime reference.

### Runtime smoke test

On 2026-09-10, the three saved PS-X EXEs were substituted into a copy of the
raw retail Mode 2 disc. GAME and OPEN are one sector shorter than their retail
ISO extents, so their final sectors were zero-filled. Every changed sector's
EDC and P/Q ECC was regenerated. The resulting temporary disc had SHA-256
`d44f66478022dafb8e2352702b4df01bb7e7839a9313c213c517bd65e35561c5`;
the source retail BIN remained hash-identical at
`ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9`.

PCSX-Redux build 236 (`b745534e`) with its bundled OpenBIOS booted the linked
disc into the game's `loading...` screen at 60 FPS. Controlled hybrid discs
then isolated the failure to the saved candidate `GAME.EXE`: candidate PSX and
OPEN with retail GAME advance into play, while substituting candidate GAME
reproduces the stall.

The saved candidate GAME map places `.bss` through `0x800a7aef`, but the exact
GAME startup calls `InitHeap(0x800a0980, 0x157680)`. The ranges overlap by
`0x7170` bytes. A debugger trace observed LIBSND initialize `_ss_score` at its
candidate address `0x800a6468` to the valid pointer `0x80057638`. The render
path then reused heap storage across that address and changed the pointer to
`0x00057d7d`. `SsSeqCalledTbyT` at candidate PC `0x800425b8` subsequently
loaded from `0x00057e0d` and raised `LoadAddressError`, leaving the loading
frame onscreen.

A diagnostic-only binary patch moved the initial heap base to `0x800a7b00`
and reduced its size to preserve the original `0x801f8000` endpoint. That run
passed the observed fault, completed the opening shimmer, and reached
`save_file_cleanup_temporary`. The patch is not a source or linker fix; it
confirms the overlap as the first fatal defect. Retail keeps `_ss_score` at
`0x800a06e0` and `game_exit_code` at `0x800958f8`, both outside the heap.

This observation does not establish that the original source hardcoded the
heap address. The reconstruction did at the time of that run. A static source audit found
four program-specific cached-RAM literals: startup store and initial heap
addresses in GAME and OPEN. The shared allocator also names three RAM bounds.
Claims and comments are excluded from this count.

At GAME `800142b8/800142bc`, retail forms `800a0980` with `lui`/`ori`.
The pinned GCC 2.5.7/ASPSX control reproduces that form for the numeric argument;
an `extern char heap_start[]` argument instead emits `lui`/`addiu` with native
HI16/LO16 patches. The curated inventory has no heap-address relocation here.
This supports the current constant model under the probe, but original
relocation records are absent and a build-generated constant remains possible.
It does not justify inventing a linker referent or forcing globals to retail
addresses. The original mechanism remains unresolved.

PSYLINK produced the saved layout without output rewriting. It placed the
sections and exported BSS requests supplied by the reconstructed units and
mixed SDK inputs according to its ordinary ordering and alignment rules. The
layout mismatch must therefore be corrected through original TU and data
ownership, section and allocation class, input order, and matching SDK objects.
Forcing retail addresses or patching the linked executable would conceal that
missing source and toolchain evidence.

Existing source-to-EXE outputs can be compared again without rebuilding them:

```sh
kf link --compare-only
```

The Release 2.5 PSYLINK controls use original Psy-Q objects and libraries to
test link order, section alignment, and BSS allocation.

## Layout-tolerant executable comparison

`kf link --compare-only` compares moved byte regions in existing outputs and
refreshes the second generated README block, `executable-score`:

```sh
kf link --compare-only
```

The command accepts `--image psx|game|open`. The README keeps one row per image
and a byte-weighted total for the available images. It reads saved reports
only when their candidate hashes match EXEs still on disk and their retail
hashes match the configured retail identities. Missing, changed or failed
outputs receive no cached score. The executable and function blocks share
the same update lock and preserve each other.

The **Data modules exact** column shows complete passing source data owners
out of all compared owners, using the same strict gate as `kf verify data`.
A module counts only when every initialized section, relocation, BSS layout
and retail section placement passes. Exact functions or equal initializer
bytes alone do not make its data exact. Separate config-owned SDK contributions
are excluded; these object checks do not measure the generated EXE's similarity.
`kf analyze`/`kf check` and `kf bank` also refresh this column. Missing comparison
artifacts or objects older than their source, headers, inventories or build
inputs show `—`, independently of executable-score availability.

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

Normal builds write tool commands and input hashes into each image's
`build.json`. They do not load retail EXEs, compare output bytes, or update
the README. Explicit comparison-only runs write `fuzzy-comparison.json` and
`fuzzy-comparison.html`, preserving the build report and executable.
The standalone HTML contains a movement
map, filterable islands with both addresses and sizes, and the largest
unanchored regions. The JSON retains all islands, complementary unanchored
ranges, parameters, counts and input hashes. These generated files stay
under `build/link/{psx,game,open}/` and are not committed.

## Current source-to-EXE evidence

The current baseline was first produced by the same direct tool sequence before
it became the active build again. These observations describe its source and
SDK inputs; they do not prove exact historical tool attribution.

### Byte attribution of the overlay scores

The saved GAME and OPEN outputs score 74.174997% and 72.352139%,
respectively. A symbol- and relocation-aware audit shows that these values do
not measure the quality of the reconstructed game code directly. The candidate
symbol files place every reconstructed OPEN function in retail order at a
uniform `-0x8d0` displacement. GAME also preserves the complete order; its 362
functions occupy five nearby displacement bands from `-0x8e8` to `-0x8c0`,
with the smaller transitions following source-object and literal-pool
boundaries.

The initial section-size difference explains those displacements:

| Image | Retail bytes before first function | Candidate `.rdata` | Shortfall |
| --- | ---: | ---: | ---: |
| GAME | `0x2268` | `0x1980` | `0x8e8` |
| OPEN | `0x1734` | `0x0e64` | `0x8d0` |

Most of the shortfall belongs to one proved retail-era LIBSPU construct. Both
overlays contain a contiguous 531-entry, `0x84c`-byte dispatch table. GAME
`8003f4b4`/`8003f4b8` loads its base at `80013320`; OPEN
`8001f2d4`/`8001f2d8` loads its twin at `800127ec`. `_spu_ioctl` rejects
selectors above `0x212`, indexes the table by the selector times four, loads a
code pointer and jumps through it. All 531 entries target one of 54 blocks in
the same `_spu_ioctl` body. The table alone accounts for 93.2% of GAME's
initial shortfall and 94.1% of OPEN's. The supplied Release 2.5 `SPU.OBJ` has a
different dispatcher and no corresponding table of that extent.

The native reconstructed objects provide a separate control. Each function was
aligned through the candidate symbol file and compared using the relocation
mask from its actual `U*.OBJ`:

| Image | Functions mapped | Compared bytes | Raw unequal bytes | Unequal at relocation-touched positions | Fixed-bit unequal bytes | Masked-exact functions |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| GAME | 361/362 | 155,880 | 14,539 | 12,627 | 1,924 | 350 |
| OPEN | 108/108 | 29,504 | 2,257 | 2,251 | 6 | 107 |

Relocation-touched and fixed-bit counts are separate diagnostics and can
overlap when a byte contains both relocated and fixed instruction bits. The
result nevertheless isolates the linked-address effect: almost every OPEN game
instruction bit already agrees, while GAME's remaining fixed-bit differences
are concentrated in its existing non-exact functions.

The same audit against supplied Release 2.5 archive members separates exact
library inputs from known lineage matches. In GAME, 362 mapped vendored
functions with exact Release 2.5 evidence cover 54,988 retail bytes. Their
fixed bits all match; the 3,324 raw unequal bytes all occur at relocated
positions. OPEN gives the same result for 346 functions and 54,732 bytes, with
3,276 raw unequal bytes. By contrast, the 177 mapped functions in
version-skewed SDK objects cover 50,208 retail bytes in either overlay. Across
the common positional spans, 23,879 GAME bytes and 23,845 OPEN bytes differ in
fixed bits. The largest contributors are `_spu_ioctl`, `InitSoundSep`,
`ContDataEntry`, `Snd_crescendo`, `SpuSetReverbModeParam`, `GetMetaEvent`,
`Snd_decrescendo`, `InitSoundSeq`, `SetControlChange` and `SpuVmKeyOn`.

An ordinary linker-order control tested eight plausible archive orders without
changing any object or library. The best GAME order gained 52 equal island
bytes, moving 74.174997% to 74.193198%; the best OPEN order gained 21 bytes,
moving 72.352139% to 72.365903%. Large library-family address movements barely
change the movement-tolerant score. This rules out archive command order as the
cause of the roughly 26–28 percentage-point residue.

These observations leave no evidence-backed score improvement using the
available inputs. Adding `0x84c` bytes of padding or copying the retail dispatch
table would manufacture layout without recovering its owner. Reconstructing
Sony library bodies would also misclassify vendored code as game progress. A
substantial legitimate improvement requires the intermediate retail-era SDK
objects identified in [the SDK object audit](sdk-object-audit.md), including a
matching `LIBSPU.LIB/SPU.OBJ`; when supplied, PSYLINK will select them through
the ordinary archive references.

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
Release 2.5 floppy medium's overlooked `H2000/LIB2000/NONE2.OBJ`, dated
1994-10-18, has exactly this 24-byte text section. Its HI16, LO16 and J26 fields
are the object's three recorded relocations; all other 134 instruction bits
match once in each overlay. The native entry name is `__SN_ENTRY_POINT`, not
the inventory's address label `start`. The present LIBSN supplies a different,
larger startup.

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

The map-object startup clear spans sixteen bytes beyond the currently modeled aggregate, into the neighboring counters/gaps. This existing ownership/layout uncertainty is retained; the experiment's successful link did not prove runtime equivalence or original BSS ordering.

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

At the time of that experiment, the full repository suite passed all 739 tests
with local retail/build inputs. `ruff check scripts tests`, `git diff --check`
and `nix flake check -L` also passed. Those counts record the retired campaign,
not the current suite.

The executable-island controls cover reordering, small edits, insertions/deletions, duplicate-copy accounting, unrelated content, padding, changed address encodings and malformed EXEs. README controls cover hash freshness, weighted totals, preservation of the function block and repeatable updates. The source-to-EXE build and a subsequent comparison-only pass gave identical island reports for all three images; the latter left native EXEs and build-provenance reports unchanged. These integration checks are recorded under `build/link/island-audit/`.

The native executable controls check the minimal startup opcodes and
symbolic entry, all four BIOS aliases, initialized zero data, exclusion of an
8192-byte BSS buffer, unchanged original archives, and rejection of unresolved
references or stale output. They do not execute a complete game image.

The existing function oracles now explicitly opt into named-object data bindings where merged modules own data from separate retail regions. Each object keeps its compiled extent and initializer; ambiguous section-relative references, unknown objects, wrong extents and overlapping initialized objects are rejected. The returned program records `data_binding=objects`. This isolates function semantics and does not prove section placement. Default oracle linking still requires a consistent section base. This facility is not used by `kf link`, the delinker or the full data-layout checks.

`kf analyze` still exits unsuccessfully on data layout and incomplete ownership.
With native LNK-derived views it reports 0/1 PSX, 9/20 OPEN and 23/41 GAME
data-owning units matching. Unplaced COMMON reservations remain explicit
failures rather than receiving inferred section offsets. Equal bytes or
unchanged resolved functions do not waive ownership and placement failures.
Earlier closure reports remain under `build/link/overlay-link-audit/`; current
native build commands and input hashes are in each image's `build.json`.
