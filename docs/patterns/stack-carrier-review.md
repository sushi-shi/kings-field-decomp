# Five stack-carrier source verdicts

Reviewed input: `6ca5b71a`. This records the five sites selected by the
external cleanup plan. It is not a census of all automatic storage or
compiler-steering constructs in the repository.

Retail files were hash-verified. Image-qualified address, disassembly/CFG,
incoming/outgoing references, strings and match views were collected for
every target. Whole source bodies, introduction commits, manifest profiles,
adjacent/calling functions and relevant SDK output writes were inspected.

## Current exactness does not establish the declarations

All five existing functions are strict objdiff 100%. The GAME reservations
were added in `71cb0cab`; the OPEN carriers replaced vertex-offset arithmetic
in `a977131b`. The menu's one-shot reset was added in `e73393b8` explicitly
to affect the probe's register choices. These histories establish why the
reconstruction uses the constructs, not why the original program did.

Each ordinary-source control changes one site in an isolated copy. Four
remove the never-read declaration; the menu control uses an ordinary
`for (i = 0; ...; i++)`. No carrier is replaced with another artificial local,
padding, arithmetic expression or forced statement.

Baseline and candidate were compiled from the same short source pathname,
using each unit's existing native compiler/profile and SDK includes. The
complete comparison objects and assembly were retained under ignored build
evidence. This paired comparison distinguishes real instruction changes
from the existing section-relative relocation presentation in `kf try`.
Listing similarity is not a strict match percentage.

| Image / function | Ordinary control | First real difference | Function bytes, baseline/control | Verdict |
| --- | --- | --- | ---: | --- |
| GAME `80018880` `player_update` | Remove `SVECTOR unused_vector` | `+0`: frame allocation 224 → 216 bytes | 6684 / 6684 | Open source reservation; reject non-exact removal |
| GAME `8001e5ec` `render_map_cell` | Remove `MATRIX unused_matrix` | `+0`: frame allocation 120 → 88 bytes | 592 / 592 | Open source reservation; reject non-exact removal |
| OPEN `8001764c` `render_enqueue_tmd` | Remove `u16 unattributed_stack_slot[2]` | `+0`: frame allocation 96 → 88 bytes | 3320 / 3320 | Open source reservation; reject non-exact removal |
| OPEN `80018344` `render_enqueue_unlit_triangles` | Remove the same carrier | `+0`: frame allocation 64 → 56 bytes | 676 / 676 | Open source reservation; reject non-exact removal |
| GAME `80027b7c` `menu_draw_item_detail` | Ordinary index reset in the `for` initializer | `+ec`: `move a0,zero` → `move a1,zero` | 732 / 732 | Open source contour; reject non-exact replacement |

The GAME player/menu units use `probe-gcc257-o2-g8`; the other three use
`probe-gcc257-o2-g0`. Both profiles specify native GCC 2.5.7, `-O2`, ASPSX
1.07 and `-mcpu=r2000`. Manifest comments about other scheduling models do
not override those actual arguments. Exact historical compiler/flags
attribution remains unproved.

Initial controls using long source paths failed before a comparison with a
native debug-name parsing error (and one assembler timeout). Those failed
runs are not mismatch evidence. Short-path controls and the subsequent
paired object builds completed successfully. No tool rule was relaxed.

## Individual stack evidence

### GAME player update

The retail frame is 224 bytes. Outgoing argument stores extend through
`sp + 28`; the first referenced vector begins at `sp + 40`.
The current never-read eight-byte aggregate carries the intervening
`sp + 32..39` reservation. It is neither addressed nor passed to a callee.
Live inputs/outputs include direction at 40, spawn offset at 48, rotation at
56, position at 64, matrix at 80 and the target-distance output at 112.
Removing the carrier moves later locals and saves by eight bytes.

The actual `ApplyMatrix` body writes its output at offsets 0/4/8;
`pitch_yaw_to_forward_vector` and `vector3s_scale_shift12` write the three
halfword components within their vector. These outputs explain live buffers;
they do not require the first unused vector. Nothing in this audit establishes
the original reservation declaration or justifies inventing another buffer.

### GAME map cell

Retail passes `sp + 80` as the position, `sp + 36` as the translation output,
and `sp + 88` as the flag to `RotTrans` at `8001e7cc..8001e7d8`.
The matrix begins at `sp + 16`; its translation starts at member offset 20.
`RotTrans` stores only the three translation words at output offsets 0/4/8
and one flag word. It never writes the VECTOR pad at offset 12.
`MulMatrix0` writes rotation within the first 20 bytes of its output matrix.
Thus these SDK writes stay within the live matrix/flag owners and do not
explain the second matrix reservation at `sp + 48..79`.

### Two OPEN emitters

The TMD emitter uses outgoing stack arguments through `sp + 28` and higher
spill slots, but does not access or pass `sp + 32` as an output buffer.
The unlit emitter has no live local stack references before its saved
registers at `sp + 24`; its `sp + 16` word is not an SDK buffer.
Deleting the four-byte carrier changes each rounded frame by eight bytes,
including save/restore offsets. Frame alignment does not prove the source
contained this two-halfword array.

The earlier
[reconstruction debt review](reconstruction-debt-review.md) records a
compiler experiment explaining how eliminated arithmetic can leave a slot.
That observation does not identify original source. Restoring the old
vertex-offset subtraction solely to recover bytes would repeat the same
source-modeling problem. Both emitters retain their direct vertex lookup.

### GAME menu item detail

The retail glyph loop at `80027c8c..80027ca4` uses `a0` as the index, `a1`
as destination and `a2` as source. The ordinary control swaps the first two
roles, beginning at `80027c68` (`+ec`), while preserving the body size and
the loop's zero/count/copy behavior. The source-level one-shot reset is not
established by the retail loop. A prior probe's reference-weight explanation
is not proof of its original spelling.

## Final verdict and remaining conflict

Every site has an individual verdict: **retain the existing banked function
while keeping its source form unresolved**. The ordinary controls are
non-exact, so no body change is landed. This follows the plan's explicit
instruction to avoid unexplained regressions while recording the conflict
with the rule against source constructs introduced solely for code generation.
Exact bytes do not resolve that conflict. Do not count these five sites as
clean, or substitute another artificial construct to close them.

The C comments point to the source uncertainty and this evidence instead of
presenting a probe mechanism as an original-source finding. Resolving the
declarations/contour still requires independently supported source/compiler
evidence. This review introduces no new banked results or compiler attribution.

## Verification of the retained source

The five tracked C changes are comments only. The full three-image build
passes and all 97 native-derived ELF comparison objects remain byte-identical
to the captured clean baseline. All three linked CPE/EXE pairs remain
byte-identical too. Ruff and diff whitespace checks pass. The existing
885-test gate (no skips) and 97/97 type checks cover the unchanged executable
inputs; they are not evidence that the five disputed source forms are original.
The overall strict count remains 465/471 and the pre-existing data/placement
analysis failures remain open.
