# GAME menu sprite coordinate control

## Function Match Plan

Campaign: the two single-quad blitters in `game.menu_runtime`, using the
existing `probe-gcc257-o2-g0` profile. Their shared descriptor, point prefix,
primitive cursor, callers and repeated complete body shape support treating
them together; the original translation-unit boundary remains WIP.

| GAME function | Extent | Starting strict match | Calls / address pairs |
| --- | --- | --- | --- |
| `menu_blit_sprite_translucent`, `0x80029ab0` | 416 bytes, 104 words | 88.269230% | 3 proven calls, 9 validated HI16/LO16 pairs |
| `menu_blit_sprite`, `0x80029c50` | 400 bytes, 100 words | 87.800000% | 2 proven calls, 8 validated HI16/LO16 pairs |

Both take `const MenuSpriteDef *` in `a0` and `const MenuPoint *` in `a1`;
none of their thirteen direct call sites consumes a result. Each has a
32-byte frame saving `ra`, `s1` and `s0`. There are no conditional branches,
strings, indirect transfers or candidate references in either body. The
return delay slot restores the complete frame.

Both first call `primitive_buffer_begin_poly_ft4`, saving the incoming point
in `s1` in its delay slot. They read page and CLUT halfwords, write the eight
coordinate halfwords in x0/y0 through x3/y3 order, then write the eight UV
bytes in the same corner order. `MenuSpriteDef` is the shared 12-byte record
with page, CLUT, u, v, width and height halfwords; the point is a four-byte
prefix shared by labels. Narrow UV loads alone do not establish byte-sized
source members. SDK `POLY_FT4` retains its authentic signed-short coordinates
and unsigned-byte UV members.

Every validated address pair refers to `current_poly_ft4` at `0x80057e88`.
The last common call is `primitive_buffer_commit_poly_ft4`, with depth 2000
set in its delay slot. Its body links the packet with `AddPrim` and advances
the shared cursor by the SDK packet size, 40 bytes. The begin helper calls
`SetPolyFT4` and sets RGB to 96.

The translucent function uses insets (4, 3), then calls the vendored
`SetSemiTrans` with `a1 = 1` in the delay slot; that callee sets bit 1 of the
packet code byte. The opaque function uses insets (18, 2) and has no such
call. Call instructions are respectively at `0x80029ac4`, `0x80029c2c`,
`0x80029c34` and `0x80029c64`, `0x80029dc4`.

### Caller and neighbor audit

All call sites were inspected with their argument setup and delay slots:

| Callee | Caller | Call sites | Descriptor / point |
| --- | --- | --- | --- |
| translucent | `menu_config_panel_draw` | `0x80025dfc`, `0x80025e1c`, `0x80025e2c` | `DAT_8005840c` / `DAT_80058418`; stack label prefixes at +32 and +56 |
| translucent | `menu_draw_item_detail` | `0x80027dd4` | `DAT_80058424`; `menu_window_layouts[7].rows[3]` |
| translucent | `menu_draw_window` | `0x80028984`, `0x800289d8` | `DAT_80058424` / `DAT_80058430`; title or 24-byte row |
| translucent | `menu_list_render` | `0x80028ac8` | `DAT_80058424`; list title prefix |
| translucent | `menu_draw_two_option` | `0x80029284`, `0x800292a4`, `0x800292b4` | `DAT_8005840c` / `DAT_80058418`; either option label |
| opaque | `menu_draw_window` | `0x80028a00` | `DAT_8005846c`; selected row |
| opaque | `menu_draw_two_option` | `0x80029238`, `0x80029250` | `DAT_8005846c`; either option label |

The preceding `menu_draw_item_name_frame` also uses the primitive cursor and
SDK helpers, returning at `0x80029aa8` with its 224-byte frame restoration at
`0x80029aac`. The following `menu_draw_string` starts at `0x80029de0` and uses
the same descriptor and positioned-label family. Neither boundary overlaps.

Neither blitter occurs in the curated vendored inventory or the available
FID matches. Their game-owned primitive cursor, menu descriptors, custom
insets and menu-only callers distinguish them from the SDK helpers they
call. The supplied Release 2.5 `LIBGPU.H` was checked directly: `setXYWH`
emits exactly the observed corner-store order and uses origin plus extent;
it is a source-macro candidate, not a library-body attribution.

History controls `28e22dd` and `539d305`, including their predecessor
`src/game/menu_sprite_blit.c`, moved the same point and descriptor layouts
to the shared header without changing these bodies. No original source was
recovered by that history pass.

### First source hypothesis and verification

At relative +0x40 retail leaves a load-delay `nop`, followed by an `addiu`
with the negative x inset. The current object fills that slot with a
positive truncated-halfword constant and uses `addu`; the same difference
recurs for y and all subsequent corners. Calls, referents, frame and store
order already agree. There is no relocation correction to make.

Test the authentic `setXYWH` macro for only the coordinate block, passing
the existing point minus inset and the existing descriptor extents. This
tests the SDK comma-expression and origin-plus-extent source form without
changing types, literal values, UV expressions or APIs. Do not introduce
artificial carriers or alter SDK packet types to select instructions.

Rebuild this unit, compare from the first raw difference and check ordered
relocations. Run the canonical strict match and full build; preserve every
previously exact row. Bank only a newly verified 100% function. If the macro
does not explain retail, record it as a negative control and retain the
existing source.

## Control result

The `setXYWH` control was rejected and reverted. Its first difference remains
at +0x40: the two truncated-halfword constants and all eight register adds
remain. At +0x5c it additionally loads the position before the extent, then
subtracts from the position, whereas retail loads and subtracts from the
extent first. That change repeats for the four far-edge coordinates.

Each function changed 16 words from its starting object and differed from
retail in 26 words, up from 10. Sizes remained 104 / 100 words, and all 21 /
18 ordered relocations still agreed. Canonical objdiff recorded 87.615390%
for translucent and 87.120000% for opaque. These are strict percentages;
the different textual similarity printed by `kf try` is not a closure metric.

This control does not establish the original macro spelling or compiler
mechanism. Keep the existing extent-first association, types and literals;
the ten original differences remain unattributed instruction-selection
residue. Neither function is exact or eligible for banking.

The final restored source was rebuilt with `kf try` and the full `kf build`.
Strict percentages return to 88.269230% / 87.800000%, and the menu unit stays
11/16 exact. Both complete instruction/relocation streams return to their
starting form. No blitter source, shared type or SDK header change is kept.
Existing lint and all 551 tests pass; the full build still reports the
pre-existing data/ownership/placement gaps, independently documented in the
[actor action-selection verification](game-actor-action-selection.md).
