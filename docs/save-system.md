# GAME memory-card and save system

This is the current semantic model of the contiguous GAME.EXE family at
`0x8002b078..0x8002c794`. It is a curated work-in-progress, not a claim that
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

Each summary is six 32-bit fields. Their individual gameplay meanings are not
yet proved, so the type deliberately keeps them as `fields[6]`. The card header
initializer writes `SC`, icon flag `0x13`, block count `5`, the Shift-JIS title,
and three icon assets (`ICO1.TIM` through `ICO3.TIM`).

The payload is a serialization container, not a distinct permanent global.
The write/read pair proves these copied ranges and offsets:

| Payload range | GAME source/destination | Size | Current meaning |
| --- | --- | ---: | --- |
| `+0x0000` | `0x800a0780` | `0x00e0` | unresolved state block |
| `+0x030c` | `0x8009ddb4` | `0x2134` | unresolved state block |
| `+0x2440` | `0x800652a8` | `0x00f0` | unresolved state block |
| `+0x2530` | every `0x14` bytes from `0x8009ce60` | `0x18` sampled bytes | unresolved 24-entry field |

The global/static inventory names the workspace pointers but intentionally
leaves these four gameplay-state owners address-derived until their wider
xrefs establish real structures and field names.

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

- Recover semantic fields for the six-word slot summary and the serialized
  gameplay-state ranges.
- Prove the translation-unit/object boundary and whether the shared workspace
  pointers and constant strings had external or file-local linkage.
- Name and group the compiler-generated status jump tables without treating
  each Ghidra-split pointer as an independent source global.
- Reconstruct the functions in C and require 100% objdiff before banking any
  new match; semantic identity does not imply byte equality.

## Reconstruction status

The band is reconstructed as the single module `src/game/save_system.c`
(unit `game.save_system`, 24 claimed functions, `0x8002b078..0x8002ca78`).
Fourteen functions are strict 100%; the remaining ten carry recorded residues
listed under "save" in [`patterns/source-shapes-gcc257.md`](patterns/source-shapes-gcc257.md):
unit-local `.rdata` referents (jump tables, the card title, the message
template), the player and display state structs that retail addresses as one
object each, and two scheduling/allocation placements. `KfSavePayload` now
names the four copied ranges by their copy-loop alignment. The two reverse
ordering tables at `0x80070ebc` are addressed only relative to
`asset_load_buffer`, so they still lack the relocation evidence the identity
checker requires and remain a declaration inside the module.
