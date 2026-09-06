# GAME map dialogue and animation state

## Function Match Plan

This campaign corrects the map-event dialogue and animation model on top of
`2489c9a`. It preserves instruction behavior, resource bytes and saved-state
encoding. GAME.EXE is the image for every address below. The pinned profile for
these units is `probe-gcc257-o2-g0`; this is not historical compiler attribution.

Before editing, the six semantic views, source history and surrounding functions
were captured under `build/constant-names/map-dialogue-state/`. Retail files were
hash-verified with `kf init`. The functions consume game-owned map records,
progress, TALK paths, item exchanges and rendering policy; none is a vendored
body. SDK calls remain SDK calls. Supplemental inspection of `item_menu_root`
at `0x800212d8` confirms that menu mode 2 enters the buy/sell interface.

| VA | Bytes | Function before rename | Initial strict % | Evidence / intended change | Final strict % / verdict |
| --- | ---: | --- | ---: | --- | --- |
| 80017fa4 | 176 | map_event_show_person_image | 100 | supplemental snapshot: byte character ID forms PRSN/PERcc.TIM; same field consumer | 100 / exact, unchanged |
| 8001b558 | 600 | map_resources_load | 100 | map definitions passed to pool loader; shared header consumer | 100 / exact, unchanged |
| 8001f0c4 | 340 | render_map_event | 100 | model bank index and u16 animation phase passed to renderer | 100 / exact, unchanged |
| 8001f218 | 1408 | render_entities | 91.25 | state byte 1 filters live events | 91.25 / partial, unchanged |
| 800205d4 | 932 | render_bind_animated_instance | 100 | phase selects/interpolates animation keyframes, separately from yaw | 100 / exact, unchanged |
| 8002c9d4 | 164 | talk_show_indexed_image | 98.78049 | floor, stage, character and page form TALK filename | 98.7805 / partial, unchanged |
| 800337ac | 116 | map_event_refresh_image_for_progress | 100 | byte stage clamp to highest floor and authored cap; resets page to 1 | 100 / exact, unchanged |
| 80033820 | 152 | map_event_advance_rotation_blocking | 100 | halfword animation phase; a1 target, signed a2 step; render/wait loop | 100 / exact, unchanged |
| 800338b8 | 556 | map_event_pool_load | 100 | byte states/behavior, five-byte limit copy, initial page/stage 1 | 100 / exact, unchanged |
| 80033b8c | 324 | map_event_pool_find_target_in_cone | 100 | active-state filter, distance and angle calls | 100 / exact, unchanged |
| 80033cd0 | 176 | map_event_pool_find_overlap | 100 | active-state filter over eight slots | 100 / exact, unchanged |
| 800341ec | 112 | map_ambient_script_floor2 | 100 | sound conditional on stage 2 and page below 3 | 100 / exact, unchanged |
| 80034438 | 388 | map_reveal_fade | 100 | moves/rotates event then disables state 3; sets transfer flag | 100 / exact, unchanged |
| 800345bc | 84 | map_action_script_floor2 | 100 | packed stage 2/page 1/delay 40 trigger and active state | 100 / exact, unchanged |
| 80034610 | 144 | map_action_script_floor3 | 100 | packed stage 3/page 1/delay 40 spell trigger | 100 / exact, unchanged |
| 80034a34 | 76 | map_action_script_floor5 | 100 | packed stage 5/page 1/delay 40 cutscene trigger | 100 / exact, unchanged |
| 80034a80 | 724 | map_event_interact | 100 | four TALK calls, character dispatch and per-stage page caps | 100 / exact, unchanged |
| 80034de4 | 2308 | map_interaction_dispatch | 83.01213 | behavior 0 shop, 1 wander, 2 animation loop; five blocking animation calls | 83.0468 / partial, target calls corrected |
| 800356e8 | 32 | map_event_timers_reset | 100 | dialogue gate 3 and ambient countdown 10 | 100 / exact, unchanged |
| 80035708 | 472 | map_event_update_wander | 100 | yaw/movement separate from phase increment 110 modulo 4096 | 100 / exact, unchanged |
| 800358e0 | 140 | map_event_update_spinner | 100 | phase increment 200 modulo 4096; sound on wrap, no yaw change | 100 / exact, unchanged |
| 8003596c | 496 | map_event_pool_update | 100 | active-state/behavior dispatch; page increment and signed promoted limit compare | 100 / exact, unchanged |
| 80035b5c | 696 | map_world_state_persist | 94.82184 | seven saved bytes per event, including current-stage last page | 94.8218 / partial, unchanged |
| 80035e44 | 1692 | map_restore_floor_state | 99.96454 | reverses seven-byte encoding; runtime activation/disable writes | 99.9645 / partial, unchanged |
| 800364e0 | 116 | map_refresh_event_images | 100 | eight-slot active-state filter and dialogue-stage refresh | 100 / exact, unchanged |
| 80036554 | 164 | map_load_floor | 100 | resource load, state restore and dialogue-stage refresh sequence | 100 / exact, unchanged |
| 80036d3c | 244 | actor_transform_definition5_to6 | 100 | disables floor-4 event slots 1 and 2 with state 3 | 100 / exact, unchanged |
| 80036e38 | 200 | menu_enter_mode | 100 | mode 2 passes character ID to shop root | 100 / exact, unchanged |

Keep control flow, delay slots, load widths and ordered referents. Propagate
distinct byte-backed state and behavior enums through definition/live fields,
the promoted state local and explicit save encoding/decoding. Do not compare
the two enum domains merely because active and wander both encode as 1.
The signed promoted page-limit local remains signed. No size assertions are
added. Rebuild affected units, compare linked words and canonicalized symbols /
relocations, run the full build, compiler type census, lint and repository tests.
Partial functions must keep their existing score; they are not exact claims.

## Semantic evidence

`image_dirty` is incremented and capped, then used as the final filename digit:
it is a one-based dialogue page. `image_index` is the one-based dialogue stage,
advanced with the highest visited floor but bounded by `image_limit`. The five
previously opaque tag bytes are last accessible pages indexed by stage minus
one. They are copied together and the current stage's limit is saved/restored.
Item exchanges raise selected limits, so these are availability caps rather
than counts of every image present on disc.

The resource decoder finds 17 authored event records in chunk 7 of the five
`MIXA` map resources. TALK files follow `TALK/Ccc/Tfsccp.TIM`: floor `f`, stage
`s`, character `cc`, page `p`. For example `TALK/C06/T33061.TIM` is floor 3,
stage 3, character 6, page 1. Character 7's floor-2 stage-2 limit starts at 1
although five pages exist; its item exchange raises the limit to 5. Character
8 similarly starts at 1 and unlocks 7. No proper character names are inferred.

State 0 occurs in the authored floor-5 character-11 record; the transfer flag
later sets it to 1. State 3 is written when runtime scripts disable events.
Both 0 and 3 fail active-state filters; the names describe these observed uses,
not an invented distinction in renderer behavior. State 255 terminates the
definition list and marks free pool slots.

`rotation_phase` feeds animation keyframes, whereas the separate `rotation`
field feeds `RotMatrix` and movement. Behavior 2 does not spin yaw. Animation
phase spans 4096 units; the blocking interaction pose is 2048 and the final
sample is 4095. Wander adds 110, looping idle animation adds 200, interaction
playback adds 200, and finishing idle animation before interaction adds 400.
These specify observed rates, not the designers' reason for choosing them.

The dialogue gate reloads 3 after its zero tick, so a delay count is decremented
once per four pool updates. A newly scheduled delay of 40 therefore takes 160
updates. This is not a duration in seconds without additional timing evidence.
The packed script predicates load the word at event+8 and ignore its low byte:
the other bytes are stage, page 1 and the freshly scheduled delay of 40.

Five relocation rows previously marked rejected are contradicted by retail
word `0x0c00ce08` at `80035070`, `800350a4`, `800350d0`, `800350f4`, `80035110`.
Each is a direct `jal 0x80033820`; each delay slot supplies a2 (200, 200, 400,
200, 200). The callee preserves a2 in s2. These are proven calls, not stack
arguments or unresolved call candidates. Correct the stale curated evidence.

## Retained literals and limits of this audit

Zero initialization, null pointers, boolean tests, unit increments, zero-based
array indices and conversion between one-based stages and array indices remain
literal arithmetic. TALK's offsets 6, 7 and 10..14 identify characters in the
literal path; division/modulo 10 and `'0'` encode decimal digits. They are kept
next to that path operation rather than given unrelated global constants.
Script-specific floor numbers, event slots, character IDs and page numbers
remain authored selectors: their operands now state the domain. Item IDs,
spell IDs, map coordinates and unrelated cutscene timing in `map_scripts.c`
still require their own evidence-led audit. This campaign does not claim that
the entire file or the global constant census is complete.

Distance/cone arithmetic and wandering movement, random-heading probability,
sound ranges and the ambient countdown are outside the constant replacements
in this campaign. Their literals remain pending the movement/audio audit;
they are not counted as explained merely because the enclosing functions were
checked for dialogue and enum propagation.

## Verification

Forced compilation covered all twelve campaign units. Across the complete
112-object source census, machine code, data and relocation bytes are unchanged;
ELF symbols retain all properties after the five function-name substitutions.
Six objects differ only in names or debug metadata. The state-versus-behavior
correction also emits exactly the previous instructions without cross-domain
casts or weakening the modern enum fields.

The 23 exact functions in the table still match all 1,691 linked retail words,
including ordered calls and data referents. Four partial functions keep their
scores. `map_interaction_dispatch` improves from 83.01213% to 83.04679% solely
because the target now has five proven call relocations. Its compiled bytes
are unchanged. An ignored copy restoring only those five rejected rows produces
the entire previous objdiff report after name normalization. The corrected
target adds five `R_MIPS_26` rows and changes only the corresponding encoded
call operands to relocation addends. No function was newly banked; the full
strict exact count remains 408 of 484.

Clang still passes 62/112 source/image variants. Its 323 error diagnostics in
the other 50 variants are unchanged after line-number normalization. Existing
pointer, SDK and prototype debt remains visible; no diagnostic suppression or
new enum/size test boilerplate was added.

The full 656-test run exposed a stale fixture field spelling, a hardcoded named
field census and the old test requiring genuine calls to stay rejected. Those
were corrected; all 114 affected inventory/world-layout tests then passed.
The remaining full-suite failure is the pre-existing, untracked save/load-hub
test's instruction mismatch. Ruff and `git diff --check` pass. The replacement
call test decodes hash-verified retail words and checks their argument delay
slots, rather than trusting the old curated assertion.

The full build retains the existing data placement/ownership failures:
source data 7/60 units match; target relink is PSX 1/1, GAME 75/77 and OPEN
34/38. The four SDK data providers match. These are not function naming
regressions and are not hidden by the native objdiff data percentages.

The three newly supported inventory fields are the definition/live dialogue
limit block and its last-page array (689 to 692 named fields). The ten
`unknown_` source lines remain unresolved; this campaign corrected misleading
names and recovered an opaque block rather than inventing names for copy-only
bytes. Repository-wide naming and the full literal ledger remain open.
