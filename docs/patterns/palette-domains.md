# Image-specific color-preset domains

## Function Match Plan

GAME and OPEN each have a `lighting_set_active_color_matrix` function and a
`color_matrix_table`, but their selector values describe different domains.
Promote the existing named selectors to `KfGameColorPreset` and
`KfOpenColorPreset`, respectively, using the project's C++20 enum compatibility
layer and the existing `s32` retail parameter representation. Propagate each
domain through its API declaration and definition. Convert to an integer only
at the table subscript, including all direct palette references in fades and
map initialization. Keep matrix pointers and the SDK API authentic.

Both retail selectors shift the O32 a0 argument left by five, form their own
table address, and add the offset in the `SetColorMatrix` call's delay slot.
Neither tests bounds or narrows the argument. Keep the 32-bit representation;
the nonnegative admitted callers do not independently prove signedness.
The two curated signatures retain their existing evidence tier and acquire
the respective enum parameter. Table counts and HUD brightness remain
integer quantities outside the selector domains.

GAME's Shadow Blade consumer is already established by the equipped-item test
in `player_update`; replace the local `WEAPON9_COLOR_BLEND` spelling with
`SHADOW_BLADE_COLOR_BLEND` while preserving 2500. No physical explanation for
that authored strength is inferred.

The baseline is the current master source after the lighting ledger and OPEN
unlit-renderer commits. Force the seven affected units, capture all six semantic
views and source history, inspect the table references and every selector-call
argument, then compare complete runtime contents and ordered relocations after
editing. Isolated before/after builds of all 112 source/image variants must
remain identical at runtime. Compiler controls must reject raw integers,
unrelated IDs and the other image's palette type at the API boundary. Explicit
enum decoding remains an unchecked conversion, not runtime validation.

## Per-function evidence snapshots

Direct call edges below are `proven`; navigator HI16/LO16 table references are
`validated`. They are curated referents, not recovered relocation records.
All thirteen changed functions have zero string references. Game wrappers and
scene controllers remain distinct from the Sony `SetColorMatrix` bodies:
GAME `0x8004d7e4` and OPEN `0x8002d5b8` are attributed to LIBGTE in the vendor
inventory. The existing MATRIX layout and table ownership are retained.

| Image / function / VA / extent | Baseline | Relevant source and retail evidence |
| --- | ---: | --- |
| GAME `player_death_apply_visual_fade`, `0x800184b0`, `0x90` | 100% | Source matrix to black (+0x80), white (+0x60) to black for the effect matrix; signed blend remains unchanged through color, fog and brightness consumers. |
| GAME `player_death_update_reverse_fade`, `0x800186c4`, `0xe0` | 100% | Black to default and black to white; completion applies zero fade to default, other branch uses complementary progress. Halfword progress and call order are unchanged. |
| GAME `lighting_apply_weapon9_environment`, `0x800187a4`, `0x4c` | 100% | Black (+0x80), blend 2500 in a2's delay slot, and subtract-half near fog; incoming call from the Shadow Blade check. |
| GAME `lighting_apply_timed_player_effect`, `0x800187f0`, `0x34` | 100% | Green (+0xa0), blend 3072; incoming call from the illusion-staff timer path. |
| GAME `lighting_apply_color_preset6`, `0x80018824`, `0x34` | 100% | Blue (+0xc0), blend 3072; no admitted incoming references establish an additional gameplay purpose. |
| GAME `lighting_set_active_color_matrix`, `0x8001bab8`, `0x2c` | 100% | a0 shifted by five, GAME table at `0x80055dbc`, one SDK call; eight direct callers select default, damage or defense-effect entries. |
| GAME `map_reveal_fade`, `0x80034438`, `0x184` | 100% | Both loops blend default to white (+0x60); final selector call restores default. Event movement, loop bounds and saved light-matrix copy are untouched. |
| GAME `map_load_floor`, `0x80036554`, `0xa4` | 100% | After resource/state/texture setup, copies the complete white MATRIX at +0x60 into the effect matrix. |
| OPEN `opening_scene0_run`, `0x80014268`, `0x174` | 100% | Black (+0x20) to default; a1 is derived by subtracting 32. Halfword blend is sign-extended in the interpolation call's delay slot. |
| OPEN `opening_scene3_run`, `0x80014804`, `0x330` | 100% | First loop uses black to default, last loop default to black; retain the scene traversal between those loops. |
| OPEN `opening_ending_scene_run`, `0x80014b34`, `0x2f4` | 100% | Black to default, later default to black. Existing color updates and frame calls remain ordered. |
| OPEN `opening_ending_scroll_run`, `0x80014e28`, `0x798` | 97.952675% | Two lighting phases: black (+0x20) to midpoint (+0x60), then midpoint to green (+0x80). Different blend steps and the scroll state machine remain unchanged. |
| OPEN `lighting_set_active_color_matrix`, `0x800168dc`, `0x2c` | 100% | a0 shifted by five, OPEN table at `0x80035944`, one SDK call; the sole admitted caller is display initialization with default. |

## Result and boundaries

The seven GAME selectors and five OPEN selectors now belong to distinct enum
types. Both public selector signatures and their curated identities use those
types. All nine source call sites already pass the correct named constant and
now receive compiler checking through the changed declarations. Direct matrix
references encode the enum explicitly at the integer array subscript. The
matrix interpolation APIs still take MATRIX pointers, since they also accept
constructed and saved matrices that are not palette entries.

For example, GAME accepts `lighting_set_active_color_matrix(KF_GAME_COLOR_BLACK)`
and rejects the same call with `KF_ITEM_SHADOW_BLADE` or a `KfOpenColorPreset`.
The shared C++20 compatibility layer rejects implicit integer assignment and
comparison too. In the pinned C build each domain remains an `s32` typedef.
Explicit `KF_ENUM_DECODE` can still construct an unlisted value; neither this
conversion nor `KF_ENUM_ENCODE` checks bounds. The array conversion is a visible
integer boundary, not a typed container that prevents all deliberate misuse.

Eighteen isolated Clang controls use the real project headers and modern
checking flags. Four accept valid domain/API/index chains and explicit decode;
fourteen reject raw API arguments, raw assignments/comparisons, item arguments,
other-image arguments/assignments and implicit array indexing. The other image's
enum is forward-declared with its actual type name and underlying `s32`, because
including both full rendering interfaces would mix incompatible image globals.
Each negative control fails on the intended operation, with no unrelated errors.

## Verification

All thirteen function verdicts retain their baseline values: twelve exact and
the unchanged OPEN ending-scroll partial. Forced rebuilt objects retain 1,269
resolved instruction words, 127 calls and 116 address references. The 783 words
in exact functions equal retail and the delinked target. Ending scroll still
first differs at OPEN `0x800150dc`: candidate `addiu t1,sp,0x98`, retail
`addiu t0,sp,0x98`. This is the existing residue, not a new compiler attribution.
All 484 strict scores are unchanged.

Isolated before/after compilation of all 112 source/image variants preserves
allocated sections, runtime symbols and ordered relocations; no object section
changes, including debug sections. The seven GAME matrices (224 bytes) and five
OPEN matrices (160 bytes) equal their source initializer values, compiled
baseline/current contents, delinked targets and retail images. Nothing is
rounded, reordered or normalized.

All 684 repository tests pass (107.539 seconds), as do Ruff, inventory and
whitespace checks. Existing campaign signature records and the GAME signature
fixture now agree with the typed declarations. No production tests or size
assertions were added. Modern checking retains the same 300 errors, with
65/112 source/image variants passing. Full `kf build` retains the existing
source-data placement failures (PSX 0/1, GAME 9/42, OPEN 3/19 matches) and
target-relink failures (PSX 1/1, GAME 75/77, OPEN 34/38 verified), with six
conflicting section bases and no data-artifact failures.

The source census is unchanged: 6,428 inline literal occurrences across 111 C
files, with 85 files / 5,078 occurrences covered by current per-occurrence
ledgers. The lighting ledger updates its one affected zero-blend expression;
the reason and multiplicity are unchanged. No remaining literal is removed by
this type-only promotion. The local Shadow Blade strength receives a clearer
owner name; the ten source lines containing unresolved fields remain open.
Ignored reproduction scripts and evidence are under
`build/constant-names/palette-domains/`.
