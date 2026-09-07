# Renderer literal accounting and HUD layer

## Function Match Plan

GAME `render_screen_sprite`, 0x8001e480 / 0x16c, inserts HUD quads at the
fixed ordering-table slot one. Name that layer `HUD_SPRITE_OT_DEPTH` locally.
The reviewed six-view retail dossier records its 32-byte frame, buffer-overflow
branch, two proven SDK calls, eight validated data references and no strings.
At 0x8001e5d0 it calls `AddPrim`; the delay slot adds four bytes to the
ordering-table pointer, selecting its second word. The sole proven caller,
`render_hud_gauges`, passes the sprite member of each visible HUD record.
The preceding projected-sprite renderer instead sorts by computed depth.

The function is game-owned HUD composition around separately attributed
`SetPolyFT4` and `AddPrim` providers. Source history and the material evidence
record strict 100% for the existing object. Name the slot without altering
values, statements, widths, calls, referents or delay-slot-sensitive behavior.
The original tuning rationale for choosing this particular layer is unknown.
Compilation, tests and post-edit matching remain deferred until the naming
pass finishes, as requested; this change makes no new exact or banking claim.

## Remaining renderer literals

The [packet ledger](renderer-packet-literal-ledger.md) covers GAME's four
enqueue functions and OPEN's general, map and unlit TMD renderers. Existing
TMD mode, body-length, vector-stride, texture-brightness and ordering-depth
constants already express their domain meaning. Remaining divisors/shifts
average the explicitly listed triangle or quad vertices. Area comparisons
use the mathematical zero boundary; SDK semi-transparency arguments are
boolean enables. Named minimum-depth bounds retain the original exclusive
comparison's subtract-one adjustment.

The [entity ledger](entity-render-literal-ledger.md) covers both GAME entity
emitters, their two sprite tables, the map-event emitter and the HUD emitter.
All 232 sprite descriptor components are authored data: texture coordinates
and spans followed by local rectangle coordinates and spans. Each occurrence
has a separate field-specific reason, including repeated values. Signed-looking
hex halfwords retain their original unsigned representation and subsequent
renderer conversion. The seven floor descriptors and 22 effect descriptors
retain their established extents and existing named selectors.

The [descriptor evidence](game-entity-sprite-tables.md),
[TMD domain review](source-constant-names.md#tmd-packet-and-scene-depth-constants),
[GAME enqueue evidence](game-tmd-enqueue.md) and
[OPEN enqueue evidence](open-tmd-enqueue.md) establish the retained meanings.
No mesh capacity or selector is inferred from a formula's three/four terms.
No SDK primitive body is counted as game reconstruction.

The projected GAME sprite's color address remains an explicit ownership
limitation: two halfwords past `active_render_clut` reach the measured SDK
color span. [Material evidence](render-material.md) supports that address
relationship but leaves the enclosing graphics owner unresolved. The ledger
records this reason rather than declaring a new overlapping global or claiming
that pointer arithmetic is a completed type model.


## Result and limits

The HUD's one inline literal is now the local named depth. All 403 remaining
occurrences across the seven reviewed files have individually reconciled ledger
rows: 153 in the packet/projected-sprite files and 250 in the entity/map-event
files; the HUD file has none. Source-content comparison checks duplicate tokens
and full expressions, not only totals or file mentions. The overall census is
6,109 inline occurrences, with current individual accounting for 100 files /
5,666 occurrences. These are accounting counts, not missing-name counts.

Only the HUD constant name changes executable source. Its value remains one,
and reversing that substitution recovers the prior file. No types, signatures,
initializers, data ownership, tests or toolchain settings change. Retail matching
and compilation remain deferred; source-only accounting is not binary proof.
The broader naming objective remains open, including unproved field identities
and the remaining files without complete literal ledgers.
