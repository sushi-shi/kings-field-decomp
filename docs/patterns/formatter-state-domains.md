# Formatter digit, parser and padding states

## Function Match Plan

Review GAME `debug_text.c`, OPEN `format.c` and OPEN `debug.c` as the shared
custom formatter family. Capture every function's image-qualified disassembly,
CFG, callers, callees, strings, history and starting match. Keep the separate
SDK formatting providers outside the game reconstruction count.

Use three distinct byte enums for the numeric converter's emitted-digit flag,
the format parser's text/conversion state and the space/zero padding mode.
Propagate declarations, initializers, assignments and comparisons through both
images. The leading minus sign does not mark a decimal digit as emitted.
Preserve nonzero digit/parser predicates and the padding mode's exact-zero
test, including their behavior for explicitly decoded noncanonical values.

Keep format characters as character data. Preserve the actual grammar: widths
one through eight overwrite the previous width, zero selects padding, strings
bypass padding, and unrecognized characters leave parser state unchanged.
Inspect the terminating-NUL return count and the disabled output sinks before
describing them as normal printf behavior. Check source comments against known
caller and buffer-ownership evidence; do not infer enclosing storage from a
gap in the BSS map.

Publish per-function snapshots before source edits. Force the affected units,
compare isolated before/after objects for header/type effects and independently
compare exact words with retail. Exercise valid and invalid enum uses with
Clang, reconcile every retained literal, then run modern checking, inventory,
Ruff, repository tests, whitespace and full build before the focused commit.

## Starting per-function evidence

Every row starts at strict 100%. The paired converters share signed decimal
or unsigned hexadecimal arithmetic, byte counters and nonzero emitted-digit
tests. The parser uses byte states, four helper call sites, word argument slots
and a return count including the final NUL. Padding scans with a wrapping byte
length and prepends bytes; it does not allocate an output buffer.

| Image | Function | VA / bytes | Constraint |
| --- | --- | --- | --- |
| GAME.EXE | `debug_stop` | `8003a7dc / 64` | Calls the disabled sink, then logically inverts the word marker; no admitted caller. |
| GAME.EXE | `format_int_dec` | `8003a81c / 224` | Signed input, decimal divisor sequence, optional minus and byte digit state. |
| GAME.EXE | `format_int_hex` | `8003a8fc / 140` | Unsigned input, four-bit divisor shifts, uppercase digits and byte digit state. |
| GAME.EXE | `format_pad_left` | `8003a988 / 108` | Byte length/width, backward writes and pointer return. |
| GAME.EXE | `format_vsprintf` | `8003a9f4 / 576` | Preserve parser/padding state separately from character codes and output count. |
| GAME.EXE | `debug_printf_sink` | `8003ac34 / 24` | Saves a0..a3 to caller argument slots and returns; no formatting/output call. |
| GAME.EXE | `func_8003ac4c` | `8003ac4c / 8` | Unreferenced return stub; no semantic identity beyond this behavior is established. |
| OPEN.EXE | `format_int_dec` | `8001a3fc / 224` | GAME homolog; different scratch anchor. |
| OPEN.EXE | `format_int_hex` | `8001a4dc / 140` | GAME homolog; different scratch anchor. |
| OPEN.EXE | `format_pad_left` | `8001a568 / 108` | Instruction-identical GAME homolog. |
| OPEN.EXE | `format_vsprintf` | `8001a5d4 / 576` | Same custom grammar and state transitions; no admitted external caller. |
| OPEN.EXE | `debug_printf_sink` | `8001a814 / 24` | Instruction-identical disabled variadic sink. |

The original GAME comment overstates both output and storage evidence. Its
sink does not print the supplied banner, and a BSS gap does not prove that the
original scratch allocation included the preceding bytes. Correct those claims
in the operational source comment and corresponding inventory notes while
preserving all source storage claims and evidence confidence levels.

## Implemented domains and grammar

`include/kf/debug.h` now separates `KfFormatDigitState`,
`KfFormatParserState` and `KfFormatPaddingMode`. Each uses byte storage in
the legacy build and a scoped byte enum in modern checking. Both images
propagate the types through declarations, initialization, assignment and
comparison. This replaces 38 state literals; it introduces no conversion
casts or runtime validation.

The decimal sign is emitted before the first digit and leaves the digit state
unchanged. Digit and parser predicates retain their nonzero meaning. Padding
selects spaces only for zero, and zero characters otherwise. Explicitly decoded
value two therefore remains non-leading, non-text and non-space respectively;
it is not silently normalized to a named member.

The parser is not a standard printf implementation. A percent enters or
restarts conversion state, clears zero padding and resets width to unspecified.
Repeated percent characters emit nothing. Width characters one through eight
overwrite the previous width. Decimal and hexadecimal conversions consume one
word argument and apply optional padding; string conversions consume a word
pointer and bypass padding. Unrecognized characters are copied while retaining
conversion state. Line feed becomes one carriage return. The final returned
count includes the NUL byte. No admitted external caller establishes extra
format-language constraints or proves the decimal minimum-value negation safe.

The [complete ledger](formatter-literal-ledger.md) covers 45 retained literals
in GAME, 43 in OPEN's formatter and zero in OPEN's sink. Character grammar,
radix arithmetic, string termination, ordinary counters and the unresolved
scratch extents remain explicit, with a reason for every occurrence.

## Storage and output evidence

GAME's `debug_stop` passes the banner to an empty sink and logically inverts
the initialized word flag. It neither formats nor outputs the banner through
that sink. No admitted caller proves a halt or pause meaning. Only the relevant
function and data inventory notes change; signatures, ownership, extent claims
and candidate confidence levels remain unchanged.

The GAME scratch claim starts at the numeric anchor, while padding writes
backward from it. Its existing twenty-four-byte declaration cannot prove the
enclosing allocation or the validity of those preceding writes. OPEN's current
nineteen-byte claim covers seven preceding bytes plus twelve numeric bytes
under the reviewed parser's width bound. The standalone padding helper accepts
a byte width and wraps its byte length counter, so that span is not a general
allocation guarantee. This campaign does not invent an enclosing object.

The sixteen banner bytes at GAME `80012dd4` and four initialized flag bytes at
`80057b98` agree with both objects and retail. That byte agreement does not
resolve the unit's existing section-placement failures.

## Verification

All five direct users of the shared header were force-rebuilt, including OPEN
matrix rotation and rendering. An isolated comparison compiled all 112 variants
twice from the captured working-source snapshot, applying only the three edited
source/header files to the second copy. All runtime sections, symbols and
ordered relocations are identical; only GAME debug line information changes.
All 112 live objects also agreed with their isolated counterparts at that check.
A subsequent concurrent experiment moves the OPEN TMD renderer's vertex-pointer
declaration outside its loop and restores it. Final verification records any
remaining difference separately from the formatter attribution.
The input
snapshot includes the concurrent OPEN transition and GAME interaction changes;
it is not represented as the earlier recorded HEAD's exact source tree.

The actual MIPS/O32 Clang command accepts a positive control covering all three
domains and rejects 24 invalid controls: raw initialization, assignment and
comparison, implicit encoding, and assignment/comparison between every pair
of domains. Compile-time expression controls also preserve decoded value two
and byte conversion of minus one. These controls are local build artifacts;
no size assertions or production tests were added for the enum substitutions.

All twelve functions retain 554 instruction words, nine direct calls and eleven
address materializations, compared with the previous objects, delinked targets
and raw retail, including delay slots. No function was newly banked.

Inventory, Ruff, whitespace and all 684 repository tests pass (114.495 seconds).
The initial test run exposed an expectation that required OPEN transition's old
frame mismatch after concurrent commit `96e1d64` made it exact. Commit `0bc1e6b`
updates that test to require equality while retaining its relocation checks.
Modern checking retains the same 300 errors and 65/112 passing variants. Full
`kf build` retains source-data mismatches (PSX 0/1, GAME 9/42, OPEN 2/19) and
target-relink gaps (PSX 1/1, GAME 75/77, OPEN 34/38), with six conflicting
section bases and zero artifact failures. Source unknowns remain ten lines
containing fourteen identifier tokens.

## Per-function final verdicts

| Image | Function | VA / bytes | Final verdict |
| --- | --- | --- | --- |
| GAME.EXE | `debug_stop` | `8003a7dc / 64` | 100%; words and ordered references unchanged. |
| GAME.EXE | `format_int_dec` | `8003a81c / 224` | 100%; words and ordered references unchanged. |
| GAME.EXE | `format_int_hex` | `8003a8fc / 140` | 100%; words and ordered references unchanged. |
| GAME.EXE | `format_pad_left` | `8003a988 / 108` | 100%; words and ordered references unchanged. |
| GAME.EXE | `format_vsprintf` | `8003a9f4 / 576` | 100%; words and ordered references unchanged. |
| GAME.EXE | `debug_printf_sink` | `8003ac34 / 24` | 100%; words and ordered references unchanged. |
| GAME.EXE | `func_8003ac4c` | `8003ac4c / 8` | 100%; words and ordered references unchanged; identity unresolved. |
| OPEN.EXE | `format_int_dec` | `8001a3fc / 224` | 100%; words and ordered references unchanged. |
| OPEN.EXE | `format_int_hex` | `8001a4dc / 140` | 100%; words and ordered references unchanged. |
| OPEN.EXE | `format_pad_left` | `8001a568 / 108` | 100%; words and ordered references unchanged. |
| OPEN.EXE | `format_vsprintf` | `8001a5d4 / 576` | 100%; words and ordered references unchanged. |
| OPEN.EXE | `debug_printf_sink` | `8001a814 / 24` | 100%; words and ordered references unchanged. |
