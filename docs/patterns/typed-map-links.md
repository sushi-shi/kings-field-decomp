# Typed map object link storage

## Function Match Plan

This campaign starts at `b3c157f` in the isolated typed-casts worktree.
The shared eight-byte link owner is established by placement loading,
resetting, serialization and per-behavior field accesses. Retail hashes were
validated for this campaign chain. All six GAME semantic queries are captured
for each function below. The preceding world-transform campaign supplies the
unchanged caller and adjacency controls; reset and sequence selection have
also been read in full alongside the original clearing evidence and history.
The selected profile remains `probe-gcc257-o2-g0`.

The first source hypothesis is a single `KfMapObjectLink` union exposing its
existing typed fields, the two-word copy/reset representation, eight serialized
bytes, and the first halfword as a gold amount. Gold pickup adds that halfword
to player gold; the actor-debris producer stores it into the gold-coin object.
The two-byte spawn field has two behavior-dependent meanings: an unsigned
sequence for dropped objects and a byte effect ID for placed effects. An
explicit union preserves those widths without pointer reinterpretation.

Runtime links start at object+0x20 with stride 0x2c; placement links start at
+0x0c with stride 0x14. Both support word alignment. Reset writes word 1 before
word 0. The placement copy is two `lw`/`sw` operations; the save loop transfers
eight bytes. Gold is loaded/stored with `lhu`/`sh`; effect IDs use `lbu` at link
+2 while sequence selection uses `lhu` there. Preserve these widths, ordered
stores, old narrowed counters, all branch/load delay slots and return slots.
Keep signatures, constants, calls, references and source ownership unchanged.

Vendor negative control: these functions manage the game's custom pool,
behaviors, gold, dialogue or floor records and are absent from the vendored
census. The supplied SDK memory declaration and prior fixed-copy evidence
remain authoritative; no SDK function body is introduced. All outgoing
references are proven/validated. Incoming dispatch-table candidates remain
candidates, including the loader, update, interaction and floor restore tables.
No selected function has a string reference.

The table counts basic blocks / direct JAL sites / branches and direct jumps /
returns. Every control-transfer count includes its following delay instruction
in the raw comparison; call/reference inventory counts also retain indirect
control separately.

| GAME function | VA / bytes | Strict before | B/C/J/R | References P/V/C, incoming; outgoing | Final verdict |
| --- | --- | ---: | --- | --- | --- |
| `player_use_item` | `80018054 / 45c` | 100 | 43/20/31/1 | 1/86/0; 22/29/0 | 100%; raw unchanged |
| `map_object_pool_clear` | `80030f7c / 60` | 100 | 3/0/1/1 | 1/0/0; 0/4/0 | 100%; raw unchanged |
| `map_object_pool_load` | `80031008 / 448` | 100 | 43/11/27/1 | 1/9/84; 12/17/0 | 100%; raw unchanged |
| `map_object_effect_pool_acquire` | `800317a4 / 90` | 100 | 9/0/4/1 | 3/0/0; 0/1/0 | 100%; raw unchanged |
| `map_object_spawn_effect` | `80031834 / 194` | 94.5049 | 21/5/13/1 | 1/3/0; 5/5/0 | Partial unchanged; raw unchanged |
| `map_object_spawn_actor_debris` | `800319c8 / 18c` | 100 | 11/6/6/1 | 1/0/0; 6/1/0 | 100%; raw unchanged |
| `map_object_pool_trigger_link` | `80031b54 / f0` | 100 | 13/1/10/1 | 4/1/0; 1/3/0 | 100%; raw unchanged |
| `map_object_pool_clear_link` | `80031c44 / 84` | 100 | 7/0/4/1 | 5/0/0; 0/1/0 | 100%; raw unchanged |
| `map_object_pool_update` | `80031cc8 / c18` | 98.9664 | 118/35/94/1 | 1/30/99; 36/53/0 | Partial unchanged; raw unchanged |
| `map_floor5_transition_cutscene` | `800346a8 / 38c` | 100 | 32/12/23/1 | 1/8/0; 12/22/0 | 100%; raw unchanged |
| `map_interaction_dispatch` | `80034de4 / 904` | 96.4905 | 119/54/92/1 | 1/34/89; 56/47/0 | Partial unchanged; raw unchanged |
| `map_world_state_persist` | `80035b5c / 2b8` | 94.8218 | 28/0/18/1 | 3/1/0; 0/8/0 | Partial unchanged; raw unchanged |
| `map_restore_floor_state` | `80035e44 / 69c` | 100 | 50/18/32/1 | 1/5/5; 19/37/0 | 100%; raw unchanged |

All 13 direct consumers require fresh compilation. Compare every function in
the six affected units against the saved objects, resolving actual relocation
referents before comparing raw words. Existing exact functions must remain
100%; partial functions retain separate honest verdicts. Follow focused
matching with full build, repository tests, lint and diff checks. Full-image
data ownership and placement failures are already present and are not relaxed.

The first typed build preserves all 45 bodies in the six units (39 retail
exact). Reviewing the complete union consumers also exposes two four-byte
item lists: hinged containers start at link+1 and ordinary containers at
link+0. Their byte cursors advance through four items rather than through
individual scalar members. Extend the same owner with those array views and
propagate the existing shared count. Keep the current first-item check and
loop CFG unchanged; this refinement supplies the actual array bounds.

## Final verification

All 45 function bodies across the six affected units are unchanged after
resolving their actual relocation targets. The 39 exact functions remain
retail-exact; all six partial neighbors preserve their prior bytes. Among the
13 direct consumers, nine are exact and four remain partial at the table's
scores. Calls, constants, ordered references, delay slots and encoded words
are unchanged. Global strict status remains 439/471: GAME 332/362, OPEN
106/108, PSX 1/1. No partial is banked and no new exact result is claimed.

This stage removes 18 pointer casts from C source and replaces the two
container cursors through individual fields with real four-element arrays.
The current C-source count is 539 pointer casts, down 267 from the original
806. The AST census covers all 112 image variants without errors and counts
797 written casts including 39 header casts (chiefly layout/domain checks).

The full suite ran 713 tests with nine skips; its only failure was an
inventory-count expectation loaded before the inventory update completed.
The finalized inventory suite passes all 113 tests. The existing drop-velocity
control still verifies the unconditional reset outside all action bands.
Ruff and `git diff --check` pass. Fresh focused builds and both full builds
were run; existing image data/reference/placement gates still fail, with no
new artifact failures. Those global closure failures are unchanged.
