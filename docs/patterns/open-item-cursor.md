# OPEN floor-item stream cursor

## Function Match Plan

At `4be4995`, OPEN `800197e4 item_load_floor_placements` is 97.731480%
strict, 424 compiled versus 432 retail bytes. The six semantic queries,
complete sole caller and adjacent color/pool functions, rand boundary,
source history and current GAME source homolog were inspected. There is one
proven rand call, three validated address pairs, no strings or candidate
outgoing references, and a 40-byte frame with restore in the return delay
slot. Twelve-byte placement and 24-byte runtime records, unsigned count/ID
and tile fields, signed local coordinates, 2000/100 scale factors and the
ffff sentinel are unchanged. The BIOS rand stub is separately attributed;
the two-pass resource expansion is game-owned.

Retail saves the working stream cursor in s2, clears count through a0,
then copies that cursor to a1 at `80019814` before scanning. At `80019854`
it restores the working cursor from that saved start for expansion. The C
instead retains the original parameter in a0 throughout the first pass;
the count uses a1 and both retail pointer-copy instructions are absent.

Test using the parameter as the actual advancing cursor, with an explicit
saved first-placement pointer assigned after count clearing. Keep the
existing count-loop CFG initially, and preserve every expansion operation.
This follows the observed two-pass pointer lifetimes and the existing GAME
source convention, without adding a redundant pointer, changing the shared
signature, or forcing registers. Check the first divergence and complete
instruction/relocation stream after a focused compile.

The cursor change recovers the saved start and its reset, leaving 428 bytes.
The first difference is now the first sentinel test: the saved-start copy
occupies its delay slot and the first advance follows it, whereas retail
advances in the delay slot even for the empty stream. Test the ordinary
postincrement `while (placements++->item_id != 0xffff)` form, as already
used by the GAME source homolog. This matches the observed read/advance
sequence and removes the manually split first-iteration spelling. The
unused cursor after an empty scan is restored before the second pass.

The postincrement loop recovers retail's first read/copy/compare/advance
order. One instruction is still absent: the count-address copy at
`80019824`, after which retail uses a separate address for the repeated
halfword increments. The current explicit count pointer instead spans both
the reset and loop. Test direct accesses to the actual graphics-owner count
member for both operations. Earlier GAME standalone-count trials do not
establish how this now-canonical OPEN struct member is lowered. This removes
a pointer rather than adding one and retains each memory increment, including
16-bit wrapping and possible input aliasing; it does not replace the count
with a local accumulator.

## Final verdict

Direct count-member accesses recover the count-address transfer and all
remaining instructions. The kept loader matches **100% strict objdiff** and
every one of the 432 retail bytes, including the empty-stream path, counter
loads/stores, pointer resets, rand call and all delay slots. All three
HI16/LO16 pairs resolve to the unchanged physical targets. No shared header,
signature, owner layout, compiler option or GAME source is changed.

OPEN advances **94 -> 95 / 108 exact**, leaving 13 partial functions and
none unstarted. Only this function is newly banked; the related unlit
renderer improves to 98.769230% but is not exact.

The affected objects were actually rebuilt before strict matching. Ruff,
all 401 existing repository tests and `git diff --check` pass. Full `kf build`
retains the known one OPEN / thirteen GAME data-addend failures and four
GAME historical-best deficits; no previously exact function regresses.
No test infrastructure or compiler profile is changed.
