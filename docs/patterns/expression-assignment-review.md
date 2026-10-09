# Assignments inside expressions

## Metric

`kf verify board` includes **assignments in expressions**, measured by
libclang over all 97 manifest C variants in retail C and modern C++ modes.
`kf verify board --assignments` lists the findings. For machine-readable output:

```sh
python -m scripts.kf.expression_assignments --json
python -m scripts.kf.expression_assignments --check
```

Count plain and compound assignments nested in arguments, conditions, returns,
initializers, comma expressions and other assignments, including overloaded
C++ assignments. Standalone assignments and the statement clauses of `for`
loops are excluded. Macro spelling locations distinguish project assignments
from vendored SDK implementations; project assignments passed into SDK macros
still count. Deduplicate matching expansion/spelling/operator sites across
translation units and language modes, retaining every image/function context.
Different overlay macro owners remain distinct contexts. Parse errors fail the
census instead of returning an incomplete zero. Inactive branches outside the
two selected language views are outside this metric.

The initial census has **179 sites**; the retained exact cleanup has **9**.
The committed down-only floor is 9. Zero remains the target; no exception
removes a retained assignment from the count.

## Kept source changes

- Capture the VAB header, advance the cursor, capture the body chunk, then call
  the audio loader. All four OPEN calls and the GAME call use statements.
- Advance every resource cursor before its consumer, including common resource
  tables, placements, TMD registration and archives.
- Put CD polling and formatter character reads in statements, preserving the
  terminating read and formatter `continue` behavior.
- Preserve both event-lookup and object-scan routes when splitting the guarded
  interaction condition. The non-idle and missing-event routes still scan objects.
- Expand seven project expression macros into their ordinary operations at
  existing call sites and remove the unused definitions. New statement blocks
  changed instructions; direct statements recover the exact functions.
- Split exact attachment-scale, packet-coordinate, material-color and modern
  enum/Boolean assignment chains. Sprite corners and texture edges use genuine
  shared values, preserving narrow conversion without extra field reloads.
- Keep one typed menu-packet helper for the real generic SDK blending boundary.
- Capture the first draw buffer's SDK dithering byte, then enable the second
  and first buffers in separate statements. Both display initializers remain
  strict 100%, including the complete affected comparison objects.

## Retained exact cases

| Image/function | Remaining sites | Statement controls |
| --- | ---: | --- |
| GAME `effect_update_dispatch` | 5 | Separate field reads: 99.69785%; shared `u16` scale: 99.62963%; first raw divergence +0x38. |
| GAME `map_object_mark_collision_edge` | 2 | Same parameter stores: 87.69231%; complete grid owner: 74.02098%; first branch difference +0x30. |
| GAME `tmd_register` | 1 | Direct separate stores or the state owner: 64 bytes, 48%; real destination pointers: 60 bytes, 86.666664%; explicit slot selection: 60 bytes, 86%. Retail is 60 bytes. |
| OPEN `tmd_register` | 1 | Same paired controls: 64 bytes, 48%; destination pointers: 60 bytes, 86.666664%; explicit slot selection: 60 bytes, 86%. |

These are probe observations, not explanations of original source or historical
compiler behavior. The final four functions retain their original 100% source.
No inline assembly, fake variables, volatile carriers, padded storage, altered
relocations or compiler flags were introduced to force zero.

## Display dithering follow-up

Ordinary duplicate stores initially changed GAME/OPEN instruction ordering;
using the DRAWENV array owner throughout each initializer also changed code.
The retained form captures the first environment's actual SDK `dtd` member,
sets the second environment's flag, then writes the captured member. It uses
an `u8 *` only for that one byte, without treating the member as an enclosing
owner. Both complete comparison objects equal the paired original objects,
including instructions and relocation records. GAME `display_initialize`
remains 332 bytes and OPEN remains 472 bytes, both strict 100%.

The paired controls are under
`build/cleanup-evidence/expression-assignments/display-dither-control/`.

## Evidence

Local generated evidence is under
`build/cleanup-evidence/expression-assignments/`: the initial inventory,
image-qualified retail disassembly/references/strings/match state, original
source controls and strict candidate scores. Tests exercise real libclang ASTs,
macro provenance, overloaded assignments and fail-closed parsing.

All 97 native-derived comparison objects and all three complete EXE files are
byte-identical to the captured baseline. CPE entry points and ordered loaded
memory are also identical. The GAME CPE's raw record segmentation differs;
raw CPE-file equality is not claimed for that image. PSX and OPEN CPE files
remain byte-identical.

Full `kf build`, the cleanliness gate, all 97 type variants with zero enum
literals, Ruff and all 890 local tests (no skips) pass. `nix flake check -L`
passes; its isolated SDK suite runs 890 tests with the expected 147 retail-data
skips. Existing non-exact functions and separate data/relink failures retain
their previous scope. No function was newly banked.
