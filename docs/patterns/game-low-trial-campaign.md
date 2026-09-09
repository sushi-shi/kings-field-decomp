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
