# OPEN complete graphics-runtime owner pilot

This is an ownership hypothesis, not an original-source or compiler claim.
The committed declarations and inventories remain unchanged during the pilot.

## Independent extent and access evidence

OPEN `opening_run` loads destination `80049a48` at `800156dc/800156e0`, zero
at `800156e4`, and length `0x24788` at `800156e8/800156f0` for the `memset`
call at `800156ec`. The region ends exactly at `8006e1d0`, the independently
modeled audio-state start. The following call clears the separate `0x510`-byte
entity state. These are proven calls with validated address pairs; a clear
region alone does not prove one historical C aggregate.

Three other access chains cross the current separate graphics owners:
display setup derives display-environment and DFE fields from a draw field;
TMD emission derives projected vertices, ordering-table pointer and material
RGB from the current-asset slot; map-cell rendering derives light-quadrant
matrices from view-quadrant matrices. The smaller render-state extension
previously failed the exact initializer control. A complete-region model is
a distinct hypothesis, not permission to ignore that control.

The temporary shared declaration preserves every existing supported nested
type and each known member's physical address. Offset checks cover all fields,
and a size check covers `0x24788`. The unresolved intervals are `+20108..20110`,
`+2011c..20120`, `+20124..20138`, `+22078..23fe0`, and `+24786..24788`.
Their byte views do not imply recovered objects or additional padding.
Projection scratch remains 1000 eight-byte records, not an invented larger
array spanning the following gap. The allocation counter at `80075928` and
visibility windows at `800439d8` lie outside the region and remain separate.

## Function Match Plan and pre-edit snapshots

At `2560194`, all six queries, full retail disassembly/CFG, callers, adjacent
boundaries, source/history, shared types and SDK provider boundaries were
checked for the affected witnesses. No signature, algorithm, literal, call,
compiler profile or GAME source changes are part of this pilot.

| OPEN function | Retail size; strict baseline | Relevant control |
| --- | --- | --- |
| `80018344 render_enqueue_unlit_triangles` | `0x2a4`; 93.455620% | Eight calls, one internal jump, one address pair; signed bias and byte/halfword packet fields; current-asset/projected/material/OT chain. |
| `80016908 render_initialize` | `0x1d4`; 100% | Eleven calls, 28 address pairs, 32-byte frame; independent light-array pairs must remain exact. |
| `80016adc display_initialize` | `0x1d8`; 89.203390% | Eighteen calls, two internal jumps, sixteen address pairs; mode `0xfe`, real SDK environment fields and converged lighting policy. |
| `80016cb4 primitive_buffer_allocate` | `0x84`; 100% | `u16` request, one printf call, one internal jump, five address pairs; buffer overflow print loop and separate counter. |

The same initializer unit contains `800168dc lighting_set_active_color_matrix`
(`0x2c`, 100%); its independent load-image matrix table is not rewritten.
The controller supplies extent evidence only and is not edited. The functions
remain game-owned policy around the separately identified SDK/archive services.

Build alternate sources under `build/open-runtime-owner-probe/` using one
coherent header, with original global accesses replaced by real fields of the
candidate owner. Do not use field aliases, out-of-bounds pointer arithmetic or
different per-file views. Resolve the candidate root to `80049a48` when auditing
instructions; symbolic-name differences alone cannot decide the result.
Reject migration if the owner fails to jointly explain the emitter and exact
initializer. Do not curate new retail relocations to fit the candidate.

## Pilot result and next discriminator

The complete-region model preserves every linked instruction of
`render_initialize` (468 bytes), `primitive_buffer_allocate` (132 bytes),
and `lighting_set_active_color_matrix` (44 bytes), including all address
targets, calls and delay slots. In particular the four independent light-array
pairs survive. This contrasts with the earlier small render-state extension;
its failure does not rule out the complete-region model.

The unlit candidate is 688 bytes rather than the current 692 (retail 676).
It forms the current-asset slot and derives projected vertices at +32, and
derives the material-color address at +16076 from that slot. Only two absolute
pairs remain, targeting `80069b60` and `80069a6c`, versus four in the current
source. All eight calls retain their ordered physical targets. It still saves
the narrow bias on the stack, keeps a separate color pointer, and uses an
absolute ordering-table load; the first raw divergence is still the bias save.
The source model is not an exact emitter reconstruction.

Display setup becomes 484 bytes instead of 496 (retail 472). Its 18 calls
and numeric targets remain correct; the independent display-environment
reference on the mode-`0xfe` path becomes a shared-base access. Four absolute
DFE references and the 40-versus-48-byte frame discrepancy remain. Neither
candidate's raw comparison is being reported as a new strict objdiff score.

`tests/fixtures/open_runtime_owner_probe.h` preserves the coherent candidate
and checks every field offset and the full extent using the pinned compiler.
`tests/test_open_runtime_owner_probe.py` reconstructs the initializer-unit
candidate in a temporary directory and resolves real ELF REL addends before
comparing every word with retail. A four-byte gap error fails the layout
checks; shifting the proposed root by four bytes fails the instruction
comparison. No byte masks, symbol-only equality or game execution are used.

This is positive evidence for continuing the ownership investigation, not
enough to migrate the canonical inventory. The next discriminator is whether
the same complete declaration also recovers the map-cell matrix-base chain
and preserves all other banked consumers. No production C, identity, claim,
relocation or baseline is changed, and no new exact function is banked.

## Verification checkpoint

The restored canonical OPEN sources were rebuilt, with unlit and floor-item
strict scores unchanged at 93.455620% and 98.795180%. All 398 workspace tests
pass without skips, as do Ruff and `git diff --check`. The staged
`nix flake check -L` passes all checks (398 tests, 48 optional controls skipped
in isolation). Full `kf build` still fails the existing strict data gates for
one OPEN and thirteen GAME units, plus the four GAME historical-best deficits.
OPEN remains 91/108 exact; no GAME reconstruction or baseline is changed.
