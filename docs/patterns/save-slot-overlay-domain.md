# Save-slot overlay selector

## Function Match Plan

Name and type the signed selector used by `menu_draw_dialog_frame`, preserving
its full signed-word representation and range behavior. Use `KfSaveSlotOverlay`
for the drawing parameter and the two-option prompt's forwarded local. Decode
the save/load cursor at drawing calls; its format/return rows and arithmetic
remain integer menu positions. This is a source naming pass with builds, tests,
compiler checks and post-edit matches deferred at the user's request.

All addresses below are in GAME.EXE. Cached retail disassembly/CFG, incoming
and outgoing references, strings, neighboring menu helpers, source history and
historical match dossiers were reviewed. The old exact results describe their
captured source versions, not the pending edits.

| Function | Address / extent | Evidence snapshot and intended change |
| --- | --- | --- |
| `menu_save_confirm` | `0x800222b4 / 0x94` | Frame 10264 bytes; the call at `0x800222e8` passes null rows and selector 3 in its delay slot. Name that selector; leave the independent three-frame loop count. |
| `menu_save_panel` | `0x800250c4 / 0x468` | Seven direct frame calls forward the signed cursor, including format/return rows. Decode at each call without changing the cursor or save-file slot arithmetic. |
| `menu_load_panel` | `0x8002552c / 0x370` | Five direct frame calls forward the signed cursor, including the return row. Decode at each call. |
| `menu_draw_dialog_frame` | `0x80027ee4 / 0x49c` | Frame 80 bytes; direct zero/one/two comparisons and signed `slti ...,3` at `0x80028000`/`0x80028060` select ordered overlay calls. Type/name the selector and its three equality values. |
| `menu_two_option_prompt` | `0x800286d4 / 0x240` | Frame 112 bytes; `0x80028718` sets the forwarded word to -1; `0x80028740` replaces it with the caller row only for save/load windows. Type/name the local and decode that row. |

The [original frame/packet review](game-menu-detail-constants.md) establishes
the three dark translucent regions from shipped `COM/STAT.DAT`: packet indices
2/3/4 cover the first/second/third save summaries. The selector decides which
overlays to draw, independently of the optional text-summary pointer.

| Name | Encoded value | Ordered packet indices |
| --- | ---: | --- |
| `KF_SAVE_OVERLAY_NONE` | -1 | None |
| `KF_SAVE_OVERLAY_SKIP_FIRST` | 0 | 3,4 |
| `KF_SAVE_OVERLAY_SKIP_SECOND` | 1 | 2,4 |
| `KF_SAVE_OVERLAY_SKIP_THIRD` | 2 | 2,3 |
| `KF_SAVE_OVERLAY_ALL` | `KF_SAVE_SLOT_COUNT` (3) | 2,3,4 |

The enum is not a range validator. Other negative values still draw no
overlays, and every value at least 3 still draws all three. In particular the
save-panel return cursor 4 remains 4 after decoding, rather than being clamped
to the named threshold. The existing signed comparison and independent `if`
blocks remain intact. This preserves the open-range behavior documented in the
earlier review while giving the consumer a distinct semantic type.

## Source result

All five planned functions have the same final verdict: source naming and type
propagation reviewed, compiler and post-edit match verification deferred.
The renderer parameter and prompt local use the signed enum; all fifteen
direct drawing calls now pass either the named value, typed local or decoded
cursor. Packet order, range comparisons, summary traversal and cursor values
are unchanged in source. The curated signature follows the source type and
parameter name without changing its evidence tier.

Five inline selector occurrences were named. The three affected ledgers now
account for 256 root/list/save occurrences, 82 detail/dialog occurrences and
26 prompt occurrences (the other confirmation-ledger modules are unchanged).
Source-wide accounting covers all 111 C files and 6,002 retained occurrences
with individual reasons. No builds, compiler checks, tests or post-edit matches
were run, and no byte-match or banking claim is made.
