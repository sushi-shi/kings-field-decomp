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

## Map formatter trace plan at `395429f`

Refresh the six GAME views, complete 144-byte retail body, sole caller,
adjacent boundaries, 16-byte private path, current source/history and exact
576-byte display callee. The caller still supplies group 0/1 and an unsigned
link byte; the signed division body does not independently settle the index
parameter's original width. Preserve its current signed-word boundary, group
enum and all four byte destinations. This custom path policy remains outside
the vendored inventory; retain the display callee's authentic SDK interfaces.

Fresh native/traced/repeated compilation reproduces the current 148-byte
candidate and all fourteen exact siblings in the sixteen-function unit.
Every delinked target reconstructs its retail body. The selected function
retains three checked-division branches, two traps, one call, five validated
address pairs and the 24-byte frame/return slot. Its first raw divergence is
the remainder's destination at +30 (hex); its path+5 address precedes the
floor-byte load instead of following it, retaining the extra load-delay nop.

Test one actual decimal decomposition: compute signed `tens = index / 10`
and `ones = index % 10` before the existing path operations, then use those
values for the two filename digits. Both values already exist as the divmod
results; the source must retain the arithmetic, stores and their order rather
than introduce a new carrier or narrow an unproved parameter. Inspect whether
the named values survive CSE and whether the independent cursor lifetime
changes. The previously rejected floor snapshot and cursor-binding variants
are not a new search axis. Compare complete words and ordered referents,
require native/traced parity, and preserve all other fifteen unit functions.
Only a strict 100% production result can be banked.

The decomposition emits exactly the baseline instructions in all sixteen
functions, preserving the fourteen exact siblings and the partial dispatcher.
Both objects' DATA and RODATA bytes also agree. The named `tens`/`ones`
pseudos 73/77 disappear by CSE1; the first divmod's quotient/remainder pseudos
74/75 retain hard registers a1/a2, just as baseline pseudos 81/82 do. Their
reference counts and live lengths remain 2/9 and 2/11, with no crossed calls.

The cursor changes pseudo number from 73 to 81 but retains v1, three references
and live length ten. Both scheduling snapshots materialize it before the
floor load and form the fixed a0 call argument before the last cursor-based
store. That surviving overlap is directly visible in the probe; retail instead
uses a0 for the cursor and derives the path in the call slot. Naming decimal
values does not change this state. The first raw difference remains the mfhi
destination at +30, and the path+5/floor address order remains reversed.
This is a rejected source control, not historical compiler attribution or a
reason to alter the reference inventory.

Native/traced/repeated whole ELFs agree per source and repeated traces are
byte-identical. A one-byte private-path shift changes exactly four address
instructions; shifting the display callee by four changes only its call word.
Complete evidence is under `build/gcc257/map-image-trace/`. No production source,
profile, header, identity, relocation or bank entry changes. Current-state
verification has zero failures and still reports GAME 324/362 and OPEN 106/108
exact. The same unchanged production inputs passed the 703-test/Ruff checks
and full-build audit recorded in the
[pool consumer checkpoint](game-graphics-owner-pilot.md#pool-lifecycle-consumer-audit-at-81fa559).
The map helper remains strict 88.888885%; all forty remaining functions still
require strict 100% closure.

## Group-boundary control at `8e26b18`

Refresh the six GAME views, all 36 retail and 37 candidate words, the sole
caller and neighboring boundaries, the exact image-display callee, shared
types, initialized path and source history. The baseline remains 88.888885%,
148 bytes, with native/traced whole-ELF parity. All fourteen exact siblings
reproduce their retail words; the interaction dispatcher is a separate
99.211440% partial. Calls remain proven and the five address pairs validated.
The formatter has three divide-guard branches, two traps, a 24-byte frame,
one call and its frame-restoring return slot, with no indirect transfers,
strings or candidate outgoing references.

At `800355c8..800355e4`, both caller arms define the group as zero or one;
unsupported object IDs return before the call. The group is consumed only
by addition of ASCII zero and a byte store. These facts permit testing an
unsigned-byte argument without changing any reachable input or output. They
do not prove that the original declaration was narrow. Keep the separate
index argument signed-word: its full-word signed division and traps remain.

First validate the byte-versus-word group behavior in a reduced formatter,
then test `KF_ENUM_PARAM(KfMapImageGroup, u8)` on the actual private function.
Preserve the caller's enum domain, all four stores, the floor-byte read,
directory cursor and every arithmetic expression. This is a boundary-width
hypothesis, distinct from the rejected digit temporaries and pointer-binding
orders. Require an unchanged caller and exact siblings; reject an ineffective
or contradicted width control instead of banking a signature assumption.

The reduced word/byte controls emit identical 148-byte instruction streams.
The byte source introduces a QI argument pseudo at EXPAND, but CSE1 removes
the narrowing because only the stored byte is consumed. All three compiler
builds agree on each complete ELF. The actual byte-group trial likewise
reproduces all 37 baseline words, five ordered referents, the partial caller
and fourteen exact siblings. Native/traced/debug whole-ELF parity holds for
the actual baseline and trial. Reject the no-effect signature change;
production C and the 88.888885% score remain unchanged. Generated evidence
is under `build/gcc257/game-image-group-traces/`.

## Map decimal-expression controls

Function Match Plan: refresh the GAME 80034d54/144-byte six-view evidence,
CFG and sole caller window at 800355c0..80035600. Preserve signed-word group
and index boundaries, the byte floor load, four byte stores, three checked
division branches/two traps, one display call, five validated address pairs,
and 24-byte frame. Current source remains 148 bytes/88.888885%; the address
and floor-load order still leaves an extra load-delay nop. The preceding
interaction routine and exact display callee retain the previously reviewed
source/retail evidence. This is custom path policy, not a vendor body.

Test the direct division/remainder baseline against signed ASCII-word digit
locals and a signed remainder computed as `index - tens * 10`, with
`tens = index / 10`. The quotient product remains within signed-word range,
including at INT_MIN; preserve the existing store order and directory cursor.
These decimal expressions introduce no new owner or parameter narrowing.

Three JSON states compile: direct and ASCII-word forms both score 88.888885%;
independent complete raw listings, including relocations, are identical. The
quotient-subtraction form scores 74.027780% and grows to 160 bytes. Its first
word already uses v1 for divisor 10 instead of retail v0, and it replaces
retail mfhi with two shifts, an add and a subtraction. The directory still
precedes the floor load, and the extra load-delay nop remains. Reject both
trials as explanations of retail; source remains unchanged and unbanked.

Results: `build/hypotheses/20260908-183712-game-map_scripts-map_show_screen_image`;
manifest: `build/map-screen-digit-hypotheses.json`; independent raw listings:
`build/map-screen-digit-objects`. Evidence refresh: `build/map-screen-evidence.txt`
and `build/map-screen-cfg-caller.txt`.

## Map path inline boundaries

Function Match Plan at `dda02e5`: refresh all six GAME views for
`80034d54` / 144 retail bytes, the sole caller at `800355ec`, current path
owner, adjacent functions, display callee, signatures, history and prior
cursor/digit trials. Current source is 148 bytes / 88.888885%. Preserve
signed-word index division and its three guards/two traps, the floor-byte
read, four ordered path stores, one display call, five retail address pairs
and the 24-byte frame. This is game path policy with no vendor body change.
The caller's group 0/1 and byte index do not prove a narrower public signature.

Test the complete path-formatting operation as an ordinary inline helper:
patch the global array, return that global array, or take and return the
existing path buffer. All keep the directory-digit cursor, full path contents
and exact statement order. These are source-boundary hypotheses; no explicit
negative-offset call argument or substitute string owner is introduced.

All three helper forms compile to the same 148-byte body at 95.111115%.
Independent raw inspection shows that they recover the retail divmod registers
and floor-load ordering, moving the first difference from `+0x30` to `+0x54`.
However, they materialize the full path instead of its directory-digit base,
write the floor digit through another symbol pair and leave a nop in the call
delay slot. Six address pairs replace retail's five; the higher score does
not justify losing the established cursor relationship. Reject all three.

A separate three-state control isolates only the two filename-index stores,
with the original directory cursor remaining in the caller. Both global-path
and supplied-buffer inline helpers are identical to baseline in all 37 linked
words, the sole display target and all five ordered address pairs. They stay
148 bytes / 88.888885%, with the same first difference at `+0x30`.

All seven independently compiled states preserve fourteen exact sibling
bodies against retail and leave the partial interaction dispatcher unchanged.
No extra helper call is emitted. Canonical source and bank remain unchanged;
neither formatting boundary explains the complete retail body.

Four-state results:
`build/hypotheses/20260908-202814-game-map_scripts-map_show_screen_image`;
three-state index results:
`build/hypotheses/20260908-202950-game-map_scripts-map_show_screen_image`.
Fresh evidence is `build/map-path-inline-evidence.txt`; independent raw checks
are in `build/map-{path,index}-inline-raw-verification.log`.

Both JSON baselines match the current source hash and all seven states compile.
Ruff, all 713 repository tests (105.710 seconds), and whitespace checks pass.
Full build retains GAME 337/362 exact and the existing data/relink failures,
with zero artifact failures. No source or bank entry is changed by this batch.

## TALK path inline boundaries

Function Match Plan at `5eb368f`: refresh the six GAME views for
`8002c9d4` / 164 bytes / 98.780490%, all four caller sites, the complete
source, shared byte/word parameter types, display callee, adjacent boundaries,
path owner, history and prior digit/cursor controls. Preserve seven ordered
byte stores, signed division and its three guards/two traps, one display call,
seven retail address pairs and the 24-byte frame. The custom dialogue policy
remains outside the vendor inventory; no signature or owner changes are tested.

A four-state JSON campaign compares local formatting with complete inline
helpers that patch the global template, return it, or take and return a
buffer. All three helpers recover v0 for the remainder but emit 168 bytes /
95.707310%, with eight address pairs. The first difference moves from `+0x30`
to `+0x64`: a0 holds the full path rather than path+6. The tens directory store
gets an independent address pair and the call delay slot becomes a nop.
Reject the three helpers because they lose the observed directory cursor.

A three-state follow-up isolates the character ID's paired filename/directory
writes. Its helper either owns the directory cursor or receives the existing
caller cursor. Passing the cursor is identical to baseline in all 41 linked
words, one numeric display target and seven ordered address pairs. The
internal-cursor form scores 84.829270% / 164 bytes and first differs at `+0x3c`:
it retains path+10 in v1, then derives the full path with minus ten, rather
than retail's path+6 in a0 and minus six. All seven stores still target the
same bytes; that does not establish the required ordered address sequence.

All seven independently compiled states preserve the 22 exact sibling bodies
against retail and leave the partial status-message wrapper unchanged. No
extra helper call is emitted. No source or bank change is retained.
Four-state results:
`build/hypotheses/20260908-203526-game-save_system-talk_show_dialogue_page`;
three character states:
`build/hypotheses/20260908-203640-game-save_system-talk_show_dialogue_page`.
Fresh evidence is `build/talk-path-inline-evidence.txt`; independent raw
checks are in `build/talk-{path,character}-inline-raw-verification.log`.

Both JSON baselines match the current source hash and all seven states compile.
Ruff, all 713 tests (121.606 seconds), and whitespace checks pass. Full build
retains GAME 337/362 exact and the existing data/relink failures, with zero
artifact failures. Canonical source and bank remain unchanged.

## Formatting-and-display inline boundary

Function Match Plan at `8a5cdba`: refresh hashes and all six GAME semantic
views for map `80034d54`/144 bytes/88.888885% and TALK
`8002c9d4`/164 bytes/98.780490%. Read both complete raw bodies, all five
caller paths, preceding/following source, exact display callee, shared
signatures, path owners, history and the earlier formatting-only controls.
The map signature remains a candidate; byte caller values do not independently
prove narrower parameters. Both remain custom game path policy, with the
existing display helper retaining its SDK boundaries.

The earlier helpers ended before the display call. Test a complete inline
format-and-display operation, once using its global path and once receiving
the existing path buffer. This keeps each directory cursor and its consumer
inside one operation, without returning a full-path pointer between scopes.
Preserve four map/seven TALK byte stores in order, the signed divmod and
three guards/two traps, one display call, and 24-byte frames. Retail has
five map/seven TALK address pairs and derives the full path in the call slot
from directory offsets five/six. Each JSON manifest has three states including
canonical; no forced inline attribute, new data owner or parameter narrowing.

All six states compile. Both map helpers emit 148 bytes / 95.111115%, with
six address pairs. Their first difference is +0x54: they materialize path+0
instead of path+5, add an independent floor-digit address pair and leave a
NOP in the call delay slot. Both TALK helpers emit 168 bytes / 95.707310%,
with eight pairs. Their first difference is +0x64: path+0 replaces path+6;
an independent directory-tens pair and NOP call slot follow. Thus including
the display consumer does not recover the retail cursor lifetime. Reject all
four helpers and retain canonical source; nothing is banked.

Independent disposable recompilation resolves every word and ordered call/
address target. All fourteen exact map siblings and twenty-two exact TALK
siblings reproduce complete retail bodies and references in every state.
The partial interaction dispatcher and status-message wrapper are unchanged.
Both canonical source hashes still match the manifest inputs. Evidence is
`build/path-display-inline-evidence.txt` and
`build/path-display-inline-verification.log`; independent objects use the
`map-display-inline-objects` and `talk-display-inline-objects` directories.
JSON results are
`build/hypotheses/20260908-215959-game-map_scripts-map_show_screen_image` and
`build/hypotheses/20260908-220004-game-save_system-talk_show_dialogue_page`.

The lost cursor motivates one additional two-state plan per function: leave
the existing directory cursor in the caller and pass it to the complete
format-and-display helper. Keep global filename writes and the original
complete-array display argument. This introduces neither an extra full-path
view nor explicit negative-offset call arithmetic. Both new helpers reproduce
canonical exactly in all linked words and ordered references: map remains
148 bytes/88.888885%, TALK 164 bytes/98.780490%, both first differing at
+0x30. All 36 exact sibling controls and both partial siblings remain unchanged.
Reject these two forms as well; moving the consumer is not the missing fact.
Results are
`build/hypotheses/20260908-220247-game-map_scripts-map_show_screen_image` and
`build/hypotheses/20260908-220251-game-save_system-talk_show_dialogue_page`;
independent verification uses the `path-directory-display` prefix.

The full production build retains GAME 337/362 exact and 99.428% aggregate,
with zero artifact failures. Source-data matches remain GAME 11/41, OPEN
3/19 and PSX 0/1; target relinks remain 75/77, 34/38 and 1/1. Existing
incomplete data/closure checks still cause its nonzero exit. Production
source, configuration, profiles and bank entries are unchanged.
Ruff, all 713 repository tests (102.621 seconds) and `git diff --check`
pass on this final state.
