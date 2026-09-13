# Complete enum equality review

This is the historical `bc4127a2` snapshot. The subsequent
[complete enum reuse review](enum-reuse-review.md) supersedes its alias and
retention decisions where documented, and owns the current coverage check.
The census and commands below describe this earlier revision.

This continues the [first shared-constant review](shared-constant-review.md)
on PR #3. The starting revision is `da7c89a0`: 1,951 declarations, 341 values,
and **179 duplicate-value groups containing 1,789 declarations** across the
101 active manifest variants. Every one of those groups was reviewed using
names, declarations, owning fields, arithmetic and call/dispatch consumers.
This is a source-model review, not recovery of original enum/header spelling.

The curated [decision ledger](enum-equality-review.tsv) has one row per value.
Its `members` column lists every `file:name` in the group, including typed
header declarations, local declarations and existing aliases. The full census
retains their image/define contexts. Exact memberships bind each decision to
the census; they do not make the semantic decision automatically.

- `alias`: this pass expresses one or more supported relationships in that
  group. Other members remain separate for the reasons in the row.
- `linked`: the useful relationship is already expressed by an alias or
  derived initializer. No additional change is needed.
- `retain`: keep the separate declarations. Equality does not establish one
  owner, representation, quantity or policy for their consumers.

Large groups at 0, 1 and 255 are included in full. A state-machine start,
array origin, GPU mode and success result do not become one constant because
all encode zero. Likewise a free object, absent spell, terminal animation
phase and maximum color channel have different meanings despite encoding 255.
The unresolved map-attribute series retains its WIP names; equal item or
notification IDs add no evidence of meaning.

## Supported relationships

Types, storage widths, signatures and function bodies remain intact. Local
names remain useful at boundaries; their initializers now express the shared
value rather than independently repeating its literal.

| Relationship | Source and retail evidence |
| --- | --- |
| Combat component order | `player_recalculate_combat_stats` reads the weapon's cutting/striking/piercing/holy/fire fields in the same order used by `actor_apply_damage`; `actor_try_attack_player` passes the first three components to player damage. `combat.h` now owns that order and the physical/full component counts. Actor attack/defense and weapon index names alias it. These are component positions, distinct from equipment slots and effect IDs. |
| Map behavior to action | `map_object_action_from_behavior` transports the byte encoding directly in door/link dispatch; explicit copy/reveal/restore/switch dispatches confirm the corresponding cases. Action aliases now refer to their definition behavior encodings. Both typed domains remain. |
| Map pickup model to item | `map_interaction_dispatch` passes the object's encoded ID to the pickup menu. The sword-transform cutscene changes the dragon-sword object to moonlight-sword while updating the corresponding inventory items; placement/drop consumers identify the chalice, seals and coin. Those eight named object IDs alias the matching item encodings. Non-item model IDs remain independent. |
| Floor-item appearance masks | The typed appearance flags and the `floor_item_appearance` / `floor_item_facing` helpers describe the same low frame-count nibble and high facing nibble. The typed flags now alias the masks used by those helpers. RNG bit counts and unrelated nibble masks remain separate. |
| Overlay intro/ending protocol | PSX `main` copies GAME's returned result into OPEN's request; GAME intro/ending result members now alias the encoded OPEN modes. GAME's no-exit result stays distinct. Retail PSX `0x80010054..0x80010060` loads and transports the returned word without an ID lookup. |
| Save format result | The two format-result names already occupy the same `KfSaveResult` channel. The confirmation spelling now aliases format-required. Card event/status codes remain separate, with their existing explicit mappings. |
| Camera pitch limit | GAME `player_update` clamps the shared camera to ±191 (`0x800190c4`, `0x800190fc..0x8001910c`). It passes that camera to `map_interaction_dispatch`; the container path approaches the same lower bound with an unsigned subtraction and 1,858-value span (`0x80035264`). Both local policy names now derive from `KF_PLAYER_CAMERA_PITCH_LIMIT`. |
| Normal fog | GAME display initialization sets `render_state.fog_near_distance` and the GTE fog parameter to the shared initial distance. `player_update` restores 11,000, or interpolates toward it after darkness (`0x800199dc`, `0x80019a14`). Its normal-fog name now aliases the existing shared distance. Actor attack range remains separate. |
| Orbit sound radius | GAME `effect_projectile_update_2d` supplies 5,000 as the audible radius in the call delay slot at `0x8003841c`; the same record's sound latch clears when distance reaches 5,000 at `0x8003849c..0x800384ac`. The reset radius now derives from the audible radius. Other equal sound ranges, damage scales and charge values retain separate owners. |
| Main RAM capacity | GAME/OPEN `memory_malloc_checked` tests an allocation against offset `0x1fffff`; overlay stack arithmetic uses the same 2 MiB RAM capacity. `memory_layout.h` owns `KF_MAIN_RAM_BYTES`, with allocator and overlay aliases. This is a fixed capacity, not an object address, linker placement or assertion about the heap/stack split. |

The semantic evidence includes retail disassembly/CFG, callers, callees,
strings, referenced data, relocation bindings, current strict matches and
source history for the consumer campaign below. The existing actor, map-object,
item, rendering and overlay dossiers supply adjacent-family context. This pass
changes no curated relocation, identity, data-owner or vendored classification.
SDK callees remain library controls, not reconstructed game progress.

The inventory layout reader's deliberately limited enum parser also needs to
follow the new component-count aliases. It now reads the common combat and
memory-capacity headers and resolves a bare alias only when its target is
already known. Unsupported expressions, missing/self-referential names and
unknown implicit successors remain unresolved. Regression controls check
cross-header counts, alias chains, implicit continuation and rejected guesses;
the inventory check still needs only Python, without a Clang dependency.

## Similar names retained

The ledger records the full review; these are useful examples of decisions
that require more than comparing names:

- `KF_ASSET_ARCHIVE_HEADER_BYTES` and `KF_RESOURCE_CHUNK_HEADER_BYTES` both
  equal four. The former skips an archive count header; the latter skips a
  payload-length word in the enclosing stream. GAME resource loading removes
  the outer header before calling the archive loader, which removes the
  inner header. They describe different formats.
- Direct save-message IDs and zero-based menu texture indices are different
  conventions. `SAVE_MESSAGE_KEEP_CARD_INSERTED = 103` loads image 103;
  `MENU_TEXTURE_LOADING_DATA = 103` loads image 104. The analogous equality
  at 114 refers to images 114 and 115. No alias is warranted.
- OPEN entity capacity and model-ID limit both equal 32. One bounds pool
  iteration; the other tests each entity's selected model. They are separate
  axes, including when used in the same loop.
- GAME and OPEN projected-vertex capacities both equal 1,000 but describe
  separate runtime owners whose original extents remain WIP. Equality is
  insufficient to assert one shared allocation policy.
- Equipment and item-preview name X coordinates both equal 174. Their
  widgets use different Y origins and row pitches. Map-marker and menu-marker
  OT depths likewise belong to different primitive families.
- Actor high-step/long-jump/player fast-step velocities, individual weapon
  magic thresholds, status durations and separate effect rotation rates keep
  their independent control branches. Even matching units and values do not
  establish a shared tuning parameter.
- OPEN transitions and GAME warp shimmer use a related staggered-cylinder
  algorithm. They still select different model/pool owners, horizontal scales
  (Q12 4,096 versus 6,144) and lifetime modes. Retain their independently named
  visual parameters while shared asset/policy identity remains unestablished.

## Reconcile coverage

The ledger includes the additional canonical declarations introduced by this
pass. Re-run the full census and compare exact memberships, not just counts:

```sh
nix develop -c kf enums --json > build/enum-review-current.json
nix develop -c python - <<'PY'
import collections
import csv
import json

current = collections.defaultdict(set)
for member in json.load(open('build/enum-review-current.json'))['constants']:
    current[member['value']].add(member['file'] + ':' + member['name'])
current = {value: members for value, members in current.items() if len(members) > 1}
with open('docs/patterns/enum-equality-review.tsv') as stream:
    rows = list(csv.DictReader(stream, delimiter='\t'))
reviewed = {int(row['value']): set(row['members'].split(';')) for row in rows}
assert len(reviewed) == len(rows), 'duplicate ledger value'
assert reviewed == current, 'enum groups/members changed: semantic re-review required'
assert all(row['decision'] in {'alias', 'linked', 'retain'} and row['reason'] for row in rows)
print(len(rows), 'groups covered;', sum(map(len, current.values())), 'members')
PY
```

This review covers active enum declarations exposed by `kf enums`; it does
not claim to review every literal, inactive preprocessor branch or external
SDK enum. Fresh groups or members require a fresh semantic decision.

## Verification and per-function verdicts

The final census has **1,960 declarations** and the same 179 duplicate-value
groups, now containing 1,798 declarations. All 1,951 starting declaration
identities and evaluated values are preserved; nine canonical declarations
were added. The ledger records **23 alias groups, 11 already-linked groups,
and 145 retained groups**. Exact group membership reconciles with the fresh
full census.

All 20 consumer units were explicitly rebuilt with `kf try`; its raw listing
differences remain comparison diagnostics, not strict match verdicts. Fresh
`kf analyze compare` output and full `kf build` preserve all 101 reconstructed
objects: allocated bytes, BSS/COMMON extents, alignments, symbol metadata and
ordered relocations. All 484 strict function scores are unchanged (470 exact).
All three native executables are byte-identical to the captured pre-PR
baseline. No additional function is banked.

| Image | Consumer/control | VA / bytes | Final strict verdict |
| --- | --- | --- | --- |
| PSX.EXE | `main` | `0x80010028 / 0xd0` | 100% exact, preserved |
| GAME.EXE | `game_main_loop` | `0x800146b8 / 0x2e4` | 100% exact, preserved |
| GAME.EXE | `player_recalculate_combat_stats` | `0x80015714 / 0x814` | 100% exact, preserved |
| GAME.EXE | `player_equip_weapon` | `0x80016a30 / 0xf4` | 100% exact, preserved |
| GAME.EXE | `player_begin_weapon_attack` | `0x80016b24 / 0x9c` | 100% exact, preserved |
| GAME.EXE | `player_update_weapon_attack` | `0x80016bc0 / 0x264` | 100% exact, preserved |
| GAME.EXE | `player_update` | `0x80018880 / 0x1a1c` | 99.969480%, preserved residue |
| GAME.EXE | `memory_malloc_checked` | `0x8001aab0 / 0x38` | 100% exact, preserved |
| GAME.EXE | `display_initialize` | `0x8001bb94 / 0x14c` | 100% exact, preserved |
| GAME.EXE | `render_floor_item` | `0x8001ed90 / 0x14c` | 100% exact, preserved |
| GAME.EXE | `render_weapon` | `0x8001f798 / 0x118` | 100% exact, preserved |
| GAME.EXE | `memory_card_check_or_format` | `0x8002b4d8 / 0xf8` | 100% exact, preserved |
| GAME.EXE | `actor_apply_damage` | `0x8002d120 / 0x388` | 100% exact, preserved |
| GAME.EXE | `actor_try_attack_player` | `0x8002d6a0 / 0x158` | 100% exact, preserved |
| GAME.EXE | `map_object_pool_load` | `0x80031008 / 0x448` | 100% exact, preserved |
| GAME.EXE | `map_object_spawn_actor_debris` | `0x800319c8 / 0x18c` | 100% exact, preserved |
| GAME.EXE | `map_object_pool_trigger_link` | `0x80031b54 / 0xf0` | 100% exact, preserved |
| GAME.EXE | `map_object_pool_update` | `0x80031cc8 / 0xc18` | 100% exact, preserved |
| GAME.EXE | `map_floor5_transition_cutscene` | `0x800346a8 / 0x38c` | 100% exact, preserved |
| GAME.EXE | `map_interaction_dispatch` | `0x80034de4 / 0x904` | 100% exact, preserved |
| GAME.EXE | `effect_projectile_update_2d` | `0x80038298 / 0x260` | 99.934210%, preserved residue |
| OPEN.EXE | `opening_ending_scroll_run` | `0x80014e28 / 0x798` | 99.917694%, preserved residue |
| OPEN.EXE | `opening_run` | `0x800156bc / 0x214` | 100% exact, preserved |
| OPEN.EXE | `memory_malloc_checked` | `0x80015dd4 / 0x38` | 100% exact, preserved |
| OPEN.EXE | `display_initialize` | `0x80016adc / 0x1d8` | 100% exact, preserved |
| OPEN.EXE | `render_floor_item` | `0x800190f4 / 0x14c` | 100% exact, preserved |

The first remaining normalized differences are unchanged:

| Consumer | First difference |
| --- | --- |
| GAME `player_update` | At +0, target stack allocation is 224 bytes; compiled allocation is 216. |
| GAME `effect_projectile_update_2d` | At +0, target stack allocation is 120 bytes; compiled allocation is 56. |
| OPEN `opening_ending_scroll_run` | At +0x29c, target HI16 names `opening_ending_scroll_camera_path`; compiled HI16 names `.data` with the existing section-relative addend. |

These observations retain the existing unattributed residues and relocation
spelling difference; they are not new closure or compiler-wall claims.

Final validation passes: `ruff check scripts tests`, 802 repository tests
(9 skipped), `kf check-types` for all 101 variants, `kf inventory check`,
`git diff --check`, and `nix flake check -L`. The isolated flake suite runs
802 tests with 142 skips because local retail/build inputs are absent there.
The pre-existing analysis data/ownership/placement gate failures documented
in the first review remain unchanged; native build success and unchanged
function scores do not claim those gates pass.
