# Shared reconstruction

Reusable game code belongs here even when only one linked program uses it.
For example, the fixed-point math family includes GAME-only helpers alongside
functions also used by OPEN. This organization is not proof of an original TU.

Shared bodies retain each image's claims, authentic types, and data owners.
Literal `.inc` implementation fragments let the original translation units
include a body at its existing position, preserving function order and pooled
strings without duplicating the implementation. `shared_graphics.h` selects
the corresponding image's state members; it introduces no runtime indirection.

Same names or similar instructions alone do not establish equivalent behavior.
The [GAME/OPEN accounting](../../docs/patterns/shared-game-open-code.md) records
the consolidated functions and the image-specific implementations retained.
