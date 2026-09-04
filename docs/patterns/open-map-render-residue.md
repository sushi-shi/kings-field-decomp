# OPEN map rendering: ownership and setup residues

Witnesses: OPEN `render_map_cell`, `0x80018bbc`/`0x1d0` bytes, and
`opening_render_map_cells`, `0x80018d8c`/`0x140`, under the existing
`probe-gcc257-o2-g0` profile. These observations do not identify the
historical compiler or prove an optimizer mechanism.

## Visibility-window ownership

`render_initialize` passes `0x800439d8` to `cd_file_load_into` for
`B0\\RTBL.`. The file contains exactly `0xcc0` bytes, SHA256
`b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72`.
Sixteen records of `0xcc` bytes each contain four little-endian halfwords
and 196 visibility bytes. All widths/heights are 14; origins vary with yaw;
cell values are 0, 1, or 2. The selector computes
`(15 - (view_rotation.vy >> 8)) * 204`. This supports the existing GAME
`KfCellWindow` layout, now declared in the shared render-type header.

The map pass publishes `const KfCellWindow *active_cell_window` at
`0x8006e1c8`; both map traversal and entity/item culling read it. The file
proves the logical table extent, not an original C file boundary.

## Conflicting matrix-base evidence

The per-cell renderer establishes `s0 = 0x8006e0c8` at
`0x80018d2c/0x80018d30`: `render_state.quadrant_matrices`. At `0x80018d54`
it adds `0x80` to that saved base. The `SetLightMatrix` call at
`0x80018d58` adds orientation times 32 in its delay slot, reaching
`0x8006e148 + orientation * 32`, the light-quadrant array.

The separate-array source emits another HI16/LO16 pair for that address
and uses different saved registers. This is a referent-expression and
ownership discrepancy before any explanation of register selection.
Numeric targets and the complete call set agree.

A controlled experiment extended `KfRenderStateOpen` by the four light
matrices at offset `0x100`, updating both the cell renderer and initializer
to use that one field. It reproduced the cell renderer's saved-base chain.
However, the exact initializer then replaced four independent HI16/LO16
pairs with offsets `192`, `224`, `256`, and `288` from its light-matrix
base and changed call delay slots. The aggregate did not jointly explain
both witnesses under this probe and was reverted.

The split storage model remains provisional. Do not introduce incompatible
per-file views, cross-object pointer arithmetic or aliases to reproduce
one witness. Further progress needs source-ownership or compiler/linker
evidence explaining both forms. Retained per-cell C is 84.094826%, with
coordinate temporary/load scheduling also different from retail. This is
not a proven register or scheduler wall.

## Traversal setup

Retail stores the selected pointer at `0x80018de4`, derives its cell
pointer at `0x80018de8`, then reloads the selected pointer at
`0x80018dec/0x80018df0` before loading view-cell coordinates and subtracting
window origins.

The probe eliminates that reload and hoists the two view-cell loads above
window-index arithmetic. Moving the C cell-pointer initialization before
coordinate initialization, following retail order, produces the same
instructions. Compiled code is eight bytes shorter. The row/column loops,
masks, skipped-row stride, nonzero-cell test and calls agree after the
shifted setup. Strict objdiff is 84.062500%.

This setup is an unattributed codegen residue. No volatile pointer,
redundant operation or forced call was added. Both functions remain
unbanked; their individual verdicts are in
`config/evidence/open_semantic_map_render.tsv`.
