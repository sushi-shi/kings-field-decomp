# Complete enum reuse review

The [semantic consistency follow-up](semantic-consistency-review.md) renames
the OPEN cylinder lifecycle domain and reserved slot without changing values.
The current ledger has 56 `reuse`, seven `canonical` and 239 `retain` rows,
with 241 member mappings, including the system-screen retry name; the original
review figures below describe PR #3.

Stack integration: this PR follows the native referent fix in
[PR #6](https://github.com/sushi-shi/kings-field-decomp/pull/6). A fresh comparison
against that base preserves all 101 objects, ordered relocations and all three
native executables. The combined report has 458/471 game functions exact, plus
13/13 vendored verification functions. The one additional exact game function
comes from the upstream referent fix. The standalone review and verdict ledger
below retain their original baseline figures.

This supersedes the alias-preserving decisions in the
[earlier equality review](enum-equality-review.md). Starting at `bc4127a2`, the
review covered **all 302 enum blocks and 1,960 members**, including scoped
compatibility declarations and anonymous local/header enums. The result has
**287 enum blocks and 1,835 members**: 125 fewer declarations, with 235 original
member sites renamed or moved to their canonical owner. Every retained or
replacement member has the same evaluated value.

The [complete enum ledger](../enum-reuse-review.tsv) has one reviewed row for
each starting block. It records every original member and value, the current
owning enum(s), direct member replacements, and a specific rationale. Its 54
`reuse` rows change ownership/names or consolidate domains; seven `canonical`
rows receive additional consumers/members; 241 `retain` rows preserve already
shared constants or distinct meanings. There are no pending rows. All 287
current blocks and all 1,835 current members are covered by those decisions.

The starting inventory scanned every project C source and `include/kf` header
before preprocessing, recognizing both `enum` and `KF_ENUM_BEGIN` blocks. Each
of its 1,960 members also appears in the evaluated census across all 101 C
image variants. Thus no inactive branch or unreferenced project header adds an
unreviewed enum at this revision. SDK declarations are outside this game-source
review. The inventory is a source-model audit, not proof of original enum
spelling or header/TU boundaries.

## Direct reuse

| Shared domain or policy | Evidence and resulting source |
| --- | --- |
| Combat components | Weapon and actor damage/defense arrays use the same cutting/striking/piercing/holy/fire order. The 16 actor/weapon aliases disappear; arrays, loops and indexing use `combat.h` directly. Physical attacks retain the three-component count. |
| Overlay mode | PSX `main` transports GAME's returned word directly into OPEN's request. `KfOverlayMode` owns none/intro/ending; `KfOverlayResultWord` preserves the unsigned result storage while the request remains signed. The loader no longer decodes one enum into another. |
| Map operation | Door/link dispatch passes `definition->behavior_type` directly to the running action. `KfMapObjectOperation` replaces separate behavior/action enums and their conversion helper. Definition-only and runtime-only operations remain named members of the same namespace. |
| Item/map-object ID | Pickup dispatch passes the object's ID to the item menu without remapping; weapon transformation and drop consumers confirm the shared identities. `KfObjectId` owns inventory and scenery IDs, with `KF_OBJECT_NONE` as their common absence marker. Inventory table bounds remain explicit; OPEN model IDs remain separate. |
| Spell/effect ID | `magic_cast` passes its selected spell ID to `effect_pool_construct`, whose record accesses use the same magic table. `KfEffectKind` now owns spell and effect identities. The five effect/spell aliases and four cast bridges disappear. Render model and billboard IDs remain separate tables. |
| Packed actor effect payload | The actor code contains a five-bit kind plus a paired-emission flag. Keep that packed format, its sentinels and the retail in-place mask. After masking, `actor_effect_kind_from_payload` exposes the canonical kind; 13 duplicated kind members disappear. |
| Stock bank | Shop arguments directly index `item_stock`, whose zero row holds player quantities. `KfItemStockBank` replaces `KfShopId` and the separate stock-index enum; canonical bank names serve both paths. |
| Trade mode and menu rows | Shop dispatch and pricing use buy=0/sell=1. Both use `KfTradeMode`; row indexing encodes those same members. System load/quit rows likewise use `KfMenuSystemAction` directly. Return/gold rows and actual row counts retain their layout meanings. |
| Menu results | Panel, confirmation, system and pickup loops use `KfMenuResult`. Root/list channels encode its negative controls. The magic panel's selection accepts shared controls or a `KfEffectKind` payload, so it no longer redeclares either controls or spells. Its retail representation remains `s32`; modern checks reject raw integers and unrelated item IDs. |
| Sprite state | HUD, effect and notification records use hidden=0/visible=1, with a 255 end marker where needed. They share `KfSpriteState`. Effect traversal still stops on any non-visible row, HUD traversal still looks for its end marker, and notifications retain their fixed row count. |
| Floor appearance | The packed appearance enum owns its frame/facing masks. Packing arithmetic encodes those members directly; the duplicate numeric-mask declarations disappear. The biased facing selector remains a separate representation. |
| Equipment ranges | Equipment selection and armor-record indexing now use the canonical item IDs directly. Fifteen first/end aliases disappear; actual record counts, array extents and explicit range comparisons remain. |
| Save completion/result | Card I/O completion and file success use `SAVE_STATUS_OK` directly. The duplicate format-result name disappears. Internal status, remapped menu result and cleanup result retain distinct contracts where their other numeric meanings conflict. |
| RAM, camera and fog | Allocator/overlay bounds directly use `KF_MAIN_RAM_BYTES`; player and container camera paths use `KF_PLAYER_CAMERA_PITCH_LIMIT`; normal fog restoration uses `KF_INITIAL_FOG_NEAR_DISTANCE`. These preserve hardware/policy meanings without adding global-placement assumptions. |
| Cylinder animation | GAME warp shimmer and OPEN transition both animate four staggered cylinders with the same tall scale, scale/yaw increments and frame budget. They share the `KF_CYLINDER_TRANSITION_*` constants, including shift/frame forms of the stagger. Pool ownership, horizontal scale, sound timing and different lifecycle mode encodings remain local. |
| Projected vertices | Both independent graphics runtimes use `KF_PROJECTED_VERTEX_CAPACITY` from `render_types.h`. The 1,000-vertex capacity remains a WIP extent; this does not merge the storage objects or assert a historical allocation declaration. |
| Extended sound range | Actor boss sounds and extended effect sounds pass the same 20,000/60,000 pair to `audio_play_spatial_range`. They now use the common audio policy directly. Other sound ranges and the equal actor-target distance remain separate quantities. |
| Menu rendering | Map and widget markers share the marker OT bucket; dialog, text and map artwork share the content bucket. The header owns those layers, preserving insertion order. Equipment and item-preview names share their X column, retaining their own Y origins/pitches. |
| Effect animation | Fire/darkness/lightning share the same projectile roll increment; projectile dissipation and ground trails share the same uniform scale decrement. Ground branches and their visual children share a yaw increment. Other growth, rise, roll and lifetime values keep their distinct behavior. |

## Distinctions checked across the full list

The ledger covers small Boolean-looking enums, large ID tables, phase markers,
array dimensions, local tuning and existing derived expressions, not just
identical-value groups. Retention is based on concrete meaning or encoding:

- Map orientation is one-based 1..4; actor headings are zero-based 0..3;
  floor-sprite facing is a biased high-nibble encoding.
- Actor action codes stop matching animation-slot indices after their early
  entries. Equipment menu ordering also differs from equipment-record slots.
- GAME and OPEN palette/model banks have different row meanings and orders.
  Effect kind, effect model and billboard-frame numbers likewise index
  different resources.
- Direct save-message image 103 and zero-based menu texture 103 load different
  images. Their integer equality cannot make them one resource ID.
- `MAP_REVEAL_LIFT` is **10,200**, computed from the 10,000 reveal depth plus
  five 40-unit settling steps. Removing that derivation would change behavior.
- Actor/player/object movement includes different integrators and event
  triggers. Equal individual velocities or gravity values do not turn those
  complete parameter sets into one movement policy.
- Independent dimensions remain distinct even inside one object: map rows
  versus columns, model-ID limit versus pool capacity, and glyph width versus
  window row capacity. Derived layout boundaries remain expressions.

## Coverage check

The ledger preserves the starting members as `name=value` entries. A
`member_reuse` entry maps an old name to its current `file:name`; unchanged
members retain their original file. This check requires exact final membership
and values, so a missing, extra or changed declaration requires re-review:

```sh
nix develop -c kf enums --json > build/enum-reuse-current-values.json
nix develop -c python - <<'PY'
import csv
import json

with open('docs/enum-reuse-review.tsv') as stream:
    rows = list(csv.DictReader(stream, delimiter='\t'))
assert len(rows) == 302
assert len({row['source_enum'] for row in rows}) == len(rows)
expected = {}
for row in rows:
    assert row['decision'] in {'retain', 'canonical', 'reuse'}
    assert row['reason'] and row['current_enums']
    source_file = row['source_enum'].split(':', 1)[0]
    replacements = dict(entry.split('=', 1)
                        for entry in row['member_reuse'].split(';') if entry)
    for member in row['members'].split(';'):
        name, value = member.split('=', 1)
        target = replacements.get(name, source_file + ':' + name)
        value = int(value)
        assert target not in expected or expected[target] == value
        expected[target] = value
actual = {}
for member in json.load(open('build/enum-reuse-current-values.json'))['constants']:
    key = member['file'] + ':' + member['name']
    assert key not in actual or actual[key] == member['value']
    actual[key] = member['value']
assert expected == actual
print(len(rows), 'reviewed starting enums;', len(actual), 'current members covered')
PY
```

## Binary and type verification

All 101 reconstructed objects preserve allocated bytes, BSS/COMMON extents,
alignments, symbol values/sizes and ordered relocations. The complete strict
objdiff report is unchanged: **470 exact functions and 14 existing partials**.
The [per-function verdict ledger](enum-reuse-verdicts.tsv) records every one
of the 484 reported functions. No new function is banked or claimed as game
progress; SDK/vendor classifications and curated relocation/data ownership
remain unchanged. Function/data identity types follow the shared domains.

The inventory layout reader now recognizes a typedef of
`KF_ENUM_STORAGE(domain, storage)`, so the shared overlay mode can retain its
unsigned result-word ABI. It requires a declared enum domain and supported
fixed storage, and rejects duplicate types and unrelated typedefs. Regression
controls check those rejections and the resulting field offsets/alignment.
Menu-selection controls separately verify that spell IDs and menu results are
accepted while raw integers and unrelated object IDs are rejected.

The actor payload experiment initially removed an entry `move s2,a0`; adding
another payload mask changed the frame and control sequence. Neither variant
was kept. The final source preserves the retail in-place mask and uses a typed
conversion of that payload, retaining exact output without artificial locals,
volatile state or assembly. Existing partials retain their earlier raw residue;
this pass makes no compiler/backend attribution.

The modern checker passes all 101 image variants. Full native `kf build`
succeeds for all three programs, and each executable is byte-identical to the
captured starting build:

| Image | SHA-256 before and after |
| --- | --- |
| PSX.EXE | `9a0271f0318986bf5e1e73ad68fc5aa438097dfbc7ede0763fa704b83bc1326f` |
| GAME.EXE | `3a7ec1d610059104c19a18447be1ffeb38f4c11f01fe0f4b2a2c59ae6be3c094` |
| OPEN.EXE | `7442af66d2e0df83197ea09080649bd9b9d6f3d85009a3bee4c08d6dbcea58a3` |

The full repository suite passes: **804 tests, nine skipped**. Ruff and
`git diff --check` pass. `nix flake check -L` also passes, including its
804-test run with 142 checks skipped because local retail/build artifacts are
unavailable in that isolated environment. The documented coverage check was
executed against the final census and accounts for all 1,835 current members.
