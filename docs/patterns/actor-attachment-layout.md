# Actor attachment and special-attack layout

GAME actor definitions contain three six-byte attachment triples at offsets
0x28, 0x2e, and 0x34. The third triple's x/y lanes also supply signed
special-attack chance and range. `KfActorDefinition` models one array of three
`KfVec3s` entries; named readers express the two special-attack meanings.
Every supported slot has a declared element, and the shared bytes have one
storage owner. Original declaration/API spelling remains unknown.

## Retail evidence

`actor_update_current_action` (GAME `8002fa88`) passes slot 0, 1, or 2 to
`actor_update_effect_action` at `8003039c`, `800303ac`, and `800303bc`.
The latter retains the incoming word, adds eight for the animation slot,
and forwards the original slot to `actor_spawn_action_effect` at `8002f4e8`.

In `actor_spawn_action_effect` (GAME `8002edd4`), `8002ee9c..8002eea8`
computes `definition + 6 * slot`. Three halfword loads at `8002eeac`,
`8002eeb8`, and `8002eec4` select displacements 0x28, 0x2a, and 0x2c.
Slot 2 therefore reads the complete triple at 0x34, 0x36, and 0x38.
The values are stored in an SDK `SVECTOR` and rotated before emission.
The full call set, delay slots, and ordered references are unchanged.

The special-attack consumers read the same storage:

| Site | Retail access | Meaning |
| --- | --- | --- |
| `8002e3b0` | `lh a2,52(s0)` | Jump-attack selection chance |
| `8002e3e0` | `lh a2,52(s0)` | Special-attack selection chance |
| `8002e3e4` | `lh a3,54(s0)` | Special-attack selection range |
| `80030108` | `lh a1,54(s1)` | Jump-attack contact range |
| `800301ec` | `lh a1,54(s1)` | Special-attack contact range |

The named readers return `s16` and preserve these sign extensions.
The later animation-step/phase arrays still begin at 0x3a/0x5a; the complete
record is 0x98 bytes and the loaded twelve-record table is 0x720 bytes.
`actor_definitions_load` copies 456 words, including the entire third triple.

## Shipped resource control

The first twelve definition records in chunk 6 of each `KF/B1..B5/MIXA.DAT`
were inspected. Only floor 5 definition 7 has a third effect code other than
zero or 0xff: code 56, paired kind 24, with EFFECT2 animation 3. Its third
triple is `(0, -1000, -2200)`. Kind 24's retail dispatch row at `80012520`
contains `8002ee94`, the block containing the indexed coordinate loads.
The corpus supports a consuming third-slot layout, including the former
opaque z bytes `68 f7`. It does not establish exhaustive runtime-state or
indirect-path coverage.

## Source and compiler controls

The earlier two-element array ended immediately before separately named
chance/range/opaque fields. Index 2 exceeded that declared array. Growing
the array while retaining those fields would move later fields; the current
model instead gives the shared bytes one owner. Ordinary inline readers
name their special-attack interpretation and access the third element's
x/y lanes. No raw-offset accessor, union pun, artificial padding, or changed
compiler profile is required.

Forced `gcc257-native` compilation of `game.actor_behavior` and
`game.actor_pool` produces byte-identical complete native objects to the
captured `19df9a3e` baseline. The textual `kf try` preserves the changed
consumer instructions; its other differences are the already-existing
native section-relative relocation presentation. Strict results come from
refreshed objdiff reports, independently of that listing comparison.

The modern C++ contract exercises all three slots and the two signed readers.
The pinned Psy-Q fixture checks the array's 18-byte extent, all third-lane
offsets, later arrays, and complete record/table sizes, alongside its wrong
owner-size negative control. The local resource control checks the shipped
third-slot record and its exact retail table word. The existing record-loader
oracle compares complete copied table bytes across retail, C, and Rust.

## Verification verdict

All eleven functions in `game.actor_behavior` and all three in
`game.actor_pool` remain strict objdiff 100% after rebuilding and refreshing
analysis. This includes both changed special-attack consumers and the
indexed attachment consumer. Across the repository, all 97 native objects
are byte-identical to the captured baseline; the strict total remains
465/471. No function was newly banked.

The full local suite passes 885 tests with no skips. The pinned type check
passes all 97 image variants with zero enum-domain literals; Ruff, the full
three-image build, and diff whitespace checks pass. Existing data and
section-placement analysis failures remain unchanged.

`nix flake check -L` passes all checks. Its isolated suite runs 885 tests,
with 147 expected skips; the modern C++ contract runs and passes there.
The local-resource test runs and passes in the full local suite.

| GAME function | Address | Final verdict |
| --- | --- | --- |
| `actor_select_next_action` | `8002e2e8` | 100%, shared signed readers |
| `actor_update_awareness` | `8002e6a8` | 100%, unchanged |
| `actor_move_xz_with_collision` | `8002e954` | 100%, unchanged |
| `actor_move_along_heading` | `8002ed00` | 100%, unchanged |
| `actor_spawn_action_effect` | `8002edd4` | 100%, bounded three-slot owner |
| `actor_prepare_charge_toward_player` | `8002f228` | 100%, unchanged |
| `actor_apply_horizontal_movement` | `8002f31c` | 100%, unchanged |
| `actor_update_effect_action` | `8002f468` | 100%, unchanged |
| `actor_apply_random_movement` | `8002f558` | 100%, unchanged |
| `actor_update_boss_death_sequence` | `8002f8cc` | 100%, unchanged |
| `actor_update_current_action` | `8002fa88` | 100%, shared signed readers |
| `actor_pool_update` | `80030818` | 100%, unchanged |
| `actor_pool_load_placements` | `800308c0` | 100%, unchanged |
| `actor_definitions_load` | `80030a6c` | 100%, complete record/table layout retained |
