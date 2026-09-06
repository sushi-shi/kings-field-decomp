# Player stats, progression, and equipment

This pass identifies seven related GAME functions and the state they share. It
does not claim that address adjacency is an original translation unit. The
names describe behavior supported by instructions, callers, data references,
and the Japanese manual; they are not recovered source symbols.

| Address | Identity | Evidence summary |
| --- | --- | --- |
| `0x80015714` | `player_recalculate_combat_stats` | Rebuilds effective stats and attack/defense lanes from player, status, weapon, armor, and accessory state. |
| `0x80015f28` | `player_increment_physical_power_training` | Raises base physical power after 100 hidden progress points. |
| `0x80015fc0` | `player_increment_magic_training` | Raises base magic after 100 hidden progress points. |
| `0x80016058` | `player_add_experience` | Adds a signed 16-bit award and applies every crossed level threshold. |
| `0x800167e4` | `player_select_magic` | Selects a 20-byte-stride magic record and clears MAGIC charge. |
| `0x80016848` | `player_set_equipment_slot` | Updates one of six equipment slots and resolves armor record pointers. |
| `0x80016a30` | `player_equip_weapon` | Selects a 44-byte-stride weapon record, loads its image, and clears POWER charge. |

The [original FromSoftware manual](https://media.fromsoftware.jp/fromsoftware/jp/static/pdf/001_KING_S_FIELD.pdf)
labels the two underlying status values `体力` and `魔力`; the inventory uses
`physical_power` and `magic`. It also shows total attack/defense and their
component lanes. The code proves five outgoing attack halfwords and six
defense/resistance halfwords, but it does not yet prove which internal lane
corresponds to every displayed label. Those identities deliberately remain
numbered.

## Progression layouts

`player_progress_state` is a four-byte `KfPlayerProgressState`:

| Offset | Field | Type | Evidence |
| --- | --- | --- | --- |
| `0x00` | `level` | `u8` | Indexed and incremented by `player_add_experience`. |
| `0x01` | `unknown_01` | `u8` | Extent is known; meaning is not. |
| `0x02` | `current_floor` | `u8` | Written by floor-transition code. |
| `0x03` | `highest_floor` | `u8` | Monotonically raised and used to clamp map-event images. |

`player_level_growth_table` contains 40 twelve-byte
`KfPlayerLevelGrowth` records copied from the gameplay asset stream:

| Offset | Field | Type |
| --- | --- | --- |
| `0x00` | `maximum_hp` | `u16` |
| `0x02` | `maximum_mp` | `u16` |
| `0x04` | `physical_power_step` | `u16` |
| `0x06` | `magic_step` | `u16` |
| `0x08` | `experience_threshold` | `u32` |

The first record seeds new-game values. Later records provide absolute HP/MP,
stat increments, and cumulative experience thresholds. At and beyond level 40,
the executable extrapolates from the final two records. HP/MP cap at 9,999,
physical power and magic at 999, and experience at 99,999.

POWER state is a four-byte `KfPlayerAttackChargeState`: current charge at
offset zero and the committed attack snapshot at offset two. MAGIC charge is a
separate `u16`. Both charge meters saturate at 5,000. The physical-power and
magic training counters are separate halfwords and should not be merged into
either charge object.

## Equipment state

The selected magic and weapon each have a byte ID and a four-byte record
pointer. Five armor categories have both IDs and resolved record pointers;
the accessory slot has an ID but applies direct effects rather than retaining
a record pointer. Empty selections use ID `0xff` and a null pointer.

The weapon pointer is now typed as `const KfWeaponRecord *`. Its `0x2c` stride,
five attack-component halfwords at `+0x02`, charge-rate byte at `+0x01`, and
attack z offset at `+0x12` are supported by stat recalculation and the reviewed
attack update. All remaining bytes stay explicitly opaque; the complete table
is in
[`player-motion-and-weapon-attack.md`](player-motion-and-weapon-attack.md).

The six-slot switch orders head, body, arm, leg, shield, and accessory. The
[retail item/menu label audit](patterns/game-item-equipment-identities.md)
corrects the earlier shield/head/body misclassification: head
`0x0d..0x12`, body `0x13..0x19`, shield `0x1a..0x1f`, arm `0x20..0x22`, leg
`0x23..0x26`, and accessory/other `0x30..0x33`. These categories describe
observed runtime selection; original enum spellings remain unknown.

Two initialized compiler tables are now explicit data identities:

- `player_combat_stat_effect_jump_table`: eleven code pointers for item IDs
  `0x2a..0x34`;
- `player_equipment_slot_jump_table`: six code pointers for slot values
  zero through five.

The three packed `SoundRef` records at `0x80055810` are used by weapon attack,
death, and level-up paths. The mutable string `WEPON\\WEP00.MIM` is
`weapon_image_path_template`; `player_equip_weapon` replaces its two decimal
digits before loading the asset.

Field layouts and opaque ranges are also recorded in the retail structure
inventory. A supported layout means its size/offset access pattern is
reproducible; it does not prove the original typedef name, linkage, or source
file.
