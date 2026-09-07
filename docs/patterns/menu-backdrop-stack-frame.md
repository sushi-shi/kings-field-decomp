# GAME menu backdrop stack-frame campaign

## Function Match Plan: shared 64-byte debug lifetime

On master `ec7018b`, GAME `8002a510 menu_draw_window_backdrop` is 1700 bytes
and strict 99.971760%.  Its retail and candidate streams have the same
single-block CFG, sixteen calls, ordered data references, constants and body
instructions.  Only the frame differs: retail reserves 104 bytes and saves
`s0`-`s3`/`ra` at 80-96, while the probe reserves 40 bytes and saves them at
16-32.  All six image-qualified semantic views, its three callers, adjacent
functions, menu asset structures, SDK providers, source history and current
unit diff were refreshed.  There are no strings or candidate references.

The adjacent `800292f8 menu_draw_item_name_frame` has the same 64-byte frame
deficit (224 retail versus 160 probe); fourteen other functions in
`game.menu_runtime` are exact.  This repeated, function-selective extent is
evidence for a shared eliminated local-object lifetime rather than arbitrary
padding.  First test the same GCC 2.5.7/O2/G0/R2000 profile with debug
information enabled, because old GCC can retain addressable local storage for
debugging even when optimized body accesses disappear.  Compare both affected
functions and all fourteen exact controls.  Do not retain the profile unless
it explains the two 64-byte extents without code regressions.  If it does not,
restore configuration before investigating a concrete shared source object.

The debug-profile control was negative.  Adding `-g` left both affected frame
sizes unchanged at 40 and 160 bytes, while debug-local relocation labels made
only six of the unit's sixteen function listings compare identically.  The
profile therefore does not explain the retail frames and was removed.  The
remaining source hypothesis must account for a real 64-byte local extent
shared by these two routines without disturbing the fourteen exact controls.

GCC 2.6.0 is also excluded for this translation unit.  It preserves the same
40- and 160-byte candidate frames instead of adding the missing extent, and
changes every one of the sixteen function listings through allocation,
scheduling, division lowering, and return-delay differences.  GCC 2.5.7 is
therefore retained; the 64 bytes must come from missing source lifetime or
structure rather than either supplied alternate profile.

A real `KfSpriteQuad` local describing the first backdrop tile adds one
16-byte stack slot, consistent with four tile descriptors accounting for the
64-byte extent.  It does not scalarize, however: the probe emits halfword
stores and reloads at `sp+20..26`, grows the frame only to 56 bytes, and
substantially changes the tile body.  `register` is byte-identical to that
failed aggregate form.  Retail has none of this stack traffic, so the typed
tile local is rejected and the direct packet construction is restored.
