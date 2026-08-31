## Objective and authority

- Reconstruct the original Japanese King's Field (`SLPS-00017`) sources so the
  pinned PlayStation toolchain can emit objects matching the retail programs.
- This repository contains three independent linked programs: `PSX.EXE`,
  `GAME.EXE`, and `OPEN.EXE`. An address is never an identity without its image.
- The PS-X EXEs retain linked bytes, not symbols or relocation tables.
  Functions, data, labels, relocations, and TU ownership under `config/retail/`
  are curated working models, not recovered ground truth.
- Correct source structure outranks a transient fuzzy score. Types, widths,
  ownership, control flow, constants, calls, GTE operations, and referents must
  not be distorted to protect a metric.
- Exact means objdiff `100%`. `--loose` is display-only and is never a closure
  or banking criterion.
- Exact compiler, assembler, linker, and optimization attribution remain open.
  `probe-gcc260-o2-g0` is a productive probe, not historical proof.

## Environment and evidence discipline

- Work in `nix develop`; use only toolchain programs supplied by the flake.
- Initialize hash-identical retail files with `kf init` before binary queries.
- Select one image explicitly for every semantic query:
  `kf sema --image psx|game|open ...`.
- Before editing a function, inspect its retail disassembly/CFG, callers,
  callees, strings, data references, relocation evidence, adjacent functions,
  source history, and current match state.
- Use the evidence tiers literally: `proven` is decoded direct control flow,
  `validated` passed conservative delinker checks, and `candidate` remains a
  hypothesis. `--confirmed-only` is a filter, not a promotion mechanism.
- Raw instructions, immediate values, delay slots, and ordered relocation
  targets decide correctness. A percentage alone does not.
- A Ninja no-op is not verification after source/config changes. Rebuild the
  affected unit, then run the full `kf build` before handoff or commit.

## Matching campaign

- Use the project `matcher` skill for reconstruction. Do not introduce separate
  wall-identification or permutation doctrine until real matching produces a
  repeatable residue corpus.
- Choose a related campaign from confirmed calls, shared data, repeated source
  shape, or evidence-backed adjacency. Address proximity alone does not prove a
  TU, but it can define a pilot band whose ownership remains explicitly WIP.
- Publish a Function Match Plan before the first edit. A bulk family may share
  one campaign plan, but every function needs its own evidence snapshot and
  final verdict.
- Reconstruct semantics first: signature and widths, structure fields, call
  set, constants, CFG, return form, data identity, and delay-slot schedule.
- Compare from the first real divergence after every focused build. Triage in
  this order: referent/relocation, call set, CFG, then directly evidenced type
  and instruction-selection differences. If those agree, record an
  unattributed codegen residue; do not name a register/scheduler wall.
- Compiler-wall taxonomy is a TODO: first match a representative function
  batch, catalogue recurring residues, prove the exact GCC/profile, then read
  the corresponding MIPS optimizer/backend and validate mechanisms with
  controlled probes. MSVC internals and levers do not transfer.
- Treat a lower intermediate score as an experiment, not automatic
  falsification, when an independently evidenced source fact moved code toward
  retail. The final kept source must be humane and must not regress a banked
  function.
- Keep one function per unit until original TU grouping is independently
  supported. Contiguous linking is not sufficient TU evidence.

## Game code versus vendored code

- Do not reconstruct Sony/Psy-Q, GNU runtime, or other vendored library bodies
  as game progress. Identify them in `functions_vendored.tsv` with reproducible
  archive/header/signature evidence and exclude them from the denominator.
- Dense COP2/GTE wrapper families, libc-shaped routines, SDK callbacks, and
  startup glue are library candidates even when the current FID census missed
  them. Check the supplied Psy-Q archives and headers before writing source.
- Game functions are reconstructed as C unless retail/source evidence proves a
  hand-written assembly owner. Never use assembly merely to bank instruction
  bytes.
- SDK macros or inline GTE forms may legitimately emit raw COP2 instructions;
  prove the header/macro identity and version rather than hand-transcribing
  `.word` instructions into game source.

## MIPS-I and PlayStation rules

- Every branch, jump, call, and return owns the following instruction as a
  delay slot. The instruction after `jr $ra` is part of the function.
- A GPR load has a one-instruction load delay on the R3000A. Preserve the
  source dependency/scheduling facts that explain intervening instructions.
- `j` and `jal` encode absolute targets within the current 256 MiB region.
  Internal jumps can still require `R_MIPS_26` in a relocatable object.
- `lui` plus a signed low instruction uses carry-adjusted HI16/LO16 semantics;
  `lui` plus `ori` is not interchangeable. Audit the exact low opcode and
  target addend.
- `jalr` and non-return `jr` are unresolved indirect control flow until a table
  or value chain is proved. Do not invent a callee from proximity.
- Signedness and width are first-class codegen facts. Propagate `char`, `u8`,
  `s16`, `u16`, `int`, and pointer types through callers and callees; `lb/lbu`,
  `lh/lhu`, extension instructions, comparisons, and stack argument slots are
  direct evidence.
- O32 passes the first four arguments in `$a0`-`$a3`, later arguments on the
  caller stack, and integer results in `$v0`/`$v1`. Do not infer a signature
  only from one body; inspect its call sites.
- Stack frames, saved `$ra`, and saved `$sN` registers are useful but not
  universal function markers. Leaf functions, tail calls, callbacks, and SDK
  assembly can omit conventional prologues.

## Source modeling

- Prefer fixed-width project types at binary boundaries and use typed structs
  once a complete-object extent/field family is supported. Add static layout
  checks for proven sizes and offsets.
- Temporary byte-array views may be used only while ownership is genuinely
  unknown; do not let raw offset arithmetic become the final model for a known
  object.
- Give semantic names only when evidence supports them. Address-derived names
  remain acceptable WIP identities in the inventories and first-pass source;
  do not replace them with invented lore.
- Do not create overlapping globals for interior addresses. Refine the owning
  datum/table/structure instead.
- Preserve authentic library/API types at SDK boundaries. Do not hand-roll an
  incompatible replacement merely because its size matches.
- Keep declarations shared when several functions operate on one object
  family. Do not create incompatible per-file structure views to improve one
  function.
- Never add fake locals, volatile carriers, dead statements, artificial
  padding, forced calls, or inline assembly solely to steer code generation.

## Relocations, data, and generated inventories

- `config/retail/{functions,data,relocs}.tsv` are manually curated WIP inputs.
  Seed/audit outputs under `build/` are proposals, never automatic truth.
- Source-level function names/signatures and global/static identities live in
  `function_identities.tsv` and `data_identities.tsv`. Preserve address-derived
  unresolved names until asm, xrefs, calls, strings, or headers support a
  semantic replacement; methods use `owner_action`.
- Run `kf inventory propose` for MIPS/xref dossiers and `kf inventory ghidra`
  for decompiler candidates. Neither generated report may overwrite a curated
  identity TSV.
- The safe delinker and semantic navigator share validation rules in
  `scripts/kf/relocations.py`. If retail evidence disproves the contract, fix
  the shared rule and add an integration control.
- Candidate relocation absence can prevent an otherwise correct source object
  from reaching exact. Diagnose and curate the relocation/data owner; do not
  remove the real source reference to accommodate an incomplete target object.
- BSS, overlay RAM, pointer tables, and GP-relative data require explicit
  ownership work. Do not infer initialized storage from aggregate objdiff data.
- Generated build products, reports, and local retail paths are not committed.

## Verification and change discipline

- Preserve user and concurrent changes. Stage only the current campaign.
- Keep source comments operational: ABI constraints, delay-slot-sensitive
  behavior, unresolved identity markers, and concise safety explanations. Do
  not keep score diaries or reconstruction history in C files.
- Put reusable compiler/MIPS findings under `docs/patterns/` and index them.
- Before commit: inspect raw constants and referents, run focused matches,
  `ruff check scripts tests`, the repository tests, `git diff --check`, and a
  full `kf build`. Run `nix flake check -L` for tooling/flake changes.
- Bank only verified 100% results with `kf bank`; never bank generated or
  unrelated dirty inputs.
- Prefer focused commits such as `match: reconstruct GAME math leaf family` or
  `tools: classify MIPS relocation walls`.
