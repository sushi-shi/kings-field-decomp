# OPEN display-initialization pointer lifetimes

## Post-increment typed DRAWENV cursor control

A single authentic `DRAWENV *` cursor, initialized after the four definition
calls and advanced by the first `PutDrawEnv` argument, retains the whole first
and then second DRAWENV across the calls. It improves the focused rendered
listing to 84.3%, but keeps all four absolute `dfe` address pairs, retains the
40-byte frame, and anchors the whole object rather than retail's first `dtd`
member. Strict objdiff falls from 92.177960% to **90.855934%**. The cursor is
removed and the canonical edge object is rebuilt back to 92.177960%.

## Function Match Plan: linked SDK-object and member lifetimes

After merging master `97b2184`, a fresh pinned build reports OPEN at 106/108.
OPEN `80016adc display_initialize` remains 472 retail versus 484 probe bytes
and strict 92.177960%.  The six image-qualified views, sole caller, complete
seven-block CFG, eighteen calls, ordered references, adjacent exact unit
controls, SDK declarations, complete graphics owner and source history were
refreshed.  There are no strings or candidate references.  The first real
divergence remains the 48/40-byte frame: retail retains the address of the
first real `DRAWENV.dtd` member in `s0` across the mode branch and draw calls,
whereas the probe discards it and emits four additional absolute address
pairs for `dfe`.

Test linked authentic lifetimes at the observed post-definition point: form
the two existing `DRAWENV *` objects, form a typed pointer to the first
object's actual `dtd` member, and perform the unchanged chained `dtd` write
through that member and the second whole object.  Continue to use the two
whole-object pointers for their genuine SDK fields and calls.  This introduces
no aliasing view, raw offset, volatile carrier, call, field, constant or
ownership change.  Retain it only if the full raw stream improves while all
three exact sibling functions remain exact; otherwise restore the source and
record the allocation result.

The typed member pointer is folded away before allocation.  The probe instead
anchors the whole first DRAWENV only at the mode branch, emits 21 address
pairs, keeps the 40-byte frame, and drops the first `PutDrawEnv` argument from
its delay slot.  Similarity falls to 78.8%; all three sibling listings remain
exact.  The source is restored.  Linking the member and whole-object source
lifetimes therefore does not reproduce retail's retained member base.

## Data-owner and register-qualification controls at 106/108

Two non-overlapping owner hypotheses were compiled as diagnostics. Separate
external DRAWENV and DISPENV arrays keep the 40-byte frame and expand the
initializer because GCC cannot derive the display array from the draw array.
A compact external aggregate containing exactly `DRAWENV[2]` followed by
`DISPENV[2]` recovers the `+162` cross-array derivation, but still keeps the
first base in a caller-saved register and rematerializes all four `dfe`
addresses. Moving the existing typed pointers to cover the aggregate's whole
customization lifetime does not change that outcome.

Qualifying the existing authentic first-DRAWENV pointer with legacy C
`register` is byte-identical to the canonical candidate. All diagnostic source
changes are removed. These results reject both separate data ownership and a
register-qualification explanation for retail's fourth saved register; the
complete graphics runtime owner remains the supported model.

## Cross-image owner and flag controls at 106/108

Pinned GCC 2.5.7 `-da` dumps localize the surviving discrepancy more narrowly
than the final assembly.  Initial RTL contains the two source `DRAWENV *`
pseudos.  The first CSE pass also creates a shared pseudo for
`display_draw_environments[0].dtd`, derives both DRAWENV pointers at `-22` and
`+70`, and rewrites the four later `dfe` stores as `+1` and `+93` from that
base.  Before allocation, equivalence substitution turns those four memory
uses back into absolute constants, so the DTD pseudo dies before the calls and
is not assigned a saved register.  This identifies the responsible compiler
stage without proving a source spelling or an optimizer defect.

Compiling the current function against standalone typed `DRAWENV[2]` and
`DISPENV[2]` symbols is a negative ownership control.  It shortens the
candidate by 24 bytes, preserves absolute later `dfe` stores, and still does
not keep the DTD base through the two `PutDrawEnv` calls.  The complete OPEN
graphics owner remains the better supported model; the temporary declarations
and references are removed.

The GAME retail counterpart provides an independent ownership control: its
initializer also anchors `DRAWENV[0].dtd`, derives the first `DISPENV` at
`+162`, and retains that base through its later fog-state store. This supports
the physical DRAWENV/DISPENV family, but does not by itself prove a source
pointer spelling.

In OPEN, an explicit full-width enabled value emits the canonical candidate
unchanged. Nesting the two DRAWENVs and two DISPENVs in a real typed aggregate
also emits the same code: the four `dfe` stores remain absolute and the frame
remains 40 bytes. A narrower pointer to the complete graphics owner, live only
from the mode branch through the final render-state writes, instead expands the
frame to 64 bytes and introduces additional address construction. All three
controls are removed. The best source therefore remains at strict 92.177960%;
no raw byte alias, overlapping view, volatile carrier, or compiler change is
supported.

Deriving the second typed DRAWENV as `first_draw + 1` also does not retain the
first pointer across the two `PutDrawEnv` calls; GCC rematerializes both later
`dfe` addresses. An explicit diagnostic pointer at the observed DTD member,
including the decoded `+1`, `+93`, and `+162` uses, is optimized back to the
same candidate under the configured profile. The derived pointer and raw view
are removed; the latter is not a source model for the known SDK objects.

GCC 2.5.7 controls with `-fno-force-addr`, `-fno-force-mem`, and
`-fno-cse-follow-jumps` are byte-identical for the target and preserve the
exact siblings. `-fno-expensive-optimizations` is also identical for the
target but regresses one instruction in exact `render_initialize`.
`-fforce-addr` is the only tested switch that recovers retail's 48-byte frame
and `s3` mode allocation, but it anchors the entire graphics runtime rather
than the DTD member and severely changes all three exact siblings. Combining
it with the explicit member diagnostic is worse. All profile controls are
removed; no compiler flag, raw alias, or source change is retained.

## Midpoint lifetime and supplied-profile controls (`ad5c075`)

At strict 92.177960%, the retained retail base begins at the chained `dtd`
write and survives through the later `dfe` clear/submit/restore sequence. A
bounded source trial therefore moved the two existing typed `DRAWENV *`
initializations from after the RGB clears to immediately after the chained
`dtd` assignment. This midpoint is distinct from the already tested
whole-field lifetime: it neither changes a field expression nor introduces a
member pointer. A real focused rebuild produced the identical 484-byte object,
with the same four extra `dfe` address pairs and the same 40-byte frame. The
trial is reverted.

The complete supplied profile matrix was also checked on the unmodified unit.
GCC 2.5.7 without the explicit CPU flag emits the same `display_initialize`
listing as the canonical R2000 profile and preserves the three exact sibling
functions. Disabling scheduling does not improve the target and makes exact
`primitive_buffer_allocate` differ in its counter register. GCC 2.6.0 changes
all four functions, uses a different return delay-slot convention, changes the
target CFG from seven to six blocks, and reaches only a 55.8% listing
similarity for `display_initialize`. Canonical `probe-gcc257-o2-g0` remains the
only exact-sibling-preserving profile. None of these results explains the
retail `s0` base, fourth saved register, or 48-byte frame, so the residue stays
unattributed rather than being assigned to a compiler mechanism.
## Remaining scheduler-model controls

An OPEN-render-init-only `-mcpu=r3000` build is byte-identical to the
configured r2000 candidate: `display_initialize` retains its 40-byte frame
and four absolute DFE pairs, while all three sibling functions remain exact.
Disabling instruction scheduling also leaves the initializer's ownership
residue intact and additionally changes the exact allocator's final global
increment register. Both temporary profiles are removed. Together with the
existing plain-profile result, the available pinned scheduling controls do
not recover the retail DTD-rooted lifetime.

## Split-DTD and complete-owner controls (`82320b9` follow-up)

Splitting the chained DTD assignment around initialization of the existing
typed first-DRAWENV pointer does not retain retail's first-member anchor. The
probe instead anchors the second DTD member and derives the later DRAWENV and
DISPENV arguments backward from it, while preserving the 40-byte frame and
four absolute DFE pairs. A block-local `KfGraphicsRuntimeOpen *` used for the
same complete customization lifetime is also wrong: it reserves 64 bytes and
collapses the independently observed absolute RGB references into base-relative
stores. Both trials are reverted. They rule out two real typed lifetimes; they
do not justify a raw byte alias or a register carrier.

## DTD-member lifetime controls (`d8f448e`)

Retail retains the address of `display_draw_environments[0].dtd` in `s0` only
after the four SetDef calls, then uses offsets 1 and 93 for the two `dfe`
fields. A typed pointer initialized at function entry reproduces the 48-byte
frame, saved `s3` mode and all four base-relative stores, but necessarily
materializes the address before the first mode branch and assigns the remaining
long-lived values to different saved registers. Creating the same real member
pointer immediately before the fourth SetDef call recovers the target frame
without the early address pair, but GCC then constant-folds all four typed
`DRAWENV.dfe` accesses back into absolute references. Both forms remain strict
92.177960% and are reverted.

Using byte indexing from `dtd` can force the observed physical offsets, but it
does not improve strict objdiff and is not an acceptable final model for the
known DRAWENV array. An explicit byte-sized enable value also emits the same
candidate. The existing `probe-gcc257-o2-plain` profile is byte-identical for
this unit. These controls isolate a lifetime/allocation residue; they do not
justify an unused stack object, `register` hint, volatile carrier or raw alias.

### Typed DRAWENV-array owner controls

A single pointer to the authentic `DRAWENV[2]` array, introduced after the four
definition calls, is constant-folded into absolute field accesses and worsens
the candidate. Moving it before the fourth call recovers retail's 48-byte frame
and saved `s3` mode, but retains the array base in `s1`, the value one in `s0`,
and still emits absolute `dfe` pairs.

Recovering a typed `DRAWENV *` from a retained first-`dtd` member and adding a
real second-element pointer recovers the complete four-register save set:
mode maps to `s3` and the second DRAWENV to `s2`, as in retail. The first member
and value remain exchanged in `s1`/`s0`, and GCC still folds all four later
flags absolute. Moving the member anchor after the call drops the fourth saved
value again. All variants are reverted; the remaining opacity cannot be forced
with a volatile or raw cross-object alias.

## Function Match Plan: SDK background-color macro (`262a978`)

OPEN `80016adc display_initialize`, 472 retail/484 probe bytes, remains strict
92.177960%. Hash validation, all six image-qualified views, the sole caller's
full-width mode/result window, all three exact unit siblings, the retail
PutDrawEnv body, source history, shared graphics layout and SDK declarations
were refreshed. The frame is 48 retail/40 probe bytes; both sides retain
eighteen calls, two conditional branches, two internal jumps and one return.
Sixteen retail address pairs are validated; the probe emits four extra pairs
for the dfe writes. There are no strings or candidate outgoing references.
LIBGPU EXT/PRIM/SYS and LIBGTE providers remain vendor boundaries; the
mode-specific initialization policy is game code.

The pinned LIBGPU.H `setRGB0` macro (lines 105-106) is a comma-expression
sequence of the real r0/g0/b0 members, which DRAWENV also declares. Retail
clears those bytes independently and in that order for each environment.
Test replacing only those six ordinary assignments with the two genuine
SDK macro invocations, without moving pointer initialization, fields or calls.
This is an authentic SDK source-form hypothesis, not a new macro or cast.
Compare all instructions and ordered numeric references, preserve the three
exact siblings, and do not bank a non-exact result or alter compiler options.

The two actual SDK macros emit identical complete unit text and ordered
relocation records to the committed source. All three siblings remain exact;
display initialization retains the same 484-byte candidate and four extra
dfe pairs. Restore the original assignments. This rules out the macro's
comma-expression boundary as the explanation, not its possible historical
use. No new OPEN source or score change is retained.

## Paired drawing-area flag control (`4cd8fae`)

The refreshed six views, complete caller/CFG, neighboring controls, SDK
definitions and history retain the 472-retail/484-probe-byte, 92.177960%
snapshot below: eighteen calls, sixteen validated retail address pairs, two
internal jumps, no strings/candidates, and the full-width mode boundary.
Retail retains enabled value one in s1 through the dfe restoration and
stores first-draw then second-draw for both clear and restore operations.
Test `second_draw->dfe = first_draw->dfe = 0` and the corresponding value-one
assignment, preserving those orders and all intervening PutDrawEnv calls.
No constant carrier or new pointer is added. All resolved instructions and
ordered targets are identical: 484 bytes, 117 unequal aligned words, twenty
probe address pairs and a 40-byte frame. The 44/468/132-byte controls stay
raw exact. Restore the separate assignments; no further assignment permutation
is justified by this result.

## Function Match Plan: complete DRAWENV-use lifetime (`6434803`)

OPEN `80016adc` starts at strict 92.177960%, 484 compiled versus 472 retail
bytes. Retail and all six semantic views, the full mode-forwarding caller,
three exact unit controls, SDK types/signatures and provider ledger, shared
graphics owner and source history were refreshed. The full-width `s32 mode`,
18 calls, two internal jumps, 16 validated retail address pairs, no strings
or candidate references, 320x240 dimensions, fog constants and byte flag/RGB
stores retain the contract below. This is game-owned initialization around
separately attributed SDK functions.

The first difference is the missing fourth saved register and 40/48-byte
frame. After the four SDK setup calls, retail retains the first `dtd` address
through initial flag/RGB writes, the mode branch, both PutDrawEnv calls and
the four `dfe` clear/restore writes. The source's typed whole-DRAWENV pointers
start only after the initial flag/RGB writes; the candidate keeps the shared
address in a caller-saved register and emits four extra absolute `dfe` pairs.
Test initializing the existing two typed pointers immediately after the last
setup call and using them for that complete field-write lifetime. Keep the
observed chained `dtd` write order and all subsequent independent store/call
order. Unlike earlier direct-member or field-pointer trials, this describes
one whole SDK-object view from its first customization to its last use. Do
not add carriers, cross-object byte offsets, new views or compiler flags.

The trial remains 484 bytes but loses the retained `dtd` anchor: its first
store becomes absolute and the branch setup introduces a new pair for the
whole first DRAWENV, then derives the second at +92 and DISPENV at +184.
All 18 calls and physical destinations remain, but there are now 21 data
pairs rather than the starting 20, and 110 unequal aligned words. The frame
and four extra `dfe` pairs are not recovered. Restore the original source;
all three unit controls remain raw exact. This result rules out that earlier
whole-object pointer lifetime, not the observed shared retail address.

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
