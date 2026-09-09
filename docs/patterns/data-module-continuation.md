# Data module continuation

## Function Match Plan

Start from `273accaa` in an isolated worktree. The fresh baseline has 453/471
exact game functions, thirteen exact vendor controls, and 41/64 matching
GAME/OPEN data owners. Preserve the existing strict placement contract and
every banked function. Original file boundaries and compiler attribution remain
working hypotheses.

First investigate GAME's menu storage. `current_poly_ft4` at `80057e88` is
currently modeled as an explicitly initialized pointer, although it lies in
the established uninitialized small-storage band between the CD location and
card-event handles. All its source consumers belong to a contiguous menu
family. The draw entry points establish/update its value before packet consumers.
The model-preview rotation at `80057b70` is shared with the runtime's pickup
preview and reset by its model-load helper; its current separate owner is
therefore also provisional.

Audit all pointer readers/writers and their call paths, retail words, callers,
callees, CFGs, strings, relocation identities and source history. Retain one
image-qualified evidence snapshot and final verdict for every affected
function. Probe a contiguous menu owner with private uninitialized packet
state and the existing initialized allocation/rotation state. Use the already
calibrated native G8 section/reservation behavior. Compare separate and
combined source contexts before retaining any consolidation. Record original
contribution coverage separately if module count changes; a smaller denominator
does not constitute a new match.

No source padding, invented arrays, data-address scattering, relocation-addend
adjustments, assembly bodies or ordering permutations may compensate for a
failed hypothesis. Reject any banked-function regression. Keep unresolved
COMMON allocation and genuinely unknown object extents visible. The compiler
source's `-fno-common` path selects initialized data, so it cannot by itself
explain an uninitialized retail allocation.

Then inspect remaining initialized section splits and narrowly supported
private/global ownership corrections. Preserve original SDK allocation classes
and exclude library bodies from game progress. Require fresh focused objects,
raw referent controls, full build, inventory/types, lint, tests and whitespace
checks before committing; tooling changes additionally require Nix checks.

## Menu storage

Retain one WIP owner for GAME `8002430c..8002b078`: 35 contiguous functions
formerly spread across thirteen sources. This is supported by the exclusive
packet-cursor consumers and shared preview state, not contiguity alone. The
preceding `menu_spell_select` and following `save_memory_card_initialize` use
the public menu/save APIs and do not access this cursor. Original file
boundaries and the historical compiler remain unproved.

All 202 validated references to `current_poly_ft4` fall inside this band.
They load/store its four-byte value; none exposes the address of the pointer
object. The draw entry points initialize it from the active primitive buffer:
configuration fields, statistics header, status details, equipment names,
model preview, item details, save summaries, window/list rendering, two-option
rendering and pickup-name rendering. Their packet helpers operate on that
assigned value. The status panel calls `menu_draw_status_details` before its
first quad. `menu_frame_begin` resets the display buffer, not this pointer;
the subsequent draw entry establishes the cursor. No initializer is required
on these paths. Existing public linkage reflected split reconstruction files.

The pointer lies between `cd_read_location` at `80057e80` and the private card
events starting at `80057e90`, in the previously established uninitialized
tail band. Rounded EXE payload zeros are not initializer evidence; see
[executable-byte-provenance.md](executable-byte-provenance.md). Retain a private
uninitialized `POLY_FT4 *` at `80057e88`. Its C width is four bytes, while the
native compiler/assembler reserves eight. Do not widen the C object.

The allocation flag at `80057b6c` and SDK `SVECTOR` at `80057b70` retain their
explicit zero initializers. The vector is used by three preview renderers;
`menu_load_item_model` resets its yaw after registering a model. Together these
objects form twelve initialized bytes, immediately before the card root path
at `80057b78`. The menu texture filename owns ten literal bytes at `80012350`.

Three controlled builds preserved every function word, ordered call and
ordered data referent: combined G0, combined G8, and private-uninitialized G8.
The first two left the distant pointer inside the initialized contribution;
only the last agrees with the reviewed storage classes and native spacing.
The production owner uses `probe-gcc257-o2-g8` and makes no function-body edits.

Independent native validation preprocesses the combined source with
`cpppsx-257 -lang-c -undef -nostdinc`, the project headers and `PSYQ_INCLUDE`,
then uses `cc1psx-257 -quiet -O2 -G8 -mcpu=r2000`. Assemble its CRLF output with
the pinned original `ASPSX.EXE` (`aspsx -G8 -o MENU.OBJ MENU.S`) through the
repository DOS runner; inspect with `psyk list --code`. ASPSX 1.07 reports zero
errors. Its LNK records contain twelve zero bytes in `.sdata`, exported flag
and vector offsets 0/4, and one eight-byte uninitialized `.sbss` allocation
with a local `current_poly_ft4` at offset zero. No fixed `.bss` content appears.
This independently supports the small-section/reservation metadata; it does
not establish an original source filename.

The production strict gate agrees on `.rodata` 10, `.sdata` 12, and `.sbss` 8,
including placement and object widths. No relocation targets, addends or
validation contracts changed. The merged target independently relinks at the
retail addresses. The source's four existing instruction residues preclude
whole-code closure, regardless of the data result.

## Function evidence and verdicts

Before editing, each function received image-qualified `addr`,
`disasm --blocks`, incoming/outgoing `xref`, `strings` and `match` inspection.
The code remains game-owned; Psy-Q packet, GTE, pad and GPU calls are library
boundaries, not new game reconstructions. Existing source signatures, field
widths, constants, branches and delay slots are unchanged. Full resolved-word
comparison against the separately compiled starting objects also preserves
the existing four partial bodies exactly. Calls/references below count ordered
source-object relocations, not heuristic census totals.

The four unchanged residues start at: status panel `+0` (`27bdff90` retail,
`27bdffd0` source); item detail `+ec` (`00002021`, `27a40010`); pickup name
frame `+0` (`27bdff20`, `27bdff60`); backdrop `+0` (`27bdff98`, `27bdffd8`).
These are existing stack/schedule differences with unchanged referents and
calls, retained as unattributed codegen residues.

| GAME VA | Function | Bytes | Calls / refs | Final verdict |
| --- | --- | ---: | ---: | --- |
| `8002430c` | `menu_status_panel` | `0x69c` | 20 / 105 | 99.962170%, unchanged residue |
| `800249a8` | `menu_drop_item` | `0x4bc` | 20 / 3 | 100% exact, unchanged |
| `80024e64` | `menu_save_load_hub` | `0x260` | 20 / 2 | 100% exact, unchanged |
| `800250c4` | `menu_save_panel` | `0x468` | 58 / 0 | 100% exact, unchanged |
| `8002552c` | `menu_load_panel` | `0x370` | 42 / 0 | 100% exact, unchanged |
| `8002589c` | `menu_config_panel` | `0x504` | 19 / 9 | 100% exact, unchanged |
| `80025da0` | `menu_config_panel_draw` | `0x198` | 9 / 11 | 100% exact, unchanged |
| `80025f38` | `menu_draw_stats_header` | `0x5a0` | 28 / 36 | 100% exact, unchanged |
| `800264d8` | `menu_draw_status_details` | `0xcb4` | 72 / 89 | 100% exact, unchanged |
| `8002718c` | `menu_draw_name_list` | `0x838` | 8 / 98 | 100% exact, unchanged |
| `800279c4` | `menu_item_model_preview` | `0x1b8` | 10 / 8 | 100% exact, unchanged |
| `80027b7c` | `menu_draw_item_detail` | `0x2dc` | 17 / 20 | 97.814210%, unchanged residue |
| `80027e58` | `menu_add_marker_quad` | `0x48` | 1 / 3 | 100% exact, unchanged |
| `80027ea0` | `menu_add_frame_quad` | `0x44` | 1 / 3 | 100% exact, unchanged |
| `80027ee4` | `menu_draw_dialog_frame` | `0x49c` | 27 / 26 | 100% exact, unchanged |
| `80028380` | `menu_list_interact` | `0x354` | 21 / 0 | 100% exact, unchanged |
| `800286d4` | `menu_two_option_prompt` | `0x240` | 16 / 0 | 100% exact, unchanged |
| `80028914` | `menu_draw_window` | `0x15c` | 6 / 9 | 100% exact, unchanged |
| `80028a70` | `menu_list_render` | `0x77c` | 14 / 70 | 100% exact, unchanged |
| `800291ec` | `menu_draw_two_option` | `0x10c` | 7 / 11 | 100% exact, unchanged |
| `800292f8` | `menu_draw_item_name_frame` | `0x7b8` | 23 / 112 | 99.975710%, unchanged residue |
| `80029ab0` | `menu_blit_sprite_translucent` | `0x1a0` | 3 / 9 | 100% exact, unchanged |
| `80029c50` | `menu_blit_sprite` | `0x190` | 2 / 8 | 100% exact, unchanged |
| `80029de0` | `menu_draw_string` | `0x530` | 6 / 24 | 100% exact, unchanged |
| `8002a310` | `menu_draw_number` | `0x200` | 2 / 8 | 100% exact, unchanged |
| `8002a510` | `menu_draw_window_backdrop` | `0x6a4` | 16 / 107 | 99.971760%, unchanged residue |
| `8002abb4` | `menu_frame_begin` | `0x80` | 1 / 4 | 100% exact, unchanged |
| `8002ac34` | `menu_present_frame` | `0x98` | 5 / 4 | 100% exact, unchanged |
| `8002accc` | `primitive_buffer_begin_poly_ft4` | `0x50` | 1 / 4 | 100% exact, unchanged |
| `8002ad1c` | `primitive_buffer_commit_poly_ft4` | `0x50` | 1 / 5 | 100% exact, unchanged |
| `8002ad6c` | `menu_list_init` | `0x8c` | 0 / 1 | 100% exact, unchanged |
| `8002adf8` | `menu_format_number` | `0xac` | 0 / 0 | 100% exact, unchanged |
| `8002aea4` | `menu_load_item_model` | `0x68` | 3 / 2 | 100% exact, unchanged |
| `8002af0c` | `menu_release_item_model` | `0x3c` | 1 / 2 | 100% exact, unchanged |
| `8002af48` | `menu_load_item_texture` | `0x130` | 3 / 2 | 100% exact, unchanged |

## Coverage accounting

This closes one formerly failing data contribution without adding exact
functions. GAME/OPEN failures fall from 23 to 22. Two previous data owners
(one passing, one failing) become one passing owner, so the current strict
owner count is 41/63 versus 41/64 before consolidation. On the original
contribution basis, coverage improves from 41/64 to 42/64. The denominator
change itself is not progress. Exact game functions remain 453/471 and all
thirteen vendor source controls remain exact.

The remaining full-build failures concern other existing data/ownership
models. COMMON allocation and the unresolved initialized section splits
remain open under the same strict gate.

## Verification

- Fresh focused compilation and objdiff: 31/35 exact; all 35 complete resolved
  bodies, ordered calls and data references equal the separate starting objects.
- Strict menu storage: all three sections match; independent target relink
  verifies the complete combined unit.
- Repository suite: 770 tests pass, nine skipped; lint and all 100 modern C
  source/image type checks pass. Nix flake checks pass, including the new
  native-compiler storage control without retail files.
- Full `kf build` runs the graph and retains 453/471 exact game functions,
  thirteen exact vendor controls and the known remaining data/ownership
  failures. The menu owner is no longer among those failures.
- Only the 31 exact menu rows are re-banked under the consolidated owner;
  partial rows are not promoted.
