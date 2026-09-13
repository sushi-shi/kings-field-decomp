# Static accesses to the three weapon unknowns

This report follows `KfWeaponRecord.unknown_00`, `unknown_14` and
`unknown_22` at source/retail snapshot `2da5b5c8`. The selected executable
is **GAME.EXE**. `weapon_records` starts at `0x8009ff10`, contains sixteen
44-byte records, and ends at `0x800a01d0` (exclusive).

For a correctly indexed weapon record, the established accesses to these
three ranges are resource-to-table copies. The checked weapon consumers
do not interpret them. There are also **conditional reads through invalid
armor IDs**, detailed below; an unconditional claim of no possible reader
would therefore be wrong. No new field name or layout is established.

## Per-field reads, writes and dependent logic

Let `W(i) = 0x8009ff10 + 44*i`, for `0 <= i < 16`. The corresponding
resource bytes are in zero-based COM.DAT chunk 2, at the same record offsets.

| Field | Target bytes | Established source read / table write | Logic depending on the copied value |
| --- | --- | --- | --- |
| `unknown_00: u8` | `W(i)+0x00` | Word at record +0 is loaded at `0x800150b4` and stored at `0x800150c0`. The other three lanes are neighboring known fields. | None in the copy. No named field read, write or address-taking in the C census. Conditional armor reads can include this byte plus its neighboring charge byte. |
| `unknown_14: u8[8]` | `W(i)+0x14..+0x1b` | Words at +0x14 and +0x18 pass through the same load/store instructions. | None in the copy. No named field read, write or address-taking in the C census. Conditional armor reads can feed combat additions or regeneration interval checks. |
| `unknown_22: u16` | `W(i)+0x22..+0x23` | Word at +0x20 passes through the same load/store instructions; its low halfword is known translation Z. | None in the copy. No named field read, write or address-taking in the C census. Conditional armor reads can feed combat additions or regeneration interval checks. |

The owner is
[`weapon_records_load_and_mirror_angles`](../../src/game/equipment.c#L12).
Its copy loop performs exactly 176 word iterations: `a1=176`, decrement at
`0x800150bc`, `bnez a1` at `0x800150c4`, and destination advancement in
the branch delay slot at `0x800150c8`. The source word is stored verbatim;
the loop decision uses the counter, not its contents. The only subsequent
table mutation is rotation Y at record +0x26 (`lhu` at `0x800150dc`,
negation at `0x800150e4`, `sh` at `0x800150e8`), outside all three ranges.

[`common_resources_load`](../../src/game/resources.c#L69) loads
`COM\\COM.DAT`, walks its length-prefixed chunks, and calls this loader at
`0x8001b2a4`. Chunk sizes select the payload address; the three field values
do not select the path or the copy length. This describes the record's
source and destination, not every lower-level CD transfer of the file bytes.

## Typed weapon pointer paths and nearby logic

A fresh Clang parse of all **101 configured source/image variants** found
39 AST references to weapon-record members, the table, its members or
`equipped_weapon_record`. All were in GAME; none named the three unknowns.
The whole-word table view is included in that count. Macro-expanded accesses
were inspected in their containing expressions rather than counted as an
independent semantic read for each nested AST node.

| Function / pointer path | Actual access and local condition | Use of the value |
| --- | --- | --- |
| `player_equip_weapon` | If weapon ID is not 255, forms `W(id)` and stores it to `player_state+0x68` at `0x80016aa8`. | Establishes the retained pointer; does not dereference these unknown fields. Weapon ID also selects the asset filename. |
| `player_recalculate_combat_stats` | If equipped weapon ID is not 255, computes `W(id)` directly. Halfword reads +2/+4/+6/+8/+0x0a are at `0x80015804/20/3c/58/74`. | Adds cutting, striking, piercing, holy and fire attack values to player stats. The reads exclude the unknown ranges. |
| `player_update_weapon_attack` | With a weapon equipped, reads +0x12 at `0x80016c58` during the hit window; reads +1 at `0x80016db0` when inactive and charge delay is zero. | Positions the hit test; computes charge gain. The special timing branch checks the weapon ID, not +0. |
| `player_update` | With a weapon equipped, reads regeneration halfwords +0x0c/+0x0e at `0x80019b28/78`. | Tests interval against zero, then ticks modulo interval, before adjusting HP/MP. Neither interval overlaps an unknown. |
| `render_weapon` | With a weapon equipped, reads projection +0x10 and translation +0x1c/+0x1e/+0x20, and passes record +0x24 to `RotMatrix`. | Builds the model transform; a later +0x20 read supplies depth bias if animation binding succeeds. |
| SDK `RotMatrix` | Reads signed halfwords at input +0/+2/+4, twice each for cosine/sine, then no longer uses the input pointer. | Selected weapon bytes are +0x24/+0x26/+0x28. The preceding unknown halfword is not read. |
| `save_file_write_slot` / `save_file_read_slot` | Copy the entire 224-byte player state, including its equipment pointer slots and item IDs. | Transport the pointer values, not the pointed-to weapon records. A successful load restores `equipped_weapon_record`; only the asset-buffer and animation-cache pointers are explicitly preserved across the load. |

The only weapon subobject pointer passed to an external helper on these
paths is the known rotation input. No address of one of the three unknown
fields is passed to a helper. This is a result of this census and the raw
paths, not an assertion that a pointer's C type inherently identifies its
containing object.

## Conditional armor aliases: concrete readers with explicit prerequisites

The armor calculations deserve a distinction absent from a simple member
search. For a byte item ID `id`, the retail calculation is

```text
A(id) = 0x800a0248 + (id - 13) * 28
      = 0x800a00dc + id * 28
```

The consumers reject ID 255 but do not locally enforce the armor item
range. If ID 0..8 reaches an armor slot, the resulting address can be in
the weapon table. This is raw retail address arithmetic for inputs outside
the modeled armor array, not a claim of valid C array indexing or normal
gameplay reachability.

The following table enumerates **all twenty overlaps** for the established
armor halfword reads +2/+4/+6/+8/+0x0a/+0x0c/+0x0e/+0x10 and all 256
possible byte IDs, excluding the sentinel. `u14+N` means byte offset N
inside `unknown_14`. All intersections are two bytes except `u00`, which
is the low byte of a read also containing the adjacent `charge_rate`.

| Armor ID | Armor read offset → weapon field |
| --- | --- |
| 0 | +2 → row 10 u14+2; +4 → row 10 u14+4; +6 → row 10 u14+6; +0x0e → row 10 u22 |
| 1 | +0x10 → row 11 u14+0 |
| 2 | +2 → row 11 u22; +0x0c → row 12 u00 |
| 3 | +4/+6/+8/+0x0a → row 12 u14+0/+2/+4/+6 |
| 5 | +6 → row 13 u22; +0x10 → row 14 u00 |
| 6 | +8/+0x0a/+0x0c/+0x0e → row 14 u14+0/+2/+4/+6 |
| 7 | +4 → row 15 u00 |
| 8 | +2 → row 15 u14+6; +0x0a → row 15 u22 |

For example, with head-armor ID 0, the calculation at
`0x8001589c..0x800158ac` makes `a0=0x800a00dc`. The `lhu` at
`0x800158b8` reads `0x800a00de`: weapon row 10's `unknown_14+2`.
It is added to the player's cutting defense, not examined as a weapon
property. The other combat offsets feed striking, piercing, poison,
magic and fire defense additions.

`player_set_equipment_slot` also stores these calculated armor pointers.
For the head slot, `0x800168dc..0x80016904` checks ID against 255,
calculates the same address, and stores it at player +0x7c. `player_update`
can then read +0x0e/+0x10 for nonzero/modulo interval checks, followed by
HP increase/decrease. Those accesses additionally require the stored
pointer to agree with the selected ID; the ID check alone does not prove it.

No store through an armor-record pointer was found in these consumers.
The alias cases establish possible interpretations by the armor code under
the stated conditions. They establish no intended meaning for the weapon
fields and do not show that shipped gameplay produces those conditions.

## Resolved static-analysis warnings

The refreshed affine proposal scanned 933 of 935 inventoried GAME function
bodies and reached its fixed point in two rounds. Its 157 access proposals
are not 157 reads of the unknown fields. Raw review resolves the following
specific warnings rather than carrying them as anonymous pointer gaps:

- The proposed weapon +0x22 load at `0x80019b78` is a stale-base false
  positive. `lw v0,player_state+0x68` at `0x80019b70`, its `nop` load-delay
  instruction, then `lhu v1,14(v0)` establish the known MP interval.
- The analogous armor reload warnings are +0x10 interval reads through
  loaded armor pointers. Their conditional overlaps are handled above.
- `main` forms heap base `0x800a0980` using `lui 0x800a; ori 0x980`.
  The intermediate `0x800a0000` lies inside the weapon table, but the actual
  pointer passed to SDK `InitHeap` does not. Its BIOS trampoline is not a
  weapon-pointer escape.
- `effect_magic_power` returns either constant 5 or player magic loaded
  at `0x80037fd4`. The load completes before the return's delay slot ends.
  The proposal's retained `lui 0x800a` is not a returned weapon pointer;
  downstream effect-power narrowing/arithmetic warnings inherit that error.
- The `player_update` warning at `0x800190f8` is signed camera-rotation
  arithmetic from player +0xb8/+0xc8, not arithmetic on a weapon pointer.
- The indirect accessory switch at `0x80015d30` has a static guard:
  unsigned `(accessory_id - 42) <= 10`, followed by eleven words at
  `0x80012000`. Their ordered targets are `80015d88`, five `80015db4`
  entries, then `80015d38`, `80015d4c`, `80015d60`, `80015d74`, `80015d9c`.
  These cases update player stats and never dereference a weapon pointer.
- The equipment-slot switch at `0x80016878` is bounded to six table words
  at `0x80012030`, targeting the six item-ID stores at
  `0x80016880/90/a0/b0/c0/d0`. It adds no unknown-field access.
- The save writer's unresolved destination stores at `0x8002b908/0c`
  belong to the player-state copy. The copied equipment-pointer bits are
  not dereferenced by that copy.

These jump-table resolutions are local raw-evidence findings; this report
does not promote candidate inventory rows or change the CFG tool's output.

## Rust codec accesses

[`WeaponRecord::decode`](../../tools/kf-codec/src/records.rs#L115) reads
byte 0, bytes +0x14..+0x1b and a little-endian halfword at +0x22 after
checking that 44 bytes are available. `encode` writes those same ranges
after its destination-length check. Neither validates or branches on their
values. `load_weapon_records` copies the table, decodes each record, changes
only rotation Y, and re-encodes it; the unknowns pass through unchanged.
Round-trip and preservation tests check bytes, not their game meaning.

## Verdicts and remaining boundaries

Each of the three fields has the same final verdict: **copied and exposed
losslessly; no interpreting read in the checked valid weapon paths;
conditional interpreting reads through misindexed armor paths; purpose
unresolved**. The eight-byte field is not promoted to two integers, and
the zero halfword is not declared padding.

The remaining boundaries are concrete:

1. Save restoration can supply equipment IDs and pointers independently;
   this pass does not prove every reachable save/menu state preserves the
   valid-weapon and valid-armor invariants. Arbitrary restored pointer
   addresses are not exhaustively enumerated.
2. The affine scan excludes fragmented `SquareRoot0`/`InvSquareRoot`
   bodies and bytes outside the function census; arbitrary unrelated
   writes, unresolved value constructions and corruption are not a proved
   whole-executable exclusion. Its generic limitations remain those in the
   [equipment audit](equipment-unknown-field-audit.md).
3. The original authoring tools and format definitions are unavailable in
   this evidence set. Absence of an intended game reader would not by itself
   identify what the resource producer called these bytes.

Verification: hash-identical retail initialization; a fresh error-free
101-variant Clang census; fifteen refreshed six-view function dossiers;
42 raw instruction controls and finite-address controls, including both jump tables;
manual review of the full relevant source bodies and raw accesses. The
selected game functions retain their recorded exact matches except the
existing `player_update` (99.96948%), `effect_update_dispatch`
(99.82781%) and startup `main` (78.52941%); their relevant instructions
were inspected directly. SDK
`RotMatrix` and `InitHeap` remain vendored, with no game-source match claim.
No C, Rust, inventory, or build configuration changes are made by this report.
Local generated evidence is under `build/weapon-field-access/`.
The full `kf build`, `ruff check scripts tests`, all 827 local Python tests
(no skips), report/ledger consistency checks and `git diff --check` pass.
