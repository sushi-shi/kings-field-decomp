# Reusable game library

Reusable game code belongs here even when only one linked program uses it.
For example, the fixed-point math family includes GAME-only helpers alongside
functions also used by OPEN, while the matrix diagnostic is linked only into
OPEN. This organization is not proof of an original TU.

Library translation units retain each image's claims and compile at the same
position in each image's linked order. `lib/graphics.h` selects the
corresponding image's state members; it introduces no runtime indirection.

Sixteen literal `.inc` fragments remain where at least one image interleaves
the reusable body with image-specific functions or storage. Their eventual TU
ownership is still unresolved, so they stay at their existing positions until
that surrounding code is reviewed. The pending set is recorded in
`docs/shared-fragment-review.tsv`.

The include-only OPEN sources for the debug sink, floor-item loader, and sound
reference player are real compiler units after preprocessing. OPEN links each
shared body as a separate object, while GAME currently owns the same body
inside a larger contiguous unit. Splitting the GAME units would assert object
boundaries that the retail executables do not yet prove.

Same names or similar instructions alone do not establish equivalent behavior.
The [GAME/OPEN accounting](../../docs/patterns/shared-game-open-code.md) records
the consolidated functions and the image-specific implementations retained.
