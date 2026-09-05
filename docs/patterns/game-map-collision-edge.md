# GAME map-object collision-edge marker

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x80030c7c map_object_mark_collision_edge`, **572 bytes /
143 words**, unit `game.map_object_pool`, initially **87.692310%** strict
objdiff. Campaign starts at clean `3e96acf`, GAME **273/362 exact**. The
related forward probe and clear helpers use the same definition owner and
cardinal-yaw object policy. Retail hashes were validated with `kf init`;
all six required semantic views were read with `--image game`, including
unfiltered incoming/outgoing references and the current match state.

- A 29-block leaf, no frame, calls, indirect transfers or strings. The
  target has seven validated HI16/LO16 pairs and eleven validated internal
  absolute jumps: **25 ordered ELF relocations**. No candidate outgoing
  references were found. The first pair selects `map_object_state`; the
  other six select `map_collision_grid` with addends 0, 0, 0, +100, 0,
  -100, in that order. Internal jumps enter the narrowed-coordinate join
  or common return; the latter ends in `jr ra; nop`.
- `lbu` reads the low byte of the shared object's halfword cell X/Z fields
  at +2/+4, and byte ID +0 selects eight-byte definitions. Shared fields
  remain `u16`; the local coordinates are intentionally `u8`. Behavior is
  loaded unsigned, yaw is masked by `0xfff` then narrowed before dispatch,
  and only the low byte of value is stored. The O32 signature remains
  `void(const KfMapObject *, u8 value, u16 yaw)`.
- Types 2/3 write the current cell, change one byte-width coordinate for
  yaw 0/400/800/c00, then write the resulting cell. Non-cardinal yaws write
  the current cell again. Type 0 instead writes a two-cell edge selected
  by cardinal yaw; unsupported type/yaw paths have no stores. There is no
  justification for changing these predicates, signed index promotions,
  byte wrapping in types 2/3, or the final return.
- All five proven callers were inspected in source and retail. Loader
  `800313e0` passes object, value zero, and signed-halfword rotation Y;
  its delay slot advances the placement cursor. Runtime `80031e0c/e58`
  pass values 1/0 and rotation Y minus 1024 in the call slot. Runtime
  `80031f04/f5c` pass rotation Y and set value 1/0 in their call slots.
  None uses a return value. These calls do not resolve the dispatcher's
  independently incomplete indirect-control inventory.
- The complete preceding map-copy body and following forward-probe body
  were inspected, together with original `7c7c843:src/game/map.c`, current
  source history and shared layouts. Neither neighbor needs an edit.
- Vendor negative control: absent from the vendored inventory and supplied
  FID matches. The custom object metadata, 100-column collision grid and
  cardinal edge policy are game logic, not an SDK/GTE/libc wrapper. No
  library body is reconstructed or added to game progress.

The current object is 564 bytes. Earlier guard displacements differ because
the return is eight bytes earlier, but the complete types-2/3 operation
already agrees. In type 0, the yaw-0 and yaw-800 cases materialize a biased
grid base (-100/+100), then derive the unshifted base from it. Retail loads
the unshifted base, derives the neighboring row, and retains the original
base for the second store. The yaw-800 final store is also merged into the
yaw-c00 tail in the current object, unlike retail. The first real source
hypothesis therefore concerns the two-destination expression, not forced
registers or a changed data identity.

First hypothesis: represent each opposite-row pair as a chained assignment,
with the current-row destination outside and the neighboring-row assignment
inside. Both cells receive the same byte, with the neighboring write still
the inner assignment. This gives the compiler visibility of both lvalues
in one expression while keeping the existing typed grid, value, constants,
case structure and dynamic stores. Leave the two same-row pairs untouched.
Require strict 100%, all 143 words and all 25 ordered relocation kinds,
referents and addends before closure; reject unsupported source distortion.

## Exact result

The first trial is **100% strict objdiff** after focused rebuilding and
canonical matching, up from 87.692310%. Both objects have **143 identical
encoded words**, including all internal jump addends and delay slots; all
**25 ordered relocations** agree in offset, kind and symbol. The opposite-row
cases now materialize the unbiased grid base, derive the neighbor by a
signed 100-byte adjustment and store the current-row byte in the jump delay
slot. The non-retail cross-case store-tail merge disappears. All other source
in the function, its signature and the shared object/data identities remain
unchanged.

The relevant source shape is simply:

```c
map_collision_grid[cell_z][cell_x + 1] =
    map_collision_grid[cell_z - 1][cell_x + 1] = value;
```

The mirrored case uses `cell_x - 1` and `cell_z + 1`. This result proves
that the two-store expression can explain the retail address association
and CFG under the pinned probe. It does not uniquely identify historical
syntax, compiler attribution or an optimizer mechanism. No forced locals,
registers, pointer casts, artificial padding or assembly were introduced.

## Verification

The unit moves from **4/8 to 5/8 exact**, and GAME from **273/362 to
274/362**. This target is the only changed comparison across all 484 report
rows (including 13 vendored controls); the other 483 are unchanged. The
previously improved forward probe remains 93.755104%, the pool reset 78.25%,
and map copy 94.256195%. OPEN stays 97/108 and PSX 1/1 exact.

All **551 repository tests pass** (53.342 s), Ruff and `git diff --check`
pass. The full `kf build` still exits 1 on existing data/ownership/relink
closure gates: source data 11/59, config SDK data 2/2 and target relink
108/114, with incomplete known-reference data ownership. No closure gate or
inventory entry was weakened. Only this strictly exact function is banked.
