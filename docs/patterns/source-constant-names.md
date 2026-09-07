# Source constant naming

The [player-update review](game-player-update-constants.md) names movement
fractions, pitch bounds, weapon-magic power/timing and projectile geometry,
then propagates the Moonlight projectile and map hazard identities through
their effect, interaction and rendering consumers. It removes 65 inline
occurrences. Its complete ledger covers all 168 retained literals in the
module, including authored data; the refreshed map-script ledger covers 272.

The [complete player-core review](game-player-core-constants.md) names weapon
charge/phase units, hit windows and collision geometry, then propagates shared
values through input, rendering and actor attacks. It removes 116 inline
occurrences across the family. The core and motion ledgers cover all 80
remaining numeric/character literals in the fourteen-function core module,
including both initialized data claims.

The [player vertical-state review](game-player-vertical-state.md) types the
saved byte as grounded/falling/step-up, propagates the shared camera height,
and names motion/bob parameters and the bottomless-pit attribute. It replaces
26 inline occurrences and documents all seven remaining literals in the three
sync/bob/vertical functions. The complete player-death module ledger now covers
197 remaining occurrences after naming its initial state and camera offset.

The [actor animation-table slot review](game-actor-animation-slots.md) separates
table indices, resource animation IDs and action codes. It replaces the false
general action-minus-two mapping and documents the remaining inline resource
IDs without assigning unsupported spell or creature identities.

The [actor state review](game-actor-states.md) names the action, lifecycle,
vertical, collision, slot-policy and progress values across their consumers.
It explains the remaining progress counters and retains the distinction
between automatic-selection locks, completed actions and unavailable animations.

The [actor constant review](game-actor-constants.md) covers all three actor
modules, including phase limits, sound/parameter slots, collision policies,
damage units and packed flags. Its inventory at `01c741c` explains 517 retained literal
occurrences without claiming their original tuning rationale or unsupported
effect identities.

The [actor motion-limit review](game-actor-motion-limits.md) promotes four
of those parameters to names: drift acceleration, per-axis speed, yaw speed,
and the exclusive home-position tolerance. It gives their measured units
and distinguishes the progress byte's state and counter roles.

The [effect protocol review](game-effect-protocol.md) names eight decoded
behaviors across construction, dispatch and callers, plus collision targets,
power/render/homing selectors, branch roles and floor phases. It distinguishes
equal-valued constants with different consumers, including actor damage unity
and player Q12 scaling. The [effect literal audit](game-effect-literals.md)
extends this to projectile/hazard phases and explains all 764 retained literals
in the five effect/magic modules, including the corrected collision-probe
offsets formerly called velocities. The wider source audit remains open.

The [map-object review](game-map-object-constants.md) separates definition
behaviors from running actions, names switch phases and drop groups across
their consumers, and explains all 259 retained literals in the two map-object
modules. It preserves the paired door's missing action-1 handler and the
switch's unsigned reverse-animation completion test.

The [map-event literal review](game-map-event-literals.md) covers the event
query/runtime and map-load modules. It recovers the floor-5 animation-slot
reset, shares the saved-record format constants, and gives a specific reason
for all 145 remaining literal occurrences in its linked ledger. Movement,
random-turn and sound parameters retain their measured units without claiming
an original tuning rationale.

The [saved floor-script review](game-map-script-state.md) replaces four
address-derived aliases and four raw prefix-byte accesses with typed fields.
It names area-trigger stages, binary flags and the four-reveal completion
count, and propagates the saved-world type through the memory-card payload.
The two earlier literal censuses above are refreshed after these substitutions.

The [floor-script literal review](game-map-script-literals.md) accounts for
all 316 retained occurrences in `map_scripts.c`, down from 363. Cutscene
phases and menu modes have stored enum types; interaction padding, facing
tolerances and container limits have named domains. Authored movement rates,
fade durations, resource IDs and path digits have per-occurrence reasons.

The [asset and animation-cache review](game-asset-animation-layout.md) types
the free/stale/live state, recovers the header's animation-clip count from
shipped table extents, and propagates asset-header pointers. It explains all
36 retained literals in `pool.c` and `asset_registry.c`; vertex allocation
uses the SDK element size and the weapon buffer has a named byte capacity.

The [pickup outcome review](game-item-pickup-outcomes.md) corrects the
pickup-confirmation identity, types its result through all pickup consumers,
names the shared player stack limit and replaces raw label arrays with
positioned glyph objects. It explains all 50 retained literals in pickup and
the mode dispatcher, and removes six numeric outcomes from the map ledger.

The [equipment domain review](game-equipment-domains.md) separates menu
categories from player slots, types both APIs, names empty item/spell/texture
selections, and shares record extents and menu ranges. Its three-module
ledger explains all 120 retained literals after the type and menu follow-ups,
down from 209; other consumers
also replace the shared empty values and armor-record origin.

The [shared confirmation review](game-menu-confirmation-protocol.md) types
footer choices, preview modes and accepted/cancelled outcomes through all
seven callers. It explains all 65 remaining widget literals and decodes the
retail Japanese labels. The two positioned labels use the existing shared
structure, recovering the retail stack spacing without pointer casts.

The [preview transform review](game-menu-preview-transform.md) replaces the
interior yaw alias with its complete SDK rotation vector and names angle-rate,
wrapping and Q12 lighting domains. Its six-function and initializer ledger
explains 192 retained occurrences after the shop, drawing-layer and dimension
follow-ups,
down from 236, and distinguishes draw-call
rates from real-time frequency.

The [shop price review](game-shop-price-domains.md) types buy/sell selection
through every price-preview consumer, names the shop-window rows, and decodes
Gold Cross and Key of the Dead from the retail item names. It propagates those
IDs through their actual consumers and explains all 139 retained shop literals,
down from 188. Shop bank IDs remain indices rather than invented merchant names.

The [displayed class-title review](game-menu-class-titles.md) decodes the
nine four-cell title groups selected by base physical power and magic. It
corrects the old color/rating interpretation in both status renderers, shares
their threshold and atlas constants, and explains all 413 remaining numeric
occurrences in the [status-text ledger](game-menu-status-literal-ledger.md).

The [menu drawing-layer review](game-menu-drawing-layers.md) shares widget,
window and background buckets, names the passive-panel input-release frame,
and separates map scale from layer indices. Its
[three-file ledger](game-menu-panel-literal-ledger.md) explains all 190
retained occurrences in the status panel, map viewer and list renderer;
the existing preview ledger drops eight newly named depth occurrences.

The [window-kind review](game-menu-window-kinds.md) decodes the loaded layout
bank and propagates its enum through drawing, prompts and list initialization.
It distinguishes the System window from the unreferenced Save/Load record,
replaces 37 source literals, and explains the 15 retained occurrences after the
dimension follow-up in the
[window/list initializer ledger](game-menu-window-literal-ledger.md).

The [save-result review](game-menu-save-results.md) propagates typed
confirmation results through the prompt and save/load panels, while keeping
the root menu's item IDs and negative controls distinct. It names window
rows and five decoded message textures, replaces 84 numeric uses, and
explains all 140 remaining literals in the
[save/confirmation ledger](game-menu-save-literal-ledger.md).

The [root-choice and panel-phase review](game-menu-choice-state.md) keeps typed
choices through root and equipment dispatch, names configuration bindings and
row boundaries, and shares heading identities with list callers. It replaces
61 numeric uses and explains all 96 retained occurrences after the list-result
follow-up in its complete
[four-function ledger](game-menu-choice-literal-ledger.md), while preserving
the configuration renderer's distinction between exact one and other values.

The [list-result and recovery review](game-menu-list-results.md) names the
pending/no-selection controls without conflating integer item IDs, spell IDs
and list rows. It also names five item HP additions and Bless's MAGIC-to-HP
multiplier, replacing 43 numeric uses. Its new
[item-use/discard ledger](game-menu-item-list-literal-ledger.md) explains 101
retained literals and refreshes the equipment, spell and root-choice ledgers.

The [item and menu dimension review](game-item-menu-dimensions.md) shares the
item-table, shop-column, stock-bank and glyph-row extents across definitions,
saved state and consumers. It derives startup clear lengths from their owners
and reuses the CD sector units. Its
[database/startup ledger](game-item-database-literal-ledger.md) explains all
120 retained numeric uses in those two functions; six existing ledgers are
refreshed, including 94 retained occurrences in the spell ledger.

The [player progression review](game-player-progression.md) names power,
HP/MP, experience and level limits, the training threshold and base-magic
learning requirements. It separates the forty loaded growth rows from the
fifty shipped rows and derives the extrapolation indices. Its complete
[four-function ledger](game-player-progression-literal-ledger.md) explains
45 retained numeric occurrences, down from 63; the shared Verdite threshold
also reduces the existing item-use ledger from 80 to 79.

The [player damage-unit review](game-player-damage-units.md) separates
intermediate damage tenths from multiplier tenths, names the poison roll
conversion and restart override, and propagates Q12/multiplier unity through
direct callers. Its [remainder ledger](game-player-death-literal-ledger.md)
covers 56 occurrences; together with startup 98 and progression 45, all 199
remaining inline numeric occurrences in `player_death.c` have specific reasons.

## Function Match Plan

The campaign covers numeric and character literals in every C source under
`src/`. The initial census contains 13,237 literal occurrences: 1,250 retail
address/size claims, 40 macro-definition values, and 11,947 other occurrences.
Comments and string contents are not counted as C literal expressions.

Name constants according to their actual domain: angle units, fixed-point
fractions, object states, IDs, units, flags, and limits. Equal values in
different domains need different names. Preserve types, expressions, operation
order, initializer contents, claims, and all ordered relocation targets.
Document literal expressions retained inline with their specific reason.

A name must add a supported distinction or make a contract easier to use.
Repeating the assignment's field name in an alias does not explain a tuning
value. For animation and other rates, explain the units, fraction of a full
range and resulting duration or displacement where the code supports them.
Distinguish those measured consequences from the designer's rationale; leave
an unexplained authored parameter explicit and document that uncertainty.

The pre-edit snapshot covers all 477 source claims in the 112 compiled units:
image/VA, raw disassembly and CFG, callers/callees, strings, references,
adjacency, source history, and current strict objdiff results. Each focused
replacement batch is rebuilt and compared with these baseline objects and
reports. No new matching or historical-toolchain attribution is intended.

## Math and packed-color batch

GAME 80014a64..800150a8 and OPEN 800158d0..80015dd4 contain the shared rotation,
angle, and vector operations. GAME 800202fc..800204c0 and OPEN
80019598..800197e4 contain matrix/fog/color interpolation. Their immediate
instructions distinguish the following domains:

- Twelve fractional bits and fixed-point unity belong to vector scaling,
  matrix coefficients, interpolation weights, and the ratio passed to `catan`.
- Quarter, half, three-quarter and full turns, and the low-twelve-bit wrap mask,
  belong to angle comparisons and trigonometry. The half-turn predicate's
  exclusive upper bound remains `half turn + 1`, preserving the original test.
- The three-bit pre-square reduction in `fixed_vector2_length` reduces the
  magnitude before 32-bit multiplication; the final shift restores the scale.
- RGB555 uses five-bit channel masks, green at bit 5, blue at bit 10, and the
  STP bit at bit 15. Interpolation retains STP from the first input.
- Matrix interpolation traverses nine rotation coefficients. Its counter is
  initialized to `element count - 1` and terminates at -1.
- Fog setup passes projection distance 200 to `SetFogNear`, consistent with
  the default `SetGeomScreen` calls in GAME and OPEN rendering.

The remaining literals in this batch are deliberately inline:

| Sites | Values | Reason |
| --- | --- | --- |
| `ADDRESS` and `RODATA` claims in all nine files | Addresses and byte extents | These are the literal retail ownership evidence consumed by the build; introducing aliases would obscure the claims. |
| Matrix elements and raw vector component accesses | Indices 0, 1, 2 | These are row/column coordinates or X/Y/Z component positions. Numeric coordinates make the small matrix formulas directly readable. |
| Axis-rotation matrices and initial forward vector | Zero assignments | Zero denotes the absent cross-axis coefficient/component in the mathematical formula. |
| `vector_xz_to_angle` comparisons | Zero | The sign boundary selects quadrants; it is the arithmetic origin, not a game state. |
| Half-turn exclusive bounds and matrix countdowns | 1 in `+ 1`, `- 1`, and `-1` | Unit adjustment expresses the inclusive endpoint or countdown sentinel directly. |
| OPEN `debug_matrix_label` | Array extent 16 | Preserve the explicit storage extent and trailing zero bytes in the claimed read-only data; it is not a tunable display limit. |

All nine units retain their strict 100% function results, and the complete
objdiff report is unchanged. Of the 112 objects, 109 are wholly byte-identical
to the original baseline. Compiling saved source controls for the other three
(`game.matrix`, `game.color_lerp_rgb555`, `open.matrix`) shows that only
`.debug_line` differs after adding direct includes. Instructions, constants,
relocations, symbols, and all other sections are identical.

The remaining source batches are still in progress.

## Map units batch

The related campaign is the shared map-grid consumer family in GAME and OPEN:
placement loaders, world/cell conversion, map geometry rendering, collision,
floor lookup, and restoration. The resource loader copies 100-by-100 byte
grids. World X/Z use 2000 units per cell; floor-height bytes use 100 world units
per step with their existing sign conversion. Tile centers are at offset 1000.
These facts are independently repeated in the placement loader multiply/add
chains, renderer offsets, and collision quotient/remainder operations.

Use shared constants for these conversions and dimensions. Do not conflate
them with the same-valued lighting coefficients, menu ordering-table depth,
actor attack distances, texture-bank size, or table initializer data. Those
sites remain assigned to their own domain review. The neighboring-cell and
diagonal-half-cell tests retain their original signedness, inequalities,
operation order and constants after substitution.

The query at GAME 8001a5ac defines the exclusion flags, target-capture flag,
height-ignore sentinel, player collision radius, and result tags. Its actor
and player distance callees independently check the same height sentinel.
`effect_update_dispatch` treats every result other than -1 as an impact, then
uses the high halfword to distinguish the actor and player. Therefore the
effect-map helper's 0x10000 return denotes terrain contact; its old comment
calling that value "no collision" was wrong.

Grid shape 1 is ordinary traversable floor; shape 6 is also traversable and
adds a 300-unit half-cell step in the floor-height helper. The occupancy update
at GAME 8001a4e8 visits a 5-by-5 neighborhood centered two cells before the
input, maintains the low five bits, and preserves the high three. The query's
rejection mask is separately 0xf0, which overlaps the high occupancy bit; the
names retain this actual behavior rather than normalizing the masks.

The four diagonal cell IDs are named for their traversable local X/Z
half-planes. `player_move_horizontal` (GAME 800171fc) projects invalid positions
back onto `x = z` or `x + z = cell size`, with the corresponding inequality
for each ID. Serialized orientations 1..4 select the zero, quarter-, half-,
and three-quarter-turn matrices built by both renderer initializers.

The height lookup initializer in `collision.c` remains literal data: negative
entries are measured floor-relative heights, nonnegative entries index the
seven adjacent rectangle records, and zero-filled tail entries preserve the
retail table. Inventing asset names for these rows is not justified by the
current identities. The table's 255-element extent covers all attribute bytes
except the 0xff sentinel. Numeric zero in flat grid accesses denotes the base
row used for flattened indexing; zero comparisons test empty masks, arithmetic
signs, or absence of vertical extent. Unit increments and the `cell size + 1`
inclusive boundary remain inline arithmetic.

The compiled-control comparison covers all 112 objects. Only `.debug_line`
differs in 20 objects; every other section, including relocations and the
complete initialized collision tables, is identical. Individual verdicts for
all 477 source claims retain their prior strict percentages (392 exact).
The initializer test accepts a named array bound while retaining its exact
payload-size and SHA-256 checks. Lint, `nix flake check -L`, and all 634 tests
pass (nine local prerequisites skipped); full `kf build` still reports the pre-existing data
ownership/placement failures. Other constant domains remain under review.

## Concurrent dispatcher updates

Master commits `534a103` and `0674985` were brought into the naming worktree
after the map batch. Conflict resolution starts with the incoming dispatcher
and applies only field and map-unit spellings; reversing those substitutions
reproduces the incoming source exactly. A focused control compiles that source
with its original effect header, then compares the resolved unit: only
`.debug_line` differs. The improved strict result, 96.93957%, is retained.
This integration brings master into the worktree; the full naming campaign
has not yet been integrated into master.

## Animation cache, camera, and audio plan

The next related batch follows the animation-cache lifecycle, the GAME/OPEN
camera interpolators, and their shared angle/distance operations in spatial
audio. The existing per-function dossiers cover their complete claims and
references. The comparison baseline is refreshed at `893a3fa`, including the
incoming dispatcher improvements. Replace only established states, capacities,
fixed-point scales, angle units, and audio settings; use the pinned LIBSND
spellings for SDK modes. Preserve the GAME tone-bit comparison exactly even
though testing `(tone & 0x80) == 1` cannot succeed. No behavior correction is
part of this batch.

Check each rebuilt object's non-debug sections and strict function results
against this baseline. Header array bounds may use named integer constants;
the inventory must resolve them to the same physical extents and reject
unresolved bounds rather than omit fields.

The animation cache has twelve records: reset/release stores free (0), the
per-frame mark stores stale (1), and a successful bind stores live (2). The
next stale sweep frees only untouched records. Clip 0xff invalidates cached
keyframe selection; the pointer-valued return 1 reports a static asset. These
contracts are now named in `pool.h`. The clip's unused header field retains
its unresolved spelling. The later VDF format audit in
`semantic-field-names.md` identifies the morph's object selector.

Both camera interpolators keep four fractional bits, reduce coordinates by
three bits before squaring, and restore the resulting distance scale. The
serialized X terminator and the runtime finished-frame sentinel both encode
-1, with separate names for the two roles. Shortest-angle helpers retain the
asymmetric half-turn endpoints exactly.

Audio keeps ten reusable voice slots, two sequence table entries and one
track per sequence. GAME allocates 0x3000 sequence bytes, plays sequences at
volume 75, uses studio-C reverb with depth 16, and divides panning products by
3000. OPEN allocates 0x4800 bytes, uses volume 80, hall reverb depth 48, waits
100 VSync calls after VAB transfer, and fades master volume by +8/-4. Spatial
defaults are a 16000-unit cutoff and 28000-unit attenuation distance. These
are observed settings, not claims about original author identifiers. The
attenuation ratio has seven fractional bits; GAME narrows panning above 64,
while OPEN equalizes its channels at 96. GAME's unreachable tone-bit branch
still contains its original +36 adjustment.

The pinned `psyq/include/LIBSND.H` defines `SS_TICK60`, the two reverb modes,
`SSPLAY_PLAY`, `SSPLAY_INFINITY`, and `SS_WAIT_COMPLETED` with the exact retail
arguments. VAB -1 distinguishes automatic slot selection at open from an
unavailable result/closed handle; voice -1 means an inactive reusable slot.
Header array dimensions are resolved from explicit integer enumerators and
remain numeric physical extents in the curated structure inventory.

The remaining 133 inline literals in these functions have these reasons.
`opening_poll_input` is outside this batch; its six input-domain literals
remain assigned to the controller audit.

| Sites | Values | Reason for retaining inline |
| --- | --- | --- |
| Camera begin functions | Index/assignment 0 | Select the first serialized point and initialize the zero-based point index. |
| Shortest-angle helpers | `+ 1` | Inclusive-endpoint adjustment to the named negative half-turn boundary. |
| Pool trailing arrays | Extent 1 | C89 variable-tail declaration idiom for keyframe offsets, morph indices and vertex deltas; this is not a one-element resource limit. |
| Pool pointer/animation-data checks and stores | 0 | Null pointer or absent animation-data offset; no lifecycle state is represented by these sites. |
| Pool countdown loops | 0, -1, `capacity - 1` | Exhaustion and last zero-based index in their existing pre/post-decrement loop forms. |
| Pool phase accumulation and reverse predicate | 0 | Start of the duration sum and a Boolean reverse flag. |
| Pool model binding | Object index 0 | The first TMD object is selected; there is no established asset-specific identity for it. |
| Pool allocation | Shift 3 | Convert vertex count to bytes for eight-byte SDK `SVECTOR`; retaining the shift preserves the existing integer expression and promotions. |
| Pool scratch blend | Indices 0/1 and `vertex_count + 1` | Two 32-bit lanes form one SDK vector; the scratch's leading vector temporarily overlays the morph header and is restored after blending one extra vector. |
| Audio sequence-active and OPEN fade flags | 0/1 | Boolean clear/set/test, including the explicit request for immediate stop. |
| Audio pointer tests/stores | 0 | Null optional listener transform or cleared VAB header. |
| Audio volume initialization, bounds and mute calls | 0 | Silence and the arithmetic lower bound of volume/fade counters. |
| Audio playback returns | 0/1 | Boolean result: sound was out of range or was issued. |
| Audio sound selector check | Three zero fields | The empty program/tone/note selector suppresses playback. |
| Audio `SsUtKeyOn` | 0 | No fine pitch offset is added to the selected note. |
| Audio `VSync` | 0 | The SDK's wait-for-next-vertical-sync argument; retaining the direct API value requires no project mode invention. |
| Audio load results and OPEN failure exit | 0 | CD loader success code and the retail process exit status, respectively. The existing exit status on failure is preserved. |
| Audio ring and fade countdowns | Index 0, `- 1`, -1 | Wrap to the first voice slot and preserve the inclusive countdown endpoints. |
| Audio maximum-volume tests | `+ 1` | Exclusive upper bound above the named maximum. |
| Audio pan formulas | Shift 1 and multipliers 2 | Halve the folded angle and scale the narrowing coefficient by two; these are arithmetic factors, not independent tuning parameters. |
| GAME tone test | `0x80`, comparison 1 | Retail masks the high bit then compares it to 1, so the branch is unreachable. A behavioral flag name would imply an unsupported meaning; both operands remain visible. |
| GAME sequence filename | Extent 20, indices 6/1, `'0'` | Preserve the original stack array extent; indices select the sequence and floor digits in the literal template, and the character is the decimal digit origin. |
| GAME/OPEN `sound_ref_key_off_bank0` | Shift 8 | Pack the MIDI note into the upper byte with zero fine pitch. The program's bank byte is zero. The later selector audit corrects the former voice-mask interpretation and replaces byte indices 0/2 with the shared program/note fields. |

Retail claim addresses/extents remain literal ownership evidence as in the
earlier batches. New enum initializer values define the documented constants
and require no second layer of aliases. Every compiled non-debug section and
the strict objdiff report are unchanged against `893a3fa`; seven objects have
only changed debug-line metadata.

Validation: lint, all 635 repository tests (nine local prerequisites skipped),
and `nix flake check -L` pass. Full `kf build` was run and retains the existing
data ownership/placement failures, including two conflicting GAME target
section bases. No instruction, relocation, initialized byte, or strict match
result changed in this batch.

## Pool capacity and notification plan

Follow the shared pool owners through their allocation, reset, render and save
sweeps. Name each owner's fixed capacity and the sentinel for that particular
record type; do not substitute equal-valued action IDs, sound levels or animation
timings. GAME's map-object definitions and its first transient-object index
both equal 160 but describe separate contracts. Keep all existing countdown
and endpoint forms. The notification ring's eight slots, six presentation
sprites, hold/rotation phases and numeric popup form are a separate small
state-machine audit. The existing function dossiers and unchanged post-merge
objects remain the evidence/comparison baseline.

The shared declarations now name 128 actor slots and twelve definitions,
190 GAME map-object slots and 160 definitions, eight map-event slots,
48 effect slots, 64 floor-item slots, and 32 OPEN scene-entity slots. The
definition-copy word counts derive from the complete typed arrays. The
map-object save payload distinguishes the first transient index (160) from
the full pool capacity; its additional transient banks remain a separate
save-format audit. Floor-item placement termination uses its own 0xffff
sentinel. Actor, effect, map-event and OPEN entity free markers are named
for their respective record types.

The notification ring wraps with `capacity - 1`; its free marker is 0xff.
GAME's gold pickup reconstructs a two-byte amount, queues ID 0x13 and adds
the same amount to `player_state.gold` at 8003554c..80035570, establishing
`KF_NOTIFICATION_GOLD`. That ID displays a label and four decimal sprites
instead of the ordinary message sprite. Their screen X coordinates establish
the ones/tens/hundreds/thousands order. The effect holds fifteen frames,
then rotates by 128 angle units per update until an eighth turn. It dequeues
repeated ordinary messages together, but keeps each gold amount separate.

The [notification identity audit](game-notification-identities.md) names all
32 atlas messages and types their queue, link fields and API. Its current
[per-occurrence ledger](game-notification-literal-ledger.md) explains all 116
retained queue literals; the table below summarizes those categories.

| Sites | Values | Reason |
| --- | --- | --- |
| Existing field-offset assertions | Offsets 0x10/0/1/2/3/4 | Independent retail field positions; size assertions were removed. |
| Static assertion idiom | Pointer 0, array extents 1/-1 | The unevaluated null-base offset expression and C89 compile-success/failure mechanism. |
| Six sprite initializer rows | All 60 scalar values | Preserve the measured UV coordinates, spans and signed screen positions stored in unsigned halfwords. Initial active flags and opaque bytes are zero; no meaning is invented for the opaque byte. Positioning/shape data remains readable as a table instead of aliases for every coordinate. |
| `notify_enqueue` varargs access | `+ 1` | Address the first stack argument after `message_id`, preserving the retail ABI access. |
| Queue head/tail and hold counter | `+ 1`, `- 1`, zero test | One-entry advance, mask endpoint and one-frame decrement/exhaustion. |
| Effect angle initialization | 0 | Arithmetic zero rotation. |
| Sprite active assignments | 0/1 | Boolean visibility. |
| `menu_format_number` | Flag 0 | The `pad_zero` Boolean is false, so the formatter leaves leading positions blank rather than filling them with zero digits. |
| Formatter output | Indices 3/2/1/0 | Select the four positional digits, read in reverse for the right-to-left screen arrangement. Sprite roles themselves have semantic names. |
| `display_flip_buffer_index` | Comparison 0 | Boolean inversion switches the two display buffers. |

The capacity changes retain literal zero-based origins and unit countdown
adjustments in the other units. Equal-valued action IDs, animation phases,
asset IDs, other flags and table values in those units remain assigned to
their own domain audit; this batch does not claim to have reviewed them all.

The final rebuild preserves all non-debug sections of all 112 objects and the
complete strict report, including all 484 scored entries. Fourteen objects
have debug-line changes relative to the post-merge baseline (seven already
changed in the audio/camera batch). All 635 tests pass (nine prerequisites
skipped), lint and `git diff --check` pass, and full `kf build` retains the
existing data-placement/ownership failures. Exact-count movement is zero.

## Allocator, formatter, and VRAM viewer plan

Follow the shared allocator from GAME/OPEN startup through arena rebasing and
the resource loaders' switch to heap allocation. Name the three modes, stack
metadata position, allocation alignment, and observed RAM/heap bounds without
changing unsigned address arithmetic or the image-specific arena budgets.
The shared decimal/hex formatter's loop bounds, highest place values, absent
width sentinel, and OPEN's evidenced padding prefix have separate meanings
from the literal character grammar and arithmetic radix operations. The OPEN
VRAM viewer's pan increment and coordinate wrapping are a small display-only
contract. Preserve every API argument and all image-specific buffer extents.

Use the existing image-specific dossiers for the allocator, its startup and
resource callers, both formatter bands and OPEN 8001a82c. Compare rebuilt
non-debug sections and every strict result with the post-merge baseline.
This is a naming-only batch; no bounds checks, formatter repairs, storage
ownership changes or altered SDK calling conventions are intended.

The allocator modes now describe creating the initial arena (0), rebasing it
at the current cursor (1), and using heap allocations (2). Both startup
callers create then rebase the arena; resource loaders select heap allocation
after assembling their persistent assets. The allocation stack retains its
seventeen words: a named depth position followed by sixteen entries. No
overflow/underflow handling is added. Four-byte alignment keeps the original
signed expression, with the alignment-minus-one subexpression grouped before
addition. The malloc address test still uses unsigned 32-bit wraparound when
adding the cached RAM base. Initial arena end 801effff is inclusive; system
heap end 801f8000 is exclusive. The rebased last-byte offsets remain fefff
for GAME and 112fff for OPEN.

The formatters traverse ten decimal places starting at 1000000000 or eight
hex places starting at 10000000 hex; those bounds are distinct from the
radix used to advance between places. Width ff means no padding width was
specified. OPEN's seven-byte prefix names only the evidenced leading padding,
not a recovered allocation boundary. The VRAM viewer moves by four storage
coordinates and wraps X/Y within the 1024-by-512 VRAM rectangle.

All 156 literals still inline in the complete memory, formatter, debug and
VRAM-viewer sources have these reasons:

| Sites | Values | Reason |
| --- | --- | --- |
| GAME/OPEN number scratch arrays | 24/19 | Retain the existing claimed BSS extents; their complete original allocations remain unresolved, so they are not presented as general formatter capacities. |
| Decimal/hex digit generation | 10, 4, `'0'`, `'A'`, `'-'` | Decimal radix, four bits per hex digit, and visible character encoding. The arithmetic directly states the conversion rule. |
| Format grammar and padding characters | `'1'`..`'8'`, `'%'`, `'0'`, `'D'`/`'d'`, `'X'`/`'x'`, `'S'`/`'s'`, space, LF/CR | Literal characters make the accepted language and newline translation clearer than aliases for each character. Width recognition remains the exact inclusive character range. |
| Formatter string scans and endings | 0, `'\0'` | C string termination, including the numeric zero comparisons. |
| Formatter state and debug flag | 0/1 | Boolean initial values, state tests, setting and inversion. |
| Formatter loop origins, counts, sign and zero-digit tests | 0 | Arithmetic zero, zero-based iteration and the signed value boundary. |
| Formatter index/count adjustments | 1 | Last-index derivation, first-character advance, and inclusion of the final terminator in the returned count. |
| Allocator null values and depth reset | 0 | Null pointer checks/assignments and an empty allocation stack. |
| Allocator bound/alignment/depth adjustments | 1 | Inclusive last-byte calculation, exclusive next-byte calculation, alignment mask derivation and one-entry push/pop. |
| VRAM viewer draw flags and buffer toggle | 0/1 | Boolean background clearing, drawing-to-display enable and double-buffer inversion. |
| VRAM viewer environment subscripts | 0/1 | Explicit updates to the two physical display buffers. |
| VRAM coordinate masks | 1 | Derive the last coordinate from each named extent. |
| `DrawSync`/`VSync` arguments | 0 | Preserve the SDK's synchronization selector; the pinned interfaces expose no corresponding symbolic constant. |
| `PadRead` arguments | 1 | Preserve the retail call-site value. The legacy unprototyped declaration permits it, while the linked routine reads the global pad identifier and does not consume this argument; naming it as a controller-port selector would be misleading. |

Claim addresses/extents and enum/macro definitions retain their literal values
for the same reasons as earlier batches. Startup/resource functions only
change their allocator-mode arguments here; their other constants remain in
the relevant startup/resource audits.

Validation preserves all 112 objects' non-debug sections and all 484 strict
scores. Eighteen objects have only debug-line changes relative to 893a3fa
(four additional objects in this batch). All 635 tests pass with nine local
prerequisites skipped; lint and whitespace checks pass. Full `kf build` ran
and retains the existing data ownership/placement failures. No exact result
or relocation changed.

## CD and resource registration plan

Trace ordinary CD reads and the error-screen read path separately: GAME's
path and indexed reads, OPEN's reads, and the error-screen loader have four
different retry limits. Name those limits and preserve the successful-read
counter assignment, status handling and polling calls. Share the existing
sector-size definition and name the error stages established by their callers.
Trace resource chunk payloads and registration IDs through both model-slot
consumers and GAME's separate animated-asset registry. The loader and renderer
must use the same domain names; equal numbers in the two registries must not
be conflated. Keep data-path strings, stream ordering, pointer arithmetic,
source types and exact retail operations unchanged.

The prior per-function snapshots cover CD loaders, GAME/OPEN resources,
error-screen rendering, model slot selection/release, asset registration and
the affected rendering consumers. Validate all objects and strict scores
against the same baseline. Resource placement and behavior corrections remain
outside this naming batch.

The CD sector definition is shared by all three loader owners. GAME attempts
three path reads or five indexed reads; OPEN attempts one hundred reads; the
system-screen loader attempts fifty. A successful read still writes one
hundred into the attempt counter before its increment. This exit assignment
has its own name even where it equals OPEN's retry limit. The linked GAME
`CdReadSync` wrapper calls `CD_readsync` (8003bc98): at 8003bd5c a nonzero
mode bypasses the wait loop and returns the current remaining-sector count,
establishing the polling argument. The API's null result buffer is retained.
OPEN still returns zero after exhausted read attempts once search succeeded;
no success claim or behavior repair is introduced.

The system-screen IDs are checked against the retail TIMs E0..E3, decoded
with the CLUT selected by GAME 8001b7b0: X=0, Y=501. E0/E1 both report
unreadable data and request a reset; their callers distinguish failed search
from failed read. E2 reports that saving is unavailable because no memory
card is present. E3 contains “PAUSE!!”, consistent with the controller-triggered
call in `player_update`. These are named message choices even though the
existing function identity still says `display_show_error_screen`.

| Retail asset | SHA-256 |
| --- | --- |
| E0. | `86bafc3ca8a85cfb8033fffdb27cde292ea68dbd31303f6e9ccb7200325bb809` |
| E1. | `3f060303a13b763c2cde1620c2e308c3c0108cda62641e0cd97056e9bfb9c102` |
| E2. | `26d1540163a9855b40925e4731e544b06893426c1af238a15a0b98560a94de82` |
| E3. | `37d6a36d8c023bdb18e62dbbaa48e4f59e79c50faa783c188bb16e0fb7288d12` |

Resource chunks have a four-byte length prefix. Their complete map-grid
copy count now derives from the typed 100-by-100 byte grid and u32 word
size, still 2500 words. The sixteen-byte arena-reuse prefix names the
observed cursor advance after release; its original rationale is unresolved,
so it is not called an allocator header or alignment requirement. The
animated-asset archive has a distinct four-byte header containing the u16
count; this is not the same format as a resource chunk.

TMD slots 0, 1 and 4 serve map geometry, scene entities and menu item models.
GAME's separate animated-asset registry uses actor base 0, map-event base 10,
weapon ID 20, shared effect-sprite ID 21, and effect-model base 30. Loader
registration and renderer selection now share those identities, without
asserting archive capacities from the gaps between their bases. OPEN's
model-ID limit 32 is distinct from its 32-entry entity pool. Placement base
Y=0 selects map-floor height; the other scene loaders add -10000 to local Y.
Q12 unity and yaw wrapping in that placement consumer reuse the math names.

The variant model buffer remains 5a000 bytes. GAME's alternate map sequence
is selected at progress 15 on floor 1, progress 25 on floor 2, or variant 3
on floor 5. These names describe the observed selection rules without
inventing a narrative event or treating the progress byte as player level.

Visibility resources have sixteen yaw windows. GAME and OPEN select them in
reverse order using the high four bits of the twelve-bit yaw (`15 - (yaw >>
8)`). The count and shift now name that shared lookup contract, with the
existing signed shift and subtraction preserved.

The six complete loader/registry/OPEN-entity sources audited here retain
102 inline literals for these reasons:

| Sites | Values | Reason |
| --- | --- | --- |
| CD prefix/suffix and resource filename arrays | 5/3/8 | Exact existing initialized string storage, including terminators and retained trailing zeros. |
| CD search test | 0 | Null `CdlFILE` result from the SDK. |
| Sector rounding | 1, zero remainder test | Last-byte mask and addition of one partial sector; zero tests divisibility. |
| CD attempt origins and loaded flags | 0/1 | Zero-based iteration and Boolean success tracking. |
| CD control/read-sync result pointers | 0 | Null optional result buffers. |
| CD remaining-sector tests | 0 | Arithmetic boundary: positive means still reading, zero means no sectors remain. |
| Loader returns | 0/1 | Existing Boolean search-failure convention: OPEN returns 1 for a missing file; other paths return 0 even when read attempts are exhausted. This limitation is preserved. |
| TIM stream and upload checks | 0 | End-of-stream result or absent CLUT/pixel pointer. |
| `DrawSync` | 0 | Existing SDK synchronization selector, as in the earlier display audit. |
| Chunk/registry copy countdowns | 0 | No entries/words remain. |
| Map path edits | Indices 1/3/6, `'0'` | Floor digit, filename start and variant digit in the actual template; character arithmetic makes the decimal encoding visible. |
| Map sequence switch | Floors 1/2/5 | Direct numbered-floor identities; aliases that merely spell the floor number add no meaning. Progress/variant thresholds and sequence choices are named separately. |
| Variant-loading condition | 0 | Boolean `use_variant` argument. |
| OPEN pool countdowns | 1, zero termination | Last-index derivation and the exact post/pre-decrement endpoints. |
| OPEN unresolved reset words | 0 | Evidenced reset value only; no consumer proves the three words' behavioral meanings. |
| OPEN entity lookup failure | 0 | Null pointer return. |
| OPEN placement exhaustion | 0/1 | Boolean state indicating that the placement terminator was reached. |
| OPEN placement X/Z rotation | 0 | No rotation on those axes. |
| OPEN `audio_stop_sequence` arguments | 0/1 | Boolean fade request: `audio_stop_sequence` fades only when the argument equals 1. |

The rendering, startup and menu consumers outside these six complete sources
only change the shared registration/screen/yaw names in this batch. Their
remaining layout, lighting, input and state constants remain in their domain
audits. Retail ownership claims and definition values remain literal.

Validation preserves all 112 objects' non-debug sections, the complete strict
report and all 484 function scores. Twenty-three objects have only changed
debug-line metadata relative to 893a3fa. The existing visibility-owner tests
now accept the named array declaration; their independent compiled allocation,
payload hash and sixteen-record assertions are unchanged. All 635 repository
tests pass (nine local prerequisites skipped), lint and whitespace checks
pass, and `nix flake check -L` passes. Full `kf build` retains the existing
data-placement failures: target relink still verifies GAME 75/77, OPEN 34/38
and PSX 1/1 units, with the same conflicting section bases.

## Controller-mask and opening-input plan

Follow `PadRead` results and saved previous-input masks through GAME menu and
player input consumers, using the existing per-function retail snapshots.
Replace only bit tests on those input values with the exact definitions in
the pinned LIBETC.H. Preserve edge-versus-held tests, branch ordering, and
the player's forced input value. Name OPEN's no-action, advance and skip
values from `opening_poll_input` and the controller's scene-transition
branches; do not conflate them with scene numbers or other status fields.
Rebuild and compare non-debug sections and the full strict report as before.

This batch replaces 160 controller-mask occurrences in ten input-consuming
sources. The mappings are the pinned Release 2.5 LIBETC.H definitions:

| Mask | SDK name |
| --- | --- |
| 1000/4000/8000/2000 hex | `PADLup` / `PADLdown` / `PADLleft` / `PADLright` |
| 10/40/80/20 hex | `PADRup` / `PADRdown` / `PADRleft` / `PADRright` |
| 100/800 hex | `PADk` / `PADh` |
| 8/4/2/1 hex | `PADl` / `PADn` / `PADm` / `PADo` |

The names retain the SDK's spellings, including its letter-named buttons.
GAME's `PADk` path still replaces the entire sampled input with `PADRdown`;
it does not OR that bit into the sample. Menu consumers still test current
and previous samples separately for a rising edge; held movement controls
remain held tests. OPEN sets skip (2) when `PADk` is present and advance (1)
for other nonzero input. Zero input leaves the previously latched action
untouched; only the existing controller sites clear it to none (0).

The remaining input-related literals are deliberate: `PadRead(1)` retains the
ignored retail call-site argument, zero bit-test operands express Boolean
absence, and raw-input zero tests mean no pressed buttons. Scene IDs, menu
selections, layout data and timing constants in the surrounding functions
belong to their separate domain audits. No new raw controller masks are
introduced, and non-input bit fields are not rewritten based on equal values.

All 112 compiled objects retain their non-debug sections and all 484 strict
scores remain unchanged. This batch adds no debug-line differences beyond
the existing twenty-three objects. All 635 tests pass (nine local prerequisites
skipped), lint and whitespace checks pass, and the full build retains the
same data ownership/placement failures. No new exact result is claimed.

## Lighting and death-fade plan

Trace GAME's color-matrix presets, the vitals-restoration color cycle and the
death/recovery fade helpers. Name established palette entries, interpolation
limits/steps, and the death camera's measured timing/position constants.
Preserve the matrix coefficient tables, both stores to the collapsed bob
offset, the asymmetric signed camera bounds, repeated frame calls and all
color/fog arithmetic. The existing dossiers cover GAME 800184b0..80018858
and 80033d80..80033ee4, with their callers and constant/data references.
Require unchanged non-debug sections and strict scores after focused rebuilds.

GAME has seven color presets. Default is entry 0; damage/camera-recoil frames
and periodic HP-loss flashes select entry 1. Entry 2 accompanies the timed
defense effect: its status bit adds ten to defense component 4 during combat
stat recalculation. Entries 3/4 are the equal-channel white and zero black
endpoints; entries 5/6 populate only the green/blue rows. The row meanings
are also explicit in `color_matrix_set_rgb`. These GAME names are not applied
to OPEN's different five-entry table.

The lighting transition advances its Q12 blend by 1024 up to and including
4096. The vitals-restoration cycle uses channel coefficient 4095 for green,
cyan and white, then restores the saved matrix. Its named status-preservation
mask keeps the upper twelve bits while clearing the low four; it does not
invent individual ailment identities. Weapon 9 blends 2500/4096 toward black
and halves the near-fog distance; the green/blue effects blend 3072/4096.
The fog projection argument uses the existing shared projection-distance name.

The update-state values distinguish normal (0), damage animation indices
1 through 7, recovery fade (fe), and dying (ff). The damage end is the
exclusive bound 8, with the existing per-frame increment preserved. Death
camera movement keeps the measured bob threshold 1000, resting bob 1060,
initial pitch step 10, separate resting acceleration 10 and falling
acceleration 15, pitch minimum -800, and floor-relative camera offset 1500.
Both consecutive bob stores remain present in source. Fade progress advances
by 100, and brightness scales between zero and the normal HUD value 86 with
twelve fractional bits. The normal-view initializer uses the same brightness
name. No brightness variable or data identity is renamed in this batch.

The three complete lighting/fade sources retain 117 literals:

| Sites | Values | Reason |
| --- | --- | --- |
| Seven color-matrix initializer rows | 84 scalar coefficients/translation values | These are the measured preset data itself. Keep the matrices readable as numeric tables; the presets have semantic selector names, and no coefficient is rounded or normalized. |
| `color_matrix_set_rgb` subscripts | Eighteen row/column indices 0/1/2 | Explicit coordinates in the small RGB matrix assignments. |
| Blend initialization and absent color channels | 0 | Arithmetic interpolation origin and zero channel contribution. |
| `render_frame` arguments | 0 | Null transform arguments reuse the current view, including both repeated calls at the death endpoint. |
| Weapon-9 fog shift | 1 | Arithmetic division by two, retaining the original subtract-half expression. |
| Fade near-fog endpoint | 0 | Zero distance endpoint in the interpolation. |
| Death pitch-step reset | 0 | No further pitch movement after clamping. |
| Final normal-color fade application | 0 | The blend origin restores the unmodified normal palette. |

Palette and update-state consumers outside these three complete sources only
receive the shared names here. Their remaining literals stay with the broader
player, map, warp and rendering audits. Claim and definition values retain
their usual literal evidence role.

Validation: all 635 tests pass with nine local prerequisites skipped; lint
and whitespace checks pass. The full build rebuilt the final affected
sources and retains the existing data-placement failures. A post-build
comparison confirms unchanged non-debug sections in all 112 objects and
unchanged strict results for all 484 scored entries. Twenty-six objects
have only debug-line changes relative to 893a3fa; exact-count movement is zero.

## Display dimensions and ordering-table bounds

Function Match Plan: name the shared 320-by-240 framebuffer geometry,
double-buffer count and initial selector, 16,384-entry ordering-table size
and wrapping mask, initial fog distance, and each image's primitive-buffer
budget. Apply the ordering-table names to all reconstructed enqueue, clear
and submit consumers. Reuse the existing angle and notification constants
in renderer initialization. Name OPEN's evidenced projected-vertex capacity
without adding a GAME bound or changing OPEN's exclusive accepted limit.
Keep the frame-toggle expression, signed depth comparisons, pointer
arithmetic, allocation order, matrix coefficient assignments, and every SDK
call intact. This is a constants-only extension of the display/TMD family,
supported by `docs/display-and-tmd.md`, the existing per-function dossiers
under `build/constant-names/`, and the raw initializer/submit/enqueue bodies.
Every changed function must retain its non-debug bytes, ordered relocation
targets, and strict score against the post-merge baseline.

The two initializers allocate two adjacent primitive intervals: GAME uses
0x19640 bytes each (0x32c80 total), OPEN 0x26160 (0x4c2c0 total). Their byte
selector starts at ff, then `index == 0` selects buffer zero on first frame
and alternates between zero and one. The draw and display rectangles occupy
opposite 240-line halves of VRAM. The geometry origin is half the framebuffer
width/height. Initial fog begins at 11000 with projection distance 200.

Both cleared ordering tables contain 0x4000 words. Submission starts at
element 0x3fff; enqueue paths mask their depth index with the same value.
The map renderers' signed `< 16384` checks stay signed and do not acquire a
new lower bound. OPEN's projected-vertex array has 1000 entries, but its
normal projection routine rejects `count >= 1000`; this distinction is
preserved. GAME's corresponding array extent remains unresolved.

Remaining literals in these initialization paths include explicit first/
second-buffer and matrix-coordinate indices, zero counters/null pointers,
Boolean SDK fields, black channels, and measured light-matrix coefficients.
They retain their positional, arithmetic, Boolean or numeric-data roles.
Texture layout, system-message geometry, OPEN color-preset/mode selectors,
and TMD packet-format constants remain separate pending audits; this batch
does not claim that the complete rendering sources have been covered.

Validation: all 635 repository tests pass (nine skipped), including the
unchanged compiled SDK/layout/retail controls for both display arrays. The
source-declaration assertion now accepts the named buffer count; the
independent literal-size controls remain intact. Lint, whitespace checks,
and `nix flake check -L` pass (635 tests, 123 skips in the flake sandbox).
The full build runs and retains the existing data-placement/ownership
failures. All 112 non-debug object contents and 484 strict scores remain
unchanged; 30 objects differ only in debug-line records from 893a3fa.

## TMD packet and scene-depth constants

Function Match Plan: share the TMD header/packet sizes, packet-field shifts
and masks, eight polygon modes and semi-transparency bit, and vector-index
stride across the GAME/OPEN preparation and enqueue paths. Name the scene
depth rejection bound, map depth bias, depth-to-OT shift, default perspective
shift and measured base texture brightness. Reuse Q12 unity for the sprite
light normal. The existing display/TMD, packet-header and projection dossiers,
the complete shipped TMD parser corpus, and each renderer's typed polygon
fields provide the evidence. This batch covers GAME render/enqueuers and
OPEN render/TMD/map/unlit/sprite functions, plus the OPEN shift initializer.

Preserve the separate packet-length byte read and header-word load; do not
replace them with a byte-only mode lookup or helper. Preserve the halfword
countdowns, in-place halfword shifts, raw mode switches versus masked
preparation switch, each mode's call set and argument order, signed depth
comparisons, and the GAME model renderer's explicit comparison tree. Keep
the original depth averaging and shifting grouped in the same order.
Require identical non-debug sections, ordered referents and strict scores
for every affected function; this does not attempt to close the existing
packet-header or graphics-owner residues.

TMD object offsets are relative to the end of the 12-byte file header. The
packet's four bytes are `olen`, `ilen`, flags and mode. The independent
length-byte path multiplies byte 1 by four; the word path uses
`(header >> 6) & 0x3fc`, equivalent to extracting that length in bytes without
changing its actual source shape. Mode is the top byte. Preparation clears
only the semi-transparency bit with fd, while enqueue consumers distinguish
their actual supported mode bytes. Vertex/normal halfwords become byte
offsets through a three-bit shift for the eight-byte records.

The ordinary scene enqueue paths accept depth from five onward. Map paths
instead add bias 200 and enforce their existing upper bound only. Projected
`sz` stores full depth; the ordering index divides it by four after vertex
averaging. A quad's shift by four combines its four-way average with that
conversion; the source still performs one shift. The projection `p2` shift
is a separate factor-of-two convention, with OPEN keeping its mutable shift
and separate right-shift path. Repeated texture base RGB values are 128,
and the sprite normal is the Q12 positive-Z unit vector.

The complete audits of GAME `render_enqueuers.c` and `sprite_add_ft4.c`, plus
OPEN `render.c`, `render_tmd.c`, `render_map.c`, `render_unlit.c`,
`render_sprite.c` and `sprite_add_ft4.c`, leave 252 inline literals:

| Sites | Values | Reason |
| --- | --- | --- |
| Triangle depth/color averages | 3 | Three explicit vertex terms are being averaged; this is the formula's divisor, not a variable mesh capacity. |
| Quad depth/color averages | 2 in a shift or combined shift | Divides four explicit vertex terms by four, retaining the original single-shift grouping for depth. |
| Sprite corner array and accesses | Extent 4; indices 0/1/2/3 | The four named GPU corner positions, assigned and passed explicitly. |
| 2D sprite rectangle accesses | 0/1/2/3 | Existing positional X/Y/width/height API view; preserve its exact halfword reads. |
| 2D sprite texture accesses | 0/2/4/6 | Existing byte positions for U/V origin and U/V span; preserve the selected byte reads rather than introduce a wider load. |
| Sprite RGB accesses | 0/1/2 | Explicit R/G/B channel coordinates. |
| Clip tests | 0 | The signed triangle-area origin separates accepted winding from rejected or degenerate triangles. |
| Sprite depth-cue option | `flag == 1`, `p >> 1` | Boolean selection and arithmetic half-term in the observed 1.5-times cue factor. |
| SDK semi-transparency calls | 1 | Boolean enable argument, distinct from the packet's named mode bit. |
| Frame synchronization calls | 0 | SDK blocking `DrawSync` and next-frame `VSync` selectors; no project-specific state is implied. |
| Frame selector toggle | `index == 0` | Boolean inversion between the two actual buffer indices. |
| Countdown/empty tests and resets | 0 and -1; `count - 1` | Arithmetic empty/terminal conditions, zero record counts and per-frame counters. The narrowing and update order are unchanged. |
| Bound adjustments | `length - 1`, `minimum - 1`, `mode + 1` | Last element and equivalent exclusive endpoints, preserving each original comparison operator. |
| Optional transform pointers | 0 | Null means retain the existing position or rotation. |
| Rotation/sprite anchor components | 0 | Absent axis rotation or model-plane coordinate and origin. |
| Initial CVECTOR command and SVECTOR pad | 0 | The command is filled from the allocated packet before lighting; the normal's pad remains the literal initialized storage byte/halfword. |
| GAME sprite CVECTOR derivation | `&active_render_clut + 2` | Two halfwords reach the measured color address four bytes past CLUT. The complete graphics owner is unresolved; preserve this evidenced pointer derivation, as documented in `render-material.md`. |

GAME `render.c` and OPEN `render_init.c` also receive shared format names,
but their remaining system-screen, texture and scene-mode literals still
belong to the pending setup audits. Retail claim values remain literal.

Validation: all 635 tests pass with nine skips; lint and whitespace checks
pass. The full build retains the existing data-placement/ownership failures.
All 112 object files retain identical non-debug sections and all 484 strict
scores are unchanged. Thirty-three objects have debug-line-only differences
relative to 893a3fa. No function is newly banked.

## Incoming renderer reconstruction reconciliation

Function Match Plan: integrate the four committed renderer improvements
through master `c518cc5` into the naming worktree. Resolve the one overlapping
source from that committed renderer, preserving its repaired allocation
exits, twelve-mode TMD dispatch, signed model-mode switch, guarded projection
base lifetimes, shared lighting/depth tails, SDK sprite output type and
transient sprite depth expression. Reapply only the already evidenced TMD,
ordering-table, texture-brightness and fixed-point constant names.

The incoming map/model/TMD/sprite dossiers were read with the source diff
and existing image-qualified retail evidence. Their four supported partial
results are respectively 98.770996%, 93.126690%, 98.932236% and 93.972980%.
Compile the exact incoming C as an independent control under the merged
headers/configuration, then require the named version to differ only in
debug-line information. Require all other object contents and strict scores
to remain unchanged from the preceding naming baseline. Keep the incoming
relocation/table-ownership corrections and their regression controls.

After that comparison, establish a separate baseline for later naming
batches without overwriting the earlier 893a3fa snapshot. This imports
committed user work into the isolated branch; final integration into master
still waits for completion of the full naming campaign.

The reconciled renderer differs from the independently compiled literal
`c518cc5` source only in `.debug_line`. All other 111 objects retain their
non-debug contents from 893a3fa. Exactly four of 484 score rows change:

| GAME function | Earlier baseline | Imported result |
| --- | ---: | ---: |
| `render_enqueue_tmd` | 52.039013% | 98.932236% |
| `render_enqueue_model` | 60.490950% | 93.126690% |
| `render_enqueue_map` | 17.954199% | 98.770996% |
| `render_enqueue_sprite` | 88.263510% | 93.972980% |

All other 480 rows and every banked exact are unchanged. The earlier model
comparison-tree audit is superseded by the incoming signed four-case switch;
its named modes preserve that switch. The eight-file literal audit now has
254 entries, all covered by the same positional, arithmetic, Boolean and
storage reasons above. No new unexplained literal category is introduced.

All 642 repository tests pass with nine skips. Lint, whitespace checks and
flake checks pass; the flake sandbox runs the same 642 tests with 129 skips.
The full build still reports the pre-existing data-placement/ownership
failures. A separate `build/constant-names/post-renderer-merge/` baseline
records all 112 objects, the complete strict report and 484 score rows for
subsequent naming work. The original post-merge baseline is retained.

## GPU setup, system screens and transition fades

Function Match Plan: name the SDK texture-format/blend/reset selectors,
the measured system-message rectangle and texture location, common
transition-image/fade contract, and the renderer's floor-item, HUD and
notification texture handles. Apply the SDK selectors at their existing
GAME/OPEN calls and share system-screen names with the image-until-input
routine. Keep both fade loops, byte stores, repeated calls, packet layout,
palette coordinates and control conditions unchanged. Reuse the existing
per-function evidence snapshots and current 1839ef2 object/strict baseline;
require every non-debug section and score to remain unchanged.

Sony's [Run-Time Library Reference, GetTPage](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libref.pdf#page=301)
specifies texture selectors 0/1/2 for 4-bit indexed, 8-bit indexed and
16-bit direct color, and blend selector 0 for equal background/foreground
weights. Its [ResetGraph entry](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libref.pdf#page=324)
distinguishes complete reset 0 from display-preserving reset 3. These
documented 2.x APIs agree with the pinned LIBGPU declarations and encoding
macro; the later manual does not attribute the retail SDK version.

The system-message routines use the same rectangle: (32,112) through
(288,240), with U span 255 and V span 128. The already decoded E0..E3 TIMs
prove the texture page at (960,256) and selected CLUT row 501. The fixed
error/pause presentation uses brightness 96; the image-until-input routine
instead starts at 32 and increments to 127. These are distinct policies.

GAME and OPEN transition fades use the same (32,0,255,240) destination,
the existing even-byte UV descriptor, page (576,256), CLUT row 495, OT
index four, eighteen frames and brightness increment six. Preserve the
byte addition and its existing ff comparison; do not turn this into a
saturating integer ramp or substitute the system-message rectangle.

Renderer consumers identify the initialized handles: floor items use the
GAME page at (896,0) and encoded CLUT 7a40; OPEN uses (832,0) and 7a00.
The HUD uses (896,256), notification text/gold (832,256), and notification
digits the directly stored page code 1c. The three effect texture-page
entries are initialized only when the caller's current-floor value is five;
their X coordinates remain distinct 320, 384 and 832 with CLUT row 491.
Keep their existing curated function/data identity spellings in this batch.

The later [render material identity campaign](game-render-material-identities.md)
names these cached globals and recovers two SDK palette records. It decodes
`7a40` as CLUT `(0,489)` and `1c` as the 4-bit page `(768,256)`, and documents
all eight retained coordinate/dimension literals in the palette initializers.

The [entity sprite-table campaign](game-entity-sprite-tables.md) replaces the
raw descriptor offsets/stride with two typed owners, names the shared packed
facing/count constants and three depth biases, and corrects `screen_scale` to
`depth_bias`. Its ledger explains the authored descriptor components, remaining
entity-renderer literals and the loader's unmasked initial-frame calculation.

This completes the literal audit of GAME `render.c` and
`display_play_transition.c` and OPEN `opening_fade.c`. Their remaining
inline values have these specific roles:

| Sites | Values | Reason |
| --- | --- | --- |
| Light-matrix assignments | 3800, -2800, 0, -3000, -3600, -3400, -1300, 2700, 800 | The nine measured matrix coefficients are numeric data, retained without rounding or per-coefficient aliases. |
| Matrix, buffer, texture-table and channel subscripts | 0/1/2/3 | Explicit coordinates in the small fixed layouts and paired buffers. |
| Geometry origin | Division by 2 | Centers the origin within each named framebuffer dimension. |
| Fade descriptor extents | 4/8 | Four rectangle halfwords, eight UV bytes, four RGB/command bytes; these express the actual serialized/view shapes. |
| Fade descriptor zeros | 0 | Top-edge/UV origins, unused high UV bytes, black initial RGB and the unused command byte. |
| Path-array extents | 7 | Exact claimed storage, including the terminating byte of each six-character literal. |
| Path digit conversion | `'0'` | Character-code origin for the selected decimal filename digit. |
| CLUT X and upper texture-page Y | 0 | Literal VRAM coordinate origin in the relevant SDK call. |
| Clear/background/dither/display fields and semitransparency | 0/1 | Boolean SDK fields and arguments; separate from the named GPU mode selectors. |
| CD return/remaining-sector tests | 0 | Failure/success and completion boundaries in the existing SDK contracts. |
| `exit` | 1 | Conventional nonzero unsuccessful termination status. |
| CD callback/result pointers and optional view pointers | 0 | Null pointers in the existing API contracts. |
| Controller read argument and empty-input tests | 1 and 0 | The pinned legacy PadRead argument is ignored; zero is the no-buttons mask. The press/release waits are unchanged. |
| Synchronization arguments | 0 | Blocking DrawSync and next-frame VSync selectors, as in the earlier renderer audit. |
| Sector rounding and last-element/loop endpoints | 1 and -1 | Arithmetic inclusive/exclusive adjustments; the original operator and narrowing are retained. |
| Counters, queue indices, absent rotation components and black/far channels | 0 | Empty/reset state or the mathematical/color origin, already distinguished from named mode/state selectors. |

OPEN `render_init.c`, the opening scene body, controller, game loop and save
system receive only the relevant shared GPU/screen names here. Their broader
mode, scene, palette and persistence audits remain in progress.

Validation: all 642 tests pass (nine skipped), lint and whitespace checks
pass, and flake checks pass (642 tests, 129 sandbox skips). The screen
regression fixture now selects the named brightness bound in both its
positive and deliberately incorrect variants; its literal retail words,
call targets and referent controls are unchanged. All 112 non-debug object
contents and 484 strict scores match the 1839ef2 baseline; six objects
differ only in debug-line records. The full build retains the existing
data-placement/ownership failures.

## Program-entry modes and OPEN color presets

Function Match Plan: name the two-word loader argument block, its request
and result positions, the fixed OPEN/GAME path indices and the opening/
ending mode values across PSX, GAME and OPEN. Retain the loader's actual
`Exec` argument count of one even though the writable block has two words.
Keep the existing signatures, casts, launch order and every startup call.
Name OPEN's five color-preset selectors by their measured contents and
ending-transition use, keeping all sixty initializer scalars intact.
The existing three entry-point dossiers, OPEN controller/reset/palette
consumers and GAME exit-code producer bodies were reviewed before edits.
Require unchanged non-debug sections and all 484 strict scores, including
the exact PSX and OPEN entry points.

PSX primes result word 1 to opening mode 1, then copies it to request word
0 before each OPEN-then-GAME iteration. OPEN reads word 0; GAME writes its
exit code to word 1. A menu return requests opening mode 1; the ending
trigger requests fe. These values select OPEN content; they do not choose
which EXE the loader runs next. Correct the earlier successor-selector
wording without changing the curated confidence or unresolved loader ABI.
GAME's internal zero exit code still means keep running.

OPEN color entry 0 is its default preset, 1 is all-zero black, and 2 has
equal 4095 coefficients in all color rows. The ending scroll blends from
black to entry 3, then from entry 3 to the green-only entry 4. Name entry
3 by its observed intermediate endpoint role without inventing a light
source or story identity. These names remain distinct from GAME's seven
different presets.

This completes the constant audit of PSX `main.c` and OPEN `render_init.c`.
PSX retains the two literal 20-byte padded filename extents, SDK Load's
success result 1, and the zero stack-address/size fields passed to Exec.
They express measured storage and ordinary SDK return/zero-parameter
contracts, rather than another overlay mode. OPEN initialization retains
the sixty color-table scalars and nine light-matrix coefficients as numeric
data. Its explicit matrix/buffer indices are positional; zero origins,
empty item count and black channels are mathematical/reset values; SDK
dither, background, drawing/display flags remain Boolean 0/1. Halving the
named framebuffer dimensions is ordinary centering arithmetic. The
primitive allocator has no other numeric literal outside its address claim.

The other entry points and controller/player/scene consumers receive the
shared mode/argument names here. Their startup RAM constants, menu decisions
and scene behavior still belong to subsequent audits.

Validation: all 642 tests pass (nine skipped), lint passes, and flake checks
pass (642 tests, 129 sandbox skips). All 112 non-debug object contents and
484 strict scores match the 1839ef2 baseline; twelve objects differ only
in debug-line records. The full build retains the existing data-placement
and ownership failures, with no new artifact failure or naming regression.

## Startup store destinations and initial heaps

Function Match Plan: name the immediate destination, repeat count, initial
heap base and heap byte budget in GAME and OPEN `main.c`. Review both entry
points and their adjacent store helpers together; preserve the non-advancing
store loop, every raw immediate, cast, call and delay-slot schedule. These
four exact functions have complete disassembly/CFG, caller/callee, reference,
string, history and match snapshots. The addresses remain integer macros,
so naming cannot introduce symbol relocations. Require all non-debug object
contents and all 484 strict scores to remain unchanged.

GAME repeats its zero store at 80058060 (the first word of
`player_death_saved_color_matrix`) 67fe8 times. OPEN repeats its zero store
at 800377a0 (the first word of `cd_search_file`) 70218 times. Neither loop
advances its destination; neither count is named a BSS byte size. Correct
the contradictory OPEN dossier sentence which previously described clearing
the BSS. The broader purpose of repeating these stores remains unresolved.

The initial GAME heap is 157680 bytes at 800a0980; OPEN's is 177f00 bytes
at 80080100. Both end at 801f8000, agreeing with the later system-heap reset.
Keep each image's directly supplied size rather than replacing its immediate
with an expression involving another module's heap-end constant.

This completes the literal audit of both `main.c` files and OPEN
`opening_controller.c`. The remaining startup zeros are the stored value,
loop exhaustion boundary and required legacy PadInit selector. InitCARD2's
one is the Boolean pad-sharing argument of the existing BIOS declaration.
Sony's later reference documents [PadInit's zero argument](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libref.pdf#page=790)
and [InitCARD's controller-sharing flag](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libref.pdf#page=175);
these describe API contracts, not a claim of exact SDK-version attribution.
The linked PadInit body independently routes zero to PAD_init2 and other
identifiers to its diagnostic path.

The OPEN controller's remaining literals need no additional selector names:

| Site | Value | Reason |
| --- | --- | --- |
| `opening_initial_tim_path` extent | 6 | Exact claimed six-character storage; do not enlarge the array to include a terminator. |
| Allocator container offset | 0 | Null-base field-offset expression, retaining the existing typed layout calculation. |
| Runtime clear and allocation-stack depth | 0 | Zero-fill byte and empty stack count. |
| PadInit | 0 | Required legacy API argument, as above. |
| SetDispMask | 1 | Boolean display enable. |
| CD load result | 0 | Success boundary of the existing loader contract. |
| Both audio_stop_sequence calls | 1 | Boolean fade enable: the callee ramps volume down before stopping the sequence. It is not a sequence ID. |

Validation: all 642 tests pass (nine skipped), lint and whitespace checks
pass, and flake checks pass (642 tests, 129 sandbox skips). All 112 objects
retain their non-debug contents and all 484 strict scores are unchanged
from 1839ef2; twelve objects differ only in debug-line records. The full
build still reports the pre-existing data-placement and ownership failures.

## Incoming animation binder closure

Integrate the user's committed frame-owner controls (`40b5700`) and exact
animation binder (`4199d60`) into the naming worktree. The binder's new retry
label and clip-before-keyframe publication are supported by its committed
[scratch/retry campaign](game-graphics-scratch.md). Retain those source
changes together with the earlier named cache states, pool capacity, Q12
constants and morph object selector. The merge needs no source conflict
resolution and does not touch the user's uncommitted OPEN renderer work.

Verification plan: compile the literal incoming `pool.c` independently with
the same headers/profile, compare its non-debug contents with the named
merged object, and check all other objects against 1839ef2. Only the incoming
binder improvement to strict 100% is expected among the 484 function scores.
Run the imported raw-word controls, repository tests, flake checks and full
build before committing the merge.

The independent literal and named pool objects differ only in debug-line
records. The other 111 objects retain all non-debug contents from 1839ef2.
Exactly one score changes: GAME 800205d4 rises from 98.798290% to 100%,
preserving the user's closure; all other 483 scores and every banked exact
remain unchanged. All 644 tests pass (nine skipped), including the imported
canonical/owner raw-word controls; lint and whitespace checks pass. Flake
checks pass (644 tests, 131 sandbox skips). The full build retains the same
ownership/placement failures, six section-base conflicts and zero artifact
failures. No new banking operation belongs to this merge.

## Opening scene timing and entity transitions

Function Match Plan: audit OPEN `opening_scene0_run`,
`opening_scene1_draw_fade`, `opening_scene1_run` and
`opening_entity_transition`, using their existing complete image-qualified
dossiers and refreshed raw timing/scale comparisons. Name camera milestones,
rotation/audio choices, image-panel geometry, fade/hold durations and the
four transition modes. Propagate mode names to the reviewed scene-3 and
ending callers. Keep every comparison, signedness, counter adjustment,
call, load/store order and scratch position copy. Compare all non-debug
objects and all 484 strict scores with the new 7c1968e baseline.

Scene 0 rotates object 11 toward decreasing yaw and object 12 toward
increasing yaw after camera point 8; point 15 starts its fade-out. Those
operational roles justify selector names without assigning story identities
to the models. Scene 1 draws two overlapping 192-pixel panels, with the
right panel starting at X=128; these are not equal halves of the screen.
The shade reaches 128 in steps of four, then holds for at most 1000 frames
with an immediate sequence stop at frame 600 before the final fade-out.

Transition mode 0 creates four model-19 entities in slots 24..27 and grows
their Y scale from zero. Mode 1 removes them immediately. Mode 2 creates
them at Y scale 8192, shrinks them and removes them. Mode 3 only creates
them at that tall scale. All variants retain unit X/Z scale; the animated
variants rotate yaw by 512 and stagger Y-scale updates by eight frames over
48 frames. Keep the unsigned Y-scale guard and its one-past limit intact.
That guard permits one step above 8192 during growth and a wrapped negative
halfword during shrinkage; neither path is rewritten as a saturating ramp.

The four audited bodies now retain only these inline numbers:

| Sites | Values | Reason |
| --- | --- | --- |
| Blend, shade, frame and entity-index origins and exhaustion tests | 0 | Mathematical zero, zero-based iteration or black/no-scale endpoint. |
| Camera-path step argument | 0 | No additional vertical displacement. |
| XY and UV origins in the two image panels | 0 | Actual top/left texture or screen coordinate origin. |
| Sequence stop arguments | 0/1 | Boolean immediate-stop versus fade-before-stop choice. |
| VSync | 0 | Next-frame synchronization argument of the SDK. |
| Transition rotation reset | 0 | Zero angle on each axis. |
| Transition frame-render arguments | 0/0 | Null view pointers selecting the current view. |
| Shade and scale comparison endpoints | +1 | One-past bounds derived from the named inclusive values; original comparisons retained. |
| Reverse-count initial/end adjustment | -1 | Count-minus-one and countdown exhaustion, not another transition mode. |

The three later scene bodies and the camera, panel, color and sound data
initializers still require their remaining audits. Their transition calls
receive only the shared mode names in this batch.

Validation: the literal census confirms 39 remaining inline numbers in
the four audited bodies, all covered above. All 112 non-debug object
contents and all 484 strict scores match 7c1968e; only OPEN's scene object
has debug-line changes. All 644 tests pass (nine skipped), and lint and
whitespace checks pass. The full build retains its existing ownership and
data-placement failures. No function's match verdict changes.

## Later opening scenes and ending sequence

Function Match Plan: finish the literal audit of OPEN `opening_scenes.c`
through `opening_scene3_run`, `opening_ending_scene_run`,
`opening_ending_scroll_run` and their initialized data. Their complete
disassembly/CFG, call/reference/string and match dossiers remain available;
the phase dispatch, countdown, volume division, panel clipping, entity
movement and brightness schedules were reviewed again before editing.
Name the lighting/audio phases, panel counts and placement contract,
camera/rotation rates, transition height and ending limits. Reuse established
Q12, angle, camera-finished, entity-free and projection/fog constants.
Keep the unrolled texture calls, serialized data, signed/unsigned accesses,
load/store order, and every original comparison and increment. Require all
112 non-debug objects and all 484 strict scores to agree with 7c1968e.

Lighting runs black-to-midpoint, midpoint-to-green, then stops updating.
The independent sequence phases wait for the scroll, count down, fade the
current sequence, then load the ending sequence. The delay starts at 20
and expires on -1 after 21 decrements. The volume accumulator starts at
381 (three times the SDK maximum), is decremented before use, then divided
by three for both master-volume channels; do not replace it with a
one-step-per-frame byte ramp.

The nine ending panels use consecutive 64-pixel texture-page X origins
starting at 448 and consecutive CLUT rows starting at 493. Scene 3 uses
the first two placements. Keep all original calls and their order while
naming that common placement rule. Both cullers add 255 to the signed Y
coordinate before narrowing to u16 and comparing with 495, corresponding
to Y values -255 through 239. The panel data's height is still 254; this
guard is not rewritten as a rectangle intersection.

The ending scroll's phase counter cycles 0,3,2,1. Panels move at phases
0 and 2; the final model's roll/color updates occur at phase 0. These are
positions in a four-frame cycle, distinct from the named lighting and
sequence states. Model 26 advances in Y until reaching -8000, then is
hidden and model 27 becomes visible. Names describe their translation and
rotation roles without assigning unsupported story identities.

This completes the literal audit of all seven functions and initialized data
in OPEN `opening_scenes.c`. The census leaves 598 inline literals, including
the 39 already explained in the early-scene audit. The rest remain for these
specific reasons:

| Sites | Values | Reason |
| --- | --- | --- |
| Entity layout assertions | 28/08 hex, 0, 1/-1 | Independent measured structure size/position offset, null-base offset expression and compile-time valid/invalid array extents. |
| Camera-path array extents | 17, 3, 9, 3 | Exact initialized record counts including each terminating row; not runtime capacities. |
| Camera-path positions, rotations and segment speeds | All remaining row scalars | Authored trajectory data. Keep the individual numeric samples and zero fourth lanes/trailing fields instead of inventing per-point constant identities. |
| Terminating camera rows after the named X marker | -1 and 0 | Serialized fill in fields skipped once the X sentinel is found; no distinct consumer-supported meaning. |
| Scene-0 SoundRef | 9, 0, 43 hex | Numeric program/tone/note recipe in the typed sound record. No supported sound-asset identity justifies another alias. |
| Overlay/panel rectangle and UV data | All remaining row scalars | Measured screen positions, nonuniform vertical spacing, cropped dimensions and UV origins/spans; preserve those explicit authored samples. |
| Color data and ending light matrix | All remaining coefficients | Numeric RGB/command bytes and matrix coefficients, including zero translation; do not round, normalize or name each coefficient. |
| Descriptor extents and cursor strides | 4/8; background count 2 | Four halfwords per rectangle, eight UV bytes, four color/command bytes and the two explicit background rectangles. |
| Texture/CLUT/framebuffer/background subscripts and placement multiples | 0..8 | Positions in the fixed arrays and the corresponding index times the named page stride or CLUT-row increment. |
| Rectangle Y subscripts | 1 | The Y coordinate in the four-halfword rectangle layout. |
| SDK CLUT X and camera-step argument | 0 | VRAM origin or no additional vertical displacement. |
| SetDispMask | 1 | Boolean display enable. |
| Scroll enable and tests | 0/1 | Boolean motion enable. |
| Angle/blend/brightness/counter origins, resets and zero tests | 0 | Mathematical or empty-state origins and black/mute endpoints. |
| Scroll phase comparisons | 0/2 | Explicit positions in the named four-frame cycle, as described above. |
| Roll step | 1 | One angular unit per selected update, analogous to the unit panel/color counter increments. |
| Inclusive/exclusive and countdown adjustments | +1/-1 | One-past blend limits, last-element indexing, countdown exhaustion and cycle wrap; preserve the original pre-decrement schedule. |
| Ending scene's intermediate `blend` counter | 40/fff hex | Its increment/guard/clamp have no value consumer before the counter is reset for the final fade. Preserve the measured operations while their original purpose is unresolved; do not label them an angle mask or claim they drive the visible brightness ramp. |

The ending brightness calls still occur before the existing upper clamp,
and the final darkening loop retains its original lower clamp. Naming the
bounds does not remove their intermediate overshoot or change call order.

Validation: all 644 tests pass (nine skipped), and lint and whitespace checks
pass. All 112 non-debug object contents and all 484 strict scores match
7c1968e; the scene object differs only in debug-line records. The initialized
data and relocation sections are unchanged. The full build retains its
existing data-placement and ownership failures, with no new artifact failure.

## Incoming error-screen and TMD setup improvements

Integrate the user's `806f1ed` renderer campaign into the naming worktree.
Its [committed evidence](game-render-setup.md) proves the error screen's
early brightness lifetime and RGB store order, and the TMD primitive
countdown's unconditional setup. Resolve the three source conflicts by
retaining those operations together with the named path digit, brightness,
ordering-table length and TMD header size. Do not restore the earlier
chained RGB assignment or move the countdown back inside its guard.

Verification plan: independently compile literal `806f1ed:src/game/render.c`
under the current headers/profile and compare every non-debug section with
the merged named object. Check the other 111 objects against 7c1968e and
allow only the two incoming strict-score improvements: error screen to 100%
and TMD preparation to 99.375%. Run the imported raw controls, repository
tests, flake checks and full build before the merge commit.

The independent literal and named render objects differ only in debug-line
records. The other 111 objects retain every non-debug section from 7c1968e.
Exactly the two expected scores change: GAME 8001b7b0 moves from 97.922680%
to 100%, and GAME 8001c2b0 from 98.333336% to 99.375%. All other 482 scores
and all banked exact functions are unchanged. All 644 tests pass (nine
skipped), lint/whitespace checks pass, and flake checks pass (644 tests,
131 sandbox skips). The full build retains the existing ownership and
placement failures. The incoming exact banking record is preserved; the
naming merge performs no additional banking.

## GAME lifecycle, warp modes and card events

Function Match Plan: audit GAME `game.c` and `player_warp.c` together with
the shimmer-mode caller in `player_warp_to_floor_entry` and the card-status
producer/cleanup path in `save_system.c`. Existing full image-qualified
dossiers cover every selected body; the main-loop clear spans, event setup,
frame-pacer arithmetic, shimmer traversal, packed-cell dispatch and card
event returns were reviewed again. Name their control values and retain
all widths, raw word loads, scratch copies, call order, countdowns, casts
and relocation referents. Require all 112 non-debug objects and all 484
strict scores to agree with the 3a2fd7d baseline.

GAME shimmer modes are grow-and-remove (0), shrink-and-remove (1), and
grow-and-keep (2). They are distinct from OPEN's transition protocol. The
constructor arguments are owning ID 10, type 17 and kind 21; correct the
old comment which confused type with kind. Four records animate for 48
paced frames with eight-frame staggering, sound at frame 8, scale steps
of 256 and yaw steps of 512. Preserve the original unsigned 8193 guard
and its overshoot/wrap behavior. Both existing frame draws before the loop
and the inherited scratch direction remain unchanged.

`KfMapCell` stores Z then X. The dispatcher still reads the aligned word
starting at `pitch_step`, masks off its low halfword, and compares keys
with X in bits 24..31 and Z in bits 16..23. Spell those keys through one
packing macro with explicit authored cell coordinates; do not substitute
separate byte reads. Ordinal floor IDs and numeric variant/coordinate data
remain visible instead of receiving unsupported place names.

The startup event arguments match the pinned KERNEL.H macros `RCntCNT3`,
`EvSpINT` and `EvMdINTR`. The frame-pacer comparison normally waits until
three counter ticks have elapsed; its unsigned addition and separate
counter-wrap branch must remain intact. The ending audio call supplies
128 Q8 units, half a master-volume unit per paced audio update, rather than
an unscaled 128-unit volume decrement. Reset-span names describe the
observed byte runs without asserting new complete object boundaries.

The card status helper reports I/O completion (1), timeout (2), new device
(3) or error (4); a status check that cannot start returns zero. Temporary
file cleanup also reports error 4 if its request fails, returns its existing
Boolean file-open result after completion/new-device events, and otherwise
forwards the card status. Startup interprets timeout as the no-card message.
Keep these mixed return conventions; do not collapse them into a new Boolean
API or rename timeout to a proved device-absence condition.

Share the already established floor-5 alternate-music variant (3) between
the resource selector and both warp paths, alongside floor-entry variant 1.
The resource selector's other floor/progress rules remain unchanged. The
actor transformation at the end of the warp unit names its result definition
and target event state without assigning unsupported identities to event
slots 1 and 2 or to the transformed actor.

The actor's motion rates need a quantitative explanation, not aliases which
merely repeat the destination fields. Retail GAME 80036d98/80036df0 adds
and subtracts 40 from world Y; 80036da0/80036df8 adds and subtracts 64 from
yaw. A full turn is 4096 angle units, so the yaw change is 1/64 turn
(5.625 degrees) per update. The matrix blend advances by 64 in Q12, also
1/64 of its full range. Spell these two ratios using their distinct angle
and fixed-point units. Both endpoints execute: the 0..4096 inclusive loop
has 65 updates, for 2600 world units of Y movement and 4160 angle units
(365.625 degrees) before the reverse phase restores both values. These
counts explain the observed behavior, not the designer's intent. There is
no evidenced derivation of the 40-unit increment; retain it as an explicit
animation parameter with that limitation. Do not derive it from an arbitrary
fraction of the map-cell size or invent a physical unit such as centimeters.

All 125 remaining inline literals in `game.c` and `player_warp.c` have these
specific reasons. The source/header definitions retain their reviewed raw
values, and the `ADDRESS`, `DATA` and `RODATA` claims retain their literal
ownership evidence.

| Sites | Values | Reason |
| --- | --- | --- |
| Initial counter data, cleared spans and shimmer intensity | 0 | Counter origin, zero-filled storage and absent intensity. |
| Display enable, near-actor-spawn reset and ending-request test/returns | 0/1 | Boolean values; the warp dispatcher reports whether the ending was requested. |
| Pacer interval and shimmer guard | -1/+1 | Convert the interval to the original strict comparison and preserve an inclusive intensity bound, respectively. |
| Shimmer loop origins/countdowns | 0 and -1 | First index/frame and descending-loop exhaustion; `count - 1` is the final element. |
| Occupancy adjustment before same-floor teleport | -1 | Remove one contribution from the old cell neighborhood. |
| Floor selector and destination floor IDs | 1..5 | Ordinal floor numbers in the authored route; no supported semantic location alias is needed. |
| Packed warp keys and destination cells | Every X/Z pair | Authored map coordinates, now visible through the packing macro or typed call arguments. Preserve the distinct trigger at (5,24) and return destination at (5,25). |
| Unnamed same-floor destination variant | 2 | Authored variant ID; neither an established location identity nor a special shared protocol meaning has been recovered. |
| Transformation color matrix | 250/100/500 repeated across its rows; zero translation | Authored lighting coefficients. Preserve their numeric samples without claiming a named color or deriving them from unrelated world coordinates. |
| Transformation event slots | 1/2 | Authored indices in this floor's event pool; this call site alone does not identify the events. |
| Transformation blend origin/end condition | 0/+1 | Mathematical zero and the inclusive full-blend endpoint. |
| Transformation range divisors | 64 | Divide the full blend range and full turn into 64 equal increments; inclusive endpoints produce 65 updates as detailed above. |
| Transformation Y increments | 40 | World units per animation update; the reason for choosing this speed remains unresolved. |
| Transformation render arguments | 0/0 | Null view arguments retain the renderer's current view. |

In the three reviewed card bodies, `_card_info` comparisons with zero test
request failure; `TestEvent == 1` tests its Boolean delivered result, and
`file != -1` tests the SDK file-open failure sentinel. Those literals remain
explicit. The updated floor-entry caller retains ordinal floor 5 and the
`floor - 1` conversion to its zero-based entry table. The rest of
`player_core.c` and `save_system.c` still needs its own literal audit; this
batch does not claim coverage of their untouched bodies.

The graphics-owner pilot rewrites the main-loop clear into its candidate
aggregate. Update that source-pattern match for the named clear extent;
keep the independent raw instruction check, literal extent and shifted-owner
negative controls intact. The first repository run exposed this stale
source-pattern expectation; it did not report a generated-code difference.

Validation after the fixture update: all eight graphics-owner tests and all
644 repository tests pass (nine repository skips). Lint and whitespace checks
pass. All 112 objects retain every non-debug section and all 484 strict scores
from 3a2fd7d; only three debug-line sections differ. The full build retains the
existing ownership and placement failures: GAME target relink 75/77, OPEN
34/38, PSX 1/1, and five of 60 source data owners matching, with no artifact
failures. The transformation remains strict 100%; no new result is banked.

## Incoming camera and map-cell source improvements

Integrate committed master `42ff64f`, including `4d59c88`, into the naming
worktree. The incoming [SDK vector publication](sdk-vector-publication.md)
and [GAME map-cell evidence](game-map-cell-source.md) establish natural XYZ
publication, the unsigned-byte object index and the ordinary door-remapping
switch. Resolve source conflicts by retaining those operations together with
the reviewed map-unit, fixed-point and angle names. Preserve the incoming
complete OPEN raw control and GAME's explicit eighteen stack-word differences.

Verification plan: compile literal incoming source controls for GAME camera
path, GAME map cells and OPEN map cells under the same current headers and
profiles. Compare every non-debug section against the merged named objects.
Require the other 109 objects and all unaffected strict scores to agree with
3a2fd7d. Exactly three incoming scores may improve: GAME 80033680 and OPEN
80018bbc to 100%, and GAME 8001e5ec to 96.743240%. Retain the incoming banking
records; this merge does not add a new banked result. Run repository and flake
checks and the full build before committing the reconciliation.

The three independent incoming controls match every non-debug section of
the merged named objects; OPEN's map-cell object is wholly identical. The
other 109 objects retain every non-debug section from 3a2fd7d. Exactly the
three expected strict scores improve, with the other 481 and every banked
exact unchanged. All 644 repository tests pass (nine skips), all flake checks
pass (644 tests, 131 sandbox skips), and lint/whitespace checks pass. The full
build retains its existing ownership and placement failures: target relink
PSX 1/1, GAME 75/77 and OPEN 34/38, five of 60 source data owners matching,
and no artifact failures.

## Map mesh banks and visibility classes

Function Match Plan: finish the literal audit of GAME `render_map_cell`
(8001e5ec), GAME `render_map_cells` (8001e83c), OPEN `render_map_cell`
(80018bbc), and OPEN `opening_render_map_cells` (80018d8c). The existing
complete image-qualified dossiers, incoming raw controls and current source
cover the emitters, traversals, callers, SDK boundaries, data references,
adjacent bodies and history. The selected resource loaders independently
identify the map TMD and visibility-table payloads. Preserve the byte object
index, narrowing casts, natural XYZ publication, callback order, loop widths
and raw relocation targets. Require all 112 non-debug objects and all 484
strict scores to remain identical to the 1a2cbce baseline.

The second raw TMD in each B1..B5 `MIXB.DAT` contains exactly 200 objects,
as does OPEN's B0 `MIXB0.` map TMD. The attribute is decremented as a byte
and rejected above 99. Window class 1 adds 100, selecting indices 100..199;
class 2 retains indices 0..99. Thus 100 is the number of map meshes in each
bank, distinct from the equal-valued map-grid width. Name that shared count.

The GAME `COM/COM.DAT` second chunk and OPEN `B0/RTBL.` each supply sixteen
204-byte records: four halfword dimensions/origins and 196 visibility bytes.
Every shipped directional window is 14 by 14 and uses only classes 0, 1, 2.
Class 0 is skipped; the six class-2 cells are a near-view rectangle inside
the central 3-by-3 neighborhood, while class 1 covers the outward region.
GAME's fixed window has a 3-by-3 class-2 center and class 1 around it. Name
these hidden/distant/near classes, but retain the explicit table pattern.
These are authored selections: a few directional class-1 cells also lie in
the central neighborhood, so do not replace the table with a distance test.

The second bank generally reduces geometric detail. Among the 100 paired
objects in floors 1..5, its primitive counts are fewer/equal/more in
59/41/0, 73/26/1, 63/37/0, 51/49/0 and 61/39/0 cases, respectively.
For example, every floor's objects 0 and 100 retain the same measured bounds
while changing from 28 polygons to four. The exception is B2 object 49/149:
two polygons in a small planar object versus fifteen in a cell-sized object.
OPEN has equal polygon counts in all 100 pairs. The bank names describe the
selection policy; they do not claim every second-bank mesh is a simplification
of its counterpart or that the two OPEN payloads are byte-identical.

GAME selects directional windows only for signed pitch -511..511, strictly
between -45 and +45 degrees in the 4096-unit turn. Express its original
511/1023 bias/span using an eighth turn and endpoint adjustments; retain
both unsigned-halfword casts and the original comparison. The zero-based
orientation tests derive from the already named one-based quarter/half/
three-quarter-turn encodings. The light timer's -1 exhaustion and two-on/
two-off remapping remain explicit, as does the authored three-pair mesh
mapping; neither a timer-phase alias nor an invented door identity adds
evidence about why those particular values were chosen.

All 201 remaining inline literals in the two map-render source files have
these reasons. The retail ownership claims remain literal evidence as in the
rest of the campaign.

| Sites | Values | Reason |
| --- | --- | --- |
| Fixed-window dimensions and origin | 13/13, 6/6 | Authored 13-by-13 sampling area with its origin at the center cell; its chosen extent is not derived from a separately proved rendering limit. |
| Fixed-window cells | 0/1/2 | Explicit hidden/distant/near pattern, with a source legend and shared class names. Numeric rows preserve the shape's readability; the remaining 27 bytes of the 196-byte array are implicitly zero-filled. |
| Light timer test | -1 | Exhausted countdown disables remapping; initialization and the per-frame decrement independently establish this sentinel. |
| Light timer mask and comparison | 3/2 | Modulo-four phase with two active and two inactive updates. The choice of that flicker period remains an authored timing parameter. |
| Light mesh-remapping switch | 44/45/46 -> 17/18/19 hex | Authored pairs of one-based map attributes. Their individual semantic mesh identities remain unresolved; the ordinary switch preserves their real control flow. |
| Mesh bound | -1 | Last zero-based index in a bank with the named mesh count. |
| Orientation subtraction and comparisons | -1 | Convert both the serialized value and named orientation tags from one-based encoding to zero-based matrix indices. |
| Pitch bias and span | -1 and 2 | Strict endpoints around zero: two equal half-ranges, minus one, with the original unsigned-halfword wrap behavior. |
| Directional-window index | -1 | Last index before reversing the sixteen yaw bins. |
| Row and column countdown tests | 0 | Exhaustion of the existing byte counters. |

The shared header's remaining 196-byte cell-array extent is the independently
measured serialized payload capacity (14 by 14 in both shipped directional
tables), not the fixed window's 13-by-13 active area. It is intentionally
unchanged by this source audit.

Validation: all 644 repository tests pass (nine skips), and lint and
whitespace checks pass. All 112 objects retain every non-debug section and
all 484 strict scores from 1a2cbce; only GAME's map-unit debug-line section
changes. The OPEN emitter/traversal and GAME traversal remain strict 100%,
and the GAME emitter remains 96.743240%. The full build retains the existing
ownership and placement failures, with target relink PSX 1/1, GAME 75/77 and
OPEN 34/38, five of 60 source data owners matching, and no artifact failures.

## Save directory and workspace contracts

Function Match Plan: follow the save-summary field recovery through catalog,
slot read/write, workspace allocation and the two menu summary arrays. Name
the three logical save slots, four physical directory entries, empty entry
tag zero and spare entry tag four. Preserve the first-spare/first-empty
search order, old-slot lookup, unguarded previous-entry store and all retry
and status conventions. The menu passes `cursor + 1` for logical slots 1..3;
the catalog ignores directory tags zero and four. These independent producer
and consumer roles distinguish counts from encoded tags despite equal values.

The workspace's 0x2800-byte allocation is exactly the 0x280-byte header plus
one 0x2580-byte slot payload. Express that relationship with the existing
complete types and assert their independently measured extents. The payload
still starts at `save_header_buffer + 1`. The card file separately reserves
five blocks for the header and four physical payload positions; this workspace
allocation is not the file's capacity. The remaining save status, icon-format
and menu constants continue to need their own detailed audit.

The six summary indices are replaced with supported named members rather
than index aliases. Layout-check literals are independent measured extents
and offsets, with 1/-1 denoting valid/invalid compile-time array bounds.
The catalog's 0x24-byte clear remains an explicitly documented retail span;
zero is its fill byte, and `slot - 1` converts the one-based logical ID.
The slot searches retain -1 for no entry found, including the existing
unguarded previous-entry store. Workspace zero tests/returns and -1 failure
retain their allocator/Boolean conventions; `header + 1` advances one complete
header. Other unchanged literals in the save and menu bodies still await
their subsequent batches. The full object/score comparison, 644 repository
tests and flake checks pass as recorded in the save-summary field audit;
the existing full-build ownership failures are unchanged.

## Save icon format constants

Function Match Plan: the [save-icon header field recovery](semantic-field-names.md#playstation-save-icon-header)
covers GAME 8002c304, its complete dossier and the three shipped TIM assets.
The three-frame type 0x13 is a format tag, distinct from the number three.
The initializer uses the shared CD-sector byte count for its rounded read
buffer and copies each destination field's `sizeof` bytes. The fixed TIM
source offsets identify the palette and pixel payload in ICO1..ICO3 only.

The remaining initializer and newly introduced layout literals have these
reasons; the rest of the save unit's pending status, path and menu audit is
not claimed by this batch.

| Sites | Values | Reason |
| --- | --- | --- |
| Initial clears | 0 | Zero-filled header/payload storage; also supplies the format-required pad and unused title bytes. |
| Signature byte indices and characters | 0/1, `S`/`C` | The two bytes of the literal file signature, in their serialized order. |
| Icon frame indices | 0/1/2 | First, second and third asset, in animation order. |
| Header array extents | 2, 64, 28, 16, 3, 128 | Two signature bytes, 64 Shift-JIS title bytes, 28 required zero bytes, sixteen palette entries, three frames, and 16 * 16 / 2 packed bytes per frame. These measured format dimensions do not need another layer of aliases. |
| Layout assertions | 0x200; 00/02/03/04/44/60/80 hex; 1/-1 | Independent serialized size and member offsets; valid/invalid compile-time array bounds. |
| Title and asset strings | All encoded characters and frame digits | Authored card title and literal file paths. Preserve the title's 53-byte copy including its terminator; the 64-byte field capacity is not the copy length. |

The five-block allocation accommodates the 640-byte header and four
9600-byte physical payload positions: 39040 bytes fit in five 8192-byte
blocks. Three logical save slots plus one spare entry explain the fourth
payload position. The value five is already named `SAVE_FILE_BLOCKS`;
its storage relationship is supported independently of the equally valued
retry caps, whose choice is still an authored policy.

Validation agrees with the field audit: every non-debug object section and
all 484 strict scores are unchanged, including the initializer's 100% result.
The repository tests, flake checks and lint/whitespace checks pass; full
`kf build` retains the existing data-ownership and placement failures.

## Incoming effect constructor

The [effect field-width reconciliation](semantic-field-names.md#incoming-effect-constructor-and-field-widths)
retains 282cbe7's exact constructor, source joins, complete SDK vector copies
and repaired retail references while preserving the naming campaign's pool
capacity/free-slot constants and typed semantic fields. The four incoming
source units have independent compiler controls; the existing raw-word and
45-row switch-table tests continue to compare directly against GAME retail.
This integration does not claim completion of the effect family's remaining
animation, asset-ID, range and timing literal audit.

The added layout assertions retain measured offsets and byte extents as
independent evidence. Their 1/-1 values are valid/invalid compile-time bounds.
The low/high byte view at control +0x38 preserves kind 36's two 0xff writes;
their designer purpose is still unknown. The parent-index and homing-selector
members instead have proved byte stores and reads, so they remain named and
are narrowed to the correct width. No unexplained store is relabelled as a
countdown or branch merely because another kind uses that same storage.

The four incoming controls, raw retail checks, all 651 repository tests and
flake checks pass. Only the incoming constructor's strict score improves;
all other 483 scores and every non-debug section of the other 111 objects
remain unchanged. Full-build source-data matches improve to six of 60,
while the existing ownership and placement gates remain unfinished.

## Floor-deformation progress and incoming byte loop

Function Match Plan: integrate 51ebe75's GAME 800384f8 floor-deformation
correction into the naming worktree. The incoming complete dossier, current
image-qualified disassembly/CFG, two callers, audio callee, data/table
references, adjacent helpers and history establish the byte coordinates,
explicit absolute-step calculation and signed predecrement countdown.
Preserve its 115 exact retail words and the incoming banking record.
Retain the map-height/cell-size/center constants, and express the progress
clamp and interpolation shift using the shared Q12 units. The maximum-volume
argument uses the established audio limit. Require an independent literal
incoming-source control, all other 111 objects and 483 strict scores to
remain unchanged from dd1e366; only this function may improve to 100%.

The sound window is 3900 <= progress < 3900 + abs(step), inside the original
unclamped 0..4096 arm. Thus its onset is about 95.2% of the height blend;
the window width follows the step magnitude. Zero step produces an empty
window. The exact choice of 3900 is an authored timing parameter without a
proved derivation, so retain the literal and explain its units/behavior.

| Remaining literal | Reason in this function |
| --- | --- |
| 0 | Sign test, zero clamp and interpolation origin. |
| -1 | Exhausted signed countdown; the predecrement also handles a zero cell count without entering the loop. |
| +1 | Preserve the inclusive full-Q12 endpoint in the original strict comparison. |
| 3900, twice | Authored sound-onset progress and lower edge of the step-sized window; its precise designer rationale is unresolved. |

No other effect-update helper is claimed fully audited by this reconciliation.

The independently compiled literal incoming unit and named unit match every
non-debug section. The floor helper reproduces all 115 retail instruction
words and its ordered sound/data referents; all seven siblings preserve their
previous raw linked words, calls and addresses. The other 111 objects retain
every non-debug section from dd1e366. Only GAME 800384f8 changes strict score,
from 86.704346% to 100%; the other 483 rows and all banked exacts are unchanged.
All 651 repository tests pass (nine skips), and lint/whitespace checks pass.
The full build retains its existing ownership/placement failures: six of
60 source-data owners match, target relink is PSX 1/1, GAME 75/77 and OPEN
34/38, and there are no artifact failures. No tooling or flake files changed.

## Save statuses, messages and I/O attempts

Function Match Plan: name the internal save statuses and their direct message
asset IDs in `game.save_system`, using the complete image-qualified dossiers,
current producer/consumer paths and decoded retail TIM messages. Preserve
the mixed BIOS-event, file-policy and menu-result contracts, all switch-arm
orders, calls, delay slots and relocation referents. The game-specific file
paths, summaries, retry policy and UI dispatch exclude these wrappers from
vendored library ownership. Require every non-debug section of all 112
objects and all 484 strict scores to remain unchanged from 93be2d6.

| GAME function | Final strict result, unchanged | Evidence and change |
| --- | --- | --- |
| 8002b4d8 `memory_card_check_or_format` | 100% | BIOS event 3 refreshes the card; event 1 either formats or requests confirmation. Keep its menu-result mapping. |
| 8002b5d0 `memory_card_format` | 100% | Five format attempts; failed format returns status 11, otherwise forwards the subsequent card event. |
| 8002b648 `save_system_write_slot` | 100% | Refreshes a new card, invokes the file writer and maps internal statuses to menu results. |
| 8002b73c `save_file_write_slot` | 100% | Create probes select format-required/no-space messages; file-open and exhausted short writes select write failure. |
| 8002bc30 `save_system_read_header` | 100% | No-data and new-device cases retain the original catalog-ready result. |
| 8002bd08 `save_file_read_header` | 100% | Missing/open failure differs from an exhausted short read. |
| 8002bde4 `save_system_read_slot` | 100% | A newly reported card invalidates the cached catalog; preserve the message and early return. |
| 8002beb0 `save_file_read_slot` | 100% | No slot/file, changed summaries and short reads have distinct status tags. |
| 8002c510 `memory_card_show_status_message` | 97.980770% | The original 15-case dispatch selects verified `TIM/Mddd.` images; keep the existing residue and pass-through default. |
| 8002c5e0 `menu_load_message_image` | 100% | Direct decimal asset ID, skip value 255 and zero-success/one-load-failure convention. |

Use the pinned SDK's `SEEK_SET` for file-origin seeks. Name total attempts,
including the first operation, rather than incorrectly calling five the
number of retries. File I/O and formatting retain separate five-attempt
limits; neither is derived from the file's five-block allocation.

The learned-flag loops use the serialized `u8` array's extent as their count,
cast to `s32` to preserve the retail signed comparisons. An unsigned `sizeof`
bound changed exactly those two `slti` instructions to `sltiu`; keep the
observed signed index/count contract.
Remove their unused `record` counters and `+= 20` statements: neither counter
has a read, and the indexed `KfMagicRecord` accesses independently supply the
real 20-byte record stride. Retain all copied ranges, explicit summary-field
comparisons, payload/header size locals and the unguarded previous-slot store.

The [save-system protocol](../save-system.md#card-events-and-status-codes)
records every named status producer, wrapper mapping and decoded message.
The selected UI text supports policy names; it does not prove a filesystem
root cause. The file and format attempt limits are separately five, with
the choice of that count unresolved. Do not derive either from the unrelated
five-block allocation.

| Remaining literal in these ten functions | Reason |
| --- | --- |
| 0 in loop initializers, comparisons and file offsets | Index/attempt origin, Boolean false, or beginning of the file; the seek-origin argument now uses SDK `SEEK_SET`. |
| 1 in increments and format API checks | One array element/attempt at a time, or the SDK's observed true/success result. |
| Menu results 0, 1, 2 and 3 | Operation-specific mappings documented in `save-system.md`; these are not interchangeable with BIOS or internal save statuses. |
| -1 in file and entry checks | SDK open failure or unsuccessful directory search. Keep the existing previous-entry store even when its index remains -1. |
| 16 in create flags | BIOS file-open encoding places the memory-card block count in the upper halfword; it is not a size or alignment estimate. |
| Cases 9 and 10 | Both select the unusable-card image; separate producers and meanings remain unresolved. |
| -1 in status-message dispatch | The literal value assigned for status 1 differs from the loader's skip ID 255; the other -1 converts loader failure to the dispatcher's error result. |
| 16 in the message path array | Observed local buffer capacity; the path text alone does not justify changing its extent. |
| 5, 6 and 7 in path subscripts | Hundreds, tens and units positions in literal `TIM\\M000.`. |
| 100, 10 and `'0'` in path construction | Decimal digit extraction and character encoding. |
| 0 and 1 in the message loader/dispatcher | Loader success or skip returns zero, CD-load failure returns one, and the dispatcher converts that failure to -1. |

Other functions in the save unit retain their earlier audit status; this
batch does not claim their remaining literals have all been reviewed.

Validation: every non-debug section of all 112 objects and all 484 strict
scores are unchanged from 93be2d6; only the save unit's debug lines differ.
Each function in the table retains its recorded result, including the
status dispatcher's existing 97.980770% residue. The 651 repository tests
pass with nine skips, and lint/whitespace checks pass. The full build still
fails on the existing data-ownership and placement issues: six of 60 source
data owners match, with target relink PSX 1/1, GAME 75/77 and OPEN 34/38,
and zero artifact failures. No new result is banked. No tooling or flake
files changed.

## Sound selectors and menu cue IDs

Function Match Plan at 6a22c63: correct the two GAME/OPEN wrappers currently
called `audio_key_off_mask`, and name the three menu sound cues through their
19 caller functions. The original complete dossiers, refreshed six-view
snapshots for both wrappers and the cue provider, all 89 source call windows,
adjacent audio helpers, source history and current object/score snapshot
support this batch. Preserve each argument, call, branch, delay slot and
ordered physical referent. Require identical executable/data/relocation
sections and all 484 strict percentages; only the intended two symbol
spellings and debug lines may change.

The two wrappers each load unsigned bytes at offsets 0 and 2 and make one
SDK call; no caller, data reference or string is admitted for either. Their
24-byte frames and note-shift call delay slots match across images. The
SDK's decoded byte selectors and the established `SoundRef` field family
identify program and note. Use the shared typed view and the precise
`sound_ref_key_off_bank0` identity: neither wrapper reads the active VAB
state or the tone byte. The absent caller leaves the original enclosing
object, use site and original symbol unresolved; this does not create a
new data-owner claim. The game wrappers are not vendored SDK bodies.

The cue provider keeps its 32-byte frame, three program/note recipes,
`SsVoKeyOn`, one `VSync(0)` and `SsVoKeyOff`, including both note-shift delay
slots. Cue 0 is the cursor/opening cue, also reused for empty-list and config
actions; 1 confirms a choice; 2 cancels/dismisses or rejects an invalid choice.
Use `MENU_SOUND_CURSOR`, `MENU_SOUND_CONFIRM`, and
`MENU_SOUND_CANCEL_OR_ERROR`. Preserve the provider's catch-all third arm:
values other than 0 or 1 still use the same recipe. These names identify
feedback styles, not a rule that every caller uses the same button mapping.

| Image/function | Starting strict result | Evidence and focused change |
| --- | --- | --- |
| GAME 80033014 `audio_key_off_mask` | 100% | Two byte loads; exact-FID `SsVoKeyOff` at 8004b6a4; rename and use program/note fields. |
| OPEN 8001a188 `audio_key_off_mask` | 100% | Same ten-word wrapper; exact-FID `SsVoKeyOff` at 8002b478; same typed contract. |
| GAME 8002b150 `menu_play_input_sound` | 100% | 33 retail words; cue compares 0/1, catch-all third recipe, equal channel volume 64 and one vertical-sync wait. |
| GAME 800212d8 `item_menu_root` | 100% | Five cue calls: opening/navigation, confirmation and cancellation. |
| GAME 80021538 `item_menu_buy` | 97.200540% | Six cue calls, including insufficient-gold rejection. |
| GAME 80021afc `item_menu_sell` | 96.771870% | Five cue calls: empty-list exit, navigation, confirmation and cancellation. |
| GAME 80021ffc `item_use_confirm` | 100% | Four cue calls: opening, choice toggle, confirmation and cancellation. |
| GAME 800222b4 `menu_save_confirm` | 100% | One opening cue. |
| GAME 80022348 `menu_root` | 96.863640% | Five opening/navigation/confirmation/cancellation calls. |
| GAME 80022608 `menu_use_item_panel` | 92.821800% | Five calls, including the cue on return from the map viewer. |
| GAME 8002317c `menu_magic_panel` | 95.897590% | Five empty-list/navigation/confirmation/cancellation calls. |
| GAME 800236ac `menu_option_root` | 100% | Five calls, including equipment-dependent rejection. |
| GAME 800238d8 `menu_equip_select` | 97.899730% | Five empty-list/navigation/confirmation/cancellation calls. |
| GAME 80023e9c `menu_spell_select` | 98.063380% | Five empty-list/navigation/confirmation/cancellation calls. |
| GAME 8002430c `menu_status_panel` | 78.468090% | One dismissal cue. |
| GAME 800249a8 `menu_drop_item` | 98.854780% | Five empty-list/navigation/confirmation/cancellation calls. |
| GAME 80024e64 `menu_save_load_hub` | 93.611840% | Four navigation/confirmation/cancellation calls. |
| GAME 800250c4 `menu_save_panel` | 96.797874% | Eight calls, including status-image entry and dismissal. |
| GAME 8002552c `menu_load_panel` | 92.613640% | Nine calls, including empty-slot rejection and status-image dismissal. |
| GAME 8002589c `menu_config_panel` | 30.713396% | The old reconstruction used five cue-0 calls. Retail uses 0/0/1/1/2; the configuration-panel reconciliation below corrects this earlier source-derived assertion. |
| GAME 80028380 `menu_list_interact` | 87.079810% | Three choice-toggle/confirmation/cancellation calls. |
| GAME 800286d4 `menu_two_option_prompt` | 100% | Three choice-toggle/confirmation/cancellation calls. |

Program IDs 14/13/15 and MIDI note numbers 68/60/63 remain the observed
sound recipes. Their exact selection has no recovered author rationale;
do not invent instrument names or infer the audible frequency of sampled
sounds. Volume 64 is 64/127 of the channel control range, with equal left
and right inputs; its precise tuning is likewise unresolved. Shift 8 places
the note in the SDK argument's upper byte and leaves fine pitch zero, rather
than expressing a Q8 gameplay value. `VSync(0)` is the existing SDK wait mode.

### Remaining save-unit literal audit

The current census has 154 retained inline numeric/character occurrences
in `src/game/save_system.c`, excluding claims, definitions and string contents.
The save-status, icon-header and sound-selector sections explain their
respective subsets. Reviewing the remaining card, workspace, image-wait and
talk-path functions completes the literal accounting for this unit; it does
not resolve every opaque serialized field or every original author decision.

| Remaining site/value | Reason |
| --- | --- |
| `save_system_read_catalog`: clear length 0x24 | Observed partial clear of an output whose three summaries span 0x48 bytes. Its rationale is unresolved; do not silently expand it. |
| Catalog result 1, index 0, slot - 1 | Operation-specific catalog-ready result, first index and one-based slot to zero-based row conversion. |
| `memory_card_initialize`: 0x80-byte local and 0xff fill | Retail retains this initialized local, but the function never passes it to a card service or otherwise consumes its contents. Its purpose is unresolved; matching a card-sector size alone does not justify a sector-buffer identity. |
| `OpenEvent`: callback 0 | Null callback for the selected non-interrupt event mode. |
| `_card_auto(0)` | Retained SDK control argument; no additional project mode name or designer rationale is established by this call. |
| `_card_info` comparisons with 0 and `TestEvent` comparisons with 1 | BIOS operation-start and event-ready results, distinct from the game status number space. |
| `save_file_cleanup_temporary`: file != -1 | SDK open success Boolean on the cleanup path. Other paths still forward card-event statuses, so the whole helper is not normalized to a Boolean return. |
| Workspace null/return 0 and -1 | Null allocation, zero-success and negative-failure conventions. |
| `save_header_buffer + 1` | The payload begins after one complete typed header. |
| Zero-fill values throughout catalog/header/payload initialization | Clear the measured byte ranges; numeric zero is the stored representation, not an invented field state. |
| Header signature `'S'`, `'C'`; subscripts 0/1 | Required two-byte PlayStation save signature and its byte positions. |
| Icon frame subscripts 0/1/2 in destinations and `sizeof` | The three ordered images and the extent of each corresponding frame. |
| Image-wait `DrawSync(0)` and `SetSemiTrans(..., 1)` | Existing SDK wait mode and enabled semitransparency. |
| Image-wait UV coordinates 0 and `GetClut` X 0 | Texture-coordinate origins and the measured palette column; the other dimensions/positions already have shared screen-layout constants. |
| Image-wait CD result 0 | CD loader's success convention. |
| `buffer_index == 0`; draw-environment flags 0/1 | Select the opposite buffer and clear/restore the background/display-area flags. Preserve both explicit flag assignments. |
| Image-wait `ordering_table[length - 1]` | Last entry in the reverse-linked ordering table. |
| Image-wait pressed flag 0/1, `PadRead(1)` and input comparisons with 0 | Wait for release, then a new press, then release again. Zero/one are flag states and no-input checks; the linked PadRead ignores the supplied 1, as documented in the input audit above, so it is not a controller-port selector. |
| Talk-path divisors 10 and character `'0'` | Decimal tens/ones extraction and character encoding. |
| Talk-path indices 6, 0xa..0xe and directory-group indices 0/1 | Existing directory digits and filename prefix/index/group/frame positions in the path template; both group copies use the same tens/ones. No gameplay ID is represented by these subscripts. |

The image-wait brightness variables also describe measured tuning: the
counter starts at 32, increments before drawing, and caps at 127. Therefore
the first submitted RGB triplet is 33/33/33, and the 95th loop iteration
reaches 127/127/127. No fixed frame duration is inferred from this loop,
and the reason for choosing its initial level and exact ceiling is unresolved.

Final verdict: all 22 functions in the sound/cue plan retain their starting
strict percentages. The two `sound_ref_key_off_bank0` bodies retain all ten
retail words each, and `menu_play_input_sound` retains all 33, with the exact
ordered SDK calls and no data referents. Reversing the 89 cue substitutions
reproduces every caller source byte from 6a22c63. All 112 compiled objects
preserve executable bytes, data and relocations; the only other differences
are debug lines and the two intended names in ELF symbols and decoded DWARF
records. All 484 strict rows and the complete report agree after those name
substitutions. The save unit retains 22 exact functions and its two previously
documented residues. The banked source hashes and score floors are retained;
only the two baseline display names follow their corrected identities.

Lint and whitespace checks pass. All 651 repository tests pass with nine
skips. The full build retains the existing ownership/placement failures:
source-data matching is 6/60, target relink is PSX 1/1, GAME 75/77 and OPEN
34/38, with two GAME and four OPEN conflicting-section-base findings and
zero artifact failures. No tooling or flake files changed.

## Effect collision reconciliation and target classes

Function Match Plan: integrate committed master a0ffa9b into c403eac's
naming worktree. GAME 80037850 `effect_map_collision` starts here at
74.821050%; the incoming source is 97.452630%, still partial. Its complete
incoming dossier and refreshed image-qualified six-view snapshot establish
the signed cell coordinates, direct neighbor indexing, early effect-pointer
snapshot, selected rectangle, absolute-height update, backward span join,
ordered diagonal expression and case-local query arguments. Preserve these
facts, the fifteen reviewed internal-jump referents, all 475 delinked retail
words, six switch entries and the 56-byte rectangle initializer.

Retain the existing map dimensions/units, cell/orientation enums, terrain
result and query exclusion flags. Name the low-two-bit effect classes
locally by their actual world-query target policy: class 1 permits actors,
class 2 permits the player, and class 3 permits both. All three exclude
terrain, map objects and map events in the downstream query because this
wrapper has already handled its map-geometry checks. Class 0 still returns
the literal encoded result 1 without a world query; do not reinterpret it as
the -1 no-collision sentinel. No semantic claim about other effect-type bits
is made by this local class enum.

Require a separate compile of literal a0ffa9b source using the same pinned
profile/headers to agree in every non-debug section with the named source.
The other 111 objects and 483 strict rows must remain unchanged from c403eac;
only this function may acquire the incoming improvement. The game-grid,
attribute, effect-class and caller policy exclude SDK body ownership. The
six-word source deficit and documented return/join/coordinate residues remain
open; no new result is banked.

The reconciled unit retains 44 inline literal occurrences:

| Remaining site/value | Reason |
| --- | --- |
| Rectangle extent 7 and its 28 signed coordinates | Complete measured seven-record asset, in oriented cell X coordinates and floor-relative world Y. Preserve the last three rows' inverted Y bounds. Their exact authored dimensions remain unexplained; replacing them with map-unit arithmetic would suggest an unproved derivation. |
| Three comparisons with 0 | Signed cell lower bounds and the split between a negative scalar height and a nonnegative rectangle selector. |
| Eight neighbor offsets +1/-1 | One adjacent row or column, with the repeated direct accesses required by the incoming source. |
| Three world-query height arguments 0 | The effect passes zero vertical extent with its radius. This is distinct from the query's 0xffff ignore-height sentinel. |
| Default result 1 | The literal result for effect class zero, without a downstream query. Preserve its encoded value; its broader author intent remains unresolved. |

The diagonal checks use a fixed half-cell margin of 1000 world units, and
the sum boundary is 2000 + 1000 = 3000. They do not derive this margin from
the caller's radius. The shared cell-size/center spellings expose those
measured relationships without attributing the choice to an unknown collision
or animation design rule.

Final verdict: the named source and an independent compile of literal
a0ffa9b source agree in every non-debug section; only `.debug_line` differs.
The other 111 objects retain all non-debug sections from c403eac. The sole
strict score change is GAME 80037850, 74.821050% to 97.452630%; the other
483 scores, including all banked exact results, are unchanged. The delinked
target restores every one of the 475 retail instructions and all six switch
entries. The source remains 469 instructions long. Both objects retain the
56-byte rectangle initializer, the single ordered world-query callee and all
twelve ordered physical data references.

`ruff check scripts tests` and `git diff --check` pass. The repository suite
passes 651 tests in 87.274 seconds with nine skips, including the existing
collision controls. The full build still fails on the existing source-data
ownership and placement findings: 6/60 units match data, with zero artifact
failures. Target relink remains PSX 1/1, GAME 75/77 and OPEN 34/38, with two
GAME and four OPEN conflicting-section-base findings. No result is newly
banked, and no tooling or flake files changed.

## Menu glyph encoding and atlas dimensions

Function Match Plan: name the shared GAME text terminator, empty glyph,
decimal-atlas blank/slash and kana voicing flags from the renderer and retail
assets. Preserve the signed halfword code stream, unsigned descriptor fields,
all SDK packet types, function shapes, literals' values and ordered referents.
The two renderers and decimal formatter define the contract; their label and
quantity producers form the related campaign. All fourteen affected functions
have refreshed image-qualified addr/disassembly/CFG/xrefs/callees/strings/match
snapshots and source history under the ignored menu-glyph-names evidence
directory. The existing glyph-render and list-format dossiers cover their
neighbors, ABI, primitive-provider boundary and non-vendored ownership.

The proposed names distinguish text glyph 255 from number glyph 10, though
both draw an empty cell. Number glyph 11 draws a slash. Text bits 0x1000 and
0x2000 independently add dakuten and handakuten marks; the low twelve bits
select the base glyph. Name composite voiced glyphs as a flag OR the retained
authored character index. The decimal divisor 10 stays literal: it is a radix,
not the coincidentally equal blank-glyph index. Likewise, -1 used as a loop
index or result is not a text terminator. Selection arrays named `codes` hold
item/spell IDs; their 255 sentinel is not a blank text glyph.

The font has sixteen columns of 14-by-12 cells. The number atlas advances
seven screen pixels and eleven texture rows per code. These dimensions agree
with the runtime-loaded descriptors and the decoded pixels; they are asset
contracts, not inferred animation tuning. The mark cells occupy columns 14
and 15 of row 2, giving UV origins (196,24) and (210,24). Keep the existing
four signed-versus-positive-immediate discrepancies in the string renderer;
the atlas does not justify a false exact claim. Text ordering-table depth
1000 is a shared draw-layer convention; no recovered rationale selects that
particular numeric depth.

Require every non-debug section of all 112 objects, every one of the 484
strict function rows and the complete strict report to remain unchanged from
58ed256. Rebuild the affected sources, inspect raw renderer/formatter words
and ordered relocations, then run the full build and repository checks. No
new exact result is proposed for banking.

The three format-defining bodies retain eleven inline numeric occurrences:

| Remaining site | Reason |
| --- | --- |
| Three index initializations to 0 | Start at the first glyph/output slot. |
| `pad_zero == 0` and its alternative glyph 0 | Test the zero-padding flag and select the actual decimal digit zero. |
| `count - 1` and index lower bound 0 | Visit the requested output positions from last to first. |
| Two decimal divisors 10 | Base-ten quotient and remainder, independent of blank glyph 10. |
| `value == 0` | Stop when no more decimal digits remain. |
| `i = -1` | End the signed index loop through its existing decrement/test tail; no terminator store occurs here. |

The shared header owns the seven encoded values. The renderer's private
constants own atlas dimensions, mark-cell coordinates and text depth. Their
numeric choices are either the measured file format or the existing draw
layer; the original artist's reasons for choosing 14-by-12 and 7-by-11 glyph
sizes remain unknown. Other authored character codes stay literal asset
indices. The atlas, exact offsets and hashes are documented in
[game-menu-glyph-render.md](game-menu-glyph-render.md#retail-glyph-format-and-asset-evidence).

The affected GAME functions have these starting scores; each must retain
the same score and all raw source-object sections outside debug line tables:

| Address | Function | Strict score |
| --- | --- | --- |
| 800238d8 | menu_equip_select | 97.899730% |
| 80023e9c | menu_spell_select | 98.063380% |
| 8002589c | menu_config_panel | 30.713396% |
| 80025f38 | menu_draw_stats_header | 97.686110% |
| 800264d8 | menu_draw_status_details | 95.110700% |
| 800279c4 | menu_item_model_preview | 98.181816% |
| 80027b7c | menu_draw_item_detail | 91.284160% |
| 80027ee4 | menu_draw_dialog_frame | 100% |
| 80028380 | menu_list_interact | 87.079810% |
| 800286d4 | menu_two_option_prompt | 100% |
| 80028a70 | menu_list_render | 88.252610% |
| 80029de0 | menu_draw_string | 99.987950% |
| 8002a310 | menu_draw_number | 100% |
| 8002adf8 | menu_format_number | 100% |

Final verdict: every row above is unchanged, as are the other 470 strict
rows and the complete report. All 112 objects retain every non-debug section;
only the menu-runtime object's debug line table differs. An independent
source-value audit also preserves every numeric value, expression and
statement across the ten edited C files. The number renderer matches all
128 retail words, with two ordered calls and eight data references; the
formatter matches all 43 words without relocations. The string renderer
retains precisely the documented four differences among 332 words, with
the same six ordered calls and 24 data references. Both blank cells decode
entirely to palette index zero.

Ruff and diff checks pass. The repository suite passes 651 tests in 103.328
seconds with nine skips. Full `kf build` still fails on the existing data
ownership/placement findings: source-data matching is 6/60, target relink is
PSX 1/1, GAME 75/77 and OPEN 34/38, and all images have zero artifact failures.
The two GAME and four OPEN conflicting-section-base findings are unchanged.
The post-test section, score, raw-word and source-value checks pass. No new
result is banked; no tooling or flake files changed.

## Player poison status and timing

The [poison campaign](game-player-poison.md) identifies status mask 4 from
the retail `毒` glyph, then joins it to equipment resistance, application,
countdown and periodic HP loss. It names the mask, initial 600-update timer,
-1 inactive sentinel, 20-update damage interval and two-update flash while
preserving the signed countdown and final zero tick. The initial timer gives
30 one-HP damage ticks if processing is uninterrupted; the next update clears
the status. Its original tuning rationale remains unknown.

The campaign dossier accounts for the remaining poison-path literals:
initial resistance 5, the zero equipment sum, random scaling 100/shift 15,
phase zero, one-HP loss, the UI's resistance weight /5 and authored poison
glyph 0x88. It also records why composite cure masks must keep their exact
upper-bit behavior. Other fields and status flags require their own evidence.

## World collision and map-object reset reconciliation

Function Match Plan: import committed master 11c81c4 into the 7804c83 naming
worktree, including b19ed81. Preserve the named map dimensions, collision
results/flags, pool capacity and shared semantic link fields. GAME 8001a5ac
`collision_query_world` starts at 93.202490%; incoming 97.943924% recovers the
early typed definition pointers, common candidate-hit value and separate
rejection-mask computation. GAME 80030f7c `map_object_pool_clear` starts at
78.25%; incoming 100% clears the existing complete link through the placement
loader's aligned word view. Its byte/halfword field layout remains shared,
including `linked_notification` and `default_notification`.

The committed world-collision and pool-clearing dossiers supply full retail
CFGs, call sites, neighbors, raw referents and independent source controls.
Refresh both image-qualified six-view snapshots and source history before
merging. Keep the incoming shared layout guards, the 255 signed height
entries, all 24 map-copy bytes, ordered transform/radius publications and
reset's upper-word-before-lower-word clear. The unit's 84 switch pointers
must acquire the incoming exact offsets. These game object/grid operations
are not vendored bodies; the failed memset experiment is not repeated.

Require a separate compile of literal committed 11c81c4 source for each
affected unit to agree with the reconciled named object in every non-debug
section. The other 110 objects and 482 strict scores must remain unchanged
from 7804c83. Verify all raw reset words and its five exact neighbors, all
world-query target words, complete initialized bytes and ordered referents.
Preserve the incoming reset bank row without banking additional results.
The user's staged main-checkout work is outside this committed import.

Final verdict: both reconciled units agree with independent compiles of the
literal 11c81c4 source in every non-debug section; only debug line tables
differ. Those control compiles use 11c81c4's map header for its older event
rotation spelling, then restore the shared named header. The kept event
field remains `rotation_x`, and both notification fields retain their names
and byte widths. All other 110 objects retain every non-debug section from
7804c83. Exactly two strict scores improve as planned, with the other 482
scores and every prior exact unchanged.

The world-query target restores all 321 retail words and both height-table
payloads retain all 510 bytes. Source remains 319 words; the five calls,
21 ordered data addresses and four complete transform/radius publication
blocks agree. The pool reset restores all 24 words and four physical address
pairs; its five exact neighbors retain all 521 words. Both pool objects also
restore every one of the 84 switch destinations and the 24 map-copy bytes.
The named capacity still yields the original 190 iterations. The two word
indices 1/0 denote the upper/lower halves of the owned link; their zero stores
clear it, and the following three zero stores reset the sequence counters.

Ruff and diff checks pass, and all 651 repository tests pass in 86.437 seconds
with nine skips. Full `kf build` still fails on remaining data ownership and
placement findings, but source-data matching improves from 6/60 to 7/60
(GAME 5/40, OPEN 2/19, PSX 0/1). Target relink remains PSX 1/1, GAME 75/77 and
OPEN 34/38, with two GAME and four OPEN conflicting-section-base findings and
zero artifact failures. Post-full-build control checks pass. The incoming
single reset bank row is preserved; no additional result is banked and no
tooling or flake file changed.

## Map-copy descriptor reconciliation

Function Match Plan: import committed master 3b1c834 into the ebf34e7 naming
worktree. GAME 80030a98 `map_apply_copy_region` starts at 94.256195%; the
incoming ordinary source-before-destination column advances reach 100%.
The refreshed six semantic views agree with the committed
[map-copy dossier](game-map-copy-region-source.md): 484-byte leaf, eight
blocks, nine direct callers, no calls or strings, six unsigned descriptor
fields, byte countdowns, promoted coordinates and eight validated address
pairs. The game-specific five-grid policy is not a vendored copy routine.
Both floor-5 callers supply index 4 in their delay slots. The fifth six-byte
record is therefore owned data, while the following `01 01` remains
unclassified before the independently owned camera path.

Preserve the naming branch's grid identities and shared fields. Import the
30-byte / five-record declaration and size guard without inventing padding,
a sixth record or extra argument validation. Descriptor coordinates and
extents remain literal authored map-cell data; zero loop tests are empty
counts, and 255 is the existing no-copy sentinel. No tuning rationale is
inferred from these values. All five assignments, unsigned widths and
source/destination row advances retain their existing meaning.

Require an independent compile of literal 3b1c834 source to agree with the
named unit in every non-debug section. Compare all seven exact bodies,
ordered calls/data targets, all 30 data bytes and 84 switch destinations
against retail. Check both index-4 call slots and the unclassified neighbor.
The other 111 objects and 483 function scores must remain unchanged from
ebf34e7. Run the focused rebuild, repository checks and full build; preserve
only the incoming map-copy bank row. Main-checkout uncommitted edits are
outside this import.

Final verdict: the reconciled unit agrees with an independent literal
3b1c834 compile in every non-debug section; only its debug line table differs.
All other 111 objects retain every non-debug section from ebf34e7. Exactly
one strict score changes, map-copy 94.256195 -> 100%; the other 483 and every
prior exact are unchanged. Seven unit functions reproduce all 666 retail
words with their ordered calls and numeric data targets. In particular,
map-copy retains 121 words, no calls and eight address pairs. Changing its
descriptor target by one record fails the raw control.

Both source and target retain all 30 descriptor bytes and 84 relocated switch
destinations. Both immediate-index-4 callers and the unclassified `01 01`
neighbor are independently verified. The forward probe remains non-exact
at 93.755104%. The incoming single map-copy bank row is preserved, with no
additional banking.

Ruff and diff checks pass; all 651 tests pass in 83.303 seconds with nine
skips. Full `kf build` still fails on the existing data ownership/placement
findings: 7/60 source-data units match, target relink remains PSX 1/1,
GAME 75/77 and OPEN 34/38, with two GAME and four OPEN section-base conflicts
and zero artifact failures. Post-full-build object, score and raw controls
pass. No tooling implementation or flake changed.

## Player damage label meanings

The [player damage-label campaign](game-player-damage-labels.md) supplies
semantic field names from the retail UI and actual damage data flow. It
preserves all numerical values. Its numeric-limits section distinguishes
known tenths scaling and zero accumulation from unexplained defense seeds,
bonus tuning and the repeated cutting-defense contribution. Naming a damage
type does not recover why the designers selected a particular magnitude.

## Scatter reconstruction and velocity range

Function Match Plan: import committed master f3edc49 into the e2b384c naming
worktree. GAME 800386c4 `effect_scatter_triple` starts at 76.923080%; the
incoming signed RNG result and centered-value accumulation reach strict 100%.
The fresh six-view snapshot agrees with the committed scatter dossier: 104
bytes, one block, 24-byte frame, three proven rand calls, no data or strings,
three unsigned halfword loads/stores, and preserved call/return delay slots.
The two direct callers, shared kind-10 movement, constructor, both neighbors,
SDK RNG header/trampoline and source history establish the relevant context.
The helper is game effect policy; the vendored RNG remains an external call.

Preserve the named floor dimensions and other semantic work in this unit.
Name the private random shift and derive its velocity bias from the authentic
RAND_MAX. The pinned header defines 32767; shifting by eight gives 0..127,
so half the 128-value range is 64. Each component receives an increment in
[-64,63], stored modulo 65536. Kind 10 adds the resulting signed halfwords
directly to world position on each movement update: these are velocity
components in world units per update. They are not angle or Q12 quantities.
The original choice of this spread remains unexplained. The two caller draws
perturb a child copy and the live parent separately; the helper makes three
RNG calls and never touches the fourth halfword.

Require an independent literal f3edc49 compile to agree with the reconciled
named unit in every non-debug section. Only scatter's score may change;
the other 111 objects and 483 scores must remain unchanged from e2b384c.
Verify all seven exact unit bodies against raw retail, their ordered numeric
references, the 35-byte floor initializer and a wrong-RNG-target control.
The other inline helper literals 0/1/2 select x/y/z; the bias expression's
+1 counts the inclusive RNG range and /2 selects its midpoint. Rebuild,
run repository checks and the full build, and preserve only the incoming
scatter bank row. Do not alter a compiler profile or main-checkout edits.

Final verdict: the named unit agrees with a separate literal f3edc49 compile
in every non-debug section; only the debug line table differs. The other
111 objects retain every non-debug section from e2b384c. Scatter improves
76.923080 -> 100%, with all other 483 strict scores and prior exacts unchanged.
Its 26 words and three ordered calls to 8005049c match retail; changing the
RNG target by four bytes fails raw equality. The unit's seven exact bodies
reproduce 519 words, and both floor initializers retain all 35 bytes. The
2D projectile remains non-exact at 99.934210%. The private data-section
reference resolves through the sole owned initializer at 80056268.

Ruff and diff checks pass. All 651 tests pass in 81.623 seconds with nine
skips. Full `kf build` retains the existing ownership/placement failures:
7/60 source-data units exact, target relink PSX 1/1, GAME 75/77 and OPEN
34/38, two GAME/four OPEN section-base conflicts, zero artifact failures.
Post-full-build control comparisons pass. Only the incoming scatter bank
row is preserved; no additional result is banked and no tooling/flake changed.

## Player status constants and units

The [remaining status campaign](game-player-statuses.md) names four masks,
duration/refresh values, penalties, bonuses, movement/turn limits and darkness
fade/fog constants. Its numeric table explains the 32-step Q12 fade and
one-fifth movement limit while retaining unexplained tuning as such. Poison
now shares the player-status -1 timer sentinel; all distinct expiry and cure
paths remain unchanged.


## Configuration-panel reconstruction reconciliation

### Function Match Plan

Integrate committed master `8c1ad49` into the semantic-naming worktree. Its
parent `6c1fcae` reconstructs GAME `8002589c / 0x504` and
`80025da0 / 0x198` under `probe-gcc257-o2-g0`; the later OPEN commit only
records traversal evidence. Starting strict scores here are 30.713396% and
100%. Refresh all six semantic views, raw calls/delay slots, neighbors and
source history before reconciliation. The game-specific menu/window/input
chain is not a vendored body; SDK calls remain intact.

Keep the two complete 24-byte by-value labels, their halfword alignment,
the separate state pointer, corrected input/exit CFG and all seven reviewed
internal jump targets. Preserve this worktree's `hud_gauges_enabled`,
`compass_enabled`, `MENU_TEXT_END` and SDK button names when taking the new
body. The five retail sound calls pass 0/0/1/1/2: navigation, navigation,
confirmation, confirmation, cancellation. Apply the existing sound names
to those actual arguments rather than retaining the old reconstruction's
five zeros. The preceding audio table's assertion was based on that old
source and is corrected above.

The panel has 19 direct calls and nine ordered data addresses. Compare the
reconciled source with an independent incoming-source compile allowing only
the two established field-name substitutions for header compatibility.
Require the helper's complete 102 words to remain retail-exact, and the
panel's only retained differences to be the incoming nine register-role
words. Compare all other objects and function scores, preserve the imported
helper bank row, run full build, lint, tests and whitespace checks before
committing. No new exact result or compiler mechanism is claimed.

### Reconciliation verdict

The named source compiles identically in every object section to the
independent incoming-source control. The panel retains 321 words, 19 calls,
nine ordered data addresses and precisely the incoming nine differing word
offsets (`034/b0/12c/1c0/230/2c4/3b4/3e8/44c`). Its strict result improves
from this branch's 30.713396% to the incoming 99.859810%. The helper retains
all 102 exact retail words, nine calls and eleven data addresses. All other
111 objects and 483 strict function scores are unchanged. Only the seven
incoming relocation rows, two function identities and one exact-helper bank
row change; the branch's other curated semantic names are preserved.

Lint, whitespace checks and all 653 tests pass (85.337 seconds, nine skips).
Full `kf build` retains the existing source-data 7/60, SDK/config-data 4/4,
and target-relink PSX 1/1, GAME 75/77 and OPEN 34/38 results. The six section
base conflicts remain, with no artifact failures. This reconciliation also
removes the earlier inaccurate assertion that all five config-menu cues
were deliberately zero; their reviewed argument sequence is 0/0/1/1/2.
