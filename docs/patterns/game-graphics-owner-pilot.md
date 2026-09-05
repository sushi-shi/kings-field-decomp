# GAME graphics owner: complete-clear pilot

This is a tested ownership hypothesis, not an adopted DATA claim or a recovered
original C declaration. The complete GAME graphics clear is larger than the
current fragmented globals. The pilot tests one shared declaration against
independent retail instructions before changing production ownership.

## Independent bounds and access evidence

At GAME `800146cc/800146d0`, `game_main_loop` materializes destination
`80070e98`. The `memset` call at `800146dc` receives zero and length `249cc`,
with the length's low half in the call delay slot at `800146e0`. The six raw
words are `3c048007 24840e98 00002821 3c060002 0c0140c3 34c649cc`.
The half-open clear ends at `80095864`; `audio_state` begins four bytes later
at `80095868`. Neither that four-byte gap nor audio storage belongs to this
candidate. Five subsequent clears have separate destinations and lengths.
A single memset span proves its accessed range, not a historical C aggregate.

The map-cell matrix chain supplies separate ownership evidence:

- `8001e7dc/8001e7e0` forms `80095760`, the view-quadrant matrix base.
- `8001e7e4` derives the view matrix at base minus 192; the next instruction
  derives the selected quadrant from the orientation times 32.
- `8001e804` adds 128 to the same saved base, producing `800957e0`, and the
  `SetLightMatrix` delay slot adds the same orientation offset.

Conversely, the exact initializer uses four independent absolute pairs for
`800957e0`, `80095800`, `80095820`, and `80095840`. Any proposed owner must
explain both functions together. Absolute reference formation alone does
**not** prove four matrices are a separate complete object. The old inventory
rationale asserting that conclusion is corrected; its name, extent, type,
linkage status, and all relocation sites/targets remain unchanged.

The complete declaration retains the existing typed objects at their physical
positions. Important groups, as offsets from `80070e98`, are:

| Offset | Current interpretation in the candidate |
| --- | --- |
| `00000..20028` | Double-buffered display state and ordering tables. |
| `20028..20108` | Two authentic SDK DRAWENV and two DISPENV records. |
| `20108..20110` | Unresolved eight-byte interval. |
| `20110..20134` | Eight TMD slots and selected asset. |
| `20134..20224` | Unresolved registry interval; not a proved 60-pointer array. |
| `20224..20318` | Current vertex pointer and twelve typed pool records. |
| `20318..241a0` | Unresolved projection/morph interval; no invented capacities. |
| `241a0..241c0` | Three effect texture pages, three CLUTs, and unresolved intervals. |
| `241c0..241d6` | Active material and HUD/notification texture state; unknown byte at `241cd`. |
| `241d6..241f4` | Eight notification message IDs and typed queue state. |
| `241f4..24200` | Floor texture state/count and unresolved six-byte interval. |
| `24200..24800` | 64 typed floor-item records. |
| `24800..24808` | Two reset words whose roles remain unresolved. |
| `24808..24948` | Existing view/lighting/fog/quadrant state. |
| `24948..249cc` | Four light-quadrant matrices and active visibility-window pointer. |

All individual offsets and the complete size are measured from the pinned C
compiler's output, without adding C size assertions. Opaque intervals are
explicitly unresolved storage, not claimed original padding. GAME's existing
projected-vertex and morph identities still describe only their referenced
eight- and 24-byte prefixes. Neighbor distances of 1000/1001 vectors and OPEN's
separate capacity evidence do not prove those GAME capacities; see
[tmd-projection.md](tmd-projection.md).

## Function Match Plan and evidence snapshots

The initial five-witness campaign at `37551a6` inspected the six semantic
queries, complete retail CFG/disassembly, callers, adjacent boundaries, source
history, SDK declarations, and current strict match reports. The shared-owner
experiment changes only these witnesses in temporary copies of their three
TUs, using the same declaration and real field expressions. Production claims
and unrelated function bodies are not rewritten. The startup clear uses the
whole candidate's address and `sizeof`, not an out-of-bounds prefix view.

| GAME function | Hex size; baseline | Signature, references, and first hypothesis |
| --- | --- | --- |
| `800146b8 game_main_loop` | `2e4`; 100% | No arguments from `main`; 32-byte frame, 43 proven calls, 23 validated references including three internal jumps, no strings/candidates. Preserve all six clears, callback pointer, loop policy, calls and epilogue while expressing the graphics clear through the complete owner. |
| `8001bce0 render_initialize` | `2d8`; 100% | No arguments from `display_initialize`; 32-byte frame, 15 calls and 41 address pairs, no strings/candidates. Preserve allocation `32c80`, buffer stride `19640`, yaw values 0/c00/800/400, all nine light constants, four absolute output pairs, SDK texture calls, and notification byte stores. |
| `8001c184 render_set_view_transform` | `12c`; 96.8% | Nullable VECTOR/SVECTOR pointers passed unchanged by `render_frame`; 32-byte frame, two calls and seven pairs, no strings/candidates. Copy full typed inputs only when non-null, divide position x/z by 2000 into halfword cells, then rebuild both matrices unconditionally. Correct the independently disproved branch boundary before using this as an exact owner control. |
| `8001e5ec render_map_cell` | `250`; 76.783780% | Word column/row and byte cell from its sole external caller, traversal; 120-byte retail frame, nine calls, fifteen validated references including five internal jumps, no strings/candidates. Preserve door IDs 44/45/46 -> 17/18/19, signed light timer -1, byte-wrapped object selection, 2000/-100 coordinates and low-halfword view reads; test the cross-matrix base chain without changing local types/CFG. |
| `8001e83c render_map_cells` | `168`; 100% | No arguments from `render_frame`; 48-byte frame, two calls, thirteen validated references including two internal jumps, no strings/candidates. Preserve pitch `(vx+511)` halfword check against 1023, signed yaw high-byte indexing, 204-byte windows, active-pointer store/reload, byte loop counters and full-width skipped-row stride. |

All return instructions include their delay slots. Source history includes
the visibility-state campaign `c17c8a2` and display/parser refinements
`110ec5b`; the OPEN complete-owner investigation is sibling evidence, not
proof of GAME ownership. These functions implement game policy, not vendor
bodies. The curated archive/FID evidence separately attributes SDK operations:
LIBGTE GEO (`RotMatrix`), MTX (`MulMatrix0`, matrix register setters), SMP
(`RotTrans`), LIBGPU texture APIs, and LIBAPI C43 (`memset`). The existing
authentic SDK types and signatures are retained. The pinned GCC 2.5.7 probe
does not establish historical compiler attribution.

## Branch correction and pilot verdicts

Retail `8001c23c` branches to `8001c26c` when the rotation pointer is null.
That destination is **before** the first `RotMatrix`, not after it. The local
pitch vector's y/z components are subsequently zeroed on both paths. Moving
the first call and these two stores outside the source's non-null block makes
the canonical `render_set_view_transform` strict **100%**, with all 300 linked
bytes exact. The old block incorrectly skipped rebuilding the view matrix and
left two local components uninitialized on the null-rotation path.

| Witness | Shared-owner result |
| --- | --- |
| `game_main_loop` | All 740 linked bytes exact, including the complete `sizeof` clear, callback address, and all other clear/call sites. |
| `render_initialize` | All 728 linked bytes exact, including all four independent absolute light-array pairs. |
| `render_set_view_transform` | All 300 linked bytes exact after the canonical CFG correction. |
| `render_map_cells` | All 360 linked bytes exact, including active-pointer publication/reloads and local static-window address. |
| `render_map_cell` | Partial: candidate 576 versus retail 592 bytes. All nine ordered call targets and ten materialized data addresses agree. The 18 words from quadrant-base formation through the final enqueue call's delay slot match exactly. The first divergence is still the 88-byte candidate frame versus retail 120; the epilogue and earlier byte-narrowing/CFG differences remain. No whole-function exact or new objdiff score is claimed for this alternate source. |

`tests/test_game_graphics_owner_probe.py` keeps per-function raw comparisons
and negative controls. A four-byte wrong gap changes measured layout/extent;
a four-byte wrong owner root breaks every exact witness. Separate controls
shift the callback pointer and the static window's section base: instruction
comparison must fail even though direct call targets remain unchanged. The
shared raw linker now handles function-pointer HI16/LO16 and unnamed ELF
section symbols explicitly. It resolves genuine REL addends, without masks.
The canonical view-transform control also recompiles the old faulty block:
both SDK calls still exist, but the retail branch comparison fails.

## Remaining ownership work

This pilot does not add DATA/RODATA claims, change numeric relocation targets,
declare config-only bytes independently compared, or reduce the reachable-data
backlog. It establishes that the complete-region hypothesis can jointly
explain the initializer and matrix-chain witnesses, and removes one false
reason to reject it. No game is booted or executed by these controls.

Before canonical adoption, audit the remaining display, TMD, pool, floor-item,
notification, material, and graphics consumers against this same owner. The
registry and projection/morph boundaries still need better evidence or an
explicitly unresolved representation that preserves their real typed uses.
Preserve every banked function, remove superseded interior DATA claims and
global identities together, and retain every numeric relocation destination.
Only then can a production owner count toward strict data matching and linked
executable reconstruction. Alignment, BSS extent, and relink failures remain
mandatory checks; passing this text-only pilot cannot waive any of them.

## Verification checkpoint

The production render object was forcibly recompiled and the full `kf build`
run. Against `37551a6`, all 484 function-score rows are unchanged except
`render_set_view_transform`, 96.8 -> 100%. All 112 target objects and the other
111 production source objects retain their hashes. No exact function regresses;
GAME advances 262 -> 263/362 exact, and all images together 360 -> 361/471.
Only the view-transform function is selected for banking.

All 616 local repository tests pass without skips; Ruff and `git diff --check`
pass. `nix flake check -L` passes, with 116 expected optional native/retail
controls skipped in its isolated 616-test run. The full build remains red:
6/60 source data-owning units pass the complete strict check, all four SDK
contributions pass, and 110/116 targets relink faithfully. The same six
section-base conflicts and 620 known-referenced config-only ranges without
independent comparison remain. Production `DAT_` references do not decrease.
