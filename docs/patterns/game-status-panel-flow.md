# GAME status-panel frame flow

## Function Match Plan

GAME `0x8002430c menu_status_panel`, 1692 bytes / 423 words, occupies the
single-function unit `game.menu_status_panel`. Starting strict objdiff is
78.468090% with `probe-gcc257-o2-g0`. The six semantic views, complete retail
disassembly, sole direct caller, adjacent boundaries, shared menu types,
primitive helpers and source history precede changes.

`menu_root` calls at `0x80022484` with no arguments and ignores the result.
The preceding spell-selector return includes its delay slot; the following
drop panel starts at `0x800249a8`. This is custom status-page composition,
not a vendored body. The separately identified `SetSemiTrans` is the exact
Release 2.5 LIBGPU/PRIM provider; the packet remains the authentic SDK
`POLY_FT4`, with halfword positions and byte UV outputs.

Retail saves ra and s0..s5 in a 112-byte frame. It makes 20 direct calls:
frame begin, details draw, four repetitions of primitive begin / semi-trans
enable / commit, backdrop draw, present, three PadRead sites and one cue.
There are no strings or indirect transfers. The 105 validated address pairs
refer to `current_poly_ft4` and `menu_assets.window_backdrop` members +0x318,
+0x31a, +0x31c, +0x31e, +0x320 and +0x322. The body has one candidate
internal MIPS26 relocation at `0x80024954`, raw `0x080090d2`, targeting the
frame head `0x80024348`. Its delay slot increments the frame counter.
Review this exact instruction/target pair before source changes.

The frame counter starts at zero. Frames zero and one advance without
input handling; frame two waits for button release and takes the same
increment tail. Later frames poll without incrementing. A nonzero input
plays cue 2, waits for release, then returns through the sole restore block.
Current nested loops duplicate the increment/jump, unlike retail's shared
tail. First test a single frame loop with the two settling paths joining
before the increment, preserving dismissal and release waits.

The four quad origins are (6,16), (77,16), (6,120), (77,120), with alternating
horizontal/vertical texture reflections and ordering depth 2900. A separate
source control may restore the natural x0/y0, x1/y1, x2/y2, x3/y3 corner
assignment order shared by the backdrop renderer, rather than spelling its
scheduled independent stores as source order.

The initial candidate uses a 48-byte frame and retains a common descriptor
base, whereas retail retains only its tpage address and rematerializes the
other members. Do not split the proven owner, add padding/volatile carriers,
or change SDK types to force these differences away. Compare calls, numeric
referents and CFG before attributing any remaining instruction differences.
Rebuild the unit and full project, inspect all-image score changes and run
existing lint/tests before handoff. Bank only strict 100%.

The explicit shared increment label and natural corner order raise strict
objdiff to 81.900710%. The first structured nested-if trial still duplicated
the increment and added a jump in the release wait, so it is not retained.
The explicit label recovers the retail branch/join sequence. Natural corner
order places the CLUT store before the y1/x2 stores, matching retail.

A final bounded owner/lifetime control makes the observed tpage-only cursor
explicit: retail initializes s5 to the page halfword and uses it only for
the four page loads across calls. Test a `const u16 *` to that member while
leaving every other descriptor access spelled through its complete owner.
This does not split storage, cache the page value, or alter any shared type.
Retain it only if it explains the retail addressing; otherwise revert it.

The tpage-only pointer control instead creates another saved pointer to the
CLUT member (+2), retains a common base for the other fields and adds s6 to
the save set. It does not recover the independent retail address pairs and
is reverted. The final kept source has no new local or type changes.

## Retained verdict and raw comparison

The reviewed jump alone changes strict objdiff from 78.468090% to
78.479904%; it corrects the delinked target, not the retail bytes. The final
single-loop source with its shared increment label and natural corner order
reaches **81.900710%**. It is not exact and is not banked.

A fresh, separate pinned compilation produces 355 words versus retail's
423. All 20 ordered direct call targets agree. Relinking the target object
reproduces every raw retail word, including the newly reviewed jump. The
candidate materializes 37 HI16/LO16 pairs versus retail's 105 because it
retains the descriptor base; this is not missing source data ownership.

Straight-line constant propagation over the draw region, treating loaded
values as unknown and clearing caller-saved registers after call delay
slots, resolves all 108 global-load opcode/address pairs identically and in
the same order. The only addresses are `current_poly_ft4` at `0x80057e88`
and the six backdrop fields at `0x80058400/02/04/06/08/0a`. This static
address check does not execute game code or imply byte exactness.

The complete 30-word frame-control/restore tail starts at relative +0x624
in retail and +0x514 in the candidate. All eleven transfers have the same
kind, opcode, delay-slot ownership and target relative to that tail or the
frame head (+0x3c). The settling branch reaches the increment at tail+36;
the active-input branch bypasses it. Both release loops and the sole return
are present. The coordinate-load/store scheduling correction also removes
one unsupported nop per quad.

The remaining 68-word extent difference corresponds to the independent
retail descriptor-address materializations. Frame size remains 112 versus
48 bytes, with no observed object accounting for the extra retail 64 bytes.
Frame, base-selection and saved-register-role differences remain unattributed;
do not restore the old history's asserted compiler-wall explanation.

## Verification

The retained source was rebuilt with `kf match --unit game.menu_status_panel`
after reverting the pointer control, then checked with a full `kf build`.
The independent fresh compilation and raw checks above use that same source.
All 657 existing tests and `ruff check scripts tests` pass; `git diff --check`
is clean. No tooling or compiler-profile change is included.

Across all 484 scored rows, only this panel changes from the campaign-start
snapshot. The 398 eligible exact functions remain exact (GAME 299/362,
OPEN 98/108, PSX 1/1), as do the separate vendor controls. No baseline row
is banked and the function identity remains a candidate reconstruction.

The full build continues to fail the pre-existing data/reference/relink gates:
source-owned data matches 8/61 units, SDK data contributions 4/4, and target
relinking 110/116 units (PSX 1/1, OPEN 34/38, GAME 75/77), with six conflicting
section-base cases and no data-artifact failures. These are not a clean
whole-image closure. Unrelated dirty player/magic/effect/menu changes are
preserved and excluded from this campaign's commit.
