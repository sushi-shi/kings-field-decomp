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
