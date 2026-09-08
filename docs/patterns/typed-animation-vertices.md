# Typed animation vertex views

## Function Match Plan

Baseline `7ca8191`. The binder, its five confirmed callers, cache lifecycle,
TMD vertex selection and projection consumers share one eight-byte vertex
representation. Image-specific CFG, signatures, callers/callees, strings,
current matches and pre-edit objects are captured under
`build/cast-model/typed-animation-vertices/`; existing animation-cache,
asset-format and graphics-owner dossiers plus source history supply context.
These are game policies around separately vendored geometry and allocation
services. `gteMIMefunc` is the byte-exact 128-byte Sony LIBGTE MSC body; its
real header accepts `SVECTOR *` and remains unchanged.

The binder copies each complete vertex as two aligned words, then passes it
to the SDK as an SVECTOR. Define a shared eight-byte `KfPackedSVector` union
with the authentic SDK member and two words; propagate that type through
the cache, current-TMD vertex pointer and game selection API. Projection
loops pass the SDK member directly. Generic allocation and encoded TMD byte
offset boundaries keep explicit casts. The complete storage extents of the
GAME projection/morph region remain unresolved: no 1000/1001 capacity or
new global subowner is inferred from spacing.

The rest-morph call deliberately includes the eight-byte base/count header
as its first vector. Model its two range words and SDK vector as alternative
views of that same prefix, retaining the VDF object selector, 12-byte delta
start and variable trailing vector convention. Save and restore both words
of the affected scratch entry around the same SDK call. Preserve its +1
count and the scratch publication at +8.

The binder returns its live KfPoolRecord pointer, null on exhaustion, or the
existing integer-one static-success sentinel. All five callers branch only
on zero/nonzero; no halfword pointer is dereferenced. Use `KfPoolRecord *`
for the shared return type, preserve the explicit sentinel conversion and
return the actual record directly. Keep the retail uninitialized incoming
keyframe-index behavior and every existing branch, argument width and delay
slot; no correctness claim is made for that pre-existing runtime defect.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME.EXE `tmd_set_current_vertices` | `8001c138 / 10` | 100 | 1/0/1 | 100%; raw unchanged |
| GAME.EXE `tmd_select_object_vertices` | `8001c148 / 3c` | 100 | 1/1/1 | 100%; raw unchanged |
| GAME.EXE `tmd_project_vertices` | `8001c60c / 9c` | 100 | 4/2/1 | 100%; raw unchanged |
| GAME.EXE `tmd_project_vertices_shift` | `8001c6a8 / ac` | 100 | 4/2/1 | 100%; raw unchanged |
| GAME.EXE `tmd_transform_vertices` | `8001c754 / a4` | 100 | 4/1/1 | 100%; raw unchanged |
| GAME.EXE `render_actor` | `8001e9a4 / 214` | 100 | 7/20/1 | 100%; raw unchanged |
| GAME.EXE `render_actor_sprite` | `8001eedc / 1e8` | 100 | 8/20/1 | 100%; raw unchanged |
| GAME.EXE `render_map_event` | `8001f0c4 / 154` | 100 | 4/15/1 | 100%; raw unchanged |
| GAME.EXE `render_weapon` | `8001f798 / 118` | 100 | 4/10/1 | 100%; raw unchanged |
| GAME.EXE `render_effect_sprites` | `8001f8b0 / 124` | 100 | 6/12/1 | 100%; raw unchanged |
| GAME.EXE `asset_registry_set` | `8002055c / 40` | 100 | 1/2/1 | 100%; raw unchanged |
| GAME.EXE `asset_registry_select` | `8002059c / 38` | 100 | 1/0/1 | 100%; raw unchanged |
| GAME.EXE `render_bind_animated_instance` | `800205d4 / 3a4` | 100 | 34/12/1 | 100%; raw unchanged |
| GAME.EXE `pool_record_release` | `800209e4 / 48` | 100 | 3/1/1 | 100%; raw unchanged |
| GAME.EXE `pool_allocate` | `80020b04 / 48` | 100 | 6/0/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_set_current_vertices` | `80016eb8 / 10` | 100 | 1/0/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_select_object_vertices` | `80016ec8 / 3c` | 100 | 1/1/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_project_vertices` | `8001738c / cc` | 100 | 5/3/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_project_vertices_perspective_right` | `80017458 / a4` | 100 | 4/2/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_project_vertices_shift` | `800174fc / ac` | 100 | 4/2/1 | 100%; raw unchanged |
| OPEN.EXE `tmd_transform_vertices` | `800175a8 / a4` | 100 | 4/1/1 | 100%; raw unchanged |

Rebuild the affected units and compare every linked word, direct call and
ordered referent against baseline and retail. Only keep humane source that
preserves every exact function. Coherently update shared signatures, field
inventories and existing layout assertions; then run full build, target-C
census, repository tests, Ruff and diff checks. Bank only directly changed
functions verified strictly 100% after raw comparison.


## Result

All 51 functions in the eight compared units retain their linked words,
ordered calls and physical references. Fifty remain exact; the existing
GAME display_initialize partial stays at 98.421684%. Every function in the
snapshot table is strictly 100%, including the binder's 932-byte body.
Twelve directly edited functions are exact and eligible for banking.

The source removes nine pointer reinterpretations. The remaining static
success value is explicitly converted to the record-pointer return type;
callers still test only zero/nonzero. The packed vertices and rest-morph
prefix use their actual SDK member at geometry boundaries. The two word-copy
loops, header-inclusive blend, saved scratch words and published +8 vertex
start preserve all retail instructions. No unknown storage capacity is added.

All 112 target-C variants parse without errors. The census has 729 written
casts, including 43 in headers, and 466 C pointer casts (340 removed from
the initial 806). The pool's editor-C++ check retains only its pre-existing
SDK free-call diagnostic, also present with baseline source and headers.

The 713-test run found only two ownership-control tests whose temporary
source rewrites still used the old vertex type (three failing subcases).
After updating those fixtures to the shared packed vertex type, all 12
ownership-probe tests pass, including shifted-address and changed-offset
negative controls. The other 711 tests passed or used the existing nine
skips. Ruff and diff checks pass. The final full build retains 439/471 exact
functions, existing closure gates and zero artifact failures.
