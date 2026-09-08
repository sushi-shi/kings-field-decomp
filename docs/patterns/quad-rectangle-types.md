# Typed screen rectangles for simple quads

## Texture-descriptor offset naming follow-up

Function Match Plan: name the existing eight-byte descriptor extent and its
four consumed byte offsets: U at zero, V at two, U span at four, V span at six.
Keep the odd bytes opaque, the byte-aligned GAME copy wrapper, all authored
descriptor bytes and the existing pointer API. These names describe decoded
byte positions, not a newly inferred halfword type or original field layout.

Fresh six-view image-specific dossiers, source history, all four callers and
adjacent helpers were reviewed. GAME `80014314 / 0x1c0` reads the twelve
texture bytes at `800143f4..80014464`; OPEN `8001399c / 0x1c0` reads the same
offset sequence at `80013a7c..80013aec`. Both use `lbu`, add the origin to
the span before packet-byte stores, make two proven SDK calls and have two
validated state references. Preserve each 48-byte frame and restoring return
delay slot. Both stored scores are 100%; neither game-owned packet builder
is a vendored body.

Use one shared integer offset/count enum in `render_types.h`, all twenty-four
helper accesses, the GAME fade wrapper and all three OPEN descriptor arrays.
The exported OPEN array declaration uses the same extent name. Builds,
compiler checks, tests and post-edit matches remain deferred by user request.

Final verdict: both helpers now have zero inline literal occurrences. All
twenty-four byte accesses and four descriptor bounds use the planned names;
source review preserves values, load widths, addition order, alignment and
authored bytes. All 111 C-file ledgers reconcile at 5,768 occurrences. No
post-edit binary claim is made.

## Function Match Plan

Propagate a shared `KfScreenRect` with unsigned halfword `x`, `y`, `w`, `h`
fields through the GAME/OPEN FT4, OPEN F4 and OPEN G4 builders and their fade
and scene callers. Replace descriptor indices with named fields, retaining
the complete eight-byte data objects, array row counts and authored values.
The rendering inputs are read-only. The scene arrays remain mutable, and
their cursors advance by one rectangle rather than four halfwords.

The rectangle loads in all four builders are `lhu` at offsets 0, 2, 4, 6.
SDK packet coordinates retain their authentic signed halfword type. The SDK's
signed `RECT` would change the source input contract, so this descriptor uses
project unsigned boundary types. Texture descriptors retain their byte views:
only their even bytes are consumed, and this review does not resolve the odd
bytes' original ownership.

| Image / function | VA / extent | Evidence and intended change |
| --- | --- | --- |
| GAME `sprite_add_ft4` | 0x80014314 / 0x1c0 | Four unsigned halfword slots form the eight vertex-coordinate writes. Name X/Y and spans without changing their addition or packet-write order. |
| OPEN `sprite_add_ft4` | 0x8001399c / 0x1c0 | Same position contract; three callers supply complete eight-byte rows. All texture, color, selector and OT inputs retain their contracts. |
| OPEN `sprite_add_f4` | 0x80013b5c / 0x114 | Unsigned rectangle reads and sole background-row caller establish the same descriptor. |
| OPEN `sprite_add_g4` | 0x80013804 / 0x198 | Unsigned rectangle reads and the first background-row caller establish the same descriptor. Four independent color pointers remain unchanged. |
| GAME `display_play_transition` | 0x800144d4 / 0x1a0 | Copies the eight-byte rectangle at 0x80057b14 into a halfword-aligned local before passing its address. Replace the private array wrapper with the shared type. |
| OPEN `opening_fade_in` | 0x800155c0 / 0xfc | Passes the eight-byte global at 0x800372c0 directly. Type the global and pass its address. |
| OPEN `opening_scene3_run` | 0x80014804 / 0x330 | The two rectangles at 0x80035878 are eight bytes apart. At 0x800149d8..f4 retail loads Y unsigned, decrements it, stores the halfword and tests a masked visibility interval. Use the named Y field and preserve the signed interpretation before clipping. |
| OPEN `opening_ending_scroll_run` | 0x80014e28 / 0x798 | The nine panels at 0x80035888 and two backgrounds at 0x80037290 use the same layout. Retail reads final-panel Y with `lh` at 0x800154b0, updates each Y with `lhu`/`sh` at 0x800154c4..d4, and advances rows by eight bytes. Keep the explicit signed stop test. |

The complete retail bodies and caller dossiers reviewed for the preceding
[color propagation](quad-color-types.md) also establish these accesses.
The current review rechecks the rectangle loads, both scrolling loops,
curated data/call references and source history. Builders have linear CFGs,
SDK packet initialization and OT insertion calls; the scene predicates and
call order remain intact. No data owners, relocation targets or SDK types
change. This is game-specific composition around separately attributed Sony
GPU providers; it does not reconstruct vendored bodies.

Stored match reports are historical evidence only: the naming edits since the
last builds have not been compiled. Compilation, tests and post-edit matching
remain deferred until the naming pass finishes, as requested. No exact result
or banking claim follows from this source change.

The stored `build/objdiff/report.json` records 100% for seven of these functions
and 97.952675% for `opening_ending_scroll_run`. Those scores describe its stored
objects, not the current source. The earlier descriptor note also records an
older scene-3 frame difference; neither report substitutes for future matching.

## Source result

All eight functions now use the shared descriptor. The scene-3 signed
halfword cursor is replaced by `(s16)--overlay_rect->y`: the halfword still
wraps on decrement and its signed interpretation is retained before the
unsigned clipping-window conversion. Ending-panel Y decrement and clipping
remain unsigned; the final-panel stopping comparison retains its signed cast.
Both row cursors advance by one complete eight-byte rectangle.

Five data identities now name `KfScreenRect` or its array type; their claims,
linkage, alignment and initializer values are unchanged. The existing inventory
fixture follows the overlay-array declaration. No new tests or assertions.

The change replaces 48 quad component indices and removes five descriptor
array extents plus nine scene cursor/index literals: 62 fewer inline numeric
occurrences. The current [fade/quad ledger](fade-quad-literal-ledger.md) has 59
rows, and the [scene ledger](open-scene-literal-ledger.md) has 579. G4's module
now has no retained literal occurrences. Remaining rows preserve their reasons;
these source-content checks establish accounting, not binary verification.
