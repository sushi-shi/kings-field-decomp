# Sony header helpers in the reconstructed sources

The supplied Sony/Psy-Q headers contain substantial preprocessor helpers.
The [complete follow-up campaign](sony-header-helper-campaign.md) now reuses all
147 original candidates and closes GAME display initialization. No C
inline-function definitions or unclassified inline GTE instructions were found
in the bounded header/function census below.

This note preserves the SDK evidence and the initial 15-site pilot through
`3205f777`, integrated as `ad214ccb`. Its trial scores and counts are historical;
the follow-up has the complete per-site ledger and current verification.

## Function Match Plan

This investigation starts at `48132a4e` in the separate
`codex/sony-header-helper-audit` worktree. The question is whether the supplied
Sony SDK contains preprocessor/inline helpers and whether our handwritten
candidate patterns correspond to them. It is not a compiler-attribution claim.
The flake's Release 2.5 headers are the primary source. All three retail images
were validated with `kf init` before the image-qualified evidence pass.

The related graphics pilot shares LIBGPU packet types, the primitive-buffer
cursor, ordering-table insertion and the same ordered XY/UV/color stores.
The vector control tests the same header's `addVector` abstraction. Unit
ownership, signatures, types and profiles stay fixed. Read the complete retail
CFG, incoming/outgoing references, strings, current strict matches, adjacent
functions, caller argument setup and source history before these trials.
Raw evidence is under `build/sony-helpers/` and is not committed.

| Image / VA / size | Function / initial strict score | Evidence and first hypothesis |
| --- | --- | --- |
| GAME / `80014314` / `1c0` | `sprite_add_ft4` / 100% | Six O32 arguments; color/index read from caller stack, selectors narrowed by `sh`; ordered `lhu` XY and `lbu` UV/RGB input reads. Two proven calls, `SetPolyFT4` and `AddPrim`; validated graphics referents `+20`, `+20024`. Test `setXYWH`, `setUVWH`, `setRGB0`, one family at a time. |
| OPEN / `8001399c` / `1c0` | `sprite_add_ft4` / 100% | Same signature, store widths, call topology and owner offsets as GAME, at independent OPEN addresses. Three proven callers: scene 3, ending scroll and fade-in. Test the same three macros. |
| OPEN / `80013b5c` / `114` | `sprite_add_f4` / 100% | Three O32 arguments; 24-byte SDK packet; two proven calls, `SetPolyF4` and `AddPrim`; same owner offsets. Test `setXYWH` and `setRGB0`. |
| OPEN / `80013804` / `198` | `sprite_add_g4` / 100% | Five pointer arguments; fifth color is on the caller stack. 36-byte SDK packet, four distinct RGB triplets, fixed OT index `3fef` (`ffbc` byte offset). Proven `SetPolyG4`/`AddPrim` calls and two validated owner references. Test `setXYWH`, then `setRGB0..3`. |
| GAME / `80033680` / `11c` | `camera_path_step` / 100% | Path pointer plus signed word Y offset; three finished-state branches, one proven call to `camera_path_compute_segment`, no global references. Word component additions followed by Q4 shifts and halfword angle stores masked with `fff`. Test two `addVector` expansions. |
| OPEN / `80016adc` / `1d8` | `display_initialize` / 96.652540% | One promoted mode argument; 48-byte frame, 18 proven calls, 16 validated data-address pairs and two validated internal jumps. Six explicit RGB zero byte stores on two authentic DRAWENVs. Test only two `setRGB0` expansions in an alternative source; compare the first address/schedule divergence and all three siblings. |

All six return void and reference no strings. Each return owns its final stack
restore delay slot: 48 bytes for FT4/G4/display, 40 for F4 and 32 for camera.
The sprite callers supply 8-byte screen rectangles and byte UV descriptors;
FT4 uses descriptor offsets 0/2/4/6 and OT mask `3fff`. Macro arguments have no
increments, assignments or calls, so repeated evaluation preserves the existing
source reads and does not introduce side effects. `addVector` updates only
vx/vy/vz, leaving the SDK padding field untouched.

Vendor negative control: the game-specific allocation/geometry/OT policy and
camera traversal are separate from the called Sony implementations. The
`functions_vendored.tsv` PRIM members identify `SetPolyFT4`, `SetPolyF4`,
`SetPolyG4` and `AddPrim` from an exact Release 2.5 archive-section match.
Their proven calls must remain calls; lowercase `setPoly*`/`addPrim` would remove
them. The candidate's use of a Sony macro does not make the enclosing game
function a vendored body.

Retain readable, evidenced macro forms only if raw instructions and ordered
relocations are preserved and strict objdiff remains 100% for banked bodies.
A neutral display trial is recorded without claiming to solve its mismatch.
Run focused rebuilding/matching, full build, lint, repository tests and
whitespace checks. The initial full build already fails its data/ownership
checks; compare that baseline explicitly rather than describing it as green.

## What the supplied SDK actually contains

The pinned include directory contains **32 headers, 158 function-like macro
definitions and 155 distinct names** (conditional assert definitions account
for duplicates). Searching every header found **no C `inline` / `__inline__`
function definitions**. This is a statement about these supplied files, not
all Sony SDK releases. The archive source and hashes are pinned by
`flake.nix` and staged by `scripts/create-toolchain.py`.

`LIBGPU.H` identifies itself as revision **3.51, 1994-11-21**; `LIBGTE.H` is
revision **1.9, 1994-12-08**. The corresponding raw-file SHA-256 values are:

- LIBGPU.H: `a33286ee6a0a41835a83d3a0cb053d6808b9772671d3d5f6c48c0267928a9a3b`
- LIBGTE.H: `a55dc74d369782fc23c8ea3001dee34f3339dae44f2c6f1870720f08ae66e62e`
- LIBCD.H: `951f1ca8add88188d692ad13a6a86445ece5cc30c5e63dc32f846948d677b61d`

The Release 2.5 archive provenance is strong evidence for availability, and
several PRIM routines match retail. It does not prove that these precise header
revisions compiled King's Field; the curated GTE SMP/CMB lineage is explicitly
older than the supplied GTE library.

| Header | Function-like definitions | Relevant helpers and boundary |
| --- | ---: | --- |
| LIBGPU.H | 74 | `setVector`, `copyVector`, `addVector`, `setRECT`; `setXY*`, `setUV*`, `setRGB0..3`, `setWH`; tag access/insertion, primitive initialization, texture-page/CLUT helpers, `limitRange`, debug dumps. |
| LIBGTE.H | 20 | `read_sz*`, `read_sxsy*`, `read_rgb*`, `read_flag`, `read_p`, `read_otz`, `read_opz`, `read_mt`, `set_trans_matrix`: register-name assembler macros inside `#ifdef ASSEMBLER`, with explicit GTE instructions and trailing nops. They are not C `gte_*` inline helpers. |
| LIBCD.H | 13 | BCD conversions `btoi`/`itob`, command wrappers such as `CdSeekL`, `CdPause`, `CdStop`, `CdPlay`, and `CdGetToc` forwarding. |
| LIBSPU.H | 6 | Voice-channel masks (`SPU_KEYCH`/`SPU_VOICECH`) and four old/new transfer-API aliases. |
| R3000.H | 12 | KSEG/physical address conversions, segment predicates and exception-code packing. |
| ABS.H / CTYPE.H | 1 / 15 | Scalar absolute value, table-backed character classes and ASCII/case helpers. The absolute-value macro has weak argument/result parenthesization; it is not a universal safe textual replacement. |
| LIBSN.H | 1 | SN Systems' `pollhost()` is an inline **assembly macro**, `asm("break 1024")`, for development-host debugging; it is not a Sony game math helper. |
| ASSERT.H / FS.H / SYS/TYPES.H / STRINGS.H | 4 / 8 / 3 / 1 | Assertions, device/circular-buffer helpers, device-number packing and a `strdup` wrapper. These are SDK runtime/system facilities rather than the graphics patterns found below. |

`MEMORY.H` supplies declarations for `memcpy`, `memmove`, `memcmp`, `memchr`
and `memset`, not inline copy loops. Neither the TMD packet traversal macros
in our source nor the typed animation vertex-copy helper has an equivalent
supplied SDK macro. In particular, `copyVector` copies only three components;
it does not explain an eight-byte SVECTOR copy including its fourth halfword,
or a whole 16-byte VECTOR assignment including `pad`.

### Similar spellings can have different calls

The exact Release 2.5 definitions matter:

- `setClut(p,x,y)` assigns the result of **`GetClut(x,y)`**, retaining the
  library call. `getClut(x,y)` expands `(y << 6) | ((x >> 4) & 0x3f)`.
- `setTPage` likewise calls **`GetTPage`**. Lowercase `getTPage` still calls
  **`GetGraphType()`**, then selects one of two GPU-format encodings. It is
  not the single call-free expression found in some other header descriptions.
- Lowercase `setPoly*`, `setSemiTrans`, `setShadeTex` and `addPrim` expand tag
  bitfield/byte operations. Capitalized APIs have actual retail calls in our
  packet builders. Replacing those calls with macros would contradict direct
  control-flow evidence even if the final packet contents looked similar.
- `setRGB0` expects `r0/g0/b0` fields. It is appropriate for POLY packets and
  DRAWENV. A bare CVECTOR has `r/g/b/cd`; casting it to a packet or inventing
  incompatible field names just to use that macro is not supported.

## Source-pattern audit

At the starting commit, a scan of all **111 C source files** found **11 uses**
of supplied function-like macro names: four `setVector`, two `setXYWH` and
five `setRGB0`. They occur in GAME camera/map-cell/menu/error-screen code and
OPEN map-cell/ending-scene code. Project enum checking helpers and our own
`static inline` functions are separate from SDK helper evidence.

A second screen looked for adjacent writes to the same object in SDK field
order. It found **147 candidate sequences**: 75 vector triplets, 26 XY quads,
26 UV quads and 20 RGB triplets (17 RGB0 plus RGB1/2/3). These are lexical
candidates, not 147 proved original macro calls. The screen requires identical
left-hand owners and adjacent assignments; it intentionally misses reordered,
interleaved, packed-word, aggregate-copy and differently named field forms.
It did not find an adjacent four-field `setRECT` sequence. No claim of complete
historical macro recovery follows from these counts.

| Current source family | Candidate pattern | Assessment |
| --- | --- | --- |
| GAME/OPEN `sprite_add_ft4`; OPEN `sprite_add_f4`, `sprite_add_g4` | Rectangle corners repeat x/y and add w/h; FT4 UVs repeat u/v and add spans; RGB triplets copy matching channels | Strong correspondence to `setXYWH`, `setUVWH`, `setRGB0..3`. Tested and retained below. |
| GAME `camera_path_step`; OPEN `opening_camera_path_step` | Add position/rotation delta components to fixed-point accumulators | Direct `addVector` shape; GAME tested and retained. Both also have `setVector`-shaped output conversion. |
| GAME `menu_runtime`, `menu_list_render`, `menu_map_viewer`, `menu_status_panel`; OPEN `opening_scene1_draw_fade` | Repeated XY/UV quads and colors | Strong field-family candidates. Choose `setXY4`/`setUV4` when corners are independently specified; choose WH forms only when the actual sums, widths and repeated reads agree. Most remain unedited. |
| GAME `display_show_error_screen`, `screen_show_image_until_input` | Explicit screen-corner and UV assignments; `prim.clut = GetClut(...)`, `prim.tpage = GetTPage(...)` | XY/UV helpers and call-preserving `setClut`/`setTPage` are candidates. Existing capitalized calls are supported; lowercase arithmetic replacements are not. |
| GAME/OPEN render initialization | Three zero rotation fields; six DRAWENV RGB zero stores | `setVector`/`setRGB0` shapes. OPEN RGB trial preserves the outstanding display mismatch. |
| GAME effect pool/dispatch, actors, map objects, entity rendering, camera setup; OPEN entity/matrix routines | Many vx/vy/vz writes, copies, additions and scaled expressions | `setVector`, `copyVector` or `addVector` candidates. Entire-object assignments and non-x/y/z store order require independent review; keep padding and input-read order. |
| GAME/OPEN TMD/map/sprite enqueuers | Packed GTE XY words, UV halfwords and complete color/code words | Field-name similarity is insufficient. SDK XY/UV macros assign separate halfwords/bytes; retail's word/halfword operations and preserved code byte explain the existing typed packed views. |
| CD resource loaders | `CdControl(CdlSetloc, location, 0)`, followed by sector reads | Supplied command wrappers cover seek/pause/play etc., not this Setloc sequence. Locations are copied as already encoded bytes; no BCD conversion pattern was found in these callers. |
| Audio | SsSeq/SsVo operations and game-specific volume/distance policy | No current SPU channel-mask/transfer-alias use. A generic shift or attenuation expression is insufficient evidence for those macros. |
| Allocator, formatter, state/parser code | Cached RAM bounds, custom glyph classification, copy loops, binary formats | Allocator's unsigned 2 MiB window check differs from R3000 segment conversion/predicates. Glyph comparisons lack CTYPE's `_ctype_` table reference. No supplied parser/serializer inline helper was identified. |

One-sided fade/velocity caps should also not be casually renamed `limitRange`:
that macro assigns a nested **lower-bound then upper-bound** conditional. The
existing branch order, input width and side effects must first justify both
comparisons. The OPEN ending scroll's upper-only background-blend cap and
its y/x/z transition snapshot are not exact matches for `limitRange` or the
x/y/z `setVector` order. No such unsupported rewrite was tested.

### Retail GTE negative control

For each image independently, scan aligned instruction words inside every
curated function extent for primary opcodes `0x12` (COP2), `0x32` (LWC2) and
`0x3a` (SWC2), then join by **image plus start VA** to
`functions_vendored.tsv`. The result is:

| Image | Non-vendored functions scanned | Non-vendored COP2-containing functions | Vendored COP2-containing functions |
| --- | ---: | ---: | ---: |
| PSX.EXE | 1 | 0 | 0 |
| GAME.EXE | 362 | 0 | 113 |
| OPEN.EXE | 108 | 0 | 113 |

Thus none of the **471 current game-function extents** supplies evidence of
missing inline GTE instructions. The direct `RotTrans*`, `ReadSZ*`, matrix and
lighting calls in our C remain the relevant API boundaries. This scan is
bounded by the curated function extents and vendor classifications; it is not
a census of undiscovered executable islands or proof of historical header use.
The `pollhost` development macro is also absent from current source.

## Initial pilot results

All trials use each unit's unchanged **GCC 2.5.7 PSX rebuild**, `-O2 -G0
-mcpu=r2000`, with the configured maspsx ASPSX 1.07 model. This is the current
manifest's practical probe, not proof of the original compiler. Every trial
is a complete alternate translation unit. Macro families were substituted
cumulatively after the previous variant preserved its comparison; no flag,
register carrier, assembly, type, global, relocation or inventory change was
introduced.

| Unit / trial sequence | Result |
| --- | --- |
| `game.sprite_add_ft4`: XY, then UV, then RGB | Each of three builds stays strict 100%, 448 bytes. |
| `open.sprite_add_ft4`: XY in both bodies, then FT4 UV, then RGB in both | Each of three builds keeps FT4 at 100% / 448 bytes and F4 at 100% / 276 bytes. |
| `open.opening_render`: XY, then four RGB helpers | Both builds keep G4 at 100% / 408 bytes and frame-render sibling at 100% / 64 bytes. |
| `game.camera_path`: two `addVector` calls | Step remains 100% / 284 bytes; both siblings remain 100% / 732 and 192 bytes. |
| `open.render_init`: two `setRGB0` calls | Display remains 96.652540% / 476 compiled bytes versus 472 retail; all three siblings remain 100%. Not retained. |

Across the **ten builds**, every tested function's complete instruction and
ordered-relocation listing equals the starting candidate. The final source
retains **15 SDK macro uses in five functions**, replacing 75 assignments;
all eight functions in the four retained units stay strict 100%.
Identical emitted code establishes compatibility with these macro expansions,
not whether the original author wrote the macro or its equivalent statements.

The display trial introduces **no new divergence**. Its first existing
raw difference remains at function `+0x20`: the candidate constructs the second
DRAWENV address into `s2` with an additional HI16/LO16 pair before the mode
comparison. Retail instead loads `0xfe` there and later derives DRAWENV pointers
from its retained DTD field address. The complete call sequence and RGB stores
are unchanged by the macro. This remains an unattributed source/codegen residue;
the helper substitution does not establish a compiler limitation.

No newly exact function is claimed. The full code census remains PSX **1/1**,
GAME **340/362**, OPEN **106/108**, totaling **447/471** strict-exact functions.
The macro family recovered readable source structure in this pilot. The later
complete pass found a counterexample to treating its neutral results as a
general conclusion: GAME display initialization becomes exact with `setRGB0`.

## Initial pilot verification and reproduction

The four retained units were explicitly rebuilt with `kf try` and then
`kf match --unit`. All eight function comparisons are strict 100%. A separate
recompile of their sources from `48132a4e` confirms equality of every loadable
section's raw bytes, sizes, flags and alignment, function/data symbol extents,
and ordered named relocations. Source-path/line debug metadata differs as
expected and is outside that comparison. The macro edits therefore preserve
constants and referents as well as the displayed percentages.

`kf check-types` passes all four affected source/image variants.
`ruff check scripts tests` passes. `python -m unittest discover -s tests -v`
passes **739 tests with nine skips**; `cargo test --offline --manifest-path
tools/Cargo.toml` passes **96 tests with five ignored corpus tests**.
`git diff --check` passes. No new tooling or flake change needs a flake check,
and no parser/oracle implementation changed.

The final **full `kf build` exits 1**, as did the initial fresh build.
`kf match --unit` also exits 1 after scoring because it invokes the image-wide
verification gate. Comparing the baseline and final logs confirms identical
code totals, divergent-unit lists and failure classes: data comparison remains
PSX 0/1, GAME 8/44, OPEN 3/20; target relink remains PSX 1/1, GAME 63/77,
OPEN 32/38. The existing placement, BSS/layout, data addend/size and incomplete
ownership findings are not repaired or hidden by this investigation. No
retained source unit owns data, and no banked code body regresses.

Reproduce the focused controls in `nix develop` after `kf init`:

```sh
kf try --unit game.sprite_add_ft4
kf try --unit open.sprite_add_ft4
kf try --unit open.opening_render
kf try --unit game.camera_path
kf sema --image game match sprite_add_ft4
kf sema --image game match camera_path_step
kf sema --image open match sprite_add_ft4
kf sema --image open match sprite_add_f4
kf sema --image open match sprite_add_g4
kf sema --image open match display_initialize
```

For the display control, copy `src/open/render_init.c` under `build/`, replace
only each contiguous DRAWENV r0/g0/b0 zero triplet with
`setRGB0(&open_graphics_runtime.display_draw_environments[i], 0, 0, 0)` for
literal indexes 0 and 1, then use `kf try --unit open.render_init --source`
with that file. Do not regroup DTD/ISBG/DFE writes or introduce a new pointer.
The ten complete trial sources, strict results, raw comparison and evidence
snapshots remain locally under `build/sony-helpers/`.

Header inventory controls are `rg --files "$PSYQ_INCLUDE"`,
`rg -n '^[[:space:]]*#[[:space:]]*define[[:space:]]+[A-Za-z_][A-Za-z0-9_]*\(' "$PSYQ_INCLUDE"`,
`rg -n '\binline\b|__inline' "$PSYQ_INCLUDE"`, and
`sha256sum "$PSYQ_INCLUDE"/LIBGPU.H "$PSYQ_INCLUDE"/LIBGTE.H "$PSYQ_INCLUDE"/LIBCD.H`.
Inspect the ASSEMBLER guard and macro expansion bodies, not just names. The
COP2 census uses the admitted `functions.tsv` extents (including return delay
slots), actual PS-X load address/file offset and the three primary opcodes
listed above. It does not promote a candidate identity or change vendor status.

After staging only this campaign, `kf bank --unit game.sprite_add_ft4 --unit
open.sprite_add_ft4 --unit open.opening_render --unit game.camera_path`
refreshes exactly eight existing 100% records. Only their input hashes change;
all scores and extents remain unchanged. No unrelated inputs or generated
reports are banked.
