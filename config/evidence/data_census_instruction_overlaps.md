# Instruction bytes incorrectly admitted by the string census

## Function Match Plan

This campaign removes only reviewed byte-pattern string claims that duplicate
instruction storage. Preserve the C bodies, signatures, function boundaries,
DATA/RODATA ownership, relocation rows and vendor attribution. Retain the
original rejected rows and their raw instruction windows in the accompanying
`data_census_instruction_overlaps.tsv`; that file is evidence, not admitted data.

The known-reference audit initially exposed seven conflicting string ranges.
Reviewing all scan rows within the same eight fully inspected function bodies
finds **31 rejected claims, totaling 147 bytes** (27 GAME, four OPEN). They are
unaligned slices of address formation, arithmetic, argument setup, calls,
branches or return delay slots. None has a data-reference consumer supporting
a separate string allocation. Classification is based on the complete decoded
CFG, consumers and source/archive evidence, not merely decodability of a word.

The broader inventory has 1,891 string-scan/code overlaps before this change
(GAME 1,004; OPEN 887). This is not permission to delete all of them. The
remaining 1,860 claims stay WIP until individually reviewed. In particular,
an admitted function bounding box or printable bytes alone cannot prove whether
an embedded table/string belongs to code or data.

## Per-function evidence snapshots

All addresses are image-qualified. Function sizes below include the return's
delay slot; every selected function has one admitted body fragment. No source
function is changed or newly claimed by this campaign.

| Image | Function / VA | Body / CFG blocks | Starting strict state | Rejected ranges owned |
| --- | --- | --- | --- | --- |
| GAME | game_main_loop / 0x800146b8 | 0x2e4 / 11 | 100% | 1 |
| GAME | player_use_item / 0x80018054 | 0x45c / 43 | 100% | 9 |
| GAME | render_map_cell / 0x8001e5ec | 0x250 / 24 | 76.783780% | 5 |
| GAME | map_object_pool_load / 0x80031008 | 0x448 / 43 | 100% | 7 |
| GAME | note2pitch / 0x80042da0 | 0xbc / 14 | vendored, no reconstruction unit | 3, including shared boundary |
| GAME | SpuVmSelectToneAndVag / 0x80042e5c | 0xb0 / 7 | vendored, no reconstruction unit | 3, including shared boundary |
| OPEN | note2pitch / 0x80022bc0 | 0xbc / 14 | vendored, no reconstruction unit | 2, including shared boundary |
| OPEN | SpuVmSelectToneAndVag / 0x80022c7c | 0xb0 / 7 | vendored, no reconstruction unit | 3, including shared boundary |

`game_main_loop` has no parameters or escaping result. Its sole direct caller
is `main` at 0x800142ec. Initialization clears game state, initializes SDK/game
services and installs the frame-pacing callback through OpenEvent; the loop
calls player, actor, map-object, effect, map-event, audio and rendering services,
then closes the event and calls `game_shutdown`. The false string at 0x8001482a
spans `addiu s2,s2,-22712` and the loop-head `jal player_update`. The jumps
at 0x80014960 and 0x8001496c target that call at 0x8001482c. The epilogue ends
with `jr ra; addiu sp,sp,32`. Neighbors are `player_warp_shimmer_at_player`
(0x80014674, 0x44) and `game_shutdown` (0x8001499c, 0x38).

`player_use_item` takes a byte item ID, proved by the a0 transfer and `andi
0xff` before subtracting 0x2a for the 33-entry switch. Its caller is
`player_update` at 0x80018974. The nested object switch has 42 entries. The
function uses rsin/rcos to form a reach point, traverses map objects, adjusts
links/player/effect state and calls notification, audio, warp and image helpers.
The nine rejected scans slice its address pairs, loop setup and update/exit
instructions. For example, 0x80018127 and 0x8001824f cross LI/MOVE pairs at
the two map-object iteration heads; direct backedges target 0x80018128 and
0x80018250. The shared epilogue restores the 72-byte frame in the return delay
slot. Neighbors are `map_event_show_person_image` (0x80017fa4, 0xb0) and
`player_death_apply_visual_fade` (0x800184b0, 0x90).

`render_map_cell` uses full-word column/row arguments and narrows its third
argument to a byte. The only caller, `render_map_cells` at 0x8001e938, supplies
the live cell and coordinates. It interprets attribute/orientation/height grids,
builds the cell transform, calls Psy-Q matrix/GTE services and selects/enqueues
the map TMD. Five rejected scans slice those instructions, including the
ADDIU/MOVE pair at 0x8001e6a8/0x8001e6ac (branch targets) and the ANDI/JAL
pair at 0x8001e810/0x8001e814. The return delay slot restores 120 bytes.
Neighbors are `render_screen_sprite` (0x8001e480, 0x16c) and
`render_map_cells` (0x8001e83c, 0x168). Its pre-existing non-exact source is
retained without attributing the remaining codegen differences.

`map_object_pool_load` takes a read-only placement pointer from
`map_resources_load` at 0x8001b67c. The input advances in 20-byte records and
the destination in 44-byte objects, over 190 slots with a byte sentinel. It
reads definition/map data, marks collision occupancy/edges, constructs effects
and starts special object actions. Seven scans slice its address pairs,
branch/call delay slots and loop argument setup. The scan at 0x80031243
crosses the LI delay slot of `j 0x80031374` and the next switch case's MOVE.
The actual table words at 0x80012824, 0x80012848, 0x8001284c and 0x8001287c
contain 0x80031244. Their existing candidate tier is retained; this correction
does not automatically promote the switch relocation census. The return delay
slot restores 96 bytes. Neighbors are `map_object_definitions_load`
(0x80030fdc, 0x2c) and `map_object_distance_to_point` (0x80031450, 0xa8).

The semantic CFG currently leaves indirect switch destinations unresolved and
labels their disconnected blocks unreachable. That is not proof of dead code:
the selector/load/JR sequences, retail pointer rows and exact source objects
support these compiler-case instructions. Resolving indirect-control closure
remains separate work; no tier or CFG filter is relaxed here.

The four SDK bodies remain excluded from game progress. `note2pitch` consumes
private voice state and a halfword pitch table, returning the narrowed pitch
in the delay slot. `SpuVmSelectToneAndVag` is a leaf that fills the two byte
output arrays passed by `SpuVmKeyOn`, using private tone/voice state and
32-byte tone records. Its caller is GAME 0x80043aac / OPEN 0x800238cc, and
its return narrows the count to a byte. The two identical `b0!H` scans begin
in `note2pitch`'s `andi v0,v1,0xffff` return delay slot and finish in the next
function's `move t1,a0` entry instruction. The boundary pairs are GAME
0x80042e58/0x80042e5c and OPEN 0x80022c78/0x80022c7c. They are neither
private literals nor padding. `SpuVmKeyOffNow` precedes note2pitch;
`SpuVmDoAllocate` follows the selector in both images.

Vendor negative controls for the four game bodies are the existing semantic
dossiers `game_semantic_math_lifecycle.tsv`,
`game_semantic_player_interactions.tsv`, `game_tu_render_map_cells.tsv` and
`game_semantic_map_objects.tsv`: SDK callees are identified, while the game
state, object policy and map interpretation have no vendored/FID signature.
For the SDK bodies, `functions_vendored.tsv`, `overlay_lineage.tsv` and
`tests/test_open_small_sdk.py` retain reproducible LIBSND/VMANAGER evidence.
The Release 2.5 selector is at object offset 0xad0, size 0xb0, with a known
private-data-layout immediate difference; no exact historical SDK revision is
claimed. No library body is reconstructed to increase game progress.

## Reproduction and ownership controls

After `kf init`, use `kf sema --image game|open` with `addr`, `disasm --blocks`,
`xref`, `xref --callees`, `strings` and `match` for each image-qualified target.
Inspect adjacent boundaries and caller argument windows listed above. The
game bodies and SDK selector have no referenced string consumers. The SDK
match query correctly reports no reconstruction unit, not a game-match failure.
`git log -S '0x8001482a' -- config/retail/data.tsv` traces the scan entry to
`d14f4fd`. `seed_retail.data_rows` prioritizes string scans over generic data
but does not test them against function ranges, explaining how these claims
entered the census. Seed outputs remain proposals and are not rerun into
curated inventories by this campaign.

The audit now reports `code-data-owner-overlap` for every reached unfragmented
function whose body intersects source/config data claims, even without a
reference to the intersection. Each per-function issue preserves every
conflicting datum's metadata and clipped intersection. It does not automatically
reject either model, claim that bytes execute dynamically, follow otherwise
unreached vendors, or infer fragment-hole coverage. Existing reference tiers
and data-owner selection remain unchanged.

Tests protect all 31 reviewed rejections against reintroduction, verify their
code coverage byte by byte (including SDK boundaries), check the entire
three-image payload partition and compare the recorded instruction windows to
hash-verified retail files when available. Controls retain the original seven
control/table witnesses and their confidence tiers. Synthetic audit cases cover
unreferenced overlaps, multiple metadata/source claims, boundary clipping,
image isolation, candidate vendor paths and fragmented extents.

## Final verdict

All 117 units were rebuilt; all 1,722 delinked object hashes and all 117
reconstructed object hashes are unchanged. All 484 reported function scores
are unchanged, preserving all 354 historically exact game functions. Focused
rebuilds retain exact `game_main_loop`, `player_use_item` and
`map_object_pool_load`; `render_map_cell` remains 76.783780%, with its first
existing divergence at the stack frame (120 retail bytes versus 88 compiled).
Other non-exact functions in the shared units remain unchanged. The four SDK
functions retain their vendored status; no function is newly banked.

The complete retail payload partition still validates. The 15 target-owner
ambiguity occurrences and the one cross-owner relocation-site diagnostic
caused by seven of these rows are gone. Config-only reached ranges fall from
673 to 666 because seven phantom code-as-data nodes are removed, not because
any genuine allocation gains a source comparison. Roots, reached vendor
functions and source-owned range counts remain unchanged.

The expanded audit reports 1,441 distinct conflicting data claims within 418
reached function bodies: GAME 773 claims / 240 functions, OPEN 668 / 178.
These are a subset of the 1,860 remaining overlaps across the entire admitted
function inventory, not evidence that the rest are unreachable in the game.
The report's known-reference scope and incomplete-coverage flag remain explicit.

All 428 local repository tests pass without skips, as do Ruff,
`git diff --check` and `nix flake check -L`. The isolated flake suite has 48
expected local-retail/oracle skips. `kf build --reconfigure` rebuilt all units,
regenerated all three reports and ran all three default image checks. It still
fails unresolved ownership/reference issues and the unchanged 14 data-addend
mismatches (strict data 49/63). No gate, byte mask or baseline exception was
introduced to hide those failures.
