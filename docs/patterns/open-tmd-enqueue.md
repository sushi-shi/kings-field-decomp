# OPEN general TMD polygon emission

## Shared AddPrim owner controls (`d8f448e`)

The enclosing-owner expression that removes the unlit emitter's extra OT pair
does not transfer directly to this twelve-mode emitter. Repeating the typed
expression in every accepted-depth arm increases register pressure, spills the
signed bias and grows the body. A single loop-local pointer to that same real
ordering-table member preserves the CFG but exchanges the persistent projected
and normal bases and still emits the absolute OT pair. Expressing all twelve
accepted paths as explicit gotos to one source-level AddPrim block collapses
retail's 80-block switch to 52 compiled blocks and is structurally false.

A distinct loop-local `KfGraphicsRuntimeOpen *` recovered from the projected
member has the same negative result: it exchanges the normal/projected saved
registers, retains the probe's 88-byte frame against retail's 96, and GCC still
constant-folds the final table load to an absolute HI16/LO16 pair. Thus the
successful block-local unlit owner expression does not transfer merely by
naming the enclosing owner once for this twelve-mode loop.

All three trials are reverted. Retail's shared machine-code tail is therefore
an optimizer merge of case-local packet paths, not evidence for a common C
label. Recovering its `lw -276(projected_base)` must preserve those twelve
case-local lifetimes and cannot be inferred solely from the successful two-mode
unlit owner expression.

## Function Match Plan: one-packet insertion consumer (`5784c14`)

OPEN `8001764c` remains 3320 retail/3324 probe bytes, strict 99.171080%,
with 125 unequal aligned words. The six semantic views, full CFG, exact
entity caller, neighboring emitters, SDK packet layouts/provider evidence,
history and prior insertion/pointer controls were refreshed. The u16 object,
s16 bias, 57 proven calls, eleven validated address pairs and internal jumps,
29 candidate incoming switch rows, no strings, and 96/88-byte frames retain
the contract below.

All twelve allocation paths use one output pointer and one insertion
consumer in retail. The older insertion-only trial added a second packet
carrier and an extra move; the later one-pointer/repeated-insertion control
was byte-identical. Test these independently supported source facts together:
one loop-local pointer to the existing eight GPU variants, each mode's actual
allocation and authentic SDK member accesses, per-mode bias/depth rejection,
and one AddPrim after the switch using that same pointer. Do not move the
depth guard out of the cases, create an automatic union packet or add a
second carrier. Rejected and unsupported packets still skip insertion.

The combined form avoids the older carrier trial's extra move, but emits
exactly the current 3324-byte candidate. All resolved words, 57 ordered calls
and numeric data targets are identical; 125 unequal aligned words remain.
It is not adopted. Retained compiler output also already expresses the OT
load as `lw $2,open_graphics_runtime+131108`, not retail's projected-base
relative access. These controls supply no new exact result, profile change,
relocation correction or evidence for a compiler mechanism.

Focused fresh baseline/trial comparisons also preserve the exact entity
caller. The full build keeps all 97 OPEN exact functions and thirteen SDK
controls, failing only the existing OPEN default-table addend, thirteen GAME
data mismatches and four GAME historical-best deficits. Ruff, all 401 existing
tests (17.696 seconds), and `git diff --check` pass. The accompanying packed
floor-item control is also rejected; no C, header, profile, test, relocation,
GAME input or banked baseline change is retained.

## Function Match Plan: initial asset dependencies (`8b1e86d`)

OPEN `8001764c` starts at strict 99.146990%, 3320 retail/3324 probe bytes
and 126 unequal aligned words. All six semantic views, full CFG, both
signed-bias calls in the exact 552-byte entity caller, adjacent projection/
unlit boundaries, accessor/allocator, source history and shared SDK packet
layouts were refreshed. The 57 direct calls, eleven validated internal jumps,
eleven retail address pairs, no strings, 29 candidate table rows, halfword
object/bias ABI and signed depth/fog operations retain the contract below.
LIBGPU PRIM and LIBGTE SMP providers are SDK controls, not this game-owned
traversal. Retail frame size remains 96 versus the probe's 88.

Unlike the map emitter, every current TMD output packet already uses the
retail `s0` register. Do not assume its newly shared packet type addresses
this function's first divergence. After the stack operands, initial packet
addition at `800176a0` reverses its operands; the normal addition in the
nonempty-test delay slot also differs. Both derive from a local `payload`
whose only uses are these two addresses. The neighboring map and unlit
emitters use the canonical current-asset member directly and reproduce
retail's offset-plus-asset dependence. Test those two direct expressions
here while preserving their order, `(offset + 12)`, count setup and all loop
operations. There is no intervening call or write to the owner, and no
cached value crosses a call. Compare every word and ordered numeric target;
do not change the output packet model, frames, declarations or profile.

The direct expressions recover the packet-add word and the normal-add
operand order without changing any other instruction or relocation. The
normal destination register still differs. Keep this simpler owner use:
3324 bytes and 125 unequal aligned words, with all 57 calls and numeric
targets unchanged. The exact entity caller and neighboring map/unlit controls
retain their prior bytes.

### Mode-discriminated output-pointer control

From that retained base, separately test the common output-object model
supported by all twelve retail allocation paths: one pointer names the
currently selected SDK packet until the shared AddPrim. The existing source
uses twelve separate case-local variables over eight real formats. Use one
loop-local pointer to a union of the existing packed/SDK variants, preserving
every per-mode constructor, field, allocation size, shading/depth expression,
guard and insertion statement. No larger packet is allocated and no automatic
union object or extra pointer carrier is introduced. Current packet-register
operands already agree, so the map result is not evidence of a likely match;
retain this control only if it supplies additional raw evidence for the
shared source model without changing the observed call/CFG topology.

The union-pointer control is byte- and relocation-identical to the retained
direct-asset source: 3324 bytes and 125 unequal aligned words. Remove it;
this body already shares the output pointer without that additional type.

### Complete graphics-owner pointer control

The remaining extra OT pair addresses a real field of the same proved
graphics runtime as the current asset and projected vertices. Retail derives
all three from one address chain, whereas the probe rematerializes the OT
field at the insertion tail. Test one typed pointer to the existing complete
runtime owner, initialized alongside the first actual field references and
used for current-asset, projected-array and OT accesses. Keep every reload
at its current semantic point, especially the OT value after SDK calls.
This introduces no cross-object alias, fabricated extent, cached OT value,
new field or changed lifetime of the packet/normal/vertex values. Compare
against the retained direct-asset source; do not propagate an unhelpful form.

The explicit owner pointer grows the candidate to 3340 bytes. It materializes
different owner anchors (`80069a48` and `80049a48`), replaces the observed
current-asset/projected-base chain with extra address construction and moves
the header spill. Although the frame becomes 96 bytes and all 57 calls remain,
the OT pair is still absolute and 811 aligned words differ after the shifts.
Reject this form and rebuild the direct-member source; the frame size alone
does not justify its extra instructions or changed referent expressions.

The restored direct-member source is byte-identical to its focused trial.
Strict objdiff records **99.146990% -> 99.171080%**, still 3324 bytes with
125 unequal aligned words. Its seventeen default switch addends remain
`.text+0xca0` versus retail `.text+0xc9c`; all twelve case addends agree.
Only the two initial address expressions are retained, not either pointer
control. This is partial progress and is not banked.

Focused recompilation, strict matching and full `kf build` preserve all
97 OPEN exact functions and all 13 SDK controls. The full check still reports
the existing OPEN default-table addend, thirteen GAME data mismatches and
four GAME historical-best deficits. Ruff, all 401 existing tests (17.550
seconds) and `git diff --check` pass. No GAME source, shared header, test,
toolchain profile, relocation or banked baseline changes are retained.

## Function Match Plan: shared acceptance path (`d1d9562`)

OPEN `8001764c`, 3320 retail bytes, starts at strict 99.044580% with a
3324-byte candidate under `probe-gcc257-o2-g0`. All six semantic queries,
the full 991-line retail CFG/disassembly, both signed-bias caller sequences,
adjacent projection/unlit boundaries, source history, complete C and shared
SDK packet/header layouts were rechecked. The current evidence remains 57
direct calls, eleven validated internal jumps and eleven retail address pairs;
the table's 29 incoming pointer rows remain candidates. No strings occur.
The eight-argument GTE color calls retain their real SDK stack arguments,
and SMP/PRIM evidence excludes the helpers, not this game-owned emitter.

Every successfully clipped supported mode reaches the same acceptance block
at `800182c4`: add signed bias, compare against five, put the allocated packet
in the AddPrim argument in the rejection branch's delay slot, mask the index
and load the OT through projected-base minus 276. The current source repeats
that whole policy in twelve cases; its compiled tail still independently
materializes the OT and shifts seventeen default rows by four bytes. Its
shifted depth also lands directly in a0 rather than retail v0. Frame space
and header/body cursor differences remain separate unresolved observations.

Test a single post-switch bias/acceptance/insertion path. Each case keeps its
own depth formula and produces its actual allocated SDK packet pointer for
the common AddPrim use. Rejected clips and unsupported modes still jump to
packet advancement and never use that pointer. Preserve all twelve shading
paths, allocation sizes, packed widths, mode order, loop, owners and compiler
flags. Unlike the earlier map trial, this joins the directly observed common
bias and lower-depth check, not two map-specific upper-depth checks. Compare
raw calls/referents, every switch addend and the first divergence; no stack
padding or forced shared-base alias is permitted.

The common acceptance path collapses several distinct shading/depth tails:
only 53 direct call sites remain versus retail's 57, the body shrinks to
2860 bytes, and the bias is spilled to a halfword while a constant divisor
occupies s8. Although the semantic paths are preserved, this does not explain
the retail call-site topology; discard that join placement. Keep bias and
depth rejection inside each mode and test only a common insertion path.
Each accepted case passes its actual SDK packet and already-biased depth to
AddPrim; rejection still skips insertion. This keeps the earlier per-mode
acceptance boundary while checking the observed single insertion consumer.

The insertion-only form preserves all 57 direct call sites but adds another
packet-pointer move before AddPrim. The extra OT pair remains and the body
grows to 3328 bytes; remove both common-tail trials and restore the original
per-mode insertion source.

The remaining header/body distinction is a separately decoded source-lifetime
question. Earlier in-place trials retained both the advancing byte cursor and
a typed `body` local, causing a cursor spill. Test one advancing byte cursor
without that second local: case polygon pointers derive directly from the
cursor, and the packed union fields are accessed through direct typed views.
The cursor alone advances past the header and later past its payload length,
as retail s1 does. No offsets, packed layouts or shaded-field identities
change; remove this variant if it merely repeats the previous spill result.

The single cursor does recover retail's in-place s1 header load/advance and
end-of-packet update, without introducing a cursor spill. The candidate stays
3324 bytes and preserves all 57 numeric calls and the same address pairs;
aligned word differences fall from 139 to 134. Before retaining repeated
union casts for packed fields, check an equivalent case-local union polygon
view. Each case's existing polygon pointer will name the shared discriminated
body union, using its matching mode member for indices and its packed texture
or color member for SDK accesses. This is a typed readability control on the
now-correct cursor lifetime, not another function-wide body pointer.

The case-local union views emit the same 134-word residue. The real focused
match confirms **99.098790%**, up from 99.044580%; keep the single cursor and
coherent case-local views. This requires twelve actual byte-cursor-to-polygon
casts instead of the previous single function-wide body cast; they are not
hidden behind macros or compensated by unrelated cast edits.

Next test the directly observed depth expression dependency. Retail shifts
the unbiased depth in v0 and adds the signed bias into a0 at `800182c4`;
current C first assigns depth then updates it in place, and the probe shifts
directly into a0. Express each existing signed triangle/quad depth formula
plus bias as one assignment. Keep all division/shift constants, per-mode
guards, packet lifetimes and SDK calls unchanged. This is the same distinction
between a scaled result and an in-place accumulator that has independent
retail evidence in OPEN fog interpolation, not a register permutation.

The combined depth expressions recover all eight differing shift/final-add
words. There are now 126 unequal aligned words, still 3324 bytes, with all
57 numeric calls and the same eleven retail data targets plus the extra OT
pair. Keep this source dependency correction; the frame, normal/projected
register roles, address operand order and extra OT pair remain unattributed.

Current result: **99.171080% strict objdiff**, 3324 compiled bytes versus
3320 retail under `probe-gcc257-o2-g0`. The function and its switch are not
exact and are not banked. The exact entity-render caller remains unchanged.

The final focused compile confirms the combined cursor/depth result: 126
unequal aligned words, down from 139, with the original 57 ordered numeric
call targets and twelve mode call-path sets preserved. The first difference
is still the 88-byte frame versus retail 96. All twelve case addends agree;
the seventeen default rows still point to `.text+0xca0` rather than retail
`.text+0xc9c`. No table row, relocation contract or compiler flag was changed
to compensate for this remaining four-byte body difference.

Verification for this pass: Ruff passes; 400 of 401 repository tests pass in
23.582 seconds. The sole failure is the live cleanliness floor: the supported
case-local typed views put the repository pointer-cast count at 883 versus
its 882 floor. That count is reported, not hidden through macros, unrelated
cleanup, a relaxed test or a raised floor. Full `kf build` still reports the
existing OPEN TMD switch-addend mismatch and unchanged GAME switch-addend/
four historical-best failures. Existing exact functions and SDK controls
remain exact; OPEN remains 95/108. The failed map vertex-scope and map-cell
coordinate trials are documented in their respective pattern notes and are
not retained in source.

## Function Match Plan: nonempty-path projected base (`7e3bb27`)

This pass starts OPEN `0x8001764c` at strict 98.598790%, 3324 compiled bytes
versus 3320 retail, using `probe-gcc257-o2-g0`. The complete graphics owner
now recovers current-asset/projected-base sharing, superseding the historical
separate-owner discussion below. All six semantic views, complete retail
CFG, both caller sequences, adjacent projection/unlit bodies, accessor and
allocator, source history and authentic SDK interfaces were read. There are
57 direct calls, eleven validated internal jumps, eleven validated address
pairs, no strings and one decoded indirect jump through the 29-word switch;
the navigator's 29 incoming table rows remain candidates, not promoted claims.
The checked table value chain and twelve mode-specific call paths are separate
evidence. SMP/PRIM archive controls still exclude the library callees.

The `u16` object index, incoming `s16` bias, unsigned halfword prepared byte
offsets, signed halfword depth/fog values, packed word XY/halfword UV, signed
division traps, clipping and lower-depth guards agree with the reconstructed
semantics. Retail uses a 96-byte frame versus 88 in the candidate; the count
spill is 48 versus 32 and the header spill is 40 in both. The current-asset
address precedes the count load in the candidate, and its projected base is
initialized before the nonempty guard. Retail initializes that base at
`0x800176c0`, immediately after the guard, followed by signed bias widening.
The candidate's extra absolute ordering-table pair shifts all seventeen
default switch addends by four bytes; all twelve case addends already agree.

Test only a loop-local projected-array pointer first, preserving the typed
body-derived next-packet expression and per-mode depth checks. This lifetime
is independently visible here and already useful in the neighboring map and
unlit emitters. Compare raw instructions/numeric referents and strict switch
addends after a focused compile; preserve all banked consumers. Further bias
or cursor experiments require their own observed divergence, not a frame or
register permutation search.

Loop-local vertices recovers the count-before-address order and guarded
projected-base initialization, but leaves 3324 bytes, the extra ordering-table
pair and 139 aligned differences. The remaining cursor discrepancy is still
concrete: retail reads and advances `s1` in place, whereas the candidate reads
the next header through `a1` and creates the body cursor in `s1`. Test one
advancing byte cursor with its typed body view limited to the iteration. The
earlier in-place trial kept the body view function-wide; this trial does not
carry that view across iterations, and composes the now-supported complete
owner and guarded projected-base lifetime. Reject any newly introduced cursor
spill even if it happens to reproduce the frame size.

The loop-local typed-view/in-place-cursor trial still spills the cursor and
grows to 3368 bytes; remove it. The next source-scope check is independently
visible in the depth tail: each case defines and consumes its own depth, yet
the C shares one function-wide temporary. The candidate writes the shifted
depth directly to `a0`; retail writes it to `v0` before adding bias into `a0`.
Test case-local depth variables (as supported in the map emitter), retaining
the existing arithmetic and per-mode checks, without combining lighting paths.

Case-local depth emits the same diff and numeric referents as loop-local
vertices alone; remove the twelve additional declarations. As a separate
type/lifetime check, represent the signed word-sized bias that retail creates
at `0x800176c4..0x800176c8` as a loop-local `s32` initialized from the `s16`
parameter, consumed by all twelve mode-local depth computations. The emitted
conversion already agrees; this trial tests whether spelling the evidenced
widened value explicitly affects the remaining base sharing, not whether a
different ABI or artificial carrier can imitate a register.

Explicit loop-local bias also emits the same diff and numeric referents;
remove it. Retain only the loop-local projected pointer if the strict report
preserves the existing result. No cursor spill, mode-local depth or additional
bias variable is kept from the unsuccessful trials.

The focused strict report confirms 98.598790% -> 99.044580%, so the guarded
projected pointer is retained. Its count-before-address order and nonempty
initialization agree with retail. All 57 ordered numeric call targets and
eleven retail data address targets agree, with one extra absolute OT pair.
The first raw difference remains the 88-byte frame versus retail 96; 139
aligned words differ, including count spill placement, normal/projected-base
register roles, address operand ordering, split header/body cursors and depth
result registers. All twelve case addends agree, but the seventeen default
addends remain `.text+0xca0` versus retail `.text+0xc9c`. Those residues are
unattributed; this is an improved partial, not an exact function or switch.

The shared verification checkpoint is recorded in
[OPEN map polygon emission](open-map-enqueue.md). No new exact function or
bank entry is claimed; all existing exact functions are preserved.

## Historical first reconstruction

The first reconstruction of OPEN `render_enqueue_tmd` at `8001764c` reached
98.802410% strict objdiff under the `probe-gcc257-o2-g0`. Retail is
3320 bytes (`0xcf8`); that probe emitted 3324 bytes (`0xcfc`), including the
return delay slot. It was not exact or banked. Its exact caller,
`opening_entity_render` at `80018ecc`, remained 100% after the identity rename.
Per-function snapshots and verdicts are in `open_semantic_tmd_enqueue.tsv`.

## Function Match Plan and evidence

Before the first edit, the campaign inspected all six OPEN semantic queries,
the complete disassembly/CFG, both calls at `80019050` and `800190d0`, the
preceding projection epilogue, following unlit renderer, allocator behavior,
source history, GAME's behavioral homolog, switch bytes and color xrefs, and
the pinned LIBGTE/LIBGPU archive symbols and headers. The published plan was
to reconstruct each mode, retain signed depth rules and SDK interfaces,
recover the local switch and complete color owner, and compare referents,
calls, CFG and widths before attributing remaining code-generation symptoms.

The sole caller supplies an unsigned halfword object ID and a signed halfword
depth bias. Case 25 projects with the perspective-right variant and passes
zero; ordinary cases project normally and pass the selected signed bias.
The renderer does not project vertices itself. Cases 26/27 use the separate
unlit emitter and are not silently folded into this function.

This is game-owned prepared-TMD traversal and allocation/depth policy. The
called NormalClip/NormalColor functions belong to LIBGTE/SMP, and the packet
constructors, SetSemiTrans and AddPrim belong to LIBGPU/PRIM. Pinned
`psyk list --recursive` identifies those providers and their XDEFs; the
existing vendored ledger retains the known GTE revision skew. No SDK bodies
are reconstructed here. GAME `render_enqueue_tmd` is a behavioral homolog,
not a byte-identical body: it has different graphics ownership and inline
allocation/error handling. A common original file is not established.

## Complete dispatch and packet behavior

The 29-word switch at `800121c0` covers modes `20` through `3c`. It is the
unit's 116-byte RODATA claim, replacing 29 independent pointer identities.
The actual `jr v0` value chain uses the unsigned `(mode - 32) <= 28` bound,
four-byte table indexing, a word load, its load delay, and the jump delay slot.
There are twelve case targets and one default packet-advance target:

| Mode | GPU packet | Lighting |
| --- | --- | --- |
| `20`, `22` | F3 | Face normal, average triangle p2; `22` semi-transparent |
| `24` | FT3 | Face normal, neutral color, average triangle p2 |
| `28`, `2a` | F4 | Face normal, average quad p2; `2a` semi-transparent |
| `2c` | FT4 | Face normal, neutral color, average quad p2 |
| `30` | G3 | NormalColorDpq3 with vertex-0 p2 |
| `32` | G3 | Semi-transparent NormalColorCol3, without depth cue |
| `34` | GT3 | NormalColorDpq3, neutral color and vertex-0 p2 |
| `38` | G4 | NormalColorDpq3 plus the fourth normal, vertex-0 p2 |
| `3a` | G4 | Four separate NormalColorDpq calls, vertex-0 p2; semi-transparent |
| `3c` | GT4 | NormalColorDpq3 plus fourth normal, neutral color and vertex-0 p2 |

Every case rejects `NormalClip <= 0` before allocating. The allocator is
called with the actual SDK packet size, including 52 bytes for GT4, and its
result is used without a NULL check. OPEN's allocator itself loops forever
printing an error on overflow; GAME's different inline return policy is not
copied into this function.

Prepared vertex and normal indices are unsigned halfword **byte offsets**.
Projected XY is copied as one word, and each UV pair as one halfword. CLUT and
texture page come from the packet, not the active sprite material. Untextured
colors come from the packet's RGB/mode prefix. Textured lighting uses one
initialized CVECTOR at `800372f0`, bytes `80 80 80 00`. The nine encoded color
references remain owner+3 (`800372f3`); the former one-byte identity is removed.
The following four bytes at `800372f4` remain separately unclassified.

Triangle depth is `((sz0 + sz1 + sz2) / 3) >> 2`; quad depth is
`(sz0 + sz1 + sz2 + sz3) >> 4`. Both use signed halfword inputs, add the signed
bias, accept depths at least five, and index with `depth & 0x3fff`. There is
no upper-depth guard. The signed division checks and both trap instructions
are retained. All modes, including defaults and rejected faces, consume the
four-byte header and advance by `(header >> 6) & 0x3fc`. The primitive count
is a word-sized postdecrement loop, with the final decrement in a delay slot.

## Shared packed views without replacing SDK APIs

`gpu_packets.h` pairs each of the eight authentic SDK polygon types with a
typed packed view in a same-size union. The packed view exposes `long` XY,
halfword UV pairs, and CVECTOR color lanes. Constructors receive the real
`POLY_*` member; lighting receives actual CVECTOR fields. These views model
the observed word/halfword accesses, not a claim that the original source used
these union declarations. SDK padding fields retain their real positions.

The shared `KfTmdPrimitive` union describes the mode-selected body and its
common color/texture prefixes. Three-vertex modes never access the fourth UV
pair. Existing TMD object and primitive layouts are unchanged.
`tests/fixtures/open_tmd_enqueue_layout.c` checks every paired SDK field,
packet size/alignment, packed widths and TMD prefix offset. A wrong FT3 X1
offset is rejected. No cast-hiding macros or incompatible per-file structs
are introduced; the source-quality ratchet stays below its existing limits.

Release 2.5 LIBGPU.H declares `SetPolyGT(POLY_GT3 *)`, whereas PRIM.OBJ and
retail export `SetPolyGT3`. The shared SDK wrapper now supplies that missing
void-returning declaration; a typed function-pointer fixture checks it.
The retail helper writes only the packet length and code and returns with
the code store in its delay slot. No replacement SDK implementation is added.

## Focused comparisons and remaining work

The first complete source scored 81.412050%. It placed one depth check after
the switch, causing the probe to merge triangle division and shading tails
that retail keeps separate. A mode-local vertex-scope experiment scored
72.243370% and did not recover that topology; it was rejected. Shared vertex
scratch and per-mode depth checks/insertions recover all 57 ordered call
sites and reach 97.272285%, with one emitted AddPrim tail as in retail.

Advancing the next packet from the current typed body removes a redundant
parallel cursor lifetime, reaching 98.802410%. It preserves the same header
and body extents while eliminating repeated stack reloads in the loop. These
are source-level CFG and cursor corrections, not flag or permutation searches.

The first remaining raw difference is the 88-byte compiled frame versus
96 bytes in retail. Both save the same ten registers and preserve the incoming
bias in a saved register. The compiled count/header occupy sp+32/+40 instead
of retail +48/+40. Retail materializes the current-asset slot `80069b60`, adds
32 for projected vertices, then loads the ordering-table pointer at -276 from
that base. The source retains the supported separate owners and introduces
two legitimate absolute HI16/LO16 pairs. This is the same unresolved wider
graphics ownership seen in the unlit and map renderers; no fake aggregate or
out-of-bounds aliases are introduced to force it.

The remaining instruction differences include normal/projected-base register
roles, commutative address operands, some packet/normal argument copies, and
the triangle result register at the depth join. All 57 direct calls, eleven
internal jumps, eleven retail address pairs, 29 table rows and two caller
relocations are reviewed. Tests compare all twelve static call-path sets,
including clip/depth rejection and division traps, between retail and the
actual compiled object. Ordered data targets agree after explicitly accounting
for the two separate global bases. Those controls establish path/referent
agreement, not identical machine CFGs or a historical compiler mechanism.

This completes the unstarted census: OPEN has **91 exact / 108 started /
108 eligible**, leaving 17 partial C reconstructions and zero unstarted
functions. The count of exact game functions across the three images stays
353. The renderer and its switch are not banked as exact.

The full three-image rebuild, OPEN strict check, Ruff, and all 376 repository
tests pass with no local skips. All 117 reconstructed object hashes are
unchanged by the SetPolyGT3 declaration correction. The new union types also
leave existing consumers' code unchanged; only the new renderer and the exact
caller's symbolic callee rename changed objects in the initial OPEN rebuild.
All 63 data-owning units and 13 vendor-source controls remain exact. The
source-quality ratchet is unchanged, with 877 pointer casts below its existing
882 limit. Only the exact caller's baseline is refreshed, not the new partial
function. The four pre-existing GAME historical-best deficits documented in
`open-format-display-sdk.md` remain outside this campaign.

Staged `nix flake check -L` also passes. Its isolated run passes the 376-test
suite with 36 controls skipped because local retail, generated objects or
tools are unavailable; the workspace run above exercises all of them.

## Master follow-up: packet cursor and strict switch addends

Pre-edit snapshot for OPEN `8001764c` (`0xcf8` bytes), after integration at
`d6178e1`: 98.802410% strict; no exact-count change. All six semantic queries,
the complete retail body/CFG, both caller argument sequences, adjacent
projection/unlit code, allocator/accessor, SDK declarations, source history
and the existing relocation/path controls were rechecked. The signature,
57 ordered calls, eleven internal jumps, return/delay slot, mode constants
and vendor-negative evidence above remain applicable. Semantic-navigation
switch targets remain candidates; the raw table value-chain control is
separate evidence, not a promotion of those rows.

The first raw difference remains the frame size. A narrower source hypothesis
is supported by the cursor instructions: retail loads the header from `s1`
at `800176cc`, advances that same pointer by four in the load-delay slot at
`800176d0`, and advances it by the masked body length at `800182f8`. The
current probe instead loads from `a1`, derives the typed body in `s1`, then
produces the next header pointer in `a1`. Test an in-place byte cursor for
both advances, keeping the typed body view and all mode-local depth checks.
Compare the first raw divergence, ordered referents, static mode call paths,
and strict switch addends before deciding whether to retain the edit.

The stricter data checker finalized on master supersedes the earlier
63-of-63 statement: 49 of 63 data-owning units pass, including all but this
OPEN unit; the other thirteen failures are in GAME and outside this campaign.
In this unit all twelve case-target addends already agree, while all seventeen
default entries point four bytes later in the probe (`.text+0xca0` versus
retail `.text+0xc9c`). Retail's shared AddPrim tail loads the ordering-table
pointer at `800182d8` using `lw v0,-276(s6)`; the probe uses an additional
absolute HI16/LO16 pair. The cursor experiment does not assume it can resolve
that separate, still-unproved wider-owner question. Do not patch the switch
targets, mask the addends, or invent an overlapping aggregate to hide it.

The in-place cursor trial scored 97.272285% strict after rebuilding the real
reconstruction object and refreshing the report. All five focused controls,
including twelve static mode call-path sets, passed against that rebuilt
object. The frame became 96 bytes, but the cursor acquired a stack slot at
sp+40 and the header moved to sp+48, with repeated loads/stores absent from
retail. The first remaining divergence moved to the projected-vertex absolute
base after the initial primitive-count load; the separate ordering-table
pair and nonmatching switch addends remained. Matching the frame alone did
not recover the source shape. The trial was reverted and the previous
body-derived next-packet expression rebuilt at 98.802410%; no C, owner, relocation or
baseline change is retained. This is a negative source-form control, not
compiler attribution or a new exact function.

Final follow-up verification: all five focused tests and all 396 repository
tests pass without skips; Ruff and `git diff --check` pass. Full `kf build`
still fails its strict data gates (one OPEN and thirteen GAME units) and the
four previously recorded GAME historical-best deficits. OPEN remains 91/108
exact with 17 partial functions. No new bank entry is made, and GAME source
is unchanged.
