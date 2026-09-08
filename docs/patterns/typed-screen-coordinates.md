# Shared packed screen coordinates

## Function Match Plan

Start at `6e1a319` in the isolated worktree. All six semantic views have been
captured for the fourteen image-qualified functions below. The existing GPU,
TMD, map, unlit and graphics-owner campaigns supply the complete source,
caller, adjacent-function and SDK evidence; history and the current shared
field/layout controls have been reviewed. GAME and OPEN retain independent
addresses, profiles and state owners.

Both images use one eight-byte KfScreenVertex: packed XY at +0, signed depth
at +4 and signed perspective/alternate depth at +6. SDK RotTransPers takes a
pointer to a packed long at +0. NormalClip and the polygon writers read that
word. Non-perspective RotTrans paths write X and Y with sh at +0 and +2.
The existing casts to DVECTOR therefore select a directly evidenced second
view of the same four-byte value.

First hypothesis: KfScreenXY is a union of SDK long `word` and authentic
DVECTOR `vector`; KfScreenVertex owns it as `sxy`. Propagate all packed-word
uses to the real member and both non-perspective writers to the halfwords.
Check size/alignment and all offsets. Add no padding or alternative packet
shape. Shared SDK output arguments, clipping values and signedness remain
unchanged. The GAME map-cell renderer's three MATRIX pointer casts are
canonical same-type casts; remove those while retaining the exact SDK calls.

No source-reference owner, address expression outside this member selection,
call set, constant, CFG, register hint or profile changes. In particular,
OPEN's separately documented vertex-address/owner-recovery expressions are
left for their own complete evidence review. Preserve every load/branch/call
and return delay instruction in the raw comparison.

Vendor negative controls remain the established game projection, clipping,
lighting, asset and display-list policies. SDK services remain vendor callees.
No vendored body is reconstructed. Candidate table rows remain hypotheses,
and no outgoing reference is promoted or masked. OPEN's checked projection
path retains its `POINT OVER !!!!!!\n` diagnostic at `0x800121ac`; the other
selected string queries are empty.

| Image / function | VA / bytes | Strict before | Blocks/JAL/branches/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME `tmd_project_vertices` | `8001c60c / 9c` | 100 | 4/2/2/1 | 100%; raw unchanged |
| GAME `tmd_project_vertices_shift` | `8001c6a8 / ac` | 100 | 4/2/2/1 | 100%; raw unchanged |
| GAME `tmd_transform_vertices` | `8001c754 / a4` | 100 | 4/1/2/1 | 100%; raw unchanged |
| GAME `render_enqueue_tmd` | `8001c7f8 / f38` | 99.3142 | 92/45/66/1 | Partial unchanged; raw unchanged |
| GAME `render_enqueue_model` | `8001d730 / 6e8` | 100 | 41/15/30/1 | 100%; raw unchanged |
| GAME `render_enqueue_map` | `8001de18 / 418` | 100 | 20/16/14/1 | 100%; raw unchanged |
| GAME `render_map_cell` | `8001e5ec / 250` | 99.8784 | 24/9/17/1 | Partial unchanged; raw unchanged |
| OPEN `tmd_project_vertices` | `8001738c / cc` | 100 | 5/3/4/1 | 100%; raw unchanged |
| OPEN `tmd_project_vertices_perspective_right` | `80017458 / a4` | 100 | 4/2/2/1 | 100%; raw unchanged |
| OPEN `tmd_project_vertices_shift` | `800174fc / ac` | 100 | 4/2/2/1 | 100%; raw unchanged |
| OPEN `tmd_transform_vertices` | `800175a8 / a4` | 100 | 4/1/2/1 | 100%; raw unchanged |
| OPEN `render_enqueue_tmd` | `8001764c / cf8` | 100 | 80/57/54/1 | 100%; raw unchanged |
| OPEN `render_enqueue_unlit_triangles` | `80018344 / 2a4` | 100 | 17/8/11/1 | 100%; raw unchanged |
| OPEN `render_enqueue_map` | `800185e8 / 3b8` | 100 | 18/18/12/1 | 100%; raw unchanged |

Fresh focused builds must preserve the exact functions. Compare every function
in the affected units against saved linked words, ordered calls/references and
retail before banking the intended exact rows. Then run the full build,
existing tests, Ruff and diff checks. Existing image closure failures remain
separate from this source model.

## Raw-object result

All 41 functions in the seven affected units have unchanged linked instruction
words, calls and ordered references. The 38 exact functions remain exact;
GAME's existing display initialization, general TMD enqueue and map-cell
partials retain their previous bytes. The fourteen direct consumers comprise
twelve exact functions and two unchanged partials. Overall strict status
remains 439/471 (GAME 332/362, OPEN 106/108, PSX 1/1).

The two halfword writers now name the SDK coordinate members directly. The
229 audited member occurrences use the same packed word for all GTE, clipping
and packet consumers. This removes four screen pointer casts and three
redundant MATRIX casts. No array extent, owner arithmetic or packet schedule
was changed.

The AST census parses all 112 image variants without errors and counts 764
written casts, including 41 header checks/conversions. C source contains 505
pointer casts, down 301 from the original 806.

The full 713-test run had one inventory-order error and nine skips. The new
structure and field rows were moved into canonical order; all 113 inventory tests then
pass. The other 712 tests completed with nine skips, including existing packet and projection
compile controls. Ruff and `git diff --check` pass. Focused rebuilding and
full `kf build` were run, including a final build after the inventory fix.
The pre-existing data/reference/placement gates still fail in all three
images without artifact failure. Bank only the twelve directly selected
exact functions; no partial function is banked.
