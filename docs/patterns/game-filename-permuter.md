# GAME filename builders: two exact matches with upstream decomp-permuter

## Function Match Plan and evidence snapshot

Continue the final fifteen campaign from `8be22a69` with two related GAME
functions that construct ASCII decimal filename components and call
`screen_show_image_until_input`. The user explicitly requested upstream
[decomp-permuter](https://github.com/simonlindholm/decomp-permuter), including
large searches and composed mutations. The compiler profiles remain probes;
these results do not establish the historical compiler or original TU boundary.

| GAME identity | Unit / profile | Starting strict % | Retail / starting bytes |
| --- | --- | ---: | ---: |
| `map_show_screen_image`, `0x80034d54` | `game.map_scripts` / GCC 2.5.7 O2 G0 | 88.888885 | 144 / 148 |
| `talk_show_dialogue_page`, `0x8002c9d4` | `game.save_system` / GCC 2.5.7 O2 G8 | 98.78049 | 164 / 164 |

Before candidate creation, `kf init` validates the retail image, and the six
semantic views, neighboring functions, caller/callee disassembly and source
history are refreshed under `build/upstream-permuter/evidence/`. The campaign
plan is written before the first production edit. Both functions are game
orchestration, absent from the vendor inventory; SDK descendants keep their
existing vendored ownership.

The map caller at GAME `0x800355ec` supplies group zero or one and a byte-loaded
index. The function keeps its signed index ABI and reads the floor with `lbu`.
Its 16-byte filename owns floor byte 5, group byte 8 and decimal digits 9/10.
Retail retains a pointer to byte 5, then subtracts five in the call delay slot
at `0x80034dd0`. There is one proven call and five validated address references.

The dialogue function has four call sites in `map_event_interact`:
`0x80034b50`, `0x80034c00`, `0x80034ca8` and `0x80034d24`. Its byte floor,
stage and page parameters and signed character-ID ABI remain unchanged.
The 20-byte filename owns directory digits 6/7, floor/stage 10/11, repeated
character digits 12/13 and page 14. Retail passes the array base by subtracting
six from its directory pointer in the call delay slot. There is one proven
call and seven validated address references. Both retail frames are 24 bytes;
the division precedes the stack adjustment.

The hypotheses cover quotient/remainder expression reuse, existing interior
pointers, separate versus chained assignments, and independent byte-store
ordering. No types, widths, constants, globals, SDK boundaries, compiler flags,
assembly, unused objects or volatile carriers are introduced.

## Search adapter and measured work

Upstream revision `059609d4aec73eb0650726772954e1ad575825f8` parses the
preprocessed function. The adapter reinserts only that function between the
original TU prefix and suffix and compiles the complete TU with its existing
profile. Both normalized parser baselines reproduce their original resolved
instructions. Each candidate must preserve every sibling function, including
the already partial `map_interaction_dispatch`.

Native objects, candidate hashes, strict objdiff scores and resolved words,
calls and ordered references are retained. Target objects are independently
resolved and checked against the complete retail bodies. Source variants with
different calls or references may remain search intermediates; adoption still
requires the full audit. Upstream's own heuristic score is not the exactness
criterion: it remained nonzero for the exact map candidate.

The reviewed finite templates contain 576 map combinations and 11,520 dialogue
combinations. Eight workers per function search those templates before a
planned AST-chain phase. Both searches stop for source review after finding
an exact candidate, after 42 seconds overall:

| Function | Unique completed compilations | Unique attempted sources | Search phase at exact |
| --- | ---: | ---: | --- |
| `map_show_screen_image` | 9 | 17 | finite template combinations |
| `talk_show_dialogue_page` | 196 | 202 | finite template combinations |

The attempted counts include interrupted work without a completed verdict.
Neither finite space was exhausted, and the AST-chain phase did not run for
these two functions. These successes could also have been expressed in the
repository's JSON combination runner; they are not evidence that deeper random
mutation was necessary. Upstream adds an existing C AST mutation engine and
chained exploration, while the project supplies full-TU compilation and
retail-specific validation. Reusing both avoids maintaining another C mutator.

## Kept source and exact verdicts

The map source writes the floor before the group and passes
`directory_floor - 5` to the display function. The pointer already writes the
floor field; subtracting five remains within the same array and recovers its
base. The stores affect distinct non-volatile bytes, with no intervening call.

The dialogue source writes the quotient at its two actual destinations,
retains the chained remainder stores, reorders independent byte writes, and
passes `directory_character - 6`. Removing the named quotient/remainder locals
does not change signed division, character conversion or the generated path.
The pointer remains an existing, used view of the same array. The repeated
division expression is pure and has a nonzero constant divisor.

Both candidates are rewritten in the existing source style, preserving the
shared enum types and encoding macros, then freshly compiled again:

| Function | Final strict % | Body bytes | Raw retail | Unchanged siblings |
| --- | ---: | ---: | --- | ---: |
| `map_show_screen_image` | 100 | 144 | complete equality | 15, including 14 exact |
| `talk_show_dialogue_page` | 100 | 164 | complete equality | 23, all exact |

Calls, ordered referents, immediates and all delay slots equal retail.
Candidate function hashes are
`e4cc7243d5498cbd45ae694549ee62260639e26f67398ec2255ab9991b006407`
and `2669fbd50ea76aaf232370b3905cafdc3925b5ffb667fe83895626886dba78db`.
Generated candidates, objects, adapter scripts and logs remain under `build/`.

## Verification and remaining scope

Fresh focused compilations and modern type checks pass for both units. The
listing-only `kf try` still reports existing section-symbol naming differences
and the other partial function; independent strict and resolved-word checks
provide the closure evidence above.

`kf build --reconfigure` recompiles both affected units and reports PSX 1/1,
GAME 350/362 and OPEN 107/108 exact functions: 458/471 overall, up from 456.
All thirteen vendored source controls remain exact. The full command exits
nonzero for existing data, ownership and placement gates: PSX data 1/1,
GAME 29/41, OPEN 12/20, with zero artifact failures; target relink remains
1/1, 57/64 and 34/38. Function exactness does not close those separate gates.

Ruff, both modern unit type checks, Rust tests and whitespace checks pass.
The Python suite passes 777 tests and 9,223 subtests using `python -m pytest`.
The bare `pytest` launcher initially failed collection because it omitted the
repository package path; the module invocation resolves that environment issue.
Banking selects only GAME `0x80034d54` and `0x8002c9d4`.

Thirteen functions remain in the final-fifteen goal. The preceding OPEN search
compiled 32,788 new source states and found no verified improvement over
99.917694%, seven differing words. Its chains reached depth 227. A candidate
with an exact dispatch region had changed the final blend input incorrectly;
repairing that semantic error reproduced a previously known inferior state.
No OPEN production change is retained. Large sample counts describe explored
states, not exhaustive coverage of possible C programs.

### Integration with current master

The source commit is `778b31f2`. Integration with master `1b458260` preserves
its newer NULL spellings, structured control flow and allocation-provenance
checks. Fresh compilation again proves both complete retail bodies and all
38 sibling bodies unchanged. The merged suite passes 785 tests and 9,227
subtests; Ruff, whitespace checks and `nix flake check -L` pass.

The integrated full build still reports 458/471 exact functions and zero
artifact failures. Master's stricter exported-allocation provenance checks
report data owners PSX 1/1, GAME 22/41 and OPEN 9/20; relink counts remain
1/1, 57/64 and 34/38. These separate ownership gates remain open. Neither the
merge nor the two function matches claims complete data or executable closure.
