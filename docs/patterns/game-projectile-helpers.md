# GAME projectile helper evidence

The later [effect literal audit](game-effect-literals.md) corrects the 3D
helper's former velocity/landing interpretation: its input is a rotated
collision-probe offset, and release raises the object. The historical matching
results below are unchanged; the current parameter is `probe_offset`.

## Function Match Plan

Start on master `1a9e57f`, with hash-validated retail and the unchanged
`probe-gcc257-o2-g0` profile. The two helpers share current-effect/magic
state, damage calls and impact-sound policy in `game.effect_update`. Inspect
both, but do not equate their near-exact scores with similar remaining work.
All six semantic views, dispatcher call sites, adjacent helpers, shared types,
source history and a fresh focused compile precede the first source edit.

| GAME function | Extent | Initial strict match | Evidence and first hypothesis |
| --- | ---: | ---: | --- |
| `80037fe0 effect_projectile_update_3d` | 696 B | 96.752880% | Two dispatcher calls pass `SVECTOR` pointers and limits 40/60. Retail has a 144-byte frame, two matrices at sp+40/+72, VECTOR at +104, ten calls, four internal jump relocations and two global-address pairs. Recover the unsigned byte lifetime local first, then inspect the signed pitch store and the landing update order. |
| `80038298 effect_projectile_update_2d` | 608 B | 99.934210% | Dispatcher passes speed 6500 and limit 40. Nine calls, three internal jump relocations, five global-address pairs. The fresh body agrees except for ten frame/save/restore instructions: retail frame 120, source 56. No source change is justified by the unexplained 64 bytes alone. |

Neither helper is vendored: neither address is in the vendor inventory, and
both use the custom 60-byte effect record, selected magic damage components,
GAME collision classes 0x10/0x80 and GAME actor/player damage APIs. SDK calls
are external (matrix/trigonometry, SquareRoot0 and rand), not bodies claimed
as game progress. Neither helper references strings or owns a switch table.
Existing address/internal-jump evidence is validated; direct calls are proven.

The shared record stores `unknown_07` as an unsigned byte. In the 3D helper,
retail loads it once into s4 with `lbu`, masks it before the initial <2 test,
and masks the preserved snapshot again at 3821c before testing ==1. The source
uses an `int` snapshot and its second mask is optimized away. Test `u8 life`
as a genuine width correction, not a dummy mask carrier. The 2D helper does
not keep its snapshot across calls and is a separate negative control.

At 381a8, retail materializes -512 with `addiu`, then stores its low halfword
to rotation_x; source materializes unsigned 0xfe00 with `ori`. Both write the
same halfword, but the signed `lh` comparisons and the SVECTOR boundary call
for reviewing the shared rotation field family before choosing a correction.
The second clamp intentionally tests rotation_y at +30, not rotation_x;
preserve this decoded behavior even though it looks asymmetric.

At 3822c..38234, retail zeroes the next pitch before publishing life=10 in
the following jump's delay slot. Current source states the byte publication
first. Recover that independent statement order after the width experiment.
Check actual branch destinations and delay slots, not just transfer counts.

The original reconstruction `1914c0e` already contains the same wide lifetime
local, unsigned rotation fields and landing statement order. No surviving
original C body is available here. Do not introduce unused matrices, padding,
volatile locals or assembly to manufacture the 2D frame. Require canonical
strict 100%, raw words and function-relative ordered relocations for closure;
preserve all banked functions and run the full build before handoff.

## Focused reconstruction

The unsigned-byte snapshot restores the second mask and the full retail body
extent. Stating `next_pitch = 0` before the life-byte update restores the
landing delay slot. Together these reach strict 99.655174%, leaving only
the signed negative-constant instruction different.

Next shared-type hypothesis: make all three rotation components `s16`,
consistent with the SDK's `SVECTOR` (Release 2.5 LIBGTE.H, signed short
components) and the existing signed `KfEffectRenderView.rotation` view of the
same fields. The constructor copies eight bytes into +1c..+23 at 3745c..37478
and 374bc..374d8, then overwrites pitch with 512 for these 3D kinds. Other
constructor paths negate pitch modulo a halfword (3741c..37428). Arithmetic
that only stores a low halfword does not establish unsigned source ownership.
Dispatcher angle-approach/matrix calls use signed angle interfaces; masked
phase accumulation, including exact `player_warp_shimmer`, remains modulo4096.
Preserve the record's size/offsets and its other field types. Freshly rebuild
all shared-header consumers and reject any banked regression.

The signed rotation fields close the 3D helper at strict 100%. All banked
GAME functions, including the five-function warp unit, remain exact. The
unfinished dispatcher moves from 37.918777% to 37.901886%; a controlled old/new
header rebuild identifies four changed argument loads in the inspected
instructions. Both compiled extents are 5872 bytes (1468 words). The captured
control listings omitted an interior span, so they do not independently prove
identity throughout that omitted span. Each identified change replaces
`lhu a0` with `lh a0`: pitch/yaw
arguments to angle_approach and matrix_set_rotation_x/y. Retail uses `lh` at
39824, 39838, 39860 and 3989c, respectively. The new field type improves all
four real argument handoffs despite the slightly lower aligned score. Keep
the correction; do not restore the wrong unsigned field to protect the metric.

Removing the now-redundant pitch casts and lifetime masks preserves the exact
3D body. Shared prototypes retain the current full-word source parameters and
explicit signed-halfword limit consumption; call constants alone do not prove
the original C parameter spelling. Curated function/field identities now agree
with source, and compile-time checks retain the 60-byte record and three
rotation offsets. The 2D body itself is unchanged.

## Final verdict

| GAME function | Strict result | Raw audit |
| --- | ---: | --- |
| `80037fe0 effect_projectile_update_3d` | 100% exact | All 174 instruction words and 18 function-relative ordered relocations agree, including constants, branch destinations and delay slots. |
| `80038298 effect_projectile_update_2d` | 99.934210%, unchanged | 152 words on each side; exactly ten frame/save/restore words differ. All 22 ordered relocations agree. Extra retail frame space remains unattributed; no artificial locals added. |

The shared field correction also makes four dispatcher argument loads agree
with retail as documented above. Those are the only two score changes among
484 native report rows (including 13 vendored controls). GAME advances from
287/362 to 288/362 strict exact, with 74 partial functions and none unstarted;
OPEN stays 97/108 and PSX stays 1/1. No banked function regresses. The full
100% GAME goal remains open.

The final full `kf build -j4` runs all three image checks and still exits
nonzero on the existing ownership/placement failures: source data 5/60,
config SDK contributions 4/4, target relinking 110/116, six conflicting
section bases and zero artifact failures. These are not function-code closure.
The focused effect and warp compiles were real rebuilds, not Ninja no-ops.
Ruff passes, all 612 existing tests pass with no skips (72.307 seconds), and
`git diff --check` is clean. No tooling or flake changes were made. Selectively
banked only GAME `80037fe0`, not the partial dispatcher or unresolved 2D helper.
