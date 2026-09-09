---
name: matcher
description: Reconstruct and byte-match King's Field PS1 C functions and translation units against PSX.EXE, GAME.EXE, or OPEN.EXE with the pinned Psy-Q GCC probe. Use for function matching, TU reconstruction, type recovery, relocation/referent correction, and evidence-based comparison of a non-exact result.
---

# King's Field matcher

Recover source structure that explains the selected retail image. Correct
types, widths, ownership, control flow, calls, constants, GTE operations, and
referents outrank a temporary score. Closure requires strict 100%.

This is the only project matching skill. Do not import Gruntz's VC5 wall or
permutation mechanics: MSVC internals do not transfer to GCC, and this project
does not yet have a real residue corpus or proved historical compiler.

## Establish the environment

1. Work in `nix develop` and select exactly one image.
2. Confirm the retail image is hash-identical with `kf init`/`kf-retail-validate`.
3. Treat `probe-gcc260-o2-g0` as a probe, not proven compiler attribution.
4. Rebuild the affected object; a Ninja no-op is not evidence after edits.
5. Use static retail bytes and repository inventories. Do not run the game.

## Reject vendored work first

Before reconstructing, check `functions_vendored.tsv`, Psy-Q archives/headers,
SDK signatures, GTE wrapper shapes, libc algorithms, and startup topology. If
the function is Sony/Psy-Q or another library, improve vendored attribution
instead of counting it as game progress. Never hand-write assembly to bank a
game function; use C unless assembly ownership is independently proved.

## Required evidence pass

For target `<image> <va|name>` run and read:

```sh
kf sema --image <image> addr <target>
kf sema --image <image> disasm <target> --blocks
kf sema --image <image> xref <target>
kf sema --image <image> xref <target> --callees
kf sema --image <image> strings <target>
kf sema --image <image> match <target>
```

Inspect adjacent functions and all call sites that constrain the O32 signature.
Audit candidate evidence separately from `--confirmed-only` output. Search git
history and any sibling/release source evidence before inventing a body.

A related bulk campaign may share common ownership/type hypotheses, but each
function keeps its own final verdict.

## Reconstruct before steering

1. Establish image, function extent, owner, signature, widths, return type,
   object/data identity, and whether it is game or vendored code.
2. Reconstruct the complete call set, constants, branches, loops, returns, and
   ordered referents.
3. Model MIPS-visible types: signed/unsigned byte and halfword loads, extension,
   comparison form, stack arguments, and structure offsets.
4. Preserve branch/jump/return delay slots and explain load-delay scheduling.
5. Claim the function with `ADDRESS(va, body_size)` in its module source
   (identity spelling, ascending order, contiguous run), iterate with
   `kf try --unit <unit>`, then record with `kf match --unit <unit>`.
6. Compare both objects from the first real divergence. Use objdiff plus raw
   MIPS disassembly and relocation tables; fuzzy percentage is insufficient.
7. Change one source-level cause at a time unless composing independently
   evidenced facts from a deliberately retained exploratory base.

## Compare in order

1. **Referent/relocation:** instruction form is equivalent but relocation
   presence, kind, symbol, or addend differs. Fix the inventory/identity.
2. **Call set:** direct call targets or indirect-call topology differ. Complete
   the body and recover inline/macro/API boundaries.
3. **CFG:** branch targets/counts, returns, or delay-slot ownership differ.
   Recover the source construct.
4. **Instruction selection/type:** `lb/lbu`, `lh/lhu`, signed compare, shift,
   multiply/divide, HI/LO address form, or argument extension differs.
5. **Unattributed codegen residue:** when earlier layers agree, record the
   remaining register/frame/order symptoms without assigning an optimizer
   cause. Continue only with independently evidenced source corrections.

Do not call a low score a register or scheduler wall. That terminology requires
the exact compiler/profile plus backend/optimizer evidence this project does
not yet have.

## Compiler-wall TODO

Matching comes first. After a representative batch, catalogue recurring
non-exact residues by observable instruction/CFG/relocation signature. Only
then prove the exact GCC/version/profile, obtain and read its MIPS optimizer,
instruction-selection, allocation, and scheduling sources, and build controlled
A/B probes. Until those prerequisites exist, stop at an unattributed residue;
do not create a wall taxonomy or run permutation.

## MIPS and source rules

- A control transfer owns the next instruction as its delay slot. The slot
  after `jr $ra` remains inside the function.
- A GPR load result is unavailable to the immediately following instruction on
  R3000A. Treat the intervening instruction as scheduling evidence.
- `jalr`/non-return `jr` targets remain unresolved until their value/table is
  proved. A pointer candidate is not a call claim.
- `lui` + signed-low and `lui` + `ori` have different relocation semantics.
- Propagate widths and signedness through the whole caller/callee family.
- Prefer shared typed structs for proved object families. Do not retain casts,
  offset macros, fake locals, volatile carriers, or inline assembly as codegen
  steering devices.
- Keep one function per unit until TU grouping is independently evidenced.

## Verify and hand off

Before committing:

1. Recheck retail/raw base instructions, delay slots, constants, calls, and
   ordered referents.
2. Run the focused match and require 100% for an exact claim.
3. Run full `kf build`, then `kf bank` for intended exact rows.
4. Run repository lint/tests and `git diff --check`; use `nix flake check -L`
   for tooling or flake changes.
5. Commit source, focused configuration, durable evidence, and baseline updates;
   never commit generated build state.

Report exact-count movement, the reconstructed source family, vendored negative
controls, unattributed residues, full-build result, and commit.
