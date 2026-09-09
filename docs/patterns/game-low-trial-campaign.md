# GAME functions with fewer than six recorded JSON trials

The campaign starts from master `c4e6bda5`. Freeze the selection against the
integration report rather than dropping functions when their trial counts
increase. Already exact functions meet the requested byte-match outcome.
The remaining selected functions are `player_update`, `render_map_cell`,
`menu_draw_status_details`, `menu_draw_item_detail`,
`menu_draw_window_backdrop`, `map_interaction_dispatch`,
`effect_map_collision`, and `effect_projectile_update_2d`.

The recorded counts are a lower bound on work: they omit manual source
edits, compiler trace controls and verification recompilations. Six trials
is a selection threshold, not a closure criterion. Every selected function
still needs strict 100% and full raw verification to close.

## Effect collision: guarded operation boundaries

Function Match Plan: GAME `80037850`, 1900 retail bytes / 1888 source bytes,
strict 98.126310%. Fresh hashes, all six semantic views and the complete
retail CFG reconfirm six callers, VECTOR pointer / signed-word radius,
one world-query call, twelve address pairs, fifteen retail internal jumps,
32-byte frame and the defined class-zero return. Six switch-table pointer
rows remain candidate evidence; indirect reachability is incomplete.
This is game geometry and effect-target policy, not an SDK implementation.
Existing source, history and the prior return/scope/query-helper controls
remain the baseline; no data owner or signature change is proposed.

The first bounds failure still shares a later rejection in source, whereas
retail has a local bounds-return block. Test two cohesive ordinary inline
boundaries independently and together: the signed-halfword cell-range
predicate, and the entire operation after successful bounds validation.
The latter receives the already computed coordinates, Z remainder, captured
effect pointer and original position/radius. It owns the existing geometry
labels and final query. Keep division order, snapshot timing, every grid
read, signed conversion, threshold, rejection value and defined fallback.
This differs from the previously ineffective final-query-only boundary.
No forced inlining, new data view, unused local or compiler option is added.

The four states compile. The in-cell operation alone reaches 98.684210%
and 1896 bytes; it restores the local bounds rejection and retail's complete
orientation selection/conversion sequence. The first raw difference is now
the epilogue jump addend at +ac, caused by the remaining four-byte deficit.
The range helper alone and both helpers emit 1956 bytes / 93.989470%, first
diverging at +8; reject those Boolean boundaries. All four preserve the
sole call, twelve ordered address pairs and complete initialized height data.

On the independently verified in-cell source, test the remaining two
operations independently and together: explicit negative-height success
continuation to `grid_shape`, and a consumed flags local selected by the
three class arms before one shared query call. The first is a conditioned
revisit after the recovered operation boundary; the second follows retail's
three flag-selection blocks and shared call. Preserve the defined default
return 1 and every API argument. Compare these four exploratory states
against the retained canonical source and the preceding helper candidate.

Both explicit-continuation states reproduce their respective baseline in all
resolved words and ordered references. The shared flags local gives 1876
bytes / 97.164210%, losing twenty bytes from the case-local call setup.
Reject these follow-ups and retain only `effect_collision_in_cell`.

The retained form restores the missing bounds-return block and all orientation
words, including selecting the word in v0 before its halfword view in v1.
It still differs at the negative-height success edge: source branches to
`+240` with the Z shift in the delay slot; retail branches to `+23c` with the
terrain result there. Final class-three flag materialization and default
delay slots account for the remaining four-byte extent deficit. The default
remains defined; no padding, forced lifetime or undefined path is introduced.
These residues are unattributed, and the function is not banked.

Fresh canonical compilation after comment/signature formatting reproduces
the audited trial and production program sections and ordered relocations.
The focused comparison now has 126 blocks, 78 branches, twelve known return
frontiers and one return on both sides; the negative-height successor still
differs, so these counts do not prove complete CFG agreement. Six jump-table
rows and indirect reachability keep their prior evidence status.

Generated manifests are `build/low-trial-effect-{boundaries,tail}.json`;
eight scored states are in their corresponding `build/hypotheses/` folders.
Independent raw outputs are `build/low-trial-effect-objects/` and
`build/low-trial-effect-tail-objects/`. Full build retains GAME 339/362 exact
at 99.629% aggregate, OPEN 106/108 and PSX 1/1; data matches remain 12/42,
3/20 and 0/1, with zero artifact failures. Existing data/ownership/placement
checks remain nonzero. The selection remains all eight original functions.

Ruff, all 722 repository tests (123.830 seconds), and `git diff --check`
pass on the retained source. The existing collision rectangle, orientation,
boundary and query-argument controls are included. The literal ledger keeps
43 occurrences, assigning the moved expressions to their helper owner.

Follow-up Function Match Plan: retain the verified in-cell boundary, signature,
widths, data owners, call set and ordered referents. Test the negative-height
predicate as the direct nonmutating `height + floor` expression, because retail
consumes the sum only in that comparison. Independently test a real shared
collision-result exit across the geometry returns and target-policy switch.
Retail loads the terrain result in the negative branch delay slot, leaves its
following exit jump empty, and also leaves both defined-default exit jumps
empty; these are consistent with a result selected before one source return.
Compare the direct expression, a scoped negative-path result, a switch result,
and their cohesive shared-exit composition. Reject any form that changes the
sole world-query call, twelve ordered address pairs, signed comparisons, grid
reads, or defined class-zero result. Do not introduce padding or an unused
carrier.

The direct-sum and inverted direct-sum predicates both emit 1896 bytes at
98.621056%; each changes five otherwise matching register words and retains
the old negative-height delay slot. A scoped result only for that path grows
to the retail extent but disrupts the entry join. Preserve the in-place height
update and carry one genuine result through the final target-policy switch
instead. With the switch's defined fallback written first, this emits all
1900 bytes and restores the negative path, every later destination, the three
case-local stack arguments, and the class-three flag load. The retained source
matches 474 of 475 complete linked words at strict 99.873690%.

The sole residue is the delay slot after the class-three selector branch at
`+718`: retail has `nop`; the probe copies the fallback's `li v0,1` into that
slot. On the taken class-three path the following block overwrites v0 with
`0x61`; on the untaken path values above three cannot occur because the loaded
byte is masked by three. Explicit fallback returns, gotos, switch fallthrough,
preinitialization, an explicit zero case, zero/default aliases, and supported
case orders do not remove this one word while keeping the default defined.
The sole call and twelve ordered address pairs remain exactly equal. This is
an unattributed code-generation residue, so the function remains unbanked.

The focused collision controls, Ruff, all 723 repository tests (105.698
seconds), and `git diff --check` pass. The full build keeps GAME at 339/362
exact and raises aggregate similarity to 99.751%; PSX remains 1/1 and OPEN
106/108 exact. Existing data ownership and section-placement checks remain
nonzero with no artifact failures.

## Item detail: complete name-row pointer

Function Match Plan at `6cbea3fd`: refresh all six GAME views and read all
183 retail words for `80027b7c`, 732 bytes / 98.579230%. Six callers preserve
the signed-word item/shop and price-mode interface. Seventeen calls, twenty
ordered address pairs, seven blocks, three branches and the 160-byte frame
remain unchanged. Three exact siblings are controls. This is game menu
policy using the established SDK matrix providers, not vendored source.

The remaining first difference at +d4 concerns the name-table base and
twenty-byte row stride. Test the direct `const MenuGlyphRow *` local and
`name->codes[i]` access, preserving the halfword loop and all statement order.
The shared row is already established by the table and string declarations,
and the item-name frame uses it. This tests direct row ownership rather than
repeating the prior inline-copy or inline-selection helpers. No signature,
layout, call, width or compiler option changes.

Both name-pointer states compile to the same 183 linked words, seventeen
call targets and twenty ordered address pairs at 98.579230%. All three
siblings remain exact. Reject the direct-pointer spelling as an explanation
of the residue; canonical source is unchanged.

Next test the cohesive name-copy-and-draw operation across an ordinary inline
boundary, either beginning at row selection or including primitive-pointer
publication and string positioning. Both receive the existing glyph string
and signed item ID; all operations keep their order and widths. The adjacent
preview renderer repeats this complete display stage. Earlier helpers ended
before the draw call, so these controls test an additional consumer boundary.

Both stage helpers compile to identical 736-byte bodies at 97.213110%.
Independent recompilation preserves seventeen calls, twenty ordered address
pairs and all three exact siblings. They add a counter initialization at
+d4, change the copy's counter/source/destination registers, and leave the
draw call delay slot empty instead of forming the glyph-string argument
there. The entry sentinel branch consequently changes displacement at +28.
Reject both forms; they do not recover the original name-address schedule.

All five new detail states compile, and the original source remains unchanged
at 98.579230%. Reports are `build/hypotheses/low-trial-detail/` and
`build/hypotheses/low-trial-detail-stage/`; independent complete linked words
and sibling checks are under their `build/*-objects/` directories. These
negative controls do not close the function or remove it from the campaign.

Follow-up Function Match Plan: retain the complete 732-byte body, 160-byte
frame, price selection, seventeen calls and twenty ordered referents. The only
twelve unequal words form one name-copy loop. Retail computes the twenty-byte
row as `item * 5`, materializes `item_name_rows`, then completes the multiply;
it assigns source, destination and count to `a2`, `a1` and `a0`. The probe
completes the multiply before materializing the table and assigns those three
values to `a1`, `a0` and `a2`. Test ordinary pointer-walk and indexed-loop
spellings with explicit, consumed source and destination pointers, plus a
typed row-index calculation that retains the same `MenuGlyphRow` owner. Keep
the ten signed-halfword loads/stores, loop direction, draw call and all
surrounding statement order. Reject casts to incompatible table types,
volatile carriers, unrolled copies and changes outside this exact operation.

Thirty ordinary typed controls compile. Direct row aliases and indexed loops
leave nine to fourteen unequal words. Walking both halfword pointers recovers
the retail source, destination and count registers; naming the complete row
table before selecting the row also restores the split `item * 20` address
schedule. Keep that cohesive form. It preserves the 732-byte extent, 160-byte
frame, all seventeen calls, all twenty ordered referents and all three exact
unit siblings. Complete linked comparison improves from 171/183 to 180/183
equal words, although objdiff's fuzzy score decreases from 98.579230% to
97.808750%; the raw retail instructions decide this source fact.

The remaining three words are one representation of the same destination
sequence. Retail initializes `a1` to the whole string at `sp+16`, then stores
at offset four while advancing it by two. The probe initializes `a1` to the
first glyph at `sp+20`, then stores at offset zero while advancing it by two;
both touch exactly `sp+20` through `sp+38`. Their initialization also straddles
the same primitive-cursor load in the opposite order. Expressing retail's
machine cursor literally would require walking across known structure fields
with an incompatible halfword view, so this unattributed residue remains.
Generated sources, objects, linked words and the summary are under
`build/detail-copy-shapes/`. The function remains unbanked.

## Orbit helper: phase width and motion boundary

Function Match Plan: GAME `80038298`, 608 bytes / 99.934210%. Fresh hashes,
six semantic views and all 152 retail words reconfirm one dispatcher caller,
nine calls, five ordered data-address pairs, three internal jumps and the
signed-word radius/limit interface. The existing seven exact unit siblings
are controls. The function is game orbit/damage/audio policy; trigonometry,
random and square-root providers retain their SDK ownership.

The only canonical differences are ten stack allocation/save/restore words:
56-byte source frame versus 120 retail. Existing profile controls do not
explain the extra space. Do not infer or add an unused 64-byte object.
Test two real source facts independently and together: retain the loaded
phase in its actual unsigned-byte local, and isolate the complete orbit
position/angle update as an ordinary inline helper of record and radius.
Keep the existing phase masks, signed angle/center conversions, three trig
calls, X/Z/Y stores, angle publication before collision, and all subsequent
damage/audio behavior. The phase is consumed before any call on its other
path; the motion boundary returns no synthetic value or new storage.

All four states preserve 608 bytes, the 56-byte frame and exactly the same
ten stack-difference offsets. Their nine calls, five ordered address pairs
and seven exact siblings agree with retail. Neither boundary explains the
missing frame extent. Next test the actual three-axis distance calculation
as a complete VECTOR local or a small inline distance helper receiving the
existing position. Preserve X/Y/Z subtraction, signed downshift, squared-sum
order, SquareRoot0, final shift and distance threshold. Every introduced
coordinate is consumed; no unused matrix or padding object is added.

The distance helper is identical to the scalar baseline, including all linked
words and ordered references. The VECTOR form instead gives 620 bytes /
97.960526% and a 72-byte frame. It stores the three actual delta components
at sp+40/+44/+48; retail has none of those stores. This legitimate aggregate
accounts for sixteen bytes of source storage but does not explain the retail
frame. Reject it. All seven new states preserve nine call targets, five
ordered address pairs and all seven exact siblings. Six states reproduce
the original complete function words; no orbit source or bank change is kept.

Generated reports are `build/hypotheses/low-trial-orbit/` and
`build/hypotheses/low-trial-orbit-distance/`; fresh independent objects and
complete resolved words are in the corresponding `build/*-objects/` folders.
The 64-byte frame difference remains unattributed. The earlier profile
controls and these source controls are negative evidence, not authority to
add an unobserved local object.

The detail/orbit follow-up changes evidence only. Ruff, all 722 tests
(106.976 seconds), and whitespace checks pass. A fresh full build retains
GAME 339/362 exact and 99.629% aggregate, with unchanged data/placement
failures and zero artifact failures. No partial function is banked.

## Map cell: computed matrix ownership

Function Match Plan at `81a59be6`: GAME `8001e5ec`, 592 bytes / 99.878380%.
Fresh hashes, six semantic views and all 148 retail words retain the word
column/row and byte visibility interface, sole dispatcher caller, nine calls,
ten ordered address pairs and five internal jumps. The dispatcher is an
exact control. The map policy is game code; its matrix operations retain
their existing SDK providers. Shared grid, view and MATRIX ownership remains.

Retail uses a 120-byte frame versus source 88. Existing profile controls
leave that discrepancy. Test a cohesive helper computing the actual cell
matrix: install the view transform, transform position into its translation,
then multiply the view and quadrant rotations. Compare ordinary inline
interfaces returning the consumed MATRIX value or filling the caller's
MATRIX pointer. The helper owns the real RotTrans flag output. The caller
keeps position construction, orientation adjustment and all rendering calls.
Preserve translation-before-rotation order and the authentic SDK types.
The returned matrix is consumed, not an unused object added for stack size.

All three states compile. The output-pointer helper gives 604 bytes /
94.533780% and a 96-byte frame, adding an s3 save and changing the downstream
register roles. Returning MATRIX gives 668 bytes / 85.006760% and a 128-byte
frame. It constructs the matrix at sp+56 and copies eight words into the
caller's matrix at sp+16; retail has no such copy. Neither explains the
retail 120-byte frame. Reject both and preserve the 592-byte canonical body.

Independent recompilation resolves all target words to retail and preserves
all nine calls, ten ordered address pairs and the exact dispatcher in each
state. Canonical retains its eighteen stack-related unequal words. Generated
states are `build/hypotheses/low-trial-cell/`; independent objects, complete
resolved words and verification are under `build/low-trial-cell-objects/`.
The full build again retains GAME 339/362 exact, 99.629% aggregate and zero
artifact failures, with the existing data/ownership/placement checks nonzero.
Canonical source, configuration and bank entries remain unchanged.

## Status detail: refreshed entry baseline

Fresh retail validation and six GAME semantic views reconfirm `800264d8`,
3252 bytes, strict 98.425580%. Independent compilation preserves the 80-byte
frame, all 72 ordered calls and 89 ordered data-address pairs. Resolving the
target object reproduces every retail word. The source remains unchanged.

The first difference is still +3c: source leaves the primitive-buffer load
delay empty; retail materializes Y=35 in s2 there. Source materializes that
coordinate later in t0, shifting the subsequent instruction stream by one
word until later differences compensate. Therefore the large count of
unequal same-offset words is not a count of independent source defects.
The earlier signed-rating corrections remain the baseline; this inspection
does not justify revisiting them or introducing a constant-carrying local.
Generated independent evidence is under
`build/low-trial-status-baseline-objects/`. This is a baseline audit, not a
new scored source hypothesis or a closure claim.

Function Match Plan: test the established signed-halfword MenuPoint operation
at the four explicit X/Y anchors through an ordinary inline setter. Separately
test the complete first experience-label draw through an inline helper taking
the existing glyph workspace and signed-halfword coordinates. The adjacent
header repeats that same label operation. Preserve primitive publication,
all four glyph stores, coordinate widths, and the draw call. These are consumed
object operations, without an artificial constant local or forced inlining.
The complete retail body, caller, adjacent sources and three shared game
render/format interfaces were reviewed; no SDK body or shared type is changed.

All three scored states compile and preserve the 80-byte frame, 72 ordered
calls and 89 ordered address pairs. The position helper gives 3264 bytes /
94.432970%; the experience-label helper gives 3256 bytes / 96.937270%.
Both first diverge at +34, introducing `s1 = sp+16` before the primitive
pointer loads. Both retain the empty load delay and late Y materialization,
and change subsequent glyph-string arguments to moves from s1. This does
not recover the retail entry sequence. Reject both helpers and retain the
3252-byte source at 98.425580%, without banking.

The generated manifest is `build/low-trial-status.json`; scored reports are
under `build/hypotheses/low-trial-status/`. Independent recompilation and
complete resolved words are under `build/low-trial-status-objects/`.
These three states count as new recorded comparisons, including the control;
the prior standalone baseline audit does not count as another hypothesis.

Full `kf build` retains GAME 339/362 exact and 99.629% aggregate, with zero
artifact failures and the existing data/ownership/placement failures.
Whitespace checks pass. No production source or configuration changed.

Follow-up Function Match Plan: preserve the complete status-page call stream,
80-byte frame, glyph workspace, derived ratings, and all 89 ordered referents.
Retail materializes the summary-column Y origin `0x23` into `s2` in the
primitive-cursor load delay, uses it for the first label row, keeps it across
the label calls, and uses it again when the numeric column restarts. The probe
materializes the same value twice in `t0`, leaving that load delay empty and
shifting allocation through most of the body. Test one ordinary signed
coordinate local consumed by exactly those two existing Y assignments, with
both boundary-width and arithmetic-width declarations. Its lifetime ends at
the restart; retail then reuses `s2` for the independently evidenced class-row
step. Do not change a coordinate value, duplicate an operation, extend the
local beyond a real use, or add volatile/register qualifiers.

The three boundary-width controls (`s16`, `u16`, and `s32`) are byte-identical
to baseline because the probe propagates `0x23` into both stores. Retail gives
the same `s2` a second, contiguous role: it assigns the value 16 during class
selection and uses that register for exactly the eight subsequent left-column
row advances, ending before the right column begins. Refine the test to one
ordinary Y-layout variable that changes from the column origin to the row
advance at that real lifetime boundary. This is a consumed source value at
every use, not an allocation-only carrier.

Eighteen unique typed and placement states compile. A single changing `s32`
value reaches 99.464940% but stays four bytes long because its second value is
scheduled in the later physical-tier branch. Giving the two evidenced roles
their actual disjoint scopes closes the function: the outer `s32` origin ends
after the numeric-column restart, and an inner `s32` row step covers class
selection through the final left-column value. The probe reuses `s2` for those
nonoverlapping lifetimes and places both initializers in the retail delay
slots. The boundary-width `s16` scoped control leaves eight extension-form
words unequal, confirming the arithmetic-width choice.

The retained source is 3252 bytes at strict 100%. Complete independent linked
comparison has zero unequal words and preserves the 80-byte frame, all 72
calls, all 89 ordered referents, 31 CFG blocks, 16 branches, and the single
return. Generated states, objects, linked words, and the summary are under
`build/status-origin-shapes-6/`. The single strict-exact row is banked by
GAME image and address.

## Map interaction: partner-definition ownership

Function Match Plan: GAME `80034de4`, 2308 retail bytes, strict 99.202774%.
Fresh hashes and all six views retain the camera VECTOR/SVECTOR interface,
sole player-update caller, 54 calls, thirteen address pairs and 72-byte frame.
All retail instructions and current source were read. The 89 switch pointers
remain candidates and both indirect successors remain unresolved. This is
game interaction policy; trigonometric SDK providers retain their ownership.

Preserve the earlier container-loop and gold corrections. The remaining
paired-door admission reads its definition once in source versus three times
in retail. Test a definition pointer scoped to the actual neighbor iteration,
and separately an ordinary inline admission predicate receiving that same
definition. Preserve the nested validation, failure notification, action call,
link writes and original-door join. Earlier selection-before-validation moved
the loop back edge; these controls leave that source topology intact. Neither
adds volatile qualification or a synthetic store to defeat reuse.

All three states compile to identical 2296-byte dispatcher bodies at
99.202774%, retaining the 72-byte frame, 54 calls and thirteen address pairs.
Independent recompilation proves complete linked-word equality between the
variants, verifies every target word against retail, and retains all fourteen
exact siblings. The first difference remains +2c, with the position pointer
in s8 instead of s7. Neither scope nor the admission helper recovers the
missing partner-definition reloads; reject both and keep canonical source.
Generated reports are `build/hypotheses/low-trial-interaction/`, with independent
objects and complete dispatcher words under `build/low-trial-interaction-objects/`.

Follow-up Function Match Plan: keep the established object-loop CFG, current
definition owner, call set, widths and thirteen ordered referents. Retail forms
the partner definition address once but loads its byte behavior separately for
the range admission, linked-door rejection, and action conversion. The probe
currently carries the first loaded byte through all three consumers and is
twelve bytes short. Test direct typed definition expressions at the three real
consumers and a split binding after admission, independently of the already
ineffective declaration-scope and predicate-helper controls. Preserve the
neighbor object ID, definition table, nested link test, notification path,
action call and both link writes. Do not use volatile qualification, synthetic
stores or incompatible structure views to force reloads.

The direct-expression, split-binding and operation-boundary states all compile.
Direct expressions at every consumer give 2300 bytes / 98.639510% and preserve
the ordered calls and referents, but still reuse the behavior byte and form the
definition address through `at`. Binding after admission and spelling the late
consumers directly give 2316 and 2332 bytes / 98.812830% and 98.674180%; both
change referent order without recovering the three retail loads. A boundary
around the action setup is identical to canonical, while the linked-door
predicate boundary gives 2300 bytes / 98.344890% and retains the same missing
loads. Reject all seven follow-up states and keep canonical source.

The complete independently resolved comparison keeps the 54 ordered calls.
Only the canonical, all-direct and operation-boundary states retain all
thirteen referents in retail order. The twelve-byte deficit therefore remains
an unattributed value-lifetime residue. Forcing three loads would require a
volatile or synthetic alias fact unsupported by the object model; neither is
introduced. Results and complete linked words are under
`build/interaction-definition-shapes/`.

Full build retains GAME 339/362 exact and 99.629% aggregate, with the existing
data/ownership/placement failures and zero artifact failures. Whitespace
checks pass; no production source, configuration or bank change is kept.

## Window backdrop: repeated tile operation

Function Match Plan: GAME `8002a510`, 1700 bytes, strict 99.971760%.
Fresh hash validation, six semantic views, the complete retail body and source,
three void callers, adjacent helpers and source history retain sixteen calls
and 107 validated address pairs. Retail allocates 104 stack bytes. The four
reflected tiles and descending four background primitives are game composition;
SetSemiTrans and AddPrim remain the established SDK providers.

Test the complete repeated tile operation through an ordinary inline helper
of signed-word X/Y and horizontal/vertical reflection flags. The status-panel
caller already uses this operation shape. Preserve each begin, transparency,
texture/CLUT, XY/UV write and commit in order, followed by the four existing
AddPrim calls. Use the same global texture-page read, authentic POLY_FT4
layout and shared sprite fields. Do not add a stack object for the frame gap.

The first helper control gives 1704 bytes / 97.334114%, with a 32-byte frame
and 111 address pairs instead of 107. It no longer keeps the texture-page
address across tiles, rematerializing direct addresses instead. Test that
observed shared address explicitly as a `const u16 *` passed to the same
helper, as the existing status-panel helper does. This pointer supplies four
real loads; it is not an unused carrier. Keep every other operation fixed.

The shared-page helper reproduces all baseline words and ordered references:
1700 bytes, 40-byte frame, sixteen calls and 107 address pairs. Its twelve
unequal retail words remain stack allocation/save/restore instructions.
Neither helper explains the 64-byte frame gap; retain the original source
at 99.971760%. Every other unit function remains identical across independent
compilations. All target functions independently resolve to their retail bytes.

Reports are `build/hypotheses/low-trial-backdrop{,-page}/`, with independent
objects and complete dispatcher words in the corresponding
`build/low-trial-backdrop{,-page}-objects/` directories. There are three distinct
source states; the second manifest repeats its two controls. Full build
retains GAME 339/362 exact and 99.629% aggregate with existing data/placement
failures and zero artifact failures. Whitespace checks pass. No source or
bank change is kept.

## Player update: magic-button failure and recharge paths

Function Match Plan: GAME `80018880`, 6684 retail bytes, strict 96.939560%.
Fresh hashes and six views, all retail instructions, current source/history
and the sole game-loop caller preserve the void interface. This is game
input/movement/magic/status policy, using established SDK providers.

Retail's normal-magic attempt reaches `800193bc` on absent magic, incomplete
charge or insufficient MP, clearing weapon_attack_fully_charged and jumping
to weapon-magic processing. Successful casting also reaches that clear after
resetting magic_charge. Recharge begins at `800193cc` only on the other
button-edge path, and first rejects the no-magic sentinel. Current source
places a selected-magic recharge block on failed attempts and an unguarded
recharge block on the other path. Restore this decoded branch ownership,
keeping the earlier successful weapon-magic jumps and promoted item argument.
No arithmetic, widths, field identities, calls or frame fillers are invented.

Keep the corrected branch ownership. Independent compilation produces 6632
bytes and a 216-byte frame in both states, with strict match improving from
96.939560% to 96.954520%. Exactly five instructions change, at function offsets
`8d0`, `8e8`, `a8c`, `aa0`, and `b18`; every corrected branch word equals retail
at the same offset. All other words, all 66 ordered calls and all 204 source
address pairs are unchanged. Retail has 210 pairs, so this is not whole-function
referent closure. The remaining 52-byte size deficit and frame difference
remain unattributed.

The independent audit resolves section-relative data references through each
named object's extent and curated VA. A single aggregate `.data` or `.bss`
base is invalid for this unit's scattered claims; its initial exploratory
address comparison is superseded by that object-owned audit. The complete
target still resolves to all 1671 retail words. Reports and controls are
`build/hypotheses/low-trial-player/`, `build/low-trial-player-objects/` and
`build/low-trial-player-audit.py`.

Focused match and full build retain 339/362 exact GAME functions; aggregate
match is 99.630%. Full build fails existing data/placement checks, with
12/42 GAME data owners matching and zero artifact failures. No exact claim
or bank promotion is made for this correction.

Ruff, all 722 repository tests (106.776 seconds), and whitespace checks pass
for the retained player correction and preceding map/menu experiments.

## Player status: shared cancellation/expiry blocks

Function Match Plan at `9e5985ab`: GAME `80018880`, 6684 retail bytes,
6632 source bytes / 96.954520%. The complete raw comparison after the magic
correction preserves 66 calls and 204 source address pairs against retail's
210. The established void interface, sole caller, typed player object, SDK
boundaries and prior six-view evidence remain applicable. Current source,
history and the complete slowed/poison/curse retail slices were reread.

Retail cancellation jumps to common flag-clearing blocks at `8001a064`,
`8001a0cc` and `8001a180`. Current C duplicates those clears in cancellation
and expiry arms, and compiled code reuses the earlier flag value rather than
retail's fresh address/load. Test explicit shared labels independently for
these three existing operations, then together. Preserve all timer writes,
poison remainder width, curse recalculation, call order and defined paths.
This is a control-flow hypothesis, not a request to force a reload or pad
frames; reject it if it fails to explain the observed retail structure.

All five states compile. Individual slowed/poison/curse shared blocks reach
97.308200%, 97.302216% and 97.401560%; together they reach 98.102936%
(6688 bytes, 216-byte frame). The complete three-status region is now equal
in all 108 instructions when internal jump targets are expressed relative
to its start. Keep all three shared blocks. Independent objects preserve
all 66 calls and restore the six missing address pairs (210 total); earlier
weapon-spawn address ordering still differs from retail. A focused regression
control recompiles the whole source and compares the full status region,
including loads, calls, branch displacements and delay slots. It passes this
source and rejects the previous source.

On this retained source, test darkness fade as a promoted signed-word value
whose two arithmetic results explicitly pass through the evidenced signed
halfword conversion. Retail sign-extends both differences before the sign
branches at `8001999c` and `800199b4`, then shifts the resulting word for both
lighting calls. The current signed-halfword local keeps a separate unextended
value across those branches and combines extension with the eventual shift.
Keep both conversions, formulas, branches and API arguments; do not widen
away the halfword semantics or alter the fixed-point scale.

The promoted halfword fade reaches 98.470980% without changing size or frame.
Independent compilation confirms that the full 66-instruction darkness
region now equals retail after resolving named relocations and expressing
internal jumps relative to the region start. The regression control covers
this region alongside the 108 slowed/poison/curse instructions. Keep the
explicit signed-halfword results in the promoted local. All 66 calls remain;
210 address pairs remain, with only earlier weapon-spawn ordering unresolved.
The first raw difference is still the frame prologue, followed by the
weapon-spawn local layout and instruction ordering. No compiler attribution
or exact claim follows from these partial improvements.

Generated evidence is under `build/hypotheses/low-trial-player-{status,fade}`,
`build/low-trial-player-{status,fade}-objects`, and the corresponding audit
scripts and residue reports. Canonical focused match and full build retain
339/362 GAME exact functions at 99.694% aggregate. Existing data/placement
failures remain (12/42 GAME data owners; zero artifact failures).

Final combined verification: Ruff and all 723 repository tests pass
(115.411 seconds), as does `git diff --check`. No partial result is banked.

## Player weapon spawn: camera-position owner

Function Match Plan at `f56b1b28`: GAME `80018880`, 6684 retail bytes,
6688 source bytes / 98.470980%. Fresh retail validation and the prior complete
six-view pass retain the void interface, sole caller, 66 calls, 210 address
pairs, typed SDK aggregates and game ownership. The frame gap is already an
unattributed residue; no placeholder object or forced alignment is permitted.

After `ApplyMatrix`, retail consumes camera position Y, camera rotation Y,
camera position Z, camera rotation X and camera rotation Z in that order.
Source contains that semantic statement order, but the probe hoists the three
rotation reads and leaves the position reads later. The existing `origin`
pointer is the proved camera-position owner used by the following cone query.
Test moving its assignment before translation, first retaining direct field
access and then consistently using `origin->vx/vy/vz` for the three additions.
Keep the same VECTOR extent, position arithmetic, rotation halfword writes,
query arguments, calls and constants. This is an ownership/alias hypothesis;
reject it if it does not restore the ordered references and retail operations.

All three pointer-placement states compile to the same object and remain at
98.470980%; assignment placement and member spelling alone do not change the
five references. Reject both. As a focused follow-up, test the same six
consumed assignments through one ordinary typed operation accepting position,
effect rotation, camera position and camera rotation pointers. This preserves
the existing object family and operation order while representing the alias
boundary visible in retail. Compare it against the unchanged baseline; the
operation must inline completely and must not add a call or unclaimed body.

The typed operation inlines but drops to 97.881510%; reject it. Its broader
alias boundary changes instructions outside the five-reference residue. Test
two direct, object-grouped statement orders next: translate X/Y/Z before
copying rotation Y/X/Z, and translate X/Y/Z before copying rotation X/Y/Z.
Both preserve all values and the final objects. Retail may interleave the
loads while scheduling these independent groups; retain a grouping only if
raw order and instructions move toward retail without changing referents.

Both object-grouped orders compile identically at 99.476960%. They recover the
exact 6684-byte function extent, all 66 ordered calls and all 210 ordered
address pairs. Keep the natural X/Y/Z grouping for both position and rotation;
retail cannot distinguish it from the Y/X/Z source order because both compile
to the same object. The first remaining difference is the 224-byte retail
frame versus 216-byte source frame. Local accesses throughout the weapon-spawn
region differ by eight bytes; attachment and direction registers also differ
after the cone query. These remain unattributed. No unreferenced object,
alignment annotation or register carrier is added.

The independent audit resolves every target function word to retail, compiles
the six source states separately, and records the reduced raw residue in
`build/low-trial-player-origin-residue.txt`. The typed helper is excluded from
that final comparison because it emitted an unclaimed body and was already
rejected. A repository regression control now compares all 66 external calls
and 210 named data references in order, in addition to the two complete exact
status regions.

Canonical focused matching confirms 99.476960% and preserves 339/362 exact
GAME functions. The required full build raises aggregate GAME similarity to
99.737% and retains the existing data/placement failure: 12/42 GAME data
owners, zero artifact failures and 75/77 target units verified. This partial
function is not banked.

Ruff, all 723 repository tests (104.379 seconds), and `git diff --check` pass
with the grouped source and expanded ordered-referent control.
