# GAME menu backdrop stack-frame campaign

## Function Match Plan: shared 64-byte debug lifetime

On master `ec7018b`, GAME `8002a510 menu_draw_window_backdrop` is 1700 bytes
and strict 99.971760%.  Its retail and candidate streams have the same
single-block CFG, sixteen calls, ordered data references, constants and body
instructions.  Only the frame differs: retail reserves 104 bytes and saves
`s0`-`s3`/`ra` at 80-96, while the probe reserves 40 bytes and saves them at
16-32.  All six image-qualified semantic views, its three callers, adjacent
functions, menu asset structures, SDK providers, source history and current
unit diff were refreshed.  There are no strings or candidate references.

The adjacent `800292f8 menu_draw_item_name_frame` has the same 64-byte frame
deficit (224 retail versus 160 probe); fourteen other functions in
`game.menu_runtime` are exact. This repeated, function-selective extent
motivates a shared local-object lifetime hypothesis but does not establish
one. First test the same GCC 2.5.7/O2/G0/R2000 profile with debug
information enabled, because old GCC can retain addressable local storage for
debugging even when optimized body accesses disappear.  Compare both affected
functions and all fourteen exact controls.  Do not retain the profile unless
it explains the two 64-byte extents without code regressions.  If it does not,
restore configuration before investigating a concrete shared source object.

The debug-profile control was negative.  Adding `-g` left both affected frame
sizes unchanged at 40 and 160 bytes, while debug-local relocation labels made
only six of the unit's sixteen function listings compare identically.  The
profile therefore does not explain the retail frames and was removed.  The
remaining source hypothesis must account for a real 64-byte local extent
shared by these two routines without disturbing the fourteen exact controls.

The tested GCC 2.6.0 profile also fails to explain this translation unit.
It preserves the same 40- and 160-byte candidate frames instead of adding
the missing extent, and
changes every one of the sixteen function listings through allocation,
scheduling, division lowering, and return-delay differences. GCC 2.5.7 is
therefore retained; the cause of the extra retail space remains unattributed.
These negative controls do not prove a missing source object.

A real `KfSpriteQuad` local describing the first backdrop tile adds one
16-byte stack slot, consistent with four tile descriptors accounting for the
64-byte extent.  It does not scalarize, however: the probe emits halfword
stores and reloads at `sp+20..26`, grows the frame only to 56 bytes, and
substantially changes the tile body.  `register` is byte-identical to that
failed aggregate form.  Retail has none of this stack traffic, so the typed
tile local is rejected and the direct packet construction is restored.

## Current-source frame controls at `1771fa2`

Revalidate retail hashes and refresh the six GAME semantic views for the
four functions below. Their documented residue is restricted to frame sizes
and stack offsets after the source and shared-owner corrections. Recompile
unchanged whole units with the three existing GCC 2.5.7 profiles, retaining
their O2/G0 settings and assembler. This tests the current sources, including
the recently recovered status-panel inline helper. It is not compiler
attribution or authorization to add an unused local object.

| Function (GAME VA) | Current strict % / bytes | Plain strict % / bytes | No-schedule strict % / bytes |
| --- | ---: | ---: | ---: |
| `render_map_cell` (`8001e5ec`) | 99.878380 / 592 | 97.175674 / 592 | 76.777020 / 592 |
| `effect_projectile_update_2d` (`80038298`) | 99.934210 / 608 | 92.151310 / 604 | 86.500000 / 624 |
| `menu_draw_window_backdrop` (`8002a510`) | 99.971760 / 1700 | 97.216470 / 1712 | 95.374115 / 1708 |
| `menu_status_panel` (`8002430c`) | 99.962170 / 1692 | 99.111115 / 1700 | 98.047280 / 1700 |

Profiles are `probe-gcc257-o2-g0` (`-mcpu=r2000`),
`probe-gcc257-o2-plain` (no additional cc1 flags), and
`probe-gcc257-o2-nosched` (`-mcpu=r2000 -fno-schedule-insns`). All twelve
selected controls first differ at +0. Every profile retains the candidate
frame size: 88 versus retail 120 for map cells, 56 versus 120 for the
projectile helper, 40 versus 104 for the backdrop, and 48 versus 112 for
the status panel. None recovers the missing extent.

Fresh canonical objects have exactly 18, 10, 12 and 16 unequal retail words,
respectively. Inspecting every unequal word confirms stack allocation,
save/restore, stack-field stores or stack-address arguments only. All other
resolved words agree without masking. All twelve controls preserve ordered
direct call targets and the data-address multiset. No-schedule reorders data
addresses in map cells and backdrop; the other ten preserve their full order.

Resolve every claim in each unit, including the private `.data` section bases
`80055e9c`, `80056268` and `80057e88` for map cells, effects and menu runtime.
Each delinked target independently reproduces its complete retail bytes.
Current/plain/no-schedule exact counts are map cells 1/0/0 of 2, effects
7/3/2 of 8, menu runtime 14/7/7 of 16, and status panel 0/0/0 of 1.
Thus the alternatives also regress exact siblings. No profile, source or
bank change is retained. Generated evidence, 81 per-function comparisons,
resolved words and source hashes are under `build/frame-profile-controls/`;
the refreshed six-view dossier is `build/frame-profile-evidence.txt`.

Full `kf build` retains GAME 337/362 exact / 99.428% aggregate and zero
artifact failures. Data matches remain GAME 11/41, OPEN 3/19 and PSX 0/1;
target relinks remain 75/77, 34/38 and 1/1. Existing data/ownership/placement
checks still fail. Ruff, all 713 repository tests (124.050 seconds), and
`git diff --check` pass. Only this evidence note changes in the campaign.
