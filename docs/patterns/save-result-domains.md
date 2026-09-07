# Save results and formatting confirmation

## Function Match Plan

At `7d21365`, keep internal `KfSaveStatus` separate from the menu-facing
`KfSaveResult`, temporary-file `KfSaveCleanupResult`, and the signed-halfword
format-confirmation argument. The current save wrapper and cleanup contracts
are integer channels, but their known values are not interchangeable.

The menu wrappers explicitly map selected statuses and otherwise preserve
the encoded value. For example GAME `8002b57c/8002b580` keeps the default
status in the return register when it lies beyond the format switch table;
the mapped arms at `8002b5a8/8002b5b0/8002b5b4` yield 0/1/2. Retain that
open set of encoded results, including read-failed 13. No conversion validates
or normalizes an unlisted value. The header/catalogue wrapper accepts no-data
as an empty catalogue, so result OK does not prove an existing file was read.

Cleanup has another contract: GAME `8002c724` returns error 4 when the BIOS
request does not start; `8002c780` forwards an unhandled card event. Events
1/3 run open, close and erase. The delay slots at `8002c774/8002c77c` compute
`file != -1`, independently of close/erase results. Name temporary-open
failure/success 0/1 and card timeout/error 2/4 without inventing removal success.

The save panel reuses an integer for both channels. Its cleanup value is
tested immediately. A typed union preserves that evidenced shared storage
while keeping the operation and cleanup interpretations distinct. Each read
uses the member written by its corresponding operation.
The catalogue checks at `80025104/80025110` accept one or three. The current
catalogue producer maps internal three to one; preserve the extra encoded
three comparison without calling it a reachable no-space catalogue result.

Format confirmation is consulted only on the I/O-end path. Zero requests
confirmation; any nonzero permits formatting. New-device handling still
refreshes and formats regardless of that argument. The enum names describe
confirmation state, not a universal prohibition on formatting.

Hash-identical retail and six image-qualified semantic views were captured
for every function below, together with current source, history, caller/callee
evidence, strings, adjacent claims and match state. The save path literals,
file layouts and menu policies establish game ownership; BIOS/file/GPU/SDK
bodies remain external. Preserve the retries, partial catalogue clear, prior
slot handling, default branches, complete data referents and delay slots.

| GAME VA / bytes | Function | Initial strict % | Source hypothesis |
|---|---|---:|---|
| `800146b8 / 0x2e4` | `game_main_loop` | 100.0 | Consume the cleanup timeout as its own result domain. |
| `8001499c / 0x38` | `game_shutdown` | 100.0 | Unchanged contiguous-module control. |
| `800149d4 / 0x20` | `frame_pacer_vsync_callback` | 100.0 | Unchanged contiguous-module control. |
| `800149f4 / 0x70` | `frame_pacer_wait` | 100.0 | Unchanged contiguous-module control. |
| `800249a8 / 0x4bc` | `menu_drop_item` | 99.54785 | Unchanged contiguous-module control. |
| `80024e64 / 0x260` | `menu_save_load_hub` | 100.0 | Unchanged contiguous-module control. |
| `800250c4 / 0x468` | `menu_save_panel` | 100.0 | Keep the shared storage as a typed operation/cleanup union; preserve the extra encoded catalogue acceptance at three. |
| `8002552c / 0x370` | `menu_load_panel` | 100.0 | Compare catalogue/slot results with the named operation OK value. |
| `8002b078 / 0xd8` | `save_system_read_catalog` | 100.0 | Propagate the wrapper result through its local, OK guard and return. |
| `8002b150 / 0x84` | `menu_play_input_sound` | 100.0 | Unchanged contiguous-module control. |
| `8002b1d4 / 0x100` | `memory_card_initialize` | 100.0 | Unchanged contiguous-module control. |
| `8002b2d4 / 0x60` | `memory_card_shutdown_events` | 100.0 | Unchanged contiguous-module control. |
| `8002b334 / 0x38` | `memory_card_begin_status_check` | 100.0 | Unchanged contiguous-module control. |
| `8002b36c / 0x58` | `memory_card_clear_events` | 100.0 | Unchanged contiguous-module control. |
| `8002b3c4 / 0xbc` | `memory_card_wait_event` | 100.0 | Unchanged contiguous-module control. |
| `8002b480 / 0x58` | `memory_card_undeliver_events` | 100.0 | Unchanged contiguous-module control. |
| `8002b4d8 / 0xf8` | `memory_card_check_or_format` | 100.0 | Type the halfword confirmation input and mapped result; preserve nonzero permission and default status pass-through. |
| `8002b5d0 / 0x78` | `memory_card_format` | 100.0 | Unchanged contiguous-module control. |
| `8002b648 / 0xf4` | `save_system_write_slot` | 100.0 | Name the 0/1/2/3 menu result mapping and retain unlisted encoded status values. |
| `8002b73c / 0x4f4` | `save_file_write_slot` | 100.0 | Unchanged contiguous-module control. |
| `8002bc30 / 0xd8` | `save_system_read_header` | 100.0 | Name failed/OK mapping; missing data and new-device still produce the accepted empty-catalogue result. |
| `8002bd08 / 0xdc` | `save_file_read_header` | 100.0 | Unchanged contiguous-module control. |
| `8002bde4 / 0xcc` | `save_system_read_slot` | 100.0 | Name failed/OK mapping and the new-card early failure; retain other encoded results. |
| `8002beb0 / 0x3cc` | `save_file_read_slot` | 100.0 | Unchanged contiguous-module control. |
| `8002c27c / 0x68` | `save_workspace_allocate` | 100.0 | Unchanged contiguous-module control. |
| `8002c2e4 / 0x20` | `save_workspace_release` | 100.0 | Unchanged contiguous-module control. |
| `8002c304 / 0x20c` | `save_file_initialize_buffers` | 100.0 | Unchanged contiguous-module control. |
| `8002c510 / 0xd0` | `memory_card_show_status_message` | 97.98077 | Unchanged contiguous-module control. |
| `8002c5e0 / 0x12c` | `menu_load_message_image` | 100.0 | Unchanged contiguous-module control. |
| `8002c70c / 0x88` | `save_file_cleanup_temporary` | 100.0 | Return a distinct result domain for temporary-open Boolean and forwarded card timeout/error; never imply erase succeeded. |
| `8002c794 / 0x240` | `screen_show_image_until_input` | 100.0 | Unchanged contiguous-module control. |
| `8002c9d4 / 0xa4` | `talk_show_dialogue_page` | 98.78049 | Unchanged contiguous-module control. |

## Shared result storage

The first experiment compared cleanup directly at the conditional. Although
the control flow was equivalent for the admitted cursor states, it removed
the assignment into the shared result word. `menu_save_panel` fell from 100%
to 99.343970%, and its complete-word integration control failed. Discard that
source form. Retail saves cleanup's return into s4 at `80025204` before the
exact-one comparison at `80025208`; this is the same result storage used by
the catalogue and write/format operations.

The final local union models that storage with `KfSaveResult operation` and
`KfSaveCleanupResult cleanup`. A forced compile restores the entire unit's
runtime sections, symbols and relocations to the baseline. This is an observed
result for the pinned probe, not an optimizer attribution. The member types
prevent mixing cleanup and operation enums in assignments or comparisons.
After a cancelled format prompt the cleanup member can remain active, but no
operation-member comparison occurs until a write or format result replaces
it. The locally maintained cursor stays within the save window's rows; the
return row bypasses the operation branch. No runtime tag or extra behavior is
introduced, and this untagged union does not independently enforce that control
flow in the modern checker.

## Result mappings and limits

`KfSaveResult` names the wrapper's failed (0), OK (1), format-required or
format-confirmation (2), no-space (3), and preserved read-failure (13) results.
The two format names describe different producers with the same encoding;
no caller behavior is consolidated. The internal status input remains a
separate `KfSaveStatus` throughout each switch.

| Wrapper | Internal statuses mapped to failed 0 | Other explicit mappings | Default |
|---|---|---|---|
| `memory_card_check_or_format` | 0, 2, 3, 4, 9, 10, 11 | 1 → OK; 8 → format confirmation 2 | Preserve encoded status |
| `save_system_write_slot` | 0, 2, 4, 14 | 1 → OK; 3 → format required 2; 5 → no space 3 | Preserve encoded status |
| `save_system_read_header` | 0, 2, 4 | 1, 3, 7 → OK | Preserve encoded status |
| `save_system_read_slot` | 0, 2, 3, 4, 7, 12 | 1 → OK; new-device early path also returns failed | Preserve encoded status |

`save_system_read_catalog` forwards the header result and populates summaries
only for OK. Explicit encode/decode pairs at the default mapping preserve the
existing numeric protocol; they do not turn every internal status into a
universally equivalent menu result. Codes 9/10 in the internal switch still
lack distinct established producers. The save panel's extra encoded catalogue
three stays explicit because this producer does not establish its original
meaning; naming it as no-space would misdescribe that acceptance test.

`KfSaveCleanupResult` names temporary-open failure/success (0/1), card timeout
(2), and card error (4). The descriptor comparison remains `file != -1`,
converted explicitly to the cleanup domain. The main loop tests cleanup's
timeout member; the save panel tests its temporary-opened member. No caller
compares these with `KfSaveResult` or the internal `KfSaveStatus` constants.
`KfCardFormatConfirmation` preserves the s16 parameter and the zero/nonzero
policy, while its call site supplies the named confirmed value.

Twelve ignored Clang controls accept three valid API/union/pass-through
chains and reject nine raw or wrong-domain assignments, arguments and
comparisons. Assigning cleanup into the operation union member fails, as does
comparing cleanup timeout with the numerically equal format-required result.
No production tests or size assertions are added. The untagged union retains
the active-member limitation described above.

## Final verification

Fourteen save-system literals and six save-menu literals become names. The
[save ledger](game-save-system-literal-ledger.md) now covers all 135 retained
occurrences; the [root/list/save ledger](menu-root-list-save-literal-ledger.md)
covers 259. All token/expression multiplicities match their current sources.
The global census is 6,428 occurrences across 111 C files; verified ledgers
cover 82 files and 4,961 occurrences. The remaining ten source lines containing
fourteen `unknown_` tokens have not acquired new semantic evidence here.

All three affected units were forced to rebuild. Isolated before/after builds
of all 112 variants preserve allocated sections, runtime symbols and ordered
relocations; only the save-menu debug line section changes. The 32 functions
retain 2,857 candidate words, 350 calls and 144 address references. Twenty-nine
exact functions reproduce 2,460 complete retail words and the delinked target
words, calls and address references.

| GAME VA | Function | Final strict % | Words / calls / references | Verdict |
|---|---|---:|---:|---|
| `800146b8` | `game_main_loop` | 100.0 | 185 / 43 / 20 | Exact retained |
| `8001499c` | `game_shutdown` | 100.0 | 14 / 4 / 0 | Exact retained |
| `800149d4` | `frame_pacer_vsync_callback` | 100.0 | 8 / 0 / 2 | Exact retained |
| `800149f4` | `frame_pacer_wait` | 100.0 | 28 / 3 / 3 | Exact retained |
| `800249a8` | `menu_drop_item` | 99.54785 | 303 / 20 / 3 | Unchanged partial |
| `80024e64` | `menu_save_load_hub` | 100.0 | 152 / 20 / 2 | Exact retained |
| `800250c4` | `menu_save_panel` | 100.0 | 282 / 58 / 0 | Exact retained |
| `8002552c` | `menu_load_panel` | 100.0 | 220 / 42 / 0 | Exact retained |
| `8002b078` | `save_system_read_catalog` | 100.0 | 54 / 2 / 1 | Exact retained |
| `8002b150` | `menu_play_input_sound` | 100.0 | 33 / 3 / 0 | Exact retained |
| `8002b1d4` | `memory_card_initialize` | 100.0 | 64 / 12 / 8 | Exact retained |
| `8002b2d4` | `memory_card_shutdown_events` | 100.0 | 24 / 5 / 4 | Exact retained |
| `8002b334` | `memory_card_begin_status_check` | 100.0 | 14 / 3 / 0 | Exact retained |
| `8002b36c` | `memory_card_clear_events` | 100.0 | 22 / 4 / 4 | Exact retained |
| `8002b3c4` | `memory_card_wait_event` | 100.0 | 47 / 8 / 4 | Exact retained |
| `8002b480` | `memory_card_undeliver_events` | 100.0 | 22 / 4 / 4 | Exact retained |
| `8002b4d8` | `memory_card_check_or_format` | 100.0 | 62 / 10 / 1 | Exact retained |
| `8002b5d0` | `memory_card_format` | 100.0 | 30 / 3 / 1 | Exact retained |
| `8002b648` | `save_system_write_slot` | 100.0 | 61 / 10 / 1 | Exact retained |
| `8002b73c` | `save_file_write_slot` | 100.0 | 317 / 24 / 28 | Exact retained |
| `8002bc30` | `save_system_read_header` | 100.0 | 54 / 10 / 1 | Exact retained |
| `8002bd08` | `save_file_read_header` | 100.0 | 55 / 8 / 3 | Exact retained |
| `8002bde4` | `save_system_read_slot` | 100.0 | 51 / 6 / 1 | Exact retained |
| `8002beb0` | `save_file_read_slot` | 100.0 | 243 / 13 / 15 | Exact retained |
| `8002c27c` | `save_workspace_allocate` | 100.0 | 26 / 3 / 3 | Exact retained |
| `8002c2e4` | `save_workspace_release` | 100.0 | 8 / 1 / 0 | Exact retained |
| `8002c304` | `save_file_initialize_buffers` | 100.0 | 131 / 5 / 15 | Exact retained |
| `8002c510` | `memory_card_show_status_message` | 97.98077 | 53 / 1 / 1 | Unchanged partial |
| `8002c5e0` | `menu_load_message_image` | 100.0 | 75 / 3 / 2 | Exact retained |
| `8002c70c` | `save_file_cleanup_temporary` | 100.0 | 34 / 5 / 2 | Exact retained |
| `8002c794` | `screen_show_image_until_input` | 100.0 | 144 / 16 / 8 | Exact retained |
| `8002c9d4` | `talk_show_dialogue_page` | 98.78049 | 41 / 1 / 7 | Unchanged partial |

The three partials retain their first raw divergence:

| GAME VA | Candidate | Retail |
|---|---|---|
| `800249cc` | `move $s2, $zero` | `move $s3, $zero` |
| `8002c5c0` | `move $a0, $v0` | `ori $v1, $zero, 1` |
| `8002ca04` | `mfhi $v1` | `mfhi $v0` |

All selected scores are unchanged; no function is newly closed or banked by
this work. Concurrent GAME `80018540` work independently improves from
93.298965% to 100%; its source, evidence and baseline updates are excluded.
Inventory, Ruff, whitespace and all 684 repository tests pass (119.103
seconds), including the restored full save-panel word/call control. Modern
checking remains at 300 errors and 65/112 passing variants. Full `kf build`
still reports the existing data/ownership differences: source data PSX 0/1,
GAME 9/42, OPEN 3/19; target relink PSX 1/1, GAME 75/77, OPEN 34/38, with six
conflicting section bases and zero data artifact failures.
