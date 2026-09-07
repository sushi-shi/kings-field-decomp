# GAME displayed class titles and status-page literals

## Function Match Plan

Review the two status-text renderers and all their retained numeric literals.
Baseline `a2ccc06`; unrelated actor-damage work is present and remains outside
this campaign. Both units use `probe-gcc257-o2-g0`, a productive probe rather
than historical compiler attribution.

| GAME function | Extent | Initial strict match | Retail control snapshot |
| --- | --- | ---: | --- |
| `menu_draw_stats_header` | `0x80025f38/0x5a0` | 97.686110% | 360 words, 28 calls, nine conditional branches, two direct jumps, one return; 72-byte frame. |
| `menu_draw_status_details` | `0x800264d8/0xcb4` | 95.110700% | 813 words, 72 calls, 16 conditional branches, two direct jumps, one return; 80-byte frame. |

Hash-identical retail files and all six semantic views were refreshed before
editing. `menu_root` calls the header at `0x8002238c`, `0x800223ec`, and
`0x800225b4`; `menu_status_panel` calls details at `0x80024350`. All four
sites supply no arguments and ignore the result. The two functions are
adjacent, preceded by configuration-panel code and followed by
`menu_draw_name_list` at `0x8002718c`. The shared formatter and glyph renderer,
their typed assets and source histories were inspected. Earlier changes
named glyph flags, recovered menu ownership, and identified player statuses
and combat fields; they left the local `color` interpretation in place.

These bodies draw game-specific player fields and menu labels; neither has
vendored attribution. Their only callees are the shared game glyph/number
renderers and number formatter. No literal-string references occur: labels
are halfword atlas codes. References to `player_state`, `menu_assets`,
`display_state`, and `current_poly_ft4` retain their identities and ordered
relocations. Candidate internal-jump relocations in the details renderer
remain candidates; naming does not promote their evidence level.

First hypothesis: the computed four-glyph run is the displayed class title,
as proved by decoding the shipped atlas. Name its shared stat thresholds,
magic-tier stride, glyph-run length and atlas origin; rename `color` to
`class_glyph_offset` in both consumers. Keep the existing comparisons,
arithmetic order, signed word local, halfword output and repeated stores.
No new class field, lookup table, helper call or stored enum is proposed.
Rebuild both units, compare complete sections, raw linked words and ordered
referents against the saved objects, check every strict score, then run
modern Clang, lint, existing repository tests and the full build.

## Class selection evidence

GAME `0x80026188..0x80026204` and `0x800267d0..0x8002684c` implement the same
selection. `lhu` reads base magic at `player_state+0x24` and base physical
power at `+0x22`. Unsigned comparisons against 40 and 60 choose three tiers
per stat. Magic contributes 0, 1 or 2; physical power adds 0, 3 or 6.
The resulting index is multiplied by four, then 256, 257, 258 and 259 are
added and stored as the four halfword glyphs. The title is followed by the
existing signed -1 terminator. These are base stats, not equipment-adjusted
physical power or magic, and the operation only chooses display glyphs.

The atlas decoder follows the
[retail menu glyph format](game-menu-glyph-render.md#retail-glyph-format-and-asset-evidence).
`KF/COM/MIX.TIM` is 331488 bytes, SHA-256
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
The selected 16-color palette starts at byte 231572; its 32768-byte texture
payload starts at 232096. Each glyph is a 14-by-12 cell in 16 columns.
Codes `0x100..0x123` form nine consecutive four-cell runs, decoded directly
from those bytes. The English text and color are baked into the texture;
the old local `color` does not set a GPU color or palette field.

| Base physical power | Base magic 0–39 | Base magic 40–59 | Base magic 60+ |
| --- | --- | --- | --- |
| 0–39 | SOLDIER, white (`0x100`) | MAGE, white (`0x104`) | WIZARD, red (`0x108`) |
| 40–59 | HIGHSOLDIER, white (`0x10c`) | MAGEFIGHTER, white (`0x110`) | WIZARD, red (`0x114`) |
| 60+ | SWORDMAN, yellow (`0x118`) | SWORDMAN, yellow (`0x11c`) | SWORDMASTER, blue (`0x120`) |

The table preserves the asset's English spellings. Repeated visible titles
still have separate four-cell origins; do not merge their indices or infer
that all nine combinations are distinct gameplay classes. The original
design rationale for thresholds 40 and 60 is unknown.

| Shared constant | Value | Contract |
| --- | ---: | --- |
| `MENU_CLASS_MIDDLE_STAT_MIN` | 40 | Inclusive start of the middle base-stat tier. The physical comparison retains `> (minimum - 1)`. |
| `MENU_CLASS_HIGH_STAT_MIN` | 60 | Inclusive start of the high base-stat tier. |
| `MENU_CLASS_MAGIC_TIER_COUNT` | 3 | Three magic columns per physical tier in the title atlas. |
| `MENU_CLASS_LABEL_GLYPHS` | 4 | Four atlas cells per title, regardless of its English character count. |
| `MENU_CLASS_FIRST_GLYPH` | `0x100` | First cell of the nine title runs. |

The local remains an integer because it accumulates a flattened table index
and then becomes a glyph offset. A stored state enum would not describe
those two arithmetic stages. Tier ordinals and positions within a glyph
run remain ordinary small integers, documented in the linked literal ledger.

## Retained literals and verification

The [complete status-text ledger](game-menu-status-literal-ledger.md) accounts
for all 413 remaining numeric occurrences: 130 in the main-menu header and
283 in the detailed page. It distinguishes atlas text from status masks,
four-cell title indices from four-digit HP/MP widths, seven-pixel number
advances from the defense-score divisor seven, and authored row pitches
from font-cell dimensions. The status-page outer panel and other menu
rendering modules are outside this two-file ledger.

Both units were forcibly recompiled. All 357 source words in the header
and all 804 in details are unchanged, including 28/72 ordered call targets
and 36/89 ordered HI16/LO16 referents. Their strict scores remain
97.686110% and 95.110700%; neither is claimed exact. Existing instruction
and relocation residues remain unchanged. The details object is completely
byte-identical; the header object differs only in `.debug_line`.

All 112 objects and 484 scored functions were compared with the saved
snapshot. The only other object change is concurrent actor radial-damage
work, whose score moved from 86.103170% to 88.817460% during these checks.
Compiling the saved pre-edit actor source independently under the new menu
header preserves all code, data, runtime symbols and their relocations;
only debug/file metadata differs. That control separates the shared-header
change from the other campaign. No banking belongs to this naming work.

Both changed units pass modern Clang checking. Across all 112 variants,
64 pass and 48 retain the same multiset of 320 pre-existing errors. All
665 repository tests pass (87.965 seconds), along with inventory validation,
Ruff and `git diff --check`. Full `kf build` retains existing data/ownership
failures: PSX data 0/1, GAME 9/42, OPEN 2/19; target relink 1/1, 75/77,
34/38 respectively, six conflicting section bases and zero artifact failures.
