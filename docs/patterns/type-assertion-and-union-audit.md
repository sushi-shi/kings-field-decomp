# Typedef assertions and union source models

Audit of the 2026-09-09 working source, including the item-detail workspace
introduced in `223b6f83`. This investigation changes documentation only.
Historical matching dossiers supply hypotheses, not proof of original C
declarations. Representative retail instructions were rechecked after
`kf init` verified all three images.

The subsequent implementation and final checks are in
[type-cleanup.md](type-cleanup.md); the counts below describe the audited
baseline before that cleanup.

## Recommendation

Remove layout-assertion declarations from reconstructed C and headers in a
separate cleanup. Preserve useful target-compiler layout validation in test
fixtures and the curated inventory. Typedef-array syntax has a purpose in
those fixtures: it implements a compile-time check accepted by the old C
compiler. It has no demonstrated role in the reconstructed program's output.

Review the unions individually. Most are representation views introduced by
our reconstruction; the evidence does not establish original union types.
A word copy, a different load width, fewer written casts, an editor type check,
or a shared machine register does not by itself establish a source union.
Some records really do have behavior-dependent payloads, and several packed
accesses genuinely overlap. Preserve those facts while reassessing syntax.

## Assertion evidence

The source/header census finds **77 `typedef char` declaration spelling
sites**, including assertion macro definitions. This is not an expanded
assertion count. It excludes `tests/fixtures` and the separate modern
`static_assert` declarations in `game_actor.h`.

An isolated compiler comparison preprocessed all **112 C build variants**
with their manifest compiler, flags and defines. For each variant it compiled
the same preprocessed input path twice, replacing only unused typedef-array
assertion declarations with whitespace in the second input. Newlines were
preserved. It checked that the removed typedef names had no remaining uses.
**All 112 complete assembly outputs were byte-identical**; 105 variants
contained assertions. This is compiler-output equivalence for these inputs,
not a new retail-match or historical-compiler claim. No working source or
baseline was changed. Generated comparisons are under
`build/type-model-audit/` and are not committed.

There is useful validation to preserve outside the game source:

- `kf inventory check` passes with **130 types and 868 fields**. It compares
  complete sizes and field extents, offsets, names and types against the TSVs.
- This inventory is a Python layout calculation, not a compiler query.
  `scripts/kf/inventory.py::_header_structure_layouts` hard-codes SDK layouts,
  scans selected headers and excludes inline anonymous aggregates. It does
  not cover local C types or `gpu_packets.h`'s nested packet views.
- Existing compiler fixtures cover important gaps. The GAME runtime-layout
  and OPEN packed-packet controls both pass, including their deliberately
  incorrect size/offset cases. Those controls should remain when production
  assertions are removed. Other uncovered assertions need a coverage decision
  before deletion, rather than assuming the inventory already checks them.

Some current assertions are weaker than they appear. A scratch-only negative
control removed `KfMagicRecord.unknown_12`, reducing a record from 20 to 18
bytes. The existing `sizeof(KfMagicTable) == 0x1e0` assertion still compiled:
the union's `words[120]` member kept its total extent at 480 bytes while its
record-array member shrank to 432. The compiler emitted `18` and `480` for
the two size probes. Thus a correct union extent can conceal an incorrect
record stride. Inventory/field checks supply a different and necessary check.

The prior removal policy is also explicit in history:

- `f2504865` removed compile-time layout assertions and added a cleanliness
  check against their reintroduction.
- `f4c7b233` removed that cleanliness check during modern type-checking work.
- `2489c9a8` removed size assertions again. The
  [modern-type-checking note](modern-type-checking.md) records the user's
  direction and warns against reintroducing them from historical dossiers.
- Later type campaigns added assertions again; for example `b6a4e7ca`
  introduced the resource-pointer union and its size assertion together.

The opening assertion-policy paragraph and counts in
[`docs/structure-layouts.md`](../structure-layouts.md) are stale. The current
AGENTS instruction to retain static layout checks can be satisfied by target
compiler fixtures; it does not require those declarations inside game code.

## Complete union census

There are **43 union declarations: 37 in project headers and six in C files**.
The categories below are review priorities, not recovered original types.
Every declaration is accounted for once.

### Source-model review first: eight declarations

| Union / source | Finding and next source hypothesis |
| --- | --- |
| `KfResourcePointer`, [`resources.h`](../../include/kf/resources.h) | `void *`, byte cursor and SDK TIM pointer views were introduced explicitly to accommodate the editor's C++ checks. Review the allocator output and typed consumer conversions. Pointer representation equality does not establish a union owner. |
| `KfNotificationDigitBuffer`, [`notify.h`](../../include/kf/notify.h) | Signed formatter output and unsigned digit reads are one halfword buffer. Test one storage declaration with numeric conversion at the consumer; unsigned loads alone do not establish two array members. Preserve the formatter's sentinel and all widths. |
| `KfRotation`, [`game_math.h`](../../include/kf/game_math.h) | SDK `SVECTOR` and a six-byte Euler prefix provide two interfaces to the same rotation. Review the storage/API boundary. Real six-byte Euler objects elsewhere prevent blindly widening every helper argument to an eight-byte object. |
| `KfPlayerMotionState`, [`game_player.h`](../../include/kf/game_player.h) | The motion aggregate was extended through the current cell so `words[2]` could expose an aligned pitch/cell load. That access does not establish the new aggregate boundary. Review the packed access without making the cell a motion field solely for this view. |
| `MenuGlyphWorkspace`, [`menu_item_detail.c`](../../src/game/menu_item_detail.c) | Adds a complete halfword-array view so a cursor starts at the workspace base and stores glyphs at offset four. Retail proves that addressing; it does not prove the union. Review the shared glyph-buffer/API model. The union improved a partial match but did not close it. |
| Local `status`, [`menu_item_drop.c`](../../src/game/menu_item_drop.c) | Save-operation and cleanup results were united after a direct condition changed generated code. A reused return-value register does not establish an original union. Review the historical integer result local and the modern enum boundary. |
| Local `selection`, [`GAME render_map_cells.c`](../../src/game/render_map_cells.c) | Attribute-to-index arithmetic on one byte was wrapped in a union after separate enum/index locals changed register use. Preserve the arithmetic and byte truncation while reviewing the enum-to-index boundary. |
| Local `selection`, [`OPEN render_map_cells.c`](../../src/open/render_map_cells.c) | Same issue in the independent OPEN implementation. |

The [resource-boundary](typed-resource-boundaries.md),
[save-result](save-result-domains.md),
[enum-field](enum-field-review.md),
[packed-state](typed-packed-state.md), and
[item-detail](game-low-trial-campaign.md) dossiers explicitly record these
motivations. Their unchanged or improved bytes demonstrate compatibility;
they do not promote the added unions to original source structure.

### Bulk-copy views: eleven declarations

| Unions | Finding |
| --- | --- |
| `KfWeaponTable`, `KfArmorTable`, `KfMagicTable`, `KfActorDefinitionTable`, `KfMapObjectDefinitionTable` (five) | Typed record arrays plus full-table word arrays. Review ordinary typed arrays and explicit copy boundaries. A whole-table copy is not a runtime variant. Preserve independently established storage alignment and array stride. |
| `KfMapGrid`, `KfMapAttributeGrid`, `KfMapCollisionGrid`, `KfMapOrientationGrid` (four) | Row/column, linear and word-copy views of each grid. The dimensions and widths are real; three union members are not established. Review one canonical grid representation plus its indexing/copy interfaces. |
| `KfMapSavedWorld` | Floor records plus byte/word serialization views. Separate the complete saved owner from its copy access mechanism. Floor-specific payload interpretations are covered by `KfMapFloorScript` below. |
| `KfPackedSVector` | SDK vertices plus two words used by copying and scratch save/restore. Preserve the eight-byte object and required alignment; review whether the SDK type and an explicit representation boundary suffice. |

GAME `weapon_records_load_and_mirror_angles` at `0x800150a8` is a concrete
control: the first loop copies `0xb0` words using `lw/sw`; the second visits
16 records with stride `0x2c` and modifies their yaw halfwords. These operations
establish the copy extent and record layout, not a table-union declaration.

### Packed access views: twelve declarations

| Unions | Finding |
| --- | --- |
| `KfGpuF3`, `KfGpuF4`, `KfGpuFT3`, `KfGpuFT4`, `KfGpuG3`, `KfGpuG4`, `KfGpuGT3`, `KfGpuGT4` (eight), [`gpu_packets.h`](../../include/kf/gpu_packets.h) | The mixed word XY, halfword UV and byte color operations are supported. The parallel `sdk`/`packed` structures are reconstruction wrappers around authentic SDK packet types. Review SDK-typed storage and evidenced packed accessors; do not assume the corresponding SDK macros emit the required operations. |
| `KfTmdPacketHeader` | Serialized four-byte header viewed as a word and its input-length byte. Packed representation access is real; the union syntax is open. |
| `KfScreenXY` | GTE packed screen word and signed `DVECTOR` coordinates. Preserve both access widths and the real SDK boundary. |
| `KfMapCell` | Byte z/x coordinates and halfword comparisons. Preserve packed comparison semantics when reviewing the representation. |
| `KfDialogueState` | Four byte fields plus a masked word predicate. Preserve the word load and mask; replacing it with separate byte conditions is a different code-generation hypothesis. |

GAME `map_action_script_floor2` at `0x800345bc` loads the dialogue word at
`0x800345d0` and applies `0xffffff00`. GAME `player_warp_trigger_update`
contains five aligned `player_state+0xc8` word loads, the first at
`0x80036b3c`. These are real overlapping-width accesses. Their existence
does not identify whether the source used casts, macros, or union members.
The supplied Release 2.5 `LIBGPU.H` declares the original packet structs;
the `KfGpu*` wrappers are project additions.

### Variant payloads and deliberate overlap: twelve declarations

| Unions | Finding |
| --- | --- |
| `KfEffectDirection`, `KfEffectVisualState`, `KfEffectControl`, `KfEffectPropagation`, `KfEffectRenderId` (five) | Effect kind or animation tag selects different interpretations of stored bytes: direction/state, phase/scale, countdown/orbit/parent/homing, generations/branch role, and billboard/model index. Stronger reasons for a variant model. Same-width naming alternatives could still have been generic scalar fields in the original. |
| `KfMapFloorScript` | Floor identity selects different script payload fields inside each saved floor. A supported variant model; fixed serialized extent remains independent evidence. |
| `KfMapObjectSpawn`, `KfMapObjectParameter`, `KfMapObjectLink` (three) | Behavior selects sequence/effect ID, parameter domain, or gold/container/link payload. Keep the behavior-dependent facts. `KfMapObjectLink`'s byte/word copy members can be reviewed separately from its actual payload alternatives. |
| `KfTmdPrimitive` | Packet mode selects different body formats. Its variant members have a sound format basis; the additional color/texture views are representation conveniences. A packet uses its selected format's extent, not necessarily the maximum union size. |
| `KfMapGpuPrimitive` | OPEN allocates GT3 or GT4 according to mode. Those formats are real, but a common union pointer remains optional; each case allocates its actual SDK size. |
| `KfMorphPrefix` | A genuinely unusual overlap: the rest-morph path passes its range header to the SDK as the first vector, includes one extra vector, and restores the affected scratch entry afterward. Preserve that behavior even if an explicit format/API conversion eventually replaces the union. |

For the morph overlap, GAME `render_bind_animated_instance` sets the source
argument to `object+4` at `0x80020904`, calls the independently vendored
`gteMIMefunc` at `0x80020924`, and increments the count in that call's delay
slot. The two scratch words are restored at `0x80020930` and `0x80020938`.
This is stronger overlap evidence than simple register reuse. Conversely,
`menu_save_panel`'s `move s4,v0` at `0x80025204` proves only that the cleanup
result is saved for comparison, not a C union declaration.

## Cleanup order and limits

First remove production assertion declarations with a fixture-coverage review.
Then address the eight source-model candidates above and the bulk-copy
wrappers in coherent caller/callee families. Keep the packed and variant
families for focused reviews that preserve their actual loads, stores, SDK
types, serialization extents and control flow.

Explicit conversions at real representation/API boundaries are acceptable.
Do not optimize a written-cast count by adding alternate union members, and
do not sacrifice the independent enum-domain checks merely to simplify a
local declaration. Source hypotheses must explain both interfaces. A lower
intermediate score can inform the investigation; production replacements
still need focused strict matches, raw referent checks and a full build,
with no regression of banked functions.

This audit does not prove that any particular original author did or did not
write a union. It establishes that the existing justifications are often
insufficient, identifies stronger overlap evidence, and supplies a concrete
compiler result for removing typedef assertions. No functions were edited,
matched anew, banked or committed, and no full-build result is claimed.
