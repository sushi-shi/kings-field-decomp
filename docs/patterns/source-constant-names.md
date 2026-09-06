# Source constant naming

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
| GAME/OPEN `audio_key_off_mask` | Indices 0/2, shift 8 | Preserve the actual byte selection passed to the SDK's two voice-mask arguments, including the unused middle byte. The shift positions the selected byte rather than defining a gameplay option. |

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

The queue's 122 remaining inline literals are retained for these reasons:

| Sites | Values | Reason |
| --- | --- | --- |
| Static layout assertions | Sizes 0x16/6, offsets 0x10/0/1/2/3/4 | Independent retail byte-layout controls; deriving these expected values from the fields being checked would weaken the checks. |
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
