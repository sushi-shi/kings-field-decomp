# OPEN unlit triangle emission

## Complete read-only pointer qualification (`82320b9` follow-up)

Qualifying the projected byte stream and all three `KfScreenVertex *` locals
as `const` emits the canonical candidate byte for byte. The 56/64-byte frame
and symmetric vertex-1/vertex-2 saved-register roles remain unchanged in both
packet modes. The established mutable types are restored; alias qualification
does not supply the missing source dependency.

## Function Match Plan: enclosing graphics owner (`62f6dc8`)

OPEN `80018344` starts at 676 retail/680 probe bytes and 98.828400% strict.
The complete CFG, sole exact caller, eight calls, one internal jump, signed
depth path, packet modes and SDK-provider negative controls remain unchanged.
Retail already derives projected vertices and the material color from one
address family, then reloads the ordering table at projected base minus 276.
The repository now has a complete `KfGraphicsRuntimeOpen` owner with checked
offsets for both named members. Test recovering that real enclosing object
from the loop's projected-array member and use its ordinary ordering-table
field at the accepted-depth AddPrim site. This is typed owner recovery, not a
raw numeric alias or a cached pointer across SDK calls.

The typed enclosing-owner expression emits retail's `lw -276(projected_base)`.
It removes the extra HI16/LO16 pair and the four-byte body excess: compiled and
retail bodies are both 676 bytes, and strict objdiff rises to **99.455620%**.
All eight calls, the one remaining state address pair, CFG, constants, widths
and numeric referents agree. The remaining differences are the probe's 56-byte
frame versus retail's 64-byte frame and exchanged register identities for the
second and third projected vertices in both triangle modes. Declaration-order
and semantics-preserving symmetric-vertex controls could recover either the
load order or register identities, but not both, and are reverted. No unused
stack object or register carrier is introduced; the function remains partial
and is not banked.

### Triangle-aggregate and use-order controls

A genuine three-pointer local array produces the retail 64-byte total frame,
but materializes its elements at `sp+16..24`, removes three saved-register
lifetimes, and adds pointer reloads throughout both modes. Retail has no such
stack traffic, so the aggregate is rejected rather than retained for its frame
size. A reused `u16` packet-index local and assignments inside the NormalClip
arguments both optimize to already-tested scalar forms. Reversing the two
post-call coordinate stores changes the physical packet write order instead of
the saved-register identities. All variants are reverted; they preserve no new
retail instruction evidence.

## Function Match Plan: accepted-packet table reload (`6a25ae8`)

OPEN `80018344` is 676 retail/680 probe bytes, strict 98.828400%, with
52 unequal aligned words. The six semantic views, complete CFG, sole
552-byte exact entity caller, accessor/allocator, adjacent emitter and
shared packet/runtime types were refreshed. Eight calls, one internal jump,
one retail HI/LO pair, no strings/candidate references, the `u16` object/
`s16` bias ABI, signed depth sum/divide/shift, mode constants and packet
advancement remain as documented below. LIBGPU PRIM and LIBGTE NormalClip
are separately attributed SDK controls, not this game-owned packet policy.

After the frame-size difference, the remaining body discrepancies are two
vertex-register roles and the extra absolute OT pair. Retail `80018590`
reloads the active table only on the accepted-depth path, after all packet
initialization calls, using projected-base minus 276. Test spelling that
actual reload as a block-local `u32 *ordering_table`, then indexing it for
AddPrim. Do not cache the pointer before the loop or across any SDK call,
invent a relative alias, alter vertex lifetimes, or change the profile.
This differs from the previous pre-loop pointer-to-slot trial: it expresses
the observed accepted-packet value dependency at its point of use. Compare
raw words and ordered numeric targets against a freshly rebuilt baseline.

The local reload still emits the extra absolute pair and 680 bytes. It moves
the load ahead of the depth mask and reverses the AddPrim address-add operands,
leaving 53 unequal aligned words rather than 52. Calls and numeric targets
are unchanged; the exact entity caller remains raw exact and the adjacent
map emitter retains its existing 20 unequal words. Revert the local: it does
not explain retail's shared-base access or its evaluation order.

### Shared output-packet follow-up

The adjacent map emitter now recovers one output-pointer dependency using
a mode-discriminated pointer to its existing packet views. Unlit has the
same concrete single-buffer dataflow: both allocation results enter `s0`,
each mode writes its selected layout, and the common AddPrim consumes that
same pointer. Its C currently carries a second `void *primitive` solely to
join two case-local typed pointers. With the restored 680-byte/52-difference
baseline and unchanged evidence above, test one typed pointer to the existing
F3/FT3 alternatives through initialization and the shared tail. Preserve all
allocation sizes, SDK argument types, operations, vertex lifetimes and CFG.
Do not infer that the map result must transfer; compare every raw word and
ordered numeric target before retaining or rejecting the source model.

The single typed pointer emits the identical 680-byte candidate and ordered
relocations, retaining all 52 raw differences. This emitter already shared
the packet pointer through its explicit common tail. Restore its existing
source: the map improvement supplies no new byte evidence for this change.

## Function Match Plan: initial packet dependency (`6434803`)

OPEN `80018344` remains 676 retail/680 compiled bytes, strict 98.769230%,
with 53 unequal aligned words. The six semantic views, complete CFG, sole
entity caller's narrowed object/bias arguments, neighboring emitters, object
accessor and allocator, shared types, SDK declarations/provider evidence and
source history were refreshed. Eight direct calls, one validated internal
jump, one retail state-address pair, no strings/candidates, signed depth
arithmetic and the postdecrement/return delay slots retain the contract below.
The SDK packet/GTE helpers are vendored, not this game-owned traversal.

After the frame discrepancy, the earliest body difference is packet address
formation at `80018394`: retail adds the header-adjusted primitive offset to
the asset value; the probe reverses those operands. The current `payload`
local is initialized before that expression and has no other consumer. The
related map emitter refers directly to the same asset owner when forming its
packet stream and reproduces that operand order. Test removing the one-use
asset temporary and forming the initial byte pointer directly from the
canonical current-asset field, preserving `(primitive_offset + 12)`, count
initialization, loop scope, every call and packet operation. No extra cache,
shared-owner change, pointer permutation or compiler setting is introduced.
Compare the complete body and ordered numeric targets, not merely its score.

The direct owner expression restores exactly the packet-add instruction at
`80018394` without changing any other raw word or relocation. The candidate
remains 680 bytes with 52 unequal aligned words; all eight call targets and
the extra OT pair are unchanged. Keep the simpler expression and this one
observed dependency correction. The exact entity caller and all three
render-initialization controls remain raw exact.

### Shared packed-packet view follow-up

The same refreshed snapshot applies. Both branches access packed XY words,
and FT3 copies UV halfwords; the current C expresses these through field
pointer casts. The shared `KfGpuF3`, `KfGpuFT3` and `KfTmdPrimitive` views
already model these exact widths and offsets, with authentic SDK members
at API boundaries and existing layout controls. Use those views here as in
the related map/general emitters, preserving each load/store and call order.
This tests the supported packet representation, not a register permutation.
Compare against the retained direct-asset result; do not infer exactness
from equivalent layouts alone.

The shared views preserve every instruction and relocation of the direct-
asset candidate while removing twelve packed-field pointer casts. Keep them
as the supported representation, with genuine SDK pointers at SetPoly and
AddPrim boundaries. Strict objdiff is now **98.828400%** (from 98.769230%);
the only byte improvement is the independently corrected packet addition.
Frame size, two vertex-register roles and the extra OT pair remain unresolved.
The function is still partial and is not banked.

The focused strict match and full build retain all 97 OPEN exact functions
and all 13 SDK source controls. The full build still reports the existing
OPEN general-emitter switch-table addend mismatch, 13 GAME data mismatches
and four GAME historical-best deficits; this campaign changes none of them.
Ruff, all 401 repository tests and `git diff --check` pass. The rejected
display and map-cell trials are restored, with their controls recompiled.
No GAME source, shared header, profile or banked baseline is changed.

## Function Match Plan: biased-depth expression (`3ee452a`)

OPEN `80018344`, 676 retail/680 compiled bytes, starts at strict 98.769230%
with 53 unequal aligned words. The six semantic views, full CFG, sole exact
caller `80018ecc`, adjacent map and general-emitter boundaries, accessor,
allocator, shared layouts, source history and pinned SDK declarations/provider
ledger were rechecked. The object argument is masked to 16 bits; the caller
sign-extends biases 1000/10000 and the callee preserves the signed-halfword
conversion. Word count/header, unsigned prepared offsets, signed depths,
eight calls, one validated internal jump, one retail address pair and no
strings/candidate references retain the supported contract. SDK helpers are
vendored; the material/packet traversal is game-owned.

The first difference is frame size (64 retail/56 probe). Later differences
are packet-base operand order, two vertex registers and an extra absolute
ordering-table address pair. The common depth tail at `8001852c..8001857c`
forms one signed sum/divide/shift/bias chain before its first comparison;
the intermediate unbiased depth has no independent consumer. Test combining
the two current assignments into that one expression, preserving signed
widths, checked division, lower-bound guard and mask. The general emitter's
retained combined expression supplies a related source-shape control, not
proof this body must respond identically. Compare raw words and referents
after a real focused compile; do not add locals or alter frame layout.

The combined expression emits the identical 680-byte candidate with the
same 53 unequal raw words, eight call targets and extra OT pair. It is
removed; the exact entity caller remains raw exact. Unlike the general
emitter, this tail already has the retail arithmetic-register dependency.
The unchanged frame/register/address-sharing residue is not an exact match.

OPEN `render_enqueue_unlit_triangles` at `0x80018344` has a complete C body
under `probe-gcc257-o2-g0`, matching **98.828400000%** in strict objdiff.
The probe emits 680 bytes against 676 retail bytes. This is neither banked
exact code nor a proven compiler wall. The per-function evidence and exact
caller control are in `config/evidence/open_semantic_unlit_triangles.tsv`.

The sections below retain the initial evidence and subsequent source experiments;
the final lifetime correction supersedes the earlier separate-base result.

## Supported behavior

The sole caller is `opening_entity_render`: object cases 26 and 27 select
depth biases 1000 and 10000 after projecting the selected object's vertices.
The renderer walks a word-sized primitive count using a postdecrement loop.
It supports TMD modes `0x20` (F3) and `0x24` (FT3), rejecting nonpositive
`NormalClip` results before allocating a GPU packet.

FT3 takes texture coordinates, CLUT and texture page from the prepared TMD
packet, but RGB from `floor_item_state.material.color`. F3 takes packet RGB.
Both paths copy projected coordinates as packed words; FT3 also copies each
UV pair as a halfword. The source uses authentic SDK `POLY_F3` and `POLY_FT3`
types, with pinned layout checks and an intentionally wrong-size negative
control in `tests/test_open_unlit_triangle_inventory.py`.

The shared tail sums signed halfword depths, performs checked signed division
by three, shifts right by two, and adds the signed halfword bias. It enqueues
only depths at least five, indexing the ordering table with `depth & 0x3fff`.
It has neither an upper-depth rejection nor an allocation-null check. After
consuming the four-byte header, the next packet advances by
`(header >> 6) & 0x3fc`, including skipped modes and rejected triangles.

There are eight direct calls, all accounted for. NormalClip and the LIBGPU
packet helpers have separate SDK attribution; the surrounding TMD/material
policy is game-owned. This is not GAME's `render_enqueue_model`: that renderer
has additional shading and mode behavior. No lighting or fog calls belong
in this OPEN body.

## Shared-base ownership remains unresolved

Retail materializes the current-asset slot at `0x80069b60`, then adds 32 to
obtain the projected-vertex base. That same base also addresses:

| Relative address | Retail identity |
| --- | --- |
| `0x80069b80` | Projected vertices |
| `+16044..16046` | Material RGB at `0x8006da2c..0x8006da2e` |
| `-276` | Ordering-table pointer at `0x80069a6c` |

These physical relationships support investigating a wider original graphics
object, but do not establish its complete extent, declaration, or original TU.
The retained source keeps the existing supported typed owners. It does not
invent out-of-bounds aliases to force the shared-base instruction sequence.

Consequently, the source object has 17 text relocations versus 11 in the target:
three additional absolute HI16/LO16 pairs materialize the projected vertices,
material, and ordering-table owners independently. The target has eight calls,
one internal jump, and one HI16/LO16 pair. Base-relative accesses do not justify
inventing extra retail relocation entries.

## First comparison and retained differences

Both versions use a 64-byte frame. The first divergence is the incoming bias:
retail retains it in a saved register, while the probe stores a halfword on
the stack and later reloads/sign-extends it. The independently materialized
global bases also change saved-register use and instruction order; vertex
pointer registers and commutative addition operand order differ.

Calls, packet dispatch, field widths, depth policy, and numeric referents are
accounted for, but address formation is not yet equivalent. Classification
therefore starts with unresolved wider data ownership/shared-base formation,
followed by unattributed register/order symptoms. It is not a demonstrated
compiler mechanism, and no permutation experiments were used.

## Cast-only controls

The new renderer requires 34 pointer casts for byte-offset vertex indices and
packed GPU/TMD field accesses. Thirty redundant casts were removed from five
existing GAME render consumers whose arguments already had the exact SDK
`MATRIX *` or `SVECTOR *` type. All three reconstructed objects were byte-identical
before and after this mechanical cleanup at parent `24d4c7a`; hashes and
unchanged scores are recorded in `render_typed_matrix_cast_cleanup.tsv`.

That comparison left 881 pointer casts, below the existing limit of 882,
without changing the gate or hiding casts in helper macros. The controls are
not new GAME matching progress.

## Master follow-up: loop-invariant signed bias

Pre-edit at `2560194`: OPEN `80018344`/`0x2a4` remains 93.455620% strict,
692 probe bytes versus 676 retail, with a 64-byte frame in both. All six
OPEN queries, the complete body/CFG, sole call at `80019094`, neighboring
general/map/sprite emitters, accessor, allocator, projection routine, source
history, shared types and pinned LIBGPU PRIM/LIBGTE SMP provider symbols and
declarations were rechecked. The eight direct calls, one internal jump, one
retail HI16/LO16 pair, no strings, word count/header, halfword indices, mode
constants and game-owned policy above remain the evidence snapshot.

The related map emitter (`800185e8`/`0x3b8`, 97.449580%) was also inspected
through all six queries, its full body, caller `80018d6c`, source/history and
SDK interfaces. Its 18 calls, two pairs, one internal jump, 80-byte frame,
signed depth rules and current-asset reload remain unchanged. Unlike the
general TMD trial, both current emitters already implement the retail
in-place packet cursor without redundant cursor spills. Do not transfer
that trial merely because the packet formats agree.

The focused unlit hypothesis is instead the real signed bias value used by
every accepted triangle: retail sign-extends it at `800183a8..800183ac`
before the first header load, then adds it at `80018578` without another
conversion. The current object saves the narrow argument at sp+16, reloads
it and sign-extends it in the loop's depth tail. Test an explicit `s32`
loop-invariant bias while preserving the `s16` ABI. This does not change
the bias range or introduce a dummy value; the widened value directly feeds
every biased depth. Compare from the first divergence, with the separate
graphics owners and all calls, guards and packet operations unchanged.

The widened-bias trial scored 92.266270% after a real object rebuild. It
recovered the initial `move s0,a1`, but emitted the sign extension before
the zero-count test and stored the widened value at sp+16. The loop still
reloaded that slot. Total code stayed 692 bytes; the extra independent
graphics bases and their different referent expressions were not resolved.
This did not recover retail's saved, loop-invariant bias. The trial was
reverted; no source or baseline change is retained from it.

## Complete-owner ordering-table follow-up plan

At `4be4995`, OPEN `80018344` is 93.071010% strict, 688 compiled versus
676 retail bytes. The six queries, full CFG, sole entity caller, adjacent
map emitter and preceding TMD tail, accessor/allocator bodies, source history,
shared owner and SDK headers/provider evidence were refreshed. The unsigned
halfword object index, signed halfword bias, word primitive count/header,
eight calls, one internal jump, one retail address pair and no strings or
candidate outgoing references retain the contract above. Both frames are
64 bytes. NormalClip and LIBGPU PRIM helpers remain separately vendored.

The complete owner now recovers current-asset/projected/material address
relationships, but the compiled AddPrim tail still adds an absolute pair for
the ordering-table field. Retail loads the pointer at `80018590` from
projected-base minus 276. Test a real `u32 **ordering_table_slot` pointing at
the named owner field, initialized alongside the projected-vertex pointer
and dereferenced at each accepted triangle. This retains the observed table
reload across SDK calls, unlike caching its value, and does not use a
cross-object cast. The exact scene-1 fade routine already uses this typed
slot form. Keep all packet operations and the signed bias unchanged; compare
raw instructions and ordered referents before considering any wider use.

The slot expression emits exactly the existing 688-byte candidate and its
relocations. The extra OT pair, material-address register and narrow-bias
spill remain. The trial is removed rather than propagated to other emitters.

## Projected-array reference lifetime

The independently inspected loader now reaches raw exact by using direct
count-member accesses, allowing its reset and loop addresses to follow their
retail lifetimes. The renderer has a related concrete discrepancy:
`800183a4` initializes the projected-array address only after the nonempty
count test. Its current C initializes an explicit byte pointer before that
test; the probe likewise computes that address early and retains a separate
material address inside the loop.

Test direct references to the complete owner's projected array at each
vertex-index use, removing the eager local pointer. Prepared halfword indices
remain byte offsets and all six effective addresses are unchanged. This
defers the references to the observed nonempty path without adding guards,
casts across owner boundaries, or a cached ordering-table value. All packet
operations, material reads, bias and SDK calls remain unchanged.

Direct array references recover the projected-address initialization after
the nonempty test and the material's +16044..16046 offsets from that same
base. They eliminate the bias stack spill as a consequence; the body is now
676 bytes, but its frame is 56 rather than retail's 64, the bias is still
sign-extended in the loop tail, and the OT pair remains. Keep this as an
exploratory base supported by the recovered address lifetime, not by score.

Retail sign-extends the bias once on the nonempty path at `800183a8/ac`,
then uses the full word at the common depth tail. Test an `s32 bias` value
inside the loop scope, derived from the unchanged `s16` parameter and used
by the actual depth addition. Unlike the earlier pre-owner trial, this
starts after recovering the shared projected/material address and expresses
the conversion on the observed nonempty path rather than at function entry.

The scoped signed value recovers the saved incoming bias and its once-per-
nonempty-path conversion, leaving 56 differing aligned words and 680 bytes.
Before retaining six repeated byte-array casts, check an equivalent scoped
projected-array pointer initialized at the top of that same loop block,
before the bias conversion, matching retail's nonempty-path order. This
keeps the lifetime correction while using one shared byte-offset view.

### Kept result

The loop-scoped projected pointer preserves the shared material offsets and
restores retail's projected-address-before-bias-conversion order without
extra casts. Together with the real widened bias, this reaches **98.769230%**
strict, up from 93.071010%, with 680 bytes and 53 differing aligned words.
All eight calls and numeric referents are preserved. The exact entity caller
stays 100%. The remaining first divergence is the 56-byte versus 64-byte
frame; the body also retains the extra OT address pair, a commutative packet
address operand difference and two vertex-pointer register roles. No frame
padding, register permutation or source alias is introduced. Not banked.
