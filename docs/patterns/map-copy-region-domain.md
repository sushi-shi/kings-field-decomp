# Map-copy region domain

## Function Match Plan

Give the map-copy selector a byte enum domain, name the five authored table
entries and preserve the existing absent selector 255. Carry the domain through
the helper signature and all nine calls: eight direct named selections and one
explicit decode from the object's overloaded action-parameter byte. Encode only
when indexing the region table. Preserve all thirty descriptor bytes and the
order of the five map-layer copies.

All addresses select GAME.EXE. Current source, source history, cached complete
retail/CFG/caller/callee/string dossiers and the
[map-copy descriptor evidence](game-map-copy-region-source.md) were reviewed.
These are game-authored map changes, not vendored library bodies.

| Function / extent | Evidence and change |
| --- | --- |
| `map_apply_copy_region`, `0x80030a98 / 0x1e4` | Leaf with 40-byte frame, eight validated address pairs, no calls or strings. Input is narrowed to a byte, compared with 255, and scaled by six for the descriptor at `0x800561b0`. Type its argument and encode at indexing; keep its loops and delay slots. |
| `map_object_pool_update`, `0x80031cc8 / 0xc18` | Copy action calls at `0x80032720` use the action-parameter byte after its absent-value guard. Completion of four floor-3 piece reveals calls region 2 then 3 at `0x800327f8`/`0x80032800`. Decode the former and name the latter, preserving the action state and order. |
| `map_action_script_floor1`, `0x800343e0 / 0x58` | A 24-byte-frame routine with two proven calls and three data pairs; possession of Dragon Chalice sets the passage-opened flag and calls region 1 at `0x80034410`. Name that selection. |
| `map_ambient_script_floor5`, `0x800342ec / 0xf4` | A 24-byte-frame routine with five proven calls and eleven data pairs. Its two TALK images and boss-animation setup precede region 4 at `0x800343c8`. Name the boss-encounter copy; preserve the cell/yaw trigger and strings. |
| `map_restore_floor_state`, `0x80035e44 / 0x69c` | Restore repeats passage region 1, floor-3 reveal regions 2/3 and encounter region 4 at `0x800362d8`, `0x800363c0`, `0x800363c8`, `0x80036438`. Use the same names as the live triggers. Its prior [progress review](progress-controlled-map-links.md) records the restore evidence. |

## Authored region identities

| Index | Name | Source X,Z → destination X,Z; extent |
| --- | --- | --- |
| 0 | `KF_MAP_COPY_FLOOR1_GRAVESTONE` | (55,33) → (50,39); 3×3 |
| 1 | `KF_MAP_COPY_FLOOR1_PASSAGE` | (47,16) → (30,20); 3×3 |
| 2 | `KF_MAP_COPY_FLOOR3_REVEAL_FIRST` | (58,44) → (15,48); 3×3 |
| 3 | `KF_MAP_COPY_FLOOR3_REVEAL_SECOND` | (64,44) → (37,45); 3×3 |
| 4 | `KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER` | (0,0) → (36,4); 7×1 |

First/second describe the observed order of the two floor-3 completion copies;
no room names or separate puzzle stages are inferred. The existing passage and
encounter flags establish the roles of entries 1 and 4.

For entry zero, decode the shipped twenty-byte object placements and common
eight-byte definitions after validating the configured retail directory. The
only live copy-behavior placements are B1 slots 97 and 98, both gravestone model
89. Slot 97 has an absent region (255). Slot 98 at cell X 51, Z 42 carries
link 54 and region zero. The placement loader starts copy-region action for
definition behavior ten; after the link is cleared, that action applies the
decoded region once. This supports the gravestone role without guessing whose
grave it is. The parameter byte retains integer storage because other actions
use it for pool indices and unrelated payloads.

The enum names the five table entries and the absent selector; it introduces no
new range validation. Authored coordinate/extent literals remain data with their
existing ledger reasons. Builds, compiler checks, tests, post-edit matches and
banking remain deferred until the naming pass finishes.

## Source result

Eight raw call arguments now have names; all nine calls carry the region domain.
The helper retains byte storage, with explicit decoding for the overloaded
object parameter and encoding for table indexing. Reverse substitution of only
these names and boundaries reproduces the previous four C files and shared
header exactly; descriptor values, branches and copy order are unchanged.
The curated helper signature now uses the same domain without changing its
confidence. The source inventory accounts for all 111 C files and 6,073 retained
literal occurrences, with no missing or extra ledger rows.

Each of the five functions above remains unverified after this edit: no build,
compiler check, test, match or banking was performed. Source accounting and
reverse substitution establish the scope of this naming change, not a binary
match. Remaining unknown fields and selector/type work keep the naming goal open.
