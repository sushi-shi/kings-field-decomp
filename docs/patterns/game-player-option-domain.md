# Player configuration option domain

## Function Match Plan

At `959b8fc`, propagate `KfPlayerOption` through the four saved configuration
bytes, their signed-word menu copies, the drawing helper pointer and the original
music-value snapshot. Use `KF_PLAYER_OPTION_OFF=0` and `KF_PLAYER_OPTION_ON=1`,
with signed-word domain values and unsigned-byte field storage. The same
on/off setting contract is shared by effects, music, gauges and compass.
Preserve arbitrary imported values and byte narrowing; the enum is not runtime
validation. Explicitly decode the toggle's Boolean result into this domain.

| Consumer | Retail contract | Source intent |
| --- | --- | --- |
| New session | Four byte stores of one | Initialize the four typed fields to ON. |
| Config import | `lbu` at player +96/+97/+98/+99; four `sw` copies | Widen each saved byte into a word option without normalization. |
| Toggle | GAME `80025c54..80025c64`: `lw`, `sltiu value,1`, `sw` in jump delay slot | Zero becomes ON; every other word becomes OFF. |
| Config draw | Four word loads, exact-one highlight, pointer step four | Preserve ON only for exact value one; two by-value labels retain the established ABI. |
| Config publication | Four low-byte loads/stores; music reloaded before comparison | Narrow each option back to its saved byte and compare the reloaded music byte with the original word. Both exit inputs publish. |
| Music / effects | GAME `80032abc` / `80033158`: zero guard | All nonzero values permit audio; do not replace this with an exact-ON check. |
| Gauges | Exact-one guard | Only ON permits gauge drawing. |
| Compass | Unconditional byte copy to HUD/effect sprite state | Explicitly encode the option into the separate existing sprite-state channel, preserving even 255. |

The complete player copy in raw save read/write includes these fields. Keep
those byte streams unchanged. HUD scanning stops on state 255 and draws only
state one; effect-sprite drawing continues only while state is one. Do not
normalize compass state or claim every nonzero byte is visible.

Retail hashes, all six GAME views, source history, caller/callee widths,
CFG, delay slots and ordered references are captured before editing. The
[configuration ABI review](game-config-panel-abi.md) establishes two 24-byte
labels followed by the state pointer at incoming +48. The
[choice review](game-menu-choice-state.md) verifies ON/OFF glyphs and loaded
row baselines. Name the local screen-pixel anchors ON X180, OFF X240, first
Y41 and row step22, preserving their halfword arithmetic and exact layout.
Encoded characters remain authored glyph data. Game configuration policy and
saved player fields establish game ownership; Sony pad/GPU/audio APIs remain
external services, not reconstructed library progress.

Require unchanged exact functions, current partial scores, raw words, ordered
calls and referents after forced compilation. Compile all 112 variants from
isolated before/after trees. Run negative compiler controls against actual
fields/headers/helper declarations, including foreign enums, integers and
pointer types; preserve unlisted values in positive controls. Run modern
checks, inventory, Ruff, repository tests, whitespace and full `kf build`.
Publish a complete current config ledger and refresh affected core/render
ledgers. Add no size assertions or permanent tests.

## Function snapshots and final verdicts

| GAME VA / bytes | Function | Strict before / after | Words / calls / address references | Verdict |
| --- | --- | ---: | ---: | --- |
| `0x80016e24 / 148` | `game_initialize_session` | 100.000000% | 37 / 2 / 12 | Exact preserved |
| `0x8001f8b0 / 292` | `render_effect_sprites` | 100.000000% | 73 / 12 / 2 | Exact preserved |
| `0x8001f9d4 / 112` | `render_hud_gauges` | 100.000000% | 28 / 1 / 0 | Exact preserved |
| `0x8001fde4 / 1304` | `render_frame` | 100.000000% | 326 / 20 / 64 | Exact preserved |
| `0x80022348 / 704` | `menu_root` | 96.863640% | 175 / 28 / 1 | Partial preserved; residue below |
| `0x8002589c / 1284` | `menu_config_panel` | 99.859810% | 321 / 19 / 9 | Partial preserved; residue below |
| `0x80025da0 / 408` | `menu_config_panel_draw` | 100.000000% | 102 / 9 / 11 | Exact preserved |
| `0x8002b73c / 1268` | `save_file_write_slot` | 100.000000% | 317 / 24 / 28 | Exact preserved |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100.000000% | 243 / 13 / 15 | Exact preserved |
| `0x80032a4c / 272` | `audio_play_map_sequence` | 100.000000% | 68 / 6 / 8 | Exact preserved |
| `0x800330f4 / 424` | `audio_play_voice` | 100.000000% | 106 / 2 / 9 | Exact preserved |

## Verification and retained literals

The four saved fields now use `KF_ENUM_STORAGE(KfPlayerOption, u8)`; the menu
array, saved music value and helper pointer use signed-word `KfPlayerOption`.
The helper identity and four curated field types agree with the source.
All direct producers and consumers use the domain. Compass publication uses
explicit unsigned-byte encoding at the two existing sprite-state boundaries.
The enum does not erase the renderer's exact-one rule, the audio nonzero rule,
the signed-word toggle or the post-publication music-byte reload.

All eleven reviewed functions retain their baseline: nine exact and two
partial. Before/after controls compare 1,796 instruction words, 136 ordered
calls and 159 ordered address references. The nine exact functions also
reproduce 1,300 retail words and the delinked referents. The unchanged first
partial divergences are:

| Function | GAME VA | Reconstruction | Retail | Verdict |
| --- | --- | --- | --- | --- |
| `menu_root` | `8002234c` | `sw ra,40(sp)` | `sw ra,44(sp)` | Existing saved-register stack-slot residue. |
| `menu_config_panel` | `800258d0` | `addiu s5,zero,-99` | `addiu s6,zero,-99` | Existing register residue for the open phase. |

These are unattributed codegen residues. The enum and layout changes preserve
call sets, CFG, return forms, delay slots and ordered referents. No partial is
promoted or banked.

All 112 source/image variants were compiled twice in isolated trees from
`959b8fc`, applying only this campaign's five source/header files to the after
tree. Runtime sections, alignment, runtime symbols and ordered relocations
are identical throughout; only the config module's debug-line data changes.
Live objects match the isolated after controls except the separately edited
`open.entity_render`. That concurrent experiment changed the observed
`OPEN.EXE 80019240` score from 96.759030% to 98.885544%; the other 483 scores
are unchanged. The live OPEN object was reproduced by a fresh compile of
its independent source snapshot (SHA-256
`420e4ff9c902fd40f04d62ae1ff9ba7a306831f0ebb12449233ac59825421bb3`).
An earlier snapshot did not reproduce the object while the experiment was
changing; no intermediate OPEN difference is attributed to this campaign.
The snapshot reproduction and campaign runtime equality are separate controls.

The valid compiler probe accepts field/local/helper flow, preserves imported
word values two and minus one, and preserves unsigned-byte narrowing of 257
to one. Twelve negative probes reject raw integers, foreign enums, implicit
Boolean/byte/word conversions, wrong pointer types and direct Boolean-result
assignment to an option. The initial probe lacked the owning player header;
after including it, the positive probe passes and every negative probe fails
for the intended type constraint. No permanent tests or size assertions were
added. Whole-tree modern diagnostics remain exactly the prior 300 errors:
65/112 variants pass and 47 fail on existing debt.

Forced compilation, strict comparison, inventory, Ruff and whitespace checks
pass. The initial inventory/test run caught four stale curated field types;
updating those rows restores inventory validation and all 683 repository tests
pass on the complete rerun (90.788 seconds). Full `kf build` was rerun after
that correction and still fails on existing data/placement/ownership gaps:
source data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1,
GAME 75/77, OPEN 34/38; six conflicting section bases and zero artifact
failures. These are outstanding full-build failures, not a passing build.

Fourteen inline occurrences become named: four fresh-session values, three
config value tests, one gauge test and six layout uses. The
[complete config ledger](game-menu-config-literal-ledger.md) accounts for its
43 retained occurrences; core has 74 and frame rendering 46. The old grouped
choice ledger now clearly marks its config slices historical. Exact
source-token/expression/multiplicity validation covers 41 files and 4,040
occurrences. The whole-source census is 6,617, not a missing-name count.
Ten source lines retain fourteen unresolved `unknown_` tokens, and the overall
naming goal remains open.
