# OPEN scene control domains and literal accounting

The later [ending-scroll review](ending-scroll-states.md) also types the scroll
enable latch and gives the four cadence ticks semantic names while retaining
the countdown arithmetic. The plan and verification below describe the earlier
transition/lighting/sequence batch.

## Function Match Plan

Starting at `8514c78`, propagate a signed-halfword transition-mode enum
through `opening_entity_transition` and its four calls. Give the independent
ending lighting and audio-sequence locals distinct signed-halfword enums.
Keep the scroll-phase countdown numeric: it is decremented and wrapped around
a four-step cycle, not assigned among independent states. Preserve every
state value, comparison, call, narrowing, delay slot and relocation target.

Retail OPEN `80014620/80014624` sign-extends the transition argument before
dispatch. Mode 0 creates cylinders at zero height and grows them; mode 1
deactivates them; mode 2 creates them tall, shrinks, then deactivates; mode 3
creates them tall and returns. The four proven callers in scene 3 and the
ending supply these modes and stack VECTORs. Do not infer behavior for other
values or change the existing unsupported-mode path.

The ending's lighting state advances black-to-midpoint, midpoint-to-green,
then stops. Its sequence state waits for panel scrolling, counts down 21
pre-decrements from 20 through -1, fades master volume using the original
divide-by-three accumulator, then replaces the sequence. The render cycle
0,3,2,1 remains separate; panels advance on 0 and 2, starfield roll/color on 0.

All seven functions have fresh six-view semantic dossiers, current source,
history and strict-score snapshots. The neighboring camera/path and renderer
contracts were inspected, including the no-offset camera-step argument and
null transform reuse. Authored camera/resource/scene policy establishes game
ownership; SDK audio, GTE, VSync and primitive helpers remain external.
The prior [scene audit](source-constant-names.md#later-opening-scenes-and-ending-sequence)
and [transition snapshot](open-transition-snapshot.md) supply the existing
data, call and relocation evidence; naming does not promote candidate edges.

Reconcile all 595 current literal occurrences with a complete per-occurrence
ledger, including authored initialized data, local matrix coefficients,
layout-offset checks, array components, numeric cycle positions and the
unconsumed intermediate blend counter. Verify each initialized DATA owner
against retail. Unknown padding/control meaning must remain explicit.

The source hypothesis is unchanged runtime output with stronger domain
checking. Require forced module compilation, isolated before/after builds
of all 112 variants, preservation of 484 strict scores and ordered words/
referents, focused compiler controls, whole-tree modern diagnostics, inventory,
Ruff, repository tests, whitespace and full `kf build`. No size assertions
or new permanent tests are added.

| OPEN VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `80014268 / 372` | `opening_scene0_run` | 100% |
| `800143dc / 384` | `opening_scene1_draw_fade` | 100% |
| `8001455c / 172` | `opening_scene1_run` | 100% |
| `80014608 / 508` | `opening_entity_transition` | 99.921260% |
| `80014804 / 816` | `opening_scene3_run` | 99.931370% |
| `80014b34 / 756` | `opening_ending_scene_run` | 100% |
| `80014e28 / 1944` | `opening_ending_scroll_run` | 97.129630% |

## Retained data and literal evidence

The [complete ledger](open-scene-literal-ledger.md) accounts for 428 occurrences
in initialized owners and the position-offset check, plus 167 occurrences
across the seven functions. Each duplicate token has its own reason. The
camera's zero speed in the first row is not a divisor: `begin` copies that
row's transform, then segment computation advances to the next row. Once the
X sentinel is found, the remaining terminal-row coordinates/speed do not
describe a segment. The final record halfword still has no supported purpose.

The FT4 helper consumes rectangle X/Y/width/height and UV bytes 0/2/4/6.
Odd UV bytes are retained without assigning them unsupported meanings.
Array positions and unrolled panel placement multiples remain explicit
ordinals. Scroll positions 0 and 2 select the two panel updates in the four-step
cycle; the one-unit starfield roll is 0.087890625 degrees per selected update.
The intermediate ending-scene blend increment/guard/clamp still has no value
consumer before reset, so 0x40/0xfff remain documented unresolved operations.

All 17 initialized DATA claims, totaling 1,047 bytes, were compared individually
against hash-verified OPEN retail and are exact; none contains a relocation.

| OPEN VA | Data owner | Bytes |
| --- | --- | ---: |
| `800354f4` | `opening_scene0_camera_path` | 476 |
| `800356d0` | `opening_scene3_camera_path` | 84 |
| `80035724` | `opening_ending_camera_path` | 252 |
| `80035820` | `opening_ending_scroll_camera_path` | 84 |
| `80035874` | `opening_scene0_sound` | 3 |
| `80035878` | `opening_scene3_overlay_rects` | 16 |
| `80035888` | `opening_ending_scroll_panels` | 72 |
| `80037284` | `opening_scene3_overlay_uv` | 8 |
| `8003728c` | `opening_scene3_overlay_color` | 4 |
| `80037290` | `opening_ending_scroll_backgrounds` | 16 |
| `800372a0` | `opening_ending_scroll_top_start` | 4 |
| `800372a4` | `opening_ending_scroll_bottom_start` | 4 |
| `800372a8` | `opening_ending_scroll_top_end` | 4 |
| `800372ac` | `opening_ending_scroll_bottom_end` | 4 |
| `800372b0` | `opening_ending_scroll_panel_color` | 4 |
| `800372b4` | `opening_ending_scroll_background_color` | 4 |
| `800372b8` | `opening_ending_scroll_uv` | 8 |

## Final domain and matching verification

`KfOpeningTransitionMode` now types the shared signature and all calls.
`KfEndingLightingPhase` and `KfEndingSequencePhase` type the two private
state locals and their switch/assignment sites. All retain signed-halfword
representation in retail C. No state value, authored literal or operation is
removed. The remaining 595 literal occurrences all have ledger reasons.

All seven functions retain their baseline scores: four exact, three partial.
Their 1,238 words, 116 calls and 90 address materializations are unchanged;
the four exact consumers' 421 words also equal retail and target-object words.
All 112 isolated before/after runtime objects, alignments, symbols and ordered
relocations agree, as do the live comparison objects and isolated after objects.
Only the scene object's debug line table changes. All 484 strict scores are
unchanged; no new result is banked.

The first partial divergences remain OPEN `80014608` (stack allocation 0x30
versus retail 0x38), `80014804` (0x60 versus 0x70) and `800150c0` (reconstructed
`sh zero,0xb0(sp)` versus retail `lui a0,0x8003`). These preserved residues do
not establish compiler attribution.

The actual shared header and the two private enum definitions extracted from
the current source pass a positive Clang C++20/MIPS control. Ten negative
controls reject raw/foreign transition arguments and locals, raw/foreign
lighting/sequence initialization, and invalid comparisons. The controls
suppress only the existing offset-check extension warning. Whole-tree modern
checking preserves the same 300 errors and 65/112 passing variants.

Inventory, Ruff and whitespace checks pass. An initial repository test run
reported two GAME config-panel subtest mismatches; both passed in isolation,
and the complete rerun passed all 683 tests (102.395 seconds). No test or GAME
source change was made for this campaign, and the cause of that initial
failure was not established.

Full `kf build` retains the existing data/relink failures: source data PSX
0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1, GAME 75/77, OPEN 34/38.
Six section-base conflicts remain, with no artifact failures. This is not a
claim that the full build or whole-tree modern compilation is green.

The full-source census remains 6,786 retained occurrences. Exact current
token/expression accounting now covers 30 files / 3,086 occurrences, including
all 595 in this module. The ten source lines containing unresolved `unknown_`
fields are unchanged; their absence of semantic evidence is not resolved by
enum checking or literal documentation.
