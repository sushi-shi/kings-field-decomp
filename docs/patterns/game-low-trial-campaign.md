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

Second follow-up Function Match Plan: GAME `80037850`, 1900 bytes and strict
99.873690%, still has one unequal linked word at `+718`. Fresh retail hashes
and all six semantic views reconfirm six callers, one world-query call, twelve
ordered address pairs, 126 blocks, 78 branches, twelve known return frontiers,
and no strings. The adjacent effect-pool setter and exact magic-power helper
support game ownership; no Psy-Q archive or vendored inventory identifies this
map-policy body. The only mismatch remains retail `nop` versus probe
`li v0,1` in the delay slot of the selector-three branch. Both paths already
carry the comparison value one in `v0` for the only masked default state.

Test directly evidenced scalar types for the consumed result and a named
masked-selector local. Then test whether the selector dispatch's existing
Boolean value can define the class-zero fallback before the three query cases
overwrite it. Keep the current shared result exit, negative-height path,
defined fallback, one call, argument flags, complete CFG and every referent.
Reject any spelling that adds a carrier, changes behavior for the four masked
states, or perturbs an otherwise equal word. Separately sweep only the existing
pinned 2.5.7 scheduling profiles; retain a profile change solely if complete
raw instructions and relocations become exact without source distortion.

All fifteen unique controls compile. Signed-word result spellings and signed
or byte selector locals reproduce the one-word residue. Halfword results add
extensions and change the extent. Initializing the real result from the
masked-selector predicate grows the body and changes 32 or more linked words;
it does not reuse the switch-lowering comparison. The generic 2.5.7 profile
changes seven words beginning at `+1d0`; disabling scheduling changes 403,
and the 2.6.0 probe changes 460. Every state retains the sole call and twelve
ordered referents, but none is exact. Keep the canonical source and R2000
2.5.7 profile. Results are under `build/effect-final-word/`; the remaining
delay-slot difference stays an unattributed code-generation residue.

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

Second follow-up Function Match Plan: refresh GAME `80027b7c`, all 732 retail
bytes, six callers, seventeen calls, twenty ordered address pairs, seven CFG
blocks, three branches, no strings, adjacent menu renderers, source history and
the current 180/183 linked-word result. The loop's destination is direct
evidence for a complete `MenuGlyphString` storage view: retail initializes
`a1` to the workspace base at `sp+16`, writes each glyph at the proven
`glyphs.codes` offset four, and advances the base as a halfword cursor. This is
game menu composition around established GTE and renderer APIs, with no
vendored-body match.

Test a union that owns the same 24-byte workspace as both the established
`MenuGlyphString` fields and a twelve-halfword representation. Walk the union's
halfword member from element zero and write element two while advancing it,
then use the structured member for every position update and renderer call.
The union must retain size 24 and alignment two. Compare a file-local proof
first; promote the view to the shared menu type only if the complete unit and
all users remain well typed. Preserve the source/name pointer walk, loop count,
frame, seventeen calls, twenty referents and three exact sibling functions.
Reject raw byte casts or a pointer that escapes a struct subobject.

Forty-nine ordinary controls compile across all six declaration orders for
the cursor, position and source setup, both pointer/count initialization
orders, indexed union forms, scoped position setup, and typed primitive-buffer
or cursor locals. The file-local union recovers the retail workspace-base
pointer and offset-four store throughout the loop. Keep it: this complete
object view is directly supported by the accesses and preserves the shared
structured API at every call.

The retained function is 732 bytes with 181 of 183 complete linked words
equal. Its seventeen calls, twenty ordered referents, seven CFG blocks, three
branches, 160-byte frame and three exact siblings all agree. At `+ec` and
`+f4`, retail initializes the zero count, performs the runtime cursor load,
then initializes the workspace pointer; the probe initializes the pointer,
performs the same load, then initializes the count. Reversing the two source
initializers changes four later loop words as well. Every tested statement
order leaves at least these two words unequal. Results are under
`build/item-workspace-order/`. The remaining pair is an unattributed scheduling
residue, so the function remains unbanked.

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

Follow-up Function Match Plan: GAME `80038298` remains 608 bytes at strict
99.934210%. Fresh retail validation and all six semantic views reconfirm the
signed-word radius/phase-limit interface, sole dispatcher caller, nine calls,
five ordered address pairs, three internal jumps, 29 CFG blocks, fourteen
branches, one return and no strings. The 56-byte probe frame and 120-byte
retail frame differ only in ten allocation/save/restore words; retail never
accesses the intervening 64 bytes. The body remains game orbit, collision,
damage and audio policy around separately owned SDK providers.

Trace the pinned compiler's actual lifetimes for the effect record, magic
record, phase and collision values, frame construction and spills. Test the
real narrower ownership already present: move `magic` and `collision` into
the only motion path that consumes them, independently and together. Retain
the earlier byte-phase, motion-helper and distance-aggregate controls as
negative evidence. Use incompatible unaccessed 64-byte shapes only to test
whether frame extent mechanically explains exact bytes; never retain them.
Preserve all nine calls, referents, collision classes, damage arguments,
sound policy, phase path, branches and delay-slot operations.

Native/instrumented parity passes for all seven states. The current allocator
keeps the effect record in `s0` across nine calls and the magic record in `s1`
across seven; collision remains a call-free `a0` value, and the named phase
folds before allocation. No spill register is required. Scoping collision to
its consuming path is byte-identical to canonical. Scoping the magic pointer
shortens its measured lifetime by three units but changes instruction order
throughout the motion path and falls to 94.144740%, without changing the
56-byte frame. Reject both magic-scope states.

As with the backdrop, three incompatible unaccessed declarations each produce
strict 100%: 64 bytes, two matrices, or sixteen words. Each merely shifts the
four saved registers by 64 bytes and leaves the 152 instruction words otherwise
unchanged. The untouched retail interval cannot distinguish these source
types, so none is a valid reconstruction. Canonical remains 608 bytes at
99.934210% and is not banked. Results and traces are under
`build/hypotheses/projectile2d-instrumentation/` and
`build/gcc257/projectile2d-instrumentation-traces/`.

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

Follow-up Function Match Plan: GAME `8001e5ec`, 592 bytes and strict
99.878380%, still differs only in eighteen stack-address words. Fresh retail
validation and all six semantic views reconfirm the word column/row and byte
visibility interface, sole exact dispatcher caller, nine calls, ten ordered
address pairs, 24 CFG blocks, twelve branches, three known return frontiers,
and no strings. Retail and source agree outside one uniform 32-byte frame
displacement: retail allocates 120 bytes and places `position` at `sp+80`, the
`RotTrans` flag at `sp+88`, and saved registers at `sp+104..116`; the probe
allocates 88 bytes and places the same objects at `sp+48`, `sp+56`, and
`sp+72..84`. The body is game map/render policy around separately owned Psy-Q
matrix providers, and no archive or vendored inventory identifies it as a
library routine.

Trace the pinned GCC 2.5.7 probe before proposing another source shape. Record
the real automatic allocations, lexical lifetimes, frame construction and
spill requirements for `cell_matrix`, `position`, and `flag`, with normal and
instrumented whole-object parity. Then test only complete, consumed object
forms supported by the retail accesses: declaration-scope alternatives for
the existing objects and authentic aggregate ownership where the same MATRIX,
SVECTOR and flag values remain observable. Preserve every call, referent,
width, branch, delay-slot operation and the exact sibling. Reject unused
objects, artificial padding, volatile/register steering, incompatible SDK
views, or any source whose extra frame storage has no accessed semantic owner.
Strict 100% plus full raw verification remains required before banking.

The native and instrumented objects are byte-identical. EXPAND allocates the
32-byte `MATRIX`, eight-byte `SVECTOR`, and four-byte flag as three real stack
objects; RELOAD needs no spill registers, and lexical scoping or combining the
three values into one authentic aggregate leaves the 88-byte frame and every
linked word unchanged. Reordering the matrix after the smaller objects grows
the frame to 96 bytes, saves an extra register, and falls to 94.527020%.

Three diagnostic controls each reach strict 100%: a two-element MATRIX array,
two separate matrices, and a record containing two matrices. They all add one
unaccessed 32-byte capacity before the real matrix, produce the same 120-byte
frame, and collapse to the same exact machine frontier. This ambiguity proves
that the retail bytes do not identify which source object occupied `sp+48..79`.
The exact controls violate the source model because the additional capacity is
never accessed; none is retained or banked. The exact OPEN counterpart uses
the same one-MATRIX/SVECTOR/flag source family without that capacity, and no
shared game type or source history supports a 64-byte owner here.

All seven repaired stack-shape states compile and preserve the nine calls and
ten ordered referents. The canonical 592-byte body remains 99.878380% with its
eighteen uniform stack-word differences. Instrumented summaries and retained
traces are under `build/hypotheses/render-map-cell-stack-shapes-v3/` and
`build/gcc257/render-map-cell-stack-shapes-v3-traces/`; the baseline allocation
trace is under `build/hypotheses/low-trial-cell-trace-baseline/`. This function
remains open because exact bytes alone do not support a truthful source choice.

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

Second follow-up Function Match Plan: keep GAME `80034de4` at the established
2308-byte target, 72-byte frame, 54 ordered calls, thirteen ordered referents,
complete switch policies and current source semantics. The remaining early
differences show two linked allocation questions: retail keeps `position` in
`s7` and its shared minus-one sentinel in `s8`, while the probe reverses those
roles; inside the hinged-container prescan, retail separately materializes a
local minus one and jumps back to the following comparison, while the probe
reuses the outer sentinel and jumps to the preceding nop. The partner-door
path also retains the separately established three-versus-one behavior loads.

Trace the pinned 2.5.7 allocator, CSE, loop and reload decisions for the real
position, rotation, object index, prescan counter, neighbor index and definition
pointer before another source experiment. Compare only actual lifetime changes:
disjoint aliases used by the coordinate and frame-render operations, explicitly
scoped countdown ownership, and direct typed definition consumers at their
three observed policy decisions. Reuse prior controls as negative evidence.
Every state must preserve the signature, widths, all calls and referents, and
the defined failure paths. Reject synthetic sentinel carriers, volatile loads,
fake aliases or code added solely to select hard registers.

Native/instrumented parity passes. The current trace assigns the `position`
pseudo to `s8`: eight references span 406 allocation units and 36 calls. CSE1
creates a separate compiler-only `-1` pseudo with 27 references over 602 units
and assigns it to `s7`; it has no unique source local. The two block-owned
`item_index` declarations remain distinct pseudos, while `neighbor_definition`
has only a two-unit, call-free lifetime in `v0`. RELOAD reports one single-register
search and no multi-register spill requirement. These are measured decisions
of the pinned probe, not historical compiler attribution.

Instrumenting the strongest prior lifetime controls explains their limits. A
word sentinel initialized at function entry receives `s8` and moves `position`
to retail's `s7`, but its 744-unit lifetime lets the scheduler place `li s8,-1`
in the first `rsin` delay slot, where retail has `nop`; the body also contracts
to 2292 bytes. Initializing the same real local at the object-loop boundary
keeps `position` in `s8` and assigns the sentinel to `s7`. Halfword sentinel
forms are optimized out as named pseudos; the best scores 99.324090% but still
has the reversed saved-register roles and does not recover the prescan CFG.
Reusing the existing general result as the sentinel assigns it to `s5`, grows
its lifetime across 30 calls, and falls to 98.778160% or lower.

The earlier separate typed partner behaviors also remain negative evidence:
the call-free definition/value region is collapsed before allocation, so
source scopes do not establish three loads. Restoring them would still require
an unsupported volatile or alias claim. Across nine new instrumented states,
the canonical 2296-byte function remains the best truthful source at
99.202774%; all 54 calls, thirteen referents, frame and failure behavior stay
unchanged, and nothing is banked. Results and traces are under
`build/hypotheses/interaction-{instrumentation,prior-lifetimes,result-sentinel}/`
and `build/gcc257/interaction-*-traces/`.

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

Follow-up Function Match Plan: GAME `8002a510` remains 1700 bytes at strict
99.971760%. Fresh retail validation and six semantic views reconfirm three
callers, sixteen calls, 107 ordered address pairs, a single CFG block, no
branches, one return and no strings. Every non-stack instruction and referent
already agrees. Retail reserves 104 bytes and saves `s0`-`s3`/`ra` at
`sp+80..96`; the probe reserves 40 and saves them at `sp+16..32`, leaving the
64-byte retail interval untouched. The menu tile composition is game code;
the packet macros and ordering-table insertion retain their SDK boundaries.

Trace the pinned compiler's real stack-object acquisition, frame construction,
saved-register allocation and address lifetime for the shared menu asset and
primitive-buffer owners. Then compare only consumed tile aggregates and
operation scopes already supported by the four repeated quads, using the
earlier helper and typed-local controls as negative evidence. Separately use
multiple unaccessed 64-byte object shapes only as diagnostic ambiguity controls.
They may show whether frame extent alone closes the bytes, but cannot be kept.
Preserve all calls, ordered referents, packet writes and persistent-quad order;
do not retain padding, unused storage, volatile state or a forced register.

The current native and traced objects agree byte for byte. The probe acquires
no automatic stack object and needs no spill register; its frame consists only
of the five saved registers. A direct `const u16 *` texture-page owner and a
direct `const MenuTileSprite *` owner are both real, consumed aliases spanning
nine references and thirteen calls in `s2`. Each compiles identically to the
canonical 1700 bytes and 40-byte frame, so neither explains retail storage.
The earlier helper and accessed `KfSpriteQuad` controls remain negative.

Three diagnostic declarations each reach strict 100%: 64 unaccessed bytes,
two unaccessed matrices, or sixteen unaccessed words. All produce the same
104-byte frame and exact 425 linked words. Their incompatible C types prove
that the untouched interval supplies no object identity; keeping any would be
an unused-storage codegen device. The canonical source remains 99.971760%, and
the function is not banked. Six valid native/instrumented states and retained
traces are under `build/hypotheses/backdrop-instrumentation-v2/` and
`build/gcc257/backdrop-instrumentation-v2-traces/`.

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

Follow-up Function Match Plan: retain GAME `80018880`, its 6684-byte extent,
66 ordered calls, 210 ordered address pairs, exact status/fade regions, typed
SDK aggregates and the natural grouped spawn transform. Retail allocates 224
stack bytes while the probe allocates 216; every address-taken spawn local is
therefore eight bytes earlier in the probe. The first local sits after the
eight outgoing stack-argument words, and neither object accesses the apparent
eight-byte retail gap. Before proposing another source object, sweep the pinned
2.5.7 scheduling profiles and inspect every call's stack-argument high-water
mark. Then test only real aggregate lifetime boundaries already present in the
spawn operation: direction, spawn offset, effect rotation, world position,
matrix and distance. A retained hypothesis must explain the frame and the
observed pointer setup while preserving every value, call, referent and the two
exact regions. Do not add an unused local, padding, alignment annotation,
volatile qualifier or incompatible SDK view.

The profile sweep and nine lifetime states compile. Debug information modes,
the generic 2.5.7 profile, and the available scheduling controls do not produce
the 224-byte retail frame. Scoping attachment objects, spawn offset, matrix and
distance independently or together reproduces the canonical 216-byte frame
and residue. Scoping direction improves objdiff's fuzzy score but moves it to
the last stack slot, contradicting retail's first address-taken local at
`sp+40`; shortening matrix and direction together shrinks the frame to 208.
Reject those layouts. Every retained control preserves the 6684-byte extent,
66 calls, 210 ordered referents and both exact regions. Generated states and
summaries are under `build/player-lifetimes/` and
`build/debug-local-allocation/`. The unexplained eight-byte frame interval is
not authority for an unused object, so the canonical source remains unbanked.

Second follow-up Function Match Plan: refresh GAME `80018880` against the
current typed source and retain the exact 6684-byte extent, 66 ordered calls,
210 ordered address pairs, complete status/fade regions and exact prefix
through offset `d60`. Retail materializes the address of the first
address-taken `SVECTOR` in `s1` before both direction calls and reuses it for
all three effect constructors; the direct-address source waits until after
the two direction calls and materializes it in `s2`. Test one ordinary
`SVECTOR *` owner for this existing object, with no new storage or call, and
require the complete register/call sequence to move toward retail. Separately,
test whether the already reconstructed signed forward, strafe and magnitude
lanes were members of one horizontal-motion `SVECTOR`: this is the only live
eight-byte aggregate suggested by the function's earlier semantics. It may
explain the untouched two-word interval only if the compiler eliminates all
member storage accesses while preserving the already exact prefix. Reject it
on any earlier instruction, call, referent, CFG or value change. Do not test or
retain an unused declaration, padding, alignment annotation or volatile
carrier.

All four current-source states compile. Grouping forward, strafe and magnitude
as an `SVECTOR` does produce the retail 224-byte frame, but adds 36 bytes of
code and first changes the already exact linked stream at offset `40`; reject
both motion states. The explicit launch-direction owner retains 6684 bytes,
the 216-byte frame, all calls/referents and both exact regions while raising
strict similarity from 99.476960% to 99.694790%. It recovers retail's `s2`
attachment value, the `s1` direction address before
`pitch_yaw_to_forward_vector`, both direction-call arguments, and all three
constructor stack arguments exactly. Keep this ordinary typed pointer owner.
The remaining linked residue consists of the eight-byte frame displacement
and the triple-projectile position/rotation schedule.

Third follow-up Function Match Plan: preserve the retained direction owner and
test typed pointer ownership already materialized by retail for the other two
constructor objects: `s4` is the effect-rotation address and `s0` is the world
position address. Use each pointer consistently in the existing direction and
triple-projectile statements, independently and together. Retail completes the
position-Y update before loading camera X and places the position argument
after that load; the direct-object probe hoists both. An owner hypothesis must
recover this order without changing a value, introducing a call, changing the
6684-byte extent, or disturbing the exact prefix, calls, referents and status
regions. Reject any wider alias or helper shape.

All four constructor-owner states compile identically at strict 99.694790%.
The compiler already derives `s4` and `s0` from the direct aggregate addresses,
so explicit rotation and position pointers change no instruction, frame slot,
call or referent. Reject them as redundant. The retained direction owner and
the current 216-byte frame remain the strongest evidenced source; no exact
claim or bank promotion is made. Instrumented matrices and owner-aware linked
residue are under `build/player-motion-owner/` and
`build/player-constructor-owners/`.

## Campaign verdict

All eight frozen functions received their own evidence snapshot and multiple
semantic source controls. `menu_draw_status_details` reached strict 100% and
is banked. `effect_map_collision` is one word short; `menu_draw_item_detail`
is two words short; `effect_projectile_update_2d`, `render_map_cell` and
`menu_draw_window_backdrop` retain unexplained stack-frame differences;
`map_interaction_dispatch` retains unsupported reload/lifetime differences;
and `player_update` retains an eight-byte frame difference after exact call,
referent, status and fade controls. Compiler/profile sweeps do not close any
of these seven residues. They remain partial because closing them would
require source facts the retail program does not establish.

## Entity rendering: disjoint pool-cursor lifetimes

Follow-up Function Match Plan: GAME `8001f218`, 1408 bytes / strict
99.517044%, unit `game.render_scene`. Fresh image-qualified address,
disassembly/CFG, callers, callees, strings and match views preserve the void
interface, sole `render_frame` caller, 61 blocks, 40 conditional branches,
nine proven calls, 25 validated address pairs, five typed pool traversals and
no strings or indirect control flow. The current source already preserves all
opcodes, immediates, branch destinations, calls and address identities; thirty
words differ only in GPR fields. This remains game visibility policy, with
`SetLightMatrix` retained as the verified Psy-Q boundary.

Retail reuses `s1` for the nonoverlapping map-object and actor cursors, keeps
the actor countdown in `s3`, and consequently retains wrapped Z/X origins in
`s6`/`s5` and the actor view-Z address in `s4`. The probe keeps both cursor
declarations at function scope and gives the actor cursor `s3`, its countdown
`s1`, the origins `s5`/`s4`, and the view address `s6`. Test the real lexical
lifetime already established by the sequential passes: scope the map-object
and actor cursors to their own passes, independently and together. Then test
all five typed cursors scoped to their disjoint traversals only if the focused
states preserve complete behavior. This changes no object, operation or
ordering and introduces no cast or carrier. Reject an identical result or any
change beyond GPR ownership; retain only a raw improvement that preserves the
1408-byte extent, CFG, calls and ordered referents.

All five states compile identically at 99.517044%, including the combined
scope around every cursor. The pinned compiler's liveness already ends each
cursor at its last use; lexical blocks do not recover retail's saved-register
assignment. Reject the no-effect source changes. The canonical 1408-byte body,
nine calls and 25 current address pairs remain unchanged and unbanked; results
are under `build/render-entity-cursor-scopes/`.

## Horizontal player movement: phase-local scalar ownership

Follow-up Function Match Plan: GAME `800171fc`, 2088 retail / 2128 probe
bytes, strict 96.568960%, within the fourteen-function `game.player_core`
unit. Fresh six-view evidence preserves the supported signed-word heading and
distance interface, three retail call sites, 100 blocks, 58 conditional
branches, seven proven calls, 42 current validated outgoing address pairs,
SDK `SVECTOR` collision delta, signed retry countdown and common result one.
The full earlier bearing, typed-owner, cursor/reread and recenter campaigns are
negative controls and will not be repeated. This is custom game movement;
exact Release 2.5 `rsin`/`rcos` remain external Psy-Q owners.

The source declares collision-bearing temporaries (`angle`, `radius`,
`delta`, `attempt`) and diagonal-recentering temporaries (`remainder_z`,
`remainder_x`, `half`, `type`) for the whole function, although their semantic
phases do not overlap. Retail holds the immutable heading in `s7`; the probe
first diverges by holding it in `s6`, before any call or control difference.
Test ordinary blocks around the collision retry and diagonal handling,
independently and together, with those existing locals declared at their first
phase. Preserve `dz`, `dx`, candidate coordinates and starting cells across
the phases exactly. A valid result must keep all operations, widths, CFG,
calls, referents, the retail 88-byte frame and all thirteen exact siblings;
reject byte-identical scopes or any semantic/code-size regression. Do not
reorder declarations or add register carriers.

All four phase-scope states compile byte-identically at 96.568960%, 2128
bytes and the retail 88-byte frame. GCC's live ranges already exclude the
collision and recenter locals outside their uses, so lexical ownership does
not change the heading register or any later address materialization. All
thirteen sibling functions remain strict 100%. Reject the no-effect scopes;
the production source and bank remain unchanged. Results are under
`build/player-move-phase-scopes/`.

## Map-object forward probe: coordinate and query lifetimes

Function Match Plan at `491870b4`: GAME `80030eb8`, 196 bytes / strict
93.755104%, unit `game.map_object_pool`. Hash-identical retail and all six
image-qualified semantic views were refreshed. The complete body retains a
32-byte frame, thirteen CFG blocks, one proven `collision_query_world` call,
one validated `map_object_state` HI16/LO16 pair, five validated internal jump
relocations and no strings or unresolved outgoing control flow. Both callers
in `map_object_pool_update` pass a halfword-derived yaw and compare the signed
word result with -1. The established word X/Z fields, byte behavior field,
cardinal adjustments of 2000, radius 3000, ignored-height sentinel 65535,
zero height and flags 0x21 remain unchanged. This door policy is game code;
no Psy-Q archive or vendored inventory identifies the body.

Retail captures X in `t0`, reads the definition behavior byte, and fills its
load-delay slot with Z in `t1`. It materializes radius 3000 in the type-two
guard delay slot and again in the first cardinal comparison delay slot, then
loads the height sentinel in the sole call's delay slot. The canonical probe
keeps X in `a3`, Z in `v1`, and result in `t0`; it materializes the sentinel
before the call and radius in the call delay slot. Earlier per-case calls,
direct owner lookup, outer/inner predicate rewrites and collision-helper
boundaries failed to recover this schedule.

Trace the canonical `definition`, `point_x`, `point_z`, `result` and `yaw`
source values through allocation and crossed calls, together with frame,
spill and cross-jump observations. Test bounded declaration/initialization
orders and scopes for the genuine captured coordinates and definition owner,
then test only query spellings that consume the same six established
arguments at the shared call. Preserve the current switch topology, single
static call, indeterminate unsupported-input paths, all 49 linked words and
all eight ordered relocations. Reject unused carriers, incompatible object
views, forced registers, volatile qualifiers, padding, inline assembly and
any candidate that adds a call or changes the admitted inputs.

All six states compile with native/instrumented byte parity. Moving the real
coordinate and definition declarations either reproduces the baseline or
worsens load order. Assigning one consumed `probe_radius` on each admitted
outer-switch path reaches strict 100% for both signed- and unsigned-word
spellings. The signed word is retained because it matches the established
`collision_query_world` radius parameter. It gives the radius pseudo three
references over a 27-unit lifetime in `a3`; X, Z and result consequently
allocate to `t0`, `a2` and `t1`, exactly as retail. This restores the complete
49-word instruction stream, including both radius delay slots and the call's
height-sentinel delay slot.

The focused unit compile reports all eight functions as identical. The probe
has the retail 32-byte frame and its eight physical relocations agree in order:
the `map_object_state` HI16/LO16 pair, the world-query call and five internal
jumps. The generated sources and summaries are under
`build/hypotheses/map-object-probe-instrumentation/`; retained traces are under
`build/gcc257/map-object-probe-instrumentation-traces/`. This function is now
strict exact and is eligible for banking after the required full verification.

Final verification passes the focused compile, complete 739-test / 9183-subtest
suite, Ruff and whitespace checks. The full build raises GAME to 341/362 exact
at 99.787% aggregate and the repository total to 448/471 exact. It reaches the
existing data ownership and section-placement failures with zero artifact
failures. `map_object_probe_forward` is banked at strict 100%.

## Statistics header: shared row-step value

Function Match Plan at `222f381b`: GAME `80025f38`, 1440 retail bytes / 1428
source bytes, strict 97.977776%, unit `game.menu_draw_stats_header`. Fresh
hash-verified evidence includes all six semantic views, the complete 360-word
retail body, three proven no-argument callers, 28 proven calls, 36 validated
data-address pairs, two internal jumps, nineteen CFG blocks, nine branches,
one return and no strings or indirect transfers. The established
`MenuGlyphString`, player fields, atlas owners and formatter/renderer APIs
remain unchanged. This is game statistics composition around existing game
renderers, not a vendored body.

Retail reserves 72 stack bytes and saves `s0` through `s5` plus `ra`; the
probe reserves 64 and saves only through `s4`. At the first class guard,
retail materializes the repeated row advance 23 in `s4`, uses that value in
every subsequent Y update, and consequently keeps the repeated X value 251
in `s5`. The probe folds every row advance as an immediate and keeps X in
`s4`. Earlier inline row/value helpers and compiler-profile controls changed
broader ownership or ordering without recovering this sequence.

Trace the current glyph-string and shared glyph-index lifetimes, frame and
allocator observations. Then test a genuine consumed row-step value at signed
word width, together with directly evidenced halfword-width controls, replacing
only the repeated Y increments. A retained source must preserve all coordinates,
glyphs, predicates, calls, referents and the complete CFG, and must recover the
retail frame and every one of its 360 linked words. Reject unconsumed constant
carriers, forced registers, volatile qualifiers, new helpers, padding and
changes to the established menu object views.

All six native/instrumented states compile with byte parity. A signed-word
row step used for every Y advance, only from the class row onward, or only
after the class row produces the same strict-exact object. The cohesive
all-row spelling is retained; the other two prove that retail does not
distinguish those source-use boundaries. Both halfword controls remain two
words short at 99.861115% and are rejected.

The retained row-step pseudo has six post-optimization references, crosses
fourteen calls and occupies `s4` over a 414-unit lifetime. Its value 23 is
materialized in the first class-guard delay slot. This moves X=251 to `s5`,
restores all seven saved-register slots and the 72-byte frame, and matches all
360 linked words. The complete 28-call and 36-data-reference sequences remain
equal to retail. Generated sources and summaries are under
`build/hypotheses/stats-header-row-step/`; retained traces are under
`build/gcc257/stats-header-row-step-traces/`.

## Item detail: load-delay initialization order

Follow-up Function Match Plan at `ad214ccb`: GAME `80027b7c`, 732 bytes /
strict 97.814210%, unit `game.menu_item_detail`. Hash-identical retail and all
six semantic views were refreshed. Six callers, seventeen proven calls,
twenty validated data-address pairs, seven CFG blocks, three branches, one
return, the 160-byte frame and three exact unit siblings remain unchanged.
The complete union workspace and ten-halfword name copy have direct access
evidence; this is game item/menu composition around established GTE and menu
APIs, not a vendored implementation.

Only two of 183 linked words differ. After loading the primitive-buffer owner,
retail initializes the loop count to zero in that load-delay slot, loads the
cursor, then initializes the union-workspace pointer in the cursor load-delay
slot. The probe schedules the workspace pointer in the first slot and the
count in the second. Forty-nine prior declaration, statement-order, pointer,
index and typed cursor controls either reproduce this pair or perturb later
loop words.

Trace the union workspace, source-name pointer, loop count and inner glyph
pointer through allocation and scheduling. Test the complete union-aware name
copy as one ordinary inline operation, with and without the already adjacent
primitive-buffer publication, plus directly evidenced signed/unsigned word
counter forms. Preserve the exact loop body, workspace offsets, all calls,
referents, frame and CFG. Reject emitted helper calls, incompatible workspace
views, fake dependencies, volatile state, dummy expressions and any candidate
that changes another linked word merely to swap the two delay-slot occupants.

All six instrumentation states compile. Moving the copy into an ordinary
inline operation, giving that operation ownership of row selection, or also
giving it primitive-buffer publication leaves the same two delay-slot words
unequal. A const-qualified source pointer is likewise identical. An unsigned
counter still leaves the pair reversed and changes retail's signed `slti` to
`sltiu`; its higher fuzzy score therefore contradicts the raw type evidence
and is rejected. The trace retains the same 160-byte frame, allocation of the
source, destination and counter pseudos, and cross-jump decisions. No source
change is kept. Results are under `build/hypotheses/item-detail-schedule/` and
retained traces under `build/gcc257/item-detail-schedule-traces/`.

## Effect collision: selector result topology

Third follow-up Function Match Plan: GAME `80037850` remains 1900 bytes with
474 of 475 complete linked words equal. The fresh prior evidence pass retains
six callers, one world-query call, twelve ordered data pairs, the complete map
geometry CFG and a four-state selector masked by three. The sole difference is
still retail `nop` versus the probe's redundant `li v0,1` in the class-three
selector branch delay slot. Existing scalar-width, selector-local, result
initialization, explicit-default and compiler-profile controls are negative
evidence.

Test source control-flow forms whose defined value follows directly from the
four proved selector states: a complete `if`/`else if` chain, a zero-first
guard followed by the three query cases, and a query-only switch after a
defined zero-state guard. Each form must return one for selector zero and use
the same three calls and flag constants for selectors one through three.
Trace the selector and result pseudos plus cross-jump and frame decisions.
Preserve every geometry path, call argument, referent and linked word before
the selector. Reject any form that changes selector semantics, adds a call or
merely suppresses the assignment through undefined behavior.

Fourteen unique selector/result states compile with native and instrumented
parity. Direct masked fallback expressions reload the effect byte and add
three words. A named unsigned selector changes the existing signed `slti` and
duplicates it in both fallback slots; signed-word naming keeps the first
`slti` but still duplicates it at `+704` and `+718`. Preinitializing the real
result from the signed comparison grows the body and changes earlier shared
exit scheduling. None removes the sole word without perturbing other retail
instructions. The current explicit fallback remains the clearest fully
defined source and is unchanged. Results are under
`build/hypotheses/effect-selector-result{-2}/`; retained traces are under the
matching `build/gcc257/` directories. The `+718` delay slot remains an
unattributed residue and the function remains unbanked.

## Item-name frame: table owner and local extent

Function Match Plan: GAME `800292f8`, 1976 bytes / strict 99.570850%, unit
`game.menu_runtime`. Hash-identical retail and all six fresh semantic views
confirm five calls from `item_pickup_confirm`, 23 proven outgoing calls, 112
validated address pairs, three CFG blocks, one loop branch, one return and no
strings or indirect control flow. The preview transform, ten-halfword name
copy, four direct pickup-window packets and four persistent background quads
are game presentation around established SDK calls; the function is absent
from the vendored inventory. All fifteen siblings are comparison controls.

Raw comparison isolates two residues. Retail reserves 224 stack bytes while
the probe reserves 160; the real glyph string and three MATRIX locals retain
the same offsets, leaving an untouched 64-byte interval before saved
registers. Separately, retail materializes `item_name_rows` between the two
shifts that form the twenty-byte row stride, while the probe completes both
shifts first. Test an explicit, typed owner pointer to the complete row table
before selecting the const row, with direct-row and codes-pointer consumers.
Trace the owner, row/name, loop counter, four real stack objects, frame and
allocator decisions. Preserve the frame's real object offsets, all packet
stores, calls, referents and exact siblings. Reject an unused 64-byte object,
padding, volatile storage, forced registers or a helper that changes the
packet body merely to manufacture the retail frame.

All six table-owner states compile with native/instrumented byte parity. Every
explicit owner form restores the retail split shift/address schedule and is
otherwise identical. Keep the const owner and const selected row: it states
the complete table identity and read-only use without changing the loop. The
function rises to 99.975710%; all 23 calls, 112 ordered data pairs, packet
stores and fifteen sibling functions remain unchanged. Raw focused comparison
now reports only twelve prologue/epilogue words for the 224-byte retail versus
160-byte probe frame. None of the four real local objects occupies the silent
64-byte tail, so no inferred object is added and the function remains
unbanked. Results are under `build/hypotheses/item-name-frame-owner/` and
traces under `build/gcc257/item-name-frame-owner-traces/`.

## Status panel: scalar lifetimes and silent frame tail

Function Match Plan: GAME `8002430c`, 1692 bytes / strict 99.962170%, unit
`game.menu_status_panel`. Fresh validated evidence confirms the sole
`menu_root` caller, twenty proven calls, 106 validated address pairs, nine CFG
blocks, five branches, one return and no strings or indirect transfers. The
four reflected packet operations and input-release loop are game menu policy;
SDK packet helpers remain callees rather than candidate body ownership.

All 423 linked body words already agree except sixteen prologue/epilogue words:
retail reserves 112 bytes and the probe 48, placing seven saved registers and
`ra` 64 bytes apart. There are no accessed automatic objects in the interval.
The earlier current/plain/no-schedule, debug and GCC 2.6 profile controls do
not recover it. Trace the real frame counter, pad result, texture-page pointer,
frame, spills and inline decisions. Test directly supported signed/unsigned
pad widths, const qualification, mature-frame input scope and loop-owned
texture selection. Preserve the complete packet body, calls, references and
CFG. Reject any unconsumed 64-byte local, dead operation, padding, volatile
carrier or SDK object with stack traffic absent from retail.

Seven native states compile. Unsigned pad input, initialized/scoped input and
loop-scoped texture ownership are byte-identical to the canonical result;
unsigned frame changes retail's signed comparison, while direct texture-page
arguments disrupt address reuse. Every state retains the 48-byte probe frame.
The instrumented compiler reproducibly faults while tracing the unchanged
baseline, both with the requested source lifetimes and with a reduced
frame/allocation request; the production compiler remains stable. Native
results are under `build/hypotheses/menu-status-lifetimes-native/`, with both
failed trace runs retained under `build/gcc257/`. No source change or bank is
made. The silent 64-byte tail remains unsupported by an accessed source object.

## Dialogue page: quotient, remainder and directory lifetimes

Function Match Plan: GAME `8002c9d4`, 164 bytes / strict 98.780490%, unit
`game.save_system`. Fresh retail validation and all six semantic views confirm
four map-event callers, one proven display call, seven validated references to
the mutable TALK path, six CFG blocks, three checked-division branches, two
traps, a 24-byte frame, one return and no strings or indirect transfers. The
floor/stage/character/page formatter is game dialogue policy and has no
vendored body match; all 23 unit siblings are controls.

Retail assigns quotient, remainder and the path+6 directory cursor to `a2`,
`v0` and `a0`; the probe assigns them to `a2`, `v1` and `v0`. Every operation,
store, call, referent and branch otherwise agrees. Earlier direct-byte,
advancing-cursor and inline-helper controls either reproduce this allocation
or disturb the address sequence. Trace the three real values through local and
global allocation. Test their declaration/initialization order, placing the
directory owner before the signed divmod, and direct versus separately assigned
quotient/remainder locals. Preserve signed division by ten, all seven byte
stores, the path+6 cursor and call-delay subtraction. Reject casts, forced
registers, volatile carriers, duplicate division or altered path ownership.

All seven lifetime-order states are byte-identical. The trace assigns the
directory cursor three references over ten allocation units in `v0`; source
declaration order is gone before allocation, while quotient/remainder source
names have already folded into the checked division. This rules out cosmetic
ordering as the cause.

Follow-up Function Match Plan: retail derives the complete path in the display
call's delay slot by subtracting six from the still-live directory cursor. Test
that exact real value chain as the call argument: `directory_character - 6`,
its indexed equivalent, and a named complete-path pointer assigned only after
both directory writes. Keep the same path object and seven stores; the pointer
must still be path+6 while writing both character digits. Trace whether this
consumer assigns the cursor directly to outgoing `a0` and consequently leaves
the remainder in `v0`. Reject a new address pair, moved store or empty call
delay slot.

The three directory-consumer spellings also compile identically to baseline:
GCC already recognizes the complete-path argument as the directory cursor
minus six, regardless of whether the source spells the global array, pointer
subtraction, indexing or a named complete-path value. The allocation remains
`a2`/`v1`/`v0`, so none recovers retail `a2`/`v0`/`a0`. All ten states preserve
the 164-byte body, 24-byte frame, one call, seven ordered address pairs and 23
sibling bodies. Results and trace are under
`build/hypotheses/talk-dialogue-{lifetimes-rerun,consumer}/` and
`build/gcc257/talk-dialogue-*-traces/`. No source change or bank is made; the
register assignment remains unattributed.

## Map screen image: floor-load and directory publication

Function Match Plan: GAME `80034d54`, 144 retail / 148 probe bytes, strict
88.888885%, unit `game.map_scripts`. Fresh six-view evidence confirms the sole
interaction-dispatch caller, one proven display call, five validated path/state
address pairs, six checked-division blocks with three branches/two traps, a
24-byte frame, one return and no strings or indirect transfers. The custom map
path formatter is game policy; all other unit functions are controls.

Retail stores the group digit, loads the current-floor byte, then materializes
the path+5 directory cursor in `a0`; quotient and remainder occupy `a1` and
`v1`. The probe materializes the cursor before the floor load, requires one
extra load-delay `nop`, and allocates the cursor/remainder to `v1`/`a2`.
Earlier cursor, floor snapshot and inline-helper controls preserved one side of
this residue while losing the other. Trace the floor digit, directory cursor,
quotient and remainder. Test a floor-digit value whose load precedes cursor
publication, late pointer assignment, and the real cursor-minus-five display
consumer, independently and together. Preserve all four byte stores, signed
division, five referents and the call delay slot. Reject duplicate loads,
forced registers, volatile carriers, a second path owner or moved stores.

All eight states compile with native/instrumented parity. Named quotient and
remainder locals and the explicit directory-minus-five display consumer are
identical to the 88.888885% baseline. Every form that loads a floor value
before publishing the directory pointer returns to the older 79.250000%
shape: it anchors addressing at path+8, loses the cursor-based floor store and
still does not match retail's allocation. The trace therefore confirms the
load/publication tradeoff but supplies no source form that has both retail
properties. All calls, byte destinations and unit controls remain intact. No
source change or bank is made; results and retained traces are under
`build/hypotheses/map-screen-image-lifetimes/` and
`build/gcc257/map-screen-image-lifetimes-traces/`.

## Effect dispatcher: entry value ownership

Follow-up Function Match Plan: GAME `80038a38`, 6156 bytes / strict
99.827810%, unit `game.effect_dispatch`. Hash-identical retail and all six
fresh semantic views confirm the sole `effect_pool_sweep` caller, 69 direct
calls, 22 validated address pairs, 257 CFG blocks, 135 conditional branches,
36 return frontiers, a 168-byte frame and 49 candidate switch-table rows. The
complete effect-kind policies remain game code around separately identified
SDK and game helpers; neither the body nor its dense dispatch topology is a
vendored library candidate.

All 1539 linked instruction positions already have matching non-register
bits. The 42 unequal words are confined to value ownership: retail keeps the
entry magic record in `s3` and kind in `s6`, while the probe reverses those
registers; ground parent and ordinary radial radius use retail `s5` versus
probe `s3`; and the ordinary radial scale sequence exchanges `v0`/`v1` while
retaining the same operations and order. Calls, referents, constants, branch
targets, table destinations and delay-slot contents otherwise agree. The
earlier value-lifetime campaign established the current complete semantic
frontier and rules out changing those recovered operations merely for score.

Trace the entry effect, magic, kind, phase and collision-radius pseudos through
allocation. Test only their real declaration and initialization boundaries:
load kind before publishing magic, defer magic assignment until after the
effect fields, and move phase or the default radius on either side of magic.
Preserve the exact effect pointer, field widths, switch, 69-call sequence, 22
referents, CFG and every non-register instruction bit. Reject forced registers,
volatile carriers, duplicated loads, artificial scopes, fake values and any
state that perturbs an established operation solely to exchange hard-register
numbers.

All ten states compile with native/instrumented parity and emit the same 6156
function bytes at 99.827810%. Effect, magic, kind, phase and radius retain
`s4`/`s6`/`s3`/`s2`/`s5`; their respective global-allocation observations are
364/73/1527, 14/22/420, 11/11/256, 19/10/277 and 5/14/182 for
references/calls-crossed/live-length. Moving phase before magic changes only
the trace: magic shortens by one unit and phase lengthens by one, while the
resolved instruction stream, 69 calls and 22 referents remain byte-identical.
Initialization spelling therefore does not account for retail's allocation.
No source change or bank is made. Results are under
`build/hypotheses/effect-dispatch-entry-lifetimes/`; the two distinct internal
states are retained under
`build/gcc257/effect-dispatch-entry-lifetimes-traces/`. The remaining 42
register-field differences stay unattributed.

## Experience progression: growth-path join

Follow-up Function Match Plan: GAME `80016058`, 548 retail / 528 probe bytes,
strict 88.824814%, unit `game.player_death`. Hash-identical retail and the six
fresh semantic views confirm the sole `actor_apply_damage` caller, three
ordered calls, twelve validated retail data pairs, eighteen CFG blocks, nine
branches, one internal jump, one return, a 32-byte frame and no strings or
indirect transfers. The caller loads an unsigned definition halfword and the
callee sign-extends its `s16` parameter. Level progression, growth and caps are
game policy; notification and sound remain separate provider calls.

Retail keeps the progress and vitals anchors in `s0`/`s1`, branches from the
level-40 test to the indexed-row path with a `nop` delay slot, and spells each
of eight terminal-row loads with its own address pair before joining the two
growth paths. The probe preserves the player anchors but schedules the indexed
row shift into that branch slot and hoists terminal row 39 into `s2`; the extra
save/restore and seven removed address materializations account for the exact
20-byte size difference. Earlier outer-loop, player/subobject pointer, inline
helper, maximum-level exit and branch-orientation controls did not preserve
both properties together.

Test the directly decoded inner join without changing the structured outer
loop: an explicit jump to the indexed-row label, an extrapolation fallthrough
with a labelled common join, and the reversed labelled layout as a control.
Trace the real level and indexed growth-row values, frame, cross-jump and spill
state. Preserve signed experience arithmetic, the duplicated byte level load,
all growth widths and order, four caps, three calls, twelve retail referents
and twelve exact sibling functions. Reject detached data identities, volatile
access, raw addresses, invented locals or a control-flow form whose branch
destinations no longer match retail.

The current, explicit table-jump and extrapolation-fallthrough forms compile
to the same resolved 528-byte body at 88.824814%, with three calls, five probe
address pairs and 120 unequal aligned words. Their level pseudo remains six
references over six allocation units in `a0`; the indexed growth row remains
twelve references over fourteen units in `a0`. Cross-jump decisions, the
32-byte frame and the extra saved `s2` are identical. Reversing the labelled
arms keeps those traced observations but falls to 61.576640% because its
branch layout disagrees with retail, reproducing the earlier orientation
control.

Independent native compilation confirms that the first three resolved bodies
are byte-identical, all twelve sibling functions remain raw-exact, and no
variant restores the missing seven terminal-row address materializations. No
source change or bank is made. Results are under
`build/hypotheses/experience-growth-join/`, the retained trace under
`build/gcc257/experience-growth-join-traces/`, and the independent raw audit
under `build/experience-growth-join-audit/`.

## Map-object effect spawn: sequence assignment result

Follow-up Function Match Plan: GAME `80031834`, 404 retail / 400 probe bytes,
strict 94.504950%, unit `game.map_object`. Fresh six-view evidence confirms the
sole actor-action caller, five ordered calls, two validated sequence-address
pairs, sixteen CFG blocks including signed-division guards, ten branches,
three internal jumps, one return, a 40-byte frame and no strings or indirect
transfers. The two caller paths supply byte kind/ID, a word position and signed
Y offset. The custom group-170/180 selection and object initialization are game
policy; `rand` remains a separately identified Sony provider. All seven unit
siblings are exact controls.

Retail allocates sequence/object/position/ID/Y-offset to
`s0`/`s1`/`s2`/`s3`/`s4`. It reloads the halfword sequence after acquisition,
copies the returned object in that load delay, increments and publishes the
sequence, then stores the old halfword into the object. The probe reuses `s0`
for sequence then object, inserts a load-delay `nop`, and shifts position/Y to
`s1`/`s2`; this removes one saved-register pair and four body bytes. Earlier
branch-local acquisition, declaration order and inline initializer boundaries
either reproduce that reuse or reverse sequence/object while adding a nop.

Test the real result of `(*sequence)++` as a named `u16`, then split its
load, incremented halfword and two stores without changing their defined
values. Trace sequence, acquired object, old value, next value and the three
input lifetimes together with frame and spill state. Preserve both sequence
owners, all initialization stores, signed division, action thresholds, five
calls, two referents and exact siblings. Reject retained values across the
acquisition call, widened counters, duplicate loads, volatile carriers,
forced registers or any state that changes postfix wraparound semantics.

All five states compile with native/instrumented parity and produce the same
resolved 400-byte body at 94.504950%, with five calls, both sequence referents
and 54 unequal aligned words. Naming the postfix result is optimized away;
the split forms expose a real old-value pseudo in `a0` for three references
over five units, and the two-result form exposes the increment in `v1` for two
references over two units. None changes sequence/object coalescing in `s0`,
the `s1`/`s2` input allocation, the load-delay `nop`, or the saved-register
set. The named states therefore alter trace provenance without altering code.

Independent native compilation confirms byte identity across all five
resolved bodies and raw-exactness for all seven sibling functions. No source
change or bank is made. Results are under
`build/hypotheses/map-object-sequence-result/`, distinct internal states under
`build/gcc257/map-object-sequence-result-traces/`, and the raw audit under
`build/map-object-sequence-result-audit/`.

## World-state persistence: inverse sparse predicate

Follow-up Function Match Plan: GAME `80035b5c`, 696 retail / 700 probe bytes,
strict 97.528730%, unit `game.map_events`. Hash-identical retail and the fresh
six-view pass confirm three no-argument callers, zero calls and strings, seven
validated address pairs plus one internal jump, 28 CFG blocks, seventeen
branches and one return. The game-owned serializer writes the established
floor/event/actor/object record layout; all five sibling functions remain
exact controls. Its first 356 bytes are exact after the retained address and
link-countdown corrections.

Retail's remaining sparse-object predicate branches to serialization only
when the behavior differs from all five excluded values; the probe spells the
equivalent positive disjunction followed by the idle-action test. Test the
direct inverse conjunction, with an explicit serialization join, and a switch
case form preserving the decoded comparison order. Trace the real behavior,
object, definition-table, index, active-count, link cursor and countdown
values together with frame, spill and cross-jump state. Preserve the unsigned
loads, five constants in order, conditional action load, output stream, zero
calls, seven referents and 28/17 CFG. Reject duplicated field reads, volatile
or forced carriers, changed constants, reordered comparisons, or a form that
adds a Boolean result solely to steer allocation. Require strict 100% before
retaining source or banking; a negative batch leaves canonical source intact.

The inverse-conjunction form compiles to the same complete 700-byte resolved
body and the same traced frontier as canonical at 97.528730%. Behavior is
eliminated as a named allocation quantity; object/definitions/index/active/
link/countdown retain registers `a2`/`t8`/`a3`/`t1`/`a1`/`a0` with respective
global lifetimes 19/45, 3/70, 46/140, 12/61, 11/9 and 11/8
(references/live units). Both forms have one accepted cross-jump and no spill
search, so predicate polarity does not explain the constant placement.

The ordered switch control reaches the retail 696-byte extent but lowers the
sparse values into range tests and an internal jump. It scores 91.258620%,
changes the decoded comparison CFG, and shifts the definition owner to `t4`;
reject it despite having only 22 unequal raw aligned words. Independent native
compilation confirms zero calls, the same seven numeric referents, byte
identity of canonical and inverse forms, and raw exactness of all five sibling
functions. No source change or bank is made. Results are under
`build/hypotheses/world-persist-inverse-predicate/`, retained traces under
`build/gcc257/world-persist-inverse-predicate-traces/`, and the raw audit under
`build/world-persist-inverse-predicate-audit/`. The final constant placement
remains an unattributed residue after the supported predicate families.

Second predicate follow-up Function Match Plan: the retail chain groups the
first four excluded behaviors at one action-check label, then materializes the
fifth value (`0x41`) in the fourth comparison's delay slot and branches around
the action check when it differs. The previous inverse conjunction kept all
five values in one optimizer-visible expression and hoisted the fifth value.
Test a source-level four-value group followed by a separate gold-pickup test,
both as structured nesting and as explicit action/serialization labels. Keep
the same comparison order, single behavior load, conditional action load and
link-copy loop. Trace behavior, object, active count, link cursor, countdown,
frame and cross-jumps. Reject duplicated action reads or any changed CFG,
constant, call, referent, field width or sibling body; retain only strict
100%.

The structured four-value group and explicit labels canonicalize to the same
700-byte body, trace frontier and 83 unequal aligned words as the original.
The positive-arm split grows to 708 bytes at 94.505745%, duplicates the action
path and changes allocation from the function entry. All four variants keep
zero calls, seven ordered referents and five exact siblings, with independent
native/instrumented parity. Reject the split forms, leave
`src/game/map_events.c` unchanged and do not bank the function. Results and
traces are under `build/hypotheses/world-persist-split-final-predicate/` and
`build/gcc257/world-persist-split-final-predicate-traces/`; the independent
linked audit is `build/world-persist-split-final-predicate-audit.json`.

## Map screen image: early directory owner with named floor

Second schedule follow-up Function Match Plan: GAME `80034d54`, 144 retail /
148 probe bytes, strict 88.888885%, unit `game.map_scripts`. The prior eight
states establish that assigning the directory pointer only after reading the
floor makes GCC retain the earlier path+8 address and use a direct path+5
store, while the canonical initialized pointer is published before the floor
load. Neither tests an early, real directory owner together with a named floor
value assigned after the group store.

Test that missing composition with the floor represented as `u8`, its enum
type and the completed digit value. Vary only declaration order and whether
the conversion occurs at definition or store. Trace the floor, directory,
quotient and remainder lifetimes, frame and spills. Preserve the division and
trap sequence, group store before the floor load, cursor-based floor store,
two digit stores, five ordered referents, one call and exact siblings. Reject
duplicate loads, reordered stores, a second path owner, volatile or forced
storage. Retain and bank only strict 100%.

All five named-floor forms compile to the canonical 148-byte body at
88.888885%. Their source lifetimes differ internally, but none changes the
nineteen unequal aligned words or the five-address referent order: the
directory stays in `v1`, the remainder in `a2`, and the floor load retains its
extra delay `nop`. Independent native/instrumented builds agree, the one call
is unchanged, fourteen exact siblings remain exact and the other non-exact
unit sibling is byte-identical to production. Reject the forms, leave
`src/game/map_scripts.c` unchanged and do not bank the function. Results and
traces are under `build/hypotheses/map-screen-image-early-directory-floor/`
and `build/gcc257/map-screen-image-early-directory-floor-traces/`; the
corrected linked audit is
`build/map-screen-image-early-directory-floor-audit-2.json`.

## Player update: camera-rotation owner at triple spawn

Fourth weapon-spawn follow-up Function Match Plan: GAME `80018880`, 6684
bytes, strict 99.694790%, unit `game.player_update`. The retained launch
direction owner preserves 66 ordered calls, 210 ordered referents, the exact
prefix and both exact status regions. Apart from the eight-byte unexplained
frame shift, the remaining operation-order residue is the triple-projectile
setup at `+fa0..+fd0`. Retail updates and stores position Y before loading
camera pitch, then publishes the position argument; the probe hoists both the
pitch load and position argument. Both already use the materialized
`player_state.camera_rotation` address held across the first constructor.

Test an ordinary `const SVECTOR *` owner for that complete camera rotation,
using it in the first constructor and/or the three triple-projectile fields.
Also test a named pitch snapshot assigned only after the position-Y update.
Trace camera rotation, pitch, effect rotation, position and direction
lifetimes with frame, spills and cross-jumps. Preserve every value, call,
referent, stack argument, the 6684-byte extent, exact prefix and exact status
regions. Reject wider aliases, copied aggregates, volatile or forced storage,
or any form that changes operations outside the triple setup. Retain only
strict 100%.

All five owner/snapshot forms compile to the canonical 6684-byte body at
99.694790%. Camera rotation, pitch and triple-position source names produce
several distinct trace frontiers but the allocator and scheduler converge on
the same 216-byte frame and all 1671 linked positions; 1608 words agree and
63 differ. Every state preserves 66 calls and all 210 ordered referents with
native/instrumented parity. Reject the redundant owners, leave
`src/game/player_update.c` unchanged and do not bank the function. Results
and traces are under `build/hypotheses/player-camera-rotation-owner/` and
`build/gcc257/player-camera-rotation-owner-traces/`; the independent linked
audit is `build/player-camera-rotation-owner-audit.json`. The eight-byte frame
and triple-setup schedule remain unattributed.

## Item model preview: complete name-table owner

Follow-up Function Match Plan: GAME `800279c4`, 440 bytes / strict
98.181816%, one-function unit `game.menu_item_model_preview`. Hash-identical
retail and the fresh six views confirm seven direct callers, ten proven calls,
eight validated address pairs, five CFG blocks, two branches, one return, a
152-byte frame and no strings or indirect transfers. Callers pass byte-derived
or word item IDs and consume no result. The game-owned preview uses authentic
SDK `SVECTOR`/`MATRIX` interfaces around separately identified GTE providers;
the complete eight-byte rotation object and 80-by-20-byte name table retain
their established owners.

Only three linked words differ: after forming `item_id * 5`, retail
materializes `item_name_rows` before the final shift by two, while the probe
finishes `item_id * 20` first. Earlier row, direct-index, early-selection,
cursor, loop and inline-boundary controls left this order unchanged. The
related `menu_draw_item_name_frame` has since established a new supported
source fact: a named pointer to the complete row table restores this exact
split-shift/address schedule while preserving the same typed row consumer.

Test that complete table owner here, with const/mutable owner controls and
complete-row versus codes-pointer consumers. Trace the table owner, selected
row/name, signed index, glyph workspace and item input, along with frame,
spills and cross-jump state. Preserve the 255 guard, ten halfword copies,
three SDK matrices, rotation update, all calls/referents, CFG and delay slots.
Reject an incompatible table view, raw byte arithmetic, duplicated read,
changed copy traversal or any source carrier without a real table consumer.
Require all 110 linked words and ordered references for strict closure.

All four complete-table-owner states are strict exact at 440 bytes with
native/instrumented whole-object parity. Const qualification and choosing a
row versus its codes member are erased; the retained const owner/const row is
the shared typed model already used by the related pickup preview. The new
`rows` quantity has two references over six allocation units in `v1`; the
item/index remain `s2` and `a2`, with the retail 152-byte frame and saved
register set. This ownership boundary places the `item_name_rows` HI16/LO16
pair between the multiply's two shifts and restores precisely the three
previously unequal words.

The focused rebuild and strict match reproduce all 110 retail words, ten calls
and eight address pairs. Ruff, whitespace checks and all 759 repository tests
pass. The full three-image build raises GAME to 344/362 exact and reaches only
the existing data ownership/placement and target-relink gates, with zero
artifact failures; PSX remains 1/1 and OPEN 107/108. The exact function is
banked. Results are under `build/hypotheses/menu-preview-table-owner/` and
retained traces under `build/gcc257/menu-preview-table-owner-traces/`.

## Item database: typed table and path-subrange owners

Follow-up Function Match Plan: GAME `80020cfc`, 1500 bytes / strict
99.746666%, unit `game.item`. Hash-identical retail and the fresh six-view pass
confirm the sole initialization caller, five proven calls, ten validated
address pairs, six internal jumps, two literal strings, a 40-byte path in an
80-byte frame and eighty model-file searches. The six resource-bank copies and
conditional sector rounding are game policy around separately identified SDK,
CD and runtime providers; this function is not a vendored library candidate.
All five contiguous unit siblings are strict-exact controls.

All 375 linked positions have matching non-register bits. The sixteen unequal
words begin at `800211b8`: retail assigns the directory divisor/quotient,
one-based file number and hundreds divisor/quotient to `a2`, `v0` and `v1`,
while the probe assigns them to `v1`, `a2` and `v0`/`a2`. The remainder and
tens division, store order, constants, signed division guards, delay slots,
calls and referents agree. Earlier arithmetic ordering, digit and input widths,
inline boundaries and loop-index widths did not retain retail's full schedule.

Test only real ownership boundaries not covered by those trials: name the
complete typed `cd_file_table` owner used by search and rounding, and name the
directory byte plus the contiguous three-byte numeric path subrange. Initialize
each owner either before the loop or at its first per-iteration use, with a
combined state as a composition control. Trace the table/path owners, index,
one-based number and remainder along with frame, spill and cross-jump state.
Preserve the 40-byte array, all four byte destinations, signed arithmetic,
five calls, ten ordered referents, six jumps and all exact siblings. Reject raw
offset views, altered traversal, duplicated loads, volatile or forced carriers,
and any state that changes a non-register instruction bit merely to improve the
score. Require strict 100% before retaining source or banking.

The per-iteration path-subrange state is byte-identical to canonical at
1500 bytes and 99.746666%. Instrumentation shows both path pointers are
eliminated before allocation; index and file number retain `s0` and `a2` with
13/37 and 6/3 references/live units. Moving those pointers outside the loop
makes them call-crossing values in `s4`/`s3`, expands the frame to 88 bytes and
the body to 1524 bytes, and falls to 97.976000% while preserving ordered
referents. This does not describe retail's frame or saved-register set.

Both complete-table initialization positions emit the same 1476-byte body at
97.984000%. The compiler collapses the complete owner into one base-relative
traversal, removes `s2`, and loses retail's independent `cd_file_table` address
pair; combining it with path owners cannot restore that topology. The
outside-loop composition additionally expands the frame and scores
97.328000%. All states retain the five calls, and all five sibling functions
remain raw-exact; only canonical and the eliminated per-iteration path owners
retain all ten ordered referents. Native/instrumented whole-object parity holds
for every state. Reject all owner forms, leave `src/game/item.c` unchanged and
do not bank the function. Results are under
`build/hypotheses/item-database-owners/`, traces under
`build/gcc257/item-database-owner-traces/`, and the independent linked audit is
`build/item-database-owner-audit.json`. The sixteen register-field differences
remain unattributed.

## OPEN ending scroll: monotone lighting-phase update

Follow-up Function Match Plan: OPEN `80014e28`, 1944 bytes / strict
99.917694%, unit `open.opening_scenes`. Hash-identical retail and the fresh
six-view pass confirm the sole `opening_run` caller, forty proven calls,
31 validated address pairs, eight internal jumps, no strings or indirect
transfers, a 264-byte frame and six strict-exact siblings. This controller's
camera, entity, lighting, audio and panel policy is game-owned around external
SDK providers.

All linked words, calls and referents agree except seven instructions in the
lighting and sequence dispatch at offsets `398` through `418`. Retail copies
the spilled lighting selector into `v0`, introduces one in `t0`, uses that
same register for the case-one comparison and first phase transition, and
then reuses one for the sequence dispatch. The probe instead puts the selector
and comparison one in `v1`/`v0`, while separately materializing the transition
values in `t0`. Earlier controls exhausted selector widths/promotions, case
orders, explicit terminal cases, early exits, joined tails, next-phase
carriers, scroll/sequence predicates and color-pointer consumer roles.

The actual lighting states form the contiguous monotone sequence zero, one,
two, and each selected transition advances exactly once. Test prefix increment
as that real phase update in the first transition, the second transition and
both, plus an explicit typed encode/add/decode form. Trace lighting phase,
lighting blend, frame, spills, cross-jumps and the constant-one definitions.
Preserve the switch and its 49-block/26-branch CFG, both blend guards and
increments, forty calls, 31 ordered referents, the 264-byte frame and all exact
siblings. Reject changed phase semantics, a synthetic carrier, duplicated
reads, volatile or forced storage, or any composition with the already rejected
scroll-state forms. Retain and bank only strict 100%; otherwise record the
first divergence and leave the production source unchanged.

All four update forms are negative. Incrementing only either transition grows
the body to 1960 bytes and scores 98.847740%, with 251 unequal aligned words
beginning at `+398`. The phase lifetime grows from seven to nine weighted
references; its selected increment survives as promoted short arithmetic, so
the corresponding literal transition value disappears rather than joining the
comparison-one definition. The first-transition form also adds a ninth internal
jump. Both retain 26 conditional branches, while neither preserves the retail
dispatch sequence.

Incrementing both transitions produces 1952 bytes at 96.868310%, with 258
unequal words beginning at `+2a0`. The phase grows to eleven references and is
allocated to `s7`, changing initialization and the persistent scene-state
assignment before the dispatch. The explicit encode/add/decode spelling emits
the same complete resolved body. Each state preserves forty calls, 31 ordered
referents, the 264-byte frame and all six exact siblings, and independent
native/instrumented compiles agree. Reject all increment forms, leave
`src/open/opening_scenes.c` unchanged and do not bank the function. Results are
under `build/hypotheses/open-ending-lighting-phase-increment/`, traces under
`build/gcc257/open-ending-lighting-phase-increment-traces/`, and the independent
linked audit is `build/open-ending-lighting-phase-audit.json`. The original
seven register/jump differences remain an unattributed code-generation residue.

## Map interaction: prescan terminal predicate

Third follow-up Function Match Plan: GAME `80034de4`, 2308 retail / 2296 probe
bytes, strict 99.202774%, unit `game.map_scripts`. The refreshed linked-word
audit confirms the established sole caller, 54 ordered calls, thirteen ordered
address pairs, 72-byte frame and fourteen strict-exact siblings. The function
remains game interaction policy around separately identified trigonometric
providers. All opcode insertions and deletions are now confined to three
missing instructions: the hinged-container prescan's local minus-one value and
two paired-door behavior reloads. Other unequal words are saved-register roles
or the resulting twelve-byte address shift.

Prior batches exhausted declaration scopes, position aliases, shared sentinel
locals, direct definition expressions, operation helpers, selection-loop
separation, labels and compound prescan guards. Test the still untried source
representations of the real prescan terminal decision: a named pre-decrement
result, split decrement followed by equality, a one-case switch, and the
equivalent post-decrement zero test. The last form is a semantic control: with
an initial count of three it rejects on the same fourth empty observation.
Trace the position, counter and terminal-result lifetimes, the minus-one/zero
definitions, frame, spills and cross-jumps. Preserve the sampled item byte,
signed-halfword countdown, exact failure path, 54 calls, thirteen referents and
all sibling bodies. Reject any form that changes the observed decrement and
sign-extension instruction sequence, and do not introduce volatile storage,
forced registers, an incrementing item cursor or an artificial sentinel owner.
Retain and bank only a strict 100% result.

The named result, split decrement and one-case switch all collapse to the
canonical 2296-byte body at 99.202774%. The named result has no allocated
pseudo; all three retain the same two signed-halfword counter allocations,
place `position` in `s8`, reuse the outer minus-one value in `s7`, and preserve
the three missing-opcode spans. The equivalent post-decrement zero test emits
2288 bytes at 98.639510%: it increases the prescan counter to nineteen weighted
references but removes one move and one sign-extension instruction, directly
contradicting retail.

All five states keep the 72-byte frame, 54 calls, thirteen ordered referents and
fourteen exact siblings. Independent native and instrumented linked-code
comparison agrees for every state. Reject all four alternatives, leave
`src/game/map_scripts.c` unchanged and do not bank the function. Results are
under `build/hypotheses/interaction-prescan-terminal/`, retained traces under
`build/gcc257/interaction-prescan-terminal-traces/`, and the independent audit
under `build/interaction-prescan-terminal-audit/`. The local minus-one value,
two definition reloads and reversed `position`/sentinel roles remain an
unattributed residue after the supported source forms.

## 2D projectile: frame identifiability follow-up

Follow-up Function Match Plan: GAME `80038298`, 608 bytes, strict 99.934210%,
unit `game.effect_update`. The refreshed six-view and complete linked audits
retain the sole dispatcher caller, nine calls, five validated address pairs,
three internal jumps, 29 CFG blocks, fourteen branches, one return, no strings
and seven exact siblings. Every non-frame instruction already agrees. Retail
uses a 120-byte frame and saves `s0`/`s1`/`s2`/`ra` at offsets 104 through 116;
the probe uses 56 bytes and offsets 40 through 52. Neither body accesses the
intervening 64 bytes.

The JSON/instrumentation batch already tests the remaining identifiable source
facts: byte phase ownership, complete motion and distance helpers, VECTOR
distance storage, and motion-path scopes for magic and collision. Those forms
either reproduce the 56-byte body or add observable stack traffic absent from
retail. It also establishes that three mutually incompatible unaccessed
declarations—64 bytes, two matrices and sixteen words—each manufacture strict
100% solely by shifting the save area. The compiler trace is identical for all
three: `record`/`magic` remain in `s0`/`s1`, collision remains call-free in
`a0`, the phase local folds away and there are no spills.

Because the linked bytes cannot distinguish those exact declarations and no
instruction consumes any candidate object, there is no further admissible
source hypothesis to compile. Adding any of them would violate the recovered
source model's ban on unused storage and would turn an arbitrary layout into a
false claim. Leave `src/game/effect_update.c` unchanged and do not bank the
diagnostic exact objects. The retained negative evidence remains under
`build/hypotheses/projectile2d-instrumentation/`, its compiler traces under
`build/gcc257/projectile2d-instrumentation-traces/`, and the full resolved
instruction audit under `build/frame-profile-controls/`. This function is
exhausted at an underdetermined 64-byte frame residue pending independent
source, debug, compiler or shared-template evidence.

## Item detail: primitive-buffer owner split

Second schedule follow-up Function Match Plan: GAME `80027b7c`, 732 bytes,
strict 97.814210%, unit `game.menu_item_detail`. The refreshed retail dossier
retains six callers, seventeen calls, twenty validated address pairs, seven CFG
blocks, three branches, one return, a 160-byte frame and three exact siblings.
Complete linked comparison has 181 of 183 words equal. Retail loads the
primitive-buffer pointer, initializes the real name-copy count in that load
delay slot, loads the buffer cursor, then initializes the complete union
workspace pointer in the cursor load-delay slot. The probe swaps those two
independent initializers. All later loop words agree.

Earlier batches varied declaration order, the loop form, pointer/index walks,
workspace views, complete inline copy boundaries and simple buffer/cursor
aliases. None split the two-level primitive-buffer expression at the exact
semantic boundary while initializing the consumed loop count between its owner
load and cursor load. Test a typed `KfPrimitiveBuffer *` owner with count
initialization before, between or after owner/cursor selection, using both an
assignment and initializer form. Trace the buffer owner, count, workspace and
glyph pointer with frame, spills and cross-jumps. Preserve the union view,
ten signed-halfword copies, all calls, referents, CFG and exact siblings.
Reject any form that changes later loop instructions, adds a dependency, uses
volatile storage or treats a pointer outside its complete object. Require all
183 linked words for retention and banking.

All four typed-owner forms compile to one 732-byte body at 99.672134%, but the
apparent score increase is misleading: each has nine unequal aligned words,
beginning at `+e0`, while the canonical source has only the two exchanged words
at `+ec` and `+f4`. The split owner allocates briefly to `v0`, moves the
seven-reference count from `a0` to `a2`, and moves the glyph pointer from `a1`
to `a0`; source order, assignment versus block initializer, and the three
count placements do not alter that allocation. The 160-byte frame and
cross-jump decisions remain unchanged.

Every state preserves seventeen calls, twenty ordered referents and all three
exact siblings, and independent native/instrumented compiles agree. Reject the
typed-owner split, leave `src/game/menu_item_detail.c` unchanged and do not bank
the function. Results are under
`build/hypotheses/item-detail-buffer-owner-2/`, traces under
`build/gcc257/item-detail-buffer-owner-traces-2/`, and the independent linked
audit is `build/item-detail-buffer-owner-audit.json`. The canonical two-word
initializer exchange remains an unattributed code-generation residue.

## Experience progression: terminal-row owners

Second schedule follow-up Function Match Plan: GAME `80016058`, 548 retail /
528 probe bytes, strict 88.824814%, unit `game.player_death`. The preceding
linked audit remains authoritative: one caller, three calls, twelve retail
address pairs, eighteen CFG blocks, nine branches, a 32-byte frame and twelve
exact siblings. Retail independently materializes the final and penultimate
growth rows for each extrapolated field; the probe keeps the final row base in
`s2`, which accounts for its extra saved register and seven missing address
pairs. The indexed growth path and every loaded field width already agree.

Test typed pointers to the complete final and penultimate
`KfPlayerLevelGrowth` rows, first shared across the extrapolation and then
scoped to each real field update. Also test equivalent end-pointer and indexed
owner spellings. Trace both owners, the indexed row, frame, spills and
cross-jumps. Preserve the table's single data identity, exact field operations,
three calls, CFG, caps and sibling bodies. Reject interior globals, raw
addresses, volatile access, forced registers, copied structures or any form
that changes the ordered growth loads. Retain only a strict 100% result.

The shared-row and operation-scoped forms collapse to the same 544-byte body
at 92.036500%. They restore four probe address pairs but allocate the
nine-reference final-row owner to `s2` across all three calls, retaining the
extra save/restore and producing nine referents rather than retail's twelve.
The end-pointer spelling collapses completely to the 528-byte canonical body.

Before rejecting this ownership family, test pointers to the five real row
subobjects consumed by extrapolation: maximum HP, maximum MP, physical step,
magic step and experience threshold. Use both branch-wide initialized owners
and operation-local owners. These are typed views into the established table,
not new global identities. Preserve the exact load widths and arithmetic, and
reject a result if any owner survives beyond its field update or changes the
indexed-row arm.

Both field-owner forms fold completely to the canonical 528-byte body: the
pointers have no allocated lifetime, only five referents survive, and the
same 120 aligned words differ. Independent native/instrumented builds agree;
all states retain three calls and twelve exact siblings. The 544-byte
shared-row form's higher percentage does not justify retention because it
still has 119 unequal aligned words, the wrong referent sequence and the same
unsupported cross-call `s2` owner. Leave `src/game/player_death.c` unchanged
and do not bank the function. Results and traces are under
`build/hypotheses/experience-terminal-{row,field}-owners/` and
`build/gcc257/experience-terminal-{row,field}-owners-traces/`; independent
linked audits are the corresponding files under `build/`. Retail's repeated
terminal-row materializations remain an unattributed code-generation residue.

## Map-object effect spawn: acquired-object and spawn-field owners

Second schedule follow-up Function Match Plan: GAME `80031834`, 404 retail /
400 probe bytes, strict 94.504950%, unit `game.map_object`. Hash-identical
retail and the refreshed six-view pass retain the sole actor-action caller,
five ordered calls, two validated sequence-address pairs, sixteen CFG blocks,
ten branches, three internal jumps, one return, a 40-byte frame and seven
strict-exact siblings. The caller supplies byte kind and object ID, a complete
word-coordinate position and signed Y offset. The allocator and sequence
publication are game-owned policy around the separately identified `rand`
provider.

Retail keeps the selected sequence pointer in `s0` and moves the acquired
object into `s1` in the post-call sequence load's delay slot. The probe defers
the return copy until after publishing the incremented sequence, reuses `s0`
for the object and inserts a load-delay `nop`. Prior trials exhausted branch-
local calls, declaration order, helper boundaries, coordinate ordering,
postincrement spellings and named old/next sequence values. They did not test
the acquisition result as an initialized lexical owner or the complete typed
spawn/link subobject as the immediate publication owner.

Test an initialized acquired-object declaration in the shared post-selection
block, both ordinary and const-qualified, then typed `KfMapObjectSpawn` and
`KfMapObjectLinkFields` owners for the real sequence destination, plus their
composition with the initialized object. Trace sequence, object, spawn/link
owner, all three surviving inputs, frame and spills. Preserve the exact
halfword update, store order, object initialization, five calls, two ordered
referents, CFG and all siblings. Reject copied objects, detached interior
globals, extra reads, changed publication order, volatile or register-forced
carriers, and any form that merely fabricates a live value. Retain and bank
only a strict 100% result.

All seven compiler-accepted owner forms emit the same resolved 400-byte body
at 94.504950%, with 54 unequal aligned words. The initialized lexical object
has the canonical five-reference sequence and eighteen-reference object
lifetimes, both allocated successively to `s0`; const qualification has no
effect. The typed spawn and link owners each fold to zero references before
allocation. Their compositions likewise retain the same 40-byte frame, omit
the retail `s4` save and keep the post-call load-delay `nop`. A mixed
declaration after statements is rejected by the pinned compiler's C parser
and supplies no additional executable hypothesis.

Every successful state preserves five ordered calls, both ordered sequence
referents and all seven exact sibling functions. Independent native rebuilds
agree with the instrumented objects and complete linked-word comparison.
Reject the owner forms, leave `src/game/map_object.c` unchanged and do not
bank the function. Results are under
`build/hypotheses/map-object-acquired-spawn-owners/`, traces under
`build/gcc257/map-object-acquired-spawn-owner-traces/`, and the independent
audit is `build/map-object-acquired-spawn-owner-audit/summary.json`. The
sequence/object allocation and delay-slot schedule remain an unattributed
code-generation residue.

## Entity traversal: actor-pass and origin owners

Follow-up Function Match Plan: GAME `8001f218`, 1408 bytes, strict
99.517044%, single-function unit `game.render_scene`. Hash-identical retail and
the refreshed six-view pass retain the no-argument `render_frame` caller,
nine ordered calls, 24 ordered address pairs plus one internal jump, 61 CFG
blocks, forty branches, eight division traps, a 48-byte frame and one return.
All immediates, opcodes, control destinations, calls and referents already
agree; the thirty unequal words change only GPR fields. Pool traversal and
visibility are game policy around the separately attributed
`SetLightMatrix` provider.

The remaining register roles are coherent across the whole body. Retail keeps
the Z/X window origins in `s6`/`s5`, then uses `s1` for the actor cursor,
`s4` for the actual view-Z field address and `s3` for the actor decrement
result. The probe assigns those roles to `s5`/`s4`, `s3`, `s6` and `s1`.
Earlier batches rejected shared window pointers, scalar and complete view-cell
pointers, width changes, visibility helpers and ordinary pass-local cursor
declarations, but did not test an initialized actor-pass cursor or a dedicated
signed actor countdown. The current initialized window origins also have no
assignment-form control.

Test the actor pass as a lexical block with its real cursor initialized from
`actor_state.actors`, its own signed-halfword countdown, and their composition.
Independently test the two existing wrapped origins as const initializers and
as separate or combined declarations followed by assignments. Trace both
origins, actor cursor, shared and dedicated counters, view-cell address, frame
and spills. Preserve the five pass order, every field width, wrapped
arithmetic, pointer increment, nine calls, 24 referents and complete CFG.
Reject added values, changed countdown semantics, hoisted memory reads,
volatile/register forcing or any form that changes a non-GPR instruction bit.
Retain and bank only strict 100%.

The sixteen-state matrix collapses to two bodies. Const qualification,
separate versus combined origin declarations, assignment after declarations,
and the initialized actor cursor all reproduce the canonical 1408-byte body
at 99.517044% with the same thirty unequal GPR words. Their traces retain
origin Z/X in `s5`/`s4`, actor in `s3`, the shared countdown in `s2`, and the
view-Z address in `s6`; the source boundaries do not alter any lifetime.

A dedicated actor countdown produces 1408 bytes at 99.474434% with 33 unequal
words. It shortens the shared counter from 27 references/six crossed calls to
22/five, allocates the actor to `s2` and its five-reference local count to
`s3`. Although the count result reaches retail's `s3`, the cursor moves farther
from retail's `s1` and three additional instructions diverge. Initializing the
cursor in the same block has no further effect. Every state preserves all nine
ordered calls and 24 ordered referents, and independent native linked-word
audits agree with the instrumented objects.

Reject all actor/origin forms, leave `src/game/render_scene.c` unchanged and
do not bank the function. Results are under
`build/hypotheses/render-entities-actor-origin-owners/`, traces under
`build/gcc257/render-entities-actor-origin-owner-traces/`, and the independent
audit is `build/render-entities-actor-origin-owner-audit/summary.json`. The
remaining register-role cycle is unattributed.
