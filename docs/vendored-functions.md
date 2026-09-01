# Vendored function inventory

`config/retail/functions_vendored.tsv` is a manually owned provider layer over
the structural function census. It answers “which admitted functions came from
an external runtime/library?” without changing function boundaries or treating
an analyzer label as ownership proof.

Only the original Japanese King's Field (`SLPS-00017`) is in scope. No KFII or
KFIII executable contributes to this list.

## Current result

| image | Release 2.5 exact object | Release 2.5 FID-only | Release 2.5 lineage | GTE lineage | Psy-Q 2.60 signature | total |
|---|---:|---:|---:|---:|---:|---:|
| `PSX.EXE` | 8 | 0 | 0 | 0 | 0 | 8 |
| `GAME.EXE` | 185 | 143 | 4 | 58 | 52 | 442 |
| `OPEN.EXE` | 184 | 143 | 4 | 58 | 37 | 426 |
| **total** | **377** | **286** | **8** | **116** | **89** | **876** |

The 876 rows comprise 862 named functions and 14 anonymous internal functions
whose containing Sony object is known. Provider counts are: 264 `LIBGTE`, 192
`LIBGPU`, 128 `LIBCD`, 121 `LIBSND`, 93 `LIBAPI`, 62 `LIBSPU`, four startup
functions attributed to `NONE2.OBJ`, two each from `LIBSN` and `LIBETC`, and
eight fully fixed `memcpy` copies whose exact member remains ambiguous across
`LIBCD`, `LIBGPU`, and `LIBSPU`. No zlib or other third-party library has been
identified, so none is claimed in the TSV.

These rows are an exclusion boundary as well as attribution evidence.
`kf-delink` may carve them to preserve the linked executable model and resolve
calls from game code, but `kf-objdiff-project` never creates matching units for
them and `kf-compile` rejects their target filenames. Vendored code therefore
does not inflate decomp progress or invite source reconstruction work.

## Evidence channels

Release 2.5 exact evidence is primary. For each preserved object-section match,
the seeder extracts the named SDK member, joins its interleaved `.text` records,
masks only bits represented by retained Psy-Q linker relocations, and rechecks
the retail bytes. It validates object size, compared-bit count, relocation
count, executable occurrence count, and XDEF offsets. Six `PSX.EXE` `LIBAPI`
stubs use separately reviewed complete 16-byte object matches.

The project-owned Release 2.5 function-ID lane does not require Ghidra. It
extracts every object from the hash-pinned Psy-Q libraries with `psy-k`, splits
each `.text` section using compiler debug Function records when present and
XDEF/local-symbol boundaries otherwise, and records the function bytes plus an
exact bit mask for linker-owned relocations. The resulting corpus currently
contains 1,226 usable function signatures. Each signature is tried at every
admitted retail function start; a `HIGH` result must be substantial, resolve to
one SDK object identity at that address, and occur no more than once in each
separately linked image. `GAME.EXE` and `OPEN.EXE` are checked independently.

The current run finds 971 raw matches: 581 `HIGH`, 364 `AMBIG`, and 26 `SHORT`.
Of the high-confidence matches, 303 corroborate an existing exact-object row
and 278 add provider rows that the whole-object lane could not see. Eight more
rows are admitted from one fully fixed 0x30-byte `memcpy` FID: its bytes and
normalized name agree across three Sony objects, although the precise archive
member does not. Other ambiguous or short hits remain report-only. Corpus and
match reports are generated under `build/fid-census` with SHA-256 provenance.

The secondary lane consumes the Psy-Q 2.60 wildcard JSON signatures bundled by
the pinned `ghidra_psx_ldr`. A match is proposed only if:

- it has at least 12 fixed bytes and an eight-byte exact search anchor;
- it occurs exactly once in the executable and begins on a word boundary; and
- at least one non-automatic signature label lands on an existing structural
  function start.

Functions wholly contained by that object signature are included, as are
labelled starts whose WIP structural extent crosses the signature boundary.
Identical archive signatures are retained as `|`-separated module candidates.
These rows identify likely Sony library code, but they do not prove that the
linked archive was version 2.60.

The plugin does not ship a Ghidra `.fidb` for this material. Its JSON wildcard
objects are therefore described as signatures, not FIDs. The project-owned
Release 2.5 FID corpus is a standalone TSV pipeline and preserves library,
module, function-boundary, object hash, function hash, and source-version
provenance without depending on Ghidra's database format.

The third evidence lane covers version-skewed SDK code that the exact 2.5
FIDs could only identify in patches. In each overlay, `MTX`, `SMP`, and `CMB`
appear between independently anchored `REG` and `GEO` code. All 68 public
functions keep the same order and size in GAME and OPEN at a constant
`0x2022c` delta. `SMP` and `CMB` are word-identical across overlays; `MTX`
preserves every instruction shape, with only its private-data and `printf`
references changing. Public names follow the archived 2.5/2.60 XDEF order and
the bodies use the corresponding GTE operations, but their sizes and some GTE
encodings differ from pinned Release 2.5. They therefore use the explicit
`sdk-lineage-supported` channel and do not claim an exact SDK revision.

The same confidence channel now covers `LIBSND/VMANAGER`'s four
`SsUtKeyOn`/`SsUtKeyOff` rows and `LIBGPU/VSYNC`'s four public/private rows.
The VMANAGER names combine exact GAME/OPEN instruction shapes with archive
order, header prototypes, and neighboring exact anchors. The two retail VSYNC
functions per overlay preserve every instruction shape, implement the public
`VSync(int mode)` behavior plus its private hardware/root-counter worker, carry
the same `vsync.c` revision string, and occupy the expected library sequence.
Their exact Release 2.5 object boundaries differ, so none is advertised as an
exact archive match.

The exact object spans also establish a partial final linked order. The full
per-image placement tables, boundaries, gaps, and contiguous chains are kept in
[object-link-order.md](object-link-order.md).

## Cross-overlay order evidence

Psy-Q order is useful evidence, but it is not MSVC incremental-link metadata.
The project does not treat archive storage order or addresses as identities.
The controlled PSYLINK 1.17 results below establish narrower behaviours instead
of relying on an MSVC analogy.

What has been tested is narrower:

| question | test and result | evidentiary limit |
|---|---|---|
| Does the available GCC 2.6.0 PSX code-generation probe preserve C definition order within one object? | `tests/compiler_mips_smoke.py` compiles deliberately non-alphabetical exported functions at `-O0` and `-O2`; both preserve their relative source order. | This calibrates the pinned Decompals rebuild, not the still-unproven retail compiler binary/profile. An explicit inline body may be deferred to the end at `-O0`, so even this is not a universal all-function rule. |
| Can optimization remove a function body and shift later offsets? | The same probe observes an explicitly inline static body at `-O0`, its omission at `-O2`, and changed offsets for later functions. | This demonstrates why VA is not identity; it does not identify King's Field's optimization flags. |
| Did one actual linked sequence keep the same order in both overlays? | `kf lineage` verifies the retail chain described below. | This proves only the admitted chain, not a universal linker rule. |
| Does direct input-object order affect linked order? | `tests/psylink_order_smoke.py` links pinned `A02.OBJ`/`A03.OBJ` in both orders with the actual PSYLINK 1.17; their symbol addresses reverse with the command line. Reversing two root objects does the same. | This supports order reasoning only after an object boundary/input sequence is independently known. |
| Does archive member storage order determine lazy-extraction placement? | The same test creates `A02,A03,A04` and `A03,A02,A04` archives. With identical unresolved roots, their CPE and symbol outputs are byte-identical (`GetRCnt`, `SetRCnt`, `StartRCnt` order). | No: do not infer linked order from the archive's member listing. The exact internal extraction algorithm remains outside this fixture's claim. |
| Is this MSVC incremental RVA behaviour? | No. Reordering direct inputs changes addresses, while no corresponding incremental-link metadata or padding mechanism has been identified or used by the tooling. | Treat the MSVC analogy as rejected, not as a matching rule. |

`config/evidence/overlay_lineage.tsv` captures an observed stronger case. A
contiguous 16-function audio-related chain appears in both `GAME.EXE` and
`OPEN.EXE` with the same sizes and a constant `0x2022c` address delta. Across
1,509 instructions, 1,509 preserve the same opcode/register shape and 1,338 are
word-identical before masking linked immediates. Run the executable-backed and
archive-backed regression check with:

```sh
kf lineage
```

The same command verifies the three GTE object chains: 21 `MTX`, 29 `SMP`, and
18 `CMB` function pairs. It also verifies that the apparent starts inside
`PushMatrix` and `PopMatrix` are branch destinations in the same routine, and
that both overlays expose the same 20-entry private matrix-stack layout.

It additionally verifies eleven display/TMD pairs at delta `0x5280` and both
VSYNC pairs at delta `0x2031c`. The display/TMD functions remain game-owned;
only the independently evidenced VSYNC pair enters the provider exclusion.

The command also asserts the corrected `lui`/`lw` prefix in both executables,
all four direct helper callsites, the helper's ambiguous-provider FID row, and
the dispatcher's absence from the provider-exclusion TSV. The helper must have
exactly two direct `jal` callers per overlay, both inside the candidate
`SsSeqCalledTbyT` body.

That repeated order proves common linked code, not Sony ownership. The FID pass
specifically prevents the earlier overreach here: Release 2.5
`LIBSND.LIB/SSCALL.OBJ` contains a named 0x244-byte `SsSeqCalledTbyT`, but it
does not match either 0x2f8-byte retail dispatcher. The preceding 0x30-byte
helper exactly matches `memcpy`/`_memcpy`-shaped functions from three SDK object
identities, and the same body occurs four times per retail image. That proves a
Sony `memcpy` body while leaving the exact archive member ambiguous, so all
eight copies use `fid-release25-ambiguous`. The dispatcher itself is not
classified as vendored. Its structural name and cross-overlay lineage remain a
useful WIP hypothesis, kept separate from provider ownership.

## TSV contract

Every provider row must match one `(image, va, size)` in `functions.tsv`.
`library` and `module` name the archive/object candidate, while `member_offset`
is relative to its `.text` or signature start. `source_version`, `evidence`,
`confidence`, and `provenance` are mandatory. `name` may remain empty when
ownership is stronger than function identity; `aliases` is semicolon-separated.

The confidence channels are intentionally mechanical:

- `exact-release25`: large unique relocation-masked section match;
- `exact-release25-short`: unique but smaller exact section match;
- `exact-release25-ambiguous`: exact bytes with multiple archive members;
- `exact-release25-complete`: reviewed complete tiny object;
- `fid-release25`: unique substantial relocation-masked function ID built from
  the pinned Release 2.5 objects;
- `fid-release25-ambiguous`: fully fixed function ID with one normalized name
  but multiple possible Release 2.5 archive identities;
- `psyq260-signature`: unique later-corpus wildcard match; and
- `psyq260-signature-ambiguous`: later signature shared by archive members; and
- `sdk-lineage-supported`: exact cross-overlay instruction shape/order plus
  SDK archive/header and instruction-semantic agreement, with the exact source
  revision unresolved.

`kf-retail-validate` rejects provider rows without a structural function,
changed sizes, duplicate addresses, invalid confidence, or missing provenance.
It does not promote signature candidates to exact claims.

## Iteration workflow

Run the reproducible candidate pass inside the default Nix shell:

```sh
kf-fid-census \
  --exe-dir /path/to/kings-field-japan-retail/disc

kf-vendored-seed \
  --exe-dir /path/to/kings-field-japan-retail/disc
```

The output is written below `build/vendored-seed`; writing directly under
`config/retail` is refused. Diff the generated rows against the hand-owned TSV,
review new or missing boundaries in `functions.tsv`, then admit only the rows
whose ownership evidence survives inspection. The seeder runs the standalone
FID lane by default; `--no-fids` exists for isolating the older evidence lanes.
This preserves manual edits when the structural census or signature corpus
changes.
