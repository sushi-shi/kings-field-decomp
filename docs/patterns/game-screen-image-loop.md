# GAME screen-image loop

## Function Match Plan

Target: GAME `0x8002c794 screen_show_image_until_input`, 576 bytes / 144
instructions in `game.save_system`, existing `probe-gcc257-o2-g0`. Starting
strict objdiff is 99.923615%. The six semantic views, all six direct caller
sites, both adjacent functions, custom callees, SDK interfaces, data owner
and source history were inspected before editing.

Signature: `void (const char *path)` in `a0`, saved across packet setup and
forwarded as the second argument to `cd_file_load_into`. Callers never
consume a result:

| Caller | GAME call sites | Path argument |
| --- | --- | --- |
| `actor_show_info_image` | `0x80017f8c` | `enemy_info_image_path_template`, derived from its +3 member in the delay slot |
| `map_event_show_person_image` | `0x8001803c` | `person_image_path_template`, derived from +8 |
| `talk_show_indexed_image` | `0x8002ca60` | `talk_image_path_template`, derived from +6 |
| `map_ambient_script_floor5` | `0x80034368`, `0x80034390` | literal addresses `0x80012a54`, `0x80012a68` |
| `map_show_screen_image` | `0x80034dcc` | `DAT_80056238`, derived from +5 |

The 80-byte frame saves `ra/s3/s2/s1/s0` and holds a real 40-byte SDK
`POLY_FT4` at sp+16. Brightness starts at 32; an unsigned byte tracks whether
all input has first been released. The packet has screen corners x=32/288,
y=112/240, UV corners u=0/255, v=0/128, CLUT (0, 501), page (0, 0, 960,
256) and semitransparency enabled. The loader's nonzero return exits through
the same epilogue; otherwise TIM upload precedes display setup.

The inverse of the current buffer index selects one of two `DRAWENV`s.
Retail clears `isbg` and `dfe`, calls `PutDrawEnv`, then uses the matching
16,384-entry ordering table. Each frame increments brightness only below
127, writes RGB, clears/links/draws the ordering table, and advances through
input-release, input-press and final input-release states. The final state
restores both environment flags to 1. The return delay slot restores 80
bytes. The internal absolute jump at `0x8002c954` returns to +0x15c, setting
the input-state byte value to 1 in its delay slot.

There are sixteen proven calls: `DrawSync` three times, `SetPolyFT4`,
`SetSemiTrans`, `GetClut`, `GetTPage`, `cd_file_load_into`,
`tim_upload_images`, `PutDrawEnv`, `ClearOTagR`, `AddPrim`, `DrawOTag` and
`PadRead` three times. Initially five validated HI16/LO16 address pairs name
display-state members and the `DRAWENV.isbg` member; the one internal jump
is also validated. There are no strings or unresolved indirect transfers
inside this function. Candidate inventory status on direct `jal` rows does
not change their decoded proven call evidence.

The custom loader builds and reads a CD path into the supplied buffer; the
TIM helper iterates `ReadTIM` records, uploads image/CLUT rectangles and
waits for GPU completion. The supplied Release 2.5 `LIBGPU.H` and project
pad interface retain authentic SDK signatures. The vendored inventory
attributes packet helpers to exact PRIM archive-section/FID evidence,
ordering/environment helpers to SYS FIDs, and `PadRead` to the revisioned
LIBETC PAD family. This custom file/input/display loop is not an SDK body
and is absent from the vendor/FID function inventory.

The immediately preceding `save_file_cleanup_temporary` ends with `jr` at
`0x8002c78c` and frame restoration at `0x8002c790`; the following
`talk_show_indexed_image` starts at `0x8002c9d4` and is also a caller. Source
history `e745bef` introduced the same conditional RGB block; later changes
replaced split display globals with typed owners, not that loop scope.

### Referents first

Three missing address pairs are independently decoded within the existing
`display_draw_environments` identity at `0x80090ec0`, extent 184 bytes:

| Pair sites | Raw HI / LO words | Absolute target | Owner addend |
| --- | --- | --- | --- |
| `0x8002c8b0` / `0x8002c8b4` | `3c018009` / `24210ed7` | `0x80090ed7` | +23 (`dfe`) |
| `0x8002c8c0` / `0x8002c8c4` | `3c028009` / `24420ec0` | `0x80090ec0` | +0 (`DRAWENV[0]`) |
| `0x8002c9a0` / `0x8002c9a4` | `3c018009` / `24210ed7` | `0x80090ed7` | +23 (`dfe`) |

The two flag addresses feed indexed byte stores; the base feeds the
`PutDrawEnv` argument. SDK `DRAWENV` has 92-byte stride and unsigned-byte
`dfe` / `isbg` at +23 / +24; the latter already has validated pairs beside
both missing `dfe` sites. Admit these three pairs without changing their
absolute referents or introducing overlapping field identities. Rebuild the
target and inspect shared validation before changing C.

### Then the loop scope

Retail's brightness branch at `0x8002c8f0` targets `0x8002c8fc`, the first
RGB store. Compiled C branches to `0x8002c908`, skipping all three stores as
well as the increment. Move the RGB assignments outside the `brightness <
127` block, keeping the increment conditional and every other expression,
type, constant and call unchanged. This follows decoded CFG, not score
steering. Require focused rebuilt comparison, strict 100%, raw words and
ordered relocation targets, full build and existing lint/tests before
banking only this intended function.

## Final verdict

The relocation-only correction reaches strict 99.965280%; all three new
pairs pass the shared semantic validator. Moving the RGB stores outside the
conditional then reaches strict **100%** in the focused canonical match.
The unit now has 22/24 exact functions.

Both function bodies contain 144 words and the same 33 ordered relocations.
All literal words agree except the internal `R_MIPS_26` jump at +0x1c0:
preceding partial functions shift the compiled section position by four
bytes, but both jumps resolve to this function's +0x15c. Constants, external
referents, branch targets and delay slots agree without an exception.

GAME advances from 279/362 to **280/362 exact**. Across all 484 native
function rows, only this function and the separately documented profiled
actor selector improve; the other 482 retain their sizes and strict scores.
OPEN remains 97/108 exact, PSX 1/1. No SDK body is counted as game progress.

Ruff, all 551 existing tests and `git diff --check` pass. Full `kf build`
rebuilds the affected comparisons and still rejects the existing explicit
data/ownership/placement gaps: 11/59 source-data units, 2/2 independent SDK
data controls, and 108/114 target relinks. Those gates are unchanged.
Only GAME `0x8002c794` is eligible for the new bank entry.
