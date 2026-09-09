# Remaining data-owner repair campaign

## Function Match Plan

Start from `c0da3dc0`: GAME 27/44 and OPEN 10/20 data-owning units pass.
The user requested work across every remaining category: BSS allocation order,
reservation sizes, initialized-data ownership, packed sound selectors,
jump-table destinations and SDK storage. Preserve this complete scope and
all banked function results. The existing strict data gate is the acceptance
criterion; changing the denominator or suppressing unresolved placement is
not closure.

Before each source change, retain each affected function's image-qualified
address/CFG/disassembly, callers/callees, strings, data/relocation evidence,
adjacent functions, history and match state under `build/data-repair/`.
Compare the first real instruction/referent divergence, not only percentages.
Separate an owning-symbol change from a change to its physical referent.

Investigate in these related campaigns:

1. Complete pool and memory objects. GAME's startup clears 0x25b8 bytes of
   map-object state, including three sequence halfwords beyond the current
   0x25a8 claim. OPEN's copied pool has the same 16-byte tail pattern. Inspect
   all consumers and both reset routines before deciding whether they share
   an owning aggregate. Do not invent opaque padding from a score alone.
2. Native allocation behavior. GCC 2.5.7 `varasm.c:assemble_variable` rounds
   tentative definitions to `BIGGEST_ALIGNMENT` (64 bits), and the MIPS
   `ASM_OUTPUT_COMMON/LOCAL` macros emit that rounded size. Reproduce compiler
   options and native assembler/linker allocation classes. The compiler source
   is the reconstructed Decompals target, not proof of Sony's original build.
3. Initialized graphics and scene data. Establish the actual owners and
   section classes of distant contributions. Follow readers/writers, original
   assembler declarations and compiled section selection. Do not scatter
   members of an indivisible section to force their retail addresses.
4. Packed selectors and switch tables. Inspect source types/array identity and
   the specific case-label instruction offsets. Preserve raw immediate values,
   ordered relocation targets and existing exact functions.
5. SDK storage. Inspect archive members, headers and library signatures before
   changing the PAD provider. No vendored body enters game progress.

Each retained change must rebuild affected units and pass focused raw/code/data
controls, followed by full `kf build`, Ruff, repository tests and diff checks.
Tooling changes also require `nix flake check -L`. Document each function and
owner's final verdict, including remaining uncertainty, and bank only verified
100% functions changed within this campaign.

## Pool and allocator owners: retained source model

The GAME startup clear spans `8006e8e0..80070e98`, not merely the definitions
and 190 records ending at `80070e88`. Sequence counters are at tail offsets
10, 12 and 14. OPEN independently clears its records plus the same 16-byte
tail and emits the same descending three halfword stores. The shared layout
and complete-object initialization support extending `KfMapObjectState` to
0x25b8, retaining the ten unknown tail bytes as explicitly opaque and naming
the three known counters as fields. No semantic role is assigned to OPEN's
reset-only counterparts. Every raw word in both GAME map-object units is
unchanged by this owner correction, including the existing partial function.

Both allocator copies place system-heap start/size at +0x50/+0x54 after the
same start/end/cursor/stack field family. Under the current compiler, three
independent tentative definitions cannot explain those adjacent four-byte
words: they request 80+8+8 bytes. One 0x58-byte allocator owner with the two
heap fields reproduces all fourteen retail function bodies, widths and
physical referents without padding or code-generation carriers. This refines
the earlier 0x50-byte model; original typedef and file boundaries remain WIP.
Original ASPSX emits exactly one XBSS request of 0x58 in each overlay, and
one of 0x25b8 for GAME's map-object state. These native requests validate the
reservation extent, not the still-unresolved whole-program COMMON ordering.

The scratch probes compared all 30 functions against their previous linked
words: every function is unchanged. Twenty-eight equal retail completely;
the two pre-existing partial map-object functions retain their prior words.
The canonical inventory removes overlapping interior globals, updates only
their owner names in relocations, and preserves every target address/status.

## ASPSX small-storage calibration

Original ASPSX 1.07 controls establish that `-G8` routes only `.lcomm` requests
of at most eight bytes into fixed `.sbss`. Exported `.comm` requests remain
XBSS in `.bss`, including one-byte requests. Private requests concatenate
without implicit per-symbol alignment: sizes 1,3,2,4,5,8,9,16,1 give G0
`.bss` offsets 0,1,4,6,10,15,23,32,48. Under G8 the two large locals have
`.bss` offsets 0,9 and the others have `.sbss` offsets 0,1,4,6,10,15,23.
These are raw assembler requests, distinct from GCC's eight-byte rounding.

The original assembler preserves absolute HI16/LO16 references to `.sdata`,
`.sbss`, exported BSS and an explicitly sized external under both G0 and G8.
MASPSX instead selected GP addressing, routed small exported COMMON to `.sbss`
and aligned private small symbols individually. The ASPSX-1.07-specific patch
corrects those three behaviors, and the compile driver now forwards its G
setting to both compiler and adapter. Other assembler revisions retain their
existing behavior. A native-versus-adapter integration control compares every
private offset, allocation class and size, complete code and relocation kinds.
Explicit load-delay NOPs keep this allocation control independent of the
adapter's separate scheduling behavior.

Exported COMMON placement is still unresolved: the adapter's existing
`--use-comm-section` option preserves the request for inspection, while the
strict placement gate rejects it. This patch does not claim that the default
flattened analysis `.bss` establishes original whole-program allocation.

## Private player storage

`player_update` already owns two private four-byte limits at `80057e68` and
`80057e70`. GCC requests eight bytes for each. Its initialized previous-input
word is at `80057b30`, separated from the two initialized matrix/offset tables.
The G8 hypothesis explains these as `.sbss`, `.sdata` and `.data` contributions
without changing C types or adding source padding. Preserve native compiler
output and original-assembler allocation records, compare every function to
the baseline, and reject the hypothesis if any banked function changes.

The target model records reviewed fixed reservation sizes separately from
`DATA` object sizes. Only private BSS can use this contract; exported COMMON
is deliberately excluded because its whole-image placement remains unknown.
The ELF writer and relinker preserve `.sbss` as a separate NOBITS section and
still require every member of each contribution to imply one physical base.

## Private event storage

The four card-event handles are used only by `save_system.c`, lie in the
retail small-BSS band at eight-byte intervals, and are written by OpenEvent
before consumption. Treating them as private tentative definitions is a source
hypothesis supported by that storage class and exclusive consumers. The shared
header's former alphabetical extern declarations forced GCC to emit the
otherwise private state in a different order. GCC `toplev.c:compile_file`
reverses `getdecls()` before emitting tentative definitions, so the first
declaration order matters; this is compiler evidence, not name-permutation
steering. Remove those unused public declarations and define the handles in
retail order. Keep the two save buffers exported: other modules consume them.
G8 independently separates the six-byte card root path into `.sdata`.
The two exported buffers remain subject to the unresolved COMMON contract.

## PAD storage

The supplied PAD archive has fixed `.sbss` locals `pad_buf:8` and
`pad_status:8` at 0/8 and exported `PadIdentifier:8` in XBSS `.bss`. The
retail private pair also has eight-byte spacing in both images. The corrected
G8 adapter and private reservation model can represent this established
storage split while retaining four-byte C objects and the SDK's exported
identifier declaration. Check all twelve already exact vendor functions;
none contributes to game progress. The archive revision still differs from
retail and exported COMMON placement is unresolved, so neither PAD unit is
an exact original provider or a closed data owner.

## Initialized owner boundaries

The resource loader's 12-byte `map_resource_path` and eight-byte
`map_mix_tim_filename` have the same demonstrated large/small split as the
actor data. G8 gives `.data` and `.sdata` without splitting one contribution;
verify the exact initializer bytes and every loader function. Its six exported
map buffers remain a separate allocation-order problem.

This mechanism cannot explain every graphics owner. GAME `render_frame`'s
56-byte sprite table, two eight-byte rectangles and 192-byte light matrices
are interleaved in retail with the notification table, the lighting module's
224-byte color matrices and the fixed-cell-window contribution. G8 would move
the rectangles out of their observed contiguous data region and cannot move
the large matrices. OPEN similarly has distant 16-byte background settings
and 32-byte light matrices alongside larger sprite/matrix contributions.
Neither a blanket G8 switch nor a new scalar value repairs those owners.
Original TU contributions remain WIP; regrouping distant definitions solely
to force bases would discard the placement evidence.

## Packed selectors and jump-table destinations

The pinned compiler source gives a concrete constraint on the thirteen
three-byte sound records: MIPS `DATA_ALIGNMENT` raises arrays, unions and
records to at least a word, and `CONSTANT_ALIGNMENT` does the same for
constructors. `varasm.c:assemble_variable` applies both after `DECL_ALIGN`.
Thus separate initialized `SoundRef` records cannot reproduce a three-byte
stride under this target; a smaller type alignment alone is insufficient.
The earlier array rejection predates the updater's direct-call rewrite.
A fresh control with the current body preserves all 1,222 map-object words,
calls and referents. [The updater evidence](game-map-door-sounds.md) records
that call/selection boundary; independent address materialization does not
prove thirteen separate globals.

The three jump-table failures are still real instruction-destination failures:
`render` is shifted by the 336-versus-332-byte `display_initialize`;
`map_object` by the 400-versus-404-byte effect constructor; and `map_scripts`
first differs after the 148-versus-144-byte image-path formatter. Its partial
interaction dispatcher adds a separate CFG difference. Reviewed relocation
referents and constants do not remove these offsets. Existing inline-boundary,
acquisition and filename-cursor controls already reject the obvious source
rewrites; the new aggregate owners leave these instructions unchanged. No
relocation addend is adjusted to hide a code difference.

### Kept packed-array owner

The external-consumer control adds 38 functions in player warp/item use,
effects, map events and map scripts. All 46 functions across the six units
retain every raw instruction and ordered call/reference; 41 equal retail and
five retain their pre-existing partial bodies. One 39-byte `SoundRef[13]`
owner now reproduces the complete initialized payload. All source consumers
use array elements directly, and the inventories remove the twelve interior
globals while preserving every relocation's physical destination and status.
The map-object unit still fails only its separate RODATA addend comparison.

## Final campaign verdict
GAME/OPEN data owners improve from **37/64 to 41/64**: GAME 27/44 to
30/44, OPEN 10/20 to 11/20. The four newly exact data owners are GAME
`player_update`, GAME `map_object_pool`, and both `memory` copies. The
packed sound payload is exact too, but its unit retains a code-derived
RODATA failure. No owner is removed from the denominator.
Every function claimed by the following 27 original owners retains all raw
instructions and ordered references. The exact/total column gives each
unit's function verdict; the only partial functions are listed individually
below. All other claimed functions in these units remain strict 100%.
| Unit | Exact functions / checked | Final data verdict |
| --- | ---: | --- |
| `game.game` | 4/4 | `.bss` size |
| `game.equipment` | 3/3 | `.bss` placement |
| `game.player_death` | 12/13 | `.bss` layout |
| `game.player_core` | 13/14 | `.bss` layout |
| `game.player_update` | 0/1 | 100% |
| `game.memory` | 7/7 | 100% |
| `game.cd_file` | 3/3 | `.bss` layout |
| `game.resources` | 8/8 | `.bss` layout |
| `game.render` | 17/18 | `.rodata` addend; `.bss` size |
| `game.render_frame` | 1/1 | `.data` placement |
| `game.menu_runtime` | 14/16 | `.data` placement |
| `game.save_system` | 23/24 | `.bss` size |
| `game.map_object_pool` | 7/8 | 100% |
| `game.map_object` | 7/8 | `.rodata` addend |
| `game.audio` | 15/15 | `.bss` placement |
| `game.map_scripts` | 14/16 | `.rodata` addend |
| `game.pad` | 6/6 | `.bss` size |
| `open.opening_helpers` | 2/2 | `.bss` size |
| `open.camera_path` | 3/3 | `.bss` size |
| `open.opening_scenes` | 6/7 | `.data` placement |
| `open.memory` | 7/7 | 100% |
| `open.resources` | 10/10 | `.bss` layout |
| `open.render_init` | 3/4 | `.bss` size |
| `open.entity_render` | 3/3 | `.data` placement |
| `open.audio` | 6/6 | `.bss` placement |
| `open.format` | 4/4 | `.bss` size |
| `open.pad` | 6/6 | `.bss` size |

The additional sound-consumer families are `game.player_use_item` (3/3 exact),
`game.player_warp` (5/5), `game.effect_update` (7/8) and `game.map_events`
(5/6). Their 22 functions likewise retain all instructions and references.
Across all 239 checked functions, 224 are raw-retail exact and 15 remain
partial; every score and all 484 function-report entries remain unchanged.

| Image | Function | Retail/source bytes | Strict score before and after |
| --- | --- | ---: | ---: |
| GAME.EXE | `player_add_experience` | 548/528 | 88.824814% |
| GAME.EXE | `player_move_horizontal` | 2088/2128 | 96.568960% |
| GAME.EXE | `player_update` | 6684/6684 | 99.476960% |
| GAME.EXE | `display_initialize` | 332/336 | 98.421684% |
| GAME.EXE | `menu_draw_item_name_frame` | 1976/1976 | 99.570850% |
| GAME.EXE | `menu_draw_window_backdrop` | 1700/1700 | 99.971760% |
| GAME.EXE | `talk_show_dialogue_page` | 164/164 | 98.780490% |
| GAME.EXE | `map_object_probe_forward` | 196/196 | 93.755104% |
| GAME.EXE | `map_object_spawn_effect` | 404/400 | 94.504950% |
| GAME.EXE | `map_show_screen_image` | 144/148 | 88.888885% |
| GAME.EXE | `map_interaction_dispatch` | 2308/2296 | 99.202774% |
| GAME.EXE | `map_world_state_persist` | 696/700 | 97.528730% |
| GAME.EXE | `effect_projectile_update_2d` | 608/608 | 99.934210% |
| OPEN.EXE | `opening_ending_scroll_run` | 1944/1944 | 99.917694% |
| OPEN.EXE | `display_initialize` | 472/476 | 96.652540% |

Original ASPSX independently reproduces all 39 sound-array bytes, the complete
player/resource/save `.data` and `.sdata` contributions, and the fixed `.sbss`
extents in those units and both PAD copies. The adapter preserves those bytes
and extents. Exported COMMON remains unclosed, including four-byte C objects
with eight-byte requests and the 100-byte camera state with a 104-byte request.
The fixed-reservation metadata does not admit those exported objects.

The full build retains 340/362 exact GAME functions, 106/108 OPEN and 1/1 PSX,
with zero comparison artifact failures. It still exits nonzero on the remaining
23 GAME/OPEN data owners and existing reference/coverage gates. Target relink
now verifies GAME 68/77 and OPEN 33/38 units; PSX remains 1/1. This is a
completed repair pass across the requested categories, not full image closure.

Verification passes: 740 repository tests (nine optional skips), 19 targeted
layout tests after making their compiler registry self-contained, Ruff and
`git diff --check`. `nix flake check -L` passes, including the original-ASPSX
allocation controls; its unit suite runs 749 tests with 138 environment/retail
skips. All 34 relocation edits change only `target_name`: sites, destinations,
statuses and evidence are byte-for-byte unchanged. Generated dossiers, native
objects, raw-word ledgers and build reports remain under `build/data-repair/`
and are not committed.

Banking refreshes only the 93 already-exact game functions in the changed
source/profile families. No partial or vendored function is banked, and no
function is newly claimed exact by this data repair.
