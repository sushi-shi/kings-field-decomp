# GAME statistics text layout and current literal ledger

## Rating-weight naming follow-up

Function Match Plan: name the eight formula literals in GAME
`800264d8 / 0xcb4`, `menu_draw_status_details`, without changing operations,
operand types or evaluation order. The fresh six-view dossier still reports
the existing 98.296430% result; it is a stored result, not post-edit verification.
The direct caller, adjacent renderers, game-owned player/atlas references,
source history and vendor exclusion agree with the evidence below.

Retail `80026b58..80026b98` forms physical attack times three then logical
half, adds twice holy/fire, and scales the signed subtotal by ten/eight.
`80026bd0` divides poison resistance by five before summation, and
`80026c38..80026c48` scales the signed defense subtotal by ten/seven.
Keep all 72 calls, 91 validated references, branch/delay-slot ownership and
the final frame release at `80027188` unchanged. The denominators are display
weights, not inferred component counts; their original calibration is unknown.

Use local integer constants for the physical multiplier and downshift,
elemental multiplier, and separate attack/defense scale numerators and
denominators. No shared API or field domain changes. Builds, compiler checks,
tests and post-edit matches remain deferred under the user's naming-pass
instruction. The final source review will verify literal values and accounting;
it cannot establish a new binary match result.

Final verdict: all eight values now have the planned local names. Source
review preserves each operation and type boundary; the retained-literal
ledger has 208 occurrences and all 111 current file ledgers reconcile.
No build or match claim is made for this naming edit.

## Function Match Plan

The two statistics renderers share four-digit HP/MP fields and six-digit
other values. Name those presentation widths and express the HP/MP slash
advance as four times the existing `MENU_NUMBER_ADVANCE`. Reuse that same
seven-pixel advance after the slash. Name each view's repeated row pitch:
23 pixels in the header, 16 in the detail summary, and 14 for individual
attack/defense component rows. These are view-specific layout settings.

The formatter consumes a digit count and a zero/nonzero padding flag; zero
selects blank glyph padding. The number renderer positions glyphs at
`i * MENU_NUMBER_ADVANCE`. Halfword coordinates, signed glyph codes, calls,
all authored glyph sequences and the original arithmetic remain unchanged.
Do not replace signed division by a shift or reorder the attack subtotal
before/after its row advance. The old ledger's arithmetic and index names
must be reconciled with current source, not copied as if still current.

Fresh six-view GAME semantic dossiers, source/history, caller sites and
adjacent render helpers were reviewed from `bbf42ec`. These are game-specific
player-statistic and atlas consumers, not vendored code. The existing
[shared-index evidence](game-stats-header-index.md),
[rating-arithmetic correction](game-status-rating-arithmetic.md) and
[class-title resource evidence](game-menu-class-titles.md) establish the
control/data contracts. The original reason for individual screen anchors
or formula weights remains unknown; document authored values honestly.

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `80025f38 / 1440` | `menu_draw_stats_header` | 97.991670% |
| `800264d8 / 3252` | `menu_draw_status_details` | 98.296430% |

The source hypothesis is identical runtime objects and ordered referents
after literal-to-constant substitutions. Freeze all 112 source/image variants,
force both units, compare before/after instructions, calls, relocations and
all 484 strict scores, then run modern checking, inventory, Ruff, repository
tests, whitespace and full `kf build`. Reconcile every retained occurrence
against an explicit current ledger row. No size assertions or tests are added.

## Names and retained values

| Setting | Value / units | Uses |
| --- | --- | --- |
| `STATS_HEADER_ROW_STEP` | 23 screen pixels | Fourteen header row advances. |
| `STATUS_SUMMARY_ROW_STEP` | 16 screen pixels | Twenty-one detailed-summary row and section-heading advances. |
| `STATUS_COMPONENT_ROW_STEP` | 14 screen pixels | Twenty individual attack/defense label and value advances. |
| `MENU_STATS_VITAL_DIGITS` | Four decimal glyphs | Eight HP/MP formatter calls and four slash-position advances. |
| `MENU_STATS_VALUE_DIGITS` | Six decimal glyphs | Twenty-two other statistic formatter calls. |
| Existing `MENU_NUMBER_ADVANCE` | Seven screen pixels | Four post-slash advances; four digits place each slash 28 pixels later. |

Field widths are presentation policy, not numeric bounds on the stored
statistics. The existing zero padding flags remain explicit booleans selecting
leading blank glyphs. Glyph positions and encoded text cells remain stream
data. Screen anchors, the one-off 30-pixel component-section gap, class-grid
indices, status-bit emptiness checks and formula weights retain individual
reasons in the [current ledger](game-menu-status-literal-ledger.md).

The ledger now uses the actual shared `glyph_index` local. Attack arithmetic
retains the unsigned physical subtotal's multiply-by-three/logical-half step,
the double holy/fire subtotal, and the signed final multiply-by-ten/divide-by-
eight operation. Defense retains poison divided by five and the signed total
scaled by ten/seven. These are display-score weights; no original design
rationale or equal-component interpretation is inferred.

## Verification

Paired compilation uses committed `bbf42ec` sources plus only the two renderer
changes and shared header above. All 112 source/image variants retain identical
allocated bytes, alignment, runtime symbols and ordered relocations. The live
objects equal the isolated after objects. Only the two renderers' debug line
tables change. The original live-object capture also differs only in those
debug sections, and all 484 strict scores are unchanged.

Both consumers retain the partial scores in the baseline table. Their 1,170
instructions, 100 direct calls and 125 resolved address materializations agree
before and after. No new exact result is claimed or banked.

The first unchanged divergences are GAME `80025f38` (stack allocation 0x40
versus retail 0x48) and `80026514` (reconstructed `nop` versus retail
`ori s2,zero,0x23`). These observations do not establish an optimizer cause.

Inventory, Ruff, all 683 repository tests (108.350 seconds), and whitespace
checks pass. Whole-tree modern checking preserves the same 300 errors and
65/112 passing variants. Full `kf build` retains the existing data/relink
failures: source data PSX 0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1,
GAME 75/77, OPEN 34/38, with six section-base conflicts and no artifact failures.

Ninety-three inline occurrences are replaced: the header goes from 130 to
104 retained literals, and the detail view from 283 to 216. The 320 remaining
occurrences have separate ledger rows, including duplicates and corrected
arithmetic. The source census is now 6,693 occurrences; verified current
accounting covers 32 files / 3,406 occurrences. The ten source lines containing
unresolved `unknown_` fields are unchanged.
