# GAME visibility-window data ownership

## Function Match Plan

Baseline: `fb1ec70`. Recover the fixed window, runtime-loaded yaw-window table
and selected-window pointer in their existing GAME source modules. Shared
references and the loader/selector/culler contract support this campaign; no
new TU boundary is inferred from adjacency. Preserve the exact selector and
resource loader, and leave the two non-exact renderers unchanged. Compare full
payloads, BSS extents, named allocations and section placement independently;
do not enlarge a claim or lower an ELF alignment to make it pass.

All addresses below belong to GAME.EXE unless explicitly marked OPEN. The
retail image was hash-validated with `kf init`; no game was booted. Full retail
disassembly/CFG, callers, callees, strings, data references, ordered relocations,
neighbors, source history and current native match state were inspected before
the source edit. This is game visibility policy: it selects loaded level masks
and dispatches game map/entity emitters. The SDK matrix services it calls stay
vendor-owned; no SDK body is reconstructed or counted as game progress.

## Complete objects and source ownership

| Owner | VA / size | Storage and source module |
| --- | --- | --- |
| `render_fixed_cell_window` | `80055e9c / cc` | Private initialized `KfCellWindow`, render_map_cells |
| `render_cell_windows` | `80065be8 / cc0` | Global BSS `KfCellWindow[16]`, resources |
| `active_cell_window` | `80095860 / 4` | Global BSS `const KfCellWindow *`, render_map_cells |

The common GAME/OPEN `KfCellWindow` layout is already supported: four unsigned
halfwords at offsets 0, 2, 4 and 6 are width, height, origin X and origin Z;
196 byte cells follow at offset 8. The complete record stride is 204 bytes.
The shared structure remains in render_types.h and shared extern declarations
remain in game/render.h. The fixed window has one direct reference, in its
own selector, so its working source scope is private. The table and pointer
cross existing module boundaries and require shared declarations. These are
curated source-ownership choices, not recovered original linkage symbols.

The fixed retail record has header `(13,13,6,6)`, a 169-byte active mask whose
cells are 0/1/2, and 27 zero bytes of remaining cell capacity. Its full 204-byte
SHA-256 is `d1eb30733225e518538de1525701956aa5912e263432fd3a0fea3eb98d724e95`.
The extent is supported by the existing complete-object family plus the
204-byte band ending at the independently referenced light-matrix allocation
at `80055f68`; adjacency alone would not suffice. The preceding color-matrix
table occupies `80055dbc+e0`. The consumer walks only 169 active cells: no read
of the other 27 bytes, or historical file boundary, is claimed. The source
uses the shared capacity and C's implicit zero initialization, not explicit
assembler padding or an incompatible smaller per-file structure.

Seven old fixed-window identities and nine census fragments represented its
header, early cells and gaps. They become one typed owner and one complete
census row without removing any bytes. There are no interior aliases, other
in-range direct references or data relocation sites. The old header extern
and cast through `DAT_80055e9c` are removed.

COM.DAT independently proves the runtime table extent. Its first chunk length
advances to the second header at file offset `1a8`; that header declares
`cc0` bytes, beginning at `1ac`. The entire payload equals OPEN's `KF/B0/RTBL.`:
SHA-256 `b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72`.
It contains sixteen 204-byte records with 14-by-14 grids and origins:

```
(6,1) (4,3) (3,4) (1,6) (1,7) (3,9) (4,10) (6,12)
(7,12) (9,10) (10,9) (12,7) (12,6) (10,4) (9,3) (7,1)
```

Both runtime owners are outside GAME's load image. They remain uninitialized
source definitions and target NOBITS allocations; asset bytes are not invented
as executable initializers. BSS layout matching is not initialized-byte proof.

## Per-function evidence and final verdicts

### render_map_cells — 8001e83c / 168

Baseline and final strict native score: **100%**. No arguments or escaping
result; 48-byte frame saves s0-s5 and ra. The selector loads pitch as an unsigned
halfword at render_state+b4 and uses `(u16)(pitch+511) < 1023` for the runtime
branch. The other branch forms the fixed owner at `8001e874/878`. The runtime
branch loads the signed high yaw byte at +b7 and computes `(15-yaw)*204`
through shifts/adds, then forms the runtime base at `8001e8a8/8ac`.

The pointer is published at `8001e8b4/8b8`, then loaded at `8001e8c0/8c4`.
Origin halfword reads at `8001e8d8` and `8001e8e0` use offsets 6 and 4;
the cell cursor starts at +8. `tmd_select(0)` is called at `8001e8e8`, with
the column-base subtraction in its delay slot. Height and width low bytes
are read at `8001e8fc` and `8001e918`; the skipped-row path uses the width
halfword at `8001e964`. Nonzero cells within unsigned 100-by-100 map bounds
call `render_map_cell` at `8001e938`, with row moved to a1 in the delay slot.
The return is `jr ra` at `8001e99c`, restoring sp by 48 in the delay slot.

There are two direct calls, thirteen validated non-call references including
two internal jumps, and no strings. The sole external caller is render_frame
at `8001fe18`, with a NOP delay slot. The adjacent following render_actor begins
at `8001e9a4` with a 168-byte frame. Source history already established the
shared window family and this contiguous two-function unit (`0fa35d1`,
`261014b`, `c528930`). Only the fixed owner's source expression changes;
all emitted text bytes remain identical.

### render_map_cell — 8001e5ec / 250

Baseline and final strict native score: **76.783780%**, not exact. This sibling
is untouched. Its retail frame is 120 bytes versus the existing source's 88.
O32 col/row are full words and the cell is narrowed to a byte. The routine
bounds-checks 100-by-100 coordinates, remaps door light cells, builds signed
halfword SDK vectors and selects the map emitter. Its nine calls are two each
to SetRotMatrix and SetTransMatrix, then RotTrans, MulMatrix0, SetLightMatrix,
tmd_select_object_vertices and render_enqueue_map. Fifteen validated data or
internal-jump references and no strings remain unchanged. Its only caller is
the selector at `8001e938`; the preceding render_screen_sprite begins at
`8001e480`. Existing frame, CFG and scheduling differences are not attributed
to a compiler mechanism or adjusted in this campaign.

### common_resources_load — 8001b180 / 210

Baseline and final strict native score: **100%**, with all eight functions in
resources still exact. No arguments or result; 32-byte frame saves s0 and ra.
The runtime destination is formed at `8001b1dc/1e0`. After skipping the first
length-prefixed chunk, source+4 points to the second payload, and the end is
source+`cc4` at `8001b204`. The aligned/unaligned copy loop at `8001b208..284`
moves sixteen bytes per iteration, copying exactly 3264 bytes without the
length header. A subsequent separate 480-byte growth-table copy is untouched.
The return restores sp by 32 in its delay slot.

Its ten calls load two CD files, upload TIMs, release two buffers, register the
common assets and load weapons, armor, magic and map definitions. The strings
`COM\\MIX.TIM` and `COM\\COM.DAT` belong to existing RODATA at `80012178` and
`80012184`. Its only external caller is game_main_loop at `80014790`.
Neighbors are tim_upload_images at `8001b100/80` and
map_resource_path_set_floor at `8001b390/14`. No function body changes.

### render_entities — 8001f218 / 580

Baseline and final strict native score: **91.250000%**, not exact. This
render_scene function is untouched. No arguments or result; 48-byte frame
saves s0-s6 and ra. It reads the shared window pointer six times, then accesses
origin/bounds halfwords at +6/+4 and +2/+0 and the selected cell byte at +8.
The pools use map objects (190, stride 44), actors (128, stride 72), variable
floor items (stride 24), sprites (48, stride 60) and map events (8, stride 68).
Its nine calls are tmd_select, render_map_object, render_actor, three
SetLightMatrix calls, render_floor_item, render_actor_sprite and render_map_event.
There are 25 validated data/internal-jump references and no strings. Its only
caller is render_frame at `800202c0`. Existing code-generation/CFG differences
remain outside this data ownership change.

## Relocations and comparison boundary

All fourteen reviewed HI16/LO16 pairs round-trip their complete raw words and
original target addresses. The single fixed-owner pair now has an explicit
curated identity; its existing site, target, low opcode and reviewed status
are preserved. The runtime pairs and all eleven pointer pairs stay unchanged.

| Object | HI sites, each followed by LO at +4 |
| --- | --- |
| Fixed window | `8001e874` |
| Runtime windows | `8001b1dc`, `8001e8a8` |
| Pointer, selector | `8001e8b4`, `8001e8c0`, `8001e8f0`, `8001e90c`, `8001e958` |
| Pointer, culler | `8001f24c`, `8001f29c`, `8001f360`, `8001f4d0`, `8001f600`, `8001f6e8` |

Every pair has owner-relative addend zero. Semantic references remain validated
data references, not proven direct control flow. The compiler's local fixed
reference uses `.data`, while the target uses the local `render_fixed_cell_window`
symbol at section offset zero; native objdiff resolves the same owner. The
initially stale `kf try` target still named DAT_80055e9c; rebuilding
the full comparison graph removes that stale relocation-name-only difference.

Both compiled and delinked fixed objects own the complete 204 identical bytes.
The source section's sixteen-byte alignment cannot place it at `80055e9c`.
The runtime table's full 3264-byte named BSS layout agrees, but the same source
alignment rejects `80065be8`. The pointer symbol itself is four bytes on both
sides; the source BSS section occupies eight bytes through compiler COMMON
rounding, against the four-byte retail claim. All three discrepancies stay
visible. No target grows, no tail is masked, and no source alignment is edited.
Existing resources `.data` placement and `.rodata` 60-versus-57 extent failures
also remain. These units are **not strict data matches**.

Tests additionally exercise 48 bounded retail selector inputs: every yaw, the
four pitch-boundary values, and all four map corners. Only the retail selector
executes, with the two callees declared as hooks. Its selected pointer and
ordered cell dispatches agree with an independent record walker. This is a
retail semantic witness, not a source-placement waiver or whole-game execution.

Reproduce in `nix develop`, after `kf init --retail-dir <verified-extraction>`:

```sh
kf try --unit game.render_map_cells
kf try --unit game.resources
kf build compare
python -m unittest tests.test_game_cell_window_data
kf build
kf verify reachability --output build/cell-window-reachability.json
```

## Final verification

The two intended source objects and two module targets change; their entire
text sections remain byte-identical to the baseline. All 484 function scores
are unchanged: **360/471 exact game functions** and thirteen exact vendor
controls are preserved. No function is newly banked.

GAME reached source owners rise 79 to 82, config-only ranges fall 360 to 357,
and unmatched config ranges fall 358 to 355. Across images, unmatched config
ranges fall **626 to 623**. Source/header DAT_ occurrences fall **170 to 167**;
data identities fall 2986 to 2980 by eliminating six interior aliases.

Strict source data changes **8/59 to 8/60**, because the selector unit now owns
compared data and honestly fails placement/extent checks. SDK data remains
4/4 and target relinking 110/116, with the same six pre-existing section-base
conflicts. The mandatory full build remains red on data, reference closure and
placement, without function regression. Full reachable-byte coverage and
linked-executable equality remain unproved.

All **587 local tests** pass without skips, including seven new whole-object,
asset, relocation and bounded retail controls. Ruff and diff checks pass.
No tooling, compiler profile, failure gate or function baseline changes.
