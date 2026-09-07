# Notification message identities and typed queue

## Function Match Plan

Start at `a7b4aa1` on master with hash-verified Japanese retail. Use the
project matcher workflow and the existing `probe-gcc257-o2-g0` profiles.
Six GAME semantic views, current sources, histories and adjacent inventory
are captured under `build/constant-names/notification-identities/` before edits.

Name the 32 messages read directly from the notification atlas. Crosscheck
hardcoded caller values and retain resource-carried IDs without inventing
gameplay behavior from message text alone. Introduce byte `KfNotificationId`
storage through the ring, placement/runtime link fields, locals and pointers.
Use the promoted enum API form so legacy C retains its full-word message
parameter and modern C++ retains the enum domain. The modern gold path must
use compiler varargs access rather than assuming a byte enum occupies the
legacy named-argument home slot. Keep the original low-halfword extraction
for the pinned retail compiler. Type the existing idle/hold/rotate-out phase
as a separate byte enum, preserving all consumers and no phase-1 meaning.

The units implement game messages, progression, item/map interaction and
render/resource policy, with no selected function in the vendored roster.
SDK GPU/audio/file calls remain provider boundaries; no SDK body or original
compiler attribution is claimed. Palette-coordinate and runtime material
globals need their own complete-owner audit before promotion.

Force affected compilations and require unchanged code/data/ordered
relocations for all 112 objects, all 484 strict scores and the whole report.
Verify exact controls against complete retail words including delay slots.
Run modern type checks and actual-header positive/negative probes, inventory,
Ruff, repository tests, whitespace and full kf build before commit. Add no
size assertions or per-field repository tests. Refresh caller literal ledgers
and account for every retained notification-unit literal. No new bank is planned.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x8001fa44 / 160` | `notify_enqueue` | 100.0 | 24 proven call sites; full-word a0 tests 255 and 19, then stores its low byte. Gold alone reads the low halfword of a1 at sp+4. Preserve this variadic ABI and head/full-ring gates. |
| `0x8001fae4 / 24` | `notification_digit_set_v` | 100.0 | Control: digit times 11 is the atlas row coordinate; preserve argument width and byte store. |
| `0x8001fafc / 716` | `notify_effect_update` | 99.385475 | Byte message loads and phase states 0/2/3; preserve none=255, gold=19, nibble UV arithmetic, 15-update hold, 128-angle exit step and duplicate non-gold coalescing. |
| `0x8001bce0 / 728` | `render_initialize` | 100.0 | Clear phase to idle and fill all eight ring bytes with 255 using a byte pointer. Palette/page setup is the atlas provenance control. |
| `0x8001fde4 / 1304` | `render_frame` | 100.0 | Control: text and gold sprites use the notification material, four digit sprites use the digit material. Keep exact active-byte-one checks. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | All three learning milestones notify with ID 1 in call delay slots; keep learning gates and thresholds. |
| `0x80015f28 / 152` | `player_increment_physical_power_training` | 100.0 | ID 30 accompanies base physical-power increase below the cap; preserve the capped branch and recalculation call. |
| `0x80015fc0 / 152` | `player_increment_magic_training` | 100.0 | ID 31 accompanies base magic increase below the cap; preserve the capped branch and recalculation call. |
| `0x80016058 / 548` | `player_add_experience` | 88.824814 | ID 0 follows each completed level increase; it is a valid message, not the empty-ring sentinel. |
| `0x80018054 / 1116` | `player_use_item` | 100.0 | ID 4 reports a mismatched key; ID 18 reports inapplicable use, an already-cleared link or missing target. Preserve all item, consumption and sound behavior. |
| `0x8003425c / 136` | `map_ambient_script_floor3` | 100.0 | Teaching Resist Fire/Bless emits ID 1 once after the paired learning writes. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Bracelet and dialogue learning each emit ID 1 under independent gates. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Mirror exchange teaches Healing and emits ID 1 after item decrement. |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.436745 | Tile notices use IDs 12/23/24/17, stack-full uses 16, and gold uses 19 with a promoted integer payload. Two call sites read link bytes +6/+7 directly; phase comparisons remain in their own domain. |
| `0x80030f7c / 96` | `map_object_pool_clear` | 100.0 | Control: clear the complete link block as two words, including its message bytes; zero is retained, not changed to none. |
| `0x80031008 / 1096` | `map_object_pool_load` | 100.0 | Control: copy the complete eight-byte link block from resource placement to runtime object without normalizing message IDs. |
| `0x8001b180 / 528` | `common_resources_load` | 100.0 | Control: upload concatenated MIX.TIM images; no changes to resource bytes, texture ownership or loading order. |

## Retail atlas and message names

The sixth concatenated TIM in `COM\MIX.TIM` supplies the notification
text atlas. The full file is 331,488 bytes, SHA-256
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
Its TIM begins at file offset 264864, palette data at 264884 and texture
data at 265408. The texture is 4-bit, 64 VRAM words by 256 rows, uploaded
at (832,256). GAME halfwords `0x80055db4/0x80055db6` select CLUT (0,499),
the first 16-color row of that TIM. `render_initialize` builds the material;
`render_frame` selects it for ordinary text and the gold-label sprite.

Retail `notify_effect_update` computes U from `(id & 0xf0) << 3` and V
from `(id & 0x0f) << 4`, then stores byte coordinates. IDs 0..31 select two
128-pixel column pitches and sixteen 16-pixel row pitches. The renderer
uses the stored spans 127/15. High ID bits can alias these cells after the
byte U store; they are not additional named messages. In particular, only
the full byte 19 gets the gold payload/digit behavior, and byte 255 remains
the empty-ring sentinel. No validation or normalization has been added.

These names identify the message text. They do not by themselves prove an
object model or tile behavior. Proper-name romanizations identify the printed
labels only; `RAITO` preserves ライト without choosing Light/Wright lore.

| ID | Enum name | Japanese text | Meaning |
| --- | --- | --- | --- |
| 0 | `KF_NOTIFICATION_LEVEL_UP` | レベルが上がった | Level increased |
| 1 | `KF_NOTIFICATION_MAGIC_LEARNED` | 魔法を覚えた | Learned magic |
| 2 | `KF_NOTIFICATION_LOCKED` | 鍵が掛かっている | Locked |
| 3 | `KF_NOTIFICATION_SEALED` | 封印されている | Sealed |
| 4 | `KF_NOTIFICATION_KEY_DOES_NOT_FIT` | 鍵が合わない | Key does not fit |
| 5 | `KF_NOTIFICATION_NOTHING_THERE` | なにもない | Nothing there |
| 6 | `KF_NOTIFICATION_NOTHING_INSIDE` | なにも入ってない | Nothing inside |
| 7 | `KF_NOTIFICATION_DEAD` | 死んでいる | Dead |
| 8 | `KF_NOTIFICATION_BROKEN_SWORD` | 折れた剣 | Broken sword |
| 9 | `KF_NOTIFICATION_OLD_BONES` | 古い骨 | Old bones |
| 10 | `KF_NOTIFICATION_SCHOLAR_BONES` | 学者の骨 | Scholar's bones |
| 11 | `KF_NOTIFICATION_HANS_CORPSE` | ハンスの死体 | Hans's corpse |
| 12 | `KF_NOTIFICATION_PITFALL` | 落とし穴 | Pitfall |
| 13 | `KF_NOTIFICATION_HEALING_SPRING` | 回復の泉 | Healing spring |
| 14 | `KF_NOTIFICATION_REINHARDT_II` | ラインハルト２世 | Reinhardt II |
| 15 | `KF_NOTIFICATION_INVISIBLE_WALL` | 見えない壁がある | Invisible wall present |
| 16 | `KF_NOTIFICATION_CANNOT_CARRY_MORE` | これ以上持てない | Cannot carry any more |
| 17 | `KF_NOTIFICATION_HIDDEN_DOOR` | 隠し扉がある | Hidden door present |
| 18 | `KF_NOTIFICATION_NOTHING_HAPPENS` | なにも起こらない | Nothing happens |
| 19 | `KF_NOTIFICATION_GOLD` | ゴールド手に入れた | Acquired gold |
| 20 | `KF_NOTIFICATION_RAITO_FAMILY_GRAVE` | ライト家の墓 | Grave of the Raito family; canonical Latin spelling unresolved |
| 21 | `KF_NOTIFICATION_GRAVESTONE` | 墓石 | Gravestone |
| 22 | `KF_NOTIFICATION_BROKEN_SHOVEL` | 折れたシャベル | Broken shovel |
| 23 | `KF_NOTIFICATION_POISON_HOLE` | 毒の穴 | Poison hole |
| 24 | `KF_NOTIFICATION_BOTTOMLESS_PIT` | 底なし穴 | Bottomless pit |
| 25 | `KF_NOTIFICATION_WIND_SEAL` | 風の封印 | Wind seal |
| 26 | `KF_NOTIFICATION_FIRE_SEAL` | 火の封印 | Fire seal |
| 27 | `KF_NOTIFICATION_EARTH_SEAL` | 土の封印 | Earth seal |
| 28 | `KF_NOTIFICATION_WATER_SEAL` | 水の封印 | Water seal |
| 29 | `KF_NOTIFICATION_RECOVERED` | 回復した | Recovered |
| 30 | `KF_NOTIFICATION_PHYSICAL_POWER_INCREASED` | 体力が上がった | Physical strength increased |
| 31 | `KF_NOTIFICATION_MAGIC_POWER_INCREASED` | 魔力が上がった | Magic power increased |

There are 31 newly named messages; gold and the empty sentinel retain their
existing identities. All 21 formerly numeric call sites now name their
messages: seven magic-learning notices, three level/training notices, four
item-use notices, four tile notices and three inventory-limit notices.
The other three proven call sites pass the existing gold ID or one of the
two serialized link message fields. Together they account for all 24 proven
`notify_enqueue` calls. A level-up notice is ID 0; empty is 255.

The tile-to-message bindings are 58→pitfall, 63→poison hole,
93→bottomless pit and 69→hidden door. The attribute numbers remain literal
bindings in their separate tile domain; these text labels do not establish
the complete collision or damage semantics of those attributes.

## Type and calling-convention propagation

`KfNotificationId` stores a byte in the ring and in both `KfMapObjectLink`
message fields. The placement and runtime objects share that link type and
still copy its whole eight-byte representation; clearing it still writes
zero words. The enqueue assignment, dequeue locals, initializer pointer and
map callers retain the enum domain. Encoding is explicit only when extracting
atlas coordinate bits. Unnamed resource bytes are preserved.

`KfNotificationArgument` uses `KF_ENUM_PROMOTED(KfNotificationId)`: a
four-byte promoted integer for legacy C and the same enum for modern C++.
Its typedef also keeps the function signature readable by the source claim
scanner. Retail spills a0..a3 into four-byte homes, compares the full a0
against 255/19, stores its low byte, and loads the optional gold halfword
from sp+4. Those complete words remain exact. The modern branch uses
compiler varargs builtins; its only payload caller passes `s32 result`
(a 32-bit long on this target), and Clang IR reads i32 then truncates to i16.
It therefore does not depend on adjacent storage after a byte enum parameter.

`KfNotificationPhase` is distinct: idle 0, hold 2 and rotate-out 3. Phase 1
has no observed handler. The typed phase pointer, reset and map comparisons
preserve that domain. The hold counter starts at 15 and reaches zero after
15 subsequent update calls. Rotation then advances 128/4096 revolution
(11.25 degrees) per update; the fourth step reaches the 512-unit threshold
and clears the sprites. These are measured update counts, not a claim about
the designers’ timing rationale. Duplicate ordinary messages are removed
together on exit; gold notices retain their separate amounts.

The new type header is registered in the inventory header list. Two existing
inventory expectations now name the enum phase and promoted API alias.
No new repository test or size assertion was introduced.

## Verification

Forced compilation and comparison cover eight selected units and shared
header dependents. Across 112 objects, this batch changes only debug line
metadata. The independent equipment/spell selector flow campaign `8c801db`
changes `game.menu_select`; its code is outside this batch. Compiling its saved
`a7b4aa1` source with the new headers independently preserves the original
code, data and relocations. All 17 reviewed notification-family scores are
unchanged. Fourteen exact controls cover 2,107 complete retail words, with
equal ordered calls and address referents, including delay slots.

| GAME function | Final strict % | Verdict |
| --- | --- | --- |
| `notify_enqueue` | 100.0 | Unchanged; all 40 retail words and ordered referents verified. |
| `notification_digit_set_v` | 100.0 | Unchanged; all 6 retail words and ordered referents verified. |
| `notify_effect_update` | 99.385475 | Unchanged object; existing non-exact result retained. |
| `render_initialize` | 100.0 | Unchanged; all 182 retail words and ordered referents verified. |
| `render_frame` | 100.0 | Unchanged; all 326 retail words and ordered referents verified. |
| `player_recalculate_combat_stats` | 100.0 | Unchanged; all 517 retail words and ordered referents verified. |
| `player_increment_physical_power_training` | 100.0 | Unchanged; all 38 retail words and ordered referents verified. |
| `player_increment_magic_training` | 100.0 | Unchanged; all 38 retail words and ordered referents verified. |
| `player_add_experience` | 88.824814 | Unchanged object; existing non-exact result retained. |
| `player_use_item` | 100.0 | Unchanged; all 279 retail words and ordered referents verified. |
| `map_ambient_script_floor3` | 100.0 | Unchanged; all 34 retail words and ordered referents verified. |
| `map_action_script_floor3` | 100.0 | Unchanged; all 36 retail words and ordered referents verified. |
| `map_event_interact` | 100.0 | Unchanged; all 181 retail words and ordered referents verified. |
| `map_interaction_dispatch` | 83.436745 | Unchanged object; existing non-exact result retained. |
| `map_object_pool_clear` | 100.0 | Unchanged; all 24 retail words and ordered referents verified. |
| `map_object_pool_load` | 100.0 | Unchanged; all 274 retail words and ordered referents verified. |
| `common_resources_load` | 100.0 | Unchanged; all 132 retail words and ordered referents verified. |

Actual-header compiler probes accept typed field/array/API/return/phase
propagation and reject seven cases: raw ring/link/API integers, both directions
of phase/message confusion, an erased byte pointer and a magic-learning state
passed as a message. The modern notification unit compiles, including its
varargs branch. All 320 pre-existing compiler diagnostics remain identical:
64 of 112 variants pass, 48 retain existing debt.

Inventory, Ruff, all 657 repository tests, whitespace and `nix flake check -L`
pass. The full build retains the existing data ownership/placement failures:
PSX target relink 1/1, GAME 75/77, OPEN 34/38, the same six section-base
conflicts and no artifact failures. Source data matches are 10/61 after the
independent selector improvement, and all four config-data contributions pass.
No new match is banked by this naming/type batch.

The [notification literal ledger](game-notification-literal-ledger.md)
explains all 116 retained queue-unit occurrences. The refreshed
[item-use ledger](game-item-use-literal-ledger.md) has 80, and the
[floor-script ledger](game-map-script-literal-ledger.md) has 276.
Per-function snapshots, atlas decoding, compiler/ABI controls and verification
logs remain under the ignored `build/constant-names/notification-identities/`.

## Sprite visibility state

The follow-up plan is to type `KfNotificationSprite.active` as a byte-backed
`KfNotificationSpriteState`, name all hidden/visible values, and propagate the
domain through initialization, `notify_effect_update` and `render_frame`.
This state is separate from both message identity and notification effect phase.

The reviewed GAME dossiers and `game_semantic_notification_sprites.tsv` establish
the two consumers. In `notify_effect_update` (0x8001fafc, extent 0x2cc), byte
stores select gold plus digits or a text row, then clear all six rows after
rotation. In `render_frame` (0x8001fde4, extent 0x518), byte loads at 0x8002021c,
0x8002023c and 0x80020288 are compared with exactly one before emission. Keep
that exact-one predicate; this edit does not turn other byte values into true.
The six 14-byte initialized rows at GAME 0x80055d20 all start hidden.

The two game-owned bodies retain their existing calls, branches and validated
data referents. Six initializer values, eighteen stores and three comparisons
now use `KF_NOTIFICATION_SPRITE_HIDDEN` or `KF_NOTIFICATION_SPRITE_VISIBLE`.
The field's curated type and existing inventory expectation agree. Its unread
neighboring byte is still opaque; no padding or semantic identity is invented.

This names 27 inline occurrences. The queue ledger now contains 92 retained
occurrences, and the frame-renderer ledger contains 35; every remaining row
was reconciled against source expressions. Builds, tests and post-edit matches
are deferred until the naming pass finishes, at the user's request. Earlier
match results above describe earlier source, not verification of this edit.
