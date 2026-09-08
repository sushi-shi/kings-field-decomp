# GAME display-owner traces

## Function Match Plan

Continue from `494484d` with hash-verified GAME.EXE and the unchanged pinned
GCC 2.5.7 profile. Refresh the six semantic views for display initialization,
frame setup and presentation; read all five caller windows, adjacent boundaries,
current source/history, complete SDK object declarations and prior owner controls.
The first two frame helpers are exact controls for the initializer's shared state.

| GAME function | Retail bytes; strict baseline | Preserved evidence |
| --- | --- | --- |
| `8001bb94 display_initialize` | 332; 94.253010% | No arguments; 32-byte frame, thirteen direct calls and fourteen validated address pairs. Four five-argument SDK setup calls use 320x240 and opposite framebuffer halves. Retail retains first DRAWENV.dtd through PutDispEnv at +162 and the fog word store at +18538. |
| `8001bfb8 display_begin_frame` | 152; 100% | No arguments; 24-byte frame, one call and seven validated pairs. Preserve byte toggle/reloads, 12-byte primitive-buffer and 64 KiB ordering-table strides, 16384-entry clear, cursor publication and three word resets. |
| `8001c050 display_present_frame` | 152; 100% | No arguments; 24-byte frame, five calls and four validated pairs. Preserve synchronization, repeated byte-index reads, SDK strides 92/20 and the ordering-table last-entry address. |

All three have one straight-line block, one return with frame restoration in
its delay slot, no strings, candidate references or unresolved transfers. Their
startup/frame policy is game code. The supplied Release 2.5 LIBGPU EXT/SYS and
LIBGTE GEO/REG services retain their existing vendor evidence and real SDK types.
SetFogNear takes two signed words, matching the stored signed fog distance.

The existing complete-owner fixture preserves both exact frame helpers but
leaves initialization at 336 bytes, first diverging at +a4 in the dtd base
register. Its registry/projection/morph byte spans remain explicitly unresolved.
Do not replace them with inferred capacities or migrate production storage here.

Capture current native/traced canonical and owner objects. Test one real data-flow
axis: after publishing fog distance 11000, pass that stored field to SetFogNear
instead of repeating the constant. No intervening call can change it; the API
receives the same value. Test this in both existing declarations, preserving all
other expressions and source order. Inspect CSE and allocation at the first
changed instruction; reject an unchanged or worse result as an explanation of
retail's retained member address. Compare complete linked bodies and ordered
targets, including every unchanged unit sibling and a wrong-root control.

Only strict objdiff 100% closes a function. A temporary raw result or trace
explanation does not bank this provisional owner. Any retained production edit
requires fresh focused matching, full build and repository verification.

## Verdict

The stored-field argument produces identical complete function words, calls and
address targets in both declarations. All target code/data sections also remain
identical; source-directory metadata differs. Native and instrumented compilation
of each individual source produces identical whole ELFs. No production edit is
retained from this trial.

In the complete-owner trace, first CSE derives both the DISPENV argument at
DTD+162 and the fog store at DTD+18538 from pseudo 75. Second CSE substitutes
the fog destination back to the absolute owner+149672 expression. The DTD
pseudo consequently has three references, crosses zero calls, has live length
22 and receives a0. The canonical separate-owner form has no remaining
allocation references to that pseudo. In both forms, pseudo 71 retains height
240 in s0 across the three setup calls; fog value 11000 is in v0.

The trial creates pseudo 91 for the additional fog-field address at expansion.
First CSE folds its load back to constant 11000, and it has zero allocation
references. Both DTD and height lifetimes are unchanged. This records an
observed probe decision, not historical compiler attribution or a source remedy.

| Function | Final verdict |
| --- | --- |
| `display_initialize` | Canonical strict 94.253010%, 340 bytes; owner diagnostic 336 bytes versus retail 332. First difference remains +a4: canonical uses at and owner uses a0, where retail uses s0. Stored-field argument changes neither body. Still open. |
| `display_begin_frame` | All 152 retail bytes remain exact with either declaration and argument form. |
| `display_present_frame` | All 152 retail bytes remain exact with either declaration and argument form. |

All eighteen functions in each compiled TU are checked: the other seventeen
functions reproduce every retail word. The delinked target independently
reproduces retail, all ordered direct calls agree, and shifting the candidate
owner by four changes each selected body while preserving its calls. The owner
initializer's additional absolute fog address remains the documented fifteenth
materialization; canonical source has sixteen, versus retail's fourteen.

Generated dossiers, compiler traces, complete raw comparisons and allocation
snapshots live under `build/gcc257/display-owner/`. The current source/config,
bank rows and compiler profile are unchanged, so the verified `494484d` full
build and 703-test checkpoint still apply. This refresh does not add three
previously unreviewed consumers to the older owner audit. GAME remains 324/362
and OPEN 106/108 exact; all forty remaining functions still require strict 100%.
