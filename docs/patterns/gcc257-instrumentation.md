# GCC 2.5.7 observational instrumentation

## Scope and source pin

This explains the Decompals GCC 2.5.7 PSX probe. It does not identify Sony's
compiler for SLPS-00017. Production profiles and `cc1psx-257` retain the pinned
release binary. The optional `gcc257Debug` and `gcc257Instrumented` Nix packages
build the host compiler with `-O0 -g3`; reconstructed C still uses
`-O2 -G0 -mcpu=r2000`.

Decompals release 0.17 resolves to
[`b74211c9d959e9724802f3177c8229cd67202c87`](https://github.com/decompals/old-gcc/tree/b74211c9d959e9724802f3177c8229cd67202c87).
Its `gcc-2.5.7-psx.Dockerfile` and release workflow identify the original GCC
archive and ordered patches. `nix/gcc257.nix` pins both downloaded archives:

| Input | SHA-256 (Nix SRI) |
| --- | --- |
| Decompals recipe archive at that revision | `sha256-De+MYfwfyABo2JUvsIWjxNu0Bp8fNi1wxpiWn8O5bMI=` |
| Original `gcc-2.5.7.tar.gz` from FUNET | `sha256-Y0W+QiNeXsTESNZYflUoPaPuV3YdtdINegpI1jmH/I4=` |

The PSX patch adds `config/mips/psx.h`, `xm-psx.h` and target recognition.
It sets little endian, soft float, ISA 1, unsigned plain char, byte `wchar_t`,
and the PSX stack-argument adjustment; it inherits `mips.h`. The separate
`mips-2.5.7.h.patch` changes driver assembler specifications and removes a
required `LINK_SPEC` diagnostic. Host patches replace varargs, repair an
obstack assignment, and adapt libc/error declarations. Their original order
is preserved. None of these upstream changes is silently treated as a Sony
source release.

The host remains i686/32-bit, as in the release recipe. The rebuild restores
`-fcommon` for old tentative definitions and permits old C declarations on
the pinned modern host GCC. Nix's automatic config.sub replacement is disabled
to preserve PSX support. Generated headers are completed before parallel
compilation because the old makefile omits some dependencies. Symbols are not
stripped; debug paths resolve to the installed source tree.

`share/gcc257/provenance.json` records the recipe revision and compiler-source
fingerprint. The fingerprint hashes canonical JSON containing both archive
hashes, the target, and the exact instrumentation patch/header/logger contents.
It identifies this source composition, rather than the host executable bytes.

## Function Match Plan: trace the two OPEN residues

This campaign develops tooling and uses disposable hypotheses before keeping
retail-verified reconstructions. Completion requires strict 100% for all
remaining OPEN and GAME functions. The fresh baseline build contains 49
non-exact GAME functions and two OPEN functions; generated census and objects
are under `build/objdiff`.

Before the OPEN
trial, hash validation, six image-qualified semantic views, focused objects,
source history, existing source-shape evidence, SDK interfaces and ownership
were refreshed. Captures live under `build/gcc257/evidence`.

- OPEN `80016adc display_initialize`, 472 retail / 484 probe bytes, strict
  **92.177960%**: eighteen direct calls, two internal jumps, seven CFG blocks,
  sixteen address pairs, no candidate references or strings. The sole caller
  is `opening_run` at `80015718`, forwarding the mode. Retail saves four s
  registers in 48 bytes; the probe saves three in 40. Inspect creation and
  substitution of `open_graphics_runtime+0x2003e`, the first DRAWENV.dtd member,
  and stores to offsets `0x2003f` and `0x2009b`. Test authentic SDK RGB
  expressions, ordered chained field assignments and the existing typed
  pointer initialization lifetime, one axis before composition. These are
  bounded controls of source-expression ownership and CSE, not evidence that
  a winning syntax was the original source.
- OPEN `80014e28 opening_ending_scroll_run`, 1944 bytes, strict
  **97.952675%**: forty direct calls, eight internal jumps, thirty-one address
  pairs, no strings/candidate references. The sole caller is `opening_run` at
  `80015890`. Both objects have a 264-byte frame and ten-register detached
  epilogue. Trace the named `lighting_phase` through its two constant writes,
  reload and the tail comparisons. Keep the real MATRIX, VECTOR snapshot,
  colors, array extents and typed state domains. A source hypothesis must
  explain a real lifetime or value use, not add an extra reload to rotate
  register selection.

The two bodies implement game scene/display policy. Sony graphics, GTE and
audio calls remain external vendored boundaries. No library body, inventory,
data owner or reconstruction is changed by the instrumentation.

## Running the probe

```sh
nix develop .#instrumentation
kf init --retail-dir /path/to/verified/disc
kf configure
# Generate the selected image's target objects, without building every TU.
ninja -f build/build.ninja build/delink/open/.delink.stamp
python3 tests/gcc257_trace_smoke.py \
  --debug "$(command -v cc1psx-257-debug)" \
  --instrumented "$(command -v cc1psx-257-trace)" \
  --output build/gcc257/verification --open-units
```

For a direct preprocessed compile, set `KF_GCC257_TRACE` to a fresh JSONL file
and `KF_GCC257_SOURCE_SHA256` to the lowercase SHA-256 of the C source.
`KF_GCC257_TRACE_FUNCTION` optionally selects one symbol. Missing or invalid
source hashes fail explicitly. Logging is otherwise disabled. The Python
compiler integration passes these settings only to the primary cc1 invocation;
preprocessing and auxiliary data-size probes cannot overwrite the trace.

`kf hypotheses` accepts `--instrumented-compiler` and `--trace-dir`. A traced
manifest must explicitly select `trace_features`, each with a unique `name`:

| `kind` | Required selector | Report |
| --- | --- | --- |
| `frame` | none | Entry prologue through the first transfer's delay slot; unavailable if allocation is behind a branch |
| `source_lifetime` | `source_value` | Named source pseudo's allocation, references and crossed calls |
| `constant_registers` | `source_value`, distinct `constants` | Hard registers after reload and observed spill-tail equality/rejection |
| `address_lifetime` | `symbol`, `offset`, `members` | CSE root, relative/absolute member stores, accepted rewrites and allocation |
| `crossjump` | none | Whole-function equal/reject/apply event counts |
| `spill_registers` | none | Reserved registers, maximum search count and per-class requirements |

The baseline source and complete object are captured before the batch, along
with the strict target. Every candidate's traced object must equal the normal
compiler's complete object. Strict objdiff ranks results first. Full traces
are retained only for baseline, the highest-scoring representative of each
selected internal state, and every exact result. UIDs and event numbers remain
in evidence but do not split the frontier. Unavailable/ambiguous selectors
remain explicitly unavailable. No candidate is applied or banked automatically.

Frame comparison uses the shared MIPS control decoder and includes the first
transfer's delay slot, where a saved-register store can be scheduled. It does
not follow branches or propagate divisor constants. A transfer before stack
allocation therefore makes the frame unavailable unless it is a complete leaf
return; two unavailable results produce null agreement fields. Missing delay
slots and annulled/control-transfer slots also remain unavailable. The observed
save list describes this entry prefix, not path-dependent later saves.

## Event contract

Every JSONL row carries schema, increasing sequence, function symbol, pass,
instruction UID, pseudo, hard register, source hash and compiler-source hash.
Zero UID and `-1` registers mean not applicable. MIPS's first pseudo is 67.
Expression enter/result events are nested using the `expression` sequence;
`source.value` adds declaration name/UID/line without treating a compiler
pseudo as a unique original variable. `parser_line` is the parser cursor,
not an invented optimizer source location. RTL serialization includes mode,
four flags and operands, omitting host pointers, instruction-list edges and
source filenames. Operand `u` references use UIDs. A freshly allocated
SEQUENCE has no safe operands yet and is omitted from `insn.create`; later
snapshots contain its completed form.

The event-specific `values` triplet is:

| Event | Values |
| --- | --- |
| `expression.enter` | target mode, expansion modifier, declaration UID or -1 |
| `source.value` | declaration UID, declaration line, reserved |
| `stack.allocate` | rounded bytes, alignment bytes, frame offset |
| `stack.acquire` / `stack.release` | object bytes, lexical level, keep/reserved |
| `stack.reuse` | requested bytes, available bytes, lexical level |
| `cse.insert` | expression cost, equivalence-class head cost or -1, reserved |
| `cse.remove` | expression cost, reserved, reserved |
| `cse.constant` | operand index, reserved, reserved |
| `change.propose` | change index, grouped flag, reserved |
| `change.commit` / `change.cancel` | change index, group size, reserved |
| `allocation.pseudo` / `global.select` | references, calls crossed, live length |
| `local.select` | quantity birth index, death index, calls crossed |
| `local.candidate` | suggested-only flag, birth index, death index |
| `global.candidate` | search pass, candidate order, register class |
| `reload.spill-cost` | cost order, weighted uses, forbidden flag |
| `reload.spill-order` | candidate order, reserved, reserved |
| `reload.required` | reload number, register class, operand lifetime kind |
| `reload.requirements` | register class, simultaneous registers, register groups |
| `reload.reserve` | spill-array index, global-allocation flag, reserved |
| `reload.search` | reload number, previous spill index, spill-register count |
| `reload.candidate` / `reload.select` | reload number, search pass, spill index |
| `reload.usage` | already used, used for inheritance, in requested class |
| `reload.free` | operand number, reload lifetime kind, actual availability result |
| `crossjump.equal` / `crossjump.reject` / `crossjump.result` | remaining minimum, reserved, reserved |
| `schedule.dependency` / `schedule.graph` | dependency type, block (graph only), reserved |
| `schedule.cost` | dependency latency, dependency type, reserved |
| `schedule.ready` | sorted ready index, priority, scheduler clock |
| `schedule.select` | scheduler clock, block, priority |
| `delay.place` | slot index, sequence length, availability flag |

`x` and `y` retain the operands or paired instructions. `reason` identifies the
observed branch; `context` distinguishes `cse.fold-address` substitutions.
The preallocation scheduler runs backward: selection order is not final
execution order. Graph events also capture dependencies already constructed
by flow analysis; add-dependence events alone would miss those edges.
Candidate mask events describe those masks only, not a fabricated complete
rejection reason. Reload availability wraps the original predicate once.
No hook allocates target RTL or invokes an optimizer predicate again for logging.

## Verification and findings

The native compiler, debug rebuild, disabled logger and two enabled logger
runs emit identical complete objects for the controlled corpus and both OPEN
target units. Repeated traces agree byte for byte. Eighteen synthetic functions
check the source-lifetime, CSE, stack, allocation, reload, cross-jump and
scheduling observations against emitted instructions. This verifies the probe;
the required remaining-function campaign is still incomplete.

Four GAME functions are restored to strict 100%, including complete
relocation-resolved retail-word controls:

| Function | Baseline | Kept correction |
| --- | --- | --- |
| `menu_save_panel` | 98.226950% | Preserve the promoted slot argument before the signed-halfword API boundary. |
| `menu_load_panel` | 97.727270% | The same slot argument correction. |
| `player_equip_weapon` | 82.377050% | Unsigned word arithmetic for decimal quotient and remainder. |
| `actor_spawn_action_effect` | 98.176895% | Retain the normalized input's lifetime and decode its kind at promoted width. |

All exact neighbors retain 100%. The full rebuild reports GAME 317/362 and
OPEN 106/108 exact; 45 GAME and two OPEN functions remain. Data placement,
ownership and reachability checks still fail. The initial Nix failures exposed
an omitted `KfScreenRect` inventory entry and stale typed signature fixtures;
those now follow the existing checked layouts and curated declarations.
Whole-source modern checking still encounters legacy empty-parameter SDK
declarations, while dedicated positive/negative enum controls confirm that the
promoted argument aliases retain their distinct domains. No partial OPEN trial
has been retained or banked.

GDB stops inside `find_reloads`, displays arguments and the reload/global
allocation backtrace, and resolves source lines to the installed pinned tree.
The final 32-state OPEN display matrix retains four complete traces including
the baseline, with no exact result. Its best score remains 92.177960%.

`ruff check scripts tests`, `git diff --check` and `nix flake check -L` pass.
The local retail-enabled suite passes 698 tests with nine optional Rust-driver
skips; the subsequently added promoted-domain control also passes. Nix runs
the resulting 699-test suite with 144 local-artifact-dependent skips, along
with compiler parity, trace controls and the other flake checks. The four
restored GAME functions are selectively banked. The full `kf build` rebuilds
the affected objects and retains all 424 exact game functions across the three
images, but exits nonzero on the documented data/ownership/placement gaps.

### Follow-up match plan: OPEN mode dispatch

The unchanged `80016adc` snapshot above now has direct CSE decision evidence:
the second CSE pass folds four `dfe` addresses from the first `dtd` root into
absolute constants inside `find_best_addr`, before equivalence cost selection.
The two retail equality dispatches establish their CFG but do not distinguish
an `if` from a two-arm `switch` in source. Test that source distinction at each
dispatch independently, then together. All arms retain their original calls,
field-write order and constants. Inspect whether the early jump/CSE paths
change the member lifetime; a different frontend shape or score alone is not
a retained correction. Keep the authentic SDK field types and original target
optimization flags.

All four dispatch states are byte-identical at 92.177960%, with one selected
trace state. Disposable pass-disable controls confirm that skipping only
`cse-skip-blocks` is also identical. Disabling both extended traversals changes
the surviving base to whole DRAWENV pointers and gives 93.610170% / 472 bytes,
still with the wrong 40-byte frame. Disabling the second CSE pass also regresses
the exact `render_initialize` control. None is a retained profile or source
change.

### Follow-up match plan: ending spill requirements

For the unchanged OPEN `80014e28` snapshot, the refreshed resolved baseline
first differs at `+0x2b4`: it computes `top_color`'s stack address in t1,
where retail uses t0. The earlier camera-path argument now agrees with retail.
Reload then rotates through two spill registers for subsequent writes,
including lighting constants 1 and 2. Retail uses t0 for the corresponding
spilled values and v0 for comparison constants. Add read-only observations of
actual per-instruction requirements and reserved spill registers to identify
the first demand for a second register.

The sequence delay and camera completion tests compare actual word values
against -1. Test each equality dispatch as a one-case switch independently;
retail's direct branch constrains behavior but does not uniquely establish
that source construct. Preserve the existing delay decrement, state writes,
camera-step call and all other operations. No extra value or reload is added
to alter the round-robin position. Compare strict bytes, every exact sibling,
and whether the recorded maximum requirement changes before composing axes.

Both one-case switch trials are byte-identical. The requirement trace now
identifies exactly one instruction demanding two GENERAL_REGS: the sequence
delay comparison (UID 589), whose operands are spilled pseudo 78 and constant
-1. The lighting writes themselves each require only one. Test the countdown
as a post-decrement zero test, and separately as an explicitly assigned
remaining-count result used by the branch. Both express the same real update
and decision; the latter temporary is the branch's actual computed value,
not an added register carrier. Compare the branch with retail's decrement,
literal -1 and delay-slot store, rejecting any changed countdown behavior.

The assigned-result form is unchanged. The post-decrement-zero form reaches
99.372430% with the same 1944-byte extent and 264-byte frame. It requires only
one spill register (t0), restores both lighting constant registers and their
shared tail, and recovers the earlier stack-address/reload sequence. Its own
countdown uses a pre-decrement value copy and zero comparison, whereas retail
compares the decremented value with -1. Keep this as a disposable diagnostic.
Next test the equivalent zero-biased/complement tests and an unsigned word
countdown. Equality and word stores establish width but do not distinguish
signed from unsigned countdown wrap; no other state's type changes.

The biased zero test reaches 99.781890%. A complete relocation-resolved raw
comparison finds nine unequal words: seven lighting/sequence dispatch register
or branch-target words, plus replacement of the literal -1/inequality pair
with add-one/zero-test instructions. Its initial setup and shared lighting
store/reset tail now agree. The complement test reaches 99.670784%; unsigned countdown
is unchanged. Test the direct result of compound and ordinary decrement
assignment next: these preserve retail's -1 test while distinguishing
assignment-result expansion from prefix-decrement expansion in GCC.

Both direct assignment-result forms are unchanged. Next distinguish a branch
on the completion predicate's value from direct conditional-jump expansion:
compute the actual `sequence_complete` predicate locally, or dispatch on that
predicate with a one-case switch. The predicate is the actual state-transition
decision, with no extra constant carrier or operation beyond the comparison.

Those predicate forms also remain non-exact. Earlier pass snapshots locate the
cause before allocation: the loop pass hoists the ordinary -1 pseudo (156),
then combines the camera-completion and scroll-wrap constants into it. It has
seven weighted references, crosses nineteen calls and receives no hard
register; reload later substitutes its constant equivalence. This is why the
sequence-delay branch requires a second spill register. Test the real infinite
frame loop as `while (1)` and `do ... while (1)` next. Retail's unconditional
back edge and detached epilogue do not distinguish these source constructs;
the experiment checks their loop notes and invariant lifetimes without
changing any operation or adding a control-flow edge to the game semantics.

All three structured infinite-loop forms are identical. Test the actual
unconditional frame back edge as an explicit label/goto, which changes GCC's
frontend loop notes without adding a branch or an operation. Inspect all other
loop invariants, including the two real color-object addresses: losing their
retail setup lifetimes would reject the simple label form.

The label form gives 93.792180%; it is not retained. The nine-word ending
residue remains open, as does the display initializer.

The original ending-loop body remains the baseline. Next test the two actual
state dispatches independently as `if`/`else if` chains. Retail's equality
branches support the same two active arms and inactive fallthrough, while
source history does not prove the original switch construct. Keep the signed
word countdown and literal -1 comparison, all state widths, update ordering,
calls, and referents. Inspect whether the different dispatch expansion changes
the loop-hoisted -1 lifetime and two-register reload requirement before
considering a composed state. Do not retain a changed CFG merely because its
allocation state is closer.

The lighting and sequence chains give 96.625510% and 96.078186%, respectively.
Both retain the baseline's selected lighting-constant and two-spill-register
state, so neither supplies an independent correction to compose. Both are
rejected; the production ending source remains at 97.952675%.

### Function Match Plan: GAME save/load argument promotion

The required six views, full CFGs, sole callers, adjacent functions, history,
shared summary/slot types and SDK boundaries were refreshed for GAME
`800250c4 menu_save_panel` (1128 bytes, 98.226950%) and
`8002552c menu_load_panel` (880 bytes, 97.727270%). Both have 120-byte frames;
the save panel has 58 proven calls and ten validated internal jumps, the load
panel 42 calls and nine jumps. Neither has strings, address pairs, indirect
transfers or candidate outgoing references. They implement game menu policy;
PadRead/memset and card SDK providers remain external vendor boundaries.

Retail passes `cursor+1` directly in the write/read call's delay slot at
`800252fc`/`800256c4`. Commit `5aca551` added explicit decoding to the signed
halfword storage type, introducing two narrowing shifts and moving the call
setup. The callee's signed-halfword slot contract remains correct. Pinned
`mips.h` defines `PROMOTE_PROTOTYPES`; `c-typeck.c` promotes the formal type to
int before converting an implicit argument. An explicit earlier short cast
therefore has a different effect.

Test the existing `KF_ENUM_PROMOTED` idiom for the actual arithmetic slot
argument, retaining the modern enum domain, its signed-halfword API and byte
directory storage. Rebuild the caller TU and compare both complete bodies,
their seven saved-register/return shapes and every ordered call/jump. Require
normal/traced object parity and preserve the exact hub plus both exact save
workers. Do not change their signatures or narrow the cursor itself.

Both corrected panels are strict 100%. The original hub remains 100%, and the
drop-item panel retains its baseline 99.547850%. Complete raw-word tests verify
the bodies, ordered calls, exits and delay slots. A shared `KfSaveSlotArgument`
uses the existing promoted-domain idiom solely for the arithmetic argument;
the slot API and directory layouts are unchanged.

### Function Match Plan: GAME weapon path arithmetic

GAME `80016a30 player_equip_weapon` is 244 bytes, currently 82.377050%, with a
24-byte frame, one saved return register, four proven calls, twelve validated
address pairs and no candidate references. The four callers pass zero, 255,
an unsigned stored item byte, or an explicitly masked menu result. The entry
stores a byte and masks the parameter to 255; its signature remains `KfItemId`.
The adjacent equipment and attack functions constrain the same player fields.
This is game asset-path construction; `exit` remains a vendored boundary.

Retail uses one unsigned division by ten and consumes both quotient and
remainder to write decimal filename digits. Commit `3da4c19` introduced explicit
byte casts before that arithmetic. Test word-width unsigned encoding for the
two divisions, preserving the byte signature, byte storage and typed item
domain. Compare from the division instruction, all twelve referents, and each
neighboring function. The direct `divu` instruction supports this signedness
correction independently of its strict score.

The unsigned word expressions restore strict 100% without changing any other
function in the unit. Baseline expansion contains signed quotient/remainder
and unsigned quotient/remainder RTL; CSE cannot combine those pairs. The
corrected expansion contains two unsigned pairs, reduced to one by CSE. The
complete raw retail comparison includes all calls, addresses and return words.

### Function Match Plan: GAME packed actor effect code

GAME `8002edd4 actor_spawn_action_effect` is 1108 bytes, currently 98.176895%,
with a 200-byte frame and ten register saves. Its sole caller loads a byte
from the selected action parameter, then passes a word attachment index.
The body tests packed bit 0x20, masks to five kind bits and retains that
normalized value across eleven calls. There are ten validated internal
jumps, three validated address pairs and a jump-table dispatch whose
twenty rows remain candidate pointers in the semantic census. Do not promote
their reachability solely from the known switch shape.

The complete CFG, call sites, structures, adjacent functions, ordered SDK
calls and history were reviewed. This is game spawning policy; matrix/GTE
providers remain external. Commit `43b7f88` separated the decoded kind from
the mutable input. Retail preserves the input in s2 before its flag test,
then masks s2 in place; the current build first keeps it in a0, omits that
copy and moves the mask into a delay slot. Test explicit normalization of
the actual packed input before decoding its kind, keeping both semantic
domains and every operation. Trace their expansion/allocation lifetimes and
compare the complete body plus all exact neighbors before retaining a form.

Separate normalization followed by a local decoded kind is byte-identical to
the baseline. Next keep the normalized packed input as the actual retained
value and decode at its switch/comparison/API boundaries. This removes the
separate value introduced by the naming pass, preserving explicit modern enum
checks and the original in-place mask. No artificial value or call is added.

Byte decoding at every boundary gives 86.588450% and is rejected. Test the
existing promoted-enum idiom for those normalized word-valued expressions:
the mask already limits them to 0..31, so an explicit second byte conversion
does not represent a separate storage boundary. The modern view retains the
effect-kind domain; the retail view preserves the arithmetic expression width.

The promoted boundary form restores strict 100%, including all 1108 linked
bytes and ordered relocations. The trace changes the original input's global
allocation from a0 (three references, `calls_crossed=0`) to s2 (29 references,
`calls_crossed=12` at that compiler stage). Its real in-place mask and lifetime
now reproduce retail's initial copy and scheduling. All exact neighboring
functions remain exact. The curated input signature now reflects the source's
existing word parameter; its sole caller's byte load alone did not prove an
eight-bit formal parameter.
