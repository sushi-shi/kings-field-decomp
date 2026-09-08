# Shared menu points, glyph rows and list titles

## Function Match Plan at `442f7d3`

GAME's menu blitters consume the first four bytes of each positioned glyph
label as a screen point. The list renderer consumes the first 24 bytes of
`KfMenuList` as the same positioned title. Eight equipment-name copies treat
the twenty code bytes as complete `MenuGlyphRow` objects. These relationships
are explicit in the call arguments, 4/24/20-byte offsets and strides, loaded
STAT.DAT records and the complete-object copies in retail.

Model a label as `MenuPoint position` plus `MenuGlyphRow glyphs`, and a list as
`MenuGlyphString title` followed by its existing list-specific fields. Use
member addresses for the blitters and list title, and whole glyph-row
assignment for the eight equipment names. Propagate the same declaration to
every consumer, retaining all extents, alignments, APIs and physical referents.

The [blitter type controls](game-menu-sprite-blit.md) support signed screen
coordinates and signed SDK dimensions; do not undo that evidence. Share
`MenuPoint`'s signed halfwords with positioned text. Retail's title guard at
`80028960` uses `lh`, while narrowed coordinate arithmetic still uses `lhu`.
The existing unsigned text declaration is not independent proof of a distinct
object type. Recheck all changed scalar loads/arithmetic against retail; retain
no per-consumer cast or incompatible view solely to rescue a score.

All twenty functions below have refreshed image-qualified disassembly/CFG,
callers, callees, strings, identity and strict-match snapshots. The existing
source history, by-value configuration ABI and SDK provider dossiers were
reviewed. These routines implement game UI policy; PadRead, GPU and GTE
providers remain vendored. There are no strings or outgoing candidate
references in these bodies. Every function has one return; its delay slot is
included. Preserve the complete calls, branches, loop structure, literal glyph
codes, atlas coordinates, enum domains and ordering-table depths.

The two by-value configuration labels must remain 24 bytes and aligned to two
bytes: retail homes a0-a3 then accesses the remaining caller-stack words,
passing labels at frame offsets 32 and 56 and the values pointer at 80.
The row-copy source and destination retain two-byte alignment, including
retail's `lwl/lwr` and `swl/swr` sequences. Do not change them to guessed word
alignment or replace the whole-record copies with a different copy policy.

The complete selected units and their siblings will be freshly compiled and
compared to saved baseline objects and retail. Every banked function must stay
strictly exact. New residues will be compared from the first raw difference;
full build, existing layout/ownership controls, lint and tests precede banking.

| GAME address / bytes | Function | Baseline % | Frame / blocks / branches |
| --- | --- | ---: | --- |
| `80021ffc / 696` | `item_pickup_confirm` | 100 | 96 / 27 / 15 |
| `8002589c / 1284` | `menu_config_panel` | 100 | 168 / 42 / 23 |
| `80025da0 / 408` | `menu_config_panel_draw` | 100 | 32 / 6 / 2 |
| `80025f38 / 1440` | `menu_draw_stats_header` | 97.991670 | 72 / 19 / 9 |
| `800264d8 / 3252` | `menu_draw_status_details` | 98.431730 | 80 / 31 / 16 |
| `8002718c / 2104` | `menu_draw_name_list` | 100 | 48 / 17 / 8 |
| `800279c4 / 440` | `menu_item_model_preview` | 98.181816 | 152 / 5 / 2 |
| `80027b7c / 732` | `menu_draw_item_detail` | 98.579230 | 160 / 7 / 3 |
| `80027ee4 / 1180` | `menu_draw_dialog_frame` | 100 | 80 / 14 / 7 |
| `80028380 / 852` | `menu_list_interact` | 100 | 104 / 54 / 28 |
| `800286d4 / 576` | `menu_two_option_prompt` | 100 | 112 / 25 / 13 |
| `80028914 / 348` | `menu_draw_window` | 87.540230 | 48 / 14 / 7 |
| `80028a70 / 1916` | `menu_list_render` | 100 | 96 / 22 / 13 |
| `800291ec / 268` | `menu_draw_two_option` | 100 | 40 / 10 / 3 |
| `800292f8 / 1976` | `menu_draw_item_name_frame` | 95.376520 | 224 / 3 / 1 |
| `80029ab0 / 416` | `menu_blit_sprite_translucent` | 100 | 32 / 1 / 0 |
| `80029c50 / 400` | `menu_blit_sprite` | 100 | 32 / 1 / 0 |
| `80029de0 / 1328` | `menu_draw_string` | 100 | 48 / 24 / 12 |
| `8002a310 / 512` | `menu_draw_number` | 100 | 40 / 4 / 2 |
| `8002ad6c / 140` | `menu_list_init` | 100 | 0 / 3 / 1 |

## Per-function result

All 36 functions in the selected units retain every linked instruction and
ordered call/data reference from the baseline; 28 remain retail-exact.
The original twenty consumers have these strict final verdicts:

| Function | After % | Verdict |
| --- | ---: | --- |
| `item_pickup_confirm` | 100.000000 | Exact preserved |
| `menu_config_panel` | 100.000000 | Exact preserved |
| `menu_config_panel_draw` | 100.000000 | Exact preserved |
| `menu_draw_stats_header` | 97.991670 | Open, unchanged |
| `menu_draw_status_details` | 98.431730 | Open, unchanged |
| `menu_draw_name_list` | 100.000000 | Exact preserved |
| `menu_item_model_preview` | 98.181816 | Open, unchanged |
| `menu_draw_item_detail` | 98.579230 | Open, unchanged |
| `menu_draw_dialog_frame` | 100.000000 | Exact preserved |
| `menu_list_interact` | 100.000000 | Exact preserved |
| `menu_two_option_prompt` | 100.000000 | Exact preserved |
| `menu_draw_window` | 87.540230 | Open, unchanged |
| `menu_list_render` | 100.000000 | Exact preserved |
| `menu_draw_two_option` | 100.000000 | Exact preserved |
| `menu_draw_item_name_frame` | 95.376520 | Open, unchanged |
| `menu_blit_sprite_translucent` | 100.000000 | Exact preserved |
| `menu_blit_sprite` | 100.000000 | Exact preserved |
| `menu_draw_string` | 100.000000 | Exact preserved |
| `menu_draw_number` | 100.000000 | Exact preserved |
| `menu_list_init` | 100.000000 | Exact preserved |

The source removes all 25 menu prefix/row pointer casts and the redundant
signed-title scalar cast. New layout checks cover the actual point, row, label
and list extents, including the untouched pointer fields at offsets 32/36.
The fresh full AST census has 841 written casts (606 pointer targets and 235
integer targets), including 591 pointer casts in C files. Compared with the
initial campaign census, 215 C-file pointer casts are gone. The increased
header cast count comes solely from the new constant layout assertions.

All 713 repository tests pass with nine skips, including all 90 menu list
initializer combinations, the six resource-bank extents and relocation
controls, and the complete graphics ownership compilation controls. Ruff and
whitespace checks pass. The full build recompiles 62 affected GAME units and
preserves all 439 exact functions; it still reports the existing data,
reference-closure and target-placement failures in all three images. The
final title-cast removal was freshly matched and raw-compared, followed by
another full build. No source profile, SDK definition or retail byte changed.
