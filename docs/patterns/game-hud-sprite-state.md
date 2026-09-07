# HUD and effect-sprite state domains

## Function Match Plan

At `2b86c6b`, give the byte state fields of `KfHudSprite` and `KfEffectSprite`
separate enum domains. Reuse the existing HUD HIDDEN=0, VISIBLE=1 and END=255
names. Give the effect table its own HIDDEN=0, ACTIVE=1 and END=255 values.
The authored end row uses 255, but effect traversal stops at every non-one
state, including hidden zero. HUD traversal skips non-visible rows until 255.
Keep these distinct algorithms and the arbitrary byte values copied from the
player's compass option. Explicitly encode/decode at that domain boundary.

Name effect slot zero COMPASS and the table's two-row bound. GAME
`80020110/80020118` copy the same loaded compass byte to the HUD compass row
and first effect record. GAME `80020124/80020128` stores negative view yaw
masked to twelve bits into that effect row's rotation Z. These consumers
establish compass identity independently of row position. Keep every other
initializer value and field opaque where its meaning is still unresolved.

The complete six-view GAME evidence, current scores/objects, history and
source snapshots precede edits. The HUD loop loads a byte at `8001f9ec`,
checks 255 at `8001f9f4`, draws only exact one at `8001fa08`, passes payload
+2 in the call delay slot and advances fourteen bytes at `8001fa18`.
The effect loop loads a byte at `8001f8ec`, rejects non-one at `8001f8f4`,
advances twenty-eight bytes and repeats only for one at `8001f9ac`. Both paths
restore their original frame/matrix state. The frame producer, adjacent
weapon renderer and common sprite/animation callees are captured controls.
Player HUD/compass resources establish game ownership; GTE/display services
remain external library calls. No vendored body is reconstructed.

Verify the entire 196-byte HUD table, 56-byte effect table and both palette
rectangles against retail, including the HUD end-row fill, effect end-row
zero fill, null animation slots and unresolved bytes. Preserve the existing
byte layout and source data ownership. Update the two curated state types
and their existing inventory-test expectations; add no tests or size assertions.
Force affected builds, compare raw words/calls/referents and exact controls,
compile all 112 variants in isolated before/after trees, and run modern enum
controls, inventory, Ruff, all repository tests, whitespace and full `kf build`.
Refresh the complete four-file rendering ledger and source counts.

## Per-function snapshots

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `0x80018880 / 6684` | `player_update` | 96.945540% |
| `0x8001e480 / 364` | `render_screen_sprite` | 100.000000% |
| `0x8001f798 / 280` | `render_weapon` | 100.000000% |
| `0x8001f8b0 / 292` | `render_effect_sprites` | 100.000000% |
| `0x8001f9d4 / 112` | `render_hud_gauges` | 100.000000% |
| `0x8001fde4 / 1304` | `render_frame` | 100.000000% |
| `0x800205d4 / 932` | `render_bind_animated_instance` | 100.000000% |

## Final implementation and verification

Both state fields now use their own byte-backed enum. The compass option is
explicitly encoded and decoded into each domain, preserving arbitrary byte
values rather than normalizing them to a Boolean. The effect table's compass
slot and two-row bound are named. Its traversal still stops on every non-active
value, while the HUD traversal skips hidden entries until its end marker.
The two curated field types and existing inventory expectations agree.

This follows the local HoMM2/Gruntz split between strict C++20 enum domains and
retail integer storage, reviewed in [the floor-domain campaign](game-floor-enum-domain.md).
The modern compiler accepts valid field/local flows and explicit encoding,
and rejects all eleven raw-integer, foreign-domain and implicit-encoding
controls. Explicit decode preserves 255 and byte narrowing of 257 to one;
effect state two remains non-active. No size assertions were introduced.

All 112 independently compiled before/after variants retain identical sections,
runtime symbols and ordered relocations. Live objects agree with the isolated
result, and all 484 strict scores are unchanged. The seven function controls
preserve 2,479 candidate instruction words, 123 ordered direct calls and 287
address materializations. The six exact functions preserve 821 complete retail
instruction words, including delay slots.

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `player_update` | 96.945540% | Partial, unchanged |
| `render_screen_sprite` | 100.000000% | Exact, unchanged |
| `render_weapon` | 100.000000% | Exact, unchanged |
| `render_effect_sprites` | 100.000000% | Exact, unchanged |
| `render_hud_gauges` | 100.000000% | Exact, unchanged |
| `render_frame` | 100.000000% | Exact, unchanged |
| `render_bind_animated_instance` | 100.000000% | Exact, unchanged |

The existing `player_update` divergence begins at GAME `80018880`: candidate
frame allocation is 216 bytes, retail is 224. This remains an unattributed
residue; no partial function was promoted or banked.

All 268 bytes across the HUD table, effect table and two palette rectangles
match retail in the source initializer reconstruction, candidate ELF and
delinked ELF, including unresolved bytes and end-row fill. Six inline literal
occurrences were replaced. The refreshed four-file rendering ledger covers
216 retained occurrences; the source census is 6,611, with 4,034 occurrences
documented across 41 verified files. The unresolved source-name count remains
10 lines containing 14 tokens.

Inventory, Ruff, whitespace checks and all 683 repository tests pass
(87.353 seconds). The whole-tree modern check retains the same 300 errors:
65 of 112 variants pass and 47 fail. Full `kf build` still reports existing
source-data and target-relink mismatches, including six conflicting section
bases, with zero artifact failures.
