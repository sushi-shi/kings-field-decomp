# World-collision remainder literal review

## Function Match Plan

At `8f27a8d`, review the complete GAME world-collision and collision-grid
modules. Fresh hash-validated retail, image-qualified address/disassembly/CFG,
caller/callee/string/match snapshots, adjacent effect collision and distance
helpers, source history and current declarations constrain the audit. Direct
calls remain proven and address references retain their validated tier.
The probe stays `probe-gcc257-o2-g0`; no historical compiler claim follows.

| GAME VA / retail bytes | Function | Strict % | Review scope |
| --- | --- | --- | --- |
| 8001a29c / 432 | map_floor_height_for_cell_position | 100 | Named half-cell step and orientations; explain three flat-grid base-row zeros. |
| 8001a44c / 156 | map_floor_height_at_position | 100 | Named grid units and delegated height query; confirm no retained numeric literals. |
| 8001a4e8 / 196 | collision_adjust_cell_occupancy | 100 | Named five-by-five neighborhood and masks; explain loop exhaustion and byte-count wrap. |
| 8001a5ac / 1284 | collision_query_world | 97.943924 | Named collision classes, flags, player radius and sentinel; account for remaining sign/mask/base-row zeros and every attribute-table entry. |

All behavior constants in these modules already have names. The remaining
work is a complete per-occurrence ledger for their arithmetic and authored
data, without creating enum aliases for null/sign/exhaustion tests or
inventing identities for map geometry. Keep source, headers and curated
configuration unchanged. Verify all 255 table halfwords against retail and
both compiled/target objects, force both units, compare runtime contents and
all scores, and resolve the four bodies and ordered references. Run the
existing repository tests, modern checking, inventory, Ruff, whitespace and
full build before committing the documentation. Add no new tests or asserts.

## Attribute table is data, including its zeros

The load-image table at GAME 800558b8 contains 255 signed halfwords, covering
attribute indices 0..254. Attribute 255 is the separate missing-attribute
sentinel. The [per-occurrence ledger](game-world-collision-literal-ledger.md)
identifies every stored entry by index, literal token and source line.
Repeated values remain authored data rather than enum aliases that would
suggest independent state or asset identities.

| Stored value | Entries | Established interpretation |
| --- | ---: | --- |
| -25000 | 4 | Floor-relative scalar height. |
| -13000 | 1 | Floor-relative scalar at attribute 0x5d; its numeric geometry remains distinct from the named bottomless-pit gameplay attribute. |
| -5000 | 22 | Floor-relative scalar height. |
| -3000 | 51 | Floor-relative scalar height. |
| -2500 | 11 | Floor-relative scalar height. |
| 0 | 160 | Rectangle ordinal zero for effect collision; also participates as scalar zero in the world query. |
| 1..6 | 1 each | The other six rectangle ordinals for effect collision. |

There are four distinct consuming functions. `effect_map_collision` adds negative
values to floor Y, and treats nonnegative values as indices into the seven
rectangle records. `collision_query_world` first adds its caller's height
to the table value, then performs its ceiling test only if that sum is
negative. Therefore zero does not universally mean “no height test,” especially
for negative caller heights. `magic_cast` performs the separately documented
attribute-minus-one lookup for Lightning's aiming threshold. The jump-attack
arm of `actor_update_current_action` also uses attribute minus one, selecting
velocity and animation step around the -5000 threshold. The direct attribute
indices in this ledger must not erase either bias. See the
[current reconciliation](collision-ledger-reconciliation.md) for the actor
consumer's raw instructions and the four-file verification.

The complete 510-byte source initializer reproduces retail and both compiled
and delinked target symbol spans. Its SHA-256 is
`61b46d401d6f82e808978b121e8344f6e4055f90751591c4b969ea1cc21206d4`.
The two bytes preceding the table remain outside this owner. The original
reasons for choosing each map geometry value remain unresolved; byte equality
does not identify the associated room, model or designer's intent.

## Query and grid behavior

The world query's ten remaining numeric literals are all zeros with distinct
roles: three select the base row for flattened grid indexing, four test that
a terrain/player/actor/object skip bit is absent, one tests the signed
height sum, one detects a nonempty rejection-bit intersection and one detects
an empty occupancy count. The ledger records those roles individually.
No flag, result class, radius, tile dimension or shift remains unnamed here.

Terrain rejection precedes cell flags, then player, actor, object and event
queries in that order. Empty occupancy skips all four entity classes. The
named rejection mask 0xf0 overlaps bit 4 of the named occupancy mask 0x1f;
these are preserved measured masks, not disjoint categories. Capturing a hit
publishes its position, rotation and radius; the final six output bytes are
not written by these paths and retain their unresolved field identity.

The height helper has three flat-grid base-row zeros. Its named step height
is 300 world units; four orientations choose strict X/Z comparisons against
the 1000-unit tile center. A point exactly on the center boundary does not
receive the added step. The position wrapper has no numeric literals.

Occupancy has only two remaining literals, the row/column loop terminators.
The named radius two and span five define the visited neighborhood; the
low-five-bit count wraps through its mask while the upper three bits are
preserved. Unsigned cell bounds skip wrapped negative coordinates at map
edges, preserving the original pointer construction and iteration order.
The query's rejection overlap remains separate from that update's preserved
upper-bit mask. No arithmetic zero needs a domain enum to express these roles.

## Coverage

The complete ledger covers 265 occurrences in `collision.c` and five in
`collision_grid.c`: all 255 data entries and all fifteen function zeros.
Its exact source/line/token/expression multiset is checked independently.
These two modules require documentation completion rather than additional
constant substitutions; source, headers and configuration remain unchanged.

## Verification and verdict

All four functions retain their strict scores. The three exact grid helpers
reproduce all 196 retail instruction words and resolved target-object
referents. Across the family, all 515 candidate words, six calls and 25
ordered address references are unchanged. The world query remains at
97.943924%, with 319 candidate words versus 321 retail. Its first difference
is GAME 8001a5b0: candidate `sw s0,24(sp)` versus retail `sw s2,32(sp)`.
This preexisting residue remains unattributed and the query is not banked.

After forcing both units, all 112 objects retain every section byte and all
484 strict scores are unchanged. Every captured source/header file remains
byte-identical. The ledger's complete table payload independently matches
retail and both ELF symbol spans; its 270 source occurrences match the
generated source/line/token/expression multiset exactly.

All 680 repository tests pass (82.440 seconds), including the existing
collision data/referent and rectangle controls. Inventory, Ruff and whitespace
checks pass. Modern checking retains 65/112 passing variants and the same
300-error diagnostic multiset. Full `kf build` retains existing data/placement
failures: data matches PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1,
GAME 75/77, OPEN 34/38; six conflicting section bases and zero artifact
failures. This documentation completion does not claim a passing full build
or completion of the broader naming goal.

## Packed collision result constants

Plan: name the result kind's 16-bit shift, its low-half detail mask, and the
below-floor/ceiling details. `collision_query_world` combines terrain or object
indices with named high-half kinds; effect damage extracts the actor index,
while vertical actor handling switches on terrain details. Reviewed GAME
consumers are the 3D/2D effect helpers (0x80037fe0/0x2b8, 0x80038298/0x260),
effect dispatch (0x80038a38/0x180c), horizontal actor movement
(0x8002f31c/0x14c), and actor action dispatch (0x8002fa88/0xd90).
Existing collision/effect/actor dossiers and current call paths establish
these encoded roles. The low half may contain an index or terrain detail,
so the mask is not named as an actor-only index mask.

Name the existing ceiling detail in the special-attack high-half comparison
without correcting the mismatched half. Integer values, shifts, operand widths
and control flow remain unchanged. No build, compiler check, test or post-edit
match is run during this naming pass.

Result: 28 raw occurrences now use packed-result constants. All five
planned functions retain their value and comparison forms; the unusual ceiling
comparison still tests the high half. Source review and literal documentation
reconcile all 111 files and 5,857 retained occurrences; builds remain deferred.
