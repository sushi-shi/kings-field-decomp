# Audio sequence state and stop mode

## Function Match Plan

At `35b6149`, separate two signed-word domains: the stored sequence bookkeeping
state (`KfAudioSequenceState`, INACTIVE=0/ACTIVE=1) and OPEN's stop request
(`KfAudioStopMode`, IMMEDIATE=0/FADE=1). Propagate the stored type through
`KfAudioState.sequence_active`, its eleven reads/writes, and the stop-mode type
through the shared declaration, OPEN definition and all ten direct callers.
Preserve exact-one comparisons; decoded values other than one remain inactive
or immediate, respectively. The state records issued playback/cleanup policy,
not independently confirmed hardware playback.

The five affected units were forcibly rebuilt before the baseline capture.
Hash-verified retail, all six image-qualified semantic views, source/history,
adjacent functions, callers, SDK callees and cached objects/scores precede edits.
GAME clears the state word at `80032954`, sets one at `80032b44`, tests exact one
at `80032b74/80032bf8`, and clears it after stop/close. OPEN performs both authored
zero stores at `80019c18/80019c48`, sets one at `80019e0c`, tests exact one at
`80019e3c`, and clears it at `80019eac`. The field is word +0x10 in both layouts.

OPEN `80019e44` compares the incoming word to the known active value one before
the optional master-volume fade. Every other value skips that loop; both paths
then zero master/sequence volume and stop/close the sequence. The ten callers
supply six fade requests and four immediate requests in their JAL delay slots.
Keep the actual word argument, source CFG, signed fade countdown, exact volume
endpoints and delay slots. GAME's fixed-point fade-step parameter is a numeric
step, not this OPEN mode, and retains its existing type.

These functions implement game sequence paths, resource transitions, voice
reuse and scene policy. The Psy-Q sequence, SPU, CD, pad and GPU services remain
external SDK calls; no vendored body is reconstructed. Preserve authentic SDK
parameter types and mode spellings. Preserve all unrelated raw values and
referents, including GAME's impossible `(tone & 0x80) == 1` branch.

Update the one curated field type and OPEN function parameter identity. Compile
all 112 source/image variants independently before/after and compare allocated
bytes, runtime symbols and ordered relocations. Check each campaign function's
words and referents, strict scores and any existing partial residue. Use actual
headers for compiler acceptance/rejection controls, then run modern whole-tree
checking, inventory, Ruff, repository tests, whitespace and full `kf build`.
Add no production tests or size assertions. Complete current audio/resource/
controller literal ledgers and reconcile the existing full OPEN scene ledger.

## Per-function snapshots

| Image | VA / bytes | Function | Strict baseline | Calls / branches |
| --- | --- | --- | ---: | --- |
| GAME.EXE | `0x800328e0 / 164` | `audio_initialize` | 100.000000% | 9 / 1 |
| GAME.EXE | `0x80032984 / 200` | `audio_load_vab` | 100.000000% | 6 / 2 |
| GAME.EXE | `0x80032a4c / 272` | `audio_play_map_sequence` | 100.000000% | 6 / 2 |
| GAME.EXE | `0x80032b5c / 128` | `audio_stop_sequence_fade` | 100.000000% | 4 / 2 |
| GAME.EXE | `0x80032bdc / 156` | `audio_stop_sequence_master_fade` | 100.000000% | 6 / 2 |
| GAME.EXE | `0x80032c78 / 56` | `audio_shutdown` | 100.000000% | 3 / 0 |
| GAME.EXE | `0x80032cb0 / 64` | `audio_close_vab` | 100.000000% | 1 / 0 |
| GAME.EXE | `0x80032cf0 / 712` | `audio_play_spatial` | 100.000000% | 5 / 18 |
| GAME.EXE | `0x80032fb8 / 48` | `audio_play_spatial_default_range` | 100.000000% | 1 / 0 |
| GAME.EXE | `0x80032fe8 / 44` | `audio_play_spatial_range` | 100.000000% | 1 / 0 |
| GAME.EXE | `0x80033014 / 40` | `sound_ref_key_off_bank0` | 100.000000% | 1 / 0 |
| GAME.EXE | `0x8003303c / 112` | `audio_set_listener_transform` | 100.000000% | 0 / 2 |
| GAME.EXE | `0x800330ac / 72` | `sound_ref_play` | 100.000000% | 1 / 0 |
| GAME.EXE | `0x800330f4 / 424` | `audio_play_voice` | 100.000000% | 2 / 6 |
| GAME.EXE | `0x8003329c / 72` | `angle_shortest_delta` | 100.000000% | 0 / 2 |
| OPEN.EXE | `0x80014268 / 372` | `opening_scene0_run` | 100.000000% | 11 / 6 |
| OPEN.EXE | `0x800143dc / 384` | `opening_scene1_draw_fade` | 100.000000% | 8 / 0 |
| OPEN.EXE | `0x8001455c / 172` | `opening_scene1_run` | 100.000000% | 8 / 5 |
| OPEN.EXE | `0x80014608 / 508` | `opening_entity_transition` | 99.921260% | 2 / 12 |
| OPEN.EXE | `0x80014804 / 816` | `opening_scene3_run` | 99.931370% | 27 / 10 |
| OPEN.EXE | `0x80014b34 / 756` | `opening_ending_scene_run` | 100.000000% | 20 / 8 |
| OPEN.EXE | `0x80014e28 / 1944` | `opening_ending_scroll_run` | 97.952675% | 40 / 26 |
| OPEN.EXE | `0x800156bc / 532` | `opening_run` | 100.000000% | 31 / 7 |
| OPEN.EXE | `0x80016014 / 328` | `cd_file_load_allocated` | 100.000000% | 7 / 5 |
| OPEN.EXE | `0x8001615c / 316` | `cd_file_load_into` | 100.000000% | 6 / 5 |
| OPEN.EXE | `0x80016298 / 128` | `tim_upload_images` | 100.000000% | 6 / 3 |
| OPEN.EXE | `0x80016318 / 48` | `resource_stream_copy_words` | 100.000000% | 0 / 2 |
| OPEN.EXE | `0x80016348 / 456` | `opening_resources_load_scene0` | 100.000000% | 18 / 0 |
| OPEN.EXE | `0x80016510 / 180` | `opening_resources_load_scene1` | 100.000000% | 8 / 0 |
| OPEN.EXE | `0x800165c4 / 240` | `opening_resources_load_scene3` | 100.000000% | 12 / 0 |
| OPEN.EXE | `0x800166b4 / 308` | `opening_resources_load_ending` | 100.000000% | 12 / 0 |
| OPEN.EXE | `0x800167e8 / 88` | `opening_resources_load_ending_entities` | 100.000000% | 3 / 0 |
| OPEN.EXE | `0x80016840 / 156` | `opening_resources_load_ending_sequence` | 100.000000% | 7 / 0 |
| OPEN.EXE | `0x80019ba4 / 184` | `audio_initialize` | 100.000000% | 9 / 1 |
| OPEN.EXE | `0x80019c5c / 244` | `audio_load_vab` | 100.000000% | 10 / 3 |
| OPEN.EXE | `0x80019d50 / 212` | `audio_play_sequence_file` | 100.000000% | 9 / 2 |
| OPEN.EXE | `0x80019e24 / 156` | `audio_stop_sequence` | 100.000000% | 6 / 3 |
| OPEN.EXE | `0x80019ec0 / 56` | `audio_shutdown` | 100.000000% | 3 / 0 |
| OPEN.EXE | `0x80019ef8 / 76` | `audio_close_vab` | 100.000000% | 1 / 1 |

## Final implementation and verification

The stored sequence state and stop-mode argument now have distinct signed-word
enum types. All eleven state uses, ten call arguments and the stop-mode predicate
use their owning constants. The modern view rejects raw integers and values from
the other domain; the legacy view retains the original field and O32 widths.
Explicitly decoded state two remains non-active, and stop modes two and minus
one remain immediate. Encoding a decoded minus-one state preserves its sign.
The exact-one tests, both OPEN initialization stores, volume loops and all
side-effect order remain unchanged.

One positive compiler control accepts field/local/argument/callback propagation
through the actual header. Twelve negative controls reject raw field/local/
argument values, mixed-domain assignment/comparison/calls, implicit integer
encoding and an untyped callback signature. These are temporary syntax controls
under `build/`; no production tests or size assertions were added.

All 112 independently compiled before/after variants retain identical sections,
runtime symbols and ordered relocations, including debug sections. Every live
object agrees with the isolated result and all 484 strict scores are unchanged.
The 39 campaign functions preserve 2,806 candidate instruction words, 310 ordered
direct calls and 222 address materializations. The 36 exact controls preserve
all 1,989 retail words, including delay slots. Existing partial controls keep
these first raw divergences:

| OPEN function | Site | Candidate / retail |
| --- | --- | --- |
| `opening_entity_transition` | `80014608` | Frame allocation 48 / 56 bytes. |
| `opening_scene3_run` | `80014804` | Frame allocation 96 / 112 bytes. |
| `opening_ending_scroll_run` | `800150dc` | `addiu t1,sp,0x98` / `addiu t0,sp,0x98`. |

These remain unattributed existing residues; no partial function was promoted
or newly banked. The field inventory, function identity and existing OPEN
resource-evidence signature all reflect the typed interface. The first test
run exposed the evidence table's stale signature; correcting that mirror makes
the full 683-test rerun pass (95.541 seconds). Inventory, Ruff and whitespace
checks pass. Modern diagnostics remain the same 300 errors, with 65/112 variants
passing. Full `kf build` retains source-data mismatches (PSX 0/1, GAME 9/42,
OPEN 2/19) and target-relink gaps (PSX 1/1, GAME 75/77, OPEN 34/38), with six
conflicting section bases and zero artifact failures.

The [complete audio/resource ledger](audio-sequence-literal-ledger.md) accounts
for 100 retained occurrences in the four newly covered modules. The existing
OPEN scene ledger now accounts for 592 after its three stop arguments were
named. Together this removes 22 inline occurrences and brings complete coverage
to 50 files / 4,194 retained occurrences. The overall census is 6,567. Its GAME
subtotal also corrects an older 38-occurrence summary overcount; all image/
category subtotals are now checked against the individual file rows. The source
unknown count remains ten lines with fourteen identifier tokens.

## Per-function final verdicts

| Image | Address | Function | Final strict score | Verdict |
| --- | --- | --- | ---: | --- |
| GAME.EXE | `0x800328e0` | `audio_initialize` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032984` | `audio_load_vab` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032a4c` | `audio_play_map_sequence` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032b5c` | `audio_stop_sequence_fade` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032bdc` | `audio_stop_sequence_master_fade` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032c78` | `audio_shutdown` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032cb0` | `audio_close_vab` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032cf0` | `audio_play_spatial` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032fb8` | `audio_play_spatial_default_range` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80032fe8` | `audio_play_spatial_range` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x80033014` | `sound_ref_key_off_bank0` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x8003303c` | `audio_set_listener_transform` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x800330ac` | `sound_ref_play` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x800330f4` | `audio_play_voice` | 100.000000% | Exact; words and referents unchanged. |
| GAME.EXE | `0x8003329c` | `angle_shortest_delta` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80014268` | `opening_scene0_run` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x800143dc` | `opening_scene1_draw_fade` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x8001455c` | `opening_scene1_run` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80014608` | `opening_entity_transition` | 99.921260% | Existing partial; words and referents unchanged. |
| OPEN.EXE | `0x80014804` | `opening_scene3_run` | 99.931370% | Existing partial; words and referents unchanged. |
| OPEN.EXE | `0x80014b34` | `opening_ending_scene_run` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80014e28` | `opening_ending_scroll_run` | 97.952675% | Existing partial; words and referents unchanged. |
| OPEN.EXE | `0x800156bc` | `opening_run` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016014` | `cd_file_load_allocated` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x8001615c` | `cd_file_load_into` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016298` | `tim_upload_images` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016318` | `resource_stream_copy_words` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016348` | `opening_resources_load_scene0` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016510` | `opening_resources_load_scene1` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x800165c4` | `opening_resources_load_scene3` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x800166b4` | `opening_resources_load_ending` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x800167e8` | `opening_resources_load_ending_entities` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80016840` | `opening_resources_load_ending_sequence` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019ba4` | `audio_initialize` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019c5c` | `audio_load_vab` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019d50` | `audio_play_sequence_file` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019e24` | `audio_stop_sequence` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019ec0` | `audio_shutdown` | 100.000000% | Exact; words and referents unchanged. |
| OPEN.EXE | `0x80019ef8` | `audio_close_vab` | 100.000000% | Exact; words and referents unchanged. |
