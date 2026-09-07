# GAME menu drawing layers and passive-panel input

## Function Match Plan

Name the shared menu widget/window/background ordering-table depths, the
status/map panel input-release frame, and the map's image/marker depths and
cell-to-pixel scale. Use the existing display-buffer count for its two local
packet arrays. Audit all remaining literals in the status panel, map viewer,
and list renderer. Baseline `c05548a` includes unrelated actor radial-damage
and root README work, which is outside this campaign.

All six semantic views were refreshed against hash-validated GAME retail
bytes. The selected functions use `probe-gcc257-o2-g0`; this remains a probe.

| GAME function | VA / body bytes | Initial strict % | Retail words / calls / conditional branches / direct jumps |
| --- | --- | ---: | --- |
| `menu_map_viewer` | `80022d7c / 1024` | 100 | 256 / 17 / 9 / 1 |
| `menu_status_panel` | `8002430c / 1692` | 81.900710 | 423 / 20 / 5 / 1 |
| `menu_list_render` | `80028a70 / 1916` | 88.252610 | 479 / 14 / 13 / 0 |
| `menu_draw_item_name_frame` | `800292f8 / 1976` | 95.376520 | 494 / 23 / 1 / 0 |
| `menu_blit_sprite_translucent` | `80029ab0 / 416` | 88.269230 | 104 / 3 / 0 / 0 |
| `menu_blit_sprite` | `80029c50 / 400` | 87.8 | 100 / 2 / 0 / 0 |
| `menu_draw_window_backdrop` | `8002a510 / 1700` | 99.971760 | 425 / 16 / 0 / 0 |

Each has one return and preserves every transfer's following delay slot.
The map viewer's frame is 224 bytes and the status panel's is 112 bytes.
The sole map caller at `80022b28` passes its selected item code; the status
caller at `80022484` has no arguments. Five pickup calls pass their current
item ID; fifteen list calls pass the shared context pointer. Window/list/
configuration/confirmation callers constrain the two sprite-pointer inputs.
The backdrop has three callers: status, window drawing and list rendering.
All results are ignored. The reviewed signatures and widths stay unchanged.

The original runtime run owns its intervening helpers and string range;
adjacent two-option, text/number, primitive-commit and frame helpers were
inspected along with caller source and history. The only string in the
selected functions is the map path template. Existing references to
`menu_assets`, `display_state`, `current_poly_ft4`, player fields and item
preview data retain their identity, numeric target and relocation order.
No curated relocation or evidence-tier changes are proposed.

These are game menu policies, not vendored bodies. The called `AddPrim` and
`ClearOTagR` are separately attributed to Psy-Q Release 2.5 `LIBGPU.LIB`
members PRIM and SYS in the vendored inventory. They remain SDK calls.
First source hypothesis: substitute constant names only, without rebuilding
quads as loops, changing their insertion order, or rewriting input control.
Force affected compiles, compare every object/score and each selected body's
linked words and ordered referents, then run Clang, lint, tests and full build.

## Ordering-table contracts

`primitive_buffer_commit_poly_ft4` at `8002ad1c` shifts its word-sized depth
argument left two and adds the active ordering-table pointer before `AddPrim`.
Thus 2000 and 2900 are table indices, not world distances. Direct background
calls similarly add 12000 bytes for entry 3000. `menu_frame_begin` calls
`ClearOTagR`; `menu_present_frame` submits the last table entry. `AddPrim`
at `80054290` links the new packet before the bucket's existing chain, so
calls sharing a depth must retain their insertion order.

| Constant | Value | Consumers and role |
| --- | ---: | --- |
| `MENU_WIDGET_OT_DEPTH` | 2000 | Both sprite blitters and the list's backdrop, row highlights and end tile. |
| `MENU_WINDOW_OT_DEPTH` | 2900 | Four mirrored quads in the status panel, pickup frame and shared window backdrop. |
| `MENU_BACKGROUND_OT_DEPTH` | 3000 | The four persistent background quads in pickup, shared backdrop and map rendering. |
| `MENU_MAP_IMAGE_OT_DEPTH` | 1000 | Map-image packet only; the text renderer's equal-valued depth has a separate role. |
| `MENU_MAP_MARKER_OT_DEPTH` | 500 | Player marker packet, ahead of the map image in the reverse ordering table. |

The shared names expose the layer contract across all consumers. The chosen
numeric gaps are authored; no evidence derives 2900 from a physical depth
or proves why these particular bucket numbers were selected. Other menu
image/text layers at 500/1000 retain their distinct existing audits.

## Passive-panel input and map coordinates

`MENU_PANEL_INPUT_RELEASE_FRAME = 2` names the zero-based frame at which both
passive panels wait for all buttons to be released. GAME `80024930..80024958`
and `8002310c..80023134` both compare the counter with 2, poll `PadRead(1)`
until zero, then increment it. Counter values 0 and 1 only advance; value 2
performs the release wait after the third draw. The first dismissal check
occurs after the fourth draw at counter 3. The release loop does not render
or call VSync, and holding a button can prolong it. The status panel plays
the cancel cue on dismissal; the map viewer does not.

`MENU_MAP_PIXELS_PER_CELL = 2` describes each marker coordinate's scale:
X increases with map-cell X and screen Y decreases with map-cell Z. The
literal endpoint origins (58/62, 216/220) place a four-pixel-span marker
around `(60 + 2*x, 218 - 2*z)`. `MENU_MAP_MARKER_SPAN = 4` also names its
four UV spans; the endpoint origins remain authored coordinates. This scale
is pixels per map cell, unrelated to display-buffer count or input frame 2.
Both local `POLY_FT4` arrays use the shared `KF_DISPLAY_BUFFER_COUNT`; their
explicit `[0]` initialization and `[1]` copies preserve packet ordering.

## Map resource and retained-literal evidence

All ten shipped `KF/MAP/M11.` through `M15.` and `M21.` through `M25.` are
33312-byte, 4-bit TIM files. Each has a 524-byte palette block at byte 8,
with rectangle `(0,501,16,16)`, and a 32780-byte image block at byte 532,
with rectangle `(960,256,64,256)` in VRAM words. The image payload is
32768 bytes from offset 544, representing 256-by-256 texels. For reproduction,
`M11.` has SHA-256
`62f7caf0d4396df5896fa6f5bc0f6c2abcc4cd900f40dbc8b084fdd64a3cd335`.
The packet's CLUT `0x7d40` selects the first palette row at (0,501), and
texture page `0x1f` selects the matching image origin. Its 0..219 UV span
and screen endpoints (50,10)..(269,229) are an authored crop/placement,
not the complete TIM dimensions.

The marker uses COM/MIX.TIM image 3: 4-bit image origin (704,256) and first
palette row (0,497), matching `tpage=0x1b` and `clut=0x7c40`. That TIM starts
at byte 198240, with palette payload at 198260 and image payload at 198784.
The whole MIX.TIM hash is
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
Both packet recipes retain RGB modulation 64, half the neutral texture
modulation 128. This is color multiplication, not an alpha value; only the
map image explicitly enables semi-transparency.

The [complete literal ledger](game-menu-panel-literal-ledger.md) explains
190 retained occurrences: status panel 44, map viewer 107, list renderer 38,
plus its 22-halfword workspace extent. It includes each explicit packet
index, path digit, texture recipe, coordinate, decimal quantity operation,
row pitch and Boolean/control boundary. The existing
[preview ledger](game-menu-preview-literal-ledger.md) is refreshed from
204 to 196 occurrences after removing its eight named window/background
depths. No new tests or layout assertions were introduced.

## Final verification

Eight new constants and the existing display-buffer count replace 49 numeric
uses across four source files. Forced compilation preserves every section of
all 112 objects except `.debug_line` in the map-viewer object. All 484 strict
scores are unchanged. For each of the seven functions in the plan, the final
verdict is unchanged at its listed percentage; complete source words and
ordered calls/address pairs were independently compared:

| Function | Source words / calls / HI16-LO16 pairs |
| --- | --- |
| `menu_status_panel` | 355 / 20 / 37 |
| `menu_map_viewer` | 256 / 17 / 8 |
| `menu_list_render` | 495 / 14 / 70 |
| `menu_draw_item_name_frame` | 498 / 23 / 112 |
| `menu_blit_sprite_translucent` | 104 / 3 / 9 |
| `menu_blit_sprite` | 100 / 2 / 8 |
| `menu_draw_window_backdrop` | 425 / 16 / 107 |

The map viewer remains strict 100%, with all 256 retail words and every
ordered referent matching. Its linked SHA-256 remains
`9bbd8beef22fb5cbef7d9be16dcee8cf35e447fa5c1836aca7ecd826c1dce7b2`.
Other selected functions retain their existing non-exact residues; naming
does not change the exact count or bank ledger.

All 665 repository tests pass (86.135 seconds), along with inventory
validation, Ruff and whitespace checks. Modern Clang retains the same
multiset of 320 existing errors: 64/112 source/image variants pass, 48 fail,
including the four edited units. Full `kf build` retains existing data and
ownership failures: PSX 0/1, GAME 9/42, OPEN 2/19 data units; target relink
1/1, 75/77, 34/38, six conflicting section bases and zero artifact failures.
