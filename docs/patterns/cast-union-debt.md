# Cast and union reconstruction debt

## Measured result and limits

This campaign starts from master `cb600b9061ad2211470ff393cd933de6b410c9dd`,
not the separate helper/enum/native-view PR stack. The target-C AST census
covers all **101 source/image variants**, deduplicating written locations.

| Measure | Master | After | Explanation |
| --- | ---: | ---: | --- |
| Written casts | 403 | 396 | Remove 35 implicit C conversions; expose 28 access boundaries |
| Pointer casts | 322 | 315 | All changed casts are pointer conversions |
| Scalar casts | 81 | 81 | 78 source locations and three enum-header macro definitions |
| Union definitions | 37 | 27 | Five record tables, four grids and one saved-world owner |
| Curated structure fields | 871 | 856 | Remove 15 alternate members; retain 128 structures |

Reproduce the live census with `kf casts --json`; inspect sites with
`kf casts --list`. The earlier README figure of 405 was stale. Unmerged
branches have different spelling counts after common-code extraction.

**This is debt reduction, not proof that the remaining 396 casts and 27
unions are necessary or that all reconstruction debt is closed.** A failed
deletion experiment establishes only that experiment's output, not the
original declaration or the impossibility of a better owner/API model.

## Cast qualification

This partitions all 322 baseline pointer casts by observed source type.
These are boundary families, not 322 proofs of unavoidable syntax. The
complete site list is reproducible from the baseline revision with
`kf casts --list`; generated census reports are not committed.

| Sites | Family | Disposition |
| ---: | --- | --- |
| 35 | `void *` to object pointers | Removed: C provides these conversions. Their spelling adds no object identity or alignment. |
| 212 | `u8 *` (206), `char *` (6) into resource/packet/record types | Retained parsing, byte-offset and arena boundaries. Case-local TMD casts select distinct disk formats, not related C object types. |
| 24 | `KfTmdHeader *` (17), `KfAssetHeader *` (7) to byte cursors | Retained serialized offsets relative to their containing assets. |
| 24 | CD location (6), matrix translation (9), projected arrays (6), packet pointer (1), overlay argument blocks (2) | Retained representation/API boundaries. Preserve authentic SDK declarations; equal addresses do not imply compatible types. |
| 15 | Integer-to-pointer (13), startup `u8[]` symbols (2) | Memory-policy/linker/argument boundaries. See startup-origin and manual-varargs audits; do not substitute unrelated globals. |
| 10 | Variadic stack base and nine argument-slot views | Open source-model debt in `effect_pool_construct`; see [its audit](effect-constructor-varargs.md). |
| 1 | Item-stock multidimensional array to byte cursor | Complete-owner byte initialization in `game_state_initialize`; retain the actual 240-byte extent. |
| 1 | `KfCdFileEntry *` to `CdlFILE *` | Open SDK ABI mismatch, detailed below. |

The 81 scalar casts include modular narrowing, signed interpretation,
logical shifts, address arithmetic and enum adapters. Examples are the
signed collision row, unsigned cell index, signed scale tests, random-value
logical shifts, clipping modulo 65536 and the halfword interpretations in
`matrix_interpolate`. Retention here does **not** certify all as necessary.
Assignment-width casts and the growth-table `memcpy` source cast remain
type-propagation candidates. The latter also conveys source alignment to
this compiler; deleting it is not a test of value semantics alone.

The 28 added casts are: ten word-pointer conversions in definition loaders,
ten destination word-pointer conversions in GAME/OPEN grid loading, six
complete-grid linear-index views, and two complete-world byte cursors.
They expose access boundaries previously hidden by alternate arrays.
These are target-toolchain representation operations, not a claim of
portable strict-aliasing semantics.

## Canonical owners and retail evidence

GAME loaders `800150a8`, `800150fc`, `80030a6c`, `80030fdc` and `8003a274`
use aligned `lw`/`sw` loops of 176, 294, 456, 320 and 120 words. The weapon
loader then edits yaw at record offset `0x26`, stride `0x2c`, for sixteen
records. Their resource-loader callers establish the corresponding buffers.
These are copies of one array, not runtime-selected payload alternatives.

Each table now owns only `.entries`. The actor extent is **12 records of
152 bytes = 1824 bytes = 456 words**; a prior unpublished review incorrectly
said 50 records. Physical word-aligned destinations remain the same globals
or containing-object offsets. The new natural C type alignment need not be
four: actual storage and call boundaries carry that precondition. No
alignment padding or per-global linker placement was added.

Each grid owns only `.cells[100][100]`, preserving its byte/enum domain.
Linear accesses convert the complete grid, not `.cells[0]` followed by
traversal beyond that row. Each loader copies 2500 words into the same
complete 10000-byte destination as retail.

`KfMapSavedWorld` owns five 1700-byte floors. Save I/O passes the whole
8500-byte object. Reset/restore cursors convert that owner to bytes, not
`floors[0].script.bytes`: its ten-byte subobject cannot model the complete
traversal. The world stays at runtime offset `0x22c`; the runtime owner
remains `0x2360` bytes. The pinned fixture checks all ten sizes, canonical
member offsets and containing layout, with a wrong-size negative control.

## Remaining 27 unions

These are retention verdicts, not original union declarations recovered from
bytes. The [earlier audit](type-assertion-and-union-audit.md) contains fuller
consumer dossiers; its old live totals are superseded here.

| Owners | Count | Qualification / remaining work |
| --- | ---: | --- |
| `KfGpuF3/F4/FT3/FT4/G3/G4/GT3/GT4` | 8 | SDK packet plus packed GTE word/UV views; shared offsets tested. Representation conveniences, not historical syntax proof. |
| `KfRotation`, `KfEffectDirection` | 2 | SDK-vector/game-field overlap; vector transfers include the fourth halfword. Do not shrink to a six-byte angle prefix. |
| `KfEffectVisualState/Control/Propagation/RenderId` | 4 | Kind/animation-selected meanings and domains within one effect record. |
| `KfMapFloorScript`, `KfMapObjectSpawn/Parameter/Link` | 4 | Floor/behavior-selected payloads. Link's extra copy/byte conveniences remain simplification candidates; preserve eight bytes and selected member extents. |
| `KfMapCell`, `KfDialogueState`, `KfPlayerMotionState` | 3 | Packed comparisons/masks overlap byte or halfword fields. Motion's third word spans pitch and cell, not just copying. |
| `KfTmdPacketHeader`, `KfScreenXY` | 2 | Serialized header byte/word and SDK packed-screen-coordinate overlap. |
| `KfTmdPrimitive`, `KfMapGpuPrimitive` | 2 | Mode-selected formats; allocation/use follows selected member extent, not maximum union size. |
| `KfMorphPrefix` | 1 | Rest-morph code passes the range prefix as an extra SDK vector, then restores the scratch entry; preserve that unusual behavior. |
| `KfPackedSVector` | 1 | Remaining copy-only word view and four-byte alignment over an eight-byte SDK vector. Open: propagate the aligned-buffer contract through pool, morph and both renderers before deleting the wrapper. |

## SDK CD record: unresolved, not hidden

GAME `CdSearchFile` at `8003c810` is vendored. Its success sequence at
`8003ca48..8003ca70` copies five words, offsets 0 through 16, into the caller
buffer. Cached row stride is 20; names begin at offset 8. The return is the
cached source row, not the destination. The supplied later SDK declares a
24-byte `CdlFILE`. The source's 20-byte `KfCdFileEntry` table does not
become API-compatible because of the existing cast.

`item_load_database` fills the separate `cd_file_table` via that API.
Ordinary `cd_search_file` storage has a curated 24-byte extent and adjacent
owners. Neither that extent nor the SDK prototype should be silently shrunk
to make the table call look clean.

The archived Runtime Library 2.0 July 1994 `LIBCD.H` has neither `CdlFILE`
nor a `CdSearchFile` prototype. It cannot settle this mismatch; provenance
is in [sony-sdk-july-1994.md](sony-sdk-july-1994.md). Closure needs an
appropriate early header/source or separately evidenced compatibility
boundary. Moving the cast into a wrapper would merely relocate the debt.

## Checker and metric corrections

The game is C89; the auxiliary C++ enum checker must not force redundant C
casts into it. After a failed modern check, `c_compat.py` parses target C,
identifies valid implicit `void *` to object-pointer expressions and inserts
explicit conversions only in a generated VFS view. The **entire modern
compilation is rerun and must succeed**. No diagnostic pattern is suppressed.

Tests cover assignments, arguments, returns, array-pointer typedefs, macro
expansions, nested/ambiguous/crossing spans, qualifiers, function pointers,
and an unrelated enum error in another argument (which must remain fatal).
Raw modern clangd inputs are not rewritten and may still flag C-only
conversions; retail editor mode remains available.

The `byte-array views` regex measures syntax, not wrong ownership. It rises
from four to eight because this campaign exposes two grid byte views and
two world cursors. It is now informational and remains visible; its floor
was removed, not raised. Total pointer casts remain ratcheted, including
these sites. Tests ensure an increase in that total still fails.

## Function verdicts and verification

A forced full rebuild produced **101 complete ELF objects byte-identical
to the saved master objects**, including instructions, relocations, symbols
and data. All 484 emitted function rows (including local/SDK helpers) retain
their sizes and strict scores. Identical fuzzy totals alone would not suffice.

Canonical-owner functions, each retaining strict **100%**:

- GAME: `weapon_records_load_and_mirror_angles`, `armor_records_load`,
  `actor_definitions_load`, `map_object_definitions_load`, `magic_load_records`,
  `map_resources_load`, `map_floor_height_for_cell_position`,
  `map_floor_height_at_position`, `collision_adjust_cell_occupancy`,
  `collision_query_world`, `map_restore_floor_state`, `game_state_initialize`,
  `save_file_write_slot`, `save_file_read_slot`.
- OPEN: `opening_resources_load_scene0`.

Implicit-conversion functions, each retaining strict **100%**:

- GAME: `audio_initialize`, `cd_file_load_allocated`, `cd_file_load_table_entry`,
  `cd_file_load_into`, `memory_set_allocation_mode`, `menu_load_item_model`,
  `game_initialize_session`, `display_show_error_screen`, `render_initialize`,
  `tim_upload_images`, `map_resources_load`, `save_workspace_allocate`,
  `render_bind_animated_instance`.
- OPEN: `audio_initialize`, `render_initialize`, `render_enqueue_map`,
  `render_enqueue_sprite`, `render_enqueue_tmd`, `render_enqueue_unlit_triangles`,
  `cd_file_load_allocated`, `cd_file_load_into`, `tim_upload_images`.

Strict game-function totals stay **457/471**: PSX 1/1, GAME 350/362,
OPEN 106/108. No new functions or vendored bodies are banked. Full `kf match`
still exits nonzero on master's existing data/allocation gates: GAME 23/41,
OPEN 10/20, PSX 0/1 data-owning units exact, with no artifact failures.
`kf build` succeeds and emits PSX.EXE, GAME.EXE and OPEN.EXE. This executable
build result is distinct from strict data-match closure. `kf check-types`
passes 101/101 variants; `ruff check scripts tests` and `nix flake check -L`
pass. The full `python -m pytest -q` run reports **796 passed, 9 skipped,
10,196 subtests passed**.
