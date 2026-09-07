# Collision ledger reconciliation

## Scope and evidence

At `5778985`, reconcile the existing world/grid and magic/effect-collision
ledgers against all four current C sources. Their six-column tables already
account for every occurrence, but the common coverage audit only accepted
five-column ledgers. Read the separate source and function columns without
changing their meaning. No reconstructed function, type, data initializer,
retail claim or source literal changes in this review.

The nine functions have fresh GAME address, CFG/disassembly, caller, callee,
string and match snapshots, plus source history and the adjacent-family
evidence in the [world review](game-world-collision-literals.md) and
[magic review](game-magic-collision-constants.md). They are game-owned map
and effect policy; no SDK or runtime implementation is counted as game work.

## Fourth height-table consumer

The earlier world review omitted the jump-attack arm of GAME
`actor_update_current_action` (`8002fa88 / 0xd90`). Its byte attribute is
doubled at `80030000`; the HI16/LO16 pair at `80030004/80030008` addresses
`map_cell_attribute_height_table - 2`. The `lh` at `80030010` therefore
reads `table[attribute - 1]`, just as Lightning aiming does in `magic_cast`.
The load-delay `nop` precedes `slti v0,v0,-4999` at `80030018`.
Branching on that result selects the `<= -5000` arm; the source expresses
the other arm as `> ACTOR_JUMP_HEIGHT_THRESHOLD`.

The greater-than arm stores vertical velocity -220 and animation step 140;
the other stores -300 and 110. Both set jump vertical state 4. The final
animation-step store is the delay slot of the call to
`actor_prepare_charge_toward_player` at `80030048`. These observations explain
the selector and operands; they do not establish why the designers chose them.
No actor code changes or new actor matching claim are made here.

There are four consuming functions, with two indexing conventions:

- `collision_query_world` and `effect_map_collision` use the direct attribute.
- `magic_cast` and actor jump selection use attribute minus one.

The attribute-zero biased access points before the table owner; retain this
fact without inventing a guard, sentinel entry or valid-input guarantee.
The existing relocation control checks both biased pairs against retail.

## Complete retained-literal coverage

| File | Occurrences | Complete ledger |
|---|---:|---|
| `collision.c` | 265 | [World/grid](game-world-collision-literal-ledger.md) |
| `collision_grid.c` | 5 | [World/grid](game-world-collision-literal-ledger.md) |
| `effect_map_collision.c` | 43 | [Magic/effect](game-magic-collision-literal-ledger.md) |
| `magic.c` | 9 | [Magic/effect](game-magic-collision-literal-ledger.md) |

All 322 token/expression occurrences, including duplicates, agree with the
current source and have an individual reason. This adds four files to verified
coverage: **78 files / 4,748 occurrences**. The global census remains 111 C
files and 6,515 occurrences; this accounting change names no new literals.

The scalar table has 255 signed halfwords (510 bytes); the rectangle table
has seven four-halfword records (56 bytes). Existing controls compare the
complete source initializers, compiled symbol spans and delinked symbol spans
with retail and their recorded hashes. All 566 bytes agree. The two-byte gap
after the scalar table remains unclassified. The reversed Y bounds of
rectangle rows 4–6 and the effect helper's unresolved fallback result 1
retain their prior documented meanings; no enum identity is invented.

## Current function verdicts

| GAME VA | Function | Strict % | Verdict |
|---|---|---:|---|
| `8001a29c` | `map_floor_height_for_cell_position` | 100 | Exact, raw retail and target agree |
| `8001a44c` | `map_floor_height_at_position` | 100 | Exact, raw retail and target agree |
| `8001a4e8` | `collision_adjust_cell_occupancy` | 100 | Exact, raw retail and target agree |
| `8001a5ac` | `collision_query_world` | 97.943924 | Unchanged partial |
| `80037850` | `effect_map_collision` | 98.221054 | Unchanged partial |
| `8003a244` | `effect_pool_reset` | 100 | Exact, raw retail and target agree |
| `8003a274` | `magic_load_records` | 100 | Exact, raw retail and target agree |
| `8003a2a0` | `magic_cast` | 98.89145 | Unchanged partial |
| `8003a760` | `effect_pool_sweep` | 100 | Exact, raw retail and target agree |

The four units were forced to rebuild. Allocated sections, runtime symbols
and ordered relocations agree with the captured objects. All nine functions
retain their 1,342 candidate words, 22 calls and 64 address references;
the six exact functions reproduce 250 complete retail words and their
delinked targets. The three partials retain these first divergences:

| GAME VA | Candidate | Retail |
|---|---|---|
| `8001a5b0` | `sw $s0, 0x18($sp)` | `sw $s2, 0x20($sp)` |
| `800378e4` | `beqz $v0, 0x800379a4` | `beqz $v0, 0x800378fc` |
| `8003a2c0` | `bnez $v0, 0x8003a740` | `bnez $v0, 0x8003a74c` |

These remain unattributed residues, not newly closed functions. All 484
current match scores agree with the campaign snapshot. No banking.

Inventory, Ruff and whitespace checks pass. Modern checking remains at
300 errors and 65/112 passing source/image variants. All 684 repository tests
pass in 123.338 seconds, including the existing collision payload and biased
relocation controls. Full `kf build` still fails on existing data/ownership
differences: source-data matches are PSX 0/1, GAME 9/42 and OPEN 3/19; target
relink verifies PSX 1/1, GAME 75/77 and OPEN 34/38. Six conflicting section
bases remain, with zero data artifact failures.
