# GAME second developer-style source pass

## Function Match Plan

Start from master `273accaa`, GAME 345/362 strict exact. Freeze all seventeen
remaining functions. The main method is to write plausible period game code
for the complete operation, then use the pinned compiler and retail comparison
to test it. Do not tune declaration order, force registers, or invent storage.
Earlier trial reports are negative controls, not a menu of variants to repeat.

Hash-identical retail, six image-qualified semantic views, source/history and
raw instruction controls are captured per function under `build/era/`.
Calls, typed object ownership, arithmetic widths and observable ordering must
survive any rewrite. Each complete candidate unit is compared with its frozen
baseline and every delinked claim is independently resolved to retail bytes.
Exact siblings must not regress. SDK providers stay external; the selected
functions are game policy, not vendored reconstruction.

Begin with item-detail name preparation and repeated menu tiles. The adjacent
exact preview independently supports selecting a complete name row before
publishing the primitive cursor and using a normal positioned glyph string.
Test that operation in the detail renderer, the repeated model-transform stage
as an ordinary inline helper, and one reusable glyph-row destination throughout
its text/number rendering. The detail interface remains three word arguments,
with the 255 guard, ten halfword copies, all seventeen calls and twenty ordered
address pairs. Its three exact sibling functions are controls.

For the three tiled menu routines, test a drawing request expressed using real
screen and texture rectangles. Four reflected tiles are independently visible
in each function. An inline helper must consume every field and preserve all
packet stores and begin/transparency/commit calls. Unaccessed frame bytes alone
do not justify any local object. Do not retain a rectangle that supplies only
stack extent, or change resource ownership to fit a score.

Continue through the movement, rendering, resource, map and effect operations
with their own source-level hypotheses and final verdicts. A negative verdict
is appropriate when current evidence supplies no new plausible correction.
Only strict 100% plus complete raw equality permits banking. Full build, lint,
repository tests and a reviewed integration conclude the campaign.

## Resource/path hypotheses

`item_load_database` has one startup caller, five retail calls, sixteen validated
references including six internal jumps, and two literals. Its six copied banks
form the documented contiguous 5,708-byte STAT.DAT payload. Test an ordinary
typed resource image in place of a running byte cursor, and a one-based file
number loop (I001 through I080) in place of an item-index loop plus `i + 1`.
Also test a small inline formatter for the repeated three-decimal-digit name.
All six copies, eighty searches and conditional sector rounding must remain.

`talk_show_dialogue_page` and `map_show_screen_image` are small game-specific
path composers around the same display call. Their four/one callers constrain
the byte inputs and signed decimal number; preserve that full signed arithmetic.
The dialogue character appears twice in its path, motivating one helper that
writes the same decimal pair to two destinations. Compare inline, ordinary
comma-expression macro and a real two-character temporary. The map path uses
the same two-digit operation once. No sprintf call, new initialized data,
indirect call, changed template byte or restricted input range is introduced.

## Object creation and persistence hypotheses

`map_object_spawn_effect` selects one of two ten-object pools, acquires the
oldest/free slot using the selected sequence, publishes its sequence, initializes
position/yaw and selects one of three authored actions. The neighboring exact
debris constructor repeats acquisition/publication and transform setup. Test
those complete operations as small ordinary inline helpers, and a local pool
request containing the actual first index and sequence pointer. Preserve byte
IDs, both sequence fields, five ordered calls and all initialization stores.

`map_world_state_persist` is a byte-stream writer with fixed event records,
sparse actor/object records and two tail formats. Its three callers consume no
return and it calls no function. Test a per-event writer returning the advanced
cursor, byte counts maintained at their reserved output positions, and direct
record indexing followed by one cursor advance. These are conventional save
format implementations. Preserve stream order, sparse selection, all narrowing
and the existing packed object-link representation. A helper must inline and
use every output; it must not manufacture an unused live quantity.

## Entity rendering hypotheses

`render_entities` has five ordered pool passes, one frame caller, nine retail
calls and twenty-five validated references including one internal jump.
Try one consistent countdown idiom across the passes, direct end-pointer
traversal of the four fixed arrays, and a complete floor-item drawing stage.
Retain the wrapped halfword culling calculations, per-item window reloads,
special actor square test and all pass/lighting order. The authentic SDK
`setRGB0` is also applicable to the actual CVECTOR used for floor-item colour;
test it as a normal API spelling, without changing that object's fourth byte.

That RGB hypothesis fails the authentic header contract: `setRGB0` accesses
packet `r0/g0/b0`, whereas CVECTOR has `r/g/b`. The compiler rejects the
control; no cast or replacement SDK type is introduced. The other entity
traversal trials remain valid independent controls.

## Horizontal movement hypotheses

The three retail movement call sites supply a signed heading and nonnegative
forward/strafe magnitude, but preserve the existing full-word signature and
arithmetic. The operation has seven calls and forty-two validated references
including nine internal jumps. Its two geometry phases are collision deflection
and diagonal-wall sliding. Test the usual length-times-trigonometric-component
spelling, small axis-projection helpers, and one real scaled distance reused
within each selected diagonal shape. Preserve negation before the signed Q12
shift, conditional multiplication only for the four diagonal cases, the two-hit
retry behavior, Z-before-X grid update and the constant return one. These tests
do not repeat the prior declaration-scope or grid-pointer controls.

## Interaction and player launch hypotheses

`map_interaction_dispatch` has one player caller, 56 proven outgoing control
references and 47 validated references, including internal jumps and the two
dispatch tables.
The two container cases repeat the complete pickup/result/notification
operation. Test that shared inline operation, then a complete four-slot pickup
pass returning whether it found an item. Preserve the hinged container's
retail precheck of slot zero on all four iterations. A separate helper can
express the two-sided facing test shared by lift and hinged doors. Preserve
short circuiting, byte item IDs, signed halfword indices and all notifications.
The repeated behavior loads in the paired-door search remain evidence; no
volatile qualifier or artificial clobber is justified.

`player_update` retains 66 calls and 210 ordered address pairs. Its 224-byte
retail frame exceeds the source frame by eight bytes; a second residue occurs
in the Triple Fang setup after the first effect constructor. Test complete
inline operations for projecting the weapon's local muzzle offset into world
space and emitting the two additional Triple Fang projectiles. All real SDK
objects, camera reads, eight-argument constructor calls, sound policy and
position/pitch changes must survive. Earlier lifetime and copied camera-owner
controls are negative evidence; neither storage padding nor another pointer
spelling is a new source hypothesis.

## Outcomes against the frozen source

Seventeen fresh unchanged controls and forty-four additional source attempts
produced no new exact function. Forty-three alternatives compiled; one SDK
negative control failed because its macro requires a packet rather than a
CVECTOR. All sixty compiled complete-unit controls/alternatives preserved
every sibling body from their respective baseline. No campaign source or
inventory change is retained, and no new function is banked.

These are tests of complete source operations. They do not establish the
original helper boundaries or attribute an optimizer mechanism. Compilation
uses each unit's manifest profile: `probe-gcc257-o2-g8` for player update and
dialogue, and `probe-gcc257-o2-g0` for the other selected units. Both use the
pinned GCC 2.5.7 compiler and ASPSX 1.07 assembler behavior. No profile is changed
or promoted to historical compiler attribution.

**Horizontal movement.** The ordinary axis-projection helpers reproduce the
baseline. Length-times-direction changes two more aligned words, while the
two real shared diagonal-distance forms lose the existing instruction shape.
Selecting the diagonal sign yields the retail 2,088-byte extent but changes
420 aligned words. Equal extent alone does not justify that source. Preserve
the existing signed arithmetic and Z-before-X update sequence.

**Player update.** The complete muzzle operation still has a 216-byte frame,
adds differences before the projection at `+d34`, and changes ordered camera
references. The separate Triple Fang pair and complete projectile operation
shorten the body to 6,680 and 6,668 bytes and disturb otherwise matched regions.
None explains the eight-byte retail frame interval or the `+fa0` setup.

**Map-cell rendering.** The fresh comparison has eighteen unequal stack
immediates: a 120-byte retail frame versus 88 source bytes, displaced position
and flag locals, and the corresponding saved registers. All other words,
nine calls and ten ordered address pairs agree. Earlier real MATRIX, SVECTOR,
flag aggregation and transform helpers already tested the supported object
set. No new object is inferred from the unused 32-byte interval, so this pass
records an evidence-limited verdict without another source trial.

**Entity rendering.** A uniform countdown across the five passes is byte
identical to the baseline. Fixed-array end cursors and a complete floor-item
pass change the body and ordered references. The SDK colour attempt is rejected
at the authentic member contract. The baseline's thirty differing words
remain register operands; no new traversal or colour model is kept.

**Item database loading.** The decimal-triple formatter is byte identical.
An I001-through-I080 loop expands the body, while the typed six-bank resource
image removes the retail copy setup from `+a4` and produces only 1,376 bytes.
Its payload layout is real, but that pointer organization does not explain
this function. The baseline retains sixteen register-operand differences in
the item path division and byte stores, beginning at `+4bc`.

**Three tiled menu functions.** Complete four-tile requests using a screen
RECT, screen plus texture RECTs, or the existing sprite request introduce
substantial packet loads, frame changes and longer bodies. Every request
field is consumed; none accounts for the retail frame while retaining the
instruction sequence. The frozen status panel, item-name frame and backdrop
retain respectively sixteen, twelve and twelve stack-immediate differences.
Their frames are 112/48, 224/160 and 104/40 retail/source bytes. The unexplained
64 bytes in each routine are not proof of four local packet objects.

**Item detail.** The exact neighboring preview motivates a complete name row
and normal positioned glyph string. That spelling raises the displayed score
from 97.814210% to 99.672134%, but increases unequal aligned words from two to
nine and introduces its first difference at `+e0` instead of `+ec`. The frozen
body differs only in the order of zeroing the index and forming the positioned
string address at `+ec`/`+f4`. Reusing one glyph-row destination, extracting the
model stage, composing model/name stages and indexing the whole workspace do
not close it. The old workspace union is not established as an original source
fact by this result; the concurrent type cleanup removes it independently.

**Dialogue and map image paths.** Inline and comma-expression decimal-pair
formatters reproduce their baselines. Real two-character temporaries add
instructions. Dialogue retains eight register-operand differences, starting
at its remainder read `+30`. Map image retains a four-byte excess and different
floor/path address order from `+48`, plus its remainder register from `+30`.
The signed quotient/remainder and all template bytes stay intact.

**Map-object creation.** A complete drop initializer is byte identical;
acquisition/publication keeps the same score and fifty-five unequal retail
words, and a real pool request grows the body to 428 bytes. The baseline
400-byte source lacks one retail saved-register pair, gains a load-delay NOP
while publishing the sequence, and ends four bytes short. No new ownership
or initialization correction is supported.

**Interaction.** The shared item-offer operation is byte identical. The
complete container pass adds four bytes; the facing helper adds thirty-two.
Both leave the position/sentinel register roles different and preserve only
one load of the paired-door behavior where retail has three. All resolved
external calls and thirteen ordered address pairs survive these alternatives.
The slot-zero precheck is preserved. Do not force the missing reloads.

**World persistence.** Returning the advanced cursor from an event writer
and assigning the seven event bytes by index produce the same 704-byte body,
with new differences from `+8`. Maintaining counts in the stream yields 712
bytes and a new divergence at `+bc`. The frozen 700-byte body first differs at
`+16c`, where the source keeps one more behavior constant live across the
object pass. The seven ordered referents agree. No save-format change is kept.

**Effect map collision.** Exactly one of 475 words differs: retail has a NOP
at `+718`, while source places its default result-one assignment in that
branch delay slot. Both have the same masked target dispatch and return
behavior. Earlier complete result/switch/geometry controls cover the supported
source alternatives. No new correction justifies deleting the fallback or
introducing an undefined result. This pass adds only the fresh control.

**Orbiting projectile.** All ten differences are stack allocation/save/restore
immediates, with a 120-byte retail frame versus 56 source bytes. The complete
motion, collision, damage, sound and phase operations otherwise agree,
including all nine calls and five address pairs. Earlier motion/distance
helpers and real vector objects did not explain the missing 64 bytes. A
matrix-shaped hole does not establish an actual matrix computation; this
pass adds only the fresh control.

**Effect dispatcher.** Forty-two of 1,539 words differ, all in GPR operands;
every non-register bit, sixty-nine resolved direct calls and twenty-two
ordered address pairs agree. Complete effect-kind policy and real record
owners remain the source model. Earlier whole SDK operations, countdowns and
scatter/spawner controls did not improve it. This pass finds no new evidenced
operation to replace and records the unchanged fresh control.

## Reproduction and limits

The selection, original complete units, six-view dossiers, source history,
frozen objdiff JSON and baseline full build are under `build/era/`. Each trial
directory contains the full candidate unit, source diff, stated hypothesis,
compiled object, strict score, target/candidate relocations, resolved words
for every unit claim, and a retail-relative disassembly diff. Every target
claim is independently asserted equal to the hash-validated GAME.EXE bytes
after resolving relocations; sibling comparisons use those resolved records.
The hand-selected drivers are `menu_detail.py`, `menu_tiles.py`,
`resource_paths.py`, `map_operations.py`, `entities.py`, `movement.py`,
`interaction.py` and `player_launch.py`. They run inside `nix develop` with
`PYTHONPATH=.`. No generated candidate, object or audit is committed.

The table below records the frozen experiment, not a claim that concurrent
source improvements should be rolled back. Unequal-word counts compare
aligned offsets and therefore include downstream displacement after inserted
instructions. Ordered-referent equality is checked separately from calls;
some unchanged baselines already have a different address-pair order. A
higher fuzzy percentage does not resolve either issue.

## Frozen function ledger

| GAME function / VA | Alternatives | Strict baseline % | Body retail/source B | First unequal offset | Unequal words |
| --- | ---: | ---: | ---: | ---: | ---: |
| `player_move_horizontal` / `800171fc` | 4 | 96.568960 | 2088/2128 | `0x2c` | 468 |
| `player_update` / `80018880` | 3 | 99.694790 | 6684/6684 | `0x0` | 63 |
| `render_map_cell` / `8001e5ec` | 0 | 99.878380 | 592/592 | `0x0` | 18 |
| `render_entities` / `8001f218` | 4 | 99.517044 | 1408/1408 | `0x5c` | 30 |
| `item_load_database` / `80020cfc` | 3 | 99.746666 | 1500/1500 | `0x4bc` | 16 |
| `menu_status_panel` / `8002430c` | 3 | 99.962170 | 1692/1692 | `0x0` | 16 |
| `menu_draw_item_detail` / `80027b7c` | 6 | 97.814210 | 732/732 | `0xec` | 2 |
| `menu_draw_item_name_frame` / `800292f8` | 3 | 99.975710 | 1976/1976 | `0x0` | 12 |
| `menu_draw_window_backdrop` / `8002a510` | 3 | 99.971760 | 1700/1700 | `0x0` | 12 |
| `talk_show_dialogue_page` / `8002c9d4` | 3 | 98.780490 | 164/164 | `0x30` | 8 |
| `map_object_spawn_effect` / `80031834` | 3 | 94.504950 | 404/400 | `0x4` | 55 |
| `map_show_screen_image` / `80034d54` | 3 | 88.888885 | 144/148 | `0x30` | 20 |
| `map_interaction_dispatch` / `80034de4` | 3 | 99.202774 | 2308/2296 | `0x2c` | 166 |
| `map_world_state_persist` / `80035b5c` | 3 | 97.528730 | 696/700 | `0x16c` | 84 |
| `effect_map_collision` / `80037850` | 0 | 99.873690 | 1900/1900 | `0x718` | 1 |
| `effect_projectile_update_2d` / `80038298` | 0 | 99.934210 | 608/608 | `0x0` | 10 |
| `effect_update_dispatch` / `80038a38` | 0 | 99.827810 | 6156/6156 | `0x38` | 42 |

## Additional source attempt ledger

All rows below are rejected or byte-identical controls. “Same” compares the
complete resolved candidate record with its frozen baseline, not with retail.

| Trial | Strict % | Body B | First unequal offset | Unequal words | Same as baseline |
| --- | ---: | ---: | ---: | ---: | --- |
| `01-axis-projection-helpers` | 96.568960 | 2128 | `0x2c` | 468 | yes |
| `01-length-times-direction` | 96.492340 | 2128 | `0x2c` | 470 | no |
| `01-scaled-diagonal-step` | 89.670500 | 2104 | `0x2c` | 470 | no |
| `01-signed-diagonal-step` | 89.706894 | 2088 | `0x2c` | 420 | no |
| `02-triple-fang-pair-operation` | 99.471570 | 6680 | `0x0` | 1027 | no |
| `02-weapon-muzzle-operation` | 99.415920 | 6684 | `0x0` | 85 | no |
| `02-weapon-projectile-operation` | 99.066430 | 6668 | `0x0` | 1600 | no |
| `04-fixed-pool-cursors` | 89.105110 | 1388 | `0x4` | 274 | no |
| `04-floor-item-pass` | 96.724434 | 1412 | `0x5c` | 212 | no |
| `04-sdk-floor-colour` | compile rejected | — | — | — | — |
| `04-uniform-countdown` | 99.517044 | 1408 | `0x5c` | 30 | yes |
| `05-database-image` | 86.336000 | 1376 | `0xa4` | 333 | no |
| `05-decimal-triple-helper` | 99.746666 | 1500 | `0x4bc` | 16 | yes |
| `05-file-number-loop` | 95.696000 | 1532 | `0x0` | 356 | no |
| `06-screen-rect` | 73.817970 | 1948 | `0x0` | 455 | no |
| `06-screen-texture-rects` | 51.765957 | 1932 | `0x0` | 460 | no |
| `06-sprite-request` | 53.768322 | 1908 | `0x0` | 458 | no |
| `07-model-and-name-stages` | 94.573770 | 728 | `0x4` | 63 | no |
| `07-model-stage` | 92.748634 | 728 | `0x4` | 56 | no |
| `07-one-glyph-row` | 94.491806 | 740 | `0x0` | 185 | no |
| `07-preview-body` | 99.672134 | 732 | `0xe0` | 9 | no |
| `07-preview-name-stage` | 99.672134 | 732 | `0xe0` | 9 | no |
| `07-text-after-position` | 99.169395 | 732 | `0xec` | 6 | no |
| `08-screen-rect` | 74.767204 | 2248 | `0x0` | 544 | no |
| `08-screen-texture-rects` | 57.222670 | 2232 | `0x0` | 541 | no |
| `08-sprite-request` | 60.127530 | 2208 | `0x0` | 534 | no |
| `09-screen-rect` | 72.557650 | 1968 | `0x0` | 468 | no |
| `09-screen-texture-rects` | 51.844707 | 1952 | `0x0` | 465 | no |
| `09-sprite-request` | 55.590588 | 1928 | `0x0` | 458 | no |
| `10-formatted-character-pair` | 89.536580 | 172 | `0x34` | 19 | no |
| `10-repeated-pair-helper` | 98.780490 | 164 | `0x30` | 8 | yes |
| `10-repeated-pair-macro` | 98.780490 | 164 | `0x30` | 8 | yes |
| `11-acquire-and-publish` | 94.504950 | 400 | `0x4` | 55 | no |
| `11-initialize-drop` | 94.504950 | 400 | `0x4` | 55 | yes |
| `11-pool-request` | 86.811880 | 428 | `0x0` | 104 | no |
| `12-decimal-pair-helper` | 88.888885 | 148 | `0x30` | 20 | yes |
| `12-decimal-pair-macro` | 88.888885 | 148 | `0x30` | 20 | yes |
| `12-formatted-character-pair` | 82.638885 | 156 | `0x34` | 22 | no |
| `13-shared-container-pass` | 98.786830 | 2300 | `0x2c` | 287 | no |
| `13-shared-door-facing` | 97.781630 | 2328 | `0x2c` | 228 | no |
| `13-shared-item-offer` | 99.202774 | 2296 | `0x2c` | 166 | yes |
| `14-count-in-stream` | 91.844826 | 712 | `0xbc` | 116 | no |
| `14-event-record-fields` | 93.655174 | 704 | `0x8` | 153 | no |
| `14-event-record-writer` | 93.655174 | 704 | `0x8` | 153 | no |

## Integration and verification

The final verification base is master `365e259e`, including the concurrent
type cleanup, Sony rotation helpers and structured-control-flow changes.
Freshly rebuilding those affected units and auditing all seventeen selected
functions preserves fifteen frozen resolved bodies. Two current bodies
intentionally differ from this experiment's starting point:

- `player_update` is now 99.967090%, with fifty-five stack-related unequal
  words and no remaining Triple Fang operation-order difference. Preserve
  the independently verified changes in
  [sony-helper-near-misses.md](sony-helper-near-misses.md).
- `menu_draw_item_detail` uses a normal `MenuGlyphString` after the concurrent
  removal of its workspace union. It has three unequal words at 97.808750%,
  with the same calls and ordered referents. Preserve the supported object
  model despite the small score change; the two-word frozen control is not
  grounds to restore the union.

The current all-function GAME report has 352 exact rows, including seven
vendored controls: **345/362 game functions exact**, unchanged from selection.
Every previously exact GAME row remains exact. The thirteen additional
control-flow cleanup commits also preserve the selected resolved bodies
relative to the preceding `48ff2dc8` verification.

Full `kf build` runs after the fresh focused compiles and again after the
concurrent source changes. Code counts are PSX 1/1, GAME 345/362 and OPEN
107/108; all seven GAME and six OPEN vendored source controls remain exact.
The build still exits nonzero for the baseline data/ownership/placement
findings: data owners match 1/1, 30/44 and 11/20; target relink verifies 1/1,
68/77 and 33/38 units respectively. The four complete GAME/OPEN config-data
contributions independently match. No ratcheted cleanliness metric regresses.
These results do not claim complete executable or data closure.

`ruff check scripts tests`, the 769-test repository suite (nine skips), and
`git diff --check` pass. Only this report and its pattern-index entry belong
to the campaign commit. No source, inventory or bank floor is changed by this
campaign, and concurrent working changes are preserved during integration.
