# Typed-storage campaign: final audit

The campaign replaces unsupported pointer views with shared fields, complete
objects and consistent interfaces across `src/` and `include/`. The final
source at `fad4ac3` has 451 written C pointer casts, down from 806 at
`c3700f8` (355 removed, 44.0%). Total written casts fall from 1,049 to 713.
These counts include casts written in macros and layout assertions; moving
a conversion into a helper does not remove it from the count.

## Models recovered

The indexed [campaign and stage plans](typed-storage-cast-campaign.md) contain
individual retail evidence snapshots and final function verdicts. The kept
changes cover:

- GPU/TMD packet fields, authentic SDK colors and screen coordinate views.
- Menu points, glyph rows, list titles and ordering-table element indexing.
- Effect records, shared rotation/position interfaces, world transforms and
  weapon render transforms.
- Packed map links, container item arrays, player motion/cell words and
  dialogue state.
- Equipment/magic table word views, complete map grids, resource definition
  tables, animation vertex views and parsed TMD header pointers.
- Generic resource allocation slots with explicit typed cursor views, shared
  arena ownership and the complete allocated save workspace.

The models retain the existing physical object boundaries and authentic SDK
types. The new arena claim unifies former interior identities. Allocated save
storage has a complete type, while the public header and payload pointers
also remain valid for independent stack buffers. No source file, compiler
profile, unit ownership interval or function extent is changed to conceal
a cast or bank instructions.

## Verification scope

All 112 original source variants were rebuilt from `c3700f8`'s `src/` and
`include/` using their unchanged manifest profiles and the pinned Nix SDK.
All final variants were rebuilt after the source changes. The whole-campaign
comparison resolves each object's HI16/LO16 and MIPS26 references to physical
addresses, including internal jumps and SDK tail jumps between claims. Former
data names resolve through the original identity table; current names resolve
through the current table. No instruction operands or addends are masked.

All **484 compiled function bodies** retain every linked word, ordered direct
call and physical reference. Strict scores are also unchanged. This is
439/471 exact game functions, 32 unchanged game partials and thirteen exact
vendored controls. SDK controls stay outside the game denominator. This is
source/type progress, with no claim of newly matched game functions.

The comparison also checks all 486 allocated non-text sections across those
112 objects: initialized bytes and their relocations are unchanged. The only
extent additions are the two newly source-owned 80-byte arena BSS blocks.
All 15,241 curated relocation rows retain their numeric sites, targets,
opcodes and evidence; 36 change only their owner name to `memory_arena`.

The final target-C AST census parses all 112 variants without errors, covers
all project headers and locates every cast. It deduplicates written sites by
source origin while retaining each macro expansion context. The 713 sites
comprise 451 C pointer casts, 214 C scalar casts, 22 header pointer casts and
26 header scalar casts. No C cast has identical canonical source and target
types. Scalar casts retain narrowing, signed/unsigned interpretation, encoded
enum or address arithmetic; header pointer casts implement layout checks.
The optional modern C++ editor view still has older SDK/interface diagnostics;
the clean parse claim here is for target C.

All 714 repository tests pass (nine environment-dependent skips), including
layout, owner/referent negative controls and retail instruction comparisons.
Ruff and `git diff --check` pass. The flake checks passed after the header
validator changes, with all 714 tests and 147 isolated-environment skips.

The final full `kf build` compiles the source and retains 439 exact functions
with zero data artifact failures. It still exits nonzero on the documented
data, reference-closure and section-placement gates. The newly owned OPEN
arena exposes one additional placement failure: retail `80075848` does not
meet the emitted BSS section's 16-byte alignment. GAME's new arena data check
passes. The source does not add padding or alignment overrides to suppress
that uncertainty. See [arena ownership](typed-memory-owner.md).

## Remaining C pointer casts

Every remaining site has a reviewed purpose. Counts below are written sites,
including casts inside container/offset expressions; they are not counts of
runtime operations or independent unresolved objects.

| Sites | Purpose and retained boundary |
| ---: | --- |
| 187 | Serialized asset decoding: variable archive/TMD/animation records, byte offsets and typed resource chunks. Parsed record headers and fixed tables are typed after decoding. |
| 103 | Prepared projected-vertex byte offsets: packed indices address eight-byte records through a byte or integer address base. The complete OPEN projected array is typed. |
| 37 | Heterogeneous GPU allocation cursors: the active primitive determines the packet type and size. Packet fields themselves use shared typed views. |
| 5 | Generic storage results entering a typed asset, ordering table, vertex or save workspace interface. |
| 2 | Shared byte workspaces: publishing a packet cursor and borrowing an asset buffer for the map display. |
| 1 | Clearing the complete three-bank item-stock object through bytes. Taking its complete-object address avoids crossing a single row's array extent. |
| 1 | Allocator stack word interpreted as a size or heap pointer according to allocation mode. |
| 24 | Native O32 variadic argument slots and formatting payload words. The modern variadic view uses its compiler's builtin argument support. |
| 6 | SDK CD command payloads: `CdlLOC` passed through the SDK's generic byte pointer. |
| 2 | SDK overlay arguments: two request/result words passed through `Exec`'s `char **` interface. |
| 4 | Opaque SDK audio work or sequence buffers. |
| 1 | SDK CD record revision boundary: the game's 20-byte entry and the supplied header's 24-byte `CdlFILE`. |
| 9 | SDK translation output boundary: `RotTrans` writes three words into the twelve-byte `MATRIX.t` array. |
| 7 | Literal MMIO/heap addresses and pointer sentinels. |
| 11 | C-file compile-time offset assertions. |
| 14 | Typed views into runtime storage whose complete registry or projection/morph subdivision remains unresolved. |
| 29 | Complete-owner recovery expressions with demonstrated non-exact direct-owner substitutions. |
| 8 | Sprite packed-XY stores with demonstrated non-exact direct-field substitutions. |
| **451** | **All remaining C pointer casts; zero unclassified sites.** |

The fourteen unresolved-storage sites comprise five registry views, six
projection views and three morph-scratch views. Their start addresses and
consumed element layouts are known; capacity or complete subdivision is not.
The existing [graphics-owner evidence](game-graphics-runtime-owner.md) explicitly
rejects guessed 60/1000/1001 capacities. Shared byte storage is retained only
for those unresolved spans, with typed views at their access boundaries.

The 29 owner-recovery sites are 24 in OPEN TMD, two in OPEN unlit rendering
and three in the opening controller. The real shared owners already exist.
Direct owner pointers or global field accesses change retail instruction
selection, extent or order. Eight sprite XY stores likewise retain their
casts after direct packed members change the emitted code. The prepared
OPEN TMD byte-pointer probe changes 41 `addu` operand orders. The controlled
comparisons and first divergences are in
[projected addresses](typed-projected-addresses.md) and
[allocator ownership](typed-memory-owner.md). These are explicitly recorded
non-exact substitutions, not evidence for a named compiler mechanism. Exact
banked functions are preserved.

The SDK translation and CD boundaries were checked against bodies as well as
headers. GAME `RotTrans` at `8004dadc` and OPEN `RotTrans` at `8002d8b0` write
output offsets 0, 4 and 8, with no write at 12; a sixteen-byte `VECTOR` cannot
be embedded in the authentic twelve-byte `MATRIX.t`. GAME `CdSearchFile` at
`8003c810` copies five words to its result (stores at `8003ca48..8003ca70`),
matching the existing twenty-byte `KfCdFileEntry`. The Release 2.5 header's
larger record remains an explicit version boundary. No vendored body is
edited or counted as game reconstruction.

## Reproduction and retained evidence

Generated artifacts remain ignored under `build/`. In this worktree:

- `build/cast-audit/retail.json`, `written.json` and `review.txt` retain the
  source hashes, AST origins, expansion contexts and complete cast census.
- `build/cast-model/final-audit/pointer-verdicts.json` records each remaining
  pointer site's source, function, source/target types and reviewed purpose.
- `build/cast-model/final-audit/build_initial.py` rebuilds the original source;
  `compare.py` and `raw-results.json` retain all 484 before/after/retail verdicts.
- `compare_data.py`, `data-results.json` and `reloc-audit.json` in that same
  directory record the allocated data and curated referent comparison.
- `build/cast-model/typed-save-workspace/` retains the final full-build,
  repository-test, lint and target-C census logs. The arena stage retains
  the flake-check log and owner-shift negative control.

Run these diagnostics inside `nix develop`; use the cast audit's `run.sh`
for its pinned libclang environment and `PYTHONPATH=.` for comparison scripts.
Each stage's six-view dossier remains image-qualified. Future cast reductions
need new ownership/extent evidence or an independently supported source form
that preserves banked instructions. Broadening unions merely to hide generic
conversions, changing SDK records, guessing capacities or adding code-generation
carriers does not improve these models.
