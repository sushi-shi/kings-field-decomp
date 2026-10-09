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

The initial census has **179 sites**. The cleanup reached 4 before the user
requested retaining readable polling/formatter conditions and sprite chains.
The current count and approved floor are **46**: 42 sites in those idioms plus
four exactness exceptions. All remain counted; zero is no longer the target.

## Kept source changes

- Capture the VAB header, advance the cursor, capture the body chunk, then call
  the audio loader. All four OPEN calls and the GAME call use statements.
- Advance every resource cursor before its consumer, including common resource
  tables, placements, TMD registration and archives.
- Retain CD polling and formatter character reads in their `while` conditions,
  with compact `{}` for empty polling bodies.
- Preserve both event-lookup and object-scan routes when splitting the guarded
  interaction condition. The non-idle and missing-event routes still scan objects.
- Expand seven project expression macros into their ordinary operations at
  existing call sites and remove the unused definitions. New statement blocks
  changed instructions; direct statements recover the exact functions.
- Split exact attachment-scale, packet-coordinate, material-color and modern
  enum/Boolean assignment chains. Keep the readable shared-value chains for
  sprite corners, screen coordinates and texture edges.
- Keep one typed menu-packet helper for the real generic SDK blending boundary.
- Capture the first draw buffer's SDK dithering byte, then enable the second
  and first buffers in separate statements. Both display initializers remain
  strict 100%, including the complete affected comparison objects.
- Use an explicit stored halfword for ground-trail scale copies and actual
  scale-member destinations plus one narrowed value for each radial growth.
  All five effect-dispatch sites are removed without changing its comparison
  object or its existing 99.82781% retail residue.

## Retained exact cases

| Image/function | Remaining sites | Statement controls |
| --- | ---: | --- |
| GAME `map_object_mark_collision_edge` | 2 | Same parameter stores: 87.69231%; complete grid owner: 74.02098%; actual edge cell: 560 bytes, 94.38461%; row view: 572 bytes, 92.72727%; shared column: 552 bytes, 88.61539%. Retail is 572 bytes. |
| GAME `tmd_register` | 1 | Direct separate stores or the state owner: 64 bytes, 48%; real destination pointers: 60 bytes, 86.666664%; explicit slot selection: 60 bytes, 86%; explicit encoded slot: 60 bytes, 86.666664%. Retail is 60 bytes. |
| OPEN `tmd_register` | 1 | Same paired controls: 64 bytes, 48%; destination pointers or encoded slot: 60 bytes, 86.666664%; explicit slot selection: 60 bytes, 86%. |

These are probe observations, not explanations of original source or historical
compiler behavior. The three banked functions retain their original 100%
source. The earlier description of the effect dispatcher as exact was incorrect;
its five sites are now removed while preserving its existing residue.
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

## Effect scale follow-up

GAME `effect_update_dispatch` already scores 99.82781%, not 100%, in the
captured baseline. A fresh ordinary objdiff comparison and the semantic
navigator confirm that result. Ground-trail shrink uses a wide value for
its signed comparison and an explicit halfword for the stored axes. Radial
and lightning growth use actual halfword destinations and a shared narrowed
result. Each growth control separately, both together, and the final combined
ground/growth control preserve the complete comparison object byte for byte.
The original and final functions are both 6156 bytes and 99.82781%.

An alternate uniform-scale inline setter changes registers at +0xb18 and scores
99.67187%; a ground-trail member-pointer control drops four bytes and scores
99.62963%. Neither is retained. Paired sources, assembly and objects are under
`build/cleanup-evidence/expression-assignments/effect-scale-control/`.
Object equality proves non-regression, not retail exactness for this function.

## Evidence

Local generated evidence is under
`build/cleanup-evidence/expression-assignments/`: the initial inventory,
image-qualified retail disassembly/references/strings/match state, original
source controls and strict candidate scores. The census uses real libclang ASTs,
macro provenance, overloaded assignments and fail-closed parsing. The tests added
by this cleanup were removed at the user's request.

All 97 native-derived comparison objects and all three complete EXE files are
byte-identical to the captured baseline. CPE entry points and ordered loaded
memory are also identical. The GAME CPE's raw record segmentation differs;
raw CPE-file equality is not claimed for that image. PSX and OPEN CPE files
remain byte-identical.

Full `kf build`, the cleanliness gate at 46, all 97 type variants with zero enum
literals, Ruff and all 765 remaining local tests (no skips) pass. `nix flake check -L`
passes; its isolated SDK suite runs 765 tests with the expected 144 retail-data
skips. Existing non-exact functions and separate data/relink failures retain
their previous scope. No function was newly banked.
