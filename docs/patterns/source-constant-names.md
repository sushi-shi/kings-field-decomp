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
