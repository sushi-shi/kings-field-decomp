# Effect homing mode

## Function Match Plan

Carry the two established homing selectors through an unsigned-byte enum and
both constructor paths. Compare that field directly in the dispatcher, and
encode named values where callers supply integer payloads. Rename the numeric
randomization label to describe its operation.

All addresses identify GAME.EXE. Current source and the cached retail protocol,
constructor and dispatcher dossiers were reviewed. The surrounding effect-kind
cases reinterpret the same control union as counters or parent indices; those
members stay in their separate domains. The [effect protocol](game-effect-protocol.md)
records the caller/callee and data-reference context for these operations.

| Function | Address / extent | Evidence snapshot and intended change |
| --- | --- | --- |
| `player_update` | `0x80018880 / 0x1a1c` | Missing target sets word register s2 to 255 at `0x8001977c`; otherwise the payload is an actor pointer difference. Another effect path uses burst parity in this same local. Encode the named wander value to its existing word payload without typing that mixed local as a homing-only enum. |
| `actor_spawn_action_effect` | `0x8002edd4 / 0x454` | `0x8002f178` supplies 254, stored in argument seven at `0x8002f17c`, before constructing visual variant 24. Encode the player selector as an integer variadic argument. |
| `effect_pool_construct` | `0x80036f44 / 0x82c` | Variant 24 and ordinary kind 20 converge at `0x80037668`; `lbu` at `0x800376a4` reads argument seven and `sb` at `0x800376ac` stores record +0x38. Decode the same byte into the enum in both source cases. |
| `effect_update_dispatch` | `0x80038a38 / 0x180c` | `lbu` at `0x800396f8` reads the selector. 255 takes random steering, 254 steers toward camera/player coordinates, and every other byte performs a fresh actor-cone query. Use typed comparisons and name the shared random-direction label. |

## Domain and boundaries

`KfEffectHomingMode` has byte storage and names the existing wander 255 and
player 254 selectors. Other byte values remain representable; there is no single
actor-search selector to invent. Although the player passes an actor index in
that category, the dispatcher does not use it as an index or retain that target.
The cone query selects a target afresh. No actor-index enum conversion is valid
for this stored mode.

The player's shared `attachment` word remains an encoded payload because its
other producer supplies burst parity to other effect kinds. The wander constant
is encoded as `s32` on assignment; the existing dynamic payload and forwarding
stay intact. The actor's direct player-mode argument is encoded as `u8`, then
promoted to integer in the variadic call. Constructor decoding is the boundary
where either payload acquires its byte homing domain. Both dispatcher comparisons
then consume the enum directly.

The byte representation, union extent, random thresholds and steering equations
remain unchanged. The pre-existing player-homing pitch path reads an uninitialized
distance slot; this naming change does not alter that behavior.

Cached match states describe historical source only: player update 96.94554%,
actor effect spawning and construction 100%, dispatcher 96.93957%. Builds,
compiler checks, tests, post-edit matches and banking remain deferred until
naming finishes. Current source propagation is not a new binary-match claim.

## Source result

All four planned functions have the same final verdict: source propagation
reviewed; compiler and post-edit matching deferred. Both constructor byte reads
now decode `KfEffectHomingMode`, both steering comparisons consume the typed
field, the two named caller values have explicit integer encodings, and the
shared randomization label describes homing direction. The curated field row
keeps its one-byte extent and evidence tier.

Source/reference review and `git diff --check` completed. All 111 source files
and 5,997 retained numeric/character occurrences reconcile with current ledger
expressions and reasons. The actor call row and both constructor rows reflect
the new explicit boundaries. No build, compiler check, test or banking ran.
There remain 14 `unknown_` tokens on 10 source lines; their unresolved consumers
and the outstanding resource selectors keep the broader naming goal open.
