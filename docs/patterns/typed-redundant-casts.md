# Redundant casts after shared type propagation

## Function Match Plan

The complete target-C AST audit finds seven remaining casts whose source and
target have identical canonical types. Two cast existing `u8` fields back to
`u8`: the ground-trail parent index and a map-height cell. Four narrow the
SDK's already unsigned-halfword `GetTPage`/`GetClut` return values to `u16`.
One casts the already-byte-typed TMD packet cursor to `u8 *` in its body-step
macro. None is an enum-domain conversion or a constness conversion.

Remove those seven casts and preserve the current typed fields, SDK API
declarations, multiply/offset expressions and return widths. This changes no
ownership, data extent, signature or relocation. The SDK calls remain
separately vendored; no library body is changed. GAME effect dispatch and map
collision are existing partials and must retain their compiled instruction streams;
every banked exact function must stay exact.

The image-specific disassembly/CFG, callers, callees, strings, current match
state, source history and pre-edit objects are collected under
`build/cast-model/typed-redundant-casts/`. The effect constructor and map loader
establish unsigned byte fields; the pinned `LIBGPU.H` declares unsigned
halfword results. The TMD preparer alone invokes its body-step macro with a
byte cursor. Adjacent functions and full-unit raw comparisons serve as
controls. Inspect the first real divergence in physical referents, calls,
CFG and instruction width after the focused rebuild. Run the full build,
all repository tests, Ruff, diff checks and target-C census before banking.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME `tmd_prepare_primitive_indices` | `8001c2b0 / 300` | 100% | 18/0/1 | 100%; raw unchanged |
| GAME `effect_map_collision` | `80037850 / 76c` | 98.2211% | 126/1/1 | 98.221054%; raw unchanged |
| GAME `effect_update_dispatch` | `80038a38 / 180c` | 96.9396% | 257/69/1 | 96.93957%; raw unchanged |
| OPEN `opening_scene3_run` | `80014804 / 330` | 100% | 19/27/1 | 100%; raw unchanged |


## Result

All seven redundant casts are removed: one pointer conversion and six
identity-width scalar conversions. All 27 function bodies across the four
units retain every linked word, call and ordered physical referent. Twenty-three
are exact; the two directly edited exact functions are bank selections. The
four existing partials, including both effect functions, are unchanged.

All 112 target-C variants parse without errors. Written casts now total 713,
including 46 in headers; C pointer casts total 453, down 353 from the initial
806. The full AST census contains no remaining C-file cast whose canonical
source and target types are identical. This is a type-equality check, not a
claim that all other casts are required.

All 714 repository tests pass (nine skips), as do Ruff and diff checks. The
full build retains all 439/471 exact game functions and the documented data
ownership/placement gates, with zero artifact failures. No profile, SDK
signature, curated identity or relocation changes.
