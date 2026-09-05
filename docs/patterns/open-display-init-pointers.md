# OPEN display-initialization pointer lifetimes

## Function Match Plan and pre-edit snapshot

OPEN `80016adc`, `display_initialize(s32 mode)`, is 472 bytes (`1d8`) at
`c5f139a`, with strict objdiff 86.084750%. Before editing, all six matcher
queries, the sole caller's argument setup, both adjacent functions, the
color-preset callee, SDK environment definitions/prototypes, source history,
and earlier render-init evidence were read. There are 18 proven calls,
18 validated outgoing references (16 address pairs and two internal jumps),
no candidate outgoing references, and no strings. Library calls remain
separately attributed providers; the mode-dependent initialization and
OPEN-specific state writes are game policy, not vendored bodies.

The sole caller `opening_run` passes its full-width mode at `80015718/71c`.
Mode `fe` chooses ResetGraph(3), PutDispEnv, and SetDispMask(1); other values
choose ResetGraph(0), two PutDrawEnv calls with drawing-on-display disabled,
restore both flags, and SetDispMask(0). Both branches converge on black
lighting, fog distance 11000 with projection 200, projection shift one, and
render initialization. The four environment setup calls use 320 by 240
buffers with the second framebuffer at Y 240.

Authentic LIBGPU.H types are retained: DRAWENV is 92 bytes with `dtd` at 22
and `dfe` at 23; DISPENV is 20 bytes. They are currently two independent
two-element array owners at `80069a70` and `80069b28`. No broader aggregate
is introduced by this experiment, and their physical adjacency does not
alone establish an original declaration boundary.

Retail first holds DRAWENV[0].dtd's address in s0. It then derives the two
whole-object pointers before the mode branch:

- `80016bf8`: a0 = s0 - 22, the first DRAWENV.
- `80016c04`: s2 = s0 + 70, the second DRAWENV, in the branch delay slot.
- `80016c20/628`: clear dfe at s0 + 1 and +93.
- `80016c24/62c`: submit the two pointers to PutDrawEnv.
- `80016c34/638`: restore those same byte flags.

The pre-edit C spells each use as an independent global expression; the
probe emits additional absolute pairs for the four dfe stores and delays
forming the two GPU arguments. The first hypothesis is two ordinary typed
DRAWENV pointers prepared before the branch and used for these operations.
They correspond to observed object addresses, not fabricated carriers.

A separate hypothesis concerns framebuffer-height initialization. Retail
sets the lower-buffer Y constant in the first mode-branch delay slot, but
sets height only after ResetGraph, in InitGeom's call delay slot. The source
initializes both at entry. Test the later height assignment separately from
the pointer change; do not run declaration permutations or change flags.

Retail's frame is 48 bytes with ra/s3/s2/s1/s0 saves; the current probe uses
40 with ra/s2/s1/s0. It also forms PutDispEnv's pointer absolutely, whereas
retail derives that other owner from s0+162. This cross-owner expression
remains unresolved; no out-of-bounds cast or overlapping global is justified.
The three exact neighboring functions in `open.render_init` must stay exact.

## Focused results

The first typed-pointer change reaches **87.796610%**. It recovers the two
object-address preparations before the branch and a retained second pointer
for PutDrawEnv. It does not recover the dfe stores' shared base: the probe
still uses four absolute address pairs for those stores. The remaining
cross-owner PutDispEnv address also stays absolute. These are not hidden by
renaming symbols or adding fake retail relocations.

Moving only the height assignment past ResetGraph then reaches
**89.203390%**. It puts `li s0,240` in the InitGeom delay slot, matching retail,
and removes the extra entry initialization instruction. Both changes are
retained. The compiled body is 496 bytes versus retail's 472, and its frame
still lacks retail's extra saved register (40 versus 48 bytes). The historical
compiler remains unproved; these are source hypotheses supported by the
observed address/control-flow lifetimes, not a claim of unique source spelling.

All 18 ordered calls agree. The 16 retail address-pair targets occur in the
compiled object, with five additional pairs: the DISPENV base and two uses
each of DRAWENV.dfe at `80069a87` and `80069ae3`. Those destinations are correct;
the problem is their absolute expression versus retail's base-relative form.
The unmodified preceding and following functions remain exact:

| OPEN function | Extent | Control and verdict |
| --- | ---: | --- |
| `800168dc` lighting_set_active_color_matrix | 44 | one SetColorMatrix call, five-entry MATRIX owner; 100% |
| `80016908` render_initialize | 468 | RTBL load, buffer split, four rotations/products and texture setup; 100% |
| `80016cb4` primitive_buffer_allocate | 132 | u16 advance, overflow loop, counter and return; 100% |

`tests/test_open_display_init.py` validates all 36 raw relocation rows and
constant-propagates both branches of the retail and compiled initializer.
It follows MIPS control-transfer delay slots, admits only this body's small
instruction set, treats call-clobbered registers as unavailable, and never
invokes an external callee. Modes 0, 1, 2, fe, 100fe, and -2 produce identical
SDK argument and ordered global-write traces, with frame/return restoration
checked separately. Mutating the fog projection or the dfe store offset is
detected. This is a bounded static comparison, not game execution or a proof
of byte exactness; SDK effects and timing are outside its scope.

The focused real compile/match, subsequent full `kf build`, OPEN strict check,
Ruff, `git diff --check`, and all 382 repository tests pass (no workspace
skips). `nix flake check -L` also passes; its isolated run skips 42 controls
requiring local retail or generated artifacts. All 353 exact game functions,
13 vendor-source controls, and 63 data-owning units retain their exact status.
Only the three exact neighbors above are selectively re-banked; the partial
display initializer's baseline is unchanged. The four pre-existing GAME
historical-best deficits listed in `open-format-display-sdk.md` remain the
only all-image strict-check failures. OPEN remains 91 exact / 108 eligible,
with 17 partials and zero unstarted functions.

## Complete-owner follow-up plan

At `0d62ce8`, the canonical complete graphics owner has recovered the
cross-field DISPENV address. OPEN `80016adc` remains 472 retail bytes versus
484 compiled, at strict 92.177960%. The six semantic queries, complete sole
caller, three neighboring/control bodies, earlier pointer experiment, and
LIBGPU provider evidence were rechecked. The signature, 18 calls, two internal
jumps, 16 retail address pairs, constants and SDK field widths are unchanged;
there are no strings or candidate outgoing references. The three neighboring
functions remain strict 100%.

The four remaining extra absolute pairs are the two `dfe` flags before and
after PutDrawEnv. Retail retains the common address rooted at the first
`dtd` member and accesses these flags at +1 and +93. The source currently
expresses these stores through the two call-argument pointers. Test those
four accesses as direct members of the canonical graphics owner, keeping
the existing typed pointers for the two calls. This checks whether consistent
complete-owner field expressions recover the observed shared address; it
does not introduce a byte pointer spanning SDK objects or alter DRAWENV.

Direct member accesses compile identically to the kept source: 484 bytes and
the same four extra pairs. A second bounded hypothesis uses pointers to the
two actual `u_char dfe` members, retained across their clear/submit/restore
sequence. Retail's address lifetime spans both PutDrawEnv calls; each pointer
stays within its named SDK member and neither introduces an offset cast.

Both variants emit identical instructions and relocations to the starting
source; neither recovers a new match. Both are reverted. The three controls
remain raw exact. Display initialization stays at 92.177960%, with its four
extra flag-address pairs and 40-byte versus 48-byte frame unattributed.
