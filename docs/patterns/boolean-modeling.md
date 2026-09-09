# Boolean modeling and source census

`kf bools` uses the flake's Python `clang.cindex` binding to libclang. It parses
the retail GNU89 view with the MIPS-I/O32 target, pinned SDK headers and every
configured source/image variant. This uses the existing Python libclang stack;
it does not add the separate `pylibclang` package.

```sh
nix develop
kf bools --output build/boolean-audit/all.json
kf bools --image game --unit game.actor --list
kf check-types
```

The report keeps every integral declaration/result it encounters, rather than
only the attractive candidates. Image-qualified Clang declaration identities
join prototypes and definitions across units. The value analysis follows
initializers, assignments, direct call arguments, returned expressions, copies,
array elements, pointer aliases and output arguments to a fixed point. It
recognizes comparisons, logical operators, masked low bits, and complete-object
zero fills. Numeric consumers propagate back through copies and calls.
Existing scoped-enum macros and Boolean aliases retain their domains.

Each row includes declaration location, original/canonical type, scalar width,
array depth, bitfield width, possible value classes, writers, readers, hazards,
and a verdict. The JSON includes coverage and limitations. A parse error,
source change during collection, or missing source/header in a full run fails
the command. Focused runs explicitly mark partial coverage.

## Representation

`include/kf/bool.h` keeps the existing retail storage type:

| Alias | Retail C type | O32 size |
| --- | --- | --- |
| `KfBool` | `int` | 4 |
| `KfBool32` | `s32` / signed long | 4 |
| `KfBoolU32` | `u32` / unsigned long | 4 |
| `KfBool8` | `u8` | 1 |
| `KfBool16` | `u16` | 2 |

The modern C++20 view wraps one integer storage member and accepts Boolean
expressions, `KF_FALSE`, `KF_TRUE`, or another Boolean storage type. Integer
and pointer assignments are rejected. The wrapper converts to `bool` for
conditions; it does not promise to prohibit every implicit Boolean-to-integer
read. Layout tests verify size, alignment, field offsets, standard layout,
trivial copying and the exact retail C canonical types. These assertions live
in test fixtures. The modern view is for checking; the pinned compiler still
builds the C89 typedef view.

Keeping `int` distinct from `s32` matters even on a target where both occupy
one word. No cast, normalization operation, volatile carrier or assembly was
introduced into the retail function bodies.

## Reviewed campaign

The campaign converts 19 declarations: seven return values, nine locals, two
inline-helper parameters and one global. Shared prototypes and curated
signature/data inventories agree with the aliases. Five of those predicates
were already called `bool` in the inventory but still declared as integers in
C. The complete per-function verdicts are in
[boolean-modeling.tsv](../../config/evidence/boolean-modeling.tsv).

True retains the existing polarity: `menu_load_message_image` returns true on
load failure, while `player_warp_trigger_update` returns true on the two game
completion paths. `memory_card_show_status_message` only changes its local
image-load result; its own return remains `s32` because it explicitly returns
`-1` on failure and falls through otherwise.

Numeric exclusions remain numeric:

| Declaration | Evidence |
| --- | --- |
| `KfFloorDeformSegment.column_step` | Added to a column coordinate. |
| `display_show_error_screen.back` | Selects a framebuffer array element. |
| `screen_show_image_until_input.index` | Selects a framebuffer array element. |
| `audio_play_current_map_sequence.sequence_id` | Selects music sequence 0 or 1; numeric consumers propagate through the callee. |
| `cd_file_load_allocated.loaded`, `cd_file_load_into.loaded` | Also hold a rounded CD sector count before becoming retry flags. |

The promising record fields already have meaningful enum domains:
`KfEffectRecord.sound_played` uses `KfAudioPlaybackResult`, and the four player
option fields use `KfPlayerOption`. `menu_format_number.pad_zero` similarly
uses `KfFormatPaddingMode`. Collapsing those domains to a generic Boolean would
discard information. No raw record field met the reviewed conversion criteria
in this pass; the audit and storage layer both support fields and arrays.

## Census and limits

At the original campaign snapshot, the full census covers 111 C sources, 112 source/image
variants and all 56 project headers, with 6,274 declarations and 3,544 integral
slots. It reports 21 Boolean slots (including shared declarations in other
images), 570 enum slots, 1,196 non-Boolean slots, 436 external boundaries and
19 vendored slots. The remaining review classes contain 25 numeric-use cases,
61 single-value cases, 101 cases with unknown writers and 1,115 unknowns. There
are no unreviewed `candidate` rows in this snapshot. These are source-census
counts, not game completion or matching progress.

This is flow-insensitive evidence about reconstructed source. It does not prove
initialization on every path, exhaustively resolve indirect pointers, or prove
what unreconstructed retail writes. Fields join all instances of their record
type, arrays join elements, and union views conservatively contaminate each
other. Unsupported expressions remain unknown; bitfield storage needs manual
review. The `other` class may overapproximate arithmetic/cast results. A
`candidate` row is a review proposal, never authorization for automatic edits.

## Verification

Fresh before/after compilation of all 15 changed units preserves every allocated
section's bytes, size and alignment, plus ordered relocation section/offset,
kind, symbol, symbol value and explicit addend. REL addends remain covered by
the unchanged section bytes. All 137 function scores are unchanged: 133 strict
100% results and four existing non-exact bodies. Exact-count movement is zero.

The campaign's two non-exact functions remain `map_interaction_dispatch`
(99.202774%) and `menu_status_panel` (99.962170%). Two unchanged neighbors in
the compared units also remain non-exact: `map_show_screen_image` (88.888885%)
and `talk_show_dialogue_page` (98.780490%). No residue is assigned a compiler
mechanism by this type-only experiment. Vendored code and SDK boundaries were
not remodeled.

All 112 modern source/image checks pass. Audit tests cover cross-unit flow,
macros, enum exclusions, aliases, pointer outputs, external mutation, unions,
array initialization, numeric consumers, bitfields, missing returns, parse and
coverage failures, and deterministic parallel results. Boolean type tests cover
positive interfaces and negative integer/pointer assignments.

The original campaign suite passes all 759 tests; `ruff check scripts tests`,
`git diff --check` and `nix flake check -L` pass. The isolated flake suite skips
140 controls that require local retail/build artifacts; the workspace suite
runs all 759 without skips.

The required full `kf build` was run. It continues to fail the existing
data-placement, ownership and target-relink checks across the images; this
campaign's compared object sections and relocations are unchanged. No new
matching result is banked by the Boolean campaign.

## Integrated verification

The pre-commit review after the data-owner and matching integrations passes
all 769 workspace tests without skips, all 112 modern source/image checks,
the structure inventory, Ruff, whitespace checks and `nix flake check -L`.
The isolated Nix suite runs 769 tests with 138 skipped artifact-dependent
controls. A fresh Boolean census still covers all 111 sources, 112 variants
and 56 headers, with no parse errors or missing inputs. It contains 6,248
declarations and 3,541 integral slots: 21 Boolean, 570 enum, 1,192 non-Boolean,
436 external, 19 vendored, 25 numeric-use reviews, 65 single-value reviews,
103 unknown-writer reviews and 1,110 unknowns. No candidate remains unreviewed.

The combined working source retains all 484 function scores from the tested
integration or improves them. The separate item-preview match adds one exact
game function, reaching 453/471; the player-update and item-name-frame changes
remain partial. Full `kf build` completes all objects and reports only the
existing data/ownership/placement gates: 41/64 data-owning units match, with
no missing artifacts. These matching gains belong to their source campaigns,
not the Boolean representation conversion.
