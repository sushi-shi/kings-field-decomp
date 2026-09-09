# Owner and storage cast reduction

At `4938eb81`, fifteen further casts are removed while preserving all production
code, data and relocation targets.
That campaign leaves **527 written casts: 430 pointer and 97 scalar**, down
from 542 / 445 / 97. The measured pointer regression ceiling is now 430.
This report supersedes the stopping decision in
[the preceding review](cast-floor-review.md).
The [master integration](#master-integration) below records the current combined census.

## Function Match Plan

Continue from `6dcf1736` in the isolated `codex/cast-floor` worktree. The
current source has 542 written casts, including 98 sites in families with
worthwhile source improvements. Retain the earlier 112-object and 484-function
baseline, and preserve every banked exact function. This campaign reopens
source work; the earlier stopping decision is historical.

The primary family consists of GAME `render_enqueue_tmd`, OPEN
`render_enqueue_tmd`, `render_enqueue_unlit_triangles` and `opening_run`.
They recover a known complete owner from an interior address. Their shared
owners, field offsets, image-qualified disassembly/CFG, callers, callees,
strings, references, source history and strict matches are the evidence.
The functions are game code; their Sony geometry and GPU calls remain
vendored controls.

Test consistent propagation of the complete owner through each function,
including asset, projected-address, material and ordering-table access. The
earlier direct substitutions changed only the final recovered owner. A real
byte cursor is another supported improvement over an integer address; retain
the prepared byte-offset representation and the actual conversion at a read.
Keep one owner and derive its fields without reverse offset arithmetic.

For both sprite enqueuers, use the existing packed XY members and inspect
the surrounding typed packet accesses. The authentic SDK packet remains
the API boundary. Do not add a second representation merely to hide casts.
The four consecutive GTE output words may form one `long sxy[4]` alongside
the existing four-corner input array. Test that real grouping independently,
then compose it with the existing packed stores. All four SDK output addresses
and the later word values must retain their retail meaning.
The array-only control moves the other scalar output homes. Follow up with
one result record containing the anchor XY, depth cue, flags and four corner
XY words. These are all real outputs of the existing two projection calls;
the input vectors remain separate. Both images must share the same declaration
if the record is retained.
Also check explicit copying of the packed GTE word into the SDK X/Y pair with
the existing `memcpy` boundary. This removes the incompatible lvalue access;
the compiler must retain the same four-byte copy and external call set.

The two OPEN FT3 paths compute the third vertex from the second vertex plus
the difference between their prepared offsets. Test the equivalent direct
lookup from the projected base, eliminating the round trip through `u8 *`.
Inspect that simplification independently and with direct owner access.
Also retain the existing named relative-offset calculation while expressing
its starting address as the projected base plus the second vertex's offset. This
removes just the unnecessary pointer-to-byte conversion without deleting
the real relative-offset locals.
For the large dispatchers, one borrowed owner per primitive may also replace
the repeated identical recoveries without changing any storage declaration.

For GAME registry, projection and morph storage, audit the full existing
retail/corpus evidence and every consumer. Distinguish a proved accessed
prefix from a recovered capacity. A possible prefix refinement must leave
the remaining interval unknown and preserve the complete owner, alignment,
and all actual accesses; shipped maxima alone do not prove original array
declarations. Do not create overlapping objects or guessed complete bounds.
Also test sharing the real borrowed scratch view within its actual lifetime.
The earlier scratch control declared its local at function scope. Test a
block beginning after the two cache-key stores, containing the scratch copy,
prefix save/blend/restore and final publication. All entries to that block
initialize the same borrowed pointer; no earlier path needs it.

Each focused control changes a source-level cause and records complete
instruction and ordered-reference differences from the saved baseline.
Inspect referents, calls, CFG and widths before remaining code-generation
symptoms. Do not use source permutations, artificial locals, volatile, new
assembly, compiler-profile changes or cast-hiding helpers.

Keep supported reductions, then inspect the remaining sites and any newly
exposed shared-model improvement. Finish with a new full cast census,
individual function verdicts, object/relocation checks, modern type checks,
the full repository build and tests, Ruff and whitespace checks. Tighten
the measured pointer regression ceiling if it decreases. Generated probes
and retail data remain uncommitted.

## Results

| Retained source improvement | Casts removed | Evidence and result |
| --- | ---: | --- |
| Type the known GAME registry prefix | 5 | Four-byte pointer stride, complete registration census and reader domains support slots 0..47. The following 48 bytes remain explicitly opaque. |
| Copy packed sprite coordinates into the SDK packet | 8 | Four `memcpy` operations per image copy the real GTE words into adjacent SDK X/Y halfwords. Native compilation retains the exact word loads/stores and introduces no external calls. |
| Use the existing projected base for OPEN FT3 relative addressing | 2 | Both functions retain the named second/third offsets and their real relative calculation. The unnecessary typed-pointer-to-byte-pointer conversion is removed. |
| **Total** | **15** | **Every production function and emitted code/data section is unchanged.** |

The sprite copy expresses an object-representation operation already present
in retail. It avoids accessing two SDK halfwords through an incompatible
`long` lvalue. The supplied libc declaration and compiler builtin are used;
there is no helper, additional representation, artificial local or dummy call.

OPEN's general TMD dispatcher retains its existing `u32` projected address;
the unlit dispatcher uses its existing byte pointer. The retained expression
starts from that address plus `vertex1_offset`, then adds the relative offset.
The fully simplified direct third-vertex lookup fails exactness in both
functions. Converting the general dispatcher's entire base to a byte pointer
also retains the earlier operand-order differences. These address models
remain explicit in source.

### Registry reader and writer bounds

The whole GAME graphics owner and all subsequent fields retain their physical
offsets. Its registry interval starts at owner offset `0x20134`. The first
`0xc0` bytes are now `KfAssetHeader *asset_registry_entries[48]`; the remaining
`0x30` bytes begin at `0x201f4` and remain unknown. The next pointer still starts
at `0x20224`, and the complete owner is still `0x249cc` bytes. A native compiled
layout control checks these independent physical values.

All **19 shipped archive/single-asset registrations** fit the prefix. Each
floor's eighteen effect assets occupy slots 30..47. Map-event archives contain
6/4/2/2/2 assets at slot ten; ordinary and alternate actor archives start at
zero, and weapon/common assets use slots 20/21.

Every registry reader was traced through its caller. Actor selection masks
the descriptor to four bits. Weapon and common effect rendering use 20 and 21.
Effect-model assignments use the finite `KfEffectModelId` values through 17;
the `NONE` and billboard paths do not enter the model registry lookup.
Map-event readers obtain the model byte from 24-byte MIXA definitions, read
up to eight entries and stop at state 255. The five floors load **6/4/2/3/2
events, seventeen total**; each model is present in its floor's archive.
The integration test checks registrations and those serialized readers.

The declaration is a supported accessed prefix, not a claim that the original
registry capacity was 48. The opaque remainder is neither asserted padding
nor asserted extra pointer entries. This refines the shared owner without
inventing a complete 60-entry registry.

### Projection and morph storage

The new read audit visits all **1,911 shipped objects, 117,119 packets and
376,781 vertex-index reads**. Every effective index is less than the object's
vertex count; no object has zero vertices. The maximum index is 663 in B1 MIXB
raw chunk zero, object 123, which declares 664 vertices. The preceding corpus
audit also covers all 827 morph ranges and the maximum 651-vertex animated
copy, including its saved/restored prefix element.

These checks establish actual accesses. They do not recover the complete
projection/morph subdivision or original array capacities. Unlike the registry's
finite effect-model domain, these loops accept resource-supplied vertex counts
without an independent capacity guard. Arrays sized 664/652 would encode
incidental shipped maxima; 1000/1001 would infer declarations from neighbor
distance. Neither is retained as recovered source. The three projection and
three morph casts stay at explicit storage boundaries. Function-scoped and
actual-lifetime-block scratch views both lose the binder's exact match.

### Controlled source alternatives

Thirty-one controls cover the supported normal source forms, including
combinations where a newly retained change could affect a previous result.
Four preserve all loadable sections and are retained: the two sprite word-copy
controls and the two OPEN relative-base controls. The registry refinement was
also built independently across its four affected units, preserving all thirty
functions. The final full build and object audit cover their composition.
Temporary-source paths change `.debug_line` metadata offsets; the four retained
controls preserve every loadable section and ordered relocation target.

All 27 rejected controls preserve the ordered external call sequence and every
unaffected sibling function. Each has its own source diff, instruction diff,
ELF sizes, strict score and CFG clues. The first differences below are observed
instructions, not explanations of optimizer behavior. Exact means 100% only.

| Rejected control | Body bytes before/after | Strict % before/after | First observed difference |
| --- | --- | --- | --- |
| OPEN unlit consistently propagated owner | 676 / 700 | 100 / 93.301770 | Frame 64 to 72 bytes. |
| OPEN TMD consistently propagated owner | 3320 / 3340 | 100 / 98.624100 | Saved depth argument moves from `s0` to `s2`. |
| GAME TMD consistently propagated owner | 3896 / 3848 | 100 / 95.098564 | Frame 88 to 96 bytes. |
| OPEN TMD owner plus byte cursor | 3320 / 3340 | 100 / 98.377106 | Saved depth argument moves from `s0` to `s2`. |
| GAME TMD owner plus byte cursor | 3896 / 3848 | 100 / 94.882960 | Frame 88 to 96 bytes. |
| OPEN direct arena start | 532 / 536 | 100 / 99.203010 | Branch target `0x1cc` to `0x1d0`; added address instruction follows. |
| GAME TMD owner derived within each iteration | 3896 / 4108 | 100 / 92.666320 | Frame 88 to 104 bytes. |
| OPEN TMD owner derived within each iteration | 3320 / 3532 | 100 / 91.704820 | Frame 96 to 104 bytes. |
| GAME sprite XY array | 592 / 604 | 100 / 96.027020 | Other scalar output stack homes move. |
| OPEN sprite XY array | 540 / 552 | 100 / 95.644450 | Other scalar output stack homes move. |
| GAME sprite XY array plus packed members | 592 / 604 | 100 / 96.027020 | Same output-home difference. |
| OPEN sprite XY array plus packed members | 540 / 552 | 100 / 95.644450 | Same output-home difference. |
| GAME sprite output record | 592 / 604 | 100 / 96.114870 | Exit displacement `0x230` to `0x23c`; texture-page store moves before XY stores. |
| OPEN sprite output record | 540 / 552 | 100 / 95.740740 | `lw v0,92(sp)` becomes `lui v0,0x2`; store order changes. |
| GAME sprite output record plus packed members | 592 / 604 | 100 / 96.114870 | Same exit/store-order differences. |
| OPEN sprite output record plus packed members | 540 / 552 | 100 / 95.740740 | Same load/store-order differences. |
| GAME one shared owner recovery per primitive | 3896 / 3900 | 100 / 99.536964 | Exit displacement `0xf08` to `0xf0c`. |
| OPEN one shared owner recovery per primitive | 3320 / 3324 | 100 / 99.456630 | Exit displacement `0xcc8` to `0xccc`. |
| OPEN unlit direct third-vertex lookup | 676 / 676 | 100 / 99.455620 | Frame 64 to 56 bytes. |
| OPEN TMD direct third-vertex lookup | 3320 / 3320 | 100 / 99.965060 | Frame 96 to 88 bytes. |
| OPEN unlit direct third vertex plus direct owner | 676 / 692 | 100 / 97.053250 | Frame 64 to 56 bytes. |
| OPEN TMD direct third vertex plus direct owner | 3320 / 3324 | 100 / 99.421684 | Frame 96 to 88 bytes. |
| GAME scratch view in its actual lifetime block | 932 / 928 | 100 / 97.532190 | Initial record load selects `s3` instead of `s4`. |
| GAME shared owner recovery plus byte cursor | 3896 / 3900 | 100 / 99.321360 | Exit displacement `0xf08` to `0xf0c`. |
| OPEN shared owner recovery plus byte cursor | 3320 / 3324 | 100 / 99.209640 | Exit displacement `0xcc8` to `0xccc`. |
| OPEN unlit retained relative base plus direct owner | 676 / 692 | 100 / 97.597630 | Exit displacement `0x274` to `0x284`. |
| OPEN TMD retained relative base plus direct owner | 3320 / 3324 | 100 / 99.456630 | Exit displacement `0xcc8` to `0xccc`. |

Known CFG successor lists are diagnostic; unresolved switch transfers remain
unresolved. No compiler wall or optimizer mechanism is inferred. The supplied
`STDDEF.H` does not define `offsetof`; adding a local offset macro would only
hide the existing conversions. It is not a further source reduction.

### Final site decisions

| Disposition | Pointer | Scalar | Total |
| --- | ---: | ---: | ---: |
| Accepted conversion boundaries and ordinary C | 371 | 71 | **442** |
| Retained matching/storage constraints | 59 | 26 | **85** |
| Undecided | 0 | 0 | **0** |
| **Current total** | **430** | **97** | **527** |

The 85 constrained sites consist of 79 complete-owner recovery casts, three
projection views and three morph views. Those owner expressions remain
desirable cleanup; none of the supported direct/shared owner forms preserves
the banked functions. Storage cleanup additionally lacks complete subobject
evidence. Every supported alternative identified in this review has a result;
there is no untested candidate queue from this campaign. Further reductions
would require new source/storage evidence or another independently justified
form. The count 430 is a measured regression ceiling, not an irreducible minimum.

The 442 accepted sites include 32 that were previously called parked: 24
ordinary per-case packet views, five explicit caller truncations, two explicit
fade narrowings and one aligned-copy boundary. Their existing C is reasonable;
moving or hiding those conversions is not a worthwhile standalone change.
The retained ledger also removes two previously accepted vertex-base round
trips. It maps all 527 survivors and all fifteen removals to their original
review IDs, with current source locations, types, contexts and individual reasons.

### Verification

All **484 production function listings and strict scores** are identical to
the saved baseline. This preserves **453/471 exact game functions** and all
thirteen exact vendored controls. All **112 complete objects** preserve every
code/data section byte, section extent and all **11,342 ordered relocations**.
No new exact function is claimed or banked.

The changed functions and main unchanged owner control have individual
retail dossiers and final verdicts:

| Image/address | Function | Final verdict |
| --- | --- | --- |
| GAME `0x8001c7f8` | `render_enqueue_tmd` | 100%, unchanged owner control. |
| GAME `0x8001e230` | `render_enqueue_sprite` | 100%, identical instructions/references. |
| GAME `0x800204c0` | `asset_registry_load_tmd_archive` | 100%, identical instructions/references. |
| GAME `0x8002055c` | `asset_registry_set` | 100%, identical instructions/references. |
| GAME `0x8002059c` | `asset_registry_select` | 100%, identical instructions/references. |
| GAME `0x800205d4` | `render_bind_animated_instance` | 100%, identical instructions/references. |
| GAME `0x80034de4` | `map_interaction_dispatch` | Existing 99.202774% partial, identical instructions/references. |
| OPEN `0x8001764c` | `render_enqueue_tmd` | 100%, identical instructions/references. |
| OPEN `0x80018344` | `render_enqueue_unlit_triangles` | 100%, identical instructions/references. |
| OPEN `0x800189a0` | `render_enqueue_sprite` | 100%, identical instructions/references. |

The fresh target-C census and modern scoped-enum check both pass all 112
source/image variants. Ruff and the tightened cleanliness gate pass. The
full build retains the prior data/ownership/relink failures: **42/65 data-owning
units match**, with 23 divergences and zero data artifact failures. Full-build
success is not claimed.

The repository suite passes **771 tests with nine skips**, including the
native registry layout control and complete shipped registration/event-model
check. The inventory expectations account for the newly named prefix and
the separate opaque remainder. Final whitespace checks pass.

Generated evidence lives in `build/cast-owner-reduction/`: the 31 controls and
their verdicts, projected-access bounds, current cast census, surviving/removed
site ledgers, all function/object verdicts, and build/type/lint/test logs.
The preceding `build/cast-floor-close/` contains the immutable object/score
baseline, per-function retail dossiers and complete resource/morph census.
Generated evidence and retail files are not committed.

## Master integration

The integration combines `4938eb81` with master `471b4a05`, preserving the newer
data-owner repairs, contiguous menu module and structured control flow. A fresh
pre-merge master build supplies the integration baseline. All 484 function
listings and strict scores remain identical, including **456/471 exact game
functions** and thirteen exact vendored controls. All **99 current objects**
preserve their emitted code/data bytes, section extents and **11,342 ordered
relocation targets**.

The merged target-C census covers all 99 source/image variants and finds
**533 written casts: 430 pointer and 103 scalar**. Master already contained six
additional scalar casts compared with the earlier campaign; integration removes
the same fifteen pointer casts from its 548-site baseline. The source/header
definition census finds **38 project union definitions**, unchanged by the merge.
The root README records these current totals in a manual reconstruction-debt
checklist. These are inventory totals; the checklist calls for reviewing and
removing avoidable representations, not deleting every valid conversion or union.

The integrated suite passes **777 tests with nine skips**. Ruff, all 99 modern
type-checking variants, the 430-pointer cleanliness gate and whitespace checks
pass. The full build preserves master's existing ownership/relink failures:
**42/62 data-owning units match**, with twenty divergences and zero artifact
failures. Generated baseline/current objects, function verdicts, cast and union
censuses, and verification logs are under `build/cast-integration/` in the
integration worktree.
