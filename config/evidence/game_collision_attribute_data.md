# GAME attribute heights and collision rectangles

## Function Match Plan

Baseline `b3d5b4a`, GAME.EXE only, existing `probe-gcc257-o2-g0` profiles.
Work remains on `codex/data-matching-20260905` in its separate worktree.
The main agent read complete retail disassembly/CFG, incoming/outgoing references,
strings, current strict matches, adjacent functions and source history before
editing. Retail was reinitialized and hash-checked with `kf init`.

First separate scalar heights from the seven four-halfword records and correct
three named referents without changing their linked targets. Rebuild and compare
this step before correcting the independently evidenced orientation dispatch.
No TU merge, compiler-profile change, fake padding or banked-score update.

### Per-function evidence snapshots

- `collision_query_world`, 0x8001a5ac, 0x504 bytes, `game.collision`,
  93.202490%: body unchanged. Six O32 arguments are signed x/y/z/radius/height
  and unsigned flags; v0 is an encoded collision result. All 16 external call
  sites and the five callees were rechecked (listed in
  `game_collision_grid_relocations.md`). The 49-block CFG excludes attribute
  0xff before signed-halfword lookup; heights are added to the query/floor.
  Return at 0x8001aaa8 owns the stack-restoring delay slot. No strings.
  Adjacent occupancy update and allocator do not prove a shared original TU.
- `effect_map_collision`, 0x80037850, 0x76c bytes,
  `game.effect_map_collision`, 71.711580%: a0 is VECTOR*, a1 signed radius;
  six calls at 0x800380a4, 0x80038354, 0x80038ac4, 0x80039264, 0x800393d0
  and 0x8003990c constrain this contract. Sole external call at 0x80037fa4
  forwards x/y/z/radius, stack height 0, and flags 0x61/0x71/0xe1 to the
  world query. Cell divisor 2000, width 100, floor multiplier -100, byte grids,
  signed halfword heights and 8-byte record stride are direct instruction facts.
  There are no strings; the 24-byte RODATA is the grid-shape dispatch table.
  Return at 0x80037fb4 owns sp += 32. Neighbors are effect_pool_set_current
  and effect_projectile_update_3d. Existing internal-jump candidates are not
  automatically promoted by this data campaign.
- `magic_cast`, 0x8003a2a0, 0x4c0 bytes, `game.magic`, 97.375000%:
  no parameters or return value; caller player_update at 0x800193ac.
  Spell dispatch 4..8, camera transforms, actor targeting, angle/vector helpers
  and effect construction remain unchanged. SDK ApplyMatrix stays an SDK call.
  Signed lh/slti -4999 at the disputed lookup agrees with actor action case 16;
  its byte attribute is scaled by two and added to 0x800558b6. The 20-byte
  RODATA is the spell jump table; no strings. Return 0x8003a758 owns sp += 144.
  Neighbors magic_load_records and effect_pool_sweep remain unchanged.

Vendor negative controls: none of these entries appears in the vendored
inventory or the supplied Psy-Q complete-object/text-section match inventories.
The world-query dossier already excludes matching SDK signatures. Shared game
grids, effect/player/actor state and class policy distinguish these bodies from
SDK primitives; authentic SDK calls and types are retained. History includes
1f6327e (effect reconstruction), a20532b (collision header), 4628a89 (DATA) and
37c26d2 (header split). The original effect commit's claim that CFG/referents
agree is contradicted by the raw branches and biased grid reference below.
Do not carry its register/scheduler attribution forward.

## Data ownership and ordered referents

The old s16[284] / 0x238 allocation incorrectly swallowed a distinct record
array. Direct byte attribute users reject 0xff; actor/magic use attribute-1.
Thus reviewed consumers address scalar slots 0..254 (510 bytes). The following
zero halfword at 0x80055ab6 is not consumed: array[256] versus array[255] plus
alignment remains unknown. Keep it explicitly unclassified in the census,
not an arbitrary extra element or a permission to crop assembler padding.
The actor/magic attribute-zero case would index -1; runtime lower-bound validity
is unproved and this campaign does not invent a guard or claim full containment.

Scalar bytes at 0x800558b8, size 0x1fe, SHA-256:
`61b46d401d6f82e808978b121e8344f6e4055f90751591c4b969ea1cc21206d4`.
All nonnegative entries are exactly the selectors 0..6. At 0x80055ab8, retail
uses selector << 3 and signed lh at offsets 0,2,4,6; seven complete records
occupy 0x38 bytes, ending at independent map_resource_path (0x80055af0).
Their SHA-256 is
`c8ec78b2662ebd64140f199d1ed306b8f1afc3b6737a26668da4d0fd74aefa0a`.
The last three records have inverted Y bounds; preserve those bytes literally,
not a speculative gameplay correction. The sole consumer owns the new source
definition; the shared collision header owns the type. Source global linkage
is a working model, not recovered original linkage or original TU proof.

| HI/LO sites | Linked S+A | Correct symbol / addend | Old model |
| --- | --- | --- | --- |
| 0x800379b8 / 0x800379bc | 0x80055ab8 | map_cell_height_records + 0 | height table + 0x200 |
| 0x80037b94 / 0x80037b98 | 0x80097fb4 | map_collision_grid - 100 | floor grid + 0x26b4 |
| 0x8003a440 / 0x8003a444 | 0x800558b6 | map_cell_attribute_height_table - 2 | camera SVECTOR array + 0x3e |

All three remain signed-low LUI/ADDIU pairs. The effect grid's +100/-100/+1/-1
neighbors prove the second correction. Actor action's existing pair at
0x80030004/0x80030008 independently corroborates the third. No linked target,
raw immediate, call target or delay slot is rewritten in the retail model.

## Orientation CFG evidence

The dispatch at 0x80037a0c..0x80037a8c selects 1: Z remainder,
2: X remainder, 3: 2000-Z, 4: 2000-X. Other orientation bytes bypass the
rectangle test and continue to grid shape. The common path sign-extends its
selected coordinate to s16, then uses inclusive bounds at 0x80037a4c/0x80037a60.
Y comparisons likewise include both endpoints (lh at 0x800379c4/0x800379dc).
The old C mapped 2 to reversed X, omitted 4 and tested defaults as X. This is
a real CFG/semantic error, not an unattributed instruction-selection residue.

The earlier player-data dossier's probe name is corrected from GCC 2.6.0 to
the actual unchanged GCC 2.5.7 profile; no compiler settings change.

## Verification and final verdict

The data/referent-only build changed effect_map_collision from 71.711580% to
72.176840%; magic_cast stayed 97.375000% and collision_query_world stayed
93.202490%. A bounded one-function execution control at cell (10,10), local
X=1250/Z=750, Y=-2000, attribute 76, effect kind 0 reproduced the old error:

| Orientation | Retail result | Baseline and data-only result |
| --- | --- | --- |
| 0 | 0x10000 | 1 |
| 1 | 0x10000 | 0x10000 |
| 2 | 1 | 0x10000 |
| 3 | 1 | 1 |
| 4 | 0x10000 | 1 |
| 5 and 255 | 0x10000 | 1 |

The separate C switch correction raises effect_map_collision to **74.821050%**.
The rebuilt dispatch at object +0x198..+0x208 now selects all four orientations
and skips the rectangle for other values. The initial raw divergence remains
the 40-byte compiled frame versus retail's 32-byte frame. This does not establish
a compiler mechanism: quotient narrowing, the rest of the grid CFG, current
effect load placement and candidate internal-jump relocations still need work.
Final source .text is 1600 bytes (1584 before the switch change) versus retail
1900. No whole-function equivalence or exact claim is made for this function.

`tests/test_game_collision_data.py` checks unique source/census owners, the
explicit two-byte gap, shared four-s16 type/field inventory, literal initializer
hashes, compiled and target named allocation bytes, and all four raw address
pairs (including the unchanged actor control). Safe delinking preserves signed
addends and restores the original pairs. The two former owners no longer receive
the false incoming references.

The existing bounded MIPS machine then checks 2,352 rectangle scenarios against
both retail and the reconstructed object: all seven records, orientations
0/1/2/3/4/5/255, twelve X values including inclusive endpoints and neighbors,
and four Y endpoints/neighbors per record. Three further scenarios verify the
forwarded O32 arguments and flags for effect kinds 1/2/3 through an explicit
world-query hook. Total: 2,355 scenarios / 4,710 isolated calls. The machine
guards writes, enforces a 1,000-instruction bound and audits R3000 load delays.
It does not boot the game, prove all runtime inputs, or validate the hooked
world-query body. The seven literal records, including inverted Y bounds,
are never patched to make these controls pass.

Final per-function verdicts:

- collision_query_world: **93.202490%, non-exact**, body/.text/ordered text
  relocations unchanged; scalar DATA reduced to the 255 consumed entries.
- effect_map_collision: **74.821050%, non-exact**, typed records, corrected
  negative-grid referent and independently tested orientation CFG correction.
- magic_cast: **97.375000%, non-exact**, only the source/target low addend
  changes from +0x3e to -2 and the relocation symbol changes to the height owner.
  All other instructions, calls, constants and delay slots remain identical.
- actor_update_current_action (unchanged corroborating control): **98.938940%**.

All 484 report function rows were compared to baseline: only effect_map_collision
changes. All **354/471 exact game functions** and 13 vendor verification functions
remain exact; no bank/ledger change. Of 114 compiled objects (112 source plus
two config SDK providers), only the three campaign source objects change. Of
1,719 delinked objects, only the same three module objects and the two individual
effect/magic targets change. PSX and OPEN objects are byte-identical.

Independent GNU MIPS linking restores every initialized target byte in
game.collision (1,794 bytes), game.effect_map_collision (1,980 bytes) and
game.magic (1,452 bytes). Full target relink remains **108/114**, with the same
six previously documented section-placement conflicts. This is target fidelity,
not reconstructed executable equality.

Strict source DATA remains **11/60**; config SDK comparison remains **2/2**.
The scalar .data is 510 target versus 512 compiled bytes; the record .data is
56 versus 64. The latter eight extra zeros overlap the nonzero beginning of
map_resource_path at the implied placement: they cannot be attributed as
record-array padding. The effect's 24-byte .rodata still differs at its first
ordered .text addend (retail +0x2a0, source +0x250). No section is cropped,
expanded or masked to make these comparisons green.

GAME source-owned reached ranges increase 69 to 70 by splitting an existing
owner, not by discovering new coverage. Config-only reached ranges stay 662
across all images, of which 660 lack a comparison path. Known-reference issue
counts do not change; exhaustive reachable-byte coverage remains unproved.

The full compare graph passed, all **540 local tests** passed without skips,
Ruff and git diff --check passed, and nix flake check -L passed (62 expected
no-retail/build-artifact skips in its clean environment). The full default
kf build was run and remains red on strict data, reachability and the six
placement conflicts. Those failures are not waived by this checkpoint.

Reproduction after initialization, within nix develop:

```sh
kf try --unit game.effect_map_collision
kf match --unit game.effect_map_collision
kf build compare -j 4
python -m unittest tests.test_game_collision_data -v
kf verify reachability --output build/collision-data-reachability.json
kf verify roundtrip --output build/collision-data-roundtrip.json
kf build -j 4
ruff check scripts tests
python -m unittest discover -s tests -v
```

The match/full-build and reachability commands intentionally return nonzero;
all-image roundtrip returns nonzero for the six unrelated placement conflicts.
