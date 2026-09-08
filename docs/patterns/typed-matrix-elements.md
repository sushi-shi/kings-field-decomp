# Typed matrix element access

## Function Match Plan

Baseline is `b6a4e7c`. GAME `matrix_interpolate` (`800202fc`, 0x68 bytes) and its OPEN twin
(`80019598`, 0x68 bytes) are strictly 100%. Both retail bodies have the same
26 instruction words, with no calls, global references or relocations.
Three blocks implement nine unsigned halfword loads/stores, signed operand
extensions, a multiply and arithmetic shift by 12, and the 16-bit countdown
8 through -1. The output increment belongs to the loop branch delay slot;
the final return owns a nop. No strings or vendor-only logic are present.
These are game interpolation helpers feeding Sony matrix setters; the setters
remain vendored. The real Psy-Q `MATRIX.m` field is nine signed halfwords.

The five confirmed GAME callers are the two player death fades, color/light
matrix setters and map reveal fade. The adjacent setters pass a complete
local MATRIX to the SDK; no translation element is read or written here.
Source history and the exact OPEN twin support replacing GAME's three
whole-object reinterpretations with `&matrix->m[0][0]`, preserving explicit
unsigned value conversions before the observed signed subtraction. This is
the already established flat traversal of the SDK's contiguous matrix cells;
no SDK layout, signature, call, constant, counter or algorithm is changed.

Image-specific address, CFG, callers/callees, strings and match snapshots for
the two helpers and GAME adjacent setters are saved under
`build/cast-model/typed-matrix-elements/`. Compare all functions in both units
against fresh baseline objects and retail, preserving each exact result. Run
the full build, existing tests, lint and source diff checks before banking
only the changed GAME function.

| Image / function | VA / extent | Strict before | Planned change | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME matrix_interpolate | 800202fc / 68 | 100 | SDK element pointers and unsigned value conversions | 100%; raw unchanged |
| GAME lighting_set_color_matrix | 80020364 / 2c | 100 | Caller/adjacent control; unchanged | 100%; raw unchanged |
| GAME lighting_set_light_matrix | 80020390 / 2c | 100 | Caller/adjacent control; unchanged | 100%; raw unchanged |
| OPEN matrix_interpolate | 80019598 / 68 | 100 | Exact typed sibling; unchanged | 100%; raw unchanged |


## Result

All 12 functions in the two matrix units are still strictly 100%, with
identical linked instructions, call targets and ordered references. The
changed GAME helper matches all 26 retail words. Three pointer casts are
removed; two explicit unsigned value conversions preserve the retail loads
and subtraction widths. All 112 target-C variants parse without errors.
There are 738 written casts, including 43 header checks/conversions, and
475 C pointer casts: 331 fewer than the original 806.

All 713 repository tests pass (nine skips), Ruff and diff checks pass, and
the complete build retains 439/471 exact functions and its existing closure
gates without artifact failures. Only GAME matrix_interpolate is banked.
