# GAME internal save-status domain

## Function Match Plan

Use one signed-word `KfSaveStatus` for the existing internal channel shared
by card-event polling, formatting and raw file operations. Preserve the
existing event and file-policy names, including success/I/O-end at one and
format-required/new-device at three. Move their declarations into the shared
save header and propagate the domain through producers and status locals.

The menu-facing wrappers have different result mappings and forward unlisted
statuses. Keep their signed integer results: encode the incoming status once,
switch on the typed status and assign the existing operation-specific result
values. Temporary-file cleanup also mixes a Boolean open result with card
statuses; explicitly encode its status exits and the game's timeout comparison.
Do not turn these APIs into a common success/failure enum.

`memory_card_show_status_message` interprets a signed halfword. GAME
`8002c518/8002c51c` narrow before the table lookup, and `8002c5b4/8002c5bc`
narrow the selected/default image value before its loader call. Preserve
the legacy halfword parameter with `KF_ENUM_PARAM` and an explicitly narrow
typed local for modern checking. Image IDs remain a separate numeric channel.
Codes 9 and 10 have a common unusable-card message but no distinct proved
producer; retain explicitly decoded numeric cases and document that limit.

Before source edits, initialize hash-identical retail and capture all six
GAME semantic views for the entire save unit, its save/load menu consumers
and `game_main_loop`. Review their raw instructions, caller/callee widths,
switch targets, ordered relocations, strings, adjacent claims, source history
and the [save protocol](../save-system.md). Polling starts only after a nonzero
`_card_info` result; a zero result returns not-started immediately. Game save
paths, payload ownership and UI mappings distinguish these routines from
the called Sony card/file/GPU services.

Preserve every retry bound, partial catalog clear, slot search, unguarded
previous-slot store, initializer, default path, call and delay slot. Build
the changed unit from the first edit; require before/after runtime equality,
all campaign scores and exact retail controls. Compile all 112 variants in
separate committed-source controls so concurrent OPEN work cannot be mistaken
for these changes. Inspect current-object/score differences explicitly.
Require modern enum rejection controls, diagnostic comparison, inventory,
Ruff, repository tests, whitespace and full `kf build`. Complete the
retained-literal ledger, with no new size assertions or tests. The baseline
has 154 occurrences. The write/read directory searches initialize and test
minus one as an absent entry; name those five occurrences
`SAVE_DIRECTORY_ENTRY_NOT_FOUND`. Keep the writer's unguarded use of the
previous-entry result unchanged; naming does not establish that it is safe.

## Per-function snapshots and final verdicts

| GAME VA / bytes | Function | Strict before / after | Words / calls / address references | Verdict |
| --- | --- | ---: | ---: | --- |
| `0x800146b8 / 740` | `game_main_loop` | 100.000000% | 185 / 43 / 20 | Exact preserved |
| `0x800250c4 / 1128` | `menu_save_panel` | 100.000000% | 282 / 58 / 0 | Exact preserved |
| `0x8002552c / 880` | `menu_load_panel` | 100.000000% | 220 / 42 / 0 | Exact preserved |
| `0x8002b078 / 216` | `save_system_read_catalog` | 100.000000% | 54 / 2 / 1 | Exact preserved |
| `0x8002b150 / 132` | `menu_play_input_sound` | 100.000000% | 33 / 3 / 0 | Exact preserved |
| `0x8002b1d4 / 256` | `memory_card_initialize` | 100.000000% | 64 / 12 / 8 | Exact preserved |
| `0x8002b2d4 / 96` | `memory_card_shutdown_events` | 100.000000% | 24 / 5 / 4 | Exact preserved |
| `0x8002b334 / 56` | `memory_card_begin_status_check` | 100.000000% | 14 / 3 / 0 | Exact preserved |
| `0x8002b36c / 88` | `memory_card_clear_events` | 100.000000% | 22 / 4 / 4 | Exact preserved |
| `0x8002b3c4 / 188` | `memory_card_wait_event` | 100.000000% | 47 / 8 / 4 | Exact preserved |
| `0x8002b480 / 88` | `memory_card_undeliver_events` | 100.000000% | 22 / 4 / 4 | Exact preserved |
| `0x8002b4d8 / 248` | `memory_card_check_or_format` | 100.000000% | 62 / 10 / 1 | Exact preserved |
| `0x8002b5d0 / 120` | `memory_card_format` | 100.000000% | 30 / 3 / 1 | Exact preserved |
| `0x8002b648 / 244` | `save_system_write_slot` | 100.000000% | 61 / 10 / 1 | Exact preserved |
| `0x8002b73c / 1268` | `save_file_write_slot` | 100.000000% | 317 / 24 / 28 | Exact preserved |
| `0x8002bc30 / 216` | `save_system_read_header` | 100.000000% | 54 / 10 / 1 | Exact preserved |
| `0x8002bd08 / 220` | `save_file_read_header` | 100.000000% | 55 / 8 / 3 | Exact preserved |
| `0x8002bde4 / 204` | `save_system_read_slot` | 100.000000% | 51 / 6 / 1 | Exact preserved |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100.000000% | 243 / 13 / 15 | Exact preserved |
| `0x8002c27c / 104` | `save_workspace_allocate` | 100.000000% | 26 / 3 / 3 | Exact preserved |
| `0x8002c2e4 / 32` | `save_workspace_release` | 100.000000% | 8 / 1 / 0 | Exact preserved |
| `0x8002c304 / 524` | `save_file_initialize_buffers` | 100.000000% | 131 / 5 / 15 | Exact preserved |
| `0x8002c510 / 208` | `memory_card_show_status_message` | 97.980770% | 53 / 1 / 1 | Partial preserved; residue below |
| `0x8002c5e0 / 300` | `menu_load_message_image` | 100.000000% | 75 / 3 / 2 | Exact preserved |
| `0x8002c70c / 136` | `save_file_cleanup_temporary` | 100.000000% | 34 / 5 / 2 | Exact preserved |
| `0x8002c794 / 576` | `screen_show_image_until_input` | 100.000000% | 144 / 16 / 8 | Exact preserved |
| `0x8002c9d4 / 164` | `talk_show_dialogue_page` | 98.780490% | 41 / 1 / 7 | Partial preserved; residue below |

## Result and verification

The six internal producers and their status locals now use `KfSaveStatus`.
The shared domain retains its existing event/policy aliases. The dispatcher
uses `KF_ENUM_STORAGE(KfSaveStatus, s16)` to retain the observed narrowing in
both modern and legacy views. Four menu wrappers and temporary-file cleanup
encode status values explicitly into their separate integer result channels.
Five directory-search sentinels now use `SAVE_DIRECTORY_ENTRY_NOT_FOUND`.

All 27 reviewed functions retain their baseline: 25 exact and two partial.
Before/after comparison covered 2,352 instruction words, 303 ordered direct-call
references and 134 ordered address references. The 25 exact controls also
match 2,258 retail words and their delinked referents. No partial was promoted
or banked. The first divergences remain:

| Function | GAME VA | Reconstruction | Retail | Verdict |
| --- | --- | --- | --- | --- |
| `memory_card_show_status_message` | `8002c5c0` | `move a0,v0` | `ori v1,zero,1` | Existing instruction-order/register residue after the image-loader call. |
| `talk_show_dialogue_page` | `8002ca04` | `mfhi v1` | `mfhi v0` | Existing register residue in decimal path encoding. |

Call sets, CFG and ordered referents were preserved. These are unattributed
codegen residues; no compiler-backend mechanism has been established.
The message jump table remains a curated static model, not newly proved
indirect reachability. Its default path and signed narrowing are retained.

All 112 source/image variants were compiled twice from isolated source trees
based on `073ada0`, with only this campaign's three source/header files applied
to the after tree. Allocated section bytes and alignment, runtime symbols and
ordered relocations are identical in every pair. Only `game.save_system`
changes debug-line contents. Live objects match those controls except for
`open.opening_scenes`: the independent `dbecb1a` commit landed during this
campaign. A fresh compile of that committed source with the pinned profile
reproduces the live OPEN object. Of 484 observed strict scores, its
`OPEN.EXE 80014e28` alone changes, from 97.129630% to 97.952675%; all others
are unchanged. That improvement belongs to the separate OPEN campaign.

Compiler controls accept valid producer/storage/message flow and explicit
encoding, including signed-halfword narrowing. Eleven negative controls reject
raw integers or foreign enums in initialization, storage, assignment,
comparison and arguments, and reject implicit producer-result conversion to
an integer. These are ignored local probes using the actual headers and
extracted function declarations. No permanent tests or size assertions were
added. Whole-tree `kf check-types` retains the same 300 error diagnostics:
65/112 variants pass and 47 fail on existing debt.

Forced campaign compilation, strict comparisons, inventory checks, Ruff and
whitespace checks pass. All 683 repository tests pass (89.008 seconds).
Full `kf build` was run and still fails on existing data ownership/placement
and relink gaps: source-data matches PSX 0/1, GAME 9/42, OPEN 2/19; target
relinks PSX 1/1, GAME 75/77, OPEN 34/38, with six conflicting section bases
and zero artifact failures. Runtime equality establishes no regression from
this campaign; it does not make those outstanding checks pass.

The [save ledger](game-save-system-literal-ledger.md) now accounts for all
149 retained numeric/character occurrences, down from 154. Current expression
and duplicate-token validation covers 39 source files / 3,927 occurrences
across the wider project. The total source census is 6,635 occurrences;
it is not a count of missing names. Ten source lines still contain fourteen
`unknown_` tokens with unresolved identities, as recorded in the
[coverage map](source-literal-coverage.md). The overall naming goal remains open.
