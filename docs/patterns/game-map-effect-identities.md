# GAME map-switch effect and restored drop bands

## Function Match Plan

Baseline `c503559`, hash-verified GAME retail, unchanged `probe-gcc257-o2-g0`.
Nine functions have fresh address/extent, disassembly/CFG, caller/callee,
string, match and source-history snapshots. Follow map loading through effect
construction, map-controlled animation, effect sweeping and rendering. Also
follow the drop ID bands from spawning to saved-floor restoration. Custom
placement/effect records and floor state establish game ownership; SDK calls
remain external library bodies. No function, data or relocation identity is
promoted from candidate evidence.

Name effect kind 48 `KF_EFFECT_KIND_MAP_SWITCH` in the shared effect header
and both constructor/loader consumers. Move the three drop-band boundaries
to the shared map header and propagate the first two into restoration.
Retain record widths, variadic slots, predicates, initialization omissions,
calls, CFG and operation order. No new enum storage or type casts are needed.
First hypothesis: these substitutions preserve all runtime bytes and ordered
relocations. Freeze all 112 source objects and all 484 scores; force affected
compiles, compare frozen before/after builds, and recheck exact functions
against complete retail words and targets. Run modern checking, inventory,
Ruff, the existing suite, whitespace checking and full `kf build`.

| GAME VA / bytes | Function | Initial strict % |
| --- | --- | ---: |
| 8001eedc / 488 | render_actor_sprite | 100 |
| 8001f218 / 1408 | render_entities | 97.380684 |
| 80031008 / 1096 | map_object_pool_load | 100 |
| 80031834 / 404 | map_object_spawn_effect | 94.455444 |
| 80031cc8 / 3096 | map_object_pool_update | 98.966410 |
| 80035e44 / 1692 | map_restore_floor_state | 99.964540 |
| 80036f44 / 2092 | effect_pool_construct | 100 |
| 80038a38 / 6156 | effect_update_dispatch | 96.939570 |
| 8003a760 / 124 | effect_pool_sweep | 100 |

## Identity evidence

Map object 135 supplies effect kind 48, stores the resulting effect-pool index
in its action parameter, and starts the named effect-switch map action.
The constructor's table word at GAME `80012cd8` points to `80037344`: clip
zero, model selectors 5/5, rotation copied from the sixth argument. It retains
kind 48. The equal-valued effect capacity 48 and TMD mode 0x30 are unrelated.
Model selector 5 is a resource index, distinct from Wind Cutter's billboard
selector 5; it is not renamed to the effect kind.

The updater's kind-48 table word at `80012da8` points directly to the return
epilogue at `8003a214`. It has no autonomous projectile/update body. Map action
83 instead changes the linked effect's animation phase, plays departure
sounds and fires the link, as documented in the
[motion audit](game-map-object-motion-constants.md). The effect sweep still
visits this live record; it does not delete it. Rendering filters live records
by the visible cell window, chooses the model path for clip zero, and passes
the phase through the render view's animation argument. This establishes a
map-switch effect identity without inventing an asset title or a projectile
meaning. Kind 14 and the type-0x20 damage-credit class remain separate open
identity work.

Drop spawning selects fall-and-tip below 43, fall-and-spin below 48, and
bounce below 65. At GAME `8003622c` restoration compares the loaded unsigned
object byte against 43 and writes quarter-turn pitch in the taken arm; at
`80036244` it compares against 48 and writes zero pitch for IDs 43..47.
These restore the settled orientations of the same first two action bands.
IDs 48 and above deliberately retain their prior pitch. Shared constants
express the existing band relationship without adding a bounce reset, range
guard, or asserted item-category identity.

## Literal accounting and final verification

Four inline uses acquire names: two kind-48 occurrences and the two restored
drop-band comparisons. The three existing private band definitions move to
the shared header. The complete pool ledger now covers 225 occurrences;
the two map-object modules cover 170 (77 action-module and 93 loader-module
occurrences). All three one-occurrence ledgers were checked against source,
including repeated tokens and shifted line numbers. The earlier grouped
map-event ledger removes the two named restoration occurrences. Other
numeric asset IDs, table data and arithmetic keep their documented reasons.

Eight evidence-bearing units were force-compiled, followed by the dependency
comparison and full build. Frozen before/after compilation of all 112 units
preserves runtime section contents/alignment, runtime symbols and ordered
relocations. Only `map_object.c` changes its debug-line section. All 484 live
scores are unchanged from the captured baseline. The nine reviewed bodies
preserve 4140 emitted words, 176 direct-call references and 126 data/address
references. All 950 words across the four exact functions independently
match retail and the delinked targets, including delay slots.

| Function | Final verdict | Words / calls / addresses |
| --- | --- | ---: |
| render_actor_sprite | Exact, unchanged | 122 / 20 / 6 |
| render_entities | Partial, unchanged | 354 / 9 / 25 |
| map_object_pool_load | Exact, unchanged | 274 / 11 / 8 |
| map_object_spawn_effect | Partial, unchanged | 100 / 5 / 2 |
| map_object_pool_update | Partial, unchanged | 774 / 35 / 23 |
| map_restore_floor_state | Partial, unchanged | 423 / 18 / 32 |
| effect_pool_construct | Exact, unchanged | 523 / 7 / 8 |
| effect_update_dispatch | Partial, unchanged | 1539 / 69 / 21 |
| effect_pool_sweep | Exact, unchanged | 31 / 2 / 1 |

The unchanged first retail differences are:

| GAME instruction | Function | Source versus retail |
| --- | --- | --- |
| 8001f274 | render_entities | `move s5,v1` versus `move s6,v1` |
| 80031838 | map_object_spawn_effect | Save `$ra` at stack +0x20 versus +0x24 |
| 80031dcc | map_object_pool_update | `lui a0,0x8005` versus `lbu v1,0(s3)` |
| 80035e8c | map_restore_floor_state | `addiu v0,v0,-0x69a` versus `addiu v1,a0,-0x69a` |
| 80038a68 | effect_update_dispatch | `lui s5,0x800a` versus `lui s4,0x800a` |

These are observations of existing residues, not attributed compiler
mechanisms. No exact-count increase or banking is claimed.

All 680 existing tests pass in 83.770 seconds. Inventory checking, Ruff and
whitespace checking pass. Modern checking preserves the same 300 errors and
65/112 passing variants. Full `kf build` remains nonzero on existing data
ownership/placement failures: source data PSX 0/1, GAME 9/42, OPEN 2/19;
target relink PSX 1/1, GAME 75/77, OPEN 34/38, with six conflicting section
bases and no artifact failures. No tests, size assertions, tooling or flake
changes were added. Ten source `unknown_` occurrences remain unresolved;
the broader naming objective is still open.
