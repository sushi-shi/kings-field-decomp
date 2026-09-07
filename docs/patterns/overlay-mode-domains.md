# GAME exit status and OPEN entry mode

## Function Match Plan

Separate the GAME runtime exit status from the OPEN entry/display mode.
`KfGameExitCode` retains the existing unsigned word storage of `game_exit_code`:
zero keeps the GAME loop running, one requests the intro, and 0xfe requests the
ending. `KfOpenMode` retains the existing signed word parameter representation
and names intro/ending. These are related protocol values, but the GAME-only
zero sentinel should not be implicitly accepted as an OPEN mode.

Propagate the exit domain through the global declaration, curated data type,
both producers and the main-loop predicate. Propagate OPEN mode through
`opening_run` and `display_initialize`. Preserve authentic BIOS entry arguments
and Exec declarations: GAME explicitly encodes its result word, PSX copies raw
words and explicitly encodes its initial intro request, and OPEN explicitly
decodes its incoming word. Do not add validation, narrow a word, change the
overlay execution order or rename unrelated input actions.

Capture six image-qualified semantic views, source history and forced baseline
objects for the seven affected functions. Inspect their callers/callees,
adjacent functions and raw mode loads/stores. Require isolated before/after
runtime equivalence across all 112 source/image variants, unchanged focused
scores and ordered referents, and compiler rejection of raw/cross-domain field
and API uses. Complete the OPEN renderer initializer's retained-literal ledger
while checking its existing palette data, coordinate indices and SDK flags.

## Evidence snapshots

Calls described here are decoded `proven` edges; HI16/LO16 data references are
`validated` curated referents. These orchestration bodies are game code around
the separately attributed Sony/GNU startup, BIOS, GPU, audio and memory callees;
those vendored bodies are not counted as reconstruction progress.

| Image / function / VA / extent | Baseline | Domain evidence |
| --- | ---: | --- |
| PSX `main`, `0x80010028`, `0xd0` | 100% | Initializes result word at stack +84 with one, copies it to request +80 at the loop head, and executes OPEN then GAME with the same two-word buffer. Load's success value one and Exec's argc one have separate meanings. |
| GAME `main`, `0x8001428c`, `0x88` | 100% | Retains incoming a1 in s0; after `game_main_loop`, loads the complete word at `0x800958f8` and stores it at s0+4. No status translation or narrowing occurs. |
| GAME `game_main_loop`, `0x800146b8`, `0x2e4` | 100% | Stores zero at `0x80014818`, loads the same global and exits on any nonzero value at `0x80014840`. The successful warp path stores 0xfe at `0x80014930` before the ending transition/shutdown sequence. |
| GAME `player_update`, `0x80018880`, `0x1a1c` | 96.94554% | Menu result -2 selects a store of one at `0x80018998`, followed by the function exit. The menu-result and overlay-exit domains remain distinct. |
| OPEN `main`, `0x80013758`, `0x6c` | 100% | Reads the full incoming request word at `0x800137a8` and forwards it as a0 to `opening_run`. Startup storage/heap addresses remain their existing numeric boundary constants. |
| OPEN `opening_run`, `0x800156bc`, `0x214` | 100% | Retains mode across initialization, forwards it unchanged to display initialization, compares with one and 0xfe, and dispatches intro/ending respectively. All other values reach common fade/audio/GPU/PAD cleanup without either scene branch. Its three path references remain B0\\L0., B0\\MIX0. and B0\\MIX3. |
| OPEN `display_initialize`, `0x80016adc`, `0x1d8` | 92.17796% | Only 0xfe selects the special reset/presentation path; all other values take the normal path. The original two mode comparisons and all SDK calls remain. There are no strings in this function. |

The 32-bit representation is directly evidenced by the loads, stores and O32
forwarding. The admitted nonnegative values do not independently establish
signedness; the types preserve the respective existing u32 global and s32
parameter contracts. The source still accepts explicitly decoded unlisted
values, including the existing nonzero exit predicate and OPEN fallback path.

## Verification

Both enums are implemented through the existing C++20 compatibility layer.
The GAME global now has the exit-status type in its shared declaration and
curated data identity. GAME's intro/ending producers use exit-status constants,
and its nonzero predicate remains unchanged. The three raw-word conversions
are explicit in PSX's initial request, GAME's returned result and OPEN's entry
decode. OPEN forwards its typed mode through the controller and display API.
The underlying BIOS buffer remains two integer words, not a pair of enum
objects passed under a different API type.

Sixteen compiler controls pass: four accept typed field/API chains and explicit
transport/unlisted-value conversions; twelve reject raw assignments, implicit
transport encoding, raw API arguments and cross-domain comparisons/calls.
Each negative has exactly one error at its intended operation. No production
tests or size assertions were added.

The seven edited bodies and three additional OPEN initializer-unit bodies
retain their baseline scores: eight exact, two partial. The additional reviewed
functions are OPEN `lighting_set_active_color_matrix` (`0x800168dc`, `0x2c`),
`render_initialize` (`0x80016908`, `0x1d4`) and `primitive_buffer_allocate`
(`0x80016cb4`, `0x84`), all 100%. Their complete sources and six-view dossiers
support the [154-occurrence initializer ledger](open-render-init-literal-ledger.md).
The SDK DRAWENV header explicitly identifies dithering, drawing to the display
area and background auto-clear flags; these remain ordinary boolean values.

The ten-body comparison preserves 2,371 resolved instruction words, 197 calls
and 292 address references. All 592 words in exact bodies equal retail and
their delinked targets. The two unchanged partials first differ at their frame
setup: GAME `0x80018880` uses candidate -0xd8 versus retail -0xe0, and OPEN
`0x80016adc` uses candidate -0x28 versus retail -0x30. These are existing
residues; the type work makes no new compiler attribution. All 484 scores are
unchanged.

Isolated before/after compilation of all 112 source/image variants preserves
runtime contents, symbols and ordered relocations, with no changed object
sections, including debug sections. Both palette tables again equal source
initializers, compiled before/after contents, delinked targets and retail:
224 GAME bytes and 160 OPEN bytes. Modern checking retains the same 300 errors
and 65/112 passing variants.

All 684 repository tests pass (143.467 seconds), as do Ruff, inventory and
whitespace checks. The existing global-type query fixture now expects
`KfGameExitCode`. Full `kf build` retains the existing data-placement failures:
source-data matches are PSX 0/1, GAME 9/42 and OPEN 3/19; target relinks are
PSX 1/1, GAME 75/77 and OPEN 34/38, with six conflicting section bases and
no data-artifact failures.

Complete literal accounting now covers 86 files / 5,232 occurrences. The total
remains 6,428 inline occurrences in 111 C files; no inline literal is removed
by this domain separation. The ten source lines with unresolved fields remain
open. Ignored reproduction inputs and reports are under
`build/constant-names/overlay-mode-domains/`.
