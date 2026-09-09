# OPEN period-style C experiment

Result: 36 valid C alternatives close display_initialize at strict 100%;
opening_ending_scroll_run remains 99.917694%. OPEN advances to 107/108 exact.

## Function Match Plan (2026-09-09, before candidate edits)

User request: use another worktree, approach the two unmatched OPEN functions
as a contemporary C developer, try many implementations, and measure whether
matching improves. This is a bounded source experiment, not a promise of two
exact functions. Worktree: `/tmp/kings-field-open-era`, branch
`codex/open-era-source-experiment`, starting commit `48132a4e`.

The pinned unit profile is `probe-gcc257-o2-g0`. Keep that profile fixed to
isolate source effects; neither it nor the proposed idioms proves original
compiler or source attribution. `kf init` verified all three retail hashes.
Fresh baseline compilation gives the following strict objdiff results.

| OPEN function | Retail extent | Baseline | Evidence and source hypothesis |
| --- | --- | ---: | --- |
| `display_initialize`, `80016adc` | 472 bytes | 96.652540% | The sole caller `opening_run` forwards a word mode. Retail has a 48-byte frame, two mode branches, 18 direct call instructions, two internal jumps and 16 ordered address pairs. SDK parameters include the fifth argument on the stack; DRAWENV flags/RGB are bytes. Try ordinary SDK setup using literals, direct array indexing, whole-object pointers, returned environment pointers and conventional mode branches. |
| `opening_ending_scroll_run`, `80014e28` | 1944 bytes | 99.917694% | The sole caller follows `opening_ending_scene_run` without arguments. Retail has a 264-byte frame, an infinite frame loop plus detached return epilogue, 40 direct calls, eight internal jumps and 31 ordered address pairs. Short phase/blend/tick values, word countdowns, nine word texture/CLUT entries and signed/unsigned rectangle tests constrain the model. Try ordinary if/else state machines, shared transition code, countdown expressions and indexed/pointer for loops. |

Both functions have no referenced strings and no candidate outgoing semantic
references. The raw CFG includes all branch/call/return delay slots and load
delays. The six required semantic views, sole caller, adjacent functions and
SDK SetDefDrawEnv/SetDefDispEnv bodies were read; the latter return their input
object pointers. LIBGPU.H supplies their authentic pointer-return prototypes,
byte flag fields and setRGB0. Neither target appears in functions_vendored.tsv;
game-specific mode/scene orchestration around the separately identified SDK
callees is the negative control. Shared owner and curated names remain intact.
History includes `f1aea601` and the existing sentinel, display-pointer,
inline/aggregate and typedef-control reports. Repeated controls, if useful
for comparison, will be labelled; trial count is not a novelty claim.

Start with independent source ideas, inspect the first real divergence and
ordered referents after each compile, then make a small number of justified
compositions. Use no register permutations, artificial carriers, volatile,
assembly, profile changes or invented data ownership. Preserve each complete
candidate source, object, strict score and resolved raw audit under
`build/era/`. An appealing source can be a negative result. Keep a production
change only if it improves the complete evidence without regressing an exact
sibling; bank only strict 100%. Finish with focused matches, full build,
repository lint/tests and diff checks, and record the two final verdicts.

The initial full build exits 1 on existing data/ownership/section-placement
checks (OPEN target relink 32/38, GAME 63/77). This is a baseline failure,
not a successful whole-image build.

## Follow-up plan: separate display flag assignments

The per-buffer setup D07 has a lower strict score (95.796610%) but restores
retail's 472 bytes and complete 48-byte frame; every instruction outside
the initialization stores agrees. Its eleven unequal words are confined to
`+c4..+100`. This is stronger source evidence than its score suggests.
The baseline chains both DTD assignments, while the per-buffer form uses
separate ordinary statements. Next, preserve D07's separate assignments and
restore the retail-supported field order. Also test splitting the baseline's
chain by itself, and its two-pointer variant separately. These are changes
to real stores, not new carrier values or declaration permutations.

The first generated panel-loop candidates E04/E05/E06 accidentally retained
the old loop tail because the generator matched a comparison rather than
the following assignment. They are invalid experiments, excluded from the
result count. E07 did not compile for the same extraction error. The fixed
generator checks the complete tail replacement; E04b/E05b/E06b/E07b are the
actual tests. The original artifacts remain available to explain the audit.

D15 (separate DTD assignments plus SDK RGB macros) restores all 16 ordered
referents and the retail frame/extent, but its retained address is the second
DTD instead of the first: 13 raw words differ, beginning at +c4. D13, identical
except for ordinary RGB statements instead of the SDK comma-expression macro,
does not preserve that useful addressing. Next test the authentic RGB macros
with the original chained DTD assignment and direct arrays, without the
baseline's explicit pointer locals. This combines observed complementary
source facts; the macro is exactly the supplied LIBGPU.H definition.

D17 is strict **100%**: every resolved instruction, all 18 calls and all 16
address pairs equal retail. Three sibling functions remain exact. It uses
direct SDK array access, the original chained DTD assignment, and two actual
setRGB0 invocations. The explicit DRAWENV pointer locals are removed.

## Ending expression follow-up plan

The display result makes real SDK expression boundaries worth testing in the
ending. LIBGPU.H also supplies setVector; test it on the existing three-field
position snapshot. Its natural XYZ assignment order differs from the retained
YXZ order, so audit that explicitly. The ending's colors are CVECTOR, with
r/g/b rather than r0/g0/b0: do not cast them to a different SDK object merely
to use setRGB0. Instead test ordinary comma-expression grouping of the actual
color clear, brightness update, phase/reset pair, volume decrement/submission,
position snapshot, and two gradient endpoint copies, independently. These
are candidate source forms, not claims that an absent SDK macro existed.

## Results

There are 36 valid alternatives (17 display, 19 ending), plus two unchanged
controls. This count excludes the four invalid generator outputs described
above. Each valid alternative has a retained complete C source, object,
source diff, ordered relocation listing, resolved word/call/address audit
and strict objdiff score in `build/era/<ID>/`. IDs with a `b` suffix are the
corrected panel/cadence experiments.

| Display ID | Source idea | Strict % | Bytes |
| --- | --- | ---: | ---: |
| D01 | Direct arrays, dimension locals retained | 90.474580 | 480 |
| D02 | Direct arrays, literal dimensions | 85.661020 | 476 |
| D03 | One whole-DRAWENV array pointer | 75.508480 | 444 |
| D04 | Independent late submission pointers | 92.177960 | 484 |
| D05 | Actual SDK setter return pointers | 80.550850 | 440 |
| D06 | Conditional ResetGraph argument | 76.177960 | 452 |
| D07 | Per-buffer flags and SDK RGB macros | 95.796610 | 472 |
| D08 | Two-buffer initialization loop | 73.169495 | 456 |
| D09 | Activation switch | 96.652540 | 476 |
| D10 | Normal-mode-first if/else | 86.059320 | 476 |
| D11 | RGB macros with retained explicit pointers | 96.652540 | 476 |
| D12 | Submission pointer cursor | 93.601690 | 484 |
| D13 | Separate DTD stores, direct arrays | 87.067795 | 480 |
| D14 | Separate DTD stores, retained pointers | 93.245766 | 476 |
| D15 | Separate DTD stores plus RGB macros | 96.372880 | 472 |
| D16 | Separate DTD stores in buffer order | 86.991520 | 480 |
| **D17** | **Direct arrays, chained DTD, actual SDK RGB macros** | **100.000000** | **472** |

D09 and D11 produce the same resolved body as the starting source. D06
merges the two static ResetGraph sites into one; D10 reverses the order of
the mutually exclusive call blocks. Both preserve the intended dynamic
mode behavior but fail the retail static call sequence. The remaining
display alternatives preserve all 18 ordered calls. D15 and D17 alone
preserve all 16 ordered explicit address referents; all three neighboring
functions remain raw exact in every valid display alternative.

| Ending ID | Source idea | Strict % | Bytes |
| --- | --- | ---: | ---: |
| E01 | Lighting if/else | 98.600820 | 1936 |
| E02 | Sequence if/else | 98.557610 | 1936 |
| E03 | Increment consecutive lighting phases | 96.868310 | 1952 |
| E04b | Index/pointer for loop | 99.917694 | 1944 |
| E05b | Indexed for loop, derive panel each iteration | 94.191360 | 1904 |
| E06b | Pointer-end loop bound | 98.236626 | 1956 |
| E07b | Four-state circular tick arithmetic | 97.930040 | 1912 |
| E08 | One named stereo-volume argument | 99.917694 | 1944 |
| E09 | Postdecrement delay through zero | 99.372430 | 1944 |
| E10 | Shared lighting completion rule | 94.450615 | 1940 |
| E11 | Stop advancing completed background fade | 98.981480 | 1960 |
| E12 | while(1) frame driver | 99.917694 | 1944 |
| E13 | Authentic setVector snapshot | 99.917694 | 1944 |
| E14 | Comma-expression phase/reset pair | 99.917694 | 1944 |
| E15 | Comma-expression color clear | 99.917694 | 1944 |
| E16 | Comma-expression brightness update | 99.917694 | 1944 |
| E17 | Comma-expression volume update/submission | 99.917694 | 1944 |
| E18 | Comma-expression snapshot in original order | 99.917694 | 1944 |
| E19 | Comma-expression gradient endpoint copies | 99.917694 | 1944 |

All ending alternatives preserve the 40 ordered calls and six raw-exact
neighbors. Only E06b changes the ordered explicit address sequence, adding
the panel-array end. E04b, E08 and E12–E19 are completely resolved-equal to
the original. E05b changes the frame to 248 bytes; every other valid ending
alternative retains 264 bytes. E09 retains the seven dispatch differences
and adds four countdown words; it does not recover the retail decrement/test
form. E01/E02 change branch arrangement and remove eight bytes. The ending
source is unchanged, with final strict score 99.917694% and differences at
`+398,+39c,+3a0,+3a4,+3ac,+414,+418`. No new compiler attribution follows.

## What the display control establishes

The actual Release 2.5 LIBGPU.H setRGB0 macro expands to a single comma
expression, not three C expression statements:

```c
#define setRGB0(p, r, g, b) \
    (p)->r0 = r, (p)->g0 = g, (p)->b0 = b
```

This is the header's expression structure with parameter names abbreviated.
The pinned header identifies revision 3.51 (1994-11-21), SHA-256
`a33286ee6a0a41835a83d3a0cb053d6808b9772671d3d5f6c48c0267928a9a3b`.
The kept source invokes the SDK macro directly on each actual DRAWENV.
Its controlled comparison is unusually useful:

| Context | Separate RGB statements | SDK comma-expression macro |
| --- | ---: | ---: |
| Retained explicit draw pointers | 96.652540% | 96.652540% |
| Direct arrays, chained DTD assignment | 90.474580% | **100%** |
| Direct arrays, separate DTD assignments | 87.067795% | 96.372880% |

Thus testing the macro in only the previously retained pointer context hid
its effect. D17 recovers the first DTD address lifetime without introducing
a member pointer, cast, assembly, forced local, or extra comparison. The
complete raw equality establishes this reconstruction under the fixed probe;
it does not prove that this exact header/compiler built the retail game.

The lower-scoring per-buffer attempt D07 was useful because its mismatches
were localized to actual store order. Following that source evidence led to
D17. Restoring only the highest score after each attempt would have missed
that connection.

## Verification and final verdicts

The accepted source was copied into src/open/render_init.c and rebuilt with
both `kf try --unit open.render_init` and `kf match --unit open.render_init`.
All four function listings match. A separate audit of the current configured
objects requires strict 100% and full resolved retail-word equality for all
four display-unit functions and the six exact ending-unit neighbors. The
ending controller must equal its original complete word/call/address audit
at 99.917694%. Both units' data/rodata/BSS contents equal the starting controls.

The existing display test now requires retail's 472-byte extent and its
ordered address sequence, replacing the prior 476-byte/extra-address
allowances. Its six mode paths still check arguments, ordered writes and
frame restoration; the argument/field corruption negative controls remain.
All three focused display tests pass. Ruff passes, unittest discovery passes
739 tests with nine optional skips, and git diff --check passes. The initial
suite run identified the obsolete extent assertion; the full rerun is green.

Full `kf build` was run after the focused rebuild. It exits 1 with the same
pre-existing data/ownership/section-placement failures as the initial run:
OPEN target relink 32/38, GAME 63/77, PSX 1/1. No whole-image closure is claimed.
Function counts are OPEN 107/108 (up one), GAME 340/362 and PSX 1/1 unchanged;
overall 448/471. No profile, retail inventory, shared types or linker rules
changed, so no flake/tooling gate is applicable.

Final display verdict: exact reconstruction through authentic SDK source
forms, banked alone with `kf bank --function open:0x80016adc`; the ledger diff
contains exactly that function row. Final ending verdict: no improvement
among the 19 valid alternatives; retain the existing source and seven-word
unattributed codegen residue. The original task's two-function experiment is
complete; this does not claim that every OPEN function is exact.

## Master integration

Integrated from experiment commit `94c2541d` onto master after `ad214ccb`,
preserving the intervening GAME match and Sony-header helper integration.
The source, focused test and selected bank row retain the verified experiment
contents. The clean committed-source totals become OPEN 107/108, GAME 341/362,
PSX 1/1, overall 449/471. README totals were regenerated in a clean checkout
of that state, excluding a concurrent uncommitted GAME experiment.

On master, kf try rebuilt all four display-unit functions with identical
listings, and the configured strict matches are all 100%. Ruff and the three
focused display tests pass. Full unittest discovery ran 739 tests with one
unrelated failure: a concurrent GAME identity edit promoted the floor-load
wrapper from address-only to supported, while its old inventory assertion
still required address-only. The same inventory test plus all three display
tests pass in the clean integration checkout. The unrelated edits remain
outside this commit.

Full kf build ran both on master and in the clean integration checkout; the
known data/ownership/section-placement checks still fail, with target relink
counts OPEN 32/38, GAME 63/77 and PSX 1/1. The clean checkout freshly compiled
all units and confirms the four strict-exact display functions. Integration
logs are retained under build/open-era-integration on master and
build/integration in /tmp/kings-field-open-master-check.
