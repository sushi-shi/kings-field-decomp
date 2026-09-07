# GAME menu sprite coordinate control

## Function Match Plan: signed screen-coordinate interpretation (`8def349`)

Revalidated retail and refreshed all six views for GAME `80029ab0` (416 bytes,
88.269230%) and `80029c50` (400 bytes, 87.800000%). All thirteen direct caller
windows, both full bodies, the adjacent item-frame/string boundaries, the
primitive begin/commit bodies, SetSemiTrans, shared types, resource loader
and source history were inspected. Keep the unchanged gcc257/O2/G0 profile,
32-byte frames, no conditional branches, one return each, and the 3/2 calls
and 9/8 validated cursor address pairs. There are no candidate references or
strings. The custom menu insets are game policy; SetSemiTrans and SetPolyFT4
have exact Release 2.5 LIBGPU PRIM provider evidence.

The first raw differences remain the near-corner subtractions: retail loads
a halfword, leaves a load-delay nop, and uses addiu with -4/-3 or -18/-2.
The candidate instead loads a truncated positive halfword constant into a
register and adds it. All coordinates are eventually stored as SDK signed
shorts (`LIBGPU.H` POLY_FT4, lines 373-383); the layout/title guards elsewhere
also consume signed halfwords. A narrowed halfword load alone cannot establish
the source's signedness when only a final halfword is observable. The current
unsigned MenuPoint originated as a reconstruction assumption, not SDK source.

Test a single shared signed-halfword screen-point interpretation, without
changing its four-byte extent, API pointer arguments, resource bits, sprite
dimensions, expressions, packet types or calls. This is a type hypothesis,
not a claim that a negative immediate uniquely proves a signed input. Check
all sixteen unit functions and every dependent unit if it produces a retained
correction; do not introduce per-file views or carry an unsupported signed
declaration merely for a score. Revert if it does not explain the observed
near-corner arithmetic. Only raw exactness and strict 100% qualify for banking.

### Signed point result and dimension follow-up plan

The shared signed point recovers all four near-corner addiu instructions in
each blitter, while preserving retail's lhu loads. Six words now differ per
function instead of ten: two positive constant materializations and the four
far-corner extent subtractions. Their remaining operands are the shared
descriptor's width/height halfwords, not the point. No other unit function's
listing changes. This demonstrates why those lhu instructions did not prove
an unsigned source field.

Inspecting all descriptor field users finds only these blitters and the text
and number renderers. Their six views and complete bodies were refreshed:
`80029de0 menu_draw_string` remains 1328 bytes/99.987950%, 48-byte frame,
12 branches, one return, six calls and 24 validated cursor address pairs;
`8002a310 menu_draw_number` is 512 bytes/100%, 40-byte frame, two branches,
one return, two calls and eight pairs. Both take read-only descriptor/label
pointers, retain the signed -1 glyph guard and have no candidate references,
strings or indirect dispatch. Their 63/47 incoming calls and shared atlas
ownership are recorded in the glyph-rendering dossier. Descriptor dimensions
are consumed only into final halfword XY or byte UV values; no full-width
signed comparison or allocation depends on them. Resource loading copies
the complete 12-byte records without interpreting or extending dimensions.

The actual SDK RECT also declares signed-short width/height (LIBGPU.H
298-301), alongside signed coordinate origins. Test that signed extent
interpretation in the one shared MenuSpriteDef definition, keeping page,
CLUT and UV fields unchanged. Retail's extent-first negative additions in
both blitters are the direct instruction-selection constraint. Do not change
the texture constants, add casts at individual uses, or replace the authentic
packet. Verify the complete four-consumer family and preserve the exact
number renderer before retaining the dimension hypothesis.

With signed dimensions, the string renderer's four immediate discrepancies
disappear without changing its positive 196/210 atlas constants; its complete
listing now agrees, as does the number renderer. Both blitters emit the
negative immediate operations, but the far corners now subtract from the
point before adding the extent. Retail subtracts from the extent first.
Test expressing that observed extent adjustment as the left operand of each
far-corner sum. Keep all eight stores in place and do not add truncation
casts, cached locals or per-use type exceptions to constrain reassociation.

The extent-left expression retains the negative adjustments and correct sum
destination, but still exchanges the point/extent loads at the four far
corners (eight words each). Revisit the genuine SDK setXYWH macro once with
the now-supported signed point and dimensions. The earlier rejected macro
control used unsigned inputs; it therefore did not test this recovered type
contract. Pass the same point-minus-inset and descriptor extents, preserving
the SDK's eight ordered assignments. Stop this expression experiment if the
real macro does not explain the remaining load order; no invented helper or
truncation is justified.

### Exact result

The real SDK setXYWH macro with signed point coordinates and signed descriptor
dimensions reproduces both blitters completely. Keep that shared type model
and those two macro invocations. No UV/page/CLUT type, resource byte, inset,
call, packet definition or compiler option changed. The ordinary source's
unsigned types and arithmetic association were separate reconstruction facts;
the earlier unsigned macro control did not rule out the SDK macro itself.

Strict objdiff is **100%** for `menu_blit_sprite_translucent` (from 88.269230%)
and `menu_blit_sprite` (from 87.800000%). A fresh independent compilation,
linked using the actual relocations, reproduces all 104/100 retail words,
3/2 ordered calls and 9/8 ordered address pairs. The target object was itself
reconstructed to raw retail before comparing the candidate. Complete linked
SHA-256 values:

- Translucent: `ef8bd8b517828832b0723610cb1f959515fcd814adfeff54101e134d728a624d`.
- Opaque: `cd1b95e77113360c923d48c7153416facd4ff2637b42385110660ab613064733`.

The same descriptor correction independently closes `menu_draw_string`:
all 332 words, six calls and 24 address pairs agree, without changing its
positive atlas constants. See the glyph-rendering dossier. The unit advances
from 11/16 to 14/16 exact. All fourteen exact bodies, the previously closed
menu-list renderer and three restored OPEN render-initialization controls
were independently raw-checked. Of all 484 scored functions, only these three
scores change; no prior exact result or other partial score regresses.
Eligible counts advance GAME 305/362 to 308/362 and overall 404/471 to
407/471. OPEN remains 98/108 and PSX 1/1.

This is evidence for the recovered type/macro combination, not historical
compiler attribution or proof of a unique original C declaration. In
particular, lhu/lbu instructions feeding narrowed stores do not by themselves
prove unsigned source fields; here the signed fields preserve those very
loads while correcting the arithmetic.

### Verification and isolation

The four curated structure-field entries now carry the recovered signed
types and evidence; all extents and offsets remain unchanged. An isolated
fresh build of committed `8def349` menu source/header plus only this campaign's
four signed fields and two SDK macros produces identical complete text and
all 699 ordered non-debug relocation identities/addends to the live tree.
Debug-line metadata differs with the temporary source path. Thus these exact
results do not depend on the concurrent dimension-naming edits.

Ruff and both whitespace checks pass. The full test rerun completes 678
tests in 79.170 seconds: 677 pass; the sole failure is the independently dirty
item-stock test's new initializer regex also matching ordinary runtime element
assignments. The inventory failure from stale unsigned field entries is fixed.
Do not change or stage that other campaign's test here. The complete build
retains the existing data/ownership/placement failures: data PSX 0/1,
GAME 9/42, OPEN 2/19; target relinks 1/1, 75/77 and 34/38; zero artifact failures.

After staging only this campaign, selected banking of these three functions
and the previously exact list renderer correctly refuses the remaining
unstaged dimension-naming inputs. No dirty override is used. Commit the
independently verified reconstruction, leaving those four exact ledger updates
pending until the unrelated inputs are committed or otherwise finalized.

### Deferred bank completed

With the independent dimension-naming work committed as `2f1f04a`, fresh
pinned compilations reconfirmed all 104/100/332 retail words and the ordered
calls/address pairs for these three functions, alongside the exact list
renderer and fourteen other exact controls. The full build preserves their
strict 100% results. Selected banking of these three functions and
`GAME.EXE:80028a70` then succeeded with clean source/config inputs and no dirty
override. Only those four ledger rows changed; the prior banking deferral is
resolved.

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
