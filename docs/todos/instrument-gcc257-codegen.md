# Instrument the GCC 2.5.7 code-generation probe

## Idea

Build the Decompals GCC 2.5.7 PSX compiler from its pinned source revision with
host debug symbols and narrow machine-readable instrumentation. Continue to
compile reconstruction candidates with the existing retail-facing
`-O2 -G0 -mcpu=r2000` profile. The debug build applies to the compiler itself,
not to the reconstructed game.

Use the compiler's internal decisions to guide semantic source hypotheses:
which source values create pseudos, where their live ranges begin and end, how
stack slots are assigned, which hard registers reload selects, which
equivalences CSE substitutes, whether jump optimization can merge tails, and
how scheduling orders the final MIPS instructions. This should make
`kf hypotheses` an evidence-directed search rather than a broad source-feature
enumerator.

## Why this is promising

The pinned `probe-gcc257-o2-g0` rebuild already emits 106 of 108 eligible
OPEN functions exactly. That does not prove the historical compiler identity,
but it makes the rebuild's internal model highly relevant to the two remaining
residues.

`opening_ending_scroll_run` has matching semantics, calls, referents, and most
of its instruction stream. Retail reuses `t0` for both failed lighting-phase
constants, allowing final cross-jumping to share the phase-store/reset tail.
The probe assigns the first constant to `t1` and the second to `t0`, leaving two
extra CFG blocks. Existing RTL dumps show the split happens after allocation,
but not why reload made those hard-register choices.

`display_initialize` has matching calls and CFG. Retail retains an address
rooted at the first `DRAWENV.dtd` member across the drawing-environment calls
and accesses the two `dfe` fields relative to it. The probe initially discovers
the same equivalence, later substitutes the four accesses back to absolute
addresses, and lets the base die before allocation. This produces four extra
address pairs and a smaller saved-register frame.

## Source and attribution boundary

The current Nix inputs install binary release archives from Decompals
`old-gcc` 0.17. They do not install the corresponding compiler source tree.
Fetch and pin the exact source revision used to produce those archives, record
its hash, and audit the PSX target patches before interpreting a trace.

The original Psy-Q media supplies historical GCC 2.4.1 and 2.6.0 binaries, but
not their complete Sony build source. Instrumentation therefore explains the
Decompals GCC 2.5.7 probe. It must not be presented as proof of the exact
compiler or optimizer that produced `SLPS-00017`.

## Proposed implementation

1. Add a separately named debug/instrumented compiler package. Do not replace
   `cc1psx-257` or change any production matching profile.
2. Build the host compiler with debug symbols and minimal host optimization so
   it can be inspected with `gdb` without changing its target optimization
   flags or generated MIPS semantics.
3. Confirm the uninstrumented debug build emits objects byte-identical to the
   currently pinned native compiler on an exact control corpus, both remaining
   OPEN units, and each GAME unit selected for a matching campaign.
4. Add opt-in trace hooks around only the relevant compiler stages:

   - RTL/pseudo creation and source expression ownership;
   - stack-slot creation, size, alignment, lifetime, and reuse;
   - CSE equivalence creation, substitution, and invalidation;
   - local and global register allocation, including reference counts and
     call-crossing lifetimes;
   - reload candidate order, conflicts, costs, and selected hard register;
   - jump optimization and cross-jump equality/rejection reasons;
   - instruction scheduling dependencies, ready-list choices, and delay-slot
     placement.

5. Make trace output deterministic and machine-readable. Key every event by
   function symbol, pass, instruction/RTL UID, pseudo number, hard register,
   source hash, and compiler-source hash. Keep logging disabled by default.
6. Extend `kf hypotheses` with an optional instrumented compiler and trace
   directory. Rank final strict objdiff first, while also reporting explicitly
   selected internal features such as:

   - whether both lighting failure constants select the same hard register;
   - whether the lighting tails become cross-jump-equivalent;
   - whether the first `DRAWENV.dtd` base survives CSE and allocation;
   - whether later `dfe` stores use base-relative rather than absolute forms;
   - stack-frame and saved-register agreement.

7. Preserve complete traces only for the baseline, distinct internal-state
   frontier, and any exact result. Store generated traces under `build/`; commit
   only the instrumentation, parser, controlled fixtures, and durable findings.

## Controlled workflow

For each target, first capture the current source and strict object as the
baseline. Run one semantic axis at a time long enough to learn which internal
event changed, then compose only independently meaningful axes. A lower
intermediate objdiff score is useful when the trace moves a missing compiler
state toward retail, but the final retained game source must remain humane and
evidence-supported.

Use small synthetic functions to validate every trace interpretation before
applying it to OPEN or GAME. In particular, construct controls that deliberately:

- create or remove one call-crossing pseudo;
- force two equivalent constants to compete for `t0`/`t1`;
- make two branch tails cross-jump-equivalent before and after reload;
- preserve or destroy a struct-member address equivalence across calls;
- alter a real stack object's size without adding artificial padding.

Then compile the actual selected TU only. Do not run a full repository build
during hypothesis iteration. If a source candidate becomes strict 100%, apply
it normally, rebuild the affected unit, run the full `kf build`, and bank it
through the existing workflow.

## Success criteria

- Complete the remaining OPEN.EXE and GAME.EXE function matches at strict
  objdiff **100%**. Controlled probes, deterministic traces, compiler parity,
  partial score improvements and bounded explanations support the campaign;
  they do not satisfy this closure requirement. Verify each retained source
  against retail, preserve banked functions, rebuild the affected unit, then
  run the full build and bank only exact results.
- The instrumented and normal GCC 2.5.7 builds emit byte-identical objects when
  tracing is disabled and enabled.
- Traces are deterministic across repeated builds of the same source hash.
- Each reported allocation, reload, CSE, cross-jump, and scheduling fact is
  validated by a controlled probe.
- `kf hypotheses` can compare dozens of isolated TU states while retaining a
  small, interpretable internal-state frontier.
- For each remaining OPEN or GAME function, a trace explains the first decisive
  divergence and identifies a semantic source hypothesis, or records a bounded
  probe limitation without inventing a compiler mechanism. A limitation leaves
  that function open; it does not replace the strict 100% completion criterion.

## Non-goals

- Do not build the game itself in a debug or unoptimized configuration and
  infer retail stack layout from that output.
- Do not patch the compiler to force retail registers or instruction order.
- Do not add fake locals, volatile carriers, raw aliases, inline assembly, or
  dead source statements to influence code generation.
- Do not import Gruntz's MSVC-specific permutation or wall taxonomy.
- Do not claim historical compiler identity from agreement with the rebuilt
  probe alone.
