# GAME configuration panel: label ABI and input policy

## Function Match Plan at `f3edc49`

Retail hashes, both functions' six semantic views, the complete caller and
callee bodies, adjacent load/status boundaries, source history, shared menu
types and SDK pad/primitive provider evidence were inspected before editing.
The profile remains `probe-gcc257-o2-g0`, not historical compiler proof.

| GAME function | Snapshot | Evidence and first correction |
| --- | --- | --- |
| `8002589c menu_config_panel` | 1284 retail bytes; strict 30.713396%; 168-byte frame | Nineteen direct calls, nine validated address pairs, seven unreviewed direct internal jumps, no strings or indirect transfers. Sole hub caller supplies no arguments. Review the seven encoded jumps, then reconstruct the actual draw arguments and input constants. |
| `80025da0 menu_config_panel_draw` | 408 bytes; strict 100%; 32-byte frame | Two call sites in the panel, nine calls, eleven address pairs and one reviewed internal jump. Two 24-byte positioned labels occupy incoming +0/+24; a state pointer occupies +48. Four rows increment each label Y by 22, then four AddPrim calls use OT slot 3000. Preserve every raw instruction while correcting the shared signature. |

Both functions are game-owned menu policy, not SDK bodies. PadRead is the
independently attributed LIBETC PAD v1.17 entry, and AddPrim belongs to the
verified LIBGPU PRIM archive span. No provider source is changed.

The seven `j` sites are `25ba4 -> 25c88`, `25bd4 -> 25c88`,
`25c18 -> 25c50`, `25c48 -> 25c84`, `25c60 -> 25c88`,
`25d0c -> 259f0`, and `25d64 -> 25d74` (all addresses prefixed `800`).
Their delay slots respectively set row 4, clear row, scale row by four,
set confirm 1, store the toggled state, nop, and nop. All targets are decoded
instruction boundaries inside this one body. Review these physical R_MIPS_26
sites without promoting any pointer-table or unrelated-image candidates.

The caller has four signed-word states at sp+56 and two 24-byte labels at
sp+72/+96. Label coordinates and glyphs are all halfwords. The first label
is passed using paired halfword loads into a0-a3 and unaligned word copies
to outgoing +16/+20; the second is copied independently to +24..+47, and
the states pointer is stored at +48. This supports two by-value
`MenuGlyphString` arguments followed by `s32 *states`, not one four-byte-
aligned 52-byte struct with artificial reserved gaps. Replace that struct
with the already shared complete label type; preserve the helper's byte ABI
and its private mutable copies. Unused glyph capacity is not initialized.

Separate directly evidenced policy corrections follow the ABI comparison:
PadRead receives 1 at all three call sites; movement cues are 0, toggle and
confirm cues are 1, and the back cue is 2. On exit, the four state bytes are
published and music is compared through the actual reloaded player byte.
The exit publication block follows the frame/input loop in retail. Preserve
the observed masks, priority, row wrap, toggle join, confirmation redraw,
release waits and audio calls; do not infer a different cancel policy.

Compare from the first raw divergence after every focused compile. Keep the
draw helper exact, inspect all numeric calls/address pairs, and record the
panel's actual strict result before any banking. Run the full build and
repository checks before committing; no percentage alone closes this panel.

## Focused reconstruction

Reviewing the seven internal jump relocations leaves the original source's
strict panel score unchanged at 30.713396%; the draw helper remains 100%.
Splitting the arguments into the two supported label objects recovers the
retail halfword register packing and unaligned outgoing copies, as well as
the caller's state/label stack offsets. The remaining caller loop structure
and input constants are not yet corrected. The draw helper initially loads
its state argument too early and loses one load-delay nop. Retail loads the
state cursor only after publishing `current_poly_ft4`; preserve this actual
cursor initialization by using a separate advancing local derived from the
incoming state pointer at that point. Do not retain a regressed helper.

The separate cursor restores the draw helper's complete focused exact match.
Keep the two-label ABI and remove the artificial reserved-byte parameter
struct. The caller still uses the wrong three non-movement cue constants and
omits the argument on its frame-input PadRead call; correct those four directly
decoded constants next, without changing the control structure yet.

The corrected constants appear in their respective call delay slots and the
helper remains exact. The caller still places configuration publication
before the input handlers and joins back to a single argument-copy preheader,
unlike retail's post-loop publication and loop-tail copies. Express the
phase exit as `break` and move the unchanged publication/audio block after
the frame loop. This is a control-flow recovery, not a new exit condition.

The publication block now follows input handling, but the negative nested
tests still reverse handler layout and retain a separate row-result value.
Retail tests each positive edge in priority order, mutates the existing row
in its movement arm, and shares the toggle block between horizontal and
confirm inputs. Express that decoded positive `if/else if` chain directly,
with the shared toggle label in the confirm arm and in-place row updates.
At `25b78/7c`, the old current pad is copied before replacing it with the
new result. Use the same old/current snapshot as the inspected hub caller:
initialize current pad to zero, then copy it to previous before each read.
These facts explain control/value lifetimes; no new input policy is added.

The fresh native match after the positive input chain is 83.277260%, up from
30.713396%; the draw helper is still strict 100%. Retail's exit loads
`player_state.audio_music_enabled` at `25d2c/30` after its byte store, whereas
the candidate reloads the temporary state array and masks it. Replace both
exit tests with the actual player byte. This restores the missing ninth
address pair before further control-flow comparison.

The player-byte correction restores the reload's identity, but the compiler
shares its address across the store/load rather than issuing retail's two
absolute pairs. Keep the real reference; it is not permission to force a
reload or add volatility. The initial capture has another direct mismatch:
retail loads all four config bytes into temporary registers, stores all four
states, then retains state 1 in the first frame-begin call's delay slot.
Capture `music_orig = states[1]` after the four array assignments, rather than
loading the player byte straight into the long-lived capture.

The capture now occupies the frame-begin delay slot, though the temporary
registers still differ. The major remaining CFG difference is the common
draw argument preparation: retail has both a first-frame preheader and a
post-input copy block, joining partway through preparation at `259f0`.
The existing C places the entire draw at the loop head and emits only one
copy block. Express the first draw before the loop, then the same ordinary
draw/window/present sequence after each input iteration, as the neighboring
main-menu source also does. This preserves the dynamic call sequence and
lets the compiler merge the common continuation; do not manufacture a
second draw at runtime or manually split aggregate arguments.

The split first-frame/post-input source restores the complete 168-byte frame,
the original config-load/store/capture schedule, the two argument-preparation
copies, and their join at `259f0`; the helper remains exact. The confirm
handler still places the toggle before the exit arm. Retail's `25c40` takes
the non-4 row to the toggle, with the row-4 confirm arm falling through before
it. Spell the row-4 arm first and the shared toggle in its `else` arm.

That arm order recovers all handler destinations and the shared exit-phase
assignment. The first remaining executable difference is the initialization
order at `258cc/d0`: retail clears current input before setting phase -99.
Move the existing phase initialization after the input clear. This is one
observed statement-order correction, not a declaration/register search.

Initialization now follows retail order, but phase and the state-array base
still exchange s5/s6. Do not add a pointer alias or constant carrier to steer
those registers. At exit, retail reads the published music byte once and
uses that snapshot in both comparisons. Test one genuine `music_current`
snapshot after publication instead of repeating the field expression in
both conditions. Its only purpose is to represent that observed read and
the two consumers; retain it only if the raw result supports the hypothesis.

The snapshot produces the same focused difference and is removed: there is
no evidence for an additional source local. A final bounded publication
control uses the same ascending field/array order as the input copy
(`96,97,98,99`), rather than retaining a source order chosen to mimic the
scheduled retail stores (`97,98,99,96`). The four independent byte stores
and subsequent read are unchanged semantically. Compare this single natural
copy order; do not enumerate store/declaration permutations.

The ascending copy restores every exit instruction and all nine ordered
address pairs. The full 1284-byte body now differs only at nine s5/s6 uses;
all branch destinations, calls, immediates, and other words agree. One
source-family control remains: the inspected `menu_root`, `menu_save_panel`,
and `menu_load_panel` initialize cursor, confirm, and current input at their
declarations, declare previous input next, then initialize the result phase.
Use that same ordinary initialization idiom here, with no extra declaration
or value and no search over alternative orders. This removes the current
separation between declarations and their known initial values. If the raw
words remain different, record the register-role residue without inventing
a compiler cause.

## Final reconstruction result

The initialized-local idiom is byte-identical to the preceding control and is
kept as ordinary source cleanup, not a matching mechanism. The fresh strict
match is **99.859810%** for `menu_config_panel`, versus **30.713396%** at
the campaign start. Its draw helper remains **100%**. Neither the partial
panel nor a loose comparison is banked as exact.

Both bodies have retail's complete extent (1284 and 408 bytes) and frame
sizes (168 and 32). Raw relinking reproduces both curated target bodies;
the independently compiled helper equals every retail word. The panel's
19 numeric calls and nine ordered address pairs agree, as do all decoded
branch/jump/return destinations. The seven reviewed jumps keep their
decoded targets and original delay-slot instructions.

Exactly nine panel words differ, at offsets `+034`, `+0b0`, `+12c`,
`+1c0`, `+230`, `+2c4`, `+3b4`, `+3e8`, and `+44c`. Retail uses s5 for
the four-state array and s6 for phase; the candidate exchanges those roles.
All other words, including both save/restore sets and all constants, match.
This is an unattributed register-role residue, not a proved compiler wall.
There is no source justification for fake locals, forced registers, or a
profile change. The retained unused `lui s3,0xffff` is emitted naturally by
this C; no source constant carrier was introduced.

The shared label now has compile-time size, halfword-alignment and prefix
checks. Two focused static controls cover the helper's raw ABI, numeric call
targets (including a wrong-target negative control), all pad/sound arguments,
and the seven reviewed jumps/delay slots. The corrected inventory records
the two by-value label arguments and the actual exit music policy.

Verification: a fresh focused build recompiles the panel and shared-header
consumers; the independent raw compile equals the production object for both
functions. Comparing all 484 score rows against `f3edc49` finds exactly one
change: this panel's 30.713396% to 99.859810%. No banked, OPEN, or vendored
control regresses. Eligible exact counts remain 395/471 (GAME 296/362,
OPEN 98/108, PSX 1/1); this campaign adds no new exact function.

Ruff, all **651 tests**, and `git diff --check` pass. Full `kf build` retains
the existing data gates: source-data 7/60 (GAME 5/40, OPEN 2/19, PSX 0/1),
SDK config-data 4/4, and target relink 110/116 (GAME 75/77, OPEN 34/38,
PSX 1/1). Six units still have conflicting section bases, with zero artifact
failures and incomplete known-reference ownership. No gate or relocation
comparison was weakened to obtain the improved text result.

The bank refresh selects only GAME `80025da0`, the previously exact helper
with its corrected source signature. The partial panel's bank row is not
updated or promoted.
