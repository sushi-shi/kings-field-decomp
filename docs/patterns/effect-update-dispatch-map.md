# effect_update_dispatch (0x80038a38) reconstruction map

GAME's per-kind effect dispatcher belongs to `game.effect_dispatch`
(`src/game/effect_dispatch.c`). Its original TU boundary remains WIP. The
first pass covers all 16 switch destinations and owns the 49-entry jump
table, but neither that coverage nor matching call counts proves its semantics.

## Function Match Plan (2026-09-06)

- Identity: GAME.EXE `0x80038a38`, body/extent `0x180c` (6,156 bytes),
  `void effect_update_dispatch(void)`. Its sole proven incoming call is
  `effect_pool_sweep` at `0x8003a7a4`, after publishing the current record;
  no argument or result is consumed there. The adjacent functions are
  `effect_spawn_ground_kind6` and `effect_pool_reset`, not evidence of a TU.
- Baseline: strict objdiff **37.901886%**, source `0x16f0` (1,468 words)
  versus retail `0x180c` (1,539 words), under `probe-gcc257-o2-g0`.
  The first raw divergence is the frame: source 200 bytes, saving `s0..s5`,
  versus retail 168 bytes, saving `s0..s8`; both save `ra`.
- Control census, source/retail: 69/69 direct calls with identical callee
  multisets, 131/135 conditional branches, 58/69 absolute internal jumps,
  one indirect switch jump and one return each. Each transfer owns its next
  instruction; the return's delay slot restores the frame. Counts are not
  paired CFG equivalence, and the navigator does not resolve the switch `jr`.
- Relocation census: source 167 versus target only 21 text relocations.
  Of 69 directly decoded retail calls, 64 still have unreviewed
  `instruction-word` rows; all 69 internal `j` rows are likewise withheld.
  The byte-indexed switch path independently bounds 49 words at
  `0x80012cf8`, whose contents select the handlers below. Its RODATA claim
  owns these pointer rows; do not create separate global identities for them.
- Referents: current effect/magic and kind-specific constants belong to
  `effect_state`; actor definitions belong to `actor_state`; grid indexing
  uses `map_floor_height_grid`. Validate signed-low HI16/LO16 encodings and
  complete-owner addends, not merely the rendered address names.
- Widths: `lbu` at record +1/+7 and later masks support byte snapshots;
  `actor_apply_damage` consumes five distinct halfword damage channels
  in `a2`, `a3`, and caller stack +16/+20/+24. Signed direction arguments,
  phase transitions and stack-object lifetimes need individual review.
- Vendor negative control: no dispatcher entry in `functions_vendored.tsv`;
  game-record kinds, magic channels, pool construction and map deformation
  exclude an SDK wrapper shape. Its `ApplyMatrix`, `rsin`, `rcos` and `rand`
  callees retain curated SDK/runtime providers rather than new game bodies.
- History: the separate unit's first pass is in `bf051cf`; the shared signed
  rotation correction in `a3a3e24` preserves every banked function. No
  recovered original source is claimed.

First repair only manually decoded direct-control rows and individually
checked owner-relative data pairs, then rebuild to separate target-model
movement from source movement. Next correct the shared projectile arm's
damage routing: retail `0x80038b78..0x80038ba8` passes
`0, 0, 0, magic[0], magic[1]`, not `0, magic[0], 0, magic[1], 0`.
Continue with independently evidenced CFG/width corrections, one cause per
focused build. Record residuals without assigning a compiler mechanism.

## Signature / register model

- `s4` = current effect record = `KfEffectRecord *` from `current_effect`.
- `s3`/`s1` = `current_effect_magic_record` (`KfMagicRecord *`).
- `s6` = `s4->kind` (`KfEffectRecord` +1). `s2` = `s4->unknown_07` (+7).
- Prologue: `sp,-168`; saves ra,s8,s7,s6,s5,s4,s3,s2,s1,s0.

## Dispatch

`switch (kind)` on `kind-4`, bounds `(unsigned)(kind-4) <= 0x30` else default.
Dense jump table `RODATA(0x80012cf8, 0xc4)` = 49 words (kinds 4..52). Our
gcc-2.5.7 should emit an equivalent table if the switch spans cases 4..52 with
`default` = return. Table (kind -> handler VA):

| kind | handler | | kind | handler |
|---|---|---|---|---|
| 4,5,7,8,10,11,12,14,22 | 0x80038ab4 (COMMON) | | 32 | 0x80039974 |
| 13 | 0x80038ab0 (common, s5=0x96 first) | | 33 | 0x80039a30 |
| 6 | 0x80039b58 | | 34 | 0x80039df0 |
| 9 | 0x80039e5c | | 36 | 0x80039250 |
| 15 | 0x8003a054 | | 52 | 0x8003a084 |
| 16 | 0x8003a06c | | 18 | 0x800395b0 |
| 17 | 0x8003a208 | | 19 | 0x800393b0 |
| 20 | 0x800396a4 | | 21,23-31,35,37-51 | 0x8003a214 (default/return) |

16 distinct table targets, including default and the kind-13 prefix that
falls into the common handler. `s5` starts at 0x64 and is overwritten with
0x96 at the kind-13 entry `0x80038ab0`.

## Callee set (all named game funcs + libc)

effect_map_collision, effect_magic_power, effect_pool_construct,
effect_spawn_trail_kind13, effect_spawn_ground_kind6, effect_scatter_triple,
effect_projectile_update_3d/2d, effect_floor_deform_line, vector_xz_to_angle,
angle_approach, rsin, rcos, matrix_set_rotation_x/y, ApplyMatrix, player_apply_damage,
player_apply_radial_damage, actor_apply_damage, actor_pool_apply_radial_damage,
actor_pool_spawn, actor_pool_find_target_in_cone, collision_query_world,
audio_play_spatial_range, audio_play_spatial_default_range, rand.

The epilogue at `0x8003a214` restores `s0..s8,ra` and executes
`addiu sp,sp,168` in the `jr ra` delay slot. Both function and RODATA claims
already exist; callees use shared typed declarations, not new K&R stubs.

## Reviewed relocations

The complete disassembly, jump-table bounds and all 49 table words were
inspected before editing. All 69 `jal` words decode to curated function
starts; all 69 `j` words decode to aligned instructions within this body,
with their actual delay slots retained. The 64 withheld call rows and 69
internal-jump rows now carry manual review evidence. Their original channel,
site, target and instruction fields are unchanged. Five previously reviewed
call rows remain untouched. This is a review of the selected decoded sites,
not a promotion based on `--confirmed-only` output.

Ten existing address-pair rows were reviewed, including the RODATA address.
Five absent pairs were added after checking the raw `lui`/`addiu` chain and
the independently curated complete owner:

| GAME HI / LO sites | Decoded target | Owner-relative interpretation |
| --- | --- | --- |
| `80038e1c / 80038e20` | `80095900` | `map_floor_height_grid` |
| `800391d4 / 800391d8` | `80095900` | `map_floor_height_grid` |
| `8003928c / 80039290` | `8009cfcd` | `effect_state + 0x16d`, magic row 18 sound 1 |
| `800394d0 / 800394d4` | `80095900` | `map_floor_height_grid` |
| `80039a20 / 80039a24` | `8009ceb5` | `effect_state + 0x55`, magic row 4 sound 1 |

All 160 rows in the function (138 MIPS26 and 22 paired rows) pass the shared
safe validator against hash-verified retail, with checked file offsets and
BSS-owner bounds. The canonical target now has **182 text relocations**
instead of 21. No relocation validator, switch-pointer inventory or data
ownership contract was weakened. Curation alone changes strict objdiff from
37.901886% to 38.048733%; it does not explain the source mismatch.

## Retained source corrections

| Retail evidence (GAME) | Source fact recovered |
| --- | --- |
| `80038b78..80038ba8` | The non-physical actor-hit path passes `0,0,0,magic[0],magic[1]`; O32 stack +20/+24 are separate damage channels. |
| `80038ca8..80038cb4` | Kind 7 advances after either actor or player damage, not actor damage alone. Unsupported collision classes still bypass that join. |
| `80039210..8003924c` | Phases 120/121 return without incrementing the phase, including the type-invalidation path. |
| `8003997c`, `80039a38`, `80039b40..80039b54` | Kinds 32/33 still increment the phase when invalidated. |
| `80039cac`, `80039d54`, `80039d98` | Kind 6's radial-damage origin is the live effect position, not its randomized spawn-position local. |
| `800395cc..80039600`, `8003962c`, `80039674` | Kind 18's radius is the entry phase times 333, saved before calls; the incremented live phase only controls lifetime/parity. |
| `80039eb0..80039eb8`, `80039f04..80039f40` | Kind 9 moves only X/Z; the phase increment also follows the collision/exhaustion arm unless entry phase was 40. |
| `800396f8..80039724` | Kind 20 compares the low byte at +0x38 with 0xff/0xfe, not the whole signed halfword with -1/-2. |
| `80039a70..80039a74` | Kind 33 stores the word value 0x0000ffff as Y, not 0xffffffff. |
| `8003a150..8003a160` | Kind 52 decrements the full position-Y word; no signed-halfword snapshot intervenes. |
| `80038a78/7c` and later `andi ...,0xff` | Kind and phase locals retain their unsigned-byte widths across calls. |
| `80038adc..80038ae0` | The collision arm reloads the current magic pointer; later impact damage/audio use this snapshot while the kind-4 path retains the entry snapshot. |
| `80038a8c`, `80038ab0`, `800393b8`, `800398d8` | Radius is initialized to 100 before dispatch, overridden to 150 for kind 13, and reused by kinds 19/20. |

Each row above received a focused rebuild before the next correction. Strict
scores in that order were 38.241714, 39.400910, 39.397660, 37.784275,
38.204030, 39.499676, 38.938923, 39.432100, 39.428200, 38.801170,
39.992850, 41.085770 and 41.769980 percent. Lower intermediate scores do not
falsify decoded argument, width or state-transition facts. No banked function
was changed by these local corrections.

The final focused experiment preserves each complete handler body and places
the cases in the order observed in retail: common, 36, 19, 18, 20, 32, 33,
6, 34, 9, 15, 16, 52, 17. The moved arms all terminate; internal labels stay
within their original arms. A text comparison verifies that this experiment
changes no handler body. It improves strict objdiff to **75.934370%**.
This is one disassembly-supported source-order correction, not a permutation
search or a claim about a compiler backend mechanism.

## Final verdict and next evidence

**Partial, not bankable.** The final body is 5,960 bytes / 1,490 words versus
retail's 6,156 / 1,539. Source/retail still have 131/135 conditional branches,
60/69 internal jumps and 69/69 direct calls with identical callee multisets.
The ordered call sequences differ: the first difference follows the common
audio paths, whose shared call lands at a different join in the source.
Thus matching the callee set is not matching the CFG or argument dataflow.

The first raw divergence remains the frame: source 208 versus retail 168,
with source saving `s0..s7` rather than `s0..s8`. Source has 171 text
relocations versus 182: the difference is nine internal jumps and one
`effect_state` HI16/LO16 pair. In kind 33 the source retains the address of
`effect_state+0x58` across calls and loads through it, whereas retail emits
another absolute pair. This is now a real source/address-lifetime difference,
not a missing curated target row. All other relocation-kind/symbol counts
agree, but ordered references are not claimed equivalent.

Known remaining source work precedes any compiler attribution: kind 19's
power-call guard differs; the kind-20 angle arguments require signed loads;
kind 52's loop counter has explicit retail halfword extension; and the
constructor's complete VECTOR input plus case-local object lifetimes still
need recovery. Do not pad the frame, force registers or add assembly to hide
these differences. The known retail uninitialized distance read also remains
explicitly documented in C, not silently initialized to an invented value.

Only this function's score changes among all 484 native function rows. GAME
remains **288/362 exact** (74 partial, none unstarted), OPEN 97/108 and PSX
1/1; all banked results and the vendored denominator are preserved. The
dispatcher is not passed to `kf bank`.

Focused comparisons and complete raw listings establish the partial result.
Shared safe relocation validation, `ruff check scripts tests`, **633 repository
tests** (76.105s, no skips) and `git diff --check` pass. `kf match --unit
game.effect_dispatch` and the full `kf build -j4` run but still fail the
unfinished data/ownership/placement gates: source data 5/60,
config-backed SDK data 4/4, target relink 110/116, six conflicting bases,
and zero artifact failures. No new tooling or flake change is part of this
campaign; concurrent clangd work is left out of its commit.
