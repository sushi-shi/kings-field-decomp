# Array enum audit

## Scope and Function Match Plan

Audit every project array declaration (global, extern, static, local, field,
parameter and typedef), its element types, aggregate initializers and member/use
chains. Use Clang's strict MIPS C++20 AST for all 111 source files and all project
headers. Layout-assertion typedef arrays and SDK/serialized transport retain
explicit dispositions rather than silently disappearing from the census.
An array index domain does not imply that its element values share that enum.
Named numerical constants, including `KF_FIXED12_ONE`, remain quantities.

The initial source state is `ce31bf2c`. The isolated branch is
`fix/array-enum-domains`. Retail images were verified with `kf init`; all 484
owned function dossiers were refreshed with explicit image identity, complete
retail disassembly/CFG, callers, callees, strings, references, match state and
source history. All 116 current objects were preserved before changes.
These are local products under `build/array-enum-audit/`.

### A1: overlay argument slots

PSX `main` (`0x80010028`, 0xd0 bytes) owns `s32 entry_args[2]`. It initializes
slot 1 to INTRO, copies slot 1 to slot 0 each iteration, and passes the same
address through the authentic SDK `Exec(..., char **)` transport to OPEN and
GAME. OPEN `main` (`0x80013758`, 0x6c bytes) reads slot 0 as `KfOpenMode`;
GAME `main` (`0x8001428c`, 0x88 bytes) writes slot 1 from `KfGameExitCode`.
All three are currently 100% exact. They retain their existing startup and SDK
calls, including the compiler-inserted `__main` hooks where present; no vendored
body is reconstructed as new game progress.

Replace the mixed integer array with a shared eight-byte argument record:
request at offset 0 uses the existing `KfOpenMode`, result at offset 4 uses the
existing `KfGameExitCode`. Propagate its pointer through both overlay entry
signatures, leaving the SDK Exec cast at the external ABI boundary. The loader's
result-to-request copy explicitly translates the two existing domains. Do not
invent a new duplicated enum or alter the unconditional overlay order.

After the focused build, compare raw allocated section bytes, sizes, alignment
and ordered relocations with the preserved objects, then verify the affected
functions against retail. Preserve every banked function. Finish the complete
array census and per-declaration review before claiming coverage. Update the
curated signatures/layouts and any existing assertions affected by them; add no
new tests. Run strict checking, existing tests, lint, whitespace checks, full
`kf build`, and flake checks if tooling changes. Record any remaining unknown
serialized content without pretending its complete semantics were recovered.


### A2: fixed floor-sprite bank selector

The seven rows of `floor_item_sprites` contain numerical UV/geometry fields.
Their selection is a separate domain: `KfFloorItemPlacement.base_sprite_index`
and `KfFloorItem.base_sprite_index` always select that fixed shared bank, and
`0xffff` terminates the serialized placement list. A shipped-resource scan
finds 117 placements: 115 select base 0 with four frames; two select base 4 with
three frames. Every range fits the seven-row bank. Treating this selector as a
generic numeric index in the earlier field review missed its fixed ownership.

Introduce one shared unsigned-halfword `KfFloorItemSpriteId` for both fields,
with encoded row identities 0–6 and the existing END sentinel. Retain the
numerical animation-frame offset and explicitly encode the base at the table
index calculation. Do not invent visual/item names for unidentified artwork.
The descriptor row's UVs, coordinates and dimensions remain numerical.

The affected functions are GAME `item_load_floor_placements` at `0x80020b4c`
(0x1b0 bytes), OPEN homolog at `0x800197e4` (0x1b0), GAME `render_floor_item`
at `0x8001ed90` (0x14c), and OPEN homolog at `0x800190f4` (0x14c). Their snapshots
retain the halfword sentinel tests, source/runtime copy offsets, one numeric
frame addition at rendering, existing geometry/lighting calls and delay slots.
All four are currently exact. Confirm actual addresses/extents against the
snapshots before editing and preserve their original instruction/relocation
sequences after the focused builds.


## Complete coverage and final dispositions

The initial pass found 466 unique declaration sites: 137 variable arrays,
150 array fields and 179 compile-time layout-check typedef arrays. The final
pass finds **467 sites: 136 variable arrays, 150 array fields and 181 layout
checks**. A1 removed one runtime array and added two layout checks. Extern
array declarations and definitions are separately counted; repeated inclusion
of the same declaration is deduplicated. No parameter-array declaration or
runtime array typedef was found. Multidimensional arrays remain one declaration
with every dimension recorded.

Both passes parsed all **111 C files and 55 project headers**, using the flake's
libclang/Python bindings with strict MIPS C++20 compile-command arguments, and
reported **zero parse errors**. Conditional C/modern branches were also searched
for declarations omitted by the selected view; no additional runtime arrays
were hidden there. SDK declarations outside the repository remain vendor-owned.
Pointer-backed table uses and explicit enum decoding were checked separately:
SDK transport, asset offsets, text buffers and the already-typed effect
constructor argument groups account for those boundaries.

The per-array review is
[`enum_array_review.tsv`](../../config/evidence/enum_array_review.tsv), with one
row for every current declaration, dimensions, disposition, use references and
rationale. All **81 explicit brace/string variable initializers** are covered
by [`enum_initializer_review.tsv`](../../config/evidence/enum_initializer_review.tsv).
That second ledger deliberately also includes single aggregate objects, because
an SDK MATRIX variable contains arrays even when the variable itself is not an
array. The raw AST/use dossiers and extraction script are local products at
`build/array-enum-audit/`; the final extraction also refreshes all 1,048 project
field declarations. The existing field ledger includes the two new overlay
members and corrects both floor-sprite selectors.

The review distinguishes these cases:

- Enum elements already enforce their domains in **22 array declarations**:
  item/magic menu codes, configuration choices, save slots, learned flags,
  actor action animations/effect codes, notification IDs, map cells and
  visibility cells. Reusing an enum as an array index does not turn stored
  prices, counts or geometry into that enum.
- Arrays of structured records retain enum members where those members are
  selectors. Numeric animation offsets, probabilities, damage amounts,
  coordinates and dimensions retain their numerical types.
- `available`/`counts` and player/shop stock arrays carry quantities. Equipped
  copies are subtracted, items are added/removed, and Gold Cross shop stock is
  decremented. The notification payload array carries a decimal gold amount;
  its notification IDs already live in a separate enum array.
- Text, glyph rows, filenames, icon pixels, GPU material encodings, SDK sequence
  workspaces, pointer tables and whole-record serialization/copy views have
  explicit non-selector dispositions. Typed component records remain the
  semantic interfaces to packed transport.
- **42 arrays remain explicitly opaque byte spans**. Their field-use dossiers
  establish no runtime element interpretation beyond copying/clearing or layout
  assertions. The audit does not manufacture enums from those unknown bytes.
  Two other formerly generic byte spans have known pointer-registry or geometry
  scratch consumers and are classified accordingly.
- The 181 typedef arrays are compile-time size/offset/alignment checks and have
  no runtime elements. They are listed for complete coverage.

No further source-supported enum gap was found in the reviewed arrays or their
initializers. This is a conclusion about reconstructed source and observed
resource uses, not a claim that all opaque retail storage has been decoded.

## Why the matrix literal stays numerical

The user's example is `floor_item_light_matrix`, an SDK `MATRIX` whose layout is
`short m[3][3]` followed by `long t[3]`. `KF_FIXED12_ONE` is a named numerical
constant, `0x1000` (4096), representing Q12 unity. The matrix coefficients are
0.0 or 1.0 here; the translation components are zero. Other authored matrices
contain values such as 2000, 700 and 4000 and are interpolated/multiplied.
Neither the repeated zeros nor unity identify a categorical state. Naming a
number with an anonymous enum constant does not make its storage an enum domain.

Likewise, the eight `KfSpriteQuad` members are UV origin/span and quad geometry.
The enum-worthy value in that path is the separately stored fixed-bank selector,
now `KfFloorItemSpriteId`. All seven rows retain their original numerical data.

## Final function verdicts and checks

A1's PSX/GAME/OPEN entries and A2's two loaders/two renderers all remain **100%**
against retail. All **116 preserved objects are unchanged** in allocated section
bytes, sizes, alignment and ordered relocation offsets/types/targets, including
four previously preserved vendored data controls verified against the current
objects. The complete 484-function verdict file is local at
`build/array-enum-audit/final-verdicts.tsv`. Exact counts remain **460/484**,
including 13 vendored functions, or **447/471 game functions**; no new match or
banked progress is claimed.

Strict checking passes **112/112 source/image variants**. The existing suite
passes **714 tests, with 9 skipped and 9,173 subtests passed**. Lint and whitespace
checks pass. The full `kf build` was run after the focused builds; it retains
the pre-existing ownership/data verification failures in all three images and
GAME/OPEN relink/placement failures. Those differences were already present in
the unchanged baseline objects. `nix flake check -L` also passes. No new tests
were added.
