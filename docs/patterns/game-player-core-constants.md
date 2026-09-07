# Player core constants and remainder ledger

## Function Match Plan

At `c6ad920`, audit all fourteen GAME.EXE functions and both data claims in
`src/game/player_core.c`, then propagate shared constants through four charge,
rendering and collision consumers. The selected probe is `probe-gcc257-o2-g0`;
it is not proof of historical compiler attribution. Each source function keeps
its signature, storage widths, ordered calls, delay slots and referents.

| GAME VA / retail bytes | Function | Strict % before/after | Snapshot / intended change |
| --- | --- | --- | --- |
| `80016848 / 488` | `player_set_equipment_slot` | 100.000000 | Typed slot switch, byte item IDs and five null record pointers; ledger only, retain all item/armor mappings. |
| `80016a30 / 244` | `player_equip_weapon` | 100.000000 | Byte charge delay 10, signed phase -1; keep decimal path digits, load/error calls and cache clearing. |
| `80016b24 / 156` | `player_begin_weapon_attack` | 100.000000 | Signed phase guard, unsigned charge equality 5000, full-volume sound call slot; preserve committed-charge copy before clearing current charge. |
| `80016bc0 / 612` | `player_update_weapon_attack` | 100.000000 | Signed phase +=300 and end 4096; unsigned-halfword windows at 1000/3072 of width 300. Keep five damage channels, committed charge and exact credit nibble 0x10. |
| `80016e24 / 148` | `game_initialize_session` | 100.000000 | Ledger only: initial position31000/0/4000, zero Euler angles and enabled display/audio flags. Restart uses a distinct Z5000. |
| `80016eb8 / 48` | `player_clear_motion` | 100.000000 | Ledger only: five signed/unsigned motion halfwords set to zero; no typed state or callback change. |
| `80016ee8 / 344` | `player_sync_position_to_map` | 100.000000 | Retain the completed camera/vertical-state audit; refresh ledger locations after module constants are inserted. |
| `80017040 / 200` | `player_distance_to_point_in_cone` | 100.000000 | Signed angle tolerance, twelve-bit wrap, half-turn fold and height-ignore sentinel; preserve both calls and -1 rejection. |
| `80017108 / 244` | `player_distance_to_point` | 100.000000 | Three arithmetic eighth-scale shifts, height-ignore sentinel and two850 half-height constants; keep signed bounds, half-height shift and SDK square root. |
| `800171fc / 2088` | `player_move_horizontal` | 93.538315 | Named cell orientations, blocked result, Q12 shifts, flags 0x881, radius 800/height 1700, deflection half-turn +/-64, slide radius 900, rise limit699 and diagonal factor2896; retain all narrowing and branch order. |
| `80017a24 / 92` | `player_update_view_bob` | 100.000000 | Already named; verify unchanged body and zero remaining numeric literals. |
| `80017a80 / 632` | `player_update_vertical_motion` | 100.000000 | Already typed; retain seven-literal sync/bob/vertical ledger scope, refresh only source locations. |
| `80017cf8 / 324` | `player_warp_to_floor_entry` | 100.000000 | Five packed two-byte entry cells indexed by one-based floor; name shared table bound and retain coordinates/floor-five variant behavior. |
| `80017e3c / 160` | `player_update_transform_snapshot` | 100.000000 | Ledger completeness and unchanged source/body control; no numeric literals. |
| `80018880 / 6684` | `player_update` | 96.945540 | Propagate idle phase, charge cap/gain factor and swing phase step into magic-window bookkeeping; name its early/late activation bounds. Movement headings use shared angle constants. |
| `8001f798 / 280` | `render_weapon` | 100.000000 | Share signed inactive phase; preserve the u16 animation-phase API boundary, weapon record accesses and rendering calls. |
| `8001fde4 / 1304` | `render_frame` | 100.000000 | Two charge values /100 yield 50-pixel gauges at charge 5000. Name gauge width and charge conversion, preserving the exact HP/MP rounding expressions. |
| `8002d6a0 / 344` | `actor_try_attack_player` | 100.000000 | Share the player collision height 1700 in the seventh argument; camera-height correction and all combat arguments stay unchanged. |

All six image-qualified semantic views, full retail disassembly/CFG, ordered
references, caller/callee lists, source histories, objects and scores were
captured before edits in ignored `build/constant-names/player-core-constants/`.
Adjacent functions are included by the complete module scope. Additional
evidence covers actor damage/overlap, world collision, angle comparison, the
charge helper, animation binding, asset selection, warping, map resource loading
and SDK sine/cosine. SDK bodies remain library evidence, not reconstructed game
progress. These routines implement player/game policy and are not vendored
library implementations.

Publish a complete per-occurrence remainder ledger for the module. Existing
motion-ledger entries remain current. Compare every reviewed body to its
baseline; require identical runtime/data sections for all 112 units, all exact
retail words and unchanged non-exact results. Run modern checking, inventory,
Ruff, existing tests and full build before committing. No new size assertions
or permanent tests are part of this campaign.

## Charge and weapon phase

Weapon and magic charge are separate u16 counters with the same 5000 full
value and twice the `fixed6_ratio_step` gain. That helper returns
`(value << 6) / (span + 1) + 1`; the caller doubles the complete returned step.
Full weapon charge is copied into the committed damage scale; actor damage divides that scale by 5000. The player
damage Q12 input is a different contract. Do not rename unrelated effect, fog
or coordinate 5000 literals as charge values. Preserve halfword narrowing before
cap comparisons; `>=5001` remains `>=full+1` and magic retains its `>full` tests.

The signed weapon phase starts at 0, advances 300 per attack update, and becomes
inactive (-1) at or beyond 4096. Rendering and menu entry check the same inactive value.
From phase zero, the swing ends on update fourteen; the Colichemarde hit
occurs at phase 1200 (update four), and the other hit at 3300 (update eleven).
The phase is a continuous counter, so its field remains s16 rather than a
scoped enum.
The animation binder consumes it as u16 and searches cumulative keyframe
durations; the swing endpoint is not renamed as an angle or blend coefficient.
The sixteen shipped `KF/WEPON/WEP00.MIM` through `WEP15.MIM` files each
contain one clip whose keyframe durations total **4096**. Parsing the bounded
asset/keyframe tables independently establishes that the endpoint completes
the resource timeline. Fifteen clips use durations
`1365, 1024, 682, 341, 341, 343`; Colichemarde (`WEP03.MIM`) uses
`221, 221, 221, 110, 1107, 1107, 1109`. The sampled ordinary hit at 3300 lies
in keyframe four's `[3071,3412)` interval; Colichemarde's hit at 1200 lies
in keyframe five's `[773,1880)` interval. This locates each hit within its
animation without proving why the original designers selected its precise
window or the common 300-unit playback step.

`WEP00.MIM` is 26496 bytes, SHA-256
`2d41ee270bd61ed7262c67e0cda5fcf506c8dd304c6f171164c7759fa4105cf5`.
`WEP03.MIM` is 25388 bytes, SHA-256
`85663c23a4b6799d031d9b0a518596a73410cbb138b8eecec0bc367b94156055`.

Hit windows are halfword-wrapped differences: Colichemarde [1000,1300), other
weapons [3072,3372). Their width 300 is named separately from the phase step.
The hit volume uses an independent radius 800, height 1000 and camera-relative
downward offset 1000; equal numbers do not establish a shared dimension.

Weapon magic uses later [2400,3900] phases for Flame Sword, Triple Fang and
Moonlight Sword, and earlier [900,2400] phases for Colichemarde. The remaining
shot expressions retain division by the shared phase step, plus one inclusive
endpoint, and the Colichemarde factor two. Their endpoint 3900 matches the last
normal sampled swing phase, but no input timing or branch is rewritten.

The HUD converts charge to pixels with /100, giving 50 pixels at full charge.
The HP/MP expressions use the same nominal width but their original rounding
formula is retained literally in structure; no algebraic correction is made.

## Spatial constants

Player collision uses radius 800 and height 1700. The distance helper adds 850
twice to form the relative interval center and combined half-height, separately
from its signed point-height shift. Those constants become height/2. The camera
height 1500 remains independent. Eighth-scale X/Z differences bound the squared
length calculation; preserve each signed shift and the final left shift.

World-query flags 2177 are 0x881: skip terrain, skip player and capture the hit
object transform/radius. The mover checks the map cells itself. The initial
query uses the player radius; the slide retry uses hit radius+900, retaining a
100-unit clearance beyond the 800 player radius. Deflection angles 2112/1984
are half-turn +/-64, or +/-5.625 degrees around the away direction. These
are geometric roles; original tuning choices remain unproven.

Cell types 2..5 retain the shared oriented half-plane identities. When the
destination is blocked, equal-magnitude diagonal components use 2896 in Q12,
approximately 4096/sqrt(2). Retail computes it as ((3*d*16-3*d)*4+d)*16 before
the signed shift; preserve negation-before-shift and truncation. The per-axis
upward step test admits a rise of at most 699 world units, not 700. Retain this
boundary even though standing map heights are multiples of 100; the current
foot height can be mid-integration.

## Verification

Every row in the table retains its original strict percentage and raw body:
3591 candidate words, 134 calls and 485 ordered address references across
18 reviewed functions. Sixteen exact bodies independently reproduce all
1405 retail words and agree with the resolved target objects. The two
non-exact bodies retain their existing instructions and are not newly banked.
The first differences remain entry stack adjustments: horizontal movement
uses 80 candidate bytes versus 88 retail bytes at `800171fc`; player update
uses 216 versus 224 at `80018880`. These unchanged residues are unattributed;
they do not justify changing the recovered constants, types or control flow.

The before/after builds compile all 112 units from frozen sources and headers
at identical paths. Every runtime/data section, runtime symbol and ordered
relocation is identical; only debug line tables change in player core, player
update and frame rendering. The live comparison agrees outside the separately
edited entity traversal unit. Across all 484 captured scores, only that
concurrent `render_entities` change moved; none of the naming results moved.

All five affected units were forcibly rebuilt. Inventory validation, Ruff
and 678 existing tests pass (79.276 seconds). The player-data initializer
parser now accepts a named array bound while retaining its exact byte/order
check; no new tests were added. Modern checking retains the same 320 error
diagnostics and 64/112 passing variants. Full `kf build` retains the existing
data/placement failures: data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks
1/1, 75/77, 34/38; six conflicting section bases and zero artifact failures.
These results do not claim a passing full build.

The [new core ledger](game-player-core-literal-ledger.md) accounts for 73
retained numeric/character occurrences, and the refreshed
[motion ledger](game-player-motion-literal-ledger.md) accounts for seven.
Together they cover all 80 remaining occurrences in the complete module,
down from 164. Both initialized data claims and all fourteen functions are
included; bob and transform-snapshot functions have no remaining literals.
An independent lexer compares every function/line/token/expression
multiplicity against the two ledgers. With the 32 occurrences removed from
the four wider consumers, the batch removes 116 inline occurrences overall.
The wider caller modules retain their separate remainder-audit scopes.
