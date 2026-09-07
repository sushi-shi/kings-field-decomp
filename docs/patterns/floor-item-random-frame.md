# Floor-item random starting frame scale

## Function Match Plan

Name the shared random scaling shift in `item_load_floor_placements`:
GAME 0x80020b4c / 0x1b0 and OPEN 0x800197e4 / 0x1b0. Both retail bodies
multiply `rand()` by the unsigned packed facing/frame-count byte and shift
the low product arithmetically right fifteen before storing the frame byte.
Use `KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS = 15` in both source bodies.

Fresh image-qualified address, disassembly/CFG, caller/callee, string and
stored-match views were read alongside source history, shared placement/runtime
types, the [descriptor evidence](game-entity-sprite-tables.md), SDK declarations
and adjacent resource/renderer contracts. Each function has a 40-byte frame,
two placement traversals, a proven `rand` call, three validated data pairs and
no strings. GAME's caller is `map_resources_load`; OPEN's caller is
`opening_resources_load_scene0`. The recorded objects are 98.888885% and 100%
respectively; these are stored baselines, not verification of this edit.

GAME 0x80020ca4..cc8 and OPEN 0x8001993c..60 show the full-byte load,
multiply, `mflo`, `sra 15` and byte store. Their twelve-byte placement and
24-byte runtime strides, terminator tests and delay-slot updates remain intact.
The loaders are game resource-expansion policy, separate from the attributed
Sony random-number provider. No callee, signature, field, owner or relocation
target changes.

The flake's `psyq/include/RAND.H` defines `RAND_MAX` as 32767. Dividing the
nonnegative product by 32768 therefore selects a starting frame below the
packed-byte value. This explains fifteen as the random generator's scale,
not an animation duration or fifteen-frame capacity. The existing `rand`
declaration returns `int`; that signature is unchanged.

The full byte also contains facing bits. Retail does not mask those bits out
before multiplication, and the renderer draws before applying its frame-count
wrap. Preserve this known initial-index limitation rather than silently fixing
it or enlarging the sprite table. Placement byte +3 remains unresolved; neither
the scale name nor the random calculation establishes that byte's purpose.

Two raw shift occurrences become one shared named constant. The remaining
OPEN loader literal is its zero count reset, documented in the
[small-module ledger](small-module-literal-ledger.md); the existing GAME item
ledger is reconciled. Builds, tests, post-edit matching and banking remain
deferred until the naming pass finishes, as requested.
