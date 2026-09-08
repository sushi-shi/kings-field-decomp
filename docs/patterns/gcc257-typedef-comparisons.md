# GCC 2.5.7 typedef identity in post-decrement comparisons

These are observations of the pinned probe, not historical compiler
attribution. Equal width and signedness do not guarantee identical frontend
folding when a named typedef is involved. Do not treat this as permission to
change a binary-boundary type or strip aliases across the repository.

## Real-function discriminator

OPEN `80014e28 opening_ending_scroll_run` has an isolated source control that
submits `(sequence_volume - 1) / 3` to `SsSetMVol`, then uses
`if (sequence_volume-- == 1)` for the existing transition. The local does not
escape; this submits the same new volume and updates state before every
subsequent consumer. The alternative update timing is a diagnostic, not kept
source. Its full plan precedes compilation in the generated campaign record.

Only the declaration of `sequence_volume` changes between these three builds:

| Declaration | Strict score | Bytes | Unequal aligned words |
| --- | ---: | ---: | ---: |
| `s32 sequence_volume` | 98.765434% | 1956 | 199 |
| `int sequence_volume` | 99.876540% | 1944 | 11 |
| `long sequence_volume` | 99.876540% | 1944 | 11 |

The project defines `s32` as `typedef signed long s32`. Bare long and int
have identical complete resolved instruction/call/reference dictionaries for
the unit. Their eleven differing words are the kept source's seven dispatch
words plus +460,+464,+480,+490 in the volume path. This control therefore
distinguishes the named typedef from its underlying long type; calling the
result simply an int-versus-long difference would be misleading. It does
not retroactively prove the cause of every earlier countdown experiment.

All three builds have normal/instrumented whole-ELF parity from the same
source path, forty ordered calls, 31 ordered referents, six raw-exact siblings
and retail's 264-byte frame with all ten saved-register homes. Whole-ELF
equality is not claimed between different sources, whose debug metadata can
differ. Evidence lives under `build/open-missed-evidence/volume-postcall*`.

## Corresponding frontend rules

The pinned source contains a direct explanation for the distinction:

- `c-decl.c` at 1879..1893 uses `build_type_copy` for a source typedef and
  assigns that copy its typedef name.
- `c-typeck.c:common_type` prefers the canonical long type for equal-precision
  long/int operands; `shorten_compare` converts both operands to the selected
  common type.
- `fold-const.c:fold` strips `STRIP_TYPE_NOPS` for comparisons. In `tree.h`,
  that macro requires exact `TREE_TYPE` identity, not merely equal modes.
- The comparison rewrite at `fold-const.c:4204` recognizes a bare
  `POSTDECREMENT_EXPR`, changes it to pre-decrement and subtracts the
  increment from the comparison constant. An intervening conversion has a
  different tree code and does not satisfy that test.

The source rules and three real-function controls support a typedef-induced
conversion preventing this early fold. No compiler option was changed and
no compiler taxonomy or historical attribution follows from this finding.

## Current OPEN source control

A separate bounded control replaces scalar aliases with their corresponding
bare signed/unsigned C types inside each current unmatched function. In the
ending this includes the four signed-short state declarations and explicit
scalar casts. In display setup it includes the full-word mode parameter and
two word locals. SDK structures, shared field definitions, pointers, layouts
and all behavior remain unchanged. The display signature remains compatible
with its existing typedef declaration; its caller forwards the full word.

Both complete resolved unit bodies equal the kept objects. Ending remains
99.917694%, 1944 bytes and seven differing words. Display remains 92.177960%,
484 bytes with its wrong 40-byte frame and four additional absolute DFE
references. Native/traced full-ELF parity and every exact sibling are
preserved. Thus the demonstrated typedef fold does not by itself explain
the two current mismatches; neither type-only control is retained. Generated
plans and audits are under `build/open-missed-evidence/canonical-scalar*`.
