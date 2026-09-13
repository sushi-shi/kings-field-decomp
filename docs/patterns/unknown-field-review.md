# Unknown-field review

The isolated review at `4d3dc5b7` accounts for all **79 `unknown_` field
declarations under `include/kf`**, with one decision per field in the
[ledger](../unknown-field-review.tsv). A fresh Clang parse of all 101 variants
after rebasing onto merged PR #7 reconciles every field identity, offset,
width, type, owner extent and curated extent; declaration links reflect
the final headers. All 79 retain their C spelling and
layout. The review establishes several codec corrections for already named
C fields, but does **not** establish semantic closure for the remaining
unknowns. In particular, no field is renamed `padding`, `reserved`, or
`unused` from alignment, zero values, or absence of named C uses.

## What an unresolved field means

`retain_unresolved` means preserve the modeled bytes and leave their purpose
open. It does not mean the field is unnecessary, padding, unused, or free to
remove. The unchanged count of 79 is not a completed semantic reconstruction.
Some declarations may eventually become parts of wider fields or known
subobjects; others may prove to be spare storage. Those changes need evidence
about the actual owner and its consumers.

The ledger now records `storage_origin`, `disk_source`, `transport_paths` and
`alias_audit` for every field. Resource files, initialized EXE data, saved
runtime state, and runtime-only objects are distinguished. "No serialized
source established" is an audit limit, not proof that none exists. Chunk
indices in the ledger are zero-based, and filenames retain their retail
spelling. A value census covering B1–B5 does not also cover OPEN's B0 files.

## Disk and bulk-copy provenance

The following paths establish storage and transport without establishing a
behavioral meaning for every byte:

| Storage | Path and effect on unknown fields |
| --- | --- |
| `COM/COM.DAT` | [`common_resources_load`](../../src/game/resources.c) passes cast chunk pointers to the weapon, armor, magic and map-object definition loaders. Their `u32` table views copy unknown lanes alongside known fields. The armor and object-definition copy extents cross nominal chunk boundaries. |
| `B1..B5/MIXA.DAT` | [`map_resources_load`](../../src/game/resources.c) passes cast payload pointers to placement/definition expanders. [`actor_definitions_load`](../../src/game/actor_pool.c) copies all definition words. Source placement bytes may be present in the loaded file without an established runtime destination. Floor-item +3 and the two event bytes have explicit source/destination mappings. |
| OPEN scene resources | [`opening_resources_load_scene0`](../../src/open/resources.c) reads floor-item placements from `B0/MIXA0.`; entity loaders also consume placement chunks in `MIXA3.`, `MIXAE.` and `MIXAF.`. These are additional source populations, not automatically included in the floor-resource census. |
| `COM/STAT.DAT` | [`item_load_database`](../../src/game/item.c) copies the complete menu-assets bank, including the five `MenuTileSprite` records. The two unknown coordinate-adjacent bytes therefore come from the disc even though individual GPU output stores are bytes. |
| Linked `GAME.EXE`/`OPEN.EXE` data | Camera points and HUD/notification/effect descriptors have modeled initializers in the executables. They are distinct from separately loaded resource records and from runtime BSS. |
| Memory-card slot payload | [`save_file_write_slot` and `save_file_read_slot`](../../src/game/save_system.c) copy the complete 0xe0-byte player state through a word-array view, including all its unknown ranges and nested progress byte. They also write/read the entire payload, including both payload gaps. Saved-world transport separately preserves event +0x0d. This is game payload data, not the standard PlayStation header. |

## Casts, arithmetic and access coverage

A named-member search misses accesses through another representation. The
review must account for casts, unions, byte-pointer arithmetic, indexed
interior pointers, stack spills, GP-relative references, wide loads/stores,
bulk copies and pointers passed to helpers. A relocation to a nearby known
field can also be the base of arithmetic that reaches an unknown interval.
The weapon/armor bias example below shows why physical overlap alone is not
enough to identify the logical object being accessed.

For each such path, record the image, original owner, pointer displacement,
access width/range, callee and evidence tier. Distinguish copying or clearing
bytes from interpreting them. Follow the callee's actual reads/writes:
passing an SDK pointer does not by itself prove that every byte in the SDK
type is used, and passing an interior subobject does not imply access to the
surrounding unknown fields. The ledger's `alias_audit` column explicitly
marks paths without that coverage as open.

Similarly, a retail byte load can result from narrowing a wider source value
into a GPU byte field. Width recovery must include the input format and all
consumers; the load opcode alone does not always determine the original C
member width. A compile probe can test a proposed representation but cannot
prove its semantic origin merely by preserving bytes.

Standard SDK ownership is checked separately in the
[format-provenance review](unknown-field-format-provenance.md). An embedded
`SVECTOR`, `VECTOR`, `DRAWENV` or other SDK member does not make the enclosing
game record an SDK format. Check documentation-version discrepancies against
the pinned headers and retail layout before adopting field names or
reserved-byte rules.

### Menu coordinate-width control

`MenuTileSprite` has the same twelve-byte extent and field positions as the
nearby `MenuSpriteDef`, whose texture coordinates are halfwords. A scratch
header therefore tested replacing `u8 u; unknown_05; u8 v; unknown_07` with
`u16 u; u16 v`, without changing the production header or packet expressions.
Using the unit's pinned compiler/profile, 34 of 35 `game.menu_runtime`
function instruction/ordered-relocation listings stayed identical, including
`menu_list_render` and `menu_draw_window_backdrop`. This demonstrates that
their narrowed outputs alone cannot discriminate the two input layouts.

`menu_status_panel` is a counterexample: its first changed instruction loads
the tile's `u` at descriptor +4 with `lhu` instead of `lbu`. Retail GAME
`0x800243f0` uses `lbu` at `menu_assets +0x31c`, and the production function is
strict 100%. Widening the fields without further source evidence would
therefore regress a banked function. No widening is retained, and no extra
cast is added to force that result back. The original descriptor width still
needs evidence beyond matching size, zero high bytes and the output packet.

The three witness functions received fresh image-qualified address, raw
block-disassembly, caller, callee, string and match views. The verdict is an
inconclusive width hypothesis with a concrete counterexample, not proof that
the two bytes are padding or behaviorally irrelevant.

A follow-up checked both generic sprite blitters with the same six evidence
views. `menu_blit_sprite` and `menu_blit_sprite_translucent` remain strict
100% with the existing `MenuSpriteDef` halfword coordinates, yet retail loads
their low bytes with `lbu` at GAME `0x80029d1c` and `0x80029b7c`. These are
concrete controls against inferring a declared byte width from a narrowed GPU
write. The reconstructed call sites pass sprite descriptors to those helpers;
the tile descriptors are consumed by the list/window/status packet expressions.
No shared descriptor-pointer path between those two families was established
in this follow-up. The verdict for both blitters is unchanged exact source and
an inconclusive control for the tile layout, not a new field identification.

### Floor-item and event alias follow-up

The [scoped raw-access audit](unknown-field-alias-audit.md) follows the
floor-item/event families beyond named members, including pointer arithmetic,
unaligned word-copy lanes, restored dialogue indices and the animation-cache
owner pointer. Normal authored dialogue stages are 1–5, but unchecked restored
indices let page accesses reach all five unknown event ranges: stages 10/11
select +0x0c/+0x0d, 15 selects +0x11, 32/33 select the two bytes at +0x22,
and 64/65 select the two bytes at +0x42. These conditional byte reads/writes
prevent an "unread" verdict; they do not establish that the bytes were intended
as extra page entries. The supplement records the exact instructions and
remaining limits.

## Coverage and limits

A Clang member-reference pass covered 101 configured retail translation
variants, including both images for shared sources. Each declaration's
owner, offset, width and type was reconciled with
`config/retail/structure_fields.tsv`. Nine fields have named C member uses:
the floor-item source/destination +3 bytes, two event source bytes and their
two destination bytes, and three OPEN control halfwords. The other 70 have
no named member use in those variants. This is a source coverage result,
not proof that retail never accesses the bytes.

A separate owner expansion located 2,163 unknown-field instances in curated
global objects and compared their intervals with admitted relocation targets.
Seventeen targets landed inside those intervals. Four produce weapon/armor
table bases, three are OPEN control clears, and ten are a useful negative
control: target `GAME.EXE:800a00dc` lies physically in weapon record 10's
+0x14 region but represents
`armor_records - 13 * sizeof(KfArmorRecord)`. Adding the item-ID stride
selects armor records beginning at `800a0248`. The equipment/stat consumers
in `player_core.c` and `player_death.c` establish this bias. Treating the
interior target alone as a weapon-field read would be incorrect. For valid
armor IDs the final reads address armor records; the
[three-field access report](weapon-unknown-access-report.md) separately
enumerates weapon-byte overlaps when IDs outside that range reach an armor
consumer.

This interval census only covers curated global ownership and admitted
relocation targets. It does not follow every indirect pointer, dynamic
base-plus-offset access, GP-relative address, stack view, unreconstructed
body, or unadmitted relocation candidate. The initial raw dossiers cover the
34 functions below, with the menu-width and scoped alias follow-ups documented
separately above;
other ledger rows explicitly say when they only have
source/curated-owner coverage. Thus **the whole-binary behavioral audit is
still incomplete**. An unresolved row records the next evidence needed,
not a claim that the field has been proved meaningless.

The local source-only `unknown_02` in `src/game/pool.c` is outside the
requested 79 header declarations. Codec owners without a corresponding
reviewed C owner were not added to the scope.

## Resource constraints

The [equipment follow-up](equipment-unknown-field-audit.md) checks the stored
weapon/armor pointers and GP uses, and compares proposed slot, attack-class
and price interpretations against the authored rows and actual consumers.
It establishes no new semantic name for the five remaining equipment ranges.
The [three-weapon-field report](weapon-unknown-access-report.md) adds explicit
read/write/guard/value-use results, a fresh typed census, resolved static
warnings and conditional armor aliases for the three weapon ranges.

The hash-validated retail corpus provides the following independent controls:

- `COM.DAT` weapon +0 is 0/1/2 across 16 records; +0x14 contains nonzero
  data, while +0x22 is zero throughout. Price-like words at weapon
  +0x14/+0x18 and armor +0x14/+0x18 disagree with `STAT.DAT`'s independent
  two-column buy/sell price tables. For example weapon row 0 holds 150/100,
  while shop buy columns are 180/200 and sell columns 150/120. This rejects
  naming the opaque pair as the operative shop prices.
- The low byte of armor +0 agrees with the existing equipment-slot codes
  for all 23 named armor items. Its high byte varies. With no decoded
  reader establishing that split, this remains a candidate, and the shared
  `u16 unknown_00` stays intact.
- Armor's chunk contains 27 rows, while its loader copies 42 rows' worth of
  bytes across the following chunk. Map-object definitions similarly have
  141 payload rows but a 160-row loader extent. Their extra copied bytes
  are recorded separately and are not additional semantic resource rows.
- Both actor-placement unknown ranges are zero across 515 active placements
  on B1–B5. Actor-definition +0x38 is zero in 59 of the 60 loaded rows;
  one B5 row contains bytes `68 f7`. A claim that this field is uniformly
  zero would be false.
- The 117 active B1–B5 floor-item placements have +3 values 8, 28, 32 and 64.
  OPEN's `B0/MIXA0.` adds 14 records, all with +3 equal to 64, for 131 authored
  records across the two populations. GAME and OPEN copy that byte without
  an established behavioral meaning.
- Event-definition +0x0b/+0x0c/+0x16 are zero in all eight source rows on
  each of five floors, including inactive rows. Runtime event +0x0d is
  persisted and restored; transport and persistence do not identify meaning.
- Map-object-placement +1 is mostly zero, but B1 has values 3 and 255.
  The magic-record suffix is zero in 24 records. The five audited menu-tile
  descriptors, 14 HUD descriptors, six notification descriptors, two effect
  sprite descriptors and 34 camera points have zero in their reviewed
  unknown lanes. Those values alone do not prove padding.

The ledger records each value census's resource, active/payload/loader
scope and count. Runtime-only fields have no independent serialized value
census. OPEN's three control words have the same relative tail positions as
GAME's map-object allocation sequences, but only clear stores are established
in OPEN. The shared-tail hypothesis from the earlier
[semantic naming review](semantic-field-names.md) remains a candidate.

## Supported codec corrections

These changes bring Rust views of the same owners into agreement with
existing shared C declarations and curated fields. C layout and executable
behavior are unchanged. Rust encodes explicit little-endian bytes and does
not expose these models as SDK FFI structures.

| Rust field | Retail witness in GAME.EXE | Kept representation |
| --- | --- | --- |
| Weapon `unknown_10` | `render_weapon` loads unsigned +0x10 at 8001f7dc and calls `SetGeomScreen` | `projection_distance: u16` |
| Weapon portion of `unknown_14` | Signed loads at 8001f7f4/8001f800/8001f80c read +0x1c/+0x1e/+0x20 | `render_translation: Vec3s` (`x`, `y`, `z`); opaque prefix stays eight bytes |
| Weapon `mirrored_angle` and surrounding opaque bytes | 8001f814/8001f818 supply record +0x24 to SDK `RotMatrix`; loader negates the +0x26 Y lane | `render_rotation: SVector` (`vx`, `vy`, `vz`, `pad`), including preserved SDK pad; +0x22 stays unresolved |
| Actor-placement `unknown_05` | Loader copies source +5 to actor +7; awareness at 8002e760/8002e810 compares it shifted by seven with `rand` | `spawn_chance: u8` |
| Actor-placement `unknown_06` | Loader copies source +6 to actor +9; death at 8002fdd4 supplies it to `map_object_spawn_effect`, except sentinel 99 | `death_drop_object_id: u8` |
| Event-definition `unknown_0d` | 80033994/8003399c copy source +0x0d to runtime +0x0e; updater dispatches behaviors 1 and 2 | `behavior: u8` |

The live loader identity note and older collision document now refer to
the established rotation-Y field, linking its superseding evidence. Historical
campaign evidence retains its original spelling.

The weapon HP/MP interval member spellings also follow the existing C
`hp_regen_interval`/`mp_regen_interval` names. The weapon loader's wrapping
negation is unchanged. The signed rotation view preserves every bit pattern,
including the fourth SDK storage lane. Independent offset tests exercise
negative translation/rotation, high-bit projection distance, opaque bytes,
SDK pad and untouched output tail, in addition to existing full-byte
round trips and placement transformation tests.

## Function verdicts

Every row received image-qualified `addr`, raw block disassembly, incoming
xref, callee xref, strings and match queries. No body was changed. Fresh
match queries for the initial 32 functions before the first build were
unscored; no exactness is inferred from those queries. The two supplemental
armor-bias dossiers queried the generated strict report. Existing reconstruction documents provide earlier
history and codegen evidence. This review neither changes banked results
nor claims a new exact function.

| Image | Function | Verdict |
| --- | --- | --- |
| GAME | weapon_records_load_and_mirror_angles | Whole-record copy and Y negation confirm codec transform; no new meaning for +0/+0x14/+0x22. |
| GAME | render_weapon | Projection distance, signed translation and SDK rotation confirm codec refinement. |
| GAME | player_equip_weapon | Weapon pointer/record selection agrees; opaque fields remain unresolved. |
| GAME | player_set_equipment_slot | Five armor base biases form stored armor pointers; strict 100% after comparison generation. |
| GAME | player_recalculate_combat_stats | Five armor base biases lead to defense fields +2 through +12, not weapon opaque bytes; strict 100% after comparison generation. |
| GAME | actor_pool_load_placements | Spawn/drop copy lanes confirmed; remaining placement ranges unresolved. |
| GAME | actor_update_awareness | Spawn-chance interpretation confirmed; no new actor unknown name. |
| GAME | actor_update_current_action | Death-drop interpretation confirmed; no new actor unknown name. |
| GAME | map_event_pool_load | Behavior and two opaque copy lanes confirmed. |
| GAME | map_event_pool_update | Behavior dispatch confirmed; opaque lanes remain unresolved. |
| GAME | item_load_floor_placements | +3 byte transport confirmed; behavioral role unresolved. |
| OPEN | item_load_floor_placements | Same +3 transport; behavioral role unresolved. |
| GAME | map_world_state_persist | Event +0x0d serialized; meaning unresolved. |
| GAME | map_restore_floor_state | Event +0x0d restored; meaning unresolved. |
| GAME | armor_records_load | Copy extent crosses nominal chunk; opaque armor fields unresolved. |
| GAME | magic_load_records | Whole-table copy; zero suffix does not prove padding. |
| GAME | map_object_definitions_load | Copy extent crosses nominal chunk; opaque lanes unresolved. |
| GAME | actor_definitions_load | Twelve records per floor; nonzero +0x38 prevents all-zero interpretation. |
| GAME | camera_path_begin | Known path initialization; point/state unknown halfwords unresolved. |
| GAME | camera_path_compute_segment | Known segment arithmetic; no new unknown-field meaning. |
| GAME | camera_path_step | Known path stepping; no new unknown-field meaning. |
| OPEN | opening_camera_path_begin | Known path initialization; point/state unknown halfwords unresolved. |
| OPEN | opening_camera_path_compute_segment | Known segment arithmetic; no new unknown-field meaning. |
| OPEN | opening_camera_path_step | Known path stepping; no new unknown-field meaning. |
| GAME | effect_pool_construct | Known effect fields and SDK vectors; +0x0a/+0x2a unresolved. |
| GAME | effect_pool_spawn_typed | Known spawn arguments; opaque effect lanes unresolved. |
| GAME | audio_initialize | Known SDK initialization; adjacent unknown halfwords unresolved. |
| OPEN | audio_initialize | Same limitation for OPEN audio state. |
| GAME | save_file_write_slot | Save transport confirmed; payload gaps/player opaque bytes unresolved. |
| GAME | save_file_read_slot | Save restoration confirmed; transport does not prove padding. |
| GAME | render_effect_sprites | SDK rotation boundary retained; adjacent descriptor gaps unresolved. |
| OPEN | opening_entity_pool_reset | Three individual control clears proven; allocation analogy remains candidate. |
| OPEN | opening_entity_pool_load_placements | Known entity construction; unknown tail/control roles unresolved. |
| OPEN | opening_entity_render | Known render transforms; adjacent unknown halfword not promoted to SDK pad. |

## Verification

The pinned `nix develop` environment and hash-validated retail were used.
`cargo test --offline --manifest-path tools/Cargo.toml -p kf-codec` passes
97 tests (five optional integration controls ignored), and a full `kf build`
builds all three programs. Repository lint, formatting, ledger identity/extent
checks and `git diff --check` pass. `nix flake check -L` passes, including
824 Python tests with 143 sandbox skips and the codec suite. The local
repository suite passes all 824 tests with no skips after rebuilding
comparison prerequisites.

The comparison rebuild and full native build on merged PR #7 preserve all
101 reconstructed objects' allocated bytes, symbol metadata and ordered
relocation targets. The complete objdiff report and all three executable
files are identical to PR #7; no identity normalization is needed.

The earlier `kf analyze` run also included the broader closure gates, which
retain the parent failures: target relink PSX 1/1, GAME 58/65 and OPEN 35/39,
with conflicting section bases in GAME/OPEN. Executable build success does
not imply closure of these comparison/ownership gates.
