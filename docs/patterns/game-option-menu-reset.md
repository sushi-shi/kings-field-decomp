# GAME option-menu selection reset

## Function Match Plan

GAME `0x800236ac menu_option_root`, 556 bytes, `game.menu_panels`, existing
`probe-gcc257-o2-g0`. At `ded8b15`, strict objdiff is 98.848920%; compiled
text is 560 bytes. The six semantic views, caller, adjacent functions,
callee interfaces, shared types, raw object streams and source history were
inspected before editing.

The signature remains `void (void)`. Its only direct caller is `menu_root`
at `0x80022474`, with a nop delay slot and no consumed result. Retail saves
`ra/s5/s4/s3/s2/s1/s0` in a 48-byte frame. Cursor, confirm flag, input and
previous input are full-width locals; the result starts at -99 and the
pending selection at -1. The menu draws nine rows, wraps the cursor through
0..8, and uses button masks 0x1000, 0x4000, 0x20 and 0x40 with edge tests.
The 0x15 head-armour ID is an unsigned byte at `player_state+0x91`.

There are twenty proven direct calls: frame-begin and name-list draw three
times each, window draw three times, frame presentation and PadRead twice
each, input sound five times, and the equipment/spell selection handlers
once each. Their arguments, return use and order already agree. PadRead is
the revisioned Sony LIBETC PAD provider, with archive/FID/retail marker
evidence and the existing K&R interface. This menu composition is absent
from the vendor/FID inventories; no SDK body is reconstructed.

The eight-row switch table is the unit's `RODATA(0x800122f0, 0x20)` claim,
not eight new source globals. The semantic navigator retains its pointer
rows as candidates and reports the `jr v0` at 0x8002377c as unresolved;
the table bytes and index/load chain must be checked independently. The two
address pairs (table and player field), six internal absolute jumps, and
twenty calls form the ordered relocation contract. There are no strings in
this function. The preceding `menu_magic_panel` ends with a return at
0x800236a4 and its frame-restore slot at 0x800236a8; the following
`menu_equip_select` starts at 0x800238d8 and is also a callee.

The initial reconstruction in `62ed6c1` placed `selection = -1` after the
exit guard, as the current source still does. Retail's guard at +0x118
(`bne s5,v0`) clears selection in its delay slot at +0x11c, on both paths.
Compiled C has a nop there and a separate reset after the guard, leaving
one extra instruction and shifting the remaining block positions.

Move the existing reset immediately before `if (result != -99) return;`.
It follows dispatch of the old selection and precedes the next input phase;
no subsequent use on the returning path exists. Keep all constants, types,
calls, switch cases, loop structure and inventory inputs unchanged. This is
a decoded state-update ordering correction, not a fake live value. Require
a focused rebuild, strict canonical comparison, raw words and ordered
relocations, all-image regression audit, full build and existing lint/tests
before banking this function alone.

## Final verdict

The single ordering correction reaches strict **100%**. Both bodies contain
139 words; 133 literal words agree and the six internal `R_MIPS_26 .text`
jumps resolve to the same function-relative +0x118, +0x1e4 and +0x54 targets.
Their raw section offsets differ by 20 bytes because the preceding partial
magic panel is longer in the compiled object. All thirty ordered text
relocations agree in offset, kind and symbol. The reset is now `li s2,-1`
in the +0x11c branch slot, and the extra instruction is gone.

The retail table words target +0xfc, +0x10c, +0xfc, +0xfc, +0xfc, +0xd8,
+0xd8 and +0xfc, matching all eight case destinations. Both objects retain
eight ordered `R_MIPS_32 .text` rows and 32-byte tables. Their addends still
differ by the preceding function's 20-byte displacement: strict whole-unit
data comparison remains non-exact. This is function closure, not TU/data
closure; no candidate pointer rows are promoted or data mismatches masked.

Only this function's strict score changes among all 484 native function
rows. GAME advances **280/362 to 281/362 exact**, OPEN stays 97/108 and PSX
1/1; all thirteen vendor controls are preserved. The other function in
`game.menu_panels`, `menu_magic_panel`, remains partial at 95.897590%.

Ruff, all **591 existing tests** and diff checks pass. Focused compilation
and the canonical match refresh the actual objects; full `kf build` runs
the all-image gates and still exits nonzero on the existing explicit
data/ownership/placement gaps (7/60 source-data owners, 4/4 SDK contributions,
110/116 target relinks). Only GAME `0x800236ac` is eligible for banking.
