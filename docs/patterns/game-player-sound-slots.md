# Player sound-table slots and current core ledger

## Function Match Plan

At `97740c4`, name the three slots of the shared GAME `player_sound_refs`
table and its bound, preserving the initializer, typed `SoundRef` pointer API,
call order and delay slots. Use the existing project pattern for named array
slots; these constants select records and are not stored state fields.
Capture all six GAME semantic views for both player modules and the common
sound helper before editing. Review callers, adjacent functions, source history,
raw MIPS words and ordered references. The game-specific cue choices and player
state updates establish game ownership; Sony voice services remain callees.

| Slot | Name | GAME reference pair / call | Evidence |
| ---: | --- | --- | --- |
| 0 | `KF_PLAYER_SOUND_WEAPON_ATTACK` | `80016b5c/80016b60`, call `80016b64` | After the inactive-phase/equipped-weapon guards, starts a swing and plays the first record. |
| 1 | `KF_PLAYER_SOUND_DEATH` | `80015188/8001518c`, call `80015190` | Sets player dying state, resets death visual increments, then plays the second record. |
| 2 | `KF_PLAYER_SOUND_LEVEL_UP` | `80016240/80016244`, call `80016248` | Each earned level recalculates stats and posts the level-up notification before playing the third record. |

All three calls pass volume 127 in their delay slot. Validated HI16/LO16
references select byte offsets 0, 3 and 6 in GAME `80055810..80055819`.
The helper loads bytes 0, 1 and 2 and forwards them as program, tone and note
to `audio_play_voice`, with duplicated stereo volume. Keep the nine authored
selector bytes `(7,0,80), (7,1,89), (13,0,67)` unchanged. Their original sound
design rationale is unknown; cue identity comes from control flow.

Reconcile every current player-core literal against the existing core/motion
ledgers. The old total of 80 predates floor-enum propagation: the current
baseline is 79. Refresh the floor-index expression and remove the already
named floor-five row. Integrate all fourteen functions and both initializers
into one complete ledger, while retaining the motion evidence link. Update the
complete death ledger after naming the bound and two slots. Do not claim that
an accounting match establishes semantics for unresolved map attribute 82.

Before commit, force all campaign objects, compare raw words/references and
strict scores, and compile all 112 variants in isolated before/after trees.
Check the nine initialized bytes against retail and both linked objects.
Run modern checking, inventory, Ruff, the existing tests, whitespace and full
`kf build`. Update the existing shared-declaration expectation for the named
bound; add no tests or size assertions. Record concurrent changes explicitly.

## Per-function snapshots and final verdicts

| GAME VA / bytes | Function | Strict before / after | Scope | Words / calls / address references | Verdict |
| --- | --- | ---: | --- | ---: | --- |
| `0x80015164 / 104` | `player_death_begin` | 100.000000% | Name selected sound slot | 26 / 2 / 7 | Exact preserved |
| `0x800151cc / 740` | `game_state_initialize` | 100.000000% | Unchanged source; complete module or common-callee control | 185 / 3 / 77 | Exact preserved |
| `0x800154b0 / 412` | `player_death_restart` | 100.000000% | Unchanged source; complete module or common-callee control | 103 / 6 / 32 | Exact preserved |
| `0x8001564c / 112` | `player_adjust_hp` | 100.000000% | Unchanged source; complete module or common-callee control | 28 / 1 / 3 | Exact preserved |
| `0x800156bc / 88` | `player_adjust_mp` | 100.000000% | Unchanged source; complete module or common-callee control | 22 / 0 / 3 | Exact preserved |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.000000% | Unchanged source; complete module or common-callee control | 517 / 3 / 130 | Exact preserved |
| `0x80015f28 / 152` | `player_increment_physical_power_training` | 100.000000% | Unchanged source; complete module or common-callee control | 38 / 2 / 5 | Exact preserved |
| `0x80015fc0 / 152` | `player_increment_magic_training` | 100.000000% | Unchanged source; complete module or common-callee control | 38 / 2 / 5 | Exact preserved |
| `0x80016058 / 548` | `player_add_experience` | 88.824814% | Name selected sound slot | 132 / 3 / 5 | Partial preserved; residue below |
| `0x8001627c / 168` | `player_calculate_damage_component` | 100.000000% | Unchanged source; complete module or common-callee control | 42 / 0 / 0 | Exact preserved |
| `0x80016324 / 912` | `player_apply_damage` | 100.000000% | Unchanged source; complete module or common-callee control | 228 / 6 / 24 | Exact preserved |
| `0x800166b4 / 304` | `player_apply_radial_damage` | 100.000000% | Unchanged source; complete module or common-callee control | 76 / 2 / 0 | Exact preserved |
| `0x800167e4 / 100` | `player_select_magic` | 100.000000% | Unchanged source; complete module or common-callee control | 25 / 0 / 6 | Exact preserved |
| `0x80016848 / 488` | `player_set_equipment_slot` | 100.000000% | Unchanged source; complete module or common-callee control | 122 / 1 / 27 | Exact preserved |
| `0x80016a30 / 244` | `player_equip_weapon` | 100.000000% | Unchanged source; complete module or common-callee control | 61 / 4 / 12 | Exact preserved |
| `0x80016b24 / 156` | `player_begin_weapon_attack` | 100.000000% | Name selected sound slot | 39 / 1 / 9 | Exact preserved |
| `0x80016bc0 / 612` | `player_update_weapon_attack` | 100.000000% | Unchanged source; complete module or common-callee control | 153 / 5 / 28 | Exact preserved |
| `0x80016e24 / 148` | `game_initialize_session` | 100.000000% | Unchanged source; complete module or common-callee control | 37 / 2 / 12 | Exact preserved |
| `0x80016eb8 / 48` | `player_clear_motion` | 100.000000% | Unchanged source; complete module or common-callee control | 12 / 0 / 5 | Exact preserved |
| `0x80016ee8 / 344` | `player_sync_position_to_map` | 100.000000% | Unchanged source; complete module or common-callee control | 86 / 2 / 17 | Exact preserved |
| `0x80017040 / 200` | `player_distance_to_point_in_cone` | 100.000000% | Unchanged source; complete module or common-callee control | 50 / 2 / 2 | Exact preserved |
| `0x80017108 / 244` | `player_distance_to_point` | 100.000000% | Unchanged source; complete module or common-callee control | 61 / 1 / 3 | Exact preserved |
| `0x800171fc / 2088` | `player_move_horizontal` | 96.568960% | Unchanged source; complete module or common-callee control | 532 / 7 / 43 | Partial preserved; residue below |
| `0x80017a24 / 92` | `player_update_view_bob` | 100.000000% | Unchanged source; complete module or common-callee control | 23 / 1 / 5 | Exact preserved |
| `0x80017a80 / 632` | `player_update_vertical_motion` | 100.000000% | Unchanged source; complete module or common-callee control | 158 / 1 / 24 | Exact preserved |
| `0x80017cf8 / 324` | `player_warp_to_floor_entry` | 100.000000% | Unchanged source; complete module or common-callee control | 81 / 6 / 13 | Exact preserved |
| `0x80017e3c / 160` | `player_update_transform_snapshot` | 100.000000% | Unchanged source; complete module or common-callee control | 40 / 0 / 5 | Exact preserved |
| `0x800330ac / 72` | `sound_ref_play` | 100.000000% | Unchanged source; complete module or common-callee control | 18 / 1 / 1 | Exact preserved |

## Verification and accounting

The three cue slots and their shared count now have enum constants. All uses
of `player_sound_refs` select the named weapon-attack, death or level-up slot;
the definition and shared declaration use the same count. The existing data
ownership test now expects that named bound. No signatures, state-field types,
selector bytes, table order or playback API behavior changed.

All 28 reviewed functions retain their baseline: 26 exact, two partial.
The raw comparison covers 2,933 instruction words, 64 ordered calls and 503
ordered address references. Exact controls also reproduce 2,269 retail words
and the delinked referents. Both partials retain their existing first divergence:

| Function | GAME VA | Reconstruction | Retail | Verdict |
| --- | --- | --- | --- | --- |
| `player_add_experience` | `8001605c` | `sw ra,28(sp)` | `sw ra,24(sp)` | Existing stack-slot residue. |
| `player_move_horizontal` | `80017228` | `move s6,a0` | `move s7,a0` | Existing register residue. |

These are unattributed codegen residues. The changes preserve all call, CFG,
return and ordered-reference evidence, including the sound-call delay slots.
Neither partial is newly banked. Source initializer parsing, the reconstructed
object and the delinked object each match all nine retail sound-selector bytes.

All 112 source/image variants were compiled from isolated before/after trees
at identical paths with the pinned profiles. Every section, including debug
sections, is unchanged; runtime symbols and ordered relocations also agree.
At the comparison snapshot, all live objects agree with the isolated controls
and all 484 strict scores are unchanged. Concurrent OPEN entity-render work
is separate; only this campaign's files are included in the commit.

Forced player/audio compilation, strict comparison, inventory, Ruff and
whitespace checks pass. All 683 existing tests pass in 91.904 seconds. Modern
checking retains the same 300 diagnostics, with 65/112 variants passing and
47 failing on existing debt. No new tests or size assertions were introduced.
Full `kf build` still fails on existing data/placement/ownership gaps: source
data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77,
OPEN 34/38; six conflicting section bases and zero artifact failures.

Four inline literal occurrences became names: three slots and one table bound.
The [complete core ledger](game-player-core-literal-ledger.md) now covers all
78 retained occurrences; the [death ledger](game-player-death-literal-ledger.md)
covers 190. The former motion-ledger page points to the consolidated rows.
Independent token/expression/multiplicity validation now covers 40 source
files and 4,002 occurrences. The total source census is 6,631; this is not a
count of missing semantic names. Ten source lines still contain fourteen
`unknown_` tokens; the overall naming goal remains open.
