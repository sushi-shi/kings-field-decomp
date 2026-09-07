# Window renderer and list-initializer literals

Complete ledger for `menu_draw_window` and `menu_list_init`: all **15**
remaining numeric occurrences have specific reasons. Retail address/extent
claims and named constant definitions are excluded. Other functions in
`menu_runtime.c` are outside this ledger. See the
[window-kind review](game-menu-window-kinds.md) for evidence and verification.

The [shared-dimension review](game-item-menu-dimensions.md) names the database
extents and glyph-row width while preserving local workspace capacities.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `menu_draw_window` | 21 | `0` | `if ((s16)layout->title.x != 0) {` | A zero title X coordinate suppresses the title. Preserve the signed-halfword interpretation and coordinate-based guard; this is not a glyph terminator test. |
| `menu_draw_window` | 26 | `0` | `if (count > 0) {` | Positive row count enters the drawing loop; zero and negative counts draw no selectable rows. |
| `menu_draw_window` | 27 | `0` | `row = 0;` | Start at the first zero-based row in the selected layout. |
| `menu_draw_window` | 32 | `1` | `if (row == highlight && flag == 1) {` | Confirmation highlight uses the exact Boolean value one in addition to a matching row; preserve equality rather than accepting every nonzero flag. |
| `menu_list_init` | 561 | `12` | `list->title_x = 12;` | Authored list-title X origin, in screen pixels, also used by the loaded titled window records; no recovered rationale chooses twelve. |
| `menu_list_init` | 562 | `19` | `list->title_y = 19;` | Authored list-title Y origin, in screen pixels, matching the loaded title baseline; no recovered rationale chooses nineteen. |
| `menu_list_init` | 563 | `0` | `for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {` | Copy positions zero through nine of the ten-halfword label representation into the equally sized title buffer, including any terminator and trailing words; do not turn this into a terminator-controlled copy. |
| `menu_list_init` | 566 | `0x16` | `list->list_x = 0x16;` | Authored list-panel X origin, 22 screen pixels. The renderer adds its three-pixel text inset independently. |
| `menu_list_init` | 567 | `0x26` | `list->list_y = 0x26;` | Authored list-panel Y origin, 38 screen pixels. The renderer adds its three-pixel inset and twelve-pixel row offsets. |
| `menu_list_init` | 568 | `0` | `list->entry_count = 0;` | The list initially has no entries; each caller fills the count after constructing its rows. |
| `menu_list_init` | 569 | `0x0b` | `list->visible_rows = 0x0b;` | Default visible-row count eleven gives a 132-pixel sequence at the renderer's twelve-pixel pitch. This is layout configuration, not the item-buffer capacity; the original choice of eleven is unknown. |
| `menu_list_init` | 570 | `0` | `list->scroll_offset = 0;` | Start the visible window at entry zero. |
| `menu_list_init` | 571 | `0` | `list->selected_index = 0;` | Initial selection is the first entry, with empty lists handled by the caller. |
| `menu_list_init` | 572 | `0` | `list->cursor_row = 0;` | Initial highlight is the top visible row. |
| `menu_list_init` | 573 | `8` | `list->glyphs_per_entry = 8;` | Default row stride is eight glyph halfwords. All seven confirmed callers overwrite it with ten before rendering; preserve this initializer behavior rather than pretending it is the common asset-name width. |
