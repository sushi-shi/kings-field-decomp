# GAME collision grid relocation-site correction

## Function Match Plan and evidence snapshot

The known-reference audit reports two invalid live, reviewed HI16/LO16 rows
inside GAME.EXE `collision_query_world`, 0x8001a5ac..0x8001aab0 (body 0x504,
one fragment, `game.collision`). Starting strict function score: 93.190030%.
This campaign corrects relocation sites only: retain the C body, signature,
data extents, identities and ownership. Rebuild the affected object and compare
ordered referents, then verify all images and every historically exact function.
Do not bank this non-exact function or turn these references into a claim of
complete grid allocation coverage.

Reproduction uses the hash-verified retail GAME.EXE and pinned Nix tools:

```sh
kf init --retail-dir <retail-disc-directory>
kf sema --image game addr collision_query_world
kf sema --image game disasm collision_query_world --blocks
kf sema --image game xref collision_query_world
kf sema --image game xref collision_query_world --callees
kf sema --image game strings collision_query_world
kf sema --image game match collision_query_world
kf sema --image game addr 0x8001a4e8
kf sema --image game addr 0x8001aab0
```

The full CFG has 49 blocks. It divides X/Z by 2000, uses a signed 16-bit Z
quotient and a flat `x + 100*z` cell index, loads byte grids, and uses signed
halfword offsets from `map_cell_attribute_height_table` (0x800558b8, 0x238
bytes). Terrain rejection precedes player, actor, map-object and map-event
tests. Flag 0x800 copies the selected transform/radius to `collision_target`;
the return encodes the class/index or 0xffffffff. The five direct calls are
`map_floor_height_for_cell_position`, `player_distance_to_point`,
`actor_pool_find_overlap`, `map_object_pool_find_near_point`, and
`map_event_pool_find_overlap`. Eight internal jumps target the shared epilogue
at 0x8001aa84. `jr ra` at 0x8001aaa8 owns the stack-restoring delay slot at
0x8001aaac. No call, branch, return or delay slot is changed.

All 16 external call sites were rechecked with their argument-setup windows:
0x800172c4, 0x8002e9d4, 0x8002ea3c, 0x8002ead4, 0x8002f37c,
0x8002f738, 0x8002f790, 0x8002f7d4, 0x8002f814, 0x80030154,
0x80030234, 0x80030734, 0x80030f04, 0x8003579c, 0x80037fa4,
0x80039ed4. O32 a0-a3 carry coordinates/radius, with height and flags at
caller sp+16/sp+20 (callee sp+80/sp+84 after its 64-byte frame). The existing
six-argument `u32 (s32, s32, s32, s32, s32, u32)` contract is retained. The
earlier semantic dossier's "24 confirmed callers" includes eight internal
epilogue jumps, not 24 external calls.

There are no referenced strings. Adjacent functions are
`collision_adjust_cell_occupancy` at 0x8001a4e8 (0xc4 bytes) and the exact
`memory_malloc_checked` at 0x8001aab0 (0x38 bytes); this does not imply shared
original TU ownership. The vendor negative control remains the evidence in
`game_semantic_player_interactions.tsv`: no vendored/FID match, and the pool
traversal, map-grid policy and encoded object classes are game-specific.
`git log -S '0x8001a680' -- config/retail/relocs.tsv` locates both mistaken
reviewed rows in `fd7081d`; their targets were already the correct grid bases.

## Raw-byte correction

| HI site / word | LO site / word | Decoded target | Existing identity |
| --- | --- | --- | --- |
| 0x8001a67c / 3c01800a | 0x8001a680 / 24218018 | 0x80098018 | map_collision_grid |
| 0x8001a6dc / 3c01800a | 0x8001a6e0 / 2421a748 | 0x8009a748 | map_cell_attribute_grid |

Both lows are signed `addiu at,at,imm`: 0x800a0000 - 0x7fe8 and
0x800a0000 - 0x58b8 respectively. The following instruction words are
0x00220821 / 0x90290000 (`addu at,at,v0; lbu t1,0(at)`) and
0x00310821 / 0x90240000 (`addu at,at,s1; lbu a0,0(at)`). The old pairs
0x8001a680/0x8001a684 and 0x8001a6e0/0x8001a6e4 incorrectly designated
ADDIU/ADDU as LUI/ADDIU. Correct the two HI sites, paired sites and file offsets
by -4, without changing targets, signed-low opcodes or confidence policy.

The existing third pair, 0x8001a73c/0x8001a740, correctly refers to
`map_collision_flag_grid` at 0x800668e8 and remains untouched. Other consumers
corroborate the collision grid base, including 0x8001a2cc/0x8001a2d0.

`tests/test_delink.py` checks both curated rows against the recorded retail
instruction windows. The shared safe validator rejects the old shifted pairs;
delinking emits the two external-symbol HI16/LO16 pairs with zero addends and
preserves the indexed byte-load instructions. Applying the linked targets to
those addends reproduces all four original HI/LO instruction words, including
the carry-adjusted high halves. This is a focused relocation round trip, not
a complete object or executable round-trip proof.

## Final verdict

The regenerated target has external HI16/LO16 rows at function-relative
0xd0/0xd4 and 0x130/0x134 with the correct grid names. Both live references
are now `validated`, removing the two `invalid-live-reference` diagnostics;
the safe validator was not relaxed. The strict objdiff function score changes
from 93.190030% to 93.202490%, still non-exact. The first raw instruction
divergence remains the saved/moved coordinate register at +4/+8 (`s2` in
retail, `s0` in reconstruction), followed by register/order and later
aggregate-addressing differences. No optimizer mechanism is attributed.

All 75 GAME units were rebuilt following target regeneration. All 117 source
object hashes remain unchanged, as do the other 483 reported function scores;
all 354 historically exact game functions remain exact. No baseline is banked.
`validate_config`, all 420 local tests (no skips), Ruff, `git diff --check`, and
`nix flake check -L` pass. The isolated flake suite also runs 420 tests, with
46 expected skips for unavailable local retail/oracle artifacts.

The full `kf build --reconfigure` was followed by explicit GAME comparison
(the initial build stopped on PSX/OPEN verification) and a second full
`kf build`, which ran all three image checks. It still fails the existing
known-reference ownership gaps and the same 14 GAME/OPEN data-addend
differences. Strict data remains 49/63 owners. This correction neither hides
those failures nor establishes exhaustive reachable-byte coverage.
