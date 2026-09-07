# Floor-script motion, fades and interaction constants

## Function Match Plan

Baseline `b99ed93`, hash-verified GAME retail, unchanged `probe-gcc257-o2-g0`.
The complete sixteen-function floor-script module has refreshed address/extent,
disassembly/CFG, caller/callee, string, history and strict-match snapshots.
Camera-path, radius-query, color/matrix interpolation and effect-construction
consumers constrain units. The [original literal audit](game-map-script-literals.md)
records the complete source and earlier slot/state recovery. Authored floor
policies, dialogue and interactions establish game ownership; SDK audio, RNG,
trigonometry and GTE providers remain external calls.

Name ambient volumes/chance and search padding, the boss-reveal yaw interval,
passage sound volume, reveal fade/light/motion steps, weapon-transform heights
and angular acceleration/limit, interaction probe distances, container camera
pitch interval/step and hinge step, and door-partner search padding. Preserve
all signedness, narrowing, comparison endpoints, control flow, optional effect
arguments, ordered calls, referents and delay slots. Keep authored coordinate
and resource tuples explicit. The container's repeated first-item precheck,
uninitialized blast direction and partial dispatcher remain unchanged.

Hypothesis: names and constant expressions preserve all emitted runtime.
Freeze all 112 objects/source sets and 484 scores, force the affected compile,
compare isolated before/after runtime sections, symbols and ordered relocations,
resolve every reviewed body's words/references, and recheck exact functions
against retail and targets. Refresh the full numeric/character ledger, including
initializers. Run modern checking, inventory, Ruff, existing tests, whitespace
checks and full `kf build`; add no tests or size assertions. Each function
retains a separate final verdict. No new exact function is expected or banked.

| GAME VA / bytes | Function | Initial strict % |
| --- | --- | ---: |
| 0x80033ee4 / 128 | `actor_pool_find_at_tile` | 100.000000 |
| 0x80033f64 / 648 | `map_ambient_script_floor1` | 100.000000 |
| 0x800341ec / 112 | `map_ambient_script_floor2` | 100.000000 |
| 0x8003425c / 136 | `map_ambient_script_floor3` | 100.000000 |
| 0x800342e4 / 8 | `map_ambient_script_floor4` | 100.000000 |
| 0x800342ec / 244 | `map_ambient_script_floor5` | 100.000000 |
| 0x800343e0 / 88 | `map_action_script_floor1` | 100.000000 |
| 0x80034438 / 388 | `map_reveal_fade` | 100.000000 |
| 0x800345bc / 84 | `map_action_script_floor2` | 100.000000 |
| 0x80034610 / 144 | `map_action_script_floor3` | 100.000000 |
| 0x800346a0 / 8 | `map_action_script_floor4` | 100.000000 |
| 0x800346a8 / 908 | `map_floor5_transition_cutscene` | 100.000000 |
| 0x80034a34 / 76 | `map_action_script_floor5` | 100.000000 |
| 0x80034a80 / 724 | `map_event_interact` | 100.000000 |
| 0x80034d54 / 144 | `map_show_screen_image` | 88.888885 |
| 0x80034de4 / 2308 | `map_interaction_dispatch` | 83.436745 |

## Named roles and exact boundaries

Names describe observed consumers and units; the original reasons for tuning
the magnitudes remain unknown.

| Family | Values and preserved meaning |
| --- | --- |
| Ambient/passage mixing | Floor-1 revival ambience uses volume 115; passage opening uses 100, both in the audio API's 0..127 scale. The resource references remain unchanged. |
| Script object search | Both floor-1 state transitions add 3000 world units to object radius at their distinct authored search points. |
| Floor-2 ambience | Raw RNG values below 4000 pass, after the existing event-slot/stage/page checks. This admits 4000 of 32768 outputs; no trial-independence claim is made. |
| Boss reveal heading | Unsigned yaw lies in [1808,2289), or half-turn ±240 inclusive. Retail subtracts 1808 and tests the resulting 481-value interval. The tile window and two image paths stay authored data. |
| White fade | Q12 blend steps by 128 through both endpoints: 33 rendered samples. Only samples strictly above one-quarter blend move the event. Light interpolation shifts the earlier blend by 2 to cover its complete transition during that quarter. |
| Reveal motion | The remaining 24 fade samples subtract 130 world units from event Y and add 128 angle units to yaw: 3120 units upward and 270 degrees in total. The two 128-valued constants have distinct color and rotation roles. |
| Return fade | Q12 blend steps down by 256 through zero: 17 rendered samples. Saved lighting is restored afterward. |
| Weapon transform | The object begins 1300 world units above its local floor; the radial blast originates another 600 units above it. Yaw speed rises by one angle unit per rendered update to 240 (21.09375 degrees/update), then falls by the same magnitude. Current speed is added before changing it. Existing hold/swap counters retain their ordering. |
| Interaction probes | Cell-attribute sampling uses 1500 world units forward; event/object sampling uses 1000. Both retain their Q12 trigonometric conversion and the separate existing radius padding. |
| Container camera | Unsigned `(pitch - 191)` below 1858 represents the inclusive pitch interval 191..2048. The span is now explicitly `half turn - minimum + 1`. Outside this interval, pitch increases by 16. Starting at zero reaches 192 after twelve increments; the original pitch is restored afterward. |
| Container hinge | Pitch decreases by 32 angle units per rendered update. From closed zero, 32 iterations reach -1024 (minus 90 degrees); the existing signed termination test stays intact. |
| Paired door | The neighboring-leaf query adds 6000 world units to the candidate object's radius. This is separate from the user's forward interaction probe. |

Twenty-two named definitions replace **26 inline numeric occurrences**.
The [complete ledger](game-map-script-literal-ledger.md) accounts for the
remaining **246 numeric/character occurrences**, down from 272: 33 initializer
tokens and 213 function tokens. Repeated tokens each retain a row and a
consumer-specific reason. Camera/keyframe tuples, coordinate windows,
dialogue/resource ordinals and path digits remain explicit data. Arithmetic
endpoints, boolean values, byte packing and nulls retain their local meanings.
The existing enum and dialogue-format macro definitions are excluded from the
census and remain explained by the earlier protocol audit.

The camera interval interpretation was independently checked over all 65536
halfword values. The four initialized data claims—camera path, sound position,
light matrix and mutable image path—retain all 120 bytes against retail and
target symbols. Symbol-relative ranges avoid assuming that a data section's
first byte is every claim's origin.

## Verification and final verdicts

All 112 independently compiled before/after objects retain their runtime
sections, alignment, symbols and ordered relocations. Live objects agree
with those controls; only the floor-script debug-line section changes.
All 484 strict function scores remain unchanged.

The sixteen reviewed functions preserve 1536 instruction words,
101 direct call sites and 120 ordered data-address pairs.
The fourteen exact functions reproduce all 924 words against retail
and target objects. Each function retains its initial strict score from
the plan above. No additional function is banked.

| Function | Words / calls / addresses | Final verdict |
| --- | --- | --- |
| `actor_pool_find_at_tile` | 32 / 0 / 1 | Exact, unchanged |
| `map_ambient_script_floor1` | 162 / 5 / 19 | Exact, unchanged |
| `map_ambient_script_floor2` | 28 / 2 / 3 | Exact, unchanged |
| `map_ambient_script_floor3` | 34 / 2 / 5 | Exact, unchanged |
| `map_ambient_script_floor4` | 2 / 0 / 0 | Exact, unchanged |
| `map_ambient_script_floor5` | 61 / 5 / 11 | Exact, unchanged |
| `map_action_script_floor1` | 22 / 2 / 3 | Exact, unchanged |
| `map_reveal_fade` | 97 / 8 / 9 | Exact, unchanged |
| `map_action_script_floor2` | 21 / 1 / 2 | Exact, unchanged |
| `map_action_script_floor3` | 36 / 2 / 4 | Exact, unchanged |
| `map_action_script_floor4` | 2 / 0 / 0 | Exact, unchanged |
| `map_floor5_transition_cutscene` | 227 / 12 / 14 | Exact, unchanged |
| `map_action_script_floor5` | 19 / 1 / 2 | Exact, unchanged |
| `map_event_interact` | 181 / 7 / 28 | Exact, unchanged |
| `map_show_screen_image` | 37 / 1 / 5 | Partial, unchanged |
| `map_interaction_dispatch` | 575 / 53 / 14 | Partial, unchanged |

The image-path helper first differs at GAME `80034d84`: source `mfhi a2`,
retail `mfhi v1`. The dispatcher differs at entry `80034de4`: source
`addiu sp,sp,-80`, retail `addiu sp,sp,-72`. These unchanged observations
do not establish an optimizer or register-allocation cause.

Inventory, Ruff and whitespace checks pass. All **680 existing tests pass**
in 86.334 seconds. Modern checking retains the same **300 error diagnostics**
and **65/112** passing source/image variants. Full `kf build` ran and retains
existing data/relink failures: source-data matches PSX 0/1, GAME 9/42,
OPEN 2/19; target relinks PSX 1/1, GAME 75/77, OPEN 34/38, with six
conflicting section bases and zero artifact failures. No tooling or flake
files changed. The ten unresolved `unknown_` source occurrences and broader
constant audit remain open.
