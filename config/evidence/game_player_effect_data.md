# GAME player effects: ownership and status-fade campaign

## Function Match Plan (before source changes)

Image is hash-checked Japanese `GAME.EXE`; baseline is `b339021`. Work stays
in the isolated `codex/data-matching-20260905` worktree. These units use
`probe-gcc257-o2-g0`, not the default GCC 2.6.0 probe. No original compiler,
linkage, or TU boundary is inferred from that choice.

| Function | VA / body | Baseline strict score | Evidence and first hypothesis |
| --- | --- | --- | --- |
| `player_update(void)` | `80018880` / `1a1c` | 96.515860% | Sole external call is `game_main_loop:8001482c`, no arguments/result consumed; 66 direct calls, 53 validated internal jump targets, no strings/candidate references. Full CFG, references, delay slots and source history inspected. Correct the status-1 branch topology separately, then own its matrix and eight view offsets. |
| `player_death_begin(void)` | `80015164` / `68` | 100% | Three game callers pass no arguments; straight-line state `ff`, cleared death accumulators, sound slot 1 at `80015188/8c`, `ReadColorMatrix`, and fog snapshot; no strings. Define the shared sound owner without editing its body. |
| `player_add_experience(s16)` | `80016058` / `224` | 88.824814% | Actor-damage caller `8002d434` loads the award halfword; entry explicitly sign-extends it. Threshold loop, level/stat caps, growth-table referents, stat recalculation, notification and sound slot 2 at `80016240/44`; no strings. Shared sound definition only. |
| `player_begin_weapon_attack(void)` | `80016b24` / `9c` | 100% | Sole caller `player_update:80019134`, no arguments/result; signed phase `-1` and byte weapon `ff` guards, sound slot 0 at `80016b5c/60`, halfword charge commit/clear and comparison with 5000; no strings. Shared declaration only. |

The corresponding complete disassemblies, incoming/outgoing references, match
reports, caller windows, neighboring function identities and previous semantic
dossiers were reviewed. All four functions are game policy, absent from the
vendor inventory; their SDK calls retain supplied Psy-Q headers. `SoundRef`
is the existing three-byte game selector, not an invented SDK structure.
`MATRIX` and `SVECTOR` retain the vendor layouts. The matrix helper at
`80020364` interpolates into a stack matrix before `SetColorMatrix`; it does
not modify the source matrix.

## Independently evidenced CFG correction

The signed status-1 timer at `player_state + 4a` is decremented after the
existing flag/cancellation clamp. An already inactive timer (`-1`) branches
at `8001991c` to `80019a10`, calling `fog_set_near(11000)` with the constant
in the call delay slot. Newly expired `-1` clears flag 2 and skips that update.

For an active timer, `80019994/98` sign-extends `timer - 968`. The `bgez` at
`8001999c` selects interpolation. Otherwise `800199a4..b0` computes and
sign-extends `32 - timer`; `bltz` at `800199b4` selects the fixed matrix.
Both nonnegative fades therefore call `lighting_set_color_matrix(matrix,
color_matrix_table, fade << 7)` and `fog_interpolate_near(5000,11000,fade << 7)`.
The middle phase calls `SetColorMatrix(matrix)` then `fog_set_near(5000)`.
The old C instead held the matrix at the high end, restored normal fog in
the middle, and omitted the inactive-timer fog call. This is a CFG error,
not a compiler/scheduler attribution.

First build only this correction. Compare bounded full-function calls with
explicit hooks and inert movement/equipment inputs. Then introduce DATA and
check code/ordered referents independently of newly exposed section placement.
No whole-function exactness or full-game execution is claimed by these controls.

## Complete reached data owners

| VA / size | Working identity / owner | Complete payload SHA-256 |
| --- | --- | --- |
| `80055810` / 9 | `player_sound_refs[3]`, `game.player_death`, shared | `40d624c1dafc7bd232bafeb0eef6328b279c4bb15f62705bf3e1e6358e547f3c` |
| `80055858` / 32 | `player_status_effect1_color_matrix`, `game.player_update`, private | `1dd66d0b6b8d902cb46da4a6fca05fb6f58a1c6b822c6a550cd5c8f934c8257f` |
| `80055878` / 64 | `player_damage_camera_offsets[8]`, `game.player_update`, private | `8cd244854dbbe4485fcebd2a0d517e42c1778c390a679ea0f15f5718765612b3` |

Sound entries are `{7,0,80}`, `{7,1,89}`, `{13,0,67}`; addends remain 0, 3,
and 6. The following three bytes stay unclassified, not added to the array.
Two consumers are in the existing death/vitals/stats module, the third uses
the shared player header. Original external linkage remains unknown.

The matrix has three equal rows `{666,233,1333}`, zero translation, and the
SDK's implicit two-byte alignment gap. Both references retain addend zero
(`800199bc/c0`, `800199f0/f4`). Its status number is literal state-field
evidence, not guessed effect lore.

`player_apply_damage:8001667c/80` sets update state 1 after nonzero damage
unless already `ff`. The update consumer copies an entire eight-byte SVECTOR
into `view_rotation_offset` at player offset `9a`. States 1..7 index at stride
8; state >=8 resets to zero and copies entry 0; `fe/ff` take the early death
dispatch. Signed X/Z pairs are `(0,0),(-32,-32),(-64,-64),(-48,-32),(-32,0),
(-16,32),(0,64),(-16,32)`; Y and each fourth lane are zero. The complete
LWL/LWR/SWL/SWR copy proves inclusion of the fourth lane. Relocation pairs
retain addends 0, 3, 0, 7, 4 at sites `80019a44`, `80019aa0`, `80019ab0`,
`80019ac0`, `80019ad0`. The old spurious magic reference was corrected in
`b339021`; these are now private to the update consumer in the known graph.
File-static is the working source scope, not recovered original linkage.

Explicit non-const initialized DATA preserves the current load-data model.
Do not merge unrelated TUs to explain address proximity. The far-away
`player_previous_input` already owned by the update module may expose an
inconsistent single `.data` placement once these early objects are defined.
Do not resolve that by padding, scattering objects, changing `-G`, or
overriding addresses in the bounded linker.

## Final verdict and verification

| Function | Final strict score | Verdict |
| --- | --- | --- |
| `player_update` | 96.945540% | Corrected the evidenced fade CFG; still non-exact. DATA ownership does not change the CFG-only score. |
| `player_death_begin` | 100% | Preserved exact; body unchanged. |
| `player_add_experience` | 88.824814% | Preserved; body unchanged, no new codegen attribution. |
| `player_begin_weapon_attack` | 100% | Preserved exact; body unchanged. |

Before defining DATA, the corrected source and retail agree on 2,516 scenarios
(5,032 isolated full-function calls): every initial timer `-1..1000` with
flag 2 set/clear, plus every update-state byte with HP zero/nonzero. The input
fixture takes the menu-cancel path to skip movement and magic, disables
equipment and other timers, admits only player/previous-input writes, and
fails unexpected callees. Matrix, fog, menu, pad, weapon-update and death
services are explicit hooks, not proofs of those bodies. Comparisons include
the entire resulting player object, previous input, fade arguments and state
dispatch. Persistent tests retain the retail witnesses, complete initializer
hashes, source/target payloads, scopes, extent boundaries and ten HI/LO pairs.

After adding DATA, the current candidate linker correctly refuses the
inconsistent `.data` placement; it is **not** reported as another successful
candidate execution. A raw CFG-only/current-object comparison instead proves
the function remains 6,632 bytes (6,640-byte emitted `.text`), with identical
relocation locations/types and no changed instructions except eleven LO16
addends: six previous-input references gain 96 and five camera-table references
gain 32. Seven HI/LO symbol pairs now name the local `.data` section. All 66
direct calls, including multiplicity, agree with retail; no call is erased.
The first remaining divergence is still the entry frame (216 compiled versus
224 retail bytes); the fade path also keeps differing sign-extension/register
and delay-slot scheduling forms. No backend mechanism is claimed and no whole
function closure follows from the bounded scenarios.

All 105 declared bytes equal retail in both complete compiled datum slices and
delinked datum slices, including the MATRIX alignment gap and all SVECTOR
fourth lanes. Ten reviewed pairs retain raw S+A, named owner and addend through
delink/re-encode controls. Original pair-review provenance is retained. The
sound module independently relinks to retail. The update target cannot yet:
its matrix/table imply section base `80055858`, while previous input at offset
96 implies `80057ad0`. Current source `.data` is 112 bytes against target 100;
the sound `.data` is 16 against 9. These are honest whole-section failures,
not permission to crop source output or scatter the target symbols.

All 484 function-report rows were compared with `b339021`; only the update
row changes. All **354/471 game exact functions** remain exact, with 13 vendor
verification functions excluded from game progress. Of 114 compiled objects,
only the two player modules change; of 1,719 delinked objects, only those two
modules and the standalone update target change. Strict DATA remains 11/60,
SDK contributions 2/2. GAME reached source owners rise 70 to 73; config-only
owners fall 383 to 380. Across images, unpaired config ranges fall 660 to 657.
Target relink is now 107/114, with seven explicit placement conflicts instead
of six. No original TU, storage-class split, or linked-image equality is proved.

All 546 local tests, Ruff and diff checks pass. `nix flake check -L` passes
with 66 expected clean-environment skips (local retail/objects are absent).
The default full build was run and remains red on strict DATA, reachability
and placement. No bank, waiver,
compiler-option change, or main-worktree integration belongs to this campaign.
