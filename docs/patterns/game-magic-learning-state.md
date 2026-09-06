# Magic learning-state storage and save propagation

## Function Match Plan

Start at `44bee6c` on master with hash-verified Japanese retail. The project
matcher workflow captures six GAME views, source histories and adjacent
inventory under `build/constant-names/magic-learning-state/`. All selected
units use `probe-gcc257-o2-g0`, still a probe rather than compiler attribution.

Introduce the byte enum `KfMagicLearningState` with unlearned 0 and learned 1.
Use it for `KfMagicRecord.learned` and the saved `magic_flags` array so the
save/read pair copies the domain directly. Reuse the established 24-record
count for the serialized extent and save/read loops. Name all literal
learning comparisons and writes. Keep exact-one menu predicates distinct from
nonzero gameplay predicates; do not replace the field with bool or normalize
unexpected resource/save values. No new unknown-byte interpretation follows.

These functions own game progression, menu filtering and save policy and are
absent from the vendored roster. SDK file I/O, audio and memory operations
remain external. No vendor body or original compiler attribution is claimed.

Require unchanged sections of all 112 objects, all 484 strict scores, complete
objdiff report and ordered referents after forced compilation. Compare exact
controls against complete retail words, check modern diagnostics and temporary
actual-header enum probes, inventory, ruff, repository tests, whitespace and
full kf build before commit. No size assertions or per-field repository tests.
Refresh affected literal ledgers and explain all remaining learning-related
thresholds. No new banked match is part of this type/naming batch.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x8002317c / 1328` | `menu_magic_panel` | 95.89759 | Instant spell list accepts exactly learned byte1; preserve the equality and all MP/status/healing behavior. |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.06338 | Ranged spell list accepts exactly byte1 before inserting its typed spell ID; preserve none row and generic menu result. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Control: startup loads resource records through the existing resource initialization; no new per-record normalization or assignment. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Healing nonzero gates Dispoison learning at37; Dispoison/Fire Wall/Lightning Bolt byte0 gates writes of1. Preserve the difference between nonzero and exactly1. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Flame Sword rejects only unlearned Fire Ball byte0, rather than requiring exactly1. Existing selected-spell and effect-kind domains remain separate. |
| `0x8003425c / 136` | `map_ambient_script_floor3` | 100.0 | If either Resist Fire or Bless is byte0, write both to1 and notify; preserve combined update behavior. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Bracelet possession grants Wind Cutter and dialogue grants Fire Ball when their learning bytes are0; preserve two independent gates. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Mirror of Truth exchange writes Healing byte1 without testing its previous value; preserve consumption and dialogue order. |
| `0x8003a274 / 44` | `magic_load_records` | 100.0 | Control: copies all24 complete20-byte records as words; learning is the first byte. Do not normalize serialized values. |
| `0x8002b73c / 1268` | `save_file_write_slot` | 100.0 | Loop at8002ba18 loads each record byte0, stores consecutive bytes at payload+2530, iterates24 times with20-byte record stride; no Boolean conversion. |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100.0 | Loop at8002c210 reverses the same24-byte transfer into record byte0. Type both ends of this assignment, preserving all encoded byte values. |

## Learning domain and resource evidence

`KF_MAGIC_UNLEARNED = 0` and `KF_MAGIC_LEARNED = 1` describe the observed
learning states. Both menus compare exactly with learned; progression and
Flame Sword retain their zero/nonzero tests. A serialized value of 2 would
therefore remain absent from menus while satisfying the existing nonzero
gameplay gates. The enum does not normalize such values or claim additional
states. Runtime and save fields share `KfMagicLearningState`, so the write/read
assignments preserve the domain directly. The legacy compiler sees a byte
typedef; the modern compiler sees a distinct enum with byte storage.
The batch replaces 19 numeric learning comparisons and assignments across
five C sources, in addition to typing both ends of the save transfer.

`common_resources_load` advances through length-prefixed `COM\\COM.DAT` chunks
and passes the fifth payload to `magic_load_records`. In the Japanese retail
file, the length header is at file offset `0x1428`, its payload starts at
`0x142c`, and its 480 bytes hold 24 records of 20 bytes. The file is 7,380 bytes,
SHA-256 `8fea86564bf5927f00156edf3163f3d008f86289103dd076746fad7a8922e329`.
Byte zero of record 8 (Light Needle) is 1; the other 23 records contain 0.
This identifies the resource default, without implying that all 24 records
are selectable player spells. The loader copies the records without converting
their learning bytes. The save loops separately copy all 24 first bytes to
and from payload offsets `0x2530..0x2547`.

The learning thresholds remain literal authored requirements: base magic 37
for Dispoison when Healing is nonzero, 70 for Fire Wall, and 75 for Lightning
Bolt. The code proves these cutoffs and their learning notifications; it does
not explain why the designers chose them. The earlier
[spell identity audit](game-spell-identities.md) records those conditions.
The 24-record count is a resource/storage extent, now reused for both save
array declaration and iteration instead of deriving an element count from
byte size. No opaque trailing record or save bytes receive speculative names.

## Verification and final verdicts

Forced compilation and the comparison build cover all seven affected/control
units and their shared-header dependents. A simultaneous, independent
[magic-panel reconstruction](game-magic-panel-flow.md) changed the shared
menu unit from 95.89759% to 100% (`8d9c147`). Consequently, equality with the original
whole-project report is not a valid gate for the combined worktree. All 111
other objects retain every section, and all 483 other function scores are
unchanged. For the shared unit, two isolated compilations of the current
source differ only in the learned comparison spelling (`1` versus
`KF_MAGIC_LEARNED`): every object section is identical. Its current code,
data and relocation sections also match that control; only temporary-source
debug metadata differs. The matching improvement belongs to the separate
flow campaign, not the enum change.

| GAME function | Final strict % | Verdict |
| --- | --- | --- |
| `menu_magic_panel` | 100 | Independent flow recovery; enum numeric/named control unchanged; all 332 retail words and ordered referents verified. |
| `menu_spell_select` | 98.06338 | Unchanged object; existing non-exact result retained. |
| `game_state_initialize` | 100 | Unchanged; all 185 retail words and ordered referents verified. |
| `player_recalculate_combat_stats` | 100 | Unchanged; all 517 retail words and ordered referents verified. |
| `player_update` | 96.94554 | Unchanged object; existing non-exact result retained. |
| `map_ambient_script_floor3` | 100 | Unchanged; all 34 retail words and ordered referents verified. |
| `map_action_script_floor3` | 100 | Unchanged; all 36 retail words and ordered referents verified. |
| `map_event_interact` | 100 | Unchanged; all 181 retail words and ordered referents verified. |
| `magic_load_records` | 100 | Unchanged; all 11 retail words and ordered referents verified. |
| `save_file_write_slot` | 100 | Unchanged; all 317 retail words and ordered referents verified. |
| `save_file_read_slot` | 100 | Unchanged; all 243 retail words and ordered referents verified. |

Nine exact controls cover 1,856 complete retail words, including delay slots,
with equal ordered calls and address referents. No match is newly banked by
this naming/type batch. Modern actual-header probes accept typed field,
parameter, save-array and return propagation, and reject raw integer writes
to either field, a spell ID assigned as learning state, and learning state
passed as a selected spell. Clang layout output keeps learning at record byte
zero, charge rate at byte one, saved states at `0x2530`, and the following
opaque field at `0x2548`. No size assertions or repository tests were added.

`kf check-types` retains the same 320 diagnostic messages: 64 of 112 variants
pass and 48 retain existing debt. Inventory and Ruff pass; all 657 repository
tests pass, as does `git diff --check`. Full `kf build` still fails the existing
data ownership/placement gates: target relinks remain PSX 1/1, GAME 75/77 and
OPEN 34/38, with the same six section-base conflicts and no artifact failures.
Source data matches are now 9/61 (the independent menu recovery adds one);
the four config-data contributions pass. This batch introduces no new build
failure.

The [equipment](game-equipment-literal-ledger.md),
[floor-script](game-map-script-literal-ledger.md) and
[spell/menu](game-spell-literal-ledger.md) ledgers retain explicit reasons for
142, 287 and 125 literal occurrences respectively. The spell/menu reduction
includes the separate flow campaign's removal of a name-pointer stride.
Evidence snapshots, numeric/named controls, resource decoding, per-function
verdicts and command logs are under the ignored
`build/constant-names/magic-learning-state/` directory.
