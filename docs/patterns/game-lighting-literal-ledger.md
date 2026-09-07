# GAME lighting and death-fade retained literals

Complete accounting for the **117 retained occurrences** in `lighting.c` (108),
`lighting_presets.c` (1), and `player_death_fade.c` (8). This expands the
[earlier grouped audit](source-constant-names.md#lighting-and-death-fade-plan)
against the current source after `0304ebf` recovered the death-update body.
Every repeated token has its own row. Claims and named definitions are excluded.

## Review plan and evidence

Force the three units through the pinned compiler, inspect all nine GAME
functions with `addr`, `disasm --blocks`, incoming/outgoing `xref`, `strings`,
and `match`, then verify the retained values against their consumers. Compare
the complete color table against the retail bytes and compiled initializer.
Keep authored coefficients, matrix coordinates, null arguments and arithmetic
origins literal where their expressions already state their roles. This review
changes documentation; it does not introduce a new source hypothesis or claim
new matching progress.

All nine functions below are game wrappers/state updates, rather than SDK
bodies. Their `ReadColorMatrix` and `SetColorMatrix` callees are separately
attributed to Sony LIBGTE in `functions_vendored.tsv`; the raw GAME setter at
`0x8004d7e4` loads offsets 0, 4, 8, 12 and 16 into GTE color-matrix registers.
The wrappers retain those authentic SDK calls. No strings occur in these nine
function dossiers. Direct calls are `proven`; the paired address references
reported by the navigator are `validated`, not recovered relocation records.

| GAME function / VA / extent | Evidence snapshot and retained-literal decision | Final verdict |
| --- | --- | --- |
| GAME `player_death_apply_visual_fade`, `0x800184b0`, `0x90` | Three calls: color interpolation toward table +0x80, effect matrix from +0x60 to +0x80, and saved near-fog distance toward zero. Brightness uses signed Q12 scaling by the named normal value 86. Zero is a distance endpoint. | 100%; unchanged. |
| GAME `player_death_update`, `0x80018540`, `0x184` | Signed halfword bob/pitch tests; both stores through the bob pointer remain. Pitch clamp stores zero at GAME `0x80018614`. Named fade step 100 advances a halfword, compared against 4096. The completed fade calls the renderer twice with null position/rotation, then restarts; the other branch only applies the current fade. | 100%; unchanged. |
| GAME `player_death_update_reverse_fade`, `0x800186c4`, `0xe0` | Reads signed halfword progress, blends black toward default/white, and fog from zero toward the saved distance. After increment, completion applies fade zero and clears the update-state byte; otherwise it applies the complementary blend. Preserve that second fade application and its order. | 100%; unchanged. |
| GAME `lighting_apply_weapon9_environment`, `0x800187a4`, `0x4c` | Three calls; black endpoint +0x80 and named blend 2500. Retail `sra` at `0x800187d4`, followed by subtraction in the fog call's delay slot, computes `distance - (distance >> 1)`. Preserve the rounding expression. | 100%; unchanged. |
| GAME `lighting_apply_timed_player_effect`, `0x800187f0`, `0x34` | Reads the current color matrix, then blends toward green at +0xa0 using named 3072. Its caller checks/decrements `illusion_staff_timer`. No retained literals. | 100%; unchanged. |
| GAME `lighting_apply_color_preset6`, `0x80018824`, `0x34` | Reads the current matrix, then blends toward blue at +0xc0 using named 3072. No incoming references in the current inventory; no additional gameplay meaning is inferred. No retained literals. | 100%; unchanged. |
| GAME `lighting_transition_color_matrix`, `0x80033d80`, `0x68` | Three calls per iteration, including the frame pacer. Progress starts at zero and advances by 1024 in the pacer call's delay slot. Signed test `< 4097` includes endpoint 4096: five iterations per transition. Renderer arguments are null view overrides. | 100%; unchanged. |
| GAME `color_matrix_set_rgb`, `0x80033de8`, `0x28` | Nine halfword stores put arguments a0/a1/a2 into rows 0/1/2; columns are written 2,1,0. The last blue store at +12 is in the return delay slot. Coordinates express the matrix layout directly. | 100%; unchanged. |
| GAME `player_restore_vitals_with_color_cycle`, `0x80033e10`, `0xd4` | Eight calls: save current matrix, construct green/cyan/white and make four transitions, finally back to saved. Named 4095 is a channel coefficient, not the Q12 blend endpoint 4096. Final halfword stores restore HP/MP; `andi 0xfff0` preserves upper status bits. Three zero arguments suppress color channels. | 100%; unchanged. |

## Palette data and arithmetic boundaries

GAME `0x80055dbc..0x80055e9c` contains seven 32-byte SDK matrices. Each source
row supplies nine halfword coefficients and three zero translation words;
the SDK layout also has two alignment bytes between those groups. The audit
checks all 224 bytes, including those bytes, without adding source padding or
size assertions. Each ledger coefficient names its preset, RGB row and column.
These are authored numeric data under already-named palette selectors, rather
than independent selector values deserving 63 more names.

`matrix_interpolate` at GAME `0x800202fc` visits exactly nine halfwords and
uses `from + (((to - from) * blend) >> 12)`, with signed halfword operands for
the difference. It does not initialize translation members. Likewise,
`color_matrix_set_rgb` writes only the nine coefficients; the temporary
matrices in the color cycle are not whole-object initializers. The SDK setter
loads through offset 16, including the alignment halfword in its last word,
but does not load the translation words at offsets 20, 24 and 28. This audit
does not assign a gameplay meaning to that alignment halfword.

Default, damage and defense coefficients are preserved exactly, including
700 versus 70 and 4000 versus 400. White/green/blue use 4095, not 4096;
replacing these authored values with a generic fixed-point-one constant would
change the data. The black preset and inactive rows contain zero contribution.
Translation zeros are initialized SDK members outside the color interpolation
operands. None of these values establishes the designer's reason for choosing
a particular hue or strength.

The fog shift is signed subtract-half, not a general replacement by division:
for a positive odd distance it retains the upper half. No physical distance
unit or real-time frame rate is inferred. The null arguments to `render_frame`
reach `render_set_view_transform`, which updates position and rotation only
when their respective pointers are non-null. Zero blend selects the supplied
source matrix and, in the death fade wrapper, restores the saved near-fog
distance and normal HUD brightness.

## Verification

Forced baseline and final compilation preserve the three objects' allocated
sections, runtime symbols and ordered relocations. All nine functions remain
100%: 323 resolved instruction words, 32 direct calls and 43 address references
equal the baseline, delinked target and retail. The source initializer and both
compiled versions of the seven matrices equal all 224 retail bytes. No source
files were changed, and no new exact result is claimed. A concurrent OPEN
`0x80018344` score improvement is outside this review.

All 684 repository tests pass (122.037 seconds), as do Ruff, inventory and
whitespace checks. Modern checking retains the same 300 diagnostics and
65/112 passing source/image variants. Full `kf build` still fails on existing
data placement: source-data matches are PSX 0/1, GAME 9/42 and OPEN 3/19;
target relinks are PSX 1/1, GAME 75/77 and OPEN 34/38, with six conflicting
section bases and no data-artifact failures. These failures do not invalidate
the separately verified color-table extent.

The complete source census and token/expression comparison now verify
**85 files / 5,078 retained occurrences**. Overall counts remain 111 C files,
6,428 literal occurrences and ten source lines containing unresolved `unknown_`
fields. This is documentation coverage, not a count of unnamed constants or
proof that all semantic work is complete. Ignored reproduction inputs, raw
dossiers and verification reports are under
`build/constant-names/lighting-literal-coverage/`.

## `src/game/lighting.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `color_matrix_table` | 18 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `2000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `700` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `4000` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFAULT, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 18 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_DEFAULT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 18 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_DEFAULT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 18 | `0` | `{{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_DEFAULT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 19 | `3000` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `1000` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `4000` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `200` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `70` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `400` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `200` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `70` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `400` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DAMAGE, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 19 | `0` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_DAMAGE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 19 | `0` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_DAMAGE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 19 | `0` | `{{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_DAMAGE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 20 | `1000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `350` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `2000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `1000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `350` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `2000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `3000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `1000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `4000` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_DEFENSE_EFFECT, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 20 | `0` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_DEFENSE_EFFECT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 20 | `0` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_DEFENSE_EFFECT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 20 | `0` | `{{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_DEFENSE_EFFECT; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `4095` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_WHITE, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 21 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_WHITE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 21 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_WHITE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 21 | `0` | `{{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_WHITE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLACK, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_BLACK; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_BLACK; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 22 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_BLACK; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `4095` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_GREEN, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `4095` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_GREEN, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `4095` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_GREEN, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_GREEN, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_GREEN; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_GREEN; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 23 | `0` | `{{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_GREEN; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, red row m[0][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, red row m[0][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, red row m[0][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, green row m[1][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, green row m[1][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero channel contribution in KF_GAME_COLOR_BLUE, green row m[1][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `4095` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_BLUE, blue row m[2][0]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `4095` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_BLUE, blue row m[2][1]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `4095` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Authored channel coefficient in KF_GAME_COLOR_BLUE, blue row m[2][2]. Keep the numeric matrix data under its named preset; do not normalize the coefficient. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[0] of KF_GAME_COLOR_BLUE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[1] of KF_GAME_COLOR_BLUE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `color_matrix_table` | 24 | `0` | `{{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},` | Zero translation component t[2] of KF_GAME_COLOR_BLUE; initialize the SDK member although color interpolation and SetColorMatrix do not consume it. |
| `lighting_transition_color_matrix` | 30 | `0` | `s32 blend = 0;` | Arithmetic origin of Q12 interpolation: the first iteration applies the source matrix unchanged. |
| `lighting_transition_color_matrix` | 34 | `0` | `render_frame(0, 0);` | Null position override; render_set_view_transform retains the current view position during each transition frame. |
| `lighting_transition_color_matrix` | 34 | `0` | `render_frame(0, 0);` | Null rotation override; render_set_view_transform retains the current view rotation during each transition frame. |
| `color_matrix_set_rgb` | 43 | `0` | `matrix->m[0][2] = red;` | Explicit red row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 43 | `2` | `matrix->m[0][2] = red;` | Explicit column 2 within the red row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 44 | `0` | `matrix->m[0][1] = red;` | Explicit red row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 44 | `1` | `matrix->m[0][1] = red;` | Explicit column 1 within the red row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 45 | `0` | `matrix->m[0][0] = red;` | Explicit red row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 45 | `0` | `matrix->m[0][0] = red;` | Explicit column 0 within the red row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 46 | `1` | `matrix->m[1][2] = green;` | Explicit green row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 46 | `2` | `matrix->m[1][2] = green;` | Explicit column 2 within the green row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 47 | `1` | `matrix->m[1][1] = green;` | Explicit green row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 47 | `1` | `matrix->m[1][1] = green;` | Explicit column 1 within the green row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 48 | `1` | `matrix->m[1][0] = green;` | Explicit green row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 48 | `0` | `matrix->m[1][0] = green;` | Explicit column 0 within the green row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 49 | `2` | `matrix->m[2][2] = blue;` | Explicit blue row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 49 | `2` | `matrix->m[2][2] = blue;` | Explicit column 2 within the blue row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 50 | `2` | `matrix->m[2][1] = blue;` | Explicit blue row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 50 | `1` | `matrix->m[2][1] = blue;` | Explicit column 1 within the blue row; all three columns receive the same channel argument. |
| `color_matrix_set_rgb` | 51 | `2` | `matrix->m[2][0] = blue;` | Explicit blue row coordinate of the three-row color matrix. |
| `color_matrix_set_rgb` | 51 | `0` | `matrix->m[2][0] = blue;` | Explicit column 0 within the blue row; all three columns receive the same channel argument. |
| `player_restore_vitals_with_color_cycle` | 65 | `0` | `color_matrix_set_rgb(0, VITAL_RESTORE_COLOR_LEVEL, 0, &first);` | Zero red contribution when constructing the green endpoint; the active channels use the named restoration level. |
| `player_restore_vitals_with_color_cycle` | 65 | `0` | `color_matrix_set_rgb(0, VITAL_RESTORE_COLOR_LEVEL, 0, &first);` | Zero blue contribution when constructing the green endpoint; the active channels use the named restoration level. |
| `player_restore_vitals_with_color_cycle` | 68 | `0` | `0, VITAL_RESTORE_COLOR_LEVEL, VITAL_RESTORE_COLOR_LEVEL, &second);` | Zero red contribution when constructing the cyan endpoint; the active channels use the named restoration level. |

## `src/game/lighting_presets.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `lighting_apply_weapon9_environment` | 17 | `1` | `SetFogNear(render_state.fog_near_distance - (render_state.fog_near_distance >> 1),` | Arithmetic right shift by one obtains the signed lower half for distance - (distance >> 1); retain the subtraction and its positive-odd rounding, rather than inventing a mode value. |

## `src/game/player_death_fade.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `player_death_apply_visual_fade` | 26 | `0` | `fog_interpolate_near(player_death_saved_fog_near, 0, blend);` | Zero near-fog distance is the destination endpoint of the signed Q12 distance interpolation, not a selector. |
| `player_death_update` | 54 | `0` | `player_state.death_camera_pitch_step = 0;` | Clear the pitch increment when the camera pitch is clamped at its named lower bound; zero means no stored increment. |
| `player_death_update` | 64 | `0` | `render_frame(0, 0);` | Null position override; render_set_view_transform retains the current view position. Keep both calls at the death endpoint. |
| `player_death_update` | 64 | `0` | `render_frame(0, 0);` | Null rotation override; render_set_view_transform retains the current view rotation. Keep both calls at the death endpoint. |
| `player_death_update` | 65 | `0` | `render_frame(0, 0);` | Null position override; render_set_view_transform retains the current view position. Keep both calls at the death endpoint. |
| `player_death_update` | 65 | `0` | `render_frame(0, 0);` | Null rotation override; render_set_view_transform retains the current view rotation. Keep both calls at the death endpoint. |
| `player_death_update_reverse_fade` | 81 | `0` | `fog_interpolate_near(0, player_death_saved_fog_near, *blend);` | Zero near-fog distance is the starting endpoint of the signed Q12 distance interpolation, not a selector. |
| `player_death_update_reverse_fade` | 85 | `0` | `player_death_apply_visual_fade(&color_matrix_table[KF_ENUM_ENCODE(s32, KF_GAME_COLOR_DEFAULT)], 0);` | Zero interpolation blend restores the unmodified default color matrix, saved near-fog distance and normal HUD brightness before normal updates resume. |

The [palette-domain review](palette-domains.md) types GAME and OPEN selectors
separately. The zero-blend row above now includes the explicit enum-to-index
conversion; its arithmetic meaning and the 117-occurrence count are unchanged.
