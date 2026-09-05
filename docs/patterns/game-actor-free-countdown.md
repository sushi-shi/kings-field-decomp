# GAME actor free-slot countdown

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x8002ca78 actor_pool_find_free`, 60 bytes, in
`src/game/actor.c` / `game.actor`. Initial strict objdiff is 72.866670%;
GAME begins this campaign at 268/362 exact. Retail hashes were validated
with `kf init`; all six matcher semantic views were read with `--image game`.

- The sole proven caller is the adjacent `func_8002cab4` (`0x20` bytes):
  its `jal` at `0x8002cabc` has a `nop` delay slot, no incoming argument
  setup, and no result use. Its unresolved purpose is not renamed. The
  returned `$v0` in the search is demonstrably a record pointer or zero,
  supporting the existing `KfActor *(void)` signature.
- The search is a frameless five-block leaf. One validated HI16/LO16 pair
  identifies `actor_state+0x720`, the start of the shared `KfActor[128]`
  member. An unsigned byte at record offset zero is compared against `0xff`.
  The pointer advances by `0x48`, the shared actor extent.
- Retail initializes the counter to 127 and the terminal constant to -1.
  At `0x8002ca9c`, it decrements first; `bne a0,a1` tests that new value.
  The branch delay slot advances the record pointer. It therefore visits
  all 128 records when none is free, without reading past the array.
- The found branch at `0x8002ca94` targets the shared return at
  `0x8002caac`; its delay slot copies the current record into `$v0`.
  Exhaustion falls through a zero-result assignment. The `jr $ra` delay
  slot is a `nop` inside the claimed function extent.
- No calls, strings, indirect transfers, other data references or candidate
  references occur. The preceding `talk_show_indexed_image`
  (`0x8002c9d4`, 164 bytes) and following wrapper were inspected and stay
  unchanged. Address adjacency is not used to merge their owners.
- Vendor negative control: neither the target nor its actor owner is an SDK
  identity; the reviewed vendored/FID inventories exclude it. This is a
  scan of the game-owned 128-by-72-byte actor array, not a generic allocator,
  libc body, startup routine or GTE wrapper. No SDK implementation is added.
- Source history, `game_semantic_actor_core.tsv`, and the historical
  `source-shapes-gcc257.md` observations were inspected. A fresh
  `kf try --unit game.actor` differs before the reported return-join symptom:
  the compiled postdecrement loop has no -1 terminal constant and tests the
  old counter against zero. The current `count-- != 0` form is semantically
  equivalent for this fixed bound, but does not express retail's countdown.

Hypothesis: replace only the bottom condition with `--count != -1`.
Keep the pointer increment, early pointer return, null return, types, initial
127 bound, shared actor owner, profiles and relocations unchanged. Require
a fresh strict match and all 15 encoded words/ordered relocations for closure.

## Focused trials

The predecrement trial reproduces the -1 terminal constant and the retail
countdown. Its first remaining divergence is now the found-branch delay
slot: the probe decrements the counter there, then emits separate result
assignments and an extra jump around the found return. Retail instead assigns
the found pointer before its shared return join. A second focused hypothesis
expresses that decoded join with one result local and one return label,
assigning the result only on the found/exhausted paths. No new condition,
initial zero assignment or memory access is introduced.

## Final verdict

Both evidenced source facts together close `actor_pool_find_free` at strict
**100%**. `kf match --unit game.actor` recompiled the canonical object and
generated the strict report. Raw objdump comparison confirms all 15 encoded
words and both ordered relocation entries, including the `actor_state+0x720`
addend, found-branch result assignment, pointer-increment branch delay slot,
null fallthrough and final `jr`/`nop` pair.

GAME advances from 268 to **269/362 exact**, leaving 93 partial functions.
The actor unit advances from 21 to **22/29 exact**. The sole caller stays
exact, and all other 483 function report rows across the three images retain
their previous sizes and scores. Only this new exact function is selected
for banking; no vendor function is counted as new game progress.

Ruff and all 551 repository tests pass. Full `kf build` still fails the
pre-existing data-comparison, known-reference ownership and section-placement
gates: 11/59 source data owners pass, and six target units have conflicting
section bases. No gate or compiler profile was changed. `git diff --check`
passes. The historical return-join symptom is no longer an open residue.

## Other triage, unchanged

Fresh comparisons reconfirm `map_restore_floor_state`'s two address-association
differences and the change-floor/same-floor warp helpers' argument-save order
differences. No further experiments on those already recorded forms were made.
The map-event image refresher has the same predicates/referents but different
load and shared-store ordering; it was not edited. The map-event cone search
is a separate follow-up candidate: its retail keeps the incoming facing word,
whereas the current `s16` parameter adds sign-extension instructions. Its full
caller/signature audit remains necessary before any change.
