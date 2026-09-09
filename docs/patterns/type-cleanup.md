# Remove assertion clutter and unsupported union views

## Function Match Plan

Worktree `codex/type-cleanup`, baseline `7ebc0c1a`, GAME/OPEN/PSX kept
independent. The fresh full build has 447/471 game functions strictly exact
(GAME 340/362, OPEN 106/108, PSX 1/1); all 13 vendored source functions are
exact. Existing data-placement, ownership and closure failures are baseline
failures, with zero artifact failures.

Remove production typedef-array layout assertions and their helper macros.
Existing target-compiler fixtures and the independently curated structure
inventory retain layout validation. The prior isolated comparison already
established that eliminating unused assertion typedefs preserves all 112 C
variants' compiler assembly; recompile the actual edited source to verify it.

Then review the 43 unions in the companion
[audit](type-assertion-and-union-audit.md). Start with resource allocation
boundaries, notification digit storage and typed resource-copy tables. Follow
with local enum/index/result workspaces, packed aggregates and SDK interfaces.
Use one canonical storage representation where source and retail accesses
support it. Explicit numeric and representation conversions are acceptable
at real interfaces. Do not replace unions with artificial storage, forced
locals, volatile, inline assembly or an unproved function-signature change.

Every affected function receives an image-qualified pre-edit evidence
snapshot under `build/type-cleanup/evidence/`: signature/extent, strict score,
retail disassembly and CFG, incoming/outgoing references and their tiers,
callees and strings. Its module context and source history constrain adjacent
functions and shared callers. These are game format/state policies around
separately vendored memory, geometry and I/O services; their SDK bodies and
authentic API types remain external.

Save the fresh baseline objects and strict report. After each source cause,
force a focused compilation and inspect the first raw instruction/relocation
divergence. Referents, calls, CFG and access widths precede code-generation
residue. Keep banked functions exact; compare all partial scores and do not
hide losses by changing profiles, inventories or baselines. A rejected trial
records its source hypothesis and observable result. Preserve supported
variant payloads when a simpler model lacks evidence or regresses matching.

Before handoff, refresh the per-function verdicts, reconcile changed type and
signature inventories, run the repository tests and Ruff, inspect constants
and ordered referents, run `git diff --check`, and complete a full `kf build`.
Only intended verified exact rows are eligible for `kf bank`. Generated
evidence, local paths and build products are not committed.

## Results

All production typedef-array assertions and their helper macros are removed,
as are the seven modern-only actor layout assertions. Layout validation
remains in the inventory and independent compiler tests. The GAME reversed
environment and OPEN shortened-gap controls now inspect emitted layout
values; both still distinguish their deliberately incorrect models. They
no longer rely on assertions being present in production headers.

Five of the baseline's 43 unions are removed:

| Source model | Kept replacement | Verdict |
| --- | --- | --- |
| `KfResourcePointer` | Byte-buffer output throughout CD callers; raw storage accepted by the existing TIM uploader, with its SDK conversion inside | All affected GAME/OPEN instructions, calls, relocations, constants and data unchanged |
| GAME `render_map_cell` selection | One byte holding the encoded attribute and then the mesh index | Strict 100%, unchanged |
| OPEN `render_map_cell` selection | The same single-byte arithmetic in the independent implementation | Strict 100%, unchanged |
| `menu_save_panel` status | One signed word for successive encoded API results | Strict 100%, unchanged |
| `MenuGlyphWorkspace` | `MenuGlyphString`, with a cursor into `glyphs.codes` | Already partial: 97.814210% → 97.808750% |

The item-detail difference is confined to two constants: the glyph cursor
starts at `sp+20` instead of `sp+16`, and its halfword store uses offset zero
instead of four. The effective addresses agree. Body size, accesses, loop,
calls and ordered referents are unchanged. No casts or artificial object
views are retained to recover those two constants.

All 484 reconstructed functions have final image-qualified verdicts in the
generated `build/type-cleanup/final-function-verdicts.json`. **483/484** have
baseline-identical raw instruction/relocation listings. Every unit's data
bytes, section extents and ordered relocation entries are unchanged. Strict
game matching remains **447/471** (GAME 340/362, OPEN 106/108, PSX 1/1), and
all **13/13** vendored controls remain exact. No compiler profile, function
extent, relocation input or score ledger was changed to obtain these results.

The same Clang AST audit parsed all 112 variants on baseline and final source.
Written casts in function bodies fall from **731 to 724**: pointer casts
**508 → 501**, scalar casts **223 → 223**. Including production declarations
and headers, the total falls **806 → 729**; most of that larger reduction is
deleted assertion machinery. The resource change adds four conversions at
the allocator/SDK boundaries and removes eleven existing caller casts.

The other 38 unions remain explicitly unresolved source models or supported
variant payloads. Retention is not evidence that the original developers
wrote them. The complete audit records each declaration. Relevant rejected
experiments are:

- Five record-table arrays preserve bytes with word-pointer casts, but merely
  exchange union views for casts. The ordinary weapon/armor `memcpy` trial
  adds alignment branches and unrolled copy paths, unlike retail's one-word
  loops. These removals are not kept.
- Flat map grids change 26 consumer functions. Keeping row-then-column
  address arithmetic restores the occupancy helper, but several consumers
  still differ. A matrix plus casts for its linear/copy views would only
  move the representation workaround. The four grid unions remain.
- Signed notification digits plus four numeric casts preserve bytes, but
  are another cast substitution. Implicit conversion emits `lh` instead of
  retail's four `lhu` instructions. A `u16` callee parameter retains those
  signed loads and adds an `andi` in the callee. That signature trial and
  the digit-union removal are rejected.
- SDK packet, rotation, packed-word and morph views still have real width or
  interface overlap. No widespread pointer reinterpretation is introduced
  to reduce their union count. Behavior-selected payload unions are retained.

Final validation: the complete modern type check passes **112/112** variants;
inventory validation passes with **129 types and 865 fields**. The full
`kf build` recompiles affected objects and completes objdiff generation, then
returns failure on the baseline data-placement, ownership and relink checks:
PSX 1, OPEN 17 and GAME 36 data-owning units diverge, with zero artifact
failures. The cleanup does not claim executable or data closure.

Repository validation passes: **726 tests, nine skipped**,
`ruff check scripts tests`, and `git diff --check`. The score ledger remains
unchanged because this cleanup introduces no newly exact functions.

## Clarified source constraint and follow-up plan

Do not count a union-to-cast substitution as a successful cleanup. The table
pointer-cast trial preserved bytes but is rejected on that basis. Ordinary
`memcpy` of the weapon/armor arrays adds alignment branches and unrolled
copy paths; the flat grid trial changes multiple consumers. Retain these
table/grid views pending a better shared model.

For the CD family, model allocated file contents as `u8 *` throughout the
out-parameter callers. The existing TIM uploader accepts raw storage and
adapts it to the authentic `OpenTIM(u_long *)` interface internally. This
removes caller casts and the pointer-view union together. GAME and OPEN
keep their independent retry/error policies. The allocator remains generic.

All four proven calls to `notification_digit_set_v` supply zero-extended
halfwords. Test a `u16` parameter with the formatter's existing signed
halfword array, instead of four caller casts or a signed/unsigned union.
The callee multiplies by 11 and stores a byte; it does not use the high bits.
For item glyphs, use the actual `glyphs.codes` field as the cursor. Accept
an explicitly measured small change in this already-partial function rather
than preserving a whole-structure cast or an invented workspace union.
