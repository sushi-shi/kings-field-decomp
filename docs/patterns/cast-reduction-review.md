# Complete C-cast review and reduction

## Function Match Plan

Baseline `0ecce809`, in the existing isolated `codex/type-cleanup` worktree.
The target-C AST census covers all 112 source/image variants and every
project header: 729 written casts, comprising 501 pointer and 228 scalar
sites. Of these, 724 are in C files and five in shared enum/domain macros.
Every baseline site has a numbered source/type/context entry under
`build/cast-reduction/`; all sites must receive a reviewed disposition.

Review the entire census, including numeric narrowing, signedness, enum
adapters, SDK calls, serialized/prepared offsets, generic allocations,
runtime owners, packet fields and variadic argument slots. Shared types and
interfaces take priority over local syntax. Do not create unions, cast-hiding
helpers, guessed array capacities, fake locals or weaker SDK declarations
to reduce the count. Retain conversions whose arithmetic or representation
is real. A retained code-generation residue must name the tested source
hypothesis and first observed divergence, without attributing a compiler wall.

Initial source hypotheses:

- Twelve `u32`-to-`u32` conversions in GAME `render_enqueue_tmd` are redundant.
  Keep its full switch, prepared byte offsets and current owner/referents.
- GAME `menu_draw_window` can address the existing `MenuGlyphString.position`
  field directly instead of reinterpreting its complete row as `MenuPoint`.
- Numeric casts immediately before an assignment to the same narrow type,
  or before an already-equivalent integer promotion, may be redundant.
  Preserve signed shifts, unsigned range tests and argument extension where
  the retail instructions require them. Test each coherent expression family.
- Review two retail-only enum adapter casts separately from modern type
  conversions. Their current operands have the same underlying byte type;
  modern scoped-enum interfaces must retain their existing domain checks.

Each edited function gets a pre-edit image-qualified dossier: address and
extent, retail disassembly/CFG/delay slots, callers, callees, strings, xrefs
and evidence tiers, and strict score. Shared callers, adjacent functions,
source history and supplied SDK interfaces constrain each change. These
are game policies around separately vendored services, whose bodies remain
controls. Expand this plan with evidence before broader model changes.

Force focused compilation after each source cause and compare complete
instructions, constants, data and ordered relocation targets with the saved
baseline. Start at the first real divergence: referents, calls, CFG, access
widths, then unattributed code-generation residue. Keep all banked exact
functions exact and explicitly report any movement in existing partials.

Before handoff, finish the per-site dispositions and per-function verdicts,
reconcile any type/signature inventories, run modern type checks, the full
repository tests, Ruff and diff checks, and run a full `kf build` after the
final source changes. Baseline is 447/471 exact game functions plus 13 exact
vendored controls; existing data/ownership/relink failures are preserved.
Do not change compiler profiles, relocation inputs or score ledgers to hide
a reduction's cost. Generated reports and objects stay under `build/`.

## Results

The complete review removes **187 of 729 written casts (25.7%)**. The final
target-C census has 542 casts: 445 pointer and 97 scalar conversions. Three
scalar sites are shared enum macros; the other 539 sites are in C files.
Pointer casts fall by 56 and scalar casts by 131. No unions or cast-hiding
helpers are added, and no SDK declaration is weakened.

| Kept reduction | Casts removed |
| --- | ---: |
| Redundant assignment, promotion, low-bit, screen-narrowing and identity conversions | 109 |
| Existing direction/position members and the signed scrolling Y field | 26 |
| GAME prepared-vertex byte cursors | 23 |
| SDK storage types, byte-loader return and generic allocator locals | 8 |
| Reused archive header and six smaller parsed packet dispatches | 21 |
| **Total** | **187** |

The archive loop reuses its real parsed header for registration and length.
GAME model/map and OPEN map/unlit dispatchers parse each body once. Both
TMD preparers use the existing mode-discriminated primitive members. The
large GAME/OPEN TMD dispatchers retain their per-case views after the shared
view probe fails exact comparison.

Every one of the 729 baseline sites has its original source/type/context
snapshot and a disposition in `build/cast-reduction/all-site-verdicts.tsv`
and `.json`: 187 removed, six consolidated into one view per dispatch, and
536 retained. This is a reviewed site ledger, not a claim that every retained
cast has a uniquely proved original spelling.

| Remaining boundary or reviewed constraint | Sites |
| --- | ---: |
| Serialized records and packet bodies, including retained large-dispatch views | 98 |
| Prepared vertex and normal byte offsets | 150 |
| Heterogeneous GPU packet cursors and allocations | 54 |
| Generic storage entering a typed interface | 12 |
| Native O32 varargs and formatted pointer payloads | 24 |
| Authentic SDK command, buffer, matrix-output, CD-record and overlay interfaces | 26 |
| Literal addresses, allocator words, whole-object byte view and cursor publication | 12 |
| Unresolved registry/projection extents and morph-scratch access | 11 |
| Complete-owner recovery with non-exact direct-owner substitutions | 79 |
| Sprite packed XY access with non-exact member substitutions | 8 |
| Growth-table copy alignment | 1 |
| Numeric/domain arithmetic, predicates and caller widths | 67 |
| **Total; no unclassified sites** | **542** |

The registry and projection/morph capacities remain explicitly unresolved.
Prepared indices are byte offsets, so changing their pointer arithmetic to
element indexing would introduce a different operation. Genuine unsigned
shifts, signed differences, wrapped range tests and ABI conversions remain
visible. The three enum macros retain modern domain checks and target widths.

## Non-exact probes

Each probe below was restored independently; no changed instruction remains
in the retained source. These are observed residues, not compiler attribution.

| Probe | First divergence or semantic difference |
| --- | --- |
| GAME sprite scale casts removed | `lh` becomes `lhu` for offsets 36/38/40; the SDK receives a word-sized `VECTOR`, so sign extension is real. |
| Growth-table `memcpy` source cast removed | After the source-address addition, an alignment test and unaligned `lwl/lwr/swl/swr` copy path appear. |
| Explicit damage-index and fade-argument casts removed | Caller `andi` becomes `move`, despite the callee's narrow prototype. |
| GAME direct graphics owner | Initial exit displacement changes from `+0xf08` to `+0xf0c`; the normals/projected registers exchange roles and address generation changes. |
| Shared parsed view in the two large TMD dispatchers | First instruction grows GAME's frame 88 to 96 bytes and OPEN's 96 to 104; packet-cursor spills follow. |
| One animation asset byte base | First instruction grows the frame 72 to 80 bytes, followed by extra pointer storage/reloads. |
| One borrowed morph-scratch pointer | Initial pool-record load changes `s4` to `s3`; later instruction placement and body extent change. |
| Signed-halfword darkness fade local | The first fade sign extension becomes a saved value and shifted sign test; later branches and blend shifts change. |

The earlier OPEN direct-owner, integer projected-base and sprite-XY controls
remain applicable and were read alongside current source:
[projected addresses](typed-projected-addresses.md),
[allocator owner](typed-memory-owner.md), and
[GAME packet views](typed-storage-cast-campaign.md).

## Verification

All **484 compiled function bodies retain their complete instructions and
strict scores**. All 112 objects retain section bytes/extents and ordered
relocation targets. The per-function and per-object verdicts are under
`build/cast-reduction/final-function-verdicts.json` and
`final-object-verdicts.json`; every function has an individual verdict.
GAME stays 340/362 exact, OPEN 106/108, and PSX 1/1: **447/471 game functions**,
with all 13 vendored controls exact. No partial score moves.

All 112 modern type-checking variants pass. After forced compilation of the
affected units, the final full build preserves the existing data/ownership/relink failures:
one PSX, 17 OPEN and 36 GAME data-owning units diverge, with zero data artifact
failures. These gates predate this change; full-build success is not claimed.
The curated audio and rectangle field types, audio workspace identities and
map-loader signature are reconciled. The historical graphics fixture keeps
its original pointer views and byte-offset arithmetic as an independent
control. The full repository suite passes: 726 tests, including nine skips.
`ruff check scripts tests` and `git diff --check` also pass. No new exact
functions are claimed or banked; the existing match ledger is unchanged.

## Probe scope recorded before implementation

Additional evidence-backed probes:

- The only `audio_sequence_table` consumer is SDK `SsSetTableSize(char *,
  short, short)`; use `char` storage in both images. The allocated sequence
  buffer goes through the existing generic CD destination API and then
  `SsSeqOpen(unsigned long *, short)`; model that shared field with the SDK
  pointer type and retain the explicit generic-allocation conversion.
- `map_resource_load_file` returns the `u8 *` written by
  `cd_file_load_allocated`; all three callers consume file bytes or hand them
  to the generic TIM wrapper. Propagate that return type. `memcpy` needs no
  typed cast for its growth-table source bytes.
- GAME model/map enqueuers immediately convert their projected pointer back
  to bytes at every prepared-index use. Carry that local as a byte cursor;
  keep the typed conversion at each actual vertex read and preserve unknown
  storage ownership.
- Narrow world-to-screen destinations and unsigned-short culling locals may
  absorb earlier low-halfword conversions. Test the coherent families and
  retain conversions if load widths or effective arithmetic change.
- GAME TMD's projected base belongs to the existing graphics runtime. Probe
  direct access through that owner independently of earlier OPEN experiments.
  Archive loops may reuse a parsed chunk header for registration and its
  byte-size step. TMD dispatch may parse the shared packet header once before
  selecting an existing variant; neither probe adds a layout or a capacity.
- Signed direction components already have matching `SVECTOR` members in
  `KfEffectDirection`. Use those members for signed motion/heading reads;
  preserve the unsigned state interpretation for consumers that use it.
  Scrolling rectangles have a signed Y lifecycle, including off-screen
  negative positions and a signed stopping test. Probe `s16 y` in the one
  shared rectangle definition and check every quad emitter and caller.
- Allocator result locals are generic blocks: neither local performs byte
  arithmetic, while the arena cursor already has its own byte-pointer type.
  Keep those locals as `void *`. Animation decoding repeatedly addresses
  offsets from one asset base and borrows the same morph-scratch region;
  probe one byte view and one borrowed typed scratch view without claiming
  any new storage extent.
- The darkness fade local receives only two explicitly narrowed signed
  halfword expressions; probe making the local `s16` and using its implicit
  assignment conversions. Check whether the damage helper's `u16` parameter
  supplies the same caller mask as its three explicit collision-index casts.
