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
The card header initializer writes `SC`, icon flag `0x13`, block count `5`,
the Shift-JIS title, and three icon assets (`ICO1.TIM` through `ICO3.TIM`).

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
the proved payload ranges. Both raw I/O paths retry short reads/writes up to
five times. The menu-facing wrappers refresh a newly inserted card, display a
numbered `TIM\\Mddd.` status image on failure, and translate internal statuses
to the smaller result sets expected by the save/load menus.

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

Higher internal codes include missing/open failure (`7`), format failure
(`0x0b`), directory-summary mismatch (`0x0c`), read failure (`0x0d`), and
write failure (`0x0e`). Their menu translations differ by operation and remain
part of each wrapper rather than a single universal enum.

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
Twenty-two functions are strict 100%. The remaining two carry recorded
residues listed under "save" in
[`patterns/source-shapes-gcc257.md`](patterns/source-shapes-gcc257.md):
`memory_card_show_status_message` (97.980770%) and `talk_show_indexed_image`
(98.780490%). `KfSavePayload` names the four copied ranges by their established
state owners while retaining the measured word-copy alignment. Graphics
references use the shared display-state declarations. The summary-field
and workspace naming changes preserve every instruction and relocation.
