# GAME world-collision argument traces

## Function Match Plan

Continue from `440b20c`, following the effect helper's sole callee. GAME
`collision_query_world`, `0x8001a5ac`, owns 1284 retail bytes in `game.collision`.
Its fresh native/traced whole-ELF-identical baseline is **97.943924%**, 1276
bytes, source hash
`e678a825bbfad01ff80d4c9044e27b95bcb9b47a4db60b2e3d58eb31529e9d5d`.
The unchanged compiler fingerprint is
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
Generated evidence is under `build/gcc257/game-world-collision-traces/`.

All six GAME semantic views, the complete retail body/CFG and fresh raw
candidate listing, all sixteen direct callers, five callee contracts,
adjacent occupancy updater and checked allocator, shared types/data,
source history and prior collision evidence were reviewed. The current
signature is five signed word coordinate/radius/height inputs and unsigned
word flags. The return is an encoded word or `-1`; the optional target output
contains complete VECTOR/SVECTOR copies and a halfword radius. Preserve the
unused six output bytes. Game grid and pool policy excludes an SDK body;
the prior vendor/FID/signature negative controls remain applicable.

Retail has a 64-byte frame saving all eight saved registers and `$ra`, two
checked signed divisions, 49 CFG blocks, eight internal jumps, five direct
calls, and 21 address pairs. There are no strings or indirect transfers.
The call set and ordered numeric referents agree. Direct calls are proven;
address and internal-jump evidence is validated, with no candidate outgoing
references. The 510 initialized signed height bytes and their data owner
remain unchanged.

The first mismatch is the saved X register at +4. A concrete later entry
difference is the sixth stack argument: retail loads it with `lw v1,84(sp)`,
waits one instruction, copies it to `s0`, and tests bit zero through `v1`.
The baseline loads the flags directly into `s5` earlier and omits the copy
and wait. A prior separate word snapshot selected `s0` but still lacked the
copy; do not repeat that control or name a register-allocation wall.

First hypothesis: test whether the source flags parameter is an unsigned
halfword passed in the word-sized O32 slot. Every decoded consumer uses only
bits 0..15, and the sixteen caller sites pass zero-extended masks from
`0x21`, `0x60`, `0x61`, `0x71`, `0xe1`, `0x881`, `0x8040`, and `0x8060`.
The values with bit 15 set are materialized with `ori`, which contradicts a
sign-extended signed-halfword call contract. These facts support testing
`u16`; they do not prove it instead of the existing `u32` declaration.

Use an isolated, consistent header/source pair and a small stack-argument
control before editing the shared production interface. Keep all flag
operations, body statements, callers, return forms, constants, data and
compiler flags fixed. Inspect whether expansion retains a promoted incoming
word plus a halfword value and whether the final load/copy matches retail.
Reject the type if it introduces an unsupported halfword load, narrowing,
or shift. Any retained interface must be propagated through the curated
identity and shared header, preserve all callers and banked functions, pass
the focused raw comparison and full build, and reach strict 100% for banking.

The `u16` parameter trial gives **96.012460%**, 1280 bytes. It retains the
word load, adds the real halfword-value copy, and recovers every saved input
register (`s2` X, `s3` Z, `s5` Y, `s6` radius, `s7` height, `s0` flags).
The load is still early, through `a0`, and changes the preceding division and
cell-arithmetic scratch registers. It is an exploratory state, not a retained
interface change. The signed shifts, call set and 21 address pairs remain
correct; there is no unsupported `lhu` load of the stack argument.

Refine ownership of that same halfword value: keep the original word-sized
API parameter and initialize a `u16 query_flags` after the real cell
calculation. Use it for the actual query-bit operations. Retail's flags copy
occurs after the cell row offset is formed, whereas a halfword parameter
introduces its conversion with the incoming arguments. This tests conversion
placement, not an extra value held alive artificially. It differs from the
previous rejected word snapshot because the independently observed halfword
copy now survives. Preserve all high-bit-insensitive outcomes and reject the
form if it does not explain the later stack-load/copy sequence.

## Exact result and measured trace facts

The local halfword form reaches **100.000000%**, 1284 bytes. The native,
uninstrumented host-debug and traced compilers emit identical whole ELF
objects for the baseline, formal-halfword control and retained local-halfword
source. The retained source hash is
`d93e2b1c72ed08220df9d2f8f508e3b9e660c6dbbd1ea5a0fcbbd48d900058c2`.
No shared declaration or curated identity changes: the O32 flags parameter
remains `u32`, while the actual bit consumers use `u16 query_flags`.

The three expansion snapshots distinguish value width from conversion
placement:

| Source | Incoming flags | Halfword value | Placement |
| --- | --- | --- | --- |
| Word baseline | UID 14, `r76:SI = mem(arg+20)` | None | All bit consumers use the word |
| Halfword formal | UID 14, `r77:SI = mem(arg+20)` | UID 16, `r76:HI = subreg(r77:SI)` | Before both divisions |
| Local halfword | UID 14, `r76:SI = mem(arg+20)` | UID 42, `r96:HI = subreg(r76:SI)` | After cell calculation UID 39 |

The baseline's first test directly masks `r76:SI`. In the local version,
CSE may use that incoming word for the first low-bit test while the separate
halfword value serves subsequent consumers. The halfword copy remains in
combine. At first scheduling, the formal-halfword word load lands between
the row multiply's shift-by-three and next addition; the local-halfword load
lands after the final shift-by-two. Reload preserves these measured
placements, selecting `a0` for the former incoming word and `v1` for the
latter. These are observed pass results, not an inferred historical compiler
identity or a general allocation rule.

Three small controls retain five live coordinate/radius/height arguments,
the sixth stack flags argument, cell arithmetic, a real call and post-call
flag consumers. Word, formal-halfword and local-halfword variants reproduce
the respective expansion ownership. The local control has cell UID 39 and
halfword conversion UID 42 (`r92:HI = subreg(r76:SI)`). All three controls
also pass whole-ELF native/debug/traced parity. They validate the interpretation
of the word argument and halfword value; they do not establish a universal
hard-register assignment.

The resulting production sequence is exactly retail:

```text
8001a65c  sll   v0,v0,2
8001a660  lw    v1,84(sp)
8001a664  nop
8001a668  move  s0,v1
8001a66c  andi  v1,v1,1
8001a670  bnez  v1,8001a738
8001a674  addu  s4,a0,v0
```

The conversion adds no unsupported stack halfword load or truncation
instruction. All consumers ignore bits above 15, including the shifted
`0xf000` rejection mask, so narrowing preserves the result for every possible
incoming word, beyond the sixteen observed caller masks. This is the real
query option value used throughout the body, not a carrier added only to
keep a register alive.

## Verification and final verdict

The focused production match recompiled `game.collision` and reports strict
100%. An independent resolved-word audit compares all **321 instructions**
against both the delinked target and hash-identical retail. This includes the
64-byte frame, saved registers, checked divisions, every branch/jump and its
delay slot, and the final return. All five ordered calls and 21 ordered
address pairs remain unchanged. The complete 510-byte initialized height
table agrees across baseline, retained source, target and retail.

GAME moves from **336/362 to 337/362 exact**; OPEN remains **106/108** and
PSX **1/1**. The 13 vendored source controls remain exact. No other function
source or build profile changes, and no banked function regresses.

Scoped type checking, Ruff, all **733 repository tests** (113.798 seconds),
and `git diff --check` pass. The full build retains its existing
data/ownership failures: data contributions match at PSX 1/1, GAME 11/41,
OPEN 3/19; target relinks verify 1/1, 75/77 and 34/38 respectively. There
are no artifact failures. The collision height payload is exact, but its
existing section-alignment placement issue remains outside this function
closure. The literal ledger preserves all 270 entries and updates only the
ten affected line references and four query-variable spellings.
