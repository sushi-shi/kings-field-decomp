# GAME menu runtime constants

## Function Match Plan

Baseline `7c75e5f94db8c9f48cf3867e450f9a127bd73cff`; image **GAME.EXE**, unit `game.menu_runtime`, profile `probe-gcc257-o2-g0`. The contiguous run remains a working ownership model.

The six semantic views were captured for every function below, with source history and adjacent menu/list/status, CD-loader and TMD consumers. Direct calls are proven; decoded data references retain their validated status. Identity signatures remain curated hypotheses. No signatures, widths, control flow, SDK calls, data ownership or ordering will change in this naming pass. Each branch/return includes its delay slot.

Game-specific menu assets, list layout and resource policies distinguish this run from vendored SDK bodies. Its calls to SetPolyFT4, SetSemiTrans, AddPrim, ClearOTagR and the matrix/display APIs remain SDK calls; no library body is reconstructed or counted here. See the [drawing-layer evidence](game-menu-drawing-layers.md).

Name the sprite point offsets, primitive RGB default and distinct list defaults. Name shared backdrop grid origins and express column/row spacing from the shipped tile dimensions, while preserving runtime descriptor reads for quad extents. Retain decimal conversion, booleans, indices and individual authored positions with occurrence-specific reasons. Require unchanged runtime sections, ordered relocations and strict scores; compare all exact functions to retail bytes.

| Function | GAME VA / bytes | Strict baseline | Retail calls / conditional branches / returns | Source hypothesis |
| --- | --- | ---: | --- | --- |
| `menu_draw_two_option` | `800291ec / 268` | 100.000000% | 7 / 3 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_draw_item_name_frame` | `800292f8 / 1976` | 95.376520% | 23 / 1 / 1 | Pickup grid at X=118/189, Y=16/120; preserve transform, name copy and mirrored UVs. |
| `menu_blit_sprite_translucent` | `80029ab0 / 416` | 100.000000% | 3 / 0 / 1 | Subtract point offsets (4,3); preserve signed coordinates and descriptor width/height. |
| `menu_blit_sprite` | `80029c50 / 400` | 100.000000% | 2 / 0 / 1 | Subtract point offsets (18,2); preserve opaque packet behavior. |
| `menu_draw_string` | `80029de0 / 1328` | 100.000000% | 6 / 12 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_draw_number` | `8002a310 / 512` | 100.000000% | 2 / 2 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_draw_window_backdrop` | `8002a510 / 1700` | 99.971760% | 16 / 0 / 1 | Shared grid at X=166/237, Y=16/120; preserve reverse persistent-quad insertion order. |
| `menu_frame_begin` | `8002abb4 / 128` | 100.000000% | 1 / 0 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_present_frame` | `8002ac34 / 152` | 100.000000% | 5 / 0 / 1 | Audit retained literals and preserve existing source behavior. |
| `primitive_buffer_begin_poly_ft4` | `8002accc / 80` | 100.000000% | 1 / 0 / 1 | Store default RGB96 after SDK packet initialization. |
| `primitive_buffer_commit_poly_ft4` | `8002ad1c / 80` | 100.000000% | 1 / 0 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_list_init` | `8002ad6c / 140` | 100.000000% | 0 / 1 / 1 | Name eleven visible rows and eight-halfword default stride; retain ten-halfword title copy. |
| `menu_format_number` | `8002adf8 / 172` | 100.000000% | 0 / 8 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_load_item_model` | `8002aea4 / 104` | 100.000000% | 3 / 2 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_release_item_model` | `8002af0c / 60` | 100.000000% | 1 / 1 / 1 | Audit retained literals and preserve existing source behavior. |
| `menu_load_item_texture` | `8002af48 / 304` | 100.000000% | 3 / 8 / 1 | Audit retained literals and preserve existing source behavior. |

## Resource and consumer evidence

`KF/COM/STAT.DAT` is 5,708 bytes, SHA-256 `3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`. Its initial 912 bytes are copied into `menu_assets`. The descriptor at file offset `0x318` decodes as tpage30, CLUT32000, U104, V0, width71, height104. These dimensions explain the fixed grid steps: 237−166 = 189−118 = 71 and 120−16 = 104. Origins are authored screen positions; their original aesthetic rationale is unknown. Runtime descriptor dimensions still determine each quad extent, independently of fixed origins.

Retail sprite instructions subtract four/three and eighteen/two before storing quad coordinates. The primitive initializer loads `0x60` at GAME `8002acec` and stores it to all three color bytes. It is texture color modulation, not alpha; semi-transparency is enabled by separate SDK calls.

Retail list stores eleven at byte27 and eight at byte31 (`8002add8..8002adf4`). All seven confirmed callers overwrite the eight-halfword stride with `MENU_GLYPHS_PER_ROW` before rendering; preserve the distinct initializer default. Eleven rows at twelve-pixel pitch span 132 pixels, but no original rationale for choosing eleven is known.

The texture loader formats zero-based IDs as one-based three-digit filenames: `id + 1`, hundred/tens/units at template indices5/6/7, then the CD helper adds its prefix/version suffix. Shipped paths include `TIM/M001.` and `TIM/M002.`. Decimal divisors and ASCII zero describe that encoding directly.

## Final verification

Fifteen private constants name 73 former inline uses; the complete
[retained-literal ledger](game-menu-runtime-literal-ledger.md) accounts for
all 103 remaining occurrences, including six initializer/offset-check tokens.
The earlier window and preview slices were refreshed to 13 and 160 occurrences.
The source-wide census is 7,025 occurrences; complete token/expression accounting
is now verified for 19 files and 1,809 occurrences. These are accounting totals,
not counts of still-missing names.

Forced unit compilation and `kf build compare` passed. Isolated before/after
compilation of all 112 source/image variants preserved every allocated section,
its alignment, runtime symbol and ordered relocation. Only this unit's debug
line section changed. Live build objects agree with the isolated controls;
all 484 strict function scores are unchanged.

All sixteen reviewed functions retain their resolved instructions and ordered
references: 1,959 words, 74 calls and 303 address pairs. The fourteen exact
functions also match target objects and all 1,036 retail instruction words.
Each function's final score equals its baseline in the plan above. The two
partial functions remain partial: pickup preview 95.376520%, backdrop 99.971760%.
Their first raw differences remain at entry: candidate/retail stack allocations
are 160/224 bytes at `800292f8` and 40/104 bytes at `8002a510`. These are
unattributed source/codegen residues; no compiler mechanism is asserted.

Inventory checks, Ruff and `git diff --check` pass. All 680 existing tests pass
(89.866 seconds), including retail/candidate mirrored packet, enqueue-order and
list-initializer controls. No tests were added. Modern checking retains exactly
the same 300 diagnostics and 65/112 passing variants. Full `kf build` still
fails on existing data ownership/section-placement and relink problems:
source-data matches PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1,
GAME 75/77, OPEN 34/38. No exact-count movement or new banking is claimed.

The initialized current-primitive pointer (four zero bytes) and ten-byte TIM
path including its terminator match candidate, target and retail. The candidate
RODATA section is ten bytes; the target's twelve-byte claim includes two trailing
zero bytes. This pre-existing section-extent difference is retained and prevents
claiming complete data-section equality to retail, despite the exact texture
loader code. No source padding was introduced to hide it.
