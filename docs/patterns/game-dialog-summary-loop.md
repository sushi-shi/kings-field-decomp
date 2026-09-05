# GAME dialog summary traversal

## Function Match Plan

GAME `0x80027ee4 menu_draw_dialog_frame`, 1180 bytes, unit
`game.menu_item_detail`, existing `probe-gcc257-o2-g0`. Strict objdiff starts
at 98.623726%; the compiled body is 1188 bytes. The six semantic views,
raw compiled instructions, all 15 direct call sites, neighbors, shared types,
vendor controls and source history were inspected before this edit.

Keep `void (const KfSaveSlotSummary *, s32)`: the save-confirm caller passes
null and kind 3; seven save-panel and five load-panel sites pass their
catalog's three-record array at sp+16 and a word-sized cursor. Both prompt
sites forward the original fourth argument and the selected composite kind.
No caller consumes a result. The 80-byte frame contains the existing 24-byte
`MenuGlyphString` at sp+16 and saves ra plus s0..s8. The preceding frame-quad
helper ends with jr at 0x80027edc and its frame slot at 0x80027ee0; the next
list-interaction function starts at 0x80028380.

This is a custom UI renderer, absent from vendor/FID inventories. Its nine
AddPrim calls retain the supplied LIBGPU.H interface; no SDK body is changed.
Four glyph draws, six number-format calls and eight number draws complete
the 27-call set. The 26 validated address pairs refer only to display state,
the current FT4 cursor, and the complete menu-assets owner. No switch table,
string literal, indirect transfer or internal absolute jump is involved.

Keep the kind 0/1/2/>=3 quad selections, OT offset 4000, signed positive
test on summary word 2, all six full-word summary reads and the complete
glyph/coordinate constants. Retail walks three 24-byte summary records with
one pointer: s0 starts at the original array, reads offsets 0..20, and
advances 24 in the backedge slot. Index s2 supplies y = 65*i+30, advances
in the occupancy-test slot and is compared to 3 at the loop tail.

The current independent `row++` source produces two advancing bases,
one biased to field 5, an extra preheader move, and an extra tail increment.
Its first divergence is the incoming rows copy at +0x4c (s4 versus s8);
the null guard's slot then copies the redundant row base rather than
initializing the index. Test direct `rows[i].fields[n]` accesses and remove
the independent row iterator. This expresses one indexed record family
without changing shared widths or storage. Preserve all other source and
inventory inputs; compare raw instructions and ordered relocations after
the focused build, then use the canonical strict match. Banking requires
100%, full-build reporting and existing lint/tests without regressions.

## Exact result

The indexed accesses close this function: strict objdiff 98.623726% to
100%, compiled size 1188 to retail's 1180 bytes. All 295 raw instruction
words agree without masking, as do all 79 ordered text relocations including
their function-relative positions. The original rows pointer now occupies
s8, the null guard's delay slot initializes s2, and the loop uses exactly
one s0 record pointer with the retail member offsets and backedge increment.
The 80-byte frame, call set, glyph values, signed occupancy test, and the
HI16/LO16 addends for the complete menu-assets object remain unchanged.

Only this function and the separately documented `menu_root` result-lifetime
correction change among all 484 native report rows. The other three functions
in this unit retain their scores, including both exact quad helpers. GAME
game-function exact coverage advances from 281/362 to 282/362; OPEN remains
97/108 and PSX 1/1. The supplied AddPrim provider remains vendored and outside
the game denominator. The item-detail sibling is still partial, so the unit
is not claimed complete.

Verification: focused rebuild and canonical match, the raw instruction and
relocation audit, all-image report comparison, Ruff, all 591 existing tests
and `git diff --check` pass. Full `kf build` still exits 1 at the existing
data/ownership/placement gates: source data is 7/60 units (PSX 0/1, GAME
5/40, OPEN 2/19), SDK complete contributions are 4/4, and target relinking
is 110/116 units (1/1, 75/77, 34/38). There are no artifact failures or
banked regressions. No gate, inventory, SDK source or compiler profile was
changed. Only the strict-100% dialog function is eligible for the new bank row.
