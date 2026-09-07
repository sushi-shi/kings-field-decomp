# Matrix, vector and packed-color literal coverage

## Review plan

Complete current occurrence accounting for the GAME matrix/rotation and
RGB555 modules, and the OPEN matrix/rotation, vector, heading, half-turn and
length helpers. Review the image-qualified disassembly, CFG, callers, callees,
strings, current matches and history for every function. Use the existing
fixed-point, angle, matrix-extent and packed-color definitions where their
contracts already apply; distinguish those named conventions from ordinary
matrix/component indices and arithmetic identities.

The game helpers wrap separately identified SDK trigonometry, matrix, square
root and GTE services. Do not count those providers as game reconstruction.
Review matrix element signedness, nine-element interpolation bounds, angle
quadrant decisions, vector component order, RGB555 control-bit preservation
and the declared diagnostic string extent. Do not infer an indirect caller
or a valid-input guarantee merely because a source comment says so.

Capture each function's baseline before any edit. Force all reviewed units
and compare runtime words and ordered referents, with independent retail
comparison for exact controls. Any source correction must preserve existing
matches. Finish a token-by-token ledger and run modern type checking,
inventory, lint, repository tests, whitespace and full build before commit.

## RGB555 signature correction plan

All 41 functions in the captured nine-unit baseline are exact. GAME
`color_lerp_rgb555` at `80020428 / 152` bytes has no incoming reference admitted
by the current navigator. Its inventory still declares `u32` arguments and
return, whereas the exact source uses `u16`. Retail masks the two colors to
sixteen bits at `80020438` and `80020444`, and masks the result at `800204bc`
in the return delay slot. OPEN `8001974c / 152` has the same 38 instructions
and already has the corresponding `u16` inventory signature.

Reconcile GAME's inventory with its existing exact source and the raw homolog.
Use `color0`, `color1` and `blend` consistently with OPEN's semantic parameter
names. Remove the GAME comment's unsupported indirect-reachability and
in-range-caller assertions. Retain the arithmetic, unsigned color inputs,
signed blend, original STP-bit term and final result width. Capture every
function's final verdict and independently compare the homolog bytes.

## Retained mathematical and data conventions

The [complete ledger](math-literal-ledger.md) accounts for 189 retained
occurrences in nine files. Matrix row/column coordinates remain explicit
numbers so the small formulas retain their usual notation. Zero coefficients
express absent cross-axis coupling, and the pitch/yaw source vector has zero
perpendicular components. Raw two/three-component halfword views retain their
ordinary indices: their current caller inventories do not establish a more
specific coordinate plane or containing object.

Both matrix interpolators visit the nine leading rotation coefficients,
leaving translation storage untouched. Unsigned loads preserve each original
halfword, while the difference uses separately sign-extended operands. The
signed sixteen-bit counter runs from eight through zero and exits at minus
one. Half-turn tests use the named bound plus one to retain the exclusive
comparison. Heading sign tests use arithmetic zero; x=z=0 takes the existing
quarter-turn fallthrough, rather than a distinct invalid-heading result.

The OPEN diagnostic declaration has sixteen bytes: twelve text bytes, a
terminator and three further zeros. Its following format string occupies
twenty bytes. All thirty-six claimed read-only bytes agree with both objects
and retail. This proves the stored bytes, not the original author's array
declaration or padding/alignment rationale. The extent remains documented
without inventing a gameplay capacity.

The RGB555 homologs agree byte-for-byte over all 152 bytes. They combine the
first input's STP term with unclamped channel results and truncate the packed
result to sixteen bits. For blend values from zero through Q12 unity, channels
remain in range and STP comes from the first input. That statement is not a
guarantee for arbitrary blends: for example, black to white at blend 8192
produces channel intermediates 62 and packed result 0xfffe, setting bit fifteen
even though it was clear in the first input. This follows from the decoded
shift/OR arithmetic; no admitted caller proves an in-range precondition.

GAME's inventory now agrees with its existing u16 source signature. Parameter
names match OPEN, and the unsupported caller assertions were removed from the
source comment. No executable operation, field width or constant changed.

## Per-function final verdicts

Every function started and remains at strict 100%; all words and ordered
references agree with the prior object, delinked target and raw retail.

| Image | Function | VA / bytes | Final verdict |
| --- | --- | --- | --- |
| GAME.EXE | `angle_approach` | `0x80014a64 / 200` | 100%; unchanged. |
| GAME.EXE | `angle_to_forward_xz` | `0x80014b2c / 80` | 100%; unchanged. |
| GAME.EXE | `matrix_set_rotation_x` | `0x80014b7c / 112` | 100%; unchanged. |
| GAME.EXE | `matrix_set_rotation_y` | `0x80014bec / 112` | 100%; unchanged. |
| GAME.EXE | `matrix_set_rotation_z` | `0x80014c5c / 112` | 100%; unchanged. |
| GAME.EXE | `matrix_set_rotation_yxz` | `0x80014ccc / 104` | 100%; unchanged. |
| GAME.EXE | `pitch_yaw_to_forward_vector` | `0x80014d34 / 212` | 100%; unchanged. |
| GAME.EXE | `vector2s_scale_shift11` | `0x80014e08 / 64` | 100%; unchanged. |
| GAME.EXE | `vector3s_scale_shift12` | `0x80014e48 / 92` | 100%; unchanged. |
| GAME.EXE | `vector2s_scale_shift12` | `0x80014ea4 / 64` | 100%; unchanged. |
| GAME.EXE | `vector3s_scale_shift12_alt` | `0x80014ee4 / 92` | 100%; unchanged. |
| GAME.EXE | `vector3i_add_xz` | `0x80014f40 / 44` | 100%; unchanged. |
| GAME.EXE | `angle_within_tolerance` | `0x80014f6c / 60` | 100%; unchanged. |
| GAME.EXE | `angle_mod_delta_le_half_turn` | `0x80014fa8 / 16` | 100%; unchanged. |
| GAME.EXE | `vector_xz_to_angle` | `0x80014fb8 / 176` | 100%; unchanged. |
| GAME.EXE | `fixed_vector2_length` | `0x80015068 / 64` | 100%; unchanged. |
| GAME.EXE | `matrix_interpolate` | `0x800202fc / 104` | 100%; unchanged. |
| GAME.EXE | `lighting_set_color_matrix` | `0x80020364 / 44` | 100%; unchanged. |
| GAME.EXE | `lighting_set_light_matrix` | `0x80020390 / 44` | 100%; unchanged. |
| GAME.EXE | `fog_interpolate_near` | `0x800203bc / 68` | 100%; unchanged. |
| GAME.EXE | `fog_set_near` | `0x80020400 / 40` | 100%; unchanged. |
| GAME.EXE | `color_lerp_rgb555` | `0x80020428 / 152` | 100%; unchanged. |
| OPEN.EXE | `matrix_set_rotation_x` | `0x800158d0 / 112` | 100%; unchanged. |
| OPEN.EXE | `matrix_set_rotation_y` | `0x80015940 / 112` | 100%; unchanged. |
| OPEN.EXE | `matrix_set_rotation_z` | `0x800159b0 / 112` | 100%; unchanged. |
| OPEN.EXE | `matrix_set_rotation_yxz` | `0x80015a20 / 104` | 100%; unchanged. |
| OPEN.EXE | `debug_dump_matrix` | `0x80015a88 / 132` | 100%; unchanged. |
| OPEN.EXE | `pitch_yaw_to_forward_vector` | `0x80015b0c / 212` | 100%; unchanged. |
| OPEN.EXE | `vector3s_scale_shift12` | `0x80015be0 / 92` | 100%; unchanged. |
| OPEN.EXE | `vector3s_scale_shift12_alt` | `0x80015c3c / 92` | 100%; unchanged. |
| OPEN.EXE | `angle_within_tolerance` | `0x80015c98 / 60` | 100%; unchanged. |
| OPEN.EXE | `angle_mod_delta_le_half_turn` | `0x80015cd4 / 16` | 100%; unchanged. |
| OPEN.EXE | `vector_xz_to_angle` | `0x80015ce4 / 176` | 100%; unchanged. |
| OPEN.EXE | `fixed_vector2_length` | `0x80015d94 / 64` | 100%; unchanged. |
| OPEN.EXE | `matrix_interpolate` | `0x80019598 / 104` | 100%; unchanged. |
| OPEN.EXE | `lighting_set_color_matrix` | `0x80019600 / 44` | 100%; unchanged. |
| OPEN.EXE | `lighting_set_light_matrix` | `0x8001962c / 44` | 100%; unchanged. |
| OPEN.EXE | `fog_interpolate_near` | `0x80019658 / 68` | 100%; unchanged. |
| OPEN.EXE | `fog_set_near` | `0x8001969c / 40` | 100%; unchanged. |
| OPEN.EXE | `color_lerp_cvector` | `0x800196c4 / 136` | 100%; unchanged. |
| OPEN.EXE | `color_lerp_rgb555` | `0x8001974c / 152` | 100%; unchanged. |

## Verification

All nine forced-rebuilt runtime objects preserve their symbols and ordered
relocations. Forty-one functions retain 982 retail instruction words, including
delay slots, 54 direct calls and eight address materializations. The changed
GAME body is also token-identical after normalizing only comments and the
three parameter names. No function was newly banked.

Inventory, Ruff, whitespace and all 684 repository tests pass (89.937 seconds).
Modern checking retains the same 300 errors and 65/112 passing variants. Full
`kf build` retains source-data mismatches (PSX 0/1, GAME 9/42, OPEN 2/19) and
target-relink gaps (PSX 1/1, GAME 75/77, OPEN 34/38), with six conflicting
section bases and zero artifact failures. The source unknown count remains
ten lines containing fourteen identifier tokens.
