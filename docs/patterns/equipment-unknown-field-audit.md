# Weapon and armor unknown-field follow-up

This bounded GAME audit follows the remaining fields in `KfWeaponRecord`
(`unknown_00`, `unknown_14`, `unknown_22`) and `KfArmorRecord`
(`unknown_00`, `unknown_12`). It extends the
[original field review](unknown-field-review.md) through stored equipment
pointers, the item-ID base bias, GP-register uses, and independent item/shop
metadata. **No semantic rename or layout change is supported by this pass.**
The result is a set of checked paths and rejected simple interpretations,
not a claim that all possible retail aliases are closed.

The [three-weapon-field access report](weapon-unknown-access-report.md)
adds a fresh typed census, per-field copy/read/write logic, resolves specific
proposal warnings and both equipment switches, and enumerates conditional
weapon-byte reads if a weapon-range item ID reaches an armor slot. The
ordinary-pointer conclusions below assume the intended equipment ID ranges.

## Actual equipment pointer paths

`common_resources_load` passes COM.DAT chunk 2 to the weapon loader and
chunk 3 to the armor loader. The weapon loader copies 176 words from the
source (`lw` at `GAME:0x800150b4`, `sw` at `0x800150c0`) into
`weapon_records` at `0x8009ff10`, then negates the Y rotation halfword at
record +0x26. The armor loader copies 294 words (`0x80015108/0x80015114`)
into `armor_records` at `0x800a0248`. These whole-table copies include every
unknown byte and establish transport, not meaning.

The authored populations are different from the copy capacities: chunk 2
has sixteen 44-byte rows, of which twelve are named weapons; rows 12–15
are additional unnamed rows and must not be labelled as armor items merely because their
numeric indices overlap item IDs. Chunk 3 has twenty-seven 28-byte rows,
including twenty-three named armor items, three gaps and a zero row at the
gold-coin item index. Its 42-row loader extent crosses the next chunk; the
extra copied bytes are not additional authored armor definitions.

`player_equip_weapon` stores `weapon_records + weapon_id * 44` at
`player_state+0x68` (`GAME:0x800a07e8`). `player_set_equipment_slot` stores
five armor pointers at player offsets +0x7c, +0x80, +0x84, +0x88 and +0x8c
(head, body, shield, arm, leg). Each resolves
`armor_records + (item_id - 13) * 28`.

The compiler materializes the armor calculation's base as `0x800a00dc`,
which is physically inside weapon row 10's +0x14 range. The ten known
base-building sites in equipment assignment and stat recalculation are
therefore armor address calculations, not weapon-price reads. Propagating
that physical address as a weapon pointer without the later 28-byte stride
produces false unknown-field hits.

| Consumer | Checked dereferences through these pointers |
| --- | --- |
| `player_recalculate_combat_stats` | Weapon halfwords +2/+4/+6/+8/+0x0a; armor halfwords +2/+4/+6/+8/+0x0a/+0x0c. The first armor +2 reads are at `0x800158b8/0x800158cc`. |
| `player_update_weapon_attack` | Weapon charge byte +1 at `0x80016db0` and attack offset halfword +0x12 at `0x80016c58`. |
| `player_update` | Weapon regeneration halfwords +0x0c/+0x0e at `0x80019b28/0x80019b78`; each armor pointer supplies +0x0e/+0x10. The head-armor pair is `0x80019bdc/0x80019c2c`. |
| `render_weapon` | Projection +0x10 at `0x8001f7dc`, translation +0x1c/+0x1e/+0x20 at `0x8001f7f4/0x8001f800/0x8001f80c`, and a rotation pointer at +0x24 passed to SDK `RotMatrix`. |
| `RotMatrix` | The selected weapon input supplies signed XYZ rotation halfwords +0x24/+0x26/+0x28. |

None of these ordinary dereferences overlaps the five unresolved ranges.
A second negative control concerns the load delay at a CFG boundary:
`0x80019b70` reloads the weapon pointer, `0x80019b74` is `nop`, and
`0x80019b78` reads +0x0e. A conservative proposal retaining the old `lui`
value as well as the loaded pointer incorrectly reported a +0x22 read.
The raw schedule selects the loaded pointer. Five equivalent armor reload
boundaries resolve to armor +0x10, not weapon +0x24.

The player-state save copy transports the pointer values as part of the
aggregate (`save_file_write_slot`, loop `0x8002b8f0..0x8002b918`). The
corresponding load restores that aggregate. This is not a read of the
pointed-to unknown bytes. Arbitrary pointer values from altered save data
were not enumerated as intended equipment consumers.

## GP and analysis bounds

GAME `start` sets GP to `0x80057b0c` at `0x8003ac5c/0x8003ac60`, then tails
to `main`. A raw instruction sweep of the 933 inventoried contiguous GAME
bodies found no memory instruction using GP as its base, and no other
GPR arithmetic use of GP. COP2 instructions whose encoded command bits
coincide with GPR field number 28 are not GP uses. The two fragmented
`SquareRoot0`/`InvSquareRoot` bodies and bytes outside the function census
remain outside this sweep. This closes the proposed direct GP bridge in
that admitted body set; it does not certify all executable bytes.

The scoped affine proposal followed fixed bases, the verified armor bias,
direct-call inputs, stack spills and discovered global pointer stores. It
reached a fixed point after two rounds and proposed accesses in seven
functions. It was manually corrected using the bias and load-delay controls
above. The proposal retains limitations for nonaffine arithmetic, incomplete
indirect successors, whole-aggregate pointer transport and unmodelled runtime
pointer values. Numerical address recognition can also mistake a transient
`lui 0x800a` for an interior weapon pointer before later arithmetic resolves
the actual owner; such proposals are not semantic findings.

## Correlations and discriminating counterexamples

Armor +0's low byte agrees with the existing equipment-slot codes for all
twenty-three named rows: head 0, body 1, arm 2, leg 3, shield 4. But the
actual `menu_equip_select` calls pass the slot independently as a constant:
shield `a1=4` at `0x80023dbc`, head zero at `0x80023dd8`, body 1 at
`0x80023df4`, arm 2 at `0x80023e38`, and leg 3 at `0x80023e54`.
This path does not load armor +0 to select the slot. The high byte varies
from 2 to 99 among named armor rows. Resemblance to weapon +1 or a plausible
weight progression supplies no consumer proving its purpose or a byte split.
`u16 unknown_00` therefore remains intact.

Weapon +0 groups Battle Axe, Morning Star and Crescent Axe under value 1;
Colichemarde and Triple Fang have 2; the other seven named weapons have 0.
An attack-style or weapon-class interpretation remains possible, but it is
not established by a dispatch using this byte. It is not simply the strongest
physical attack-component index: Triple Fang has cutting 7, striking 3 and
piercing 4 while its byte is 2. The reviewed attack path uses the equipped
item ID for its special timing case and the existing components for damage.

The two little-endian words at +0x14/+0x18 in each record look monetary.
Independent operative prices are loaded from STAT.DAT at +0x13cc (buy) and
+0x150c (sell), each an 80-by-two-halfword table. Buying uses
`item_buy_prices` at `0x800594b8`, selling uses `item_sell_prices` at
`0x800595f8`, and detail rendering selects between those same tables.
No conversion from the opaque COM pair was found on these paths.

| Authored item | COM +0x14/+0x18 words | STAT buy columns | STAT sell columns |
| --- | --- | --- | --- |
| Short Sword | 150 / 100 | 180 / 200 | 150 / 120 |
| Triple Fang | 3700 / 3500 | 6500 / 7800 | 5300 / 6200 |
| Dragon Sword | 23000 / 160 | 250 / 18000 | 200 / 30 |
| Full Plate | 3500 / 1200 | 1800 / 1900 | 1500 / 1550 |
| Stone Hand | 4800 / 0 | 1900 / 2800 | 1580 / 2300 |
| Feather Boots | 0 / 0 | 4300 / 6300 | 3600 / 5250 |

The second COM word is exactly four-fifths of the first for eight of twelve
named weapons and thirteen of twenty-two named armor rows with a nonzero
first word. Feather Boots adds a vacuous 0/0 equality. The first word equals
one of the actual sell columns for six weapons and eleven armor items;
those partial agreements cannot define a general derivation. Dragon Sword,
Full Plate and Feather Boots reject simple universal current-price rules.
Earlier authoring prices, overridden defaults or another historical format
remain hypotheses requiring a reader, converter source, or independently
documented matching record grammar. Neither the word split nor names such
as `base_price`, `sell_price` or `weight` are promoted from these numbers.

Weapon +0x22 and armor +0x12/+0x13 are zero in all authored rows. Known
weapon translation precedes +0x22 and the SDK rotation begins after it;
that does not make the intervening halfword an SDK `pad`. The armor zero
pair likewise precedes the opaque words without establishing padding.

## Final function verdicts and verification

All entries below were queried with `addr`, block disassembly, callers,
callees, strings and strict `match`, explicitly selecting GAME. Match queries
inspect existing results; no source or build input changed in this pass,
and no new exact-function or banking claim is made. Local dossiers and raw
proposals are under `build/equipment-dossiers/`,
`build/equipment-alias-raw-accesses.json`, and
`build/equipment-resource-matrix.json`. They are not curated inputs.
The GP sweep reports are `build/equipment-gp-sites.json` and
`build/equipment-gp-register-reads.json`; the load-delay witness is in
`build/equipment-dossiers/game-player_update.txt`.

| Function | Bounded final verdict |
| --- | --- |
| `weapon_records_load_and_mirror_angles` | Whole-record transport plus known Y negation; unknown fields copied without an interpretation. |
| `armor_records_load` | Whole-record transport; 42-row extent must not be confused with 27 authored rows. |
| `common_resources_load` | Establishes COM chunk provenance and typed loader boundaries. |
| `player_set_equipment_slot` | Establishes five stored armor pointers from independent slot arguments and the item-ID bias. |
| `player_equip_weapon` | Establishes the stored weapon pointer and item-ID-selected asset. |
| `player_recalculate_combat_stats` | Checked combat dereferences miss unresolved ranges; accessory-switch successors remain a separate CFG limit. |
| `player_update` | Checked loaded-pointer regeneration paths miss unresolved ranges; resolves false load-boundary hits. |
| `player_update_weapon_attack` | Known charge/attack-offset reads; no unknown-byte attack-class dispatch established. |
| `render_weapon` | Known projection/translation and SDK rotation boundary; no +0x22 read. |
| `RotMatrix` | Vendored boundary: selected rotation XYZ reads; not game reconstruction progress. |
| `menu_equip_select` | Independent hardcoded slot arguments; does not prove armor +0 as the controlling field. |
| `item_load_database` | Independent STAT buy/sell table provenance. |
| `item_menu_buy` | Operative buy-price table used for affordability and gold deduction. |
| `item_menu_sell` | Operative sell-price table used for gold credit. |
| `menu_draw_item_detail` | Displays the independent buy/sell prices; no COM-pair derivation established. |
| `save_file_write_slot` | Copies player aggregate, including pointer values; no pointed-to unknown-field read implied. |
| `save_file_read_slot` | Restores player aggregate; arbitrary saved pointer values remain outside intended-consumer closure. |
| `start` | Vendored startup establishes GP; admitted body sweep gives no equipment GP-relative bridge. |

The next discriminator is specific: find an intended reader or writer of
the remaining record bytes, or authentic producer/format evidence tying
these exact offsets and complete records to a meaning. More correlations
among the same rows do not resolve that missing link.
