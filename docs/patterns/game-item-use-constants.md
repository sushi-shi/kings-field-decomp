# GAME item-use range and effect settings

## Function Match Plan

Baseline `9c95ad4ee55e1cdec5eb0843d957eb314b9a768d`; image **GAME.EXE**, pinned `probe-gcc257-o2-g0`.

The six semantic views, source history, adjacent functions and consumers were captured before edits. Only `player_use_item` receives expression changes. Three existing map-interaction definitions move to the shared map header; the other functions in both units serve as controls. Preserve byte item IDs, signed query sentinels, key fallthrough, all six harp argument positions, timer/stock ordering, actor-before-event selection, source claims, direct call targets and ordered data references. Branch and return delay slots and the existing unresolved-switch reachability classification remain unchanged.

These are game item/floor consumers of external SDK sine/cosine and rendering APIs; none of the campaign functions is attributed to a vendored library. No SDK body is reconstructed. Calls retain proven classification and address/table references retain validated classification.

| Function | GAME VA / bytes | Strict baseline | Retail calls / conditional branches / returns | Hypothesis |
| --- | --- | ---: | --- | --- |
| `actor_show_info_image` | `0x80017edc / 200` | 100.000000% | 3 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `map_event_show_person_image` | `0x80017fa4 / 176` | 100.000000% | 3 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `player_use_item` | `0x80018054 / 1116` | 100.000000% | 20 / 20 / 1 | Name shared reach/padding, door tolerance, unlock volume, mirror limits, staff reload and harp recipe arguments. |
| `actor_pool_find_at_tile` | `0x80033ee4 / 128` | 100.000000% | 0 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `map_ambient_script_floor1` | `0x80033f64 / 648` | 100.000000% | 5 / 24 / 1 | Unchanged function control for the rebuilt unit. |
| `map_ambient_script_floor2` | `0x800341ec / 112` | 100.000000% | 2 / 3 / 1 | Unchanged function control for the rebuilt unit. |
| `map_ambient_script_floor3` | `0x8003425c / 136` | 100.000000% | 2 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `map_ambient_script_floor4` | `0x800342e4 / 8` | 100.000000% | 0 / 0 / 1 | Unchanged function control for the rebuilt unit. |
| `map_ambient_script_floor5` | `0x800342ec / 244` | 100.000000% | 5 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `map_action_script_floor1` | `0x800343e0 / 88` | 100.000000% | 2 / 2 / 1 | Unchanged function control for the rebuilt unit. |
| `map_reveal_fade` | `0x80034438 / 388` | 100.000000% | 8 / 3 / 1 | Unchanged function control for the rebuilt unit. |
| `map_action_script_floor2` | `0x800345bc / 84` | 100.000000% | 1 / 2 / 1 | Unchanged function control for the rebuilt unit. |
| `map_action_script_floor3` | `0x80034610 / 144` | 100.000000% | 2 / 4 / 1 | Unchanged function control for the rebuilt unit. |
| `map_action_script_floor4` | `0x800346a0 / 8` | 100.000000% | 0 / 0 / 1 | Unchanged function control for the rebuilt unit. |
| `map_floor5_transition_cutscene` | `0x800346a8 / 908` | 100.000000% | 12 / 15 / 1 | Unchanged function control for the rebuilt unit. |
| `map_action_script_floor5` | `0x80034a34 / 76` | 100.000000% | 1 / 1 / 1 | Unchanged function control for the rebuilt unit. |
| `map_event_interact` | `0x80034a80 / 724` | 100.000000% | 7 / 17 / 1 | Unchanged function control for the rebuilt unit. |
| `map_show_screen_image` | `0x80034d54 / 144` | 88.888885% | 1 / 3 / 1 | Unchanged function control for the rebuilt unit. |
| `map_interaction_dispatch` | `0x80034de4 / 2308` | 83.436745% | 54 / 58 / 1 | Use the same shared interaction definitions; preserve the existing partial body. |

Require forced affected builds, isolated before/after compilation of every
source/image variant, allocated bytes/alignment/symbol/ordered-relocation equality,
strict score preservation, exact retail word checks, modern compiler diagnostics,
inventory, Ruff, existing tests, whitespace checks and a full build.

## Interaction and targeting evidence

The item-use sine/cosine chains at `80018098..800180ac` and
`800180c4..800180e0` multiply by 1000 and shift signed Q12 results. Ordinary
interaction uses the same projection, followed by the same object query.
Calls at `80018134` and `8001825c` pass radius padding 800 in `$a3` in their
delay slots. The query adds this to the model definition's interaction radius;
it is not a standalone maximum distance. Share the existing map constants.

The model-89 facing call at `800181e0` receives 341 in `$a2`, while ordinary
door interaction uses the same tolerance at `800353b0..80035420`.
`angle_within_tolerance` wraps the delta to 12 bits and accepts both sides of
the inclusive tolerance. Define it as `KF_ANGLE_FULL_TURN / 12`, preserving
integer truncation: 341/4096 turn, or 29.970703125 degrees.

Mirror queries at `80018408` and `80018434` receive distance 6000 and angular
tolerance 341; name the mirror's policy separately from door interaction.
Actor and event consumers fold angular error into a half-turn, reject targets
outside the supplied radius/tolerance, and favor smaller angular error.
The caller prefers an actor and only falls back to an event after an actor miss.
6000 world units equal three map tiles, but the original tuning rationale is
unproven. The unlock cue receives volume 110 at `80018210`; name its mix setting
without assigning a new sound-resource identity.

## Harp recipe and staff timer evidence

At `80018348..8001838c`, the harp passes `(0,4,150,800,43,70)` on floor 2
and `(4,1,150,800,88,270)` on floor 3. The constructor at `80037770` preserves
its `u16,u16,u16,u16,s32,s32` boundary: the last two arguments are caller-stack
words and the first four are register arguments. Its stores and the deformation
dispatch arm establish these meanings:

| Setting | Floor 2 | Floor 3 | Meaning |
| --- | ---: | ---: | --- |
| First segment / count | 0 / 4 | 4 / 1 | Consecutive rows in the five-row deformation table. |
| Progress per update | 150 | 150 | Q12 interpolation progress per sweep update. |
| Cell stagger | 800 | 800 | Q12 progress difference between consecutive cells. |
| Sweep updates | 43 | 88 | Calls that update the selected lines before a separate transition call. |
| Hold countdown | 70 | 270 | Initial counter; predecrement to -1 gives 71 / 271 hold-state calls. |

`effect_floor_deform_line` clamps each cell's progress to 0..4096, interpolates
height and advances the authored column/row steps. The table has cell counts
2,2,3,3 for floor 2 and 12 for floor 3. This establishes units and ownership;
original timing margins remain authored settings. Floor and object identifier domains remain pending typed propagation. Preserve the one-active-deformation scan, unchanged stock behavior
and acknowledgement even when allocation fails.

Staff use stores 1000 at player+0x52 (`800183c4..800183cc`) before the stock
check. The signed-halfword consumer at `8001a24c..8001a270` tests the old value
against -1, decrements and then calls lighting. Thus an uninterrupted reload
produces 1001 qualifying calls, including the final zero-to-minus-one update.
Name this `PLAYER_ILLUSION_STAFF_TIMER_RELOAD`, not a duration in frames.
The renderer separately tests this same field for inactivity and uses its low
two bits for the existing two-on/two-off mesh selection.

The [special-item review](game-item-special-identities.md) supplies earlier
item/resource identities and consumer evidence. The [current retained ledger](game-item-use-literal-ledger.md)
will be refreshed after the naming changes. Model IDs without established mesh
identities retain their documented unresolved status.

## Verification

All 112 source/image variants were compiled independently before and after.
Allocated bytes, alignment, runtime symbols and ordered relocations are identical;
only debug line sections changed in the two edited C modules. All 484 strict
function scores remain unchanged. The 19 function controls preserve
1,909 candidate instruction words, 127 ordered direct calls and
144 address materializations. The 17 exact functions retain all
1,297 linked retail instruction words, including delay slots.

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `actor_show_info_image` | 100.000000% | Exact, unchanged |
| `map_event_show_person_image` | 100.000000% | Exact, unchanged |
| `player_use_item` | 100.000000% | Exact, unchanged |
| `actor_pool_find_at_tile` | 100.000000% | Exact, unchanged |
| `map_ambient_script_floor1` | 100.000000% | Exact, unchanged |
| `map_ambient_script_floor2` | 100.000000% | Exact, unchanged |
| `map_ambient_script_floor3` | 100.000000% | Exact, unchanged |
| `map_ambient_script_floor4` | 100.000000% | Exact, unchanged |
| `map_ambient_script_floor5` | 100.000000% | Exact, unchanged |
| `map_action_script_floor1` | 100.000000% | Exact, unchanged |
| `map_reveal_fade` | 100.000000% | Exact, unchanged |
| `map_action_script_floor2` | 100.000000% | Exact, unchanged |
| `map_action_script_floor3` | 100.000000% | Exact, unchanged |
| `map_action_script_floor4` | 100.000000% | Exact, unchanged |
| `map_floor5_transition_cutscene` | 100.000000% | Exact, unchanged |
| `map_action_script_floor5` | 100.000000% | Exact, unchanged |
| `map_event_interact` | 100.000000% | Exact, unchanged |
| `map_show_screen_image` | 88.888885% | Partial, unchanged |
| `map_interaction_dispatch` | 83.436745% | Partial, unchanged |

Modern compiler diagnostics remain the same 300 errors (65/112 variants pass).
Inventory, Ruff and the existing repository tests pass. The full build still
reports pre-existing source-data/relink mismatches. No new tests or size checks
were added, and no new exact result is banked.

The suite ran 680 tests in 87.157 seconds. Complete token/expression accounting
passes for all 26 previously verified files, now 2,381 retained occurrences;
the source-wide census is 6,880. The partial screen-image helper first differs
at `80034d84` (`mfhi a2` versus retail `mfhi v1`). The interaction dispatcher
first differs at its entry frame allocation (80 versus retail 72 bytes).
Both residues predate this change and remain unattributed.
