# Compiler warning triage

## Current review

The refreshed audit on `6158b383` covers **101 image/TU variants** (including two
vendored PAD references). The older README counts came from 99 variants and
predate shared fragments and the current compatibility headers. Deduplication
uses complete diagnostic lines per compiler mode, so shared includes reached
through GAME and OPEN paths can appear twice. Counts are not distinct defects.

| Mode | Before cleanup | After cleanup | Compiler failures |
| --- | ---: | ---: | ---: |
| Clang C++20 | 3,860 | 3,845 | 0 |
| Clang C89 | 2,734 | 2,719 | 0 |
| Pinned GCC | 646 | 631 | 0 |

Reproduce all flags, per-unit logs and the deduplicated diagnostic TSV:

```sh
nix develop -c python3 -m scripts.kf.warnings --output-dir build/warning-review/current
# report.json, diagnostics.tsv, and per-mode/unit logs under that directory
```

The command retains `-Weverything`, `-Wsystem-headers` and `-pedantic` for Clang,
and the supported maximal warning set for the pinned GCC front ends. No warning
is suppressed to lower the totals. It does not change the normal build flags.

### Safe cleanup and remaining priorities

Removed 13 trailing enum commas, the unused `step` local in
`GAME:800171fc player_move_horizontal`, and the unreferenced
`invalidate_and_return` label in `GAME:80038a38 effect_update_dispatch`.
All 100 GAME/OPEN ELF base objects and all three native CPE outputs are
byte-identical to the verified pre-cleanup baseline. All 101 units retain their
function, DATA and RODATA claims. Fresh full build and analysis retain
**458/471 exact game functions** (PSX 1/1, GAME 351/362, OPEN 106/108) and all
13 exact vendor references. The existing data/closure audit failures remain.

The remaining categories have these dispositions. This is a family-level
triage, not a claim to have individually resolved every conversion or buffer
access diagnostic.

| Diagnostics | Disposition on decomp |
| --- | --- |
| C++98/pre-C++14/pre-C++20 compatibility; C89 GNU `typeof` extensions | Expected language-view differences. Keep C++20 enum/type checking and the historical C ABI view. |
| SDK invalid UTF-8 comments, CRLF, old-style/non-prototype declarations, redundant declarations, traditional macro spelling | Preserve original SDK headers. Do not replace them with guessed externs or rewrite archived headers to make the audit quiet. |
| Reserved/dollar identifiers, ABI padding | Check ownership; SDK names and compiler/linker spellings are not ordinary application identifiers. Do not pack SDK types or rename binary identities for warning counts. |
| Cast alignment, unsafe buffer/libc access, pointer arithmetic | Alignment and complete-object/buffer contracts need per-site evidence. A cast or a bounds guard alone does not settle the warning. Keep the existing cast/ownership campaigns as the worklist. |
| Narrowing, signedness, enum-domain conversions, old-style casts, zero null constants | Candidate type/spelling fixes after inspecting callers and retail extension/truncation instructions. Keep intentional packed-field narrowing and historical parameter promotions. Do not add casts just to suppress diagnostics. |
| Missing function/variable declarations | Candidate shared-owner header work. Establish linkage and authentic signatures first; adding a local extern, `static`, or a guessed SDK prototype can change ABI/relocations. |
| Missing field initializers | Implicit zero initialization is defined; explicit field values are possible after layout/data comparison. Do not add artificial members or change constructor/enum-storage semantics. |
| Missing/covered switch cases, default labels, fallthrough, comma operator | Inspect selector domain and control flow. Partial selectors and intentional fallthrough are not grounds for adding behavior. Comma-expression expansion is a focused match candidate. |
| Shadowed names | Straight renaming is a promising source cleanup; scope-specific use/identity review and object comparison are still required. |
| Unused parameters/macros/locals | Keep ABI parameters, C++-only configuration macros, and the two documented OPEN stack-slot locals. The genuinely unused local/label above were removed. |
| Missing `noreturn`, missed NRVO | Optional annotation/modern optimization diagnostics. Adding attributes can change caller control flow and code generation; no decomp requirement to eliminate them. |
| Uninitialized reads/passed buffers and missing returns | See the instruction- and caller-based verdicts below. Keep inherited behavior visible; intentional repairs belong in `port`. |
| Tautological comparisons / unreachable code | Evidence-review candidates: reconstructed spatial audio's `(tone & 0x80) == 1`, actor's high-half/low-half ceiling-code comparison, and the unsigned charge clamp's lower bound. Dead source spelling cannot be recovered just from bytes that omit it. Do not claim these are all original C bugs or change predicates based on host diagnostics alone. |
| Unsequenced resource cursor access | Five loader sites: GAME `map_resources_load`; OPEN scene0, scene1, ending and ending-sequence loaders. The C arguments read and modify `stream` without sequencing. C++20 sequences parameter initializations but leaves their order unspecified, so the intended header/body pairing is still not guaranteed by the source. Requires a matching source repair. |

For the resource warning, a controlled GAME candidate saved the first chunk
pointer, advanced `stream` in a separate statement, then passed the two payload
pointers. Retail proves the intended pair at `8001b5c4..8001b5d8`. The candidate
adds `move a0,s2` before loading the chunk size and moves the audio call from
function offset `0x7c` to `0x80` (body `0x258` becomes `0x25c`). It was reverted;
the five warnings remain open, and no equivalent OPEN rewrite was applied.
This is a concrete rejected source spelling, not evidence that every properly
sequenced implementation must fail to match.

### Expanded uninitialized-read and data review

The three inherited scalar reads below are the historical **GCC** subset.
Clang also identifies `target_distance` in GAME's `effect_update_dispatch`:
the player-homing path loads stack `sp+120` at `80039760` before negating it for
the pitch call at `8003976c`; only the alternative actor-search path passes that
slot to `actor_pool_find_target_in_cone` at `8003978c`. That makes **four known
inherited scalar reads** across the two compiler reports. Clang C also warns
about three uninitialized direction buffers passed to the effect constructor
from `actor_spawn_action_effect`, `map_object_pool_load`, and
`map_object_pool_update`. Their construction paths are documented in the
[actor constant](game-actor-constants.md),
[map-object constant](game-map-object-constants.md), and
[script/motion](game-map-script-motion-constants.md) reviews. These are separate
buffer warnings, not three more proved scalar reads or a reason to zero memory.

The two open data preconditions now have shipped-data checks:

- The existing GAME asset corpus (`tmd_oracle.shipped_cases`, decoded with
  `animation_oracle.parse_asset`) contains **214 clips across 70 animated assets**;
  none has zero keyframes. This removes the empty-clip concern for the enumerated
  serialized clips. It does not prove all runtime clip selectors, cache states,
  or alternate resource paths valid. The inherited keyframe-index read remains.
- The five `B1..B5/MIXA.DAT` object-placement lists contain **51 door placements**,
  including **14 hinged doors**; every hinged placement has a cardinal masked
  initial yaw. Definitions come from chunk 5 of `COM/COM.DAT` (141 serialized
  eight-byte entries); placements are chunk 4, 20-byte records through the
  `0xff` terminator. This checks initial orientation, not every runtime action,
  paired-door mutation, or save-restored state. The action/definition consistency
  requirement in `map_object_probe_forward` remains open.

No default result or initialization was added to game logic. Fresh image-specific
evidence and the before/after reports are under `build/warning-review/`.

## Historical GCC local/return audit

The maximum-warning audit at `d158ac60` found **18 potentially uninitialized
locals**, not eight, and **three missing-return diagnostics** in GCC 2.5.7.
These are counts of locals/functions, not repeated header diagnostics.
This review changes no C source or compiler flags.

## Local verdicts

| Image and function | Locals | Verdict and evidence |
| --- | --- | --- |
| GAME `8003a9f4`, OPEN `8001a5d4` `format_vsprintf` | `width`, `padding_mode` in each image (4) | State-invariant false positives. Parser state begins as text. `%` initializes both locals and enters conversion state; numeric conversion can reach padded output only from that state. Literal and string paths do not read these locals. |
| OPEN `80014e28` `opening_ending_scroll_run` | `sequence_delay`, `sequence_volume` (2) | State-invariant false positives. Entering DELAY assigns the delay; entering FADE assigns the volume. The initial WAIT_SCROLL state reads neither. |
| GAME `80036618` `player_warp_shimmer` | `scale_y`, `scale_y_step` (2) | Caller contract. All three supported modes initialize both. Known direct callers and callers of `player_warp_shimmer_at_player` supply supported constants; unsupported values fall through without initialization. |
| OPEN `80014608` `opening_entity_transition` | `initial_scale_y`, `scale_step` (2) | Caller contract. GROW and SHRINK initialize both; CREATE initializes and returns, REMOVE bypasses their use. The four scene call sites supply these supported modes. |
| GAME `80036e38` `menu_enter_mode` | `result` (1) | Caller contract. ROOT, PICKUP and SHOP assign a result. Known callers select these modes; there is no default for unsupported values. |
| GAME `800238d8` `menu_equip_select` | `start`, `end` (2) | Caller contract. Seven equipment categories assign both bounds. `menu_option_root` calls this function only from those seven switch cases; MAGIC is handled separately. |
| GAME `80030eb8` `map_object_probe_forward` | `result` (1) | Conditional hazard. Lift doors assign it; hinged doors require a cardinal masked yaw. The two calls are in door-closing paths. Valid placement orientation and action/definition consistency remain preconditions, not a general proof that every possible object is safe. |
| GAME `800205d4` `render_bind_animated_instance` | `keyframe` (1) | Asset precondition. A selected clip needs at least one keyframe. Zero keyframes bypass assignment and can reach a dereference when the vertex cache is refreshed. This review does not establish a complete shipped-asset census proving that case absent. |
| GAME `8002fa88` `actor_update_current_action` | `home_x`, `home_z` (2) | Retail-inherited uninitialized reads. RETURN_HOME initialization reaches the heading call before either coordinate is assigned. The assignments belong to the later collision-clear branch. |
| GAME `800205d4` `render_bind_animated_instance` | `keyframe_index` (1) | Retail-inherited uninitialized read. The index is incremented/decremented and used for cache comparison/storage without an initial assignment, including with nonempty clips. |

Totals: **six state-invariant false positives**, **nine input/data-contract
warnings**, **three retail-inherited uninitialized reads**. The nine contract
warnings include seven locals whose known callers constrain the selector and
two whose data preconditions remain explicitly open. This is a triage of the
GCC list, not a claim that all Clang-only warnings or all undefined behavior
have been audited.

## Decisive retail instructions

In GAME's actor function, RETURN_HOME initialization branches from
`800303ec` or jumps from `800303fc` to `80030500`. That path bypasses the
coordinate assignments to `s3` at `80030444` and `s2` at `80030450`.
`80030508` subtracts the current X from incoming `s3`; the delay slot at
`80030510` subtracts current Z from incoming `s2` for the call to
`vector_xz_to_angle` at `8003050c`.

In GAME's animation binder, `s5` is saved at `800205e8`, but never initialized.
`80020790` increments it, `800207a4` decrements it on loop exhaustion,
`800207c4` narrows it for comparison, and `800208ac` stores it as the cached
index. Separately, the zero-count branch at `80020710` bypasses the keyframe
pointer assignment to `s3` at `8002072c`. These are raw instruction facts;
matching C with an uninitialized local does not create a portable C contract
for an incoming saved register.

## Missing returns

| Image and function | Verdict |
| --- | --- |
| GAME `8002c510` `memory_card_show_status_message` | Deliberately incomplete return contract. The exceptional loader result produces `return -1`; the other path falls through. All five known direct callers discard the result. Retail leaves the loader's physical `v0` unchanged on that path, which is not a C promise to return it. |
| GAME `800501ac`, OPEN `8002ff80` `pad_init_bad_identifier` | Two vendored Sony LIBETC diagnostics. Each calls `printf` and returns without assigning `v0`. `PadInit` captures the incidental value, preserves it across `ResetCallback`, and returns it. Known game/opening initialization callers use `PadInit(0)`, bypassing the bad-identifier path. Keep these outside game reconstruction progress. |

The status-message form already has controlled explicit-return comparisons
in [game-status-result-traces.md](game-status-result-traces.md). The SDK
fallthrough evidence is also recorded in
[source-shapes-gcc257.md](source-shapes-gcc257.md). Adding a default return
or initializing every warned local would invent behavior and can break banked
matches. The unresolved data preconditions deserve evidence work; the inherited
reads and incomplete returns should remain visible.

## Verification scope

`kf init` verified the retail images. Image-qualified address, CFG/disassembly,
incoming/outgoing references, strings and current match views were collected
for all thirteen functions under `build/warning-triage/`; the warning commands
and logs remain under `build/warning-audit/`. Generated evidence is untracked.
Current recorded matches are strict 100% for twelve functions; OPEN's ending
scroll is 99.917694%. These are inspected match states, not fresh build claims.
No source/configuration changed, no result was banked, and no game was run.
