# Review of every explicit void-pointer boundary from PR #5

Audit snapshot: master `23491d46d6897f84be700023bf5af9124f895652`, containing
PR #5 at `a9e20721`. This is a review-only follow-up: no game source, ABI,
inventory, checker policy, allocation size, or bank is changed. Proposed
corrections below require a separately reviewed implementation.

## Coverage and verdicts

The [curated site ledger](../void-pointer-boundary-review.tsv) accounts for
**79 written casts**, in **82 image contexts**. Three shared `memory.c` casts
appear in both GAME and OPEN. There are no PSX sites in this change.

- **37 from `void *`**: 35 restored casts and two newly introduced word-copy
  implementation casts.
- **42 to `void *` or `const void *`**, including array decay and both
  read-only save-file writes: all newly introduced.
- Thus 44 are net-new casts, not 79 net-new modeling decisions. The review
  deliberately includes all 35 restorations rather than silently excluding
  the older boundaries that the previous PR temporarily removed.

Every row identifies its source location, direction, types, image-qualified
function address, expression, verdict and reason. Verdict totals:

| Verdict | Sites | Meaning |
| --- | ---: | --- |
| retain | 20 | A genuine allocator, raw heap, or file-I/O boundary |
| retain-with-precondition | 28 | Boundary justified; capacity/alignment/lifetime not established by the cast |
| narrow-interface | 13 | Homogeneous data flow permits a more informative game API |
| narrow-owner | 4 | More informative owner/view type is supported |
| model-gap | 14 | Current representation or complete-storage contract is inadequate |

The 14 model-gap sites are **three findings**, not fourteen independent bugs:
12 grid-copy sites, one undersized union allocation, and one sector-read
destination. Likewise, cast removal is not itself the acceptance criterion.

Scope exclusions are explicit: pre-existing integer-to-`void *` in
`memory_release_last`, null conversions, SDK macro expansions, and typed-to-typed
casts were not introduced by this pointer-policy change. `va_arg` AST nodes in
the two formatters are not written casts. They are not disguised omissions
from the 79-site denominator. This audit is not a claim that every pointer
reinterpretation in the repository is settled.

## RTBL: logical table size is not physical read capacity

**Highest-priority ownership finding — V48.** OPEN `render_initialize`
(`0x80016908`) passes `render_cell_windows` at `0x800439d8` to
`cd_file_load_into` (`0x8001615c`), with the call at `0x80016930`. The address
pair at `0x80016920/24` is validated; the direct call is proven.

The shipped `KF/B0/RTBL.` is **3264 (`0xcc0`) bytes**, matching sixteen
204-byte `KfCellWindow` records and the current `DATA(0x800439d8, 0xcc0)`
claim. Its SHA256 is
`b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72`.
But the loader rounds the length to 2048-byte sectors, then calls
`CdRead(2, destination, CdlModeSpeed)`: the transfer footprint is **4096
(`0x1000`) bytes**, 832 (`0x340`) beyond the declared table.

The retail destination interval is `[0x800439d8, 0x800449d8)`. The logical
table ends at `0x80044698`; the next curated owner,
`map_collision_flag_grid`, begins at `0x800446c8`. The transfer therefore
crosses a 48-byte gap and reaches 784 bytes into that grid's current claim.
This is not evidence that the sixteen-window interpretation is wrong. It
is evidence that its logical extent alone is not a complete writable-storage
contract for the CD consumer.

The initialization order matters: rendering initialization precedes the scene
resource copy that fills the collision grid. Intentional startup scratch
overlap and/or a larger physical owner remain hypotheses; this audit did not
execute the game or establish an observable retail corruption. Nevertheless,
separately allocated native C objects cannot assume the same writable tail.

Proposed next work: trace the transfer and overlapping lifetimes to establish
the physical owner, then model the evidenced storage or staging boundary.
Do **not** enlarge the logical table to invented records, pad a global solely
to satisfy a metric, create overlapping globals, or change CdRead to a
one-sector read (which would truncate valid data).

This qualifies the earlier
[visibility-window ownership note](open-map-render-residue.md#visibility-window-ownership):
the resource proves a logical payload, not sector-rounded writable capacity.

## GPU-UNION: a 40-byte packet is not a complete 52-byte union

**V52–V53.** OPEN `render_enqueue_map` (`0x800185e8`) uses a
`KfMapGpuPrimitive` union of `KfGpuGT3` and `KfGpuGT4`. Pinned SDK/compiler
layouts are 40 and 52 bytes respectively; the union is 52 bytes.

Retail calls `primitive_buffer_allocate` at `0x800186d0` with 52 bytes for
GT4, and at `0x80018824` with 40 bytes for GT3. The source follows those
sizes, but both branches restore the pointer as the full union. Triangle
writes remain within the active 40-byte member: this is an incomplete-owner
model, **not an observed packet overrun**.

Proposed correction: branch-local `KfGpuGT4 *` and `KfGpuGT3 *`. Retain
the existing SDK allocation sizes, constructors, field offsets, call order
and subsequent arena cursor. Allocating `sizeof(KfMapGpuPrimitive)` for
triangles would change retail behavior and is not a repair.

V54–V68 were checked individually, including repeated TMD modes. Their target
wrappers match the requested SDK packet extents: F3 20, F4 24, FT3 32,
FT4 40, G3 28, G4 36, GT3 40, GT4 52. The existing pinned fixture
`tests/fixtures/open_tmd_enqueue_layout.c` checks size, alignment and field
offsets. These are legitimate restorations from a heterogeneous allocator,
subject to its capacity/lifetime contract. This does not prove that every
packed/SDK union access is portable to an unrelated C++ implementation.

V51's `primitive_buffer_allocate` (`OPEN 0x80016cb4`) is genuinely generic;
it should not return one arbitrarily selected GPU packet type.

## GRID: void round trips do not establish word-array ownership

**V23, V26–V30, V72–V77.** GAME `map_resource_copy_words`
(`0x8001b3e4`) and OPEN `resource_stream_copy_words` (`0x80016318`) restore
`void *` to `u32 *`, then write words into declared byte/enum grid objects.
The five chunks in each image consistently target attribute, floor-height,
orientation, collision-flag, and collision-kind grids, in that order.
No swapped grid or wrong enum domain was found in those casts.

Each grid is 10000 bytes, copied as 2500 words. The pinned C compiler gives
the four grid types alignment **1**, versus **4** for `u32`. Actual curated
global addresses are word-aligned, and the retail loops use word loads/stores.
Those facts justify describing the observed machine transfer, but do not
turn the declared byte/enum object into a C word array or make a generic
`void *` destination promise word alignment.

In particular, removing `.words` and moving a cast inside a generic helper
has not resolved effective-type/aliasing or representation ownership. The
current `.cells`/`.linear` members model byte indexing, not a word member.
This corrects any stronger interpretation of the earlier
[whole-object-copy discussion](cast-union-debt.md).

Proposed next work: establish an explicit representation-copy/storage contract
that explains the retail word loop while retaining the evidenced cell types.
Test its alignment, extent, domains and all consumers. Neither reinserting
an invented word union nor switching blindly to memcpy is justified just by
cast count or a compiler warning. No miscompilation was demonstrated here;
the source-model gap remains even though these bodies currently match.

## TIM, ITEM and DISPLAY: informative types were erased unnecessarily

These are narrower-interface candidates, not proven runtime bugs or recovered
original declarations.

- **TIM — V11, V14, V21–V22, V25, V42, V46–V47, V71, V78–V79.**
  GAME `tim_upload_images` (`0x8001b100`) and OPEN's homolog
  (`0x80016298`) immediately call SDK `OpenTIM(u_long *)`, then walk images.
  The nine audited callers supply serialized TIM streams; the other current
  callers through the display field were also checked. This game helper
  need not accept arbitrary object pointers. A byte-stream or SDK-compatible
  aligned input is a candidate; select it with all callers and SDK mutability
  in view. A byte cursor remains honest for variable-length storage. Narrowing
  the helper does not eliminate the real serialized-data/SDK interpretation.
- **ITEM — V04 and V12.** GAME `item_load_database` (`0x80020cfc`)
  fills the table from `ITEMn/Ixxx.TMD` paths; the known source caller of
  `cd_file_load_table_entry` (`0x8001ae60`) is `menu_load_item_model`
  (`0x8002aea4`), which immediately registers the allocation as a TMD.
  Consider a byte-result API consistent with `cd_file_load_allocated(u8 **)`,
  or a TMD-specific API if the complete table contract supports it. Change
  declarations and callers together: casting `KfTmdHeader **` to `void **`
  would introduce a different invalid pointer-to-pointer contract. The
  separately documented `KfCdFileEntry`/`CdlFILE` size/prototype discrepancy
  in the table producer is pre-existing typed-to-typed debt, not cleared by
  this audit.
- **DISPLAY — V18, V20, V50.** Both display-state `asset_load_buffer`
  fields are always assigned a primitive allocation's first `u8 *` base.
  Consumers use the area as CD/TIM/transition scratch. A `u8 *` field
  preserves byte-arena meaning better than `void *`; declaring the whole
  arena a TIM or GPU object would be too narrow across its lifetimes.
  Keep shared declarations and homologous field offsets consistent.

## Retained boundaries and their limits

**ALLOC / HEAP.** `memory_allocate` and `memory_malloc_checked` have genuinely
heterogeneous consumers. Their casts create byte arenas, SDK sequence/vector
storage, a complete `KfSaveWorkspace`, or a variable-length weapon asset
headed by `KfAssetHeader`. A prefix pointer does not claim that a 0xc000-byte
weapon allocation is only one header. Generic returns remain appropriate.
The arena rounds sizes to four; this is not a promise of arbitrary alignment.
`BSS_END` is a linker boundary, not an erased game object. InitHeap accepts
raw storage and size; its declaration here is a project ABI shim, not an
original SDK-header discovery.

**SAVE.** V32–V36 read/write complete header or payload extents derived from
`sizeof`, with header and payload obtained from the single workspace or a
complete local header. The two writes preserve `const` in their casts.
Generic byte I/O is appropriate; do not force all file I/O through a save
record pointer. The `read`/`write` declarations are also project ABI shims.

**CD.** The common loader receives sequences, serialized image/archive bytes,
weapon asset storage and a visibility table: its general destination type is
justified. Its `u_long *` restoration follows the pinned `LIBCD.H` prototype,
not an assertion that each destination owns an array of longs. The allocating
path rounds the file size before allocating the same extent. The caller-owned
path encodes neither remaining capacity nor word alignment.

V38, V39 and V40 were checked separately against the retail ICO1/ICO2/ICO3
files: each is 192 bytes, so each one-sector transfer fits `image[2048]`.
Retail's stack placement supplies word alignment; the `u8[]` type alone does
not. Sequence budgets are GAME 0x3000 / OPEN 0x4800; weapon storage is
0xc000; map-variant storage is 0x5a000. These capacity facts are not a proof
that every dynamically selected file or primitive-cursor tail fits. Rows
marked `retain-with-precondition` intentionally do not claim that stronger
whole-program bounds result. V48 is the concrete failing extent found here.

## POLICY: explicit casts are review points, not model validation

The checker has a coverage limit outside these 79 written sites.
`pointer_policy.py` inspects the **retail C** AST. `vendor/include/psyq/libc.h`
retains old-style MEMORY.H/MALLOC.H declarations in C, while adding typed
`memcpy`, `memset` and `free` prototypes only in its C++ branch. Implicit
erasures induced only by those modern prototypes are not visible to that C
AST. A passing `kf check-types` therefore does not establish a universal
both-directions policy across both language views.

Proposed tooling follow-up: add controls for the differing SDK declaration
views and decide their intended coverage explicitly, without rewriting source
or changing authentic SDK types merely to satisfy the audit. This PR does not
silently extend that policy or add more casts. Even exhaustive enforcement
would not detect the RTBL capacity or union-extent findings by itself.

## Evidence and reproduction

Work was performed in `nix develop` after hash-validated `kf init`. The
site census is `kf casts --json` at the snapshot above. Select pointer casts
with a canonical `void *`/qualified-void source or destination, including
constant/incomplete arrays on the other side, then reconcile with the prior
PR's explicit-pointer change. Do not count every AST appearance of a shared
source as a new written cast.

For each ledger function/context, inspect these image-qualified views:

```sh
kf sema --image open addr render_enqueue_map
kf sema --image open disasm render_enqueue_map --blocks
kf sema --image open xref render_enqueue_map
kf sema --image open xref render_enqueue_map --callees
kf sema --image open strings render_enqueue_map
kf sema --image open match render_enqueue_map
```

Use the corresponding `game` selector for GAME, especially shared memory
source. Direct calls are proven; curated HI16/LO16 address pairs are validated;
proposed original C owner/interface choices remain candidates. The ledger
does not promote source declarations to retail symbols.

Additional checks used the local Psy-Q Release 2.5 `LIBGPU.H`, `LIBCD.H`,
`LIBSND.H`, allocator headers, shared layout fixtures, source callers and
consumers, resource file sizes, and the table/grid identities and relocations.
A pinned `cpppsx-257`/`cc1psx-257 -O2 -G0 -mcpu=r2000` assertion probe
confirmed GT3=40, GT4=union=52, windows=3264, grids=10000, grid alignment=1
and word alignment=4. No SDK or retail bytes, generated census, or local
retail path is committed with this manually adjudicated ledger.

Fresh `kf match --all --reconfigure` rebuilt the 101 units; strict game-source
totals remain **458/471** (GAME 351/362, OPEN 106/108, PSX 1/1). Its existing
data/allocation failures are not claimed as passing gates. A full `kf build`
built all three executables. No new exact result or source correction is
claimed by this documentation-only review.

Final baseline checks: Ruff passes; `kf check-types` passes **101/101**
source/image variants; the full repository suite passes **825 tests**, with
12 skips and 10,245 passing subtests. The ledger's 79 source anchors, types,
directions, provenance totals and 82 contexts were reconciled against the
fresh census. `git diff --check` passes. No tooling/flake files are modified.
