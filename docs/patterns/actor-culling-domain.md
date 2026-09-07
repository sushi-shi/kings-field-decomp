# Actor culling mode and placement stream state

## Function Match Plan

Recover the meaning of live actor byte +2 from its placement writer, dynamic
spawn writer and renderer consumer. Rename `variant` to `culling_mode` and use
a byte enum distinguishing visibility-grid culling from the nearby square.
Rename placement flag 0x20 and the renderer's square dimensions accordingly.
Preserve the renderer's exact-zero test: every nonzero byte selects the square,
including explicitly decoded values other than the normal one.

Give the placement loader's separate signed-word stream state its own local
enum. Preserve its exact-one exhaustion test, the shared free-slot tail, and
the fact that neither the terminator slot nor subsequent slots advance the
placement pointer. Lifecycle, slot policy, culling and stream exhaustion are
distinct domains. No enum casts are needed for ordinary assignments.

Capture each function's image-qualified address, disassembly/CFG, callers,
callees, strings, history and match state before editing. Review adjacent pool
functions and preserve all existing code/data ownership. These are game pool
and rendering policies; the SDK/GTE and libc callees remain vendor boundaries.
Correct the earlier asset-variant description and the inventory's statement
that every actor uses the visibility grid without promoting confidence levels.

Force the three affected units, compare words and ordered references, and keep
every banked function exact. For the existing partial matches, require unchanged
runtime objects and record the first retail divergence without assigning a
compiler cause. Compile all 112 variants in isolation to check the shared type
change, exercise Clang acceptance/rejection controls, reconcile the actor and
renderer ledgers and add complete pool literal coverage. Run modern checking,
inventory, Ruff, repository tests, whitespace and full build before commit.

The initial full comparison encountered concurrent merge conflicts in unrelated
OPEN sources and a concurrent GAME map-script edit. The actor units were
force-built successfully, and a fresh objdiff report supplied their baselines.
Only these freshly rebuilt units are used for starting match claims. The
isolated control uses committed source at the recorded HEAD, which agrees with
the selected sources and header; conflicted working snapshots are not compiled
as a purported clean baseline. Full checks must follow merge resolution.

## Direct field and control-flow evidence

GAME `80030954` masks placement definition byte +1 with 0x20. Its branch stores
one or zero into live actor +2 at `80030964` / `80030968`. The low five bits
independently select the definition. Dynamic spawn stores zero to live actor
+2 at `8002cf3c` before position, rotation, initialization and action helpers.

The renderer uses s0 = actor +22. Its `lbu -20(s0)` at `8001f34c` therefore
loads actor +2; `bnez` at `8001f354` chooses the square path. Zero checks the
active cell window's dimensions and row-major visibility byte. Nonzero adds
twelve to each actor cell coordinate, subtracts the view cell, narrows to u16,
and compares below twenty-four. This is a 24-by-24-cell square, with ordinary
nearby differences from -12 through +11 inclusive, not an asset selector or
a radial-distance test. Preserve the modulo-65536 arithmetic at wrap boundaries.
Both paths still require active lifecycle and eventually call `render_actor`.

The loader initializes s5 to zero at `800308ec`, compares it to the exact one
in s7 at `8003090c`, and sets it to one in the jump delay slot at `80030a1c`
after seeing slot-state 0xff. It then marks that actor and all remaining actors
free/dormant. The output always walks 128 actors at stride 0x48; only consumed
placements advance by 0x10. Existing fields in freed slots are otherwise retained.

## Starting per-function snapshots

| Image | Function | VA / bytes | Starting strict score | Constraint |
| --- | --- | --- | ---: | --- |
| GAME.EXE | `render_entities` | `0x8001f218 / 1408` | 97.380684% | Byte +2 selects visibility-grid versus square culling; all other pool passes remain unchanged. |
| GAME.EXE | `actor_pool_find_free` | `0x8002ca78 / 60` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `func_8002cab4` | `0x8002cab4 / 32` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_set_player_transform` | `0x8002cad4 / 112` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_update_cell_from_position` | `0x8002cb44 / 116` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_set_position` | `0x8002cbb8 / 156` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_set_rotation` | `0x8002cc54 / 16` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_initialize` | `0x8002cc64 / 196` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_initialize_current` | `0x8002cd28 / 164` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_initialize_slot` | `0x8002cdcc / 188` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_clear` | `0x8002ce88 / 64` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_set_action` | `0x8002cec8 / 12` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_spawn` | `0x8002ced4 / 176` | 100.000000% | Default culling selects the grid; free-slot search and four helper calls unchanged. |
| GAME.EXE | `actor_pool_begin_death_by_definition` | `0x8002cf84 / 244` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `combat_calculate_damage_component` | `0x8002d078 / 168` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_apply_damage` | `0x8002d120 / 904` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_apply_radial_damage` | `0x8002d4a8 / 504` | 90.087300% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_try_attack_player` | `0x8002d6a0 / 344` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_find_target_in_cone` | `0x8002d7f8 / 388` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_distance_to_point` | `0x8002d97c / 240` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_find_overlap` | `0x8002da6c / 324` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_bind_current` | `0x8002dbb0 / 140` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_advance_animation_wrapped` | `0x8002dc3c / 52` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_advance_animation_clamped` | `0x8002dc70 / 92` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_animation_crossed_phase` | `0x8002dccc / 48` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_play_sound_at_phase` | `0x8002dcfc / 152` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_try_select_action_distance_facing` | `0x8002dd94 / 288` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_try_select_ground_action` | `0x8002deb4 / 356` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_try_select_facing_action` | `0x8002e018 / 216` | 100.000000% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_try_select_profiled_action` | `0x8002e0f0 / 504` | 96.333336% | Unedited actor-core control; preserve its signature, field widths, calls, data references and current match. |
| GAME.EXE | `actor_pool_update` | `0x80030818 / 168` | 100.000000% | Bind every actor; run awareness for occupied slots and action only for active lifecycle; unbind with null. |
| GAME.EXE | `actor_pool_load_placements` | `0x800308c0 / 428` | 100.000000% | Decode bit 0x20 into culling enum; separate exact-one stream exhaustion. |
| GAME.EXE | `actor_definitions_load` | `0x80030a6c / 44` | 100.000000% | Copy 0x1c8 words, preserving twelve definitions and the word-loop termination. |

## Implemented domains and retained literals

`KfActorCullingMode` gives live actor byte +2 its observed meaning. The field is
`culling_mode`; placement bit 0x20 is `KF_ACTOR_PLACEMENT_NEAR_SQUARE_CULLING`.
Its writers use visibility-grid or near-square members, and the renderer compares
against the grid member before retaining the original nonzero fallback. The
square width names now describe culling geometry. This naming does not infer
why particular authored actors choose the flag or what the original symbol was.

The local `KfActorPlacementStreamState` separately distinguishes reading from
exhaustion using signed-word storage. The exact-one predicate is preserved;
explicitly decoded two is not exhausted. Culling uses byte storage, so decoded
two takes the nonzero square path and decoded minus one encodes as 255. Ordinary
source assignments need no casts. Lifecycle, slot policy and stream state cannot
be assigned to the culling field in modern checking.

Seven literal uses become enum members: four culling assignments/comparisons
and three stream-state uses. The [complete pool ledger](actor-pool-literal-ledger.md)
accounts for its six remaining pointer/countdown occurrences. The actor-core
ledger now has 171 occurrences and the scene renderer fourteen. Their removed
literal rows are replaced by the typed domain, and the earlier claim that the
spawn field selected an asset is corrected. The structure inventory agrees on
the field's offset, byte storage, name and type, and documents the placement bit.
Existing confidence levels remain unchanged.

## Compiler and object controls

The actual Clang command accepts valid field assignments, reads and comparisons
plus the source's exact local stream enum. Fourteen negative controls reject raw
field values/comparisons, lifecycle and slot members used as culling, culling
used as lifecycle, stream/culling mixing, raw stream initialization/assignment/
comparison, implicit integer encoding, and the obsolete `variant` field spelling.
These controls are ignored build artifacts; no production tests or size assertions
were added. The structure-inventory check caught its stale field row, which was
then reconciled before the final test run.

All 112 isolated before/after runtime sections, symbols and ordered relocations
agree. Only the pool unit's debug line information changes. The clean control
uses committed source at `49cd63e`; selected source/header inputs agree with the
captured working baseline. At full verification, five live objects differ from
that isolated tree because of concurrent GAME map-script and OPEN scene/TMD/
unlit/entity changes. The remaining 107 objects agree, and every selected actor
object agrees with both its pre-edit object and isolated control. Merge
`8bc5c3b` integrates the OPEN changes. No concurrent input is claimed as an
actor-culling improvement.

## Per-function final verdicts

All 33 selected scores remain unchanged. The thirty exact functions agree with
raw retail and delinked targets. The three partial functions retain identical
candidate words, calls and ordered address references; their first divergences
below remain unattributed, with no compiler mechanism inferred from registers.

| Image | Function | VA / bytes | Final verdict |
| --- | --- | --- | --- |
| GAME.EXE | `render_entities` | `0x8001f218 / 1408` | 97.380684%; unchanged. First divergence `0x8001f274`: candidate `move $s5, $v1`, retail `move $s6, $v1`. |
| GAME.EXE | `actor_pool_find_free` | `0x8002ca78 / 60` | 100%; exact words and references unchanged. |
| GAME.EXE | `func_8002cab4` | `0x8002cab4 / 32` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_set_player_transform` | `0x8002cad4 / 112` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_update_cell_from_position` | `0x8002cb44 / 116` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_set_position` | `0x8002cbb8 / 156` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_set_rotation` | `0x8002cc54 / 16` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_initialize` | `0x8002cc64 / 196` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_initialize_current` | `0x8002cd28 / 164` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_initialize_slot` | `0x8002cdcc / 188` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_clear` | `0x8002ce88 / 64` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_set_action` | `0x8002cec8 / 12` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_spawn` | `0x8002ced4 / 176` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_begin_death_by_definition` | `0x8002cf84 / 244` | 100%; exact words and references unchanged. |
| GAME.EXE | `combat_calculate_damage_component` | `0x8002d078 / 168` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_apply_damage` | `0x8002d120 / 904` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_apply_radial_damage` | `0x8002d4a8 / 504` | 90.087300%; unchanged. First divergence `0x8002d4d4`: candidate `move $s4, $a0`, retail `andi $s6, $a2, 0xffff`. |
| GAME.EXE | `actor_try_attack_player` | `0x8002d6a0 / 344` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_find_target_in_cone` | `0x8002d7f8 / 388` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_distance_to_point` | `0x8002d97c / 240` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_find_overlap` | `0x8002da6c / 324` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_bind_current` | `0x8002dbb0 / 140` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_advance_animation_wrapped` | `0x8002dc3c / 52` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_advance_animation_clamped` | `0x8002dc70 / 92` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_animation_crossed_phase` | `0x8002dccc / 48` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_play_sound_at_phase` | `0x8002dcfc / 152` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_try_select_action_distance_facing` | `0x8002dd94 / 288` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_try_select_ground_action` | `0x8002deb4 / 356` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_try_select_facing_action` | `0x8002e018 / 216` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_try_select_profiled_action` | `0x8002e0f0 / 504` | 96.333336%; unchanged. First divergence `0x8002e108`: candidate `andi $s3, $a2, 0x1f`, retail `andi $s2, $a2, 0x1f`. |
| GAME.EXE | `actor_pool_update` | `0x80030818 / 168` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_pool_load_placements` | `0x800308c0 / 428` | 100%; exact words and references unchanged. |
| GAME.EXE | `actor_definitions_load` | `0x80030a6c / 44` | 100%; exact words and references unchanged. |

## Verification and coverage

The selected objects retain 2,079 candidate instruction words, 69 direct calls
and 76 address materializations; 1,472 words belong to the thirty exact
functions. Delay slots and ordered targets are included. No function was newly
banked. Outside this campaign, the fresh report changes GAME interaction dispatch
from 94.840550% to 95.585785% and OPEN entity/item traversal from 98.885544% to
100%; the other 482 scores are unchanged from the captured report.

Inventory, Ruff, whitespace and all 684 repository tests pass (109.237 seconds).
Modern checking retains the same 300 errors and 65/112 passing variants. Full
`kf build` reports source-data matches PSX 0/1, GAME 9/42 and OPEN 3/19, and
target-relink matches PSX 1/1, GAME 75/77 and OPEN 34/38, with six conflicting
section bases and zero artifact failures. OPEN's extra data match belongs to
the concurrent merge; the existing full-build gaps remain open.

The refreshed ledger map verifies 74 files and 4,426 retained occurrences.
The global census is 6,515: this actor pass removes seven inline uses, while
the concurrent OPEN TMD/unlit changes add thirteen. These counts describe literal
coverage, not the number still deserving names. Source unknowns remain ten
lines containing fourteen identifier tokens; no unsupported meaning is invented.
