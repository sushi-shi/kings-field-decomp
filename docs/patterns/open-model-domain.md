# OPEN model enum domain

## Function Match Plan

Give the opening entity's byte selector and lookup key a separate
`KfOpeningModelId` domain. Keep the renderer's halfword local with
`KF_ENUM_STORAGE`, explicitly encode only at generic TMD APIs, and decode
the shared placement format at the OPEN loader. GAME and OPEN use different
model tables despite sharing the serialized placement layout.

The finder at OPEN `800199e0` uses `lbu`, masks its second argument to 255,
walks 40-byte entities and returns a pointer or null. The loader reads and
stores the selector with `lbu`/`sb`. The renderer retains its `andi 0xffff`
before generic TMD calls. Both sweep callers compare the byte against 32;
this is the retail render bound, not the resource's 28-model count. Preserve
255 as the inactive selector, all comparisons, load/branch delay slots,
ordered calls, switch rows and relocation targets.

Before edits, capture all six semantic views for the eleven consumers below,
inspect source/history and the adjacent scene/resource/render evidence in
[the renderer campaign](open-render-constants.md). These are authored game
scene operations and resource consumers, not SDK bodies. SDK transform and
draw helpers remain external. Direct calls are proven; the renderer's
indirect jump remains unresolved by the navigator. Its 17 pointer rows at
OPEN `80012238` retain the separately decoded switch ownership.

The first hypothesis is identical runtime objects and stronger compiler
domain checking. Require forced affected compiles, isolated before/after
compilation of all 112 variants, all 484 strict scores unchanged, exact
retail-word controls, focused compiler rejection controls, modern diagnostics,
inventory, Ruff, repository tests, whitespace and full `kf build`.

## Resource evidence

Hash-verified `KF/B0/MIXB0.`, `MIXB3.` and `MIXBE.` begin with identical
200764-byte TMD chunks, header `(65, 0, 28)`, SHA-256
`c43643f2253d087b4d292d690abe054d5e71515fa4d8d267188b16db29f9ceb3`.
The scene loaders register these as entity models. Static inspection decodes
the standard TMD object records, vertices and polygon packets; no game code
is executed. Orthographic views establish geometry, not in-game lighting or
the identity of a fictional location. TIM uploads can differ between scenes;
the new color names below come from untextured polygon colors.

| ID | Vertices / primitives | Supported identity and consumer |
| ---: | ---: | --- |
| 11 / 12 | 110 / 100 each | Paired scene-0 rotating leaves; retain the established decreasing/increasing yaw role names. |
| 13 / 14 | 247 / 456; 254 / 456 | Paired scene-3 door leaves, reused by the ending; retain established motion names. |
| 19 | 144 / 240 | Yellow cylinder scaled vertically and rotated by `opening_entity_transition`. |
| 20 | 64 / 112 | Tapered column with broad end caps; shares the door depth bias. |
| 21 / 23 | 7 / 16 each | Green pointed crystals; renderer increases/decreases yaw respectively. |
| 22 / 24 | 7 / 16 each | Pink pointed crystals; renderer increases/decreases yaw respectively. |
| 25 | 430 / 716 | Mountain-and-castle silhouette backdrop; sole perspective-right projection case. |
| 26 | 32 / 30 | Orange disk translated vertically during the ending, then hidden. |
| 27 | 81 / 128 | Starfield rotated around Z after the disk is hidden. |

IDs 21–24 update yaw after composing the transform, affecting a later render.
The crystal direction names describe that existing behavior. No elemental,
character or location lore is inferred from color or geometry.

## Consumer baseline

| OPEN VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `80014268 / 372` | `opening_scene0_run` | 100% |
| `80014608 / 508` | `opening_entity_transition` | 99.921260% |
| `80014804 / 816` | `opening_scene3_run` | 99.931370% |
| `80014b34 / 756` | `opening_ending_scene_run` | 100% |
| `80014e28 / 1944` | `opening_ending_scroll_run` | 97.129630% |
| `80018ecc / 552` | `opening_entity_render` | 100% |
| `80019240 / 664` | `opening_render_entities_and_items` | 96.759030% |
| `80019520 / 120` | `opening_render_entities` | 100% |
| `80019994 / 76` | `opening_entity_pool_reset` | 100% |
| `800199e0 / 68` | `opening_entity_find_by_object_id` | 100% |
| `80019a24 / 384` | `opening_entity_pool_load_placements` | 100% |

## Final implementation and verification

The field, lookup key and renderer local now carry `KfOpeningModelId`.
The renderer retains its unsigned-halfword storage through `KF_ENUM_STORAGE`.
The loader decodes the shared placement byte, while generic TMD calls encode
the selector explicitly. The render bound and inactive selector belong to the
enum; pool capacity remains a count. All seven newly shared geometry names and
the refined disk/starfield identities retain their original numeric IDs.

All eleven consumers retain the scores in the baseline table: seven exact,
four partial, no new result banked. Their 1,566 instruction words, 128 calls
and 114 resolved address materializations are unchanged. The seven exact
consumers' 582 words also equal linked retail and target-object words.
All 112 isolated source/image variants retain identical allocated contents,
alignment, runtime symbols and ordered relocations; live objects equal the
isolated after objects. Only the scene module's debug line table changes.
All 484 project strict scores are unchanged.

| Partial consumer | First unchanged retail divergence |
| --- | --- |
| `opening_entity_transition` | OPEN `80014608`: reconstructed stack allocation 0x30, retail 0x38. |
| `opening_scene3_run` | OPEN `80014804`: reconstructed stack allocation 0x60, retail 0x70. |
| `opening_ending_scroll_run` | OPEN `800150c0`: reconstructed `sh zero,0xb0(sp)`, retail `lui a0,0x8003`. |
| `opening_render_entities_and_items` | OPEN `80019370`: reconstructed `lui a0,0x8007`, retail `lui at,0x8007`. |

These are preserved residues, not new compiler-attribution claims.

The configured Clang C++20/MIPS flags and actual headers accept a positive
control covering fields, halfword storage, lookup, comparison and explicit
encoding/decoding. Ten independent negative controls reject raw/foreign
assignments, lookup arguments, local initialization and comparisons, implicit
integer encoding, and decoding a foreign enum as if it were serialized data.
The existing offset-check extension warning is suppressed for these controls;
no size assertions or permanent tests are added. Whole-tree modern checking
retains exactly the same 300 errors and 65/112 passing variants.

Inventory, Ruff, all 683 repository tests (86.596 seconds), and whitespace
checks pass. Full `kf build` retains the existing incomplete data/relink
results: source data PSX 0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1,
GAME 75/77, OPEN 34/38. There are six section-base conflicts and no artifact
failures; the full build is not green.

Six inline renderer IDs become named enum cases. The source census falls
from 6,792 to 6,786 retained occurrences. The updated renderer ledger and new
[entity-pool ledger](open-entity-pool-literal-ledger.md) bring verified current
accounting to 29 files / 2,491 occurrences. The three reset-only OPEN control
fields remain unresolved; initialization does not establish their meanings.
