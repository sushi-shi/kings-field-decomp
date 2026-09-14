# GAME material identities and dispatcher correction

The six-function evidence pass is recorded in
`config/evidence/game_semantic_render_material.tsv`. The shared values at
`GAME.EXE:0x80095058..0x80095060` have supported meanings:

| Address | Identity | Evidence |
| --- | --- | --- |
| `0x80095058` | `active_render_clut` | unsigned halfword copied to GPU CLUT fields |
| `0x8009505a` | `active_render_tpage` | unsigned halfword copied to GPU texture-page fields |
| `0x8009505c..0x8009505e` | `active_render_red`, `active_render_green`, `active_render_blue` | byte loads copied to POLY_FT4 RGB fields |
| `0x8009505f` | `active_render_code` | primitive command byte copied before depth-cued lighting |

The pinned Psy-Q `LIBGTE.H` declares `CVECTOR` as four bytes `r,g,b,cd` and
declares the second argument of `NormalColorDpq` as `CVECTOR *`. `LIBGPU.H`
supplies the authentic packet fields. The sprite renderer materializes the
CLUT base at `0x8001e358`, reads it with `lhu`, and advances that base by four
in the delay slot at `0x8001e404` before calling `NormalColorDpq`. This proves
the colour span and its relationship to the CLUT; it does not by itself prove
the original complete C object boundary.

The retained change names all six values and moves their declarations from
the global-state catch-all to `game/render.h`. All 27 existing relocation rows
retain their sites, targets, opcodes, classifications and provenance; only
their identity spelling changes. No computed offset is promoted to a new
relocation. No new BSS definition or completed aggregate is claimed.

## Aggregate experiments remain open

Two source models were tested under the existing per-unit GCC 2.5.7 profiles:

- An eight-byte `KfRenderMaterial` containing two `u16` selectors and `CVECTOR`
  matched strict data and preserved the screen sprite. It changed
  `render_frame` from 100% to 97.15031%: the probe saves an additional register
  and retains a material pointer across calls, whereas retail rematerializes
  the stores independently.
- Separate selectors and a four-byte `CVECTOR` left the frame at 97.89877%.
  Its first difference is the HUD setup: the probe retains the blue-component
  address across the HUD and notification calls. The three tentative BSS
  definitions also appeared at object offsets 0, 8 and 16, producing a
  32-byte section against the curated 16-byte section, including tail alignment.

Neither experiment is retained. These results do not disprove the colour
semantics or prove a compiler mechanism. They leave the original enclosing
graphics object and compiler attribution unresolved. The sprite's CLUT-based
colour view is explicitly still WIP, not a finished typed model. This work
does not close the wider ownership investigation described in
[notification-state.md](notification-state.md).

## Dispatcher semantics

`render_entities` tests effect-record byte zero and sprite ID against `0xff`.
Retail branches at `0x8001f5b4` and `0x8001f5c4` both target `0x8001f698`,
which increments the record pointers and decrements the 48-record loop count.
The old source used `break`, incorrectly dropping all following records.
It now uses `continue`, with the pointer advance in the loop increment.

The visible-window origins are named `window_origin_x` and `window_origin_z`
from their subtraction and cell-bound uses. Floor items use the existing
`floor_items` identity rather than a pointer 62 bytes beyond the texture-page
scalar. Retail's shared-base calculation remains evidence for a larger object;
the new source intentionally does not invent that object's extent.

The dispatcher is 91.25%, versus 92.96023% before these corrections. The first
raw difference is the map-object base register and window-setup schedule;
the floor-item pass additionally materializes the named array independently.
The corrected effect guards now reach the loop increment in both objects.
This is a semantic improvement, not an exact match claim.

The other five consumers retain their prior scores, including both exact
renderers. All 340 exact game functions remain exact (GAME 260, OPEN 79,
PSX 1), and all 55 data-owning units pass strict data comparison. The existing
vendor exclusions remain unchanged. The subsequent
[TMD count-width campaign](tmd-counts.md) corrects the source halfword
primitive count where the three retail polygon emitters read a word.
Broader TMD ownership and packet-dispatch differences remain unresolved.
