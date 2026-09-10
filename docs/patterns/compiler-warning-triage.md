# Uninitialized-local and missing-return warning triage

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
