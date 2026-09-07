# GAME warp mode domain and shimmer identity

## Function Match Plan

Type the shared shimmer mode as `KfWarpShimmerMode`, preserving its full
signed-word public entry and unsigned-word forwarding-wrapper ABI. Keep the
shimmer's signed-halfword local with `KF_ENUM_STORAGE`; the retail body
interprets the low halfword at both mode decisions. Modern compilation should
reject unrelated enums and raw integer arguments without changing that
truncation. The seven calls into the shimmer and two wrapper calls already
use its three named values.

Promote kind 0x15 from the private warp recipe to shared
`KF_EFFECT_KIND_WARP_SHIMMER`, including the effect constructor's case. GAME
`800366c8/800366cc` pass type 0x11 and kind 0x15 separately. Express the stored
type as its existing actor-target and player-magic bits; that encoding does
not imply this scripted visual performs collision or damage. Correct the
identity note that formerly called 0x15 a type.

The constructor's kind-21 arm selects model 13, zero Y scale, Q12 1.5 X/Z
scale and camera yaw. The warp's `sh/lhu` accesses at +0x26 manipulate Y
scale; +0x1e is yaw. The renderer consumes those same fields through the
effect view, passes the scales to `ScaleMatrix` and composes the rotation.
Remove the misleading intensity/phase aliases and name the locals for actual
Y scale. This is geometry, not an independently evidenced brightness channel.

Before edits, collect all six GAME semantic views for the warp module, its
wrapper and callers, the constructor and render consumers. Inspect current
source/history and the [traversal](game-warp-shimmer-traversal.md) and
[position/ABI evidence](game-warp-position-buffers.md). This is authored game
warp/effect policy; vendor audio/GTE calls remain external. Preserve the
exact allocation/traversal order, stack position buffers, initialization,
signed/unsigned scale guard, release stores, delay slots, calls and referents.

Complete the current warp/lifecycle literal ledger. Keep the explicitly
explained 40-world-unit actor movement and 1/64-turn/blend steps, authored
warp coordinates, unresolved floor-five variant-2 identity and initialization
data. Do not invent map/location or sound identities from their ordinals.

Require forced affected builds, isolated compilation of all 112 variants,
all 484 strict scores, raw retail controls for exact functions, focused
Clang rejection controls, whole-tree diagnostics, inventory, Ruff, repository
tests, whitespace and full `kf build`. No size assertions or new tests.

| GAME VA / bytes | Consumer | Strict baseline |
| --- | --- | ---: |
| `0x80014674 / 68` | `player_warp_shimmer_at_player` | 100.000000% |
| `0x800146b8 / 740` | `game_main_loop` | 100.000000% |
| `0x8001499c / 56` | `game_shutdown` | 100.000000% |
| `0x800149d4 / 32` | `frame_pacer_vsync_callback` | 100.000000% |
| `0x800149f4 / 112` | `frame_pacer_wait` | 100.000000% |
| `0x80017cf8 / 324` | `player_warp_to_floor_entry` | 100.000000% |
| `0x8001eedc / 488` | `render_actor_sprite` | 100.000000% |
| `0x8001f218 / 1408` | `render_entities` | 97.380684% |
| `0x80036618 / 568` | `player_warp_shimmer` | 100.000000% |
| `0x80036850 / 348` | `player_warp_change_floor` | 100.000000% |
| `0x800369ac / 324` | `player_warp_same_floor` | 100.000000% |
| `0x80036af0 / 588` | `player_warp_trigger_update` | 100.000000% |
| `0x80036d3c / 244` | `actor_transform_definition5_to6` | 100.000000% |
| `0x80036f44 / 2092` | `effect_pool_construct` | 100.000000% |

## Final evidence and verification

GAME `80036644/80036648` sign-extend the incoming mode's low halfword before
the first dispatch; `800367e4/800367e8` repeat that interpretation before
release. The wrapper forwards the full word. Mode zero grows and removes,
one shrinks and removes, and two grows and keeps the effects. Both entry
points and the local now carry this domain through the existing enum helpers.
The [sibling implementation comparison](game-floor-enum-domain.md) explains
the modern C++20 checking and legacy storage split.

Clang accepted both typed entry points, local storage, assignment and
comparison, and preserved the low-halfword interpretation of an explicitly
decoded `0x10001`. Ten negative controls rejected raw integers and foreign
floor/OPEN-transition enums as arguments, local initializers, assignments and
comparison operands. These temporary compiler controls remain under `build/`;
no permanent tests or size assertions were added.

The constructor's `80037718` store initializes Y scale to zero. The renderer
passes that field to `ScaleMatrix`, so the removed intensity alias had no
evidenced brightness meaning. The animator rotates yaw by 512 angle units
(45 degrees) per update for 48 updates. Its four staggered scale updates start
at frames 1, 9, 17 and 25. The inclusive scale guard runs before adding the
step and can overshoot; it was preserved. Actor transformation retains 40
world units per update and 1/64-turn/blend intervals with both endpoints,
giving 65 updates per phase. The original tuning rationale remains unknown.

Every function in the plan retains its listed strict result: thirteen exact,
one partial. Across the fourteen controls, all 1,850 candidate words, 128 calls
and 114 address references are unchanged; the thirteen exact functions also
agree with 1,496 retail instruction words and their delinked targets. The
existing partial renderer first differs at GAME `8001f274`: candidate
`move s5,v1`, retail `move s6,v1`. This remains an unattributed residue.
The transform color matrix and two initialized VBlank counters also match
all 40 claimed retail bytes, including matrix padding and zero translations.

Isolated before/after compilation of all 112 variants preserved allocated
bytes, alignment, runtime symbols and ordered relocations. Only the warp
object's debug line table changed. At campaign verification all 484 strict
scores and live objects agreed with the controls. Subsequent concurrent
`map_load.c` work changed that unrelated live object; a final campaign check
confirmed all reviewed objects still equal the verified isolated output.

Whole-tree modern checking retains the same 300 diagnostics, with 65/112
variants passing. Inventory, Ruff, whitespace and all 683 repository tests
pass (89.121 seconds). Full `kf build` retains the existing source-data
mismatches (PSX 0/1, GAME 9/42, OPEN 2/19 matching units) and target-relink
conflicts (PSX 1/1, GAME 75/77, OPEN 34/38 verified), with no artifact failures.
No new exact functions were claimed or banked.

The [complete warp/lifecycle ledger](game-warp-literal-ledger.md) accounts for
108 retained occurrences in three files, including a zero-literal wrapper.
Naming the constructor case removes one occurrence from the effect-pool
ledger. The current source census is 6,692 occurrences; 35 files containing
3,513 occurrences have fully reconciled ledgers. The ten source lines with
fourteen `unknown_` tokens remain unresolved. These accounting results do not
declare the broader semantic naming work complete.
