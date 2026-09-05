# GAME image-path component addressing

## Function Match Plan

Campaign: two custom path-digit formatters calling the now-exact
`screen_show_image_until_input`, on `probe-gcc257-o2-g0`. The six semantic
views, all five retail call sites, both compiled bodies, adjacent function
boundaries, shared declarations, retail strings, original source history and
exact actor/person path-formatting siblings were inspected before editing.
Neither function is a vendor/FID library body; the only call is to the custom
image-display helper. No indirect transfers or switch tables occur.

### TALK path: GAME 0x8002c9d4, 164 bytes

`talk_show_indexed_image` in `game.save_system` starts at strict 95.707310%.
It takes byte prefix/index/frame values and a signed word group ID, divides
the group by ten with the three checked-division branches and traps, then
patches `TALK\\C00\\T00000.TIM` at 0x8005606c. Four calls at 0x80034b50,
0x80034c00, 0x80034ca8 and 0x80034d24 load event/player byte fields and frame
2/3 or image_dirty; none consumes a result. Preserve the 24-byte frame,
ra at +16, digits +10/+11/+14 and duplicated group tens/ones at +12/+6 and
+13/+7, in that exact store order. The display call is at 0x8002ca60 and its
slot derives the full path from the directory tens address minus six.
Seven validated address pairs and one proven call yield fifteen relocations.
The preceding display helper ends with jr/stack restore at 0x8002c9cc/d0;
the next actor free-slot helper begins at 0x8002ca78.

Fresh compiled evidence differs from the old source-shape note: the current
TALK source keeps the full-path base in a0, separately materializes both group
stores and leaves a nop in the call slot. It is 168 bytes/42 words versus
retail's 164/41, with seventeen versus fifteen relocations. The call set,
arithmetic, store order and actual string bytes/referents agree. Test a real
two-digit directory-component cursor at path +6, used for its tens and ones
writes; leave all filename writes as direct references and pass the complete
array to the display API. Do not add a second global, change the path bytes,
force a subtraction into the call, or change the array's ownership here.

### Map path: GAME 0x80034d54, 144 bytes

`map_show_screen_image` in `game.map_interaction` starts at strict 79.250000%.
Retail patches `KAN\\B0\\K000.TIM` at 0x80056238: group at +8, current-floor
byte from player_state+10 at +5, then index quotient/remainder at +9/+10.
One call at 0x800355ec passes group 0/1 and a byte link ID; the existing C
uses signed word arguments and checked signed division. A historical candidate
`char` annotation does not independently establish a different boundary type.
Keep current signatures until stronger evidence exists. The frame and return
match the TALK helper. Five validated pairs and one proven call give eleven
relocations; the display call's slot subtracts five from the floor-digit base.
The interaction helper ends at 0x80034d4c/50; the main dispatcher follows at
0x80034de4. Initial compiled code is 148 bytes/37 words versus retail's 144/36,
anchoring on +8 instead of +5 and carrying the remainder in a2 rather than v1.

Inspect the TALK component experiment's first raw divergence before deciding
whether an analogous map-path source correction is supported. These string
arrays are not invitations to invent a struct or a padding owner. The SDK
sine table's previously corrected biased reference does not belong to the
TALK string and must remain untouched. Closure requires canonical strict 100%,
raw words, ordered referents, full build, existing lint/tests and preserved
banked functions. Bank only independently verified intended exact functions.

## Directory-component control

The TALK directory cursor recovers the retail 164-byte body, the +6 base,
the zero-offset tens store and the call-slot minus-six derivation. All fifteen
ordered relocation referents now agree. Its remaining differences are the
cursor in v0 rather than a0 and the remainder in v1 rather than v0. Test the
same substring cursor derived from the existing complete-path local, retaining
that path local for filename writes and the display API argument. Both locals
then represent actual string views; do not force a register or explicit
negative-offset call argument if the compiler still chooses other roles.

Retaining both full-path and directory locals reproduces the original TALK
object, including its extra word and address pair; revert that no-improvement
control. Keep the directory-only exploratory base for canonical comparison.
The map path has a one-digit directory component at +5, formed just after
the global floor load. Test a cursor to that existing digit for its store,
with all filename writes and the full-array call argument unchanged. Do not
move store order or add a second view solely to choose a register.

The map cursor recovers the directory-based zero-offset store and minus-five
call argument, but forms its address before the global floor read, leaving
an extra load-delay nop. Retail reads the current floor before forming the
directory address. Test snapshotting that actual byte after the group store
and then binding the directory cursor, preserving all four stores and their
order. This is an input-read lifetime correction, not an extra storage owner.

The explicit floor snapshot returns to the old +8-anchored map output; it
does not recover retail's load/address schedule. Revert that control and
retain the direct directory-digit cursor for canonical comparison. Raw byte
dumps verify each string including its NUL: the TALK terminator is at +19,
the map terminator at +15. The census's 19/15-byte text lengths exclude those
NULs; neither is proof of additional array capacity or a new data-owner extent.

Canonical cursor-only scores are 98.780490% for TALK and 88.888885% for the
map helper. All other function rows are unchanged; existing lint/tests pass
and the full build retains its prior data gates. Before finalizing, isolate
the cursor-binding lifetime itself: keep its declaration, but bind TALK's
directory cursor only after the three filename digit stores, where retail
actually forms it and the input argument registers are no longer live.
This differs from the rejected extra full-path local; no new value is added.

Late TALK binding loses the directory base and anchors on the filename's +10
store, despite keeping the source write order. Reject it and restore the
already verified directory-only source. The analogous extra floor snapshot
had likewise lost the map directory base; do not continue cycling pointer
declaration/binding forms without new source evidence.

## Retained verdicts

Both directory-component cursors are retained, without signature, type,
configuration, ownership or string-byte changes. Neither function is exact
or banked.

- TALK improves **95.707310% to 98.780490%**. Compiled size falls from
  168 to the retail 164 bytes. All fifteen ordered relocation rows, their
  function-relative sites and their referent addends agree. Eight of the
  41 words still differ only in cursor/remainder register roles: retail's
  a0/v0 become v0/v1 at +0x30, +0x60, +0x64, +0x74, +0x78, +0x80, +0x88
  and +0x90. Every literal, byte destination, checked-division branch,
  load/return delay slot and display-call target agrees.
- Map improves **79.250000% to 88.888885%**. Size remains 148 bytes versus
  retail's 144. The five address pairs have the correct objects/addends, but
  the directory +5 pair precedes rather than follows the player-floor load,
  leaving an extra load-delay nop. The cursor remains v1 instead of a0 and
  the remainder a2 instead of v1. All four byte destinations, store order,
  arithmetic and the call's minus-five path derivation agree. Do not reorder
  target relocations to conceal the source's address/load schedule.

The rest of the 484 native report function rows are unchanged, including
thirteen vendored controls: GAME remains 283/362 exact with 79 partials,
OPEN 97/108, and PSX 1/1. No banked function regresses.

Ruff and all 591 existing tests pass on the kept cursor-only state
(60.649 seconds). After rejecting late TALK binding, its complete raw body
and relocation sequence were rebuilt and verified identical to that tested
state. The restored full build retains only its pre-existing data/ownership/
placement gaps: source data 7/60, SDK complete contributions 4/4, target relink
110/116, six conflicting-base units and zero artifact failures. Diff check
passes. No tooling change, new test campaign or baseline update is required.

Subsequent data-branch integration moves the map helper into `game.map_scripts`
and names its complete private array `map_screen_image_path`. The directory
cursor and instruction sequence are preserved after normalizing the owning
`.data` addends; the strict result remains 88.888885%. DATA ownership evidence
is in `config/evidence/game_map_script_data.md`.
