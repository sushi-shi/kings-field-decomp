# GAME projected sprite depth and SDK outputs

## Function Match Plan (2026-09-06)

GAME `8001e230` / `250` (592 bytes), `render_enqueue_sprite`, starts at
strict 88.263510% on `04ffd96`, under unchanged `probe-gcc257-o2-g0`.
Hash validation, all six semantic views, the complete retail body/CFG,
five call sites, adjacent map/HUD emitters, source history, shared types
and the exact OPEN behavioral homolog were inspected. The ABI is supported
`void(KfSpriteQuad *, s16 screen_scale, s32 flag)`: floor items pass 150/200
and flag one; actor billboards and all three notification sites pass zero
for bias/flag. Retail sign-extends the bias and compares the full flag to
one. No caller consumes a result and no strings are referenced.

Retail has a 136-byte frame, five ordered calls, seven validated address
pairs, three conditional branches and one return with its restoring delay
slot. Source has a 144-byte frame and 600 bytes. RotTransPers/RotTransPers4
have supported earlier LIBGTE SMP/CMB lineage; SetPolyFT4/AddPrim belong to
the exact LIBGPU PRIM contribution. This sprite composition/allocation
policy is game-owned, not a reconstructed SDK body.

Keep four signed-halfword corners, zero origin, authentic SDK 40-byte FT4,
cursor advancement before the unsigned overflow return, packed word XY
copies, byte UV arithmetic, flag-one signed perspective scaling, and
signed biased depth at least five with OT mask 3fff. The normal is the
existing eight-byte `{0,0,1000,0}` SVECTOR (hex). The material's CLUT-plus-four
colour reference remains a separately recorded ownership question; do not
introduce a partial aggregate or rewrite exact consumers in this campaign.

First preserve the original projected depth across lighting and express
`otz + screen_scale` at the threshold/index, matching retail's transient
sum in a0. Then use the SDK's actual `long` origin-screen output instead of
a cast DVECTOR, and test the zero-origin chained assignment supported by
retail's z/y/x stores. Finally place the texture-page assignment beside
CLUT before the coordinate copies: retail loads the page before x0 is
stored, although its page store is scheduled later. Inspect the first raw
divergence after each focused compile; keep every banked function and the
other three emitter scores unchanged. Full build, raw referents, lint and
tests remain mandatory; bank only strict 100%.

## Focused results and owner witness

The transient depth expression reaches 92.466220% and restores the 136-byte
frame. The authentic SDK `long` output restores all projection-output stack
locations and reaches 92.554054%; the chained origin stores reach 92.567566%.
Texture-page publication before the coordinates reaches 93.972980%. The
remaining first difference is material address formation: source directly
loads CLUT, then materializes its colour pointer separately; retail keeps
the CLUT address through the UV stores and derives the colour pointer in
the flag branch's delay slot. Calls, widths and projection outputs agree.

Add this corrected body as a fourth witness to the existing complete-owner
pilot, using that same unchanged declaration. Replace only the existing
display/material references in a temporary copy; do not adopt a narrower
material aggregate, create storage, alter the colour expression, or change
another function's algorithm. Record complete raw agreement or the first
remaining divergence independently from the production objdiff score.

The shared-owner sprite probe reproduces all 592 raw bytes, five ordered
calls and seven address pairs exactly. This does not bank the production
function or settle complete graphics ownership. As a bounded alternative,
test the already evidenced eight-byte CLUT/page/CVECTOR material record
using OPEN's existing SDK-compatible type shape and its real colour field.
Unlike the projection interval, this complete field family has a bounded
four-byte SDK colour access at +4 and independent halfword selectors at
0/+2. Keep its following HUD state outside the record. Only a temporary
sprite copy changes for this comparison; migrating its real production
owner would still require checking all other material consumers together.

## Final verdict

The eight-byte material-only probe also reproduces all 592 retail bytes,
including all five calls, seven address pairs and delay slots. Its shading
argument uses the real `color` member, not arithmetic beyond a standalone
halfword global. It leaves the three polygon functions unchanged and needs
no projection-array capacity assumption. This is a concrete next production
ownership campaign: inspect all material readers/writers and migrate the
six identities, shared declarations, numeric referents and complete BSS
claim together, preserving every banked consumer. Do not bank the temporary
extern record or create overlapping storage to shortcut that migration.

Canonical source retains strict **93.972980%**, up from 88.263510%, and now
has exactly 592 bytes and the 136-byte frame. All five calls occur at the
retail offsets, and the full eight-word restore/return tail is exact.
Overflow and biased-depth rejection reach that tail. The flag branch and
its scaling block are eight bytes earlier because material publication is
scheduled differently; source has eight address pairs versus seven retail.
Its separate colour-pointer formation is the remaining first cause to
investigate, not a proved compiler wall. No owner migration is retained in
production yet.

Only this sprite's score changes among all 484 native rows. The neighboring
TMD/model/map functions and every banked function retain their scores; GAME
stays 288/362 exact and OPEN 97/108. GAME aggregate fuzzy becomes 95.99%
(from 95.97%), overall 96.51% (from 96.49%). OPEN sources are unchanged.
Focused recompiles and the recorded match preserve the result. Ruff, all
640 repository tests (79.262s, no skips) and `git diff --check` pass.
Full `kf build -j4` still exits 1 on the existing data/ownership/placement
gates: source data 5/60, config-backed SDK data 4/4, target relink 110/116,
six conflicting section bases and zero artifact failures. No partial result
is banked.

## Material-consumer control plan at `c518cc5`

Before adopting the sprite's eight-byte material, recheck the already exact
GAME `8001fde4` / `518` frame renderer. Its current six semantic views,
complete retail disassembly, prior material campaign and source were read.
It has an 80-byte frame, 20 direct call sites, 68 validated references, 22
external callers and no strings. Nullable VECTOR/SVECTOR arguments pass
directly to the view-transform helper; no caller consumes a return value.
The material copies retain halfword texture selectors and byte RGB, including
HUD grayscale, notification white, and three conditional sprite call sites.
SDK calls are independently attributed; composition policy is game-owned.

First reproduce the earlier eight-byte-owner regression with the current
frame source, beside canonical and complete-graphics-owner controls. This
is a declaration/reference-only temporary experiment: no production storage,
identity, algorithm, compiler setting or function score is changed. Compare
all linked instruction words, ordered calls and materialized addresses.
The old experiment specifically warns that a narrow owner can retain a
pointer across calls. Do not adopt it unless the exact frame and screen
sprite are preserved. A full-owner control does not settle the opaque
projection/morph interval or authorize overlapping production globals.

### Frame-owner verdict

The current eight-byte material probe reproduces the earlier regression:
1296 source bytes versus 1304 retail, the same 20 ordered direct calls, and
59 address pairs versus 64. The first mismatch is the saved-register layout
at +4. It retains the texture-page member's address in s2 across the HUD,
lighting and notification calls and writes CLUT at that address minus two.
Retail instead forms all six selector stores independently. The 80-byte
frame size is unchanged; this is not a missing call or a source correction
to force with extra locals. The narrow owner is not adopted.

The same frame body using the unchanged complete graphics-owner declaration
reproduces **all 1304 linked retail bytes**, all 20 calls and all 64 address
pairs. Canonical source is also raw exact. This is a new positive witness for
the broader owner, not a new production exact function: render_frame was
already banked. Its nullable argument setup was checked at all 22 external
call sites, with adjacent flip-buffer and matrix-interpolation boundaries.

The durable graphics-owner controls now include that complete frame result
and a shifted-root negative control. A separate canonical/narrow comparison
records why the sprite-only eight-byte result cannot authorize production
migration. Existing material identities, headers, storage claims, source
algorithms and numeric relocation destinations remain untouched. The next
ownership step is the complete region's typed projection/morph representation
and remaining consumer audit, not the previously proposed standalone material.

The canonical frame was freshly rebuilt and remains strict 100%. All 484
native function-score rows are unchanged from `c518cc5`: GAME 288/362 exact,
OPEN 97/108, PSX 1/1. No temporary owner or partial function is banked.
Ruff, all 641 repository tests (77.437s, no skips), the seven focused owner
controls and `git diff --check` pass. Full `kf build -j4` retains source data
5/60, config-backed SDK data 4/4 and target relink 110/116, with the same six
section-base conflicts and zero artifact failures. No production source,
header, inventory, compiler profile or build tool changes in this follow-up.
