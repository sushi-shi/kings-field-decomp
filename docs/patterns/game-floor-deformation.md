# GAME floor-deformation line

## Function Match Plan at `282cbe7`

GAME `800384f8 effect_floor_deform_line` owns 460 bytes (`0x1cc`) in
`game.effect_update`, initially strict 86.704346% under the unchanged
`probe-gcc257-o2-g0` probe. Hash-identical retail, all six semantic views,
both dispatcher call windows, adjacent helper boundaries, the sound callee,
table bytes, shared types and source history were inspected before editing.
The [map-script ownership dossier](../../config/evidence/game_map_script_data.md)
establishes the five initialized seven-byte segments and prior progress fix.

Retail has a 72-byte frame, saves s0..s8/ra, makes one spatial-sound call,
and uses three validated address pairs and two internal absolute jumps.
There are no candidate outgoing references, strings, switches or unresolved
indirect transfers. The private ELF `.data+0` reference and the target's
`floor_deform_segments` identify the same bytes at `80056268`; their spelling
difference does not justify changing the referent or relocation inventory.

Both calls in dispatcher kind 52 pass a word segment index and signed
halfword-derived progress/step values; one negates the step. Neither uses a
result. Keep the existing three-s32/void interface. Table fields are all
unsigned bytes, including steps of 255. Every coordinate use wraps modulo 256.
The grid and sound policy are game-owned, not a vendored SDK routine; no
vendor inventory entry exists. The audio wrapper remains an external game
boundary with authentic SDK VECTOR coordinates and signed-halfword volume.

The zero-count path and completed loop reach the same epilogue. The running
progress advances in the initial clamp branch's delay slot on every cell,
before clamping its snapshot to 0..4096. Sound is emitted only for snapshots
at least 3900 and below 3900+abs(step), within the unclamped arm. Preserve this
behavior, constants 100/1000/2000/3900/4096/127, all seven field accesses and the
final frame-restoring return slot.

First trial: retail copies progress_step into a mutable range before testing
and conditionally negating that range. Express the copy and conditional
negation instead of the current ternary. Then investigate byte coordinate
locals, supported by the field widths, masks and copied update results.
Finally inspect a signed-word predecrement countdown: retail explicitly
decrements the loaded count and compares with -1 before entering the body;
the current initializer/test optimizes into an original-byte zero test.
Compile and inspect the first real divergence after each focused change.

The initial source emits 440 bytes with an 80-byte frame. Do not manufacture
storage to recover retail's 72-byte frame. Five exact sibling helpers are
controls; the 2D helper's unexplained 64-byte frame difference and scatter's
previously investigated arithmetic association are outside this trial.
Require strict objdiff 100%, complete raw words and ordered physical referents
before banking. Preserve all existing exact functions and run the full build.

## Focused source trials

The explicit range copy and conditional negation restore the retail entry
instructions and roles of progress/step/range/height delta. Strict comparison
rises to 91.721740%. Frame 80 versus 72, the count preheader and word-sized
coordinate updates still differ. Next change only col/row to unsigned bytes;
their input fields, modulo 256 uses and explicit retail update copies support
this source-width hypothesis independently of the percentage.

Byte coordinates restore the two update-result copies and the retail
multiply/store/count-decrement scheduling; strict comparison reaches
95.634780%. The remaining preheader still tests the original count byte
against zero. Next express the real signed countdown directly as
`count = segment->cell_count; while (--count != -1)`, keeping the coordinate
and height initialization order and the running-progress update unchanged.

The direct predecrement loop reaches strict 100%. It restores the missing
preheader instructions, the retail register roles and the 72-byte frame
without changing the local storage declarations. The full 460-byte body now
agrees; `kf try` still prints the harmless private-section/symbol spelling
difference, so its normalized text percentage is not the closure criterion.
Remove the now-redundant coordinate masks and verify the whole result again.

## Final verdict and raw audit

Removing the redundant masks preserves strict 100%. A separate fresh
production-profile compile resolves every relocation to its physical retail
destination and compares all 115 instruction words without masking: no
differences. All nine ordered relocation sites/kinds agree, including the
two internal jumps, the sound call at `80032fb8` and the three data targets
`80056268`, `80056194`, `80095900`. All 35 private table bytes are unchanged.
The five exact siblings retain every linked instruction, while the 2D helper
still has precisely ten differing stack words and scatter six arithmetic
words. Neither partial is claimed exact.

All 484 native function-report rows were compared with the pre-edit snapshot:
only this floor helper changes, from 86.704346% to 100%. No exact function
regresses. GAME advances from 292/362 to 293/362 strict exact; OPEN stays
98/108 and PSX stays 1/1, for 392/471 non-vendored functions exact. There
remain 69 GAME and ten OPEN partial functions; the full objective is open.

This is a source reconstruction result under the unchanged GCC 2.5.7 probe,
not historical compiler attribution. No register/frame mechanism is inferred
from the successful source correction, and no compiler flags, identities,
relocation evidence, data placement or ownership rules were changed.

The final full `kf build -j4` ran after the fresh focused compile and remains
red on the pre-existing data/ownership/placement checks: 6/60 source data
units match, all four independent SDK data contributions match, and 110/116
target units relink, with six conflicting section bases and zero artifact
failures. The unchanged floor-table source alignment still fails placement;
exact function code does not waive that separate data requirement.
Ruff passes, all 649 existing repository tests pass without skips in
91.770 seconds, and `git diff --check` passes. No tooling or flake changes
were made. Selective banking is limited to GAME `800384f8`.
