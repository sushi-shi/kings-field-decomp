# GAME memory-card and save system

This is the current semantic model of the contiguous GAME.EXE family at
`0x8002b078..0x8002ca78`. It is a curated work-in-progress, not a claim that
the names or source-file boundaries are original. The supporting per-function
snapshots are in `config/evidence/game_semantic_save_system.tsv`.

## File and workspace layout

The main file name is `bu00:BISLPS-00017KF      ` and the temporary name is
`bu00:BISLPS-00017KFTMP   `. Both serial strings agree with the Japanese
SLPS-00017 release under investigation.

The routines bind two shared pointer variables to either a caller's 0x2800
stack workspace or a 0x2800 allocation from the game's LIFO arena:

```text
save_header_buffer  +0x0000  KfSaveHeader   (0x0280 bytes)
  PlayStation save header    (0x0200 bytes)
    header/title/CLUT         (0x0080 bytes)
    icon frame 1             (0x0080 bytes)
    icon frame 2             (0x0080 bytes)
    icon frame 3             (0x0080 bytes)
  King's Field directory     (0x0080 bytes)
    +0x200 slot_ids[4]        (4 bytes; observed 0, 1..3, or 4)
    +0x204 reserved           (0x1c bytes)
    +0x220 summaries[4]       (4 * 0x18 bytes)

save_payload_buffer +0x0280  KfSavePayload  (0x2580 bytes)
```

These file-layout types, the shared workspace declarations, and the public
cross-TU save API are owned by `include/kf/game_save.h`. Internal card and file
helpers remain declared inside `save_system.c`.

Each summary contains six named 32-bit fields: experience, current floor,
current HP, maximum HP, current MP and maximum MP. The writer independently
identifies each source field; the dialog's display loads and positive-current-HP
row predicate agree with those offsets. See the
[field evidence](patterns/semantic-field-names.md#save-summary-field-recovery).
The card header initializer writes `SC`, the three-frame icon type `0x13`,
block count `5`, the Shift-JIS title, and three icon assets (`ICO1.TIM`
through `ICO3.TIM`). `KfPsxSaveHeader` names the signature, type, block count,
title, zero padding, palette and frames, with measured size/offset checks.
See the [format and retail evidence](patterns/semantic-field-names.md#playstation-save-icon-header).

There are three logical slots (IDs 1..3) and four physical directory entries.
Tag zero denotes an empty entry, and tag four denotes the spare entry reused
before searching for an empty one. A completed payload write retags its old
entry as spare. The existing unguarded old-entry index is preserved even when
the search leaves it at -1. The allocation size is expressed as one header
plus one payload; their independent layout assertions still total 0x2800.

The payload is a serialization container, not a distinct permanent global.
The write/read pair proves these copied ranges and offsets:

| Payload range | GAME source/destination | Size | Current meaning |
| --- | --- | ---: | --- |
| `+0x0000` | `0x800a0780` | `0x00e0` | complete `player_state` copy |
| `+0x030c` | `0x8009ddb4` | `0x2134` | persisted map world state |
| `+0x2440` | `0x800652a8` | `0x00f0` | three item-stock banks |
| `+0x2530` | every `0x14` bytes from `0x8009ce60` | `0x18` sampled bytes | learned flag of each magic record |

The copied state owners now have curated identities. The unused payload
ranges at +0x0e0 and +0x2548 remain opaque; the read/write pair alone does
not establish their intended contents.

## Function layers

The family separates low-level BIOS event polling, raw file operations, and
menu-facing result mapping:

| Layer | Functions |
| --- | --- |
| Card service | `memory_card_initialize`, `memory_card_shutdown_events`, `memory_card_begin_status_check`, `memory_card_clear_events`, `memory_card_wait_event`, `memory_card_undeliver_events` |
| Card policy/UI | `memory_card_check_or_format`, `memory_card_format`, `memory_card_show_status_message` |
| Raw save file | `save_file_initialize_buffers`, `save_file_write_slot`, `save_file_read_header`, `save_file_read_slot`, `save_file_cleanup_temporary` |
| Menu-facing save API | `save_system_read_catalog`, `save_system_write_slot`, `save_system_read_header`, `save_system_read_slot` |
| Workspace | `save_workspace_allocate`, candidate `save_workspace_release` |
| Shared menu helpers | `menu_play_input_sound`, `menu_load_message_image` |

`save_file_write_slot` and `save_file_read_slot` are inverse operations over
the proved payload ranges. Each raw I/O loop makes at most five attempts,
including the first operation. The menu-facing wrappers refresh a newly
inserted card, display a numbered `TIM\\Mddd.` status image on failure, and
translate internal statuses to the smaller result sets expected by the
save/load menus.

`menu_play_input_sound` consumes the shared menu cue IDs; its program/note
recipes, bank-zero SDK argument packing, volume and wait behavior are
documented in the [audio selector audit](audio-system.md#packed-key-onkey-off-selectors).

`save_system_read_catalog` receives storage for three slot summaries. It scans
four on-card directory entries and indexes the output by one-based slot ID.
The retail function clears only the first `0x24` bytes before the scan even
though three `0x18` summaries occupy `0x48` bytes; this is recorded as an
observed retail behavior rather than silently normalized.

## Card events and status codes

The four event-handle globals are directly identified by the constants passed
to `OpenEvent(HwCARD, spec, EvMdNOINTR, 0)` in the pinned Psy-Q `KERNEL.H`:

| Global | Event spec | Poll result |
| --- | ---: | ---: |
| `memory_card_io_end_event` | `EvSpIOE` (`0x0004`) | `1` |
| `memory_card_timeout_event` | `EvSpTIMOUT` (`0x0100`) | `2` |
| `memory_card_new_device_event` | `EvSpNEW` (`0x2000`) | `3` |
| `memory_card_error_event` | `EvSpERROR` (`0x8000`) | `4` |

The card poller tests those events in table order and undelivers them after
selecting a result. `memory_card_begin_status_check` returns zero when
`_card_info` does not start an operation. The public `KF_CARD_STATUS_*`
constants describe that BIOS-event contract.

The file/UI layer reuses this number space. The shared `KfSaveStatus` domain
in `game_save.h` contains both card-event and `SAVE_STATUS_*` policy names,
with success also equal to one. Producers and status locals carry that type;
the message dispatcher retains its signed-halfword storage. The
[domain review](patterns/game-save-status-domain.md) records the conversions
and the [current ledger](patterns/game-save-system-literal-ledger.md) explains
every retained literal:

| Value | Source name suffix | Producer or observed handling |
| ---: | --- | --- |
| 1 | `OK` | Completed raw read/write; also card I/O-end. |
| 3 | `FORMAT_REQUIRED` | Main-file creation and a subsequent temporary-file creation both fail. Also the BIOS new-device event. |
| 5 | `NO_SPACE` | Main-file creation fails but temporary-file creation succeeds; the selected image reports insufficient space. |
| 6 | `FAILED` | Generic failure image; no distinct producer identified. |
| 7 | `NO_DATA` | Requested slot is absent or the save file cannot be opened. |
| 8 | `FORMAT_CONFIRMATION` | Existing-card I/O-end when `allow_format` is zero. |
| 11 | `FORMAT_FAILED` | All five format attempts fail. |
| 12 | `STALE_CATALOG` | On-card summaries differ from cached summaries, or a new-device event is reported before loading a slot. |
| 13 | `READ_FAILED` | All five attempts fail to read the complete requested range. |
| 14 | `WRITE_FAILED` | Write-open fails or all five attempts fail to write the complete requested range. |

These are game policies, not authoritative diagnoses of a BIOS failure.
In particular, an open failure does not prove absent data, and the create
probes do not prove every underlying failure is a formatting or space issue.
Status 12 protects the cached catalog; a new-device event alone does not
prove any saved bytes changed. Codes 9 and 10 both select the unusable-card
message, but no distinct producer identifies their separate meanings; retain
their numeric cases.

The menu-facing wrappers then translate status values again. All unlisted
values pass through unchanged:

| Wrapper | Status to result mapping |
| --- | --- |
| `memory_card_check_or_format` | 0, 2, 3, 4, 9, 10, 11 → 0; 1 → 1; 8 → 2 |
| `save_system_write_slot` | 0, 2, 4, 14 → 0; 1 → 1; 3 → 2; 5 → 3 |
| `save_system_read_header` | 0, 2, 4 → 0; 1, 3, 7 → 1 |
| `save_system_read_slot` | 0, 2, 3, 4, 7, 12 → 0; 1 → 1. A new-device event first refreshes the header, displays status 12 and returns 0. |

Thus a wrapper result of two is not universally a card timeout, and these
functions do not all return Booleans. Preserve their operation-specific
result values and the menu callers' comparison with one.

## Status message assets and attempt limits

Decoded retail `TIM/M101.` through `TIM/M115.` images identify the selected
messages. `menu_load_message_image` takes the decimal filename ID directly:

| Status | Image ID | Message meaning |
| --- | ---: | --- |
| 1 | -1 | Literal success-arm value; not the image loader's skip value. |
| 2 | 101 | Insert a memory card in slot 1. |
| 5 | 102 | Insufficient free space. |
| 12 | 103 | Do not remove the memory card. |
| 14 | 107 | Could not save. |
| 11 | 108 | Could not initialize the card. |
| 3 | 109 | Format the card. |
| 9, 10 | 110 | Card cannot be used; replace it. |
| 7 | 111 | No data. |
| 13 | 112 | Could not load. |
| 0, 4 | 113 | System error. |
| 6 | 114 | Failed. |
| 8 | 115 | Game data exists; confirm initialization. |

The loader skips only ID 255. Status one selects image ID -1, which still
enters filename construction; the usual wrappers display a status only when
it differs from one. Unknown statuses become signed-halfword image IDs.
Separately, the dispatcher returns -1 when the image loader reports failure
one and forwards every other loader result, including successful zero.
The image selection and function result are distinct contracts.

`menu_load_item_texture` uses a different contract: it adds one to its
argument before constructing the same filename family. Consequently menu
indices 103, 104, 105 and 114 select images M104 (loading), M105 (saving),
M106 (formatting) and M115 (confirmation). Direct message IDs must not be
substituted for those indices without the conversion.

`SAVE_FILE_IO_ATTEMPTS` and `CARD_FORMAT_ATTEMPTS` separately name the two
five-attempt policies. The code does not explain why five was chosen. Neither
count is derived from `SAVE_FILE_BLOCKS`: that independent allocation holds
the 640-byte header plus four 9600-byte payload positions in five 8192-byte
card blocks. Each file retry seeks back to the requested start using the
pinned SDK's `SEEK_SET`; it does not resume after a partial transfer.

## Ownership and vendor controls

The functions use Sony BIOS/Psy-Q APIs such as `_card_info`, `_new_card`,
`OpenEvent`, `open`, `read`, and `write`, but the surrounding retry policies,
King's Field paths, layout, serialization, and UI result maps are game code.
The named routines are absent from the curated vendored inventory and from
unique SDK FID matches.

The 0x20-byte `save_workspace_release` wrapper is the only weaker identity. Its
body is too generic and produces twelve incompatible FID candidates. It calls
the game's matching LIFO arena-pop helper and immediately follows the proved
allocation wrapper, so its signature is supported while its save ownership
remains a candidate pending a caller or translation-unit boundary.

## Remaining work

- Recover the remaining opaque fields and unused serialized payload ranges.
- Prove the translation-unit/object boundary and whether the shared workspace
  pointers and constant strings had external or file-local linkage.
- Name and group the compiler-generated status jump tables without treating
  each Ghidra-split pointer as an independent source global.
- Reconstruct the functions in C and require 100% objdiff before banking any
  new match; semantic identity does not imply byte equality.

## Reconstruction status

The band is reconstructed as the single module `src/game/save_system.c`
(unit `game.save_system`, 24 claimed functions, `0x8002b078..0x8002ca78`).
Twenty-three functions are strict 100%. `talk_show_dialogue_page` remains
98.780490%. The [status-message closure](patterns/game-status-result-traces.md)
recovers a non-void C function with only an explicit failure return; its
success path falls through and every retail caller discards the result.
The missing-return warning is retained, and no successful C return value
is promised. `KfSavePayload` names the four copied ranges by their established
state owners while retaining the measured word-copy alignment. Graphics
references use the shared display-state declarations. The summary-field
and workspace naming changes preserve every instruction and relocation.
