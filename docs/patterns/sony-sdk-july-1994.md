# Earlier Sony SDK header control

## Function Match Plan

Starting point: `5f380319`, 456/471 strict-exact functions. This follow-up keeps
the fifteen unfinished functions and their image-qualified dossiers in
[the current campaign](sony-final-fifteen.md). It investigates an earlier
primary SDK package rather than inventing a macro variant.

The [archived Runtime Library 2.0 disc](https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%202.0%20%28Japan%29%20%28En%2CJa%29_DTL-S2160_redump.zip)
contains `PSX.LZH`. Its ISO primary-volume creation field encodes
1994-07-15 11:20:00; the extracted archive headers identify LIBGPU revision
3.19 (1994-07-07), LIBGTE revision 1.2 (1994-07-06), and LIBCD revision 1.1
(1994-07-14). These are file provenance observations, not proof of the headers
used for King's Field.

The July include tree contains 30 headers and 74 distinct function-like macro
names, compared with 32 headers and 155 names in the staged December package.
The vector, XY/UV, RGB, rectangle and clamp expressions are unchanged after
removing comments and whitespace. The only changed common macro is a debug
formatter, `dumpUV3`; the five July-only function-like names belong to the
conditional old display-environment compatibility layer. No C inline helper
definition is present.

The July LIBGTE header has no function declarations. Its MATRIX, VECTOR,
SVECTOR, CVECTOR, DVECTOR and EVECTOR declarations otherwise equal the common
December types after removing comments and whitespace. July samples call
`RotTrans` directly with `MATRIX.t`, including
`SAMPLE/GRAPHICS/FASTER/TAKE/TUTO1.C`'s object emitter, which then calls
`MulMatrix0`, `SetRotMatrix` and `SetTransMatrix` on that matrix. This supports
the current output-owner interpretation without identifying the game's local
frame layout.

Compile unchanged complete units for all fifteen functions with the original
July LIBGTE header supplied through a disposable include directory. Keep the
current compiler, optimization, small-data settings and all other headers.
This isolates the historically available declaration boundary; it is not a
claim that the mixed-header set was an original SDK release. Inspect warnings
and all affected SDK call sites, particularly default argument promotions and
consumed return values. Require resolved word, call, reference, size and sibling
comparisons. Do not retain an incompatible declaration or infer a source type
solely from a score. Preserve the three exact UV functions as controls.

No downloaded executable is run. Disc contents, original headers, samples,
hashes, temporary includes and compilation outputs stay under the ignored
`build/sony-final-15/sdk-history/` directory.

## Result

All fourteen complete unit ELF objects are byte-identical to their normal
December-header controls, including symbol tables and relocation records.
Resolved target objects reproduce retail, all sibling bodies are preserved,
and each row below retains its complete ordered calls and data references.
No source or header change is retained. **456/471 remains exact.**

| Image | Function | Strict % after July-header control |
| --- | --- | ---: |
| GAME | `player_move_horizontal` | 96.56896 |
| GAME | `player_update` | 99.96709 |
| GAME | `render_map_cell` | 99.87838 |
| GAME | `render_entities` | 99.517044 |
| GAME | `item_load_database` | 99.746666 |
| GAME | `menu_draw_item_detail` | 97.80875 |
| GAME | `talk_show_dialogue_page` | 98.78049 |
| GAME | `map_object_spawn_effect` | 94.50495 |
| GAME | `map_show_screen_image` | 88.888885 |
| GAME | `map_interaction_dispatch` | 99.202774 |
| GAME | `map_world_state_persist` | 97.52873 |
| GAME | `effect_map_collision` | 99.87369 |
| GAME | `effect_projectile_update_2d` | 99.93421 |
| GAME | `effect_update_dispatch` | 99.82781 |
| OPEN | `opening_ending_scroll_run` | 99.917694 |
| GAME | `menu_status_panel` (UV control) | 100 |
| GAME | `menu_draw_item_name_frame` (UV control) | 100 |
| GAME | `menu_draw_window_backdrop` (UV control) | 100 |

A separate frontend audit confirms the July header's actual inclusion and
the absence of its later GTE declarations in all fourteen preprocessed units.
Compiler diagnostics are empty except for the existing unsigned charge clamp
in `player_update`; its lower-bound warning is identical with the normal
header. Default argument promotions and return handling therefore create no
different instruction in these units under this probe.

This closes the tested declaration-boundary hypothesis. The older package
also supports availability of the retained reflected-UV expression before
the game's release; it does not prove which header revision compiled the game.
The existing per-function source, referent, CFG and frame constraints remain.

## Reproduction inputs

The downloaded ZIP is 55,343,573 bytes, SHA-256
`7bcffd476485f9a19bdadb9b33020127e0c138caf0a32f5580c9eaed2fc57c81`.
The raw CD is decoded with the existing ISO9660 reader, then `PSX.LZH` is
extracted with the pinned 7-Zip. Original header SHA-256 values are:

| July header | SHA-256 |
| --- | --- |
| LIBGPU.H | `cb8faaf7082438555a4d32625ffbb72c807e8ff11f0f4ec37440e1c2fc458bc3` |
| LIBGTE.H | `f92340ecb71854e3d7dbe1150a2e7200154d38c95a8f85ce39d87e9a35fc55a3` |
| LIBCD.H | `e0b76f1f27ae0339bbaced157160e7de8dac7db872431e92d0944e1d2eda9f27` |

The temporary include directory contains only that unmodified LIBGTE.H.
All other includes retain their existing resolution. Whole-ELF comparisons,
resolved instruction audits, preprocessed-source hashes and diagnostics are
stored with the generated evidence. No historical compiler executable is run,
and no compiler, assembler, profile or inventory input changes.

## Verification

All fourteen units were freshly compiled with both include configurations.
The frontend, complete-object and resolved-word audits passed. The repository
suite passed 775 tests and 9,223 subtests; `ruff check scripts tests` and
`git diff --check` passed.

The full `kf build` retains the existing twenty data-section failures, with
no artifact failures: PSX 1/1, GAME 29/41 and OPEN 12/20 data-owning units match.
Target relink verification remains PSX 1/1, GAME 57/64 and OPEN 34/38.
These checks do not establish complete data ownership or executable closure.

After the concurrent cast cleanup reached master at `2b1baa89`, a separate
fresh compilation and resolved-word audit preserved all fifteen recorded
function bodies and all three exact UV controls. The header experiment and
repository suite above use the campaign base `5f380319`; no production source,
profile, inventory or bank changes belong to this follow-up.
