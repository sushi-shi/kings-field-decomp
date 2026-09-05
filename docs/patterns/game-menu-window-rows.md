# GAME menu-window row addressing

## Function Match Plan

GAME `0x80028914 menu_draw_window`, 348 bytes/87 words, single-function unit
`game.menu_draw_window`, current `probe-gcc257-o2-g0`. At `80add05`, canonical
strict objdiff is 86.908040%; the source compiles to 352 bytes/88 words.
Retail has a 48-byte frame with ra and s0..s5 saved. The reconstruction adds
s6 and moves ra from stack +40 to +44, while retaining the same frame size.

The six semantic views, raw target and compiled body, all 31 caller windows,
neighbor boundaries, shared menu types, layout-loader evidence and source
history precede this experiment. The four arguments remain full-width
kind/count/highlight/flag in a0..a3; callers select kinds 0/1/2/4/5/6/7 or
forward the prompt's kind and pass counts, cursor indices and 0/1 confirmation
state. None consumes a result. All calls are proven, with no candidate call
or address references in the function. This is custom menu composition,
absent from the vendor/FID inventory, and has no SDK calls, strings, indirect
transfers or switch tables.

The six direct calls are two translucent blits, two string draws, one opaque
cursor blit and the backdrop helper. Nine validated HI16/LO16 pairs name
`menu_window_layouts`, `display_state+0x20`, `current_poly_ft4`, and menu asset
members +0x33c, +0x30c, +0x348 and +0x384 in observed order. No relocation or
data-ownership correction is indicated. Keep the 264-byte layout, 24-byte
labels, ten-row capacity, optional nonzero-title guard, positive-count loop
guard, both highlighted-row tests, flag==1 test and kind!=6 backdrop policy.

Retail retains the layout base and a row offset starting at 24, comparing
against `highlight*24+24`, and recomputes their sum in all three row-call
delay slots. The current local `label` caches that sum across calls. The
first experiment removes that local and spells `&layout->rows[row]` directly
at each call, keeping the typed owner, index bounds and background selection
unchanged. Do not restore byte-array arithmetic or split the complete menu
asset owner to force code generation. Inspect the first raw divergence after
the focused build before making a second independent change.

Separate observed differences are the title load (`lh` retail, `lhu` source)
and the confirmed-background address (fresh symbol pair retail, base+12
source). A zero test alone does not prove all shared coordinate fields signed;
do not globally change those types to address one instruction. The old source
history's optimizer-wall labels are not established compiler attribution.
Require canonical strict comparison, ordered relocations, full build,
all-image regression checks and existing lint/tests before committing. Only
strict 100% may be banked.

## Focused controls

Direct row expressions do not recover retail's repeated call-slot additions.
The compiler still caches the row address, merely exchanging s0/s1 roles for
the row offset and pointer; its extra saved register and address-form residue
remain. Revert that control. Separately interpret the title's existing
halfword as `s16` at the zero-test boundary, matching the directly observed
`lh` without changing any shared layout or signedness of the draw APIs.
Retain this only if its emitted load actually changes toward retail.

## Retained verdict

The signed title interpretation changes exactly one compiled word at +0x50,
`lhu v1,0(s3)` to `lh v1,0(s3)`, recovering the retail opcode. Every other
word and all 22 existing ordered relocations are unchanged from `80add05`.
Canonical strict objdiff improves **86.908040% to 87.540230%**. The direct
row-address experiment is not retained.

The function remains 88 compiled words versus retail's 87. Remaining
differences include the extra saved row pointer/register, call-slot address
formation, default-background lifetime and the confirmed-background load:
the source derives +12 from `row_background`, whereas retail materializes
the +0x348 member independently. Consequently retail has 24 relocation rows
versus 22, but the derived address still reaches the same proven complete
asset member. This is not permission to split the owning data or remove the
retail pair. Loop back-edge scheduling also differs. These residues remain
unattributed; no exact or banking claim is made for this renderer.

The complete 31-caller and neighbor audit confirms the previous prompt's
return at 0x8002890c and restoration slot at 0x80028910, followed by this
function; `menu_list_render` begins at 0x80028a70. Full-image regression and
build results are recorded with the concurrently completed
[event-image helper](game-event-progress-image.md). No shared coordinate
type or function signature was changed.
