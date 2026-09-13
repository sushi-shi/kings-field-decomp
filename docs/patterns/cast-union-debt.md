# Cast and union reconstruction debt

## Readability follow-up

The first cleanup made several callers harder to read. This follow-up to
`4f2940c5` corrects that tradeoff; fewer union declarations alone were not a
sufficient verdict. Current counts are **351 casts** (270 pointer, 81 scalar)
and **30 unions**. The integration snapshot below is historical.

- Grid queries now spell `map_floor_height_grid.linear[cell_index]`, with
  corresponding typed attribute, collision-kind and orientation views. Four
  grids retain `.cells[z][x]` and `.linear[index]` over the same 10000-byte
  extent. Their `.words` members remain removed. These dual-index unions are
  deliberate reconstruction conveniences, not recovered original declarations
  or proof that no better union-free model exists.
- GAME `map_resource_copy_words` and OPEN `resource_stream_copy_words` accept
  complete destination objects through `void *`, converting to a word cursor
  inside the implementation. Callers pass `&map_cell_orientation_grid` without
  a word-pointer cast. Both buffers must remain word-aligned; count and return
  value remain a word count and advanced source cursor. No SDK prototype changes.
- `TMD_OBJECT_VERTICES(asset, object)` names the shared payload-relative decoding
  in both renderers. Its two casts remain visible in the shared definition;
  this is API clarification and deduplication, not elimination of serialized
  offset interpretation. Each argument occurs once. The offset is in bytes
  from the end of the 12-byte TMD header, not from the object record.
- The two TIM-loading locals in `menu_load_item_texture` and
  `menu_load_message_image` retain the existing primitive-buffer cursor's
  `u8 *` type rather than discarding it to `void *`. This is independent type
  propagation at the same resource boundary. The source `void*` count stays
  14; all previously ratcheted metrics remain enforced without exclusions.

The 18 fewer written casts comprise six replaced grid casts, ten eliminated
destination casts and two deduplicated TMD casts. Four union definitions return:
the original 37 are now 30, so the net owner/SDK wrapper removal is seven, not
eleven. Cast counting does not measure whether the remaining interfaces are
historically original or portable under every aliasing model.

Two union-free grid experiments were rejected after inspecting the first raw
divergence. Typed `.cells[index / 100][index % 100]` accessors introduce a
`divu`/quotient/remainder sequence before the first floor-grid load; the floor
query falls to 83.379630% and the world query to 87.149536%. Flat canonical
arrays with a cast-free row-address macro change 25 coordinate consumers in
19 objects: OPEN `render_map_cell` first loses the `move a3,a0` at +0x14 and
computes the combined index before selecting the grid base. All those source
experiments are reverted. Neither result establishes an unavoidable codegen
limit. The inline TMD experiment using `(u8 *)(asset + 1)` changes the final
`addu` and stored register at GAME +0x20 / OPEN +0x20; the retained named macro
uses the existing shared-header style and reproduces the original expression.

Final verdicts for changed functions (all strict **100%**, unchanged):

| Image | Address | Function |
| --- | --- | --- |
| GAME | `8001a29c` | `map_floor_height_for_cell_position` |
| GAME | `8001a5ac` | `collision_query_world` |
| GAME | `8001b3e4` | `map_resource_copy_words` |
| GAME | `8001b558` | `map_resources_load` |
| GAME | `8001c148` | `tmd_select_object_vertices` |
| GAME | `8002af48` | `menu_load_item_texture` |
| GAME | `8002c5e0` | `menu_load_message_image` |
| OPEN | `80016318` | `resource_stream_copy_words` |
| OPEN | `80016348` | `opening_resources_load_scene0` |
| OPEN | `80016ec8` | `tmd_select_object_vertices` |

All 101 complete objects and all 484 emitted function rows equal the saved
master `4d3dc5b7` baseline. Strict totals remain **458/471**, with no new banking
or vendored progress. All three executables build and all 101 modern type-check
variants pass. Data/allocation gates retain the same pre-existing failures.
New controls check grid row transitions and endpoints, matching enum domains,
const-destination rejection, TMD payload offset 28 resolving to asset byte 40,
and single evaluation of both macro arguments. The pinned layout fixture also
checks the four linear members at offset zero.

Final validation: `ruff check scripts tests`, `git diff --check`, and
`kf verify board --gate` pass. The full repository suite reports **811 passed,
9 skipped, 10,229 subtests passed**; `nix flake check -L` passes with 820 tests
and 143 skips in the isolated environment.

## Master integration snapshot (before readability follow-up)

PR #5 incorporates master `4d3dc5b7`, including the enum, common-helper and
explicit-referent work. Conflict resolutions retain master's `KfObjectId`,
overlay domains, `TMD_PREPARED_VERTEX` and `CD_LOCATION_COPY` while keeping
this campaign's canonical owners and removed implicit-conversion casts.

Fresh target-C censuses measured **368 casts on current master** (287 pointer,
81 scalar) and **369 after integration** (288 pointer, 81 scalar), with
**26 unions** in the merged source. Shared helpers change the number of
written locations; the earlier 403/404 figures below describe the original
baseline and are historical, not live totals.

Both revisions were freshly compiled. All **101 complete reconstructed ELF
objects** are byte-identical between current master and the merged branch;
all **484 emitted function rows** retain their sizes and strict scores.
Current-master exact totals remain **458/471**: GAME 351/362, OPEN 106/108,
PSX 1/1. This campaign does not claim master's additional exact function as
new progress. All 101 modern type-check variants pass. The existing data
and section-placement failures remain unchanged.

The integrated branch builds all three executables. Ruff, the source ratchets,
`git diff --check` and `nix flake check -L` pass. The full repository suite
reports **808 passed, 9 skipped, 10,223 subtests passed**; the isolated flake
suite runs 817 tests with 143 skips.

## Original baseline: measured result and limits

This campaign starts from master `cb600b9061ad2211470ff393cd933de6b410c9dd`,
not the separate helper/enum/native-view PR stack. The target-C AST census
covers all **101 source/image variants**, deduplicating written locations.

| Measure | Master | First pass | Follow-up | Explanation |
| --- | ---: | ---: | ---: | --- |
| Written casts | 403 | 396 | 404 | Remove 35 implicit C conversions; expose 36 access boundaries |
| Pointer casts | 322 | 315 | 323 | All retained cast changes are pointer conversions |
| Scalar casts | 81 | 81 | 81 | 78 source locations and three enum-header macro definitions |
| Union definitions | 37 | 27 | 26 | Five record tables, four grids, saved world and SDK vertex wrapper |
| Curated structure fields | 871 | 856 | 852 | Remove alternate members; 127 structures after deleting the vertex wrapper |

Reproduce the live census with `kf casts --json`; inspect sites with
`kf casts --list`. The earlier README figure of 405 was stale. Unmerged
branches have different spelling counts after common-code extraction.

**This is debt reduction, not proof that the remaining 404 casts and 26
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

The 36 added casts are: ten word-pointer conversions in definition loaders,
ten destination word-pointer conversions in GAME/OPEN grid loading, six
complete-grid linear-index views, two complete-world byte cursors, three
aligned SDK vertex word views, and five complete-link copy/serialization views.
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

At first each grid owned only `.cells[100][100]`, preserving its byte/enum
domain. The readability follow-up above restores the explicit typed linear
view. Each loader still copies 2500 words into the same complete 10000-byte
destination as retail, now through the whole-object copy API.

`KfMapSavedWorld` owns five 1700-byte floors. Save I/O passes the whole
8500-byte object. Reset/restore cursors convert that owner to bytes, not
`floors[0].script.bytes`: its ten-byte subobject cannot model the complete
traversal. The world stays at runtime offset `0x22c`; the runtime owner
remains `0x2360` bytes. The pinned fixture checks all ten sizes, canonical
member offsets and containing layout, with a wrong-size negative control.

## SDK vertices and map-link follow-up

`KfPackedSVector` was a wrapper around the authentic eight-byte `SVECTOR`
solely to supply `u32 words[2]`. It is now deleted, not renamed or replaced
by another union. The cached vertex pointer, GAME/OPEN current vertex pointer,
morph scratch array, selectors and projection consumers all use `SVECTOR`
directly. Both images' public selector signatures and inventories agree.

GAME `render_bind_animated_instance` (`800205d4 / 3a4`) copies each vertex
with two words in the loops at `800207f8` and `800208c8`. Its scratch save
at `8002091c/80020920` and restore at `80020930/80020938` preserve both XY
and Z/padding around `gteMIMefunc`. These operations remain explicit aligned
word accesses. Asset payloads and the allocated cache are word-aligned; scratch
starts at `800930f0` (graphics owner + `22258`), advances in eight-byte steps,
and output begins at element one. This is a buffer contract, not a false claim
that the SDK's naturally halfword-aligned type has four-byte alignment.
The unusual extra range-prefix vector and inherited uninitialized incoming
keyframe register are unchanged. No SDK function body or declaration changes.

`KfMapObjectLink` still has genuine behavior-selected fields, gold and item
payloads. Its extra `.words` and `.bytes` members are removed. Reset at
`80030f7c` writes word one before word zero; loading at `80031008` copies two
words at `80031170..8003117c`. Persistence at `80035b5c` and restoration at
`80035e44` transfer all eight bytes. The explicit cursors cover the complete
link, not an individual field. Layout tests retain link size eight, placement
size twenty/link offset twelve, and runtime object size forty-four/link offset
thirty-two. No artificial alignment member is retained.

Three additional source experiments were rejected, not counted as reductions:

- Saving the scratch `SVECTOR` by aggregate assignment changes the frame from
  72 to 80 bytes and emits unaligned merge loads/stores. The retained word save
  expresses the independently established buffer alignment.
- Capturing status-panel UV width/height in two `u8` locals eliminates six
  written casts but changes the frame from 112 to 80 bytes, captures loads
  before packet writes and changes subsequent instruction sequences. The
  retail `lbu` UV domain and halfword screen dimensions remain unchanged.
- Removing the two shade casts in OPEN `opening_scene1_run` (`8001455c / ac`)
  changes both call delay slots from `andi a0,s0,0xff` to `move a0,s0`, despite
  the byte-typed callee. Those casts remain. Neither rejected scalar control
  proves an unavoidable original spelling or rules out future type recovery.

## Retained 26 non-grid unions

Together with the four dual-index grids above these give 30 unions. These are
retention verdicts, not original union declarations recovered from
bytes. The [earlier audit](type-assertion-and-union-audit.md) contains fuller
consumer dossiers; its old live totals are superseded here.

| Owners | Count | Qualification / remaining work |
| --- | ---: | --- |
| `KfGpuF3/F4/FT3/FT4/G3/G4/GT3/GT4` | 8 | SDK packet plus packed GTE word/UV views; shared offsets tested. Representation conveniences, not historical syntax proof. |
| `KfRotation`, `KfEffectDirection` | 2 | SDK-vector/game-field overlap; vector transfers include the fourth halfword. Do not shrink to a six-byte angle prefix. |
| `KfEffectVisualState/Control/Propagation/RenderId` | 4 | Kind/animation-selected meanings and domains within one effect record. |
| `KfMapFloorScript`, `KfMapObjectSpawn/Parameter/Link` | 4 | Floor/behavior-selected payloads. Link's extra copy/byte members are removed; preserve eight bytes and selected member extents. |
| `KfMapCell`, `KfDialogueState`, `KfPlayerMotionState` | 3 | Packed comparisons/masks overlap byte or halfword fields. Motion's third word spans pitch and cell, not just copying. |
| `KfTmdPacketHeader`, `KfScreenXY` | 2 | Serialized header byte/word and SDK packed-screen-coordinate overlap. |
| `KfTmdPrimitive`, `KfMapGpuPrimitive` | 2 | Mode-selected formats; allocation/use follows selected member extent, not maximum union size. |
| `KfMorphPrefix` | 1 | Rest-morph code passes the range prefix as an extra SDK vector, then restores the scratch entry; preserve that unusual behavior. |

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

The `byte-array views` and `pointer casts` regexes measure syntax, not wrong
ownership. This narrow byte-view regex rises from four to nine (it omits the
const-qualified link cursor), and total pointer casts end one
above master. Enforcing a down-only total would reward retaining the artificial
vertex wrapper and map-link members. Both counts are now informational and
remain visible; their floors were removed, not raised or silently exempted by
site. No conversion is excluded from the AST census. Tests check visible
increases and ensure unrelated ratcheted regressions still fail. Strict type
compilation and byte comparison remain independent requirements.

## Pre-integration function verdicts and verification

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

Follow-up functions, each retaining strict **100%**:

- GAME: `render_bind_animated_instance`, `pool_reset`, `tmd_set_current_vertices`,
  `tmd_select_object_vertices`, `tmd_project_vertices`, `tmd_project_vertices_shift`,
  `tmd_transform_vertices`, `map_object_pool_clear`, `map_object_pool_load`,
  `map_restore_floor_state`.
- OPEN: `tmd_set_current_vertices`, `tmd_select_object_vertices`,
  `tmd_project_vertices`, `tmd_project_vertices_perspective_right`,
  `tmd_project_vertices_shift`, `tmd_transform_vertices`.
- GAME `map_world_state_persist` remains **97.52873%**, with its complete object
  unchanged. Rejected controls leave GAME `menu_status_panel` and OPEN
  `opening_scene1_run` at **100%**.

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
