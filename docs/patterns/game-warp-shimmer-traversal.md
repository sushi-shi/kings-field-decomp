# GAME warp-shimmer effect traversal

## Function Match Plan

At master `20a285a`, GAME `80036618 player_warp_shimmer` is 568 retail
bytes and strict 74.964790%, in `game.player_warp` with `probe-gcc257-o2-g0`.
The six semantic views, complete raw source/target, all seven direct call
sites, neighboring warp helpers, shared effect types, and reconstruction
history were read before editing. No strings or candidate outgoing references
occur. Nine proven calls, four validated address pairs and two validated
internal jumps yield nineteen ordered relocations. The linked constructor,
rendering, sound and pacing helpers are custom game policy; this function is
absent from the vendored/FID inventory and does not reconstruct an SDK body.

The full-width mode arrives in a0, with signed-low-halfword interpretation at
both mode decisions; the position pointer in a1 supplies three words and is
forwarded unchanged to effect_pool_construct. Existing caller signatures and
the position/scratch types are not this experiment's subject. The wrapper
at 146a0 forwards its mode; calls at 17d30/17e24, 36890/3698c and 369f8/36acc
pass modes zero/one. Unsupported modes retain the existing indeterminate
intensity behavior; do not invent a default. The exact wrapper/floor-entry
callers and actor-transition sibling must remain unchanged.

Retail's 104-byte frame saves ra and s0-s7. It stores four effect pointers at
sp+24, copies x/z/y to sp+40/48/44, and passes sp+56 as the constructor's fifth
argument. Preserve those actual stores and the original position argument;
no padding, zero initialization or invented whole-object extent is justified.
The next function starts at 36850 immediately after jr ra and its +104 slot.

Three traversals have independently visible roles:

- Allocation: pointer starts at sp+24, signed-halfword count starts at three,
  decrements to minus one, and pointer advances by four. Constructor arguments
  are 10,17,21, the original position, and the existing direction scratch.
  Store initial intensity at effect+38 before saving the pointer.
- Each of 48 frames: reset the pointer, count upward 0..3 for the stagger
  predicate i*8 < frame, and load each effect once before that guard. Increase
  its unsigned-halfword intensity only below 8193; advance rotation at +30 by
  512 modulo4096 regardless of the intensity guard. Frame eight plays sound
  reference6 at volume127. Three flips/two initial renders and one render/wait
  per frame remain in their observed order.
- Release unless mode's low halfword is two: pointer again walks forward while
  a signed-halfword count descends three to minus one, storing type byte255.

The current probe is 616 bytes with a 96-byte frame, indexed up-counting
allocation/release loops, and repeated array/effect loads in the frame loop.
First test the allocation's real forward cursor and descending count. Inspect
the first raw divergence before extending the same cursor to the independently
evidenced frame/release traversals. Do not attribute old notes' claimed
optimizer/register limitations without backend evidence. No declaration
permutation, fake carrier or compiler flag change is part of this plan.

Require strict100%, raw words and ordered relocation referents for closure.
Run focused actual compilation, full build, existing lint/tests and diff check;
preserve every banked function and bank only a newly verified exact result.

## Allocation result and next correction

The descending allocation cursor recovers retail's 104-byte frame, all eight
saved registers, and the entire instruction prefix through the constructor
call. The first remaining difference is storing its return directly through
v0 instead of copying it to a0. The unchanged indexed frame/release walks
still diverge. Keep this evidenced allocation and test the frame loop's single
effect-pointer load before the stagger guard, advancing the same real cursor.

The frame cursor removes the repeated indexed pointer loads and recovers
retail's forward increment in the stagger branch delay slot. Its intensity
expression still reloads the same halfword, whereas retail reads it once into
v1 and uses that value for both threshold and addition. Finish the independently
evidenced descending release traversal before isolating that value reuse.

The release correction recovers the three-to-minus-one count and forward
pointer walk, but exchanges the cursor/count register roles across all three
loops. Retain the directly evidenced traversal while testing the single loaded
unsigned-halfword intensity used by both comparison and addition. This is an
actual field-value snapshot, not a register carrier or changed field type.

The intensity snapshot recovers the one lhu, unsigned comparison, and addition
in its branch delay slot, removing the repeated read and its load-delay nop.
Retail resets the frame cursor in the frame-eight guard's delay slot, before
the optional sound call, and initializes its index afterward. Move that real
cursor reset before the guard to express the observed use lifetime.

The earlier reset recovers both retail cursor/index register roles and the
frame-eight delay slot. Remaining differences are the constructor-result move,
the intensity addition's operand order, and the release effect/sentinel roles.
Retail uses the same active-effect register through initialization, frame
updates and release. Test one actual active-effect pointer reused by those
three traversals instead of separate loop-local/anonymous pointer expressions.
Every assignment has an existing effect use; no extra carrier or lifetime
padding is introduced, and the pointer must not escape its four-element list.

Reusing that active-effect pointer recovers the constructor-result move and
release register roles. The focused instruction/relocation comparison now has
one remaining difference: retail adds delta then current intensity, while the
probe adds current intensity then delta. Spell that same defined addition in
the directly observed operand order; neither widths nor arithmetic grouping
change. Raw bytes and the strict native comparator must still verify closure.

## Final verdict

`player_warp_shimmer` reaches **strict objdiff 100%**, from 74.964790%.
The source compiles to exactly **568 bytes / 142 words**, down from 616 bytes.
Every raw word and all nineteen ordered relocation rows agree, including
internal jump addends, the four data pairs, call targets and every delay slot.
The 104-byte frame, saved-register set, mode narrowing, position stores,
constructor handoff, stagger/brightness checks, phase wrap and release order
all match. No compiler flag, signature, type layout, data owner or relocation
inventory change was needed. This closes the source hypothesis; it does not
prove the original compiler version or unique original C spelling.

All other **483 native function rows** remain unchanged, including thirteen
vendored controls. GAME advances **283 to 284 / 362 exact**, leaving 78
partials; OPEN stays 97/108 and PSX 1/1. The unit is now 2/5 exact. The adjacent
change-floor and same-floor helpers retain their 99.770120% / 99.629630%
prologue move-order residues; the floor-trigger switch remains 68.326530%.
The exact actor transition and all external callers retain their scores.

Actual focused compilation and the canonical GAME rebuild pass. The full
build retains its pre-existing strict data/ownership/placement failures:
source data **5/60**, SDK contributions **4/4**, target relink **110/116**,
six target section-base conflicts and zero artifact failures. No banked
function regresses. Ruff and all **609 existing tests** pass without skips
(64.952 seconds), and diff check passes. Only GAME `80036618` is selected
for banking; no tooling/flake change or new test campaign is involved.
Generated logs are `build/game-warp-shimmer-*`, never committed.

## Selection notes

Fresh scouting leaves GAME `8001bb94 display_initialize` at 94.253010%:
its dimensions/setup instructions already agree, while retail derives both
DISPENV and distant fog state from the first DRAWENV dither address. Recover
the enclosing graphics ownership before attempting cross-object pointer
expressions; no source trial was made. GAME `80030eb8 map_object_probe_forward`
retains the previously documented 93.755104% shared-call CFG and radius/X
register residue. Its rejected duplicated-call trial was not repeated.
