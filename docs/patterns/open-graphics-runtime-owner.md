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

## Map-owner extension: Function Match Plan

At `0794850`, the six semantic queries, full disassembly/CFG, callers,
adjacent boundaries, source history and shared declarations were inspected
again for both map functions. `opening_scene0_render_frame` calls traversal
without arguments; traversal supplies signed column/row and a byte cell to
the renderer. The supplied LIBGTE archive independently attributes matrix
operations to `MTX` and `RotTrans` to `SMP`; their authentic header signatures
remain unchanged. Grid selection and traversal are OPEN policy, not those
library bodies. Neither function references strings or unresolved candidate
relocations.

| OPEN function | Retail size; strict baseline | Pre-edit evidence and hypothesis |
| --- | --- | --- |
| `80018bbc render_map_cell` | `0x1d0`; 84.094826% | Nine calls, nine reviewed address pairs, one internal jump, 80-byte frame. Byte attribute wraps before the unsigned 99 check; orientation selects 32-byte matrices; positions use 2000, -100 and unsigned low halfwords of view coordinates. At `80018d2c..80018d5c`, the quadrant base `8006e0c8` supplies view at -128 and light matrices at +128. Test whether the complete owner recovers that cross-field chain without changing coordinate expressions or call delay-slot semantics. |
| `80018d8c opening_render_map_cells` | `0x140`; 84.062500% | Two calls, nine reviewed address pairs, one internal jump, 48-byte frame. Signed high yaw byte selects one of sixteen 204-byte windows. Retail reloads the stored active pointer at `80018dec/80018df0`; canonical code omits it. Row/column counters narrow to bytes, skipped rows advance by full halfword width, and unsigned bounds are 100. Test the same owner declaration, including its active-window field, without manufacturing a reload. |

Create only an alternate map-unit source with real owner-field expressions.
Remove its separate active-window definition from that temporary source;
the window table at `800439d8` remains independently owned. Rebuild and audit
numeric relocation targets, call order, first raw divergence and the exact
initializer controls. A positive map result must precede the wider banked-
consumer audit; no inventory migration or exact claim follows from a fuzzy
score or a raw-only candidate comparison.

### Map extension result

The same complete declaration makes `opening_render_map_cells` equal all
320 linked retail bytes. Both calls, all nine absolute data targets, the
active-pointer reload, branch targets and delay slots agree without changing
its C statements. The old 312-byte code hoisted the view-cell reads and
omitted that reload. This is a full numeric-relocation comparison, not a
banked strict objdiff result for the unadopted root identity.

`render_map_cell` recovers the quadrant/view/light chain and the exact saved
register set. Its last 96 bytes, starting with the quadrant-base load through
the return delay slot, equal retail after relocation. All nine calls retain
their ordered targets and the extra independent light-array pair disappears.
It is still 472 bytes versus retail 464 (canonical 468). Ignoring early branch
displacement changes caused by that size difference, the first divergence is
the empty delay slot at `80018c1c`: retail loads constant one there, whereas
the candidate loads it later into `a2`. Coordinate arithmetic and view-load
interleaving also differ. No new source-order or register-steering experiment
is justified by that symptom alone.

The initializer, allocator and color-matrix controls still preserve every
linked instruction. The regression test now also checks the complete map
traversal, the renderer's exact suffix and its literal ordered calls/data
targets; an incorrect root address fails the traversal comparison. The two
map verdicts are **raw-exact candidate traversal** and **partial renderer with
recovered shared base**, respectively. Canonical sources remain untouched.

The next required step is the broader banked-consumer audit, starting with
the display/TMD/projection unit. There are seventeen OPEN source files with
references to candidate members; they must share this declaration if the
ownership migration is adopted. Do not migrate only the newly raw-exact
traversal while retaining incompatible separate globals elsewhere.

### Map checkpoint verification

The canonical map unit was freshly compiled with its pinned manifest profile;
its strict scores remain 84.094826% and 84.062500%. All 399 workspace tests
pass without skips, Ruff and `git diff --check` pass, and `nix flake check -L`
passes (399 tests, 49 optional controls skipped in isolation). Full `kf build`
still fails only the recorded strict gates: one OPEN and thirteen GAME data
units, plus four GAME historical-best deficits. OPEN remains 91/108 banked
exact, with no production source, inventory or baseline changed by this pilot.
