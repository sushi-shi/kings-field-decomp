# GAME display environments and screen fade

The later [render setup correction](game-render-setup.md) closes the error
screen at strict 100% and improves TMD preparation to 99.375000%. The earlier
source/owner-only results below remain historical; the complete graphics
owner is still provisional.

## Function Match Plan

Image: hash-checked Japanese `GAME.EXE`. Start: `6137a42`. Use the unit's
pinned native compiler profile; it is not historical compiler attribution.
The six semantic views, callers, adjacent functions, source history and SDK
boundaries were inspected before editing. This is a shared-data campaign,
not an inferred historical TU boundary or complete graphics-owner migration.

First define the two complete environment arrays in their initializer's
existing `game.render` module, preserving the shared declarations in
`game/render.h`. Validate missing references before changing the screen
function's conditional RGB stores. Require raw instruction/referent checks,
strict function matching and complete BSS extent/identity/placement checks.
Do not count zero-filled BSS as retail initialized bytes. Preserve the
unresolved larger graphics owner and every previously exact function.

### Per-function evidence snapshots

| GAME VA / size | Function / starting strict score | Evidence and initial hypothesis |
| --- | --- | --- |
| `800144d4 / 1a0` | `display_play_transition` / 100% | No arguments, main-loop caller, 64-byte frame; 12 direct calls, seven address pairs and one internal jump; `B0\\L0.` path, two direct `PutDrawEnv` arguments, 18-frame fade with saturating increments of six. Keep the body unchanged as an external-owner control. |
| `8001b7b0 / 308` | `display_show_error_screen` / 97.922680% | Word stage argument, seven call sites, 80-byte frame; 19 calls, 22 validated address pairs, no indirect call; `\\E0.;1` plus stage digit, 50 CD attempts, 2 KiB rounding, FT4 corners `(32,112)..(288,240)`, brightness `0x60`. Byte buffer selection and 92-byte stride address `isbg` and `dfe`. Keep body unchanged; new storage must not manufacture a function improvement. |
| `8001bb94 / 14c` | `display_initialize` / 94.253010% | No arguments, main-loop caller, 32-byte frame; 13 calls and 14 validated pairs, no strings or indirect call. Initializes exactly two DRAWENV and two DISPENV records through four SDK calls, each with height 240 on the fifth-argument stack slot; width 320, alternating y=0/240. Body remains unchanged. Derived display/fog references retain a larger-owner hypothesis, not permission to claim unknown capacity. |
| `8001c050 / 98` | `display_present_frame` / 100% | No arguments, two callers, 24-byte frame; five calls and four validated pairs, no strings/indirect control. Byte index, draw stride 92, display stride 20, OT endpoint `+0x3fff`. Keep body unchanged. |
| `8002ac34 / 98` | `menu_present_frame` / 100% | No arguments, 45 call sites, same frame/call/stride/OT pattern as display presentation. Keep this separate TU body unchanged as an external-owner control. |
| `8002c794 / 240` | `screen_show_image_until_input` / 99.923615% | `const char *` path passed by six call sites; 80-byte frame, 16 direct calls, initially five validated pairs and one internal jump, no literal string or indirect call. Same FT4 coordinates/texture constants as the error screen; brightness starts at 32 and saturates at 127. Three genuine environment references are absent from the inventory. Raw fresh-source comparison first differs at `+0x15c`: source branch skips the RGB stores, retail branch reaches them. Fix these independently evidenced issues in referent-then-CFG order. |

All returns include their stack-restoring delay slots. Screen-loop branch
delay slots retain the comparison used on re-entry; the end-of-input loop
also begins environment-stride computation in its delay slot. The RGB change
must preserve all 16 direct calls and all eight materialized data targets.

The SDK calls are vendor controls, not reconstructed game bodies. GAME
`PutDrawEnv` at `80050d48` and `PutDispEnv` at `80050e68` match the supplied
LIBGPU SYS member offsets `834` and `954` and Release-2.5 FID evidence.
`SetDefDrawEnv` at `800548ec` and `SetDefDispEnv` at `80054978` are offsets
`14c` and `1d8` of the independently exact `20c`-byte LIBGPU EXT contribution.
Use the pinned `LIBGPU.H` types rather than private lookalikes.

## Complete arrays, not a complete graphics object

`DRAWENV` is 92 bytes: clip, offset, texture window/page, six byte fields and
the 64-byte `DR_ENV`. Its `dtd`, `dfe`, `isbg`, and RGB offsets are
`16`, `17`, `18`, `19`, `1a`, `1b` (hex). `DISPENV` is 20 bytes: two
rectangles, two byte flags and a halfword pad. Retail SDK initialization and
indexed presentation independently establish both two-element arrays:

| Identity | GAME base | Complete size |
| --- | --- | --- |
| `display_draw_environments` | `80090ec0` | `b8` (2 x `5c`) |
| `display_disp_environments` | `80090f78` | `28` (2 x `14`) |

The source claims cover 224 B, ending at `80090fa0`. The following eight bytes
before `tmd_state` are not part of either array. No initializer or fabricated
padding is added. Existing interior field references resolve to their parent
array; there are no overlapping byte identities.

The initializer also derives `PutDispEnv`'s pointer from DRAWENV[0].dtd
(`80090ed6 + a2`) and writes fog distance at `80090ed6 + 486a = 80095740`.
Together with the startup clear these support the separate
[graphics-owner pilot](game-graphics-owner-pilot.md); they do not prove
registry, projection or morph-array capacities. These logical array claims
must migrate together if a complete aggregate is subsequently established.

## Three reviewed missing references

All are adjacent `lui` / signed `addiu` pairs in the screen function:

| High site | Low site | Physical target | Parent-array addend |
| --- | --- | --- | --- |
| `8002c8b0` | `8002c8b4` | `80090ed7` (`dfe`) | `17` |
| `8002c8c0` | `8002c8c4` | `80090ec0` (array base) | `0` |
| `8002c9a0` | `8002c9a4` | `80090ed7` (`dfe`) | `17` |

Before curation, the shared safe validator accepted each decoded pair;
delinking selected `display_draw_environments` and the listed addends, and
reapplying its address reproduced both original words. Shifting each proposed
target by one byte was rejected as `decoded-target-mismatch`. No validation
rule is relaxed. There are now 26 reviewed incoming draw-array pairs and four
display-array pairs; derived intra-object addresses are not extra relocations.

## RGB branch fact

At `8002c8f0` retail `beqz v0,8002c8fc` has a `nop` delay slot. Only the
increment at `8002c8f8` is conditional; stores at `8002c8fc`, `8002c900`, and
`8002c904` execute on both paths. The old source places all three stores
inside the condition, emitting `10400005` instead of `10400002`. A very high
fuzzy score and an identical call set do not establish this CFG.

## Verification and final verdicts

The initial standalone definitions compile to 224 B of BSS, but with the old
shared header the compiler emits DISPENV at offset 0 and DRAWENV at offset 40.
The target has DRAWENV at 0 and DISPENV at 184. Strict comparison rejects the
identity layout and conflicting source section bases. Equal aggregate size
is not a data match.

A later declaration-only control locates this reversal in the **first
declarations**, not the DATA definition order: the header declared DISPENV
before DRAWENV. Prepending those declarations in either order reproduces the
corresponding allocation order while leaving every `.text` byte unchanged.
The production header now declares DRAWENV then DISPENV, matching the
independently established physical run and source-claim order. It retains
one shared declaration of each object and the original SDK types. No new
aggregate, padding, alignment/size waiver or invented capacity is adopted.
This probe-specific declaration-order result does not prove that the arrays
were independent globals in the historical source.

A controlled two-array `struct { DRAWENV draw[2]; DISPENV disp[2]; }` trial
does allocate one 224-byte object. It is nevertheless rejected as a production
rewrite: both previously exact presentation functions become 148 B with a
32-byte frame instead of retail's 152 B / 24-byte frame. The compiler also
derives the display pointer from the shared draw base, unlike these retail
consumers. The source structure needs evidence across consumers, not just one
initializer or one equal-sized allocation.

The extended complete-clear pilot preserves all 13 exact controls below but
does not close its three partial functions. It retains the opaque unknown
registry/projection/morph spans and does not become a production DATA claim.

### Additional complete-owner pilot snapshots and verdicts

The six semantic views for each row were inspected with the same GAME image,
including adjacent functions. None of these bodies is changed in production.

| GAME VA / size | Function / initial score | Independent evidence and pilot verdict |
| --- | --- | --- |
| `8001bab8 / 2c` | `lighting_set_active_color_matrix` / 100% | Word index, eight callers, 24-byte frame; one SDK call and one pair to the seven 32-byte colour matrices outside the graphics owner. Raw exact negative ownership control. |
| `8001bae4 / b0` | `effect5_texture_cache_prepare` / 100% | Word mode from floor loading, 24-byte frame, mode==5 branch; six calls/pairs, pages `(1,0,140/180/340,100)` and CLUT `(0,1eb)` repeated three times, halfword stores. Raw exact with shared-owner references. |
| `8001bfb8 / 98` | `display_begin_frame` / 100% | Two callers, no arguments, 24-byte frame, one call/seven pairs; byte toggle/reload, 12-byte primitive-buffer and 64 KiB OT strides, `ClearOTagR(...,4000)`. Raw exact, including cursor and three word resets. |
| `8001c0e8 / 2c` | `tmd_select` / 100% | Three callers constrain a u16 slot, 8-byte leaf frame, two pairs; index*4 asset load then selected-pointer store. Raw exact. |
| `8001c114 / 24` | `tmd_get_object` / 100% | Fourteen callers, u16 object index, frameless leaf, one pair; asset+12+index*28, return delay-slot addition. Raw exact. |
| `8001c138 / 10` | `tmd_set_current_vertices` / 100% | Vertex-pointer argument from pool binder, frameless leaf store/return/nop, one pair. Raw exact. |
| `8001c148 / 3c` | `tmd_select_object_vertices` / 100% | Eight callers, u16 index, 24-byte frame, one call/two pairs; object vertex offset plus selected asset+12. Raw exact. |
| `8001c2b0 / 300` | `tmd_prepare_primitive_indices` / 98.333336% | Three callers, no arguments, 8-byte frame, zero direct calls, one indirect jump; four pairs/seven internal jumps, 29 candidate incoming table pointers kept candidate. Halfword countdowns, 28-byte object stride, packet advancement `4+ilen*4`, eight modes `20..3c` after mask `fd`, indices shifted by three. Pilot remains 768 B but first differs at +8 (nop versus retail stack adjustment); switch-table ownership/addends remain unresolved. |
| `8001c5b0 / 3c` | `tmd_register` / 100% | Three callers constrain u16 slot and asset pointer, 24-byte frame, one call/one pair; slots accessed 32 bytes below current asset. Raw exact. |
| `8001c5ec / 20` | `tmd_release_last_allocation` / 100% | Menu release caller, word argument remains unused/candidate, 24-byte frame, one memory-release call and no pair. Raw exact negative ownership control. |

The existing six campaign rows have these final focused verdicts:

- `display_play_transition`, `display_present_frame`, and `menu_present_frame`
  stay 100% in canonical source and raw exact in the complete-clear pilot.
- `display_show_error_screen` remains 97.922680%. The pilot has a 72-byte
  rather than 80-byte frame and 772 versus 776 body bytes, first differing
  at the initial stack adjustment. Its 19 calls and 22 targets are retained.
- `display_initialize` remains 94.253010%. The pilot is 336 versus 332 B;
  first difference at +a4 selects a0 instead of s0 for the dtd address.
  Its 13 calls are preserved; all 15 materialized source targets are checked,
  including fog distance, which retail derives from the retained dtd base.
- `screen_show_image_until_input`: relocation-only curation moves 99.923615%
  to 99.965280%, with the same raw CFG difference. Unconditional RGB stores
  then reach strict 100% and match all 144 linked words, all 16 calls and
  eight ordered data targets. The complete-clear pilot is also raw exact.

`test_game_display_environment_data.py` measures authentic SDK sizes and
offsets through compiler output (no C size assertions), checks all 30 reviewed
retail pairs and wrong-target rejection, checks whole BSS layout/placement,
and rejects reversed first-declaration order, the old RGB branch and the
narrower aggregate. The extended pilot
in `test_game_graphics_owner_probe.py` checks complete raw bodies for exact
controls and records only the measured residues for partials. Shifting the
owner's address fails while retaining the same call set.

### Verified checkpoint

- Forced/rebuilt affected objects, then full `kf build`: 362/471 exact game
  functions (GAME 264), plus all 13 vendored source controls. Across all 484
  score rows, only the screen function changes from `6137a42`; no exact
  regression. Only `game.render` and `game.save_system` source/target object
  hashes change; all other 110 source and 110 target hashes are unchanged.
- The complete 224-byte render BSS now independently matches allocations,
  linkage, offsets, extent and section placement. Both arrays remain BSS, not
  synthesized retail load bytes. The seven-byte error path also matches;
  render RODATA remains non-exact, so whole-data unit totals stay 6/60.
- Four SDK contributions still independently match and relink. Target relink
  remains 110/116 with the same six pre-existing conflicting-base failures.
  GAME known-reference accounting moves from 88 to 90 source ranges and
  354 to 352 config-only ranges. After its two independent SDK providers,
  GAME has 350 un-compared config ranges; OPEN still has 268, total 618.
  This is not exhaustive reachable-byte closure.
- `ruff check scripts tests`, `git diff --check`, and all 624 local tests pass
  with no skips. `nix flake check -L` passes; its 624-test isolated run skips
  123 optional native/retail cases, which the local run exercises.
- Full `kf build` remains red for the known strict data/reachability/placement
  failures, not compilation, delinking, missing artifacts or exact regression.
  Only GAME `8002c794` is selected for function banking; no whole-data claim is
  banked or waived.

Next ownership work must retain these complete arrays and exact consumers
while resolving the wider graphics layout and the three recorded partial
pilot bodies. First-declaration ordering can explain a standalone COMMON
allocation; it does not resolve those separate referent/CFG/codegen questions.
