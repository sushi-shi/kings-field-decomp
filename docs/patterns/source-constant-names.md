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
