# GAME status-message return traces

## Function Match Plan

At `6b14629`, GAME `8002c510 memory_card_show_status_message`, 208 retail
bytes, remains strict **97.980770%**, 212 compiled bytes. The selected
`game.save_system` source SHA-256 is
`e14173a0b9d6ea9c59f8a0943043cef086e7fc8c8718c4849b7aa0706506fd1a`;
fresh native/traced whole ELF parity holds with compiler fingerprint
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
Evidence is under `build/gcc257/game-status-result-traces/`.

The six GAME views, complete retail and candidate, all five direct caller
windows, adjacent initializer and image-loader bodies, source history and
the [previous return controls](game-save-ui-matches.md) were read. Callers
pass card/file statuses or literal 12 and ignore the result. Preserve the
signed-halfword status parameter, typed storage, halfword selected/default
message conversion and signed word result. The loader returns zero or one,
but keep the mapper's forwarding behavior for every result other than one.
Game status-to-image policy excludes a vendored body; the exact loader and
its SDK file/image interfaces remain unchanged.

Retail has a 24-byte frame with ra at +16, one direct call, one validated
table-address pair and twelve validated internal jumps. Fifteen switch
pointer rows remain candidates, and the navigator's indirect jump remains
unresolved. No strings belong to this function. The complete delinked target
was checked against all 52 retail words, including branch, jump and return
delay slots. All 22 exact siblings and the partial dialogue body retain
their complete baseline words and ordered referents.

All words through the loader call's delay slot agree. The first difference
at +0xb0 is an extra `move a0,v0`; retail immediately loads comparison value
one into v1, branches over `li v0,-1` and returns. By CSE1, the return copy
is ahead of the comparison while the loader-result pseudo remains live for
the branch. Reload gives that pseudo a0; final delay processing removes the
copy back to v0 but keeps the initial copy. The selected message is a
distinct HI value, so merging its domain with the SI loader result is not
the source hypothesis to pursue.

Test an explicit returned outcome, initialized to -1 after the existing
loader call. When the unchanged `result != 1` predicate holds, assign the
actual loader result to this outcome, then return it once. This represents
the decoded exception/forwarding join. It differs from overwriting the
loader result itself, a conditional expression, or changing the return-arm
order, all of which were already rejected. Both locals have consumed roles;
no extra call, fake lifetime, forced register or width conversion is allowed.

Inspect the first changed RTL and every raw word, including the original
two narrowing pairs, ordered table targets and call destination. Require
native/debug/traced parity and a small control before claiming a compiler
mechanism. Keep only supported source progress; strict 100%, raw retail
agreement, focused/full builds and repository verification are required
before banking. Rejected candidates leave production and all banked rows
unchanged.

The explicit outcome is rejected at **91.730770%**, 212 bytes. It copies
the loader result to v1, puts -1 in a0 in an equality-branch delay slot,
then copies a0 into v0 at the epilogue. The table, call and all 23 siblings
are unchanged. It does not explain retail's direct use of the call result.

The five caller windows provide a separate return-contract question: none
uses this function's returned register. Retail explicitly writes only the
exceptional -1 after the loader; the other path leaves v0 unchanged. A
decompiler's inferred forwarding return is therefore not proof of an
explicit C return statement. Test the original-C possibility of a non-void
function with only the exceptional `return -1` and a fall-through success
path. Keep the result comparison, all earlier source and the signature
unchanged. This is an incomplete return contract, not a promise to forward
arbitrary loader results in C; it is admissible only if the complete caller
audit confirms the value is unused and raw generated behavior agrees.
Record any diagnostic and do not hide it with an unreachable intrinsic,
compiler flag, fake expression or invented return value.

The success-fallthrough control reaches strict **100%**, 208 bytes. All
52 resolved instruction words equal the independently verified retail target;
the table address, twelve internal jumps and image-loader call also agree.
All 23 sibling bodies retain their baseline, including 22 exact functions.
This supports an incomplete original C return contract. No caller may be
given a new promise that the successful C path returns the loader result.
Complete parity, diagnostics and a small forwarding/fallthrough control
before applying this result; record the successful path explicitly in the
identity note and source comment, without suppressing its warning.

## Controls and retained contract

The baseline, rejected explicit outcome and exact fallthrough each have
native/debug/traced whole ELF parity. Repeating the exact trial reproduces
both its complete ELF and JSONL trace byte-for-byte. Two minimal C controls
also have three-compiler parity. With a signed-halfword message argument and
one external loader call, the forwarding form is 56 bytes and the fallthrough
form 52; their tails reproduce the real function's extra `move a0,v0` and
its removal respectively.

In the real fallthrough source, COMBINE deletes UID 146, the loader result's
copy to pseudo 75, and UID 150 compares the fixed return register directly.
The forwarding source still needs that pseudo for its explicit return.
The small pair reproduces this at its UID 19/23 copy/comparison. The result
therefore follows from the source return form before hard-register allocation;
it does not establish a historical allocator or scheduler limitation.

The retained C preserves the exceptional return and leaves the successful
path without an explicit value. All five decoded direct callers discard the
result, and no incoming function-pointer reference is established. This
incomplete C contract must stay visible: using its successful result is not
supported. The pinned compiler's output reproduces retail's physical v0
contents on both paths, but that does not add a portable C forwarding promise.

Clang's retail-C syntax check succeeds with one new, unsuppressed
`-Wreturn-type` warning. The modern C++ checking view retains the same 27
pre-existing errors for this unit, with no changed error diagnostics.
No unreachable intrinsic, warning suppression or profile change is used.

## Production verification

The final commented source has SHA-256
`7bca59ec6a1dcc6f46134e88244ff812b32362d99b308944ca2a5d6b17b66ddc`
and fresh native/debug/traced whole ELF parity. The focused match performs
a real `game.save_system` compile and records **100%**, 208 bytes. Its
production object equals the independent native compile byte-for-byte.

The raw audit verifies all 52 mapper instructions, all fifteen switch-table
pointer values and every transfer's delay slot against GAME retail. The
wrong-callee control changes exactly its single call word. All 23 exact unit
functions reproduce **1,623** retail instruction words; the dialogue body's
41 words remain unchanged at 98.780490%. The existing graphics-owner probe
now requires the mapper's complete retail body instead of treating it as a
partial function. Table evidence tiers and inventories are not promoted.

Ruff, all 734 repository tests and `git diff --check` pass. The existing
literal ledger retains its occurrence count and updates the two return
expressions' interpretation and affected line references. No SDK body or
production compiler/profile changes.

The required full build preserves all banked functions and all thirteen exact
vendored source controls, while retaining the existing data/ownership/placement
failures. Data comparisons pass for PSX 1/1, GAME 11/41 and OPEN 3/19 units;
target relinking verifies 1/1, 75/77 and 34/38 respectively. There are no
comparison artifact failures. Exact counts advance to GAME **338/362**,
OPEN **106/108**, PSX **1/1**, or **445/471** overall. Twenty-four GAME and
two OPEN functions still require strict closure. Only GAME `8002c510` is
eligible for banking from this campaign.

`kf bank --function game:0x8002c510` banks exactly that one verified row.
The remaining partials and generated evidence are not banked.
