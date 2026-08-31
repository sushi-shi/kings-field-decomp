# Vendored function inventory

`config/retail/functions_vendored.tsv` is a manually owned provider layer over
the structural function census. It answers “which admitted functions came from
an external runtime/library?” without changing function boundaries or treating
an analyzer label as ownership proof.

Only the original Japanese King's Field (`SLPS-00017`) is in scope. No KFII or
KFIII executable contributes to this list.

## Current result

| image | Release 2.5 exact | Psy-Q 2.60 signature | manual SDK lineage | total |
|---|---:|---:|---:|---:|
| `PSX.EXE` | 8 | 0 | 0 | 8 |
| `GAME.EXE` | 184 | 77 | 2 | 263 |
| `OPEN.EXE` | 183 | 62 | 2 | 247 |
| **total** | **375** | **139** | **4** | **518** |

The 518 rows comprise 504 named functions and 14 anonymous internal functions
whose containing Sony object is known. Provider counts are: 116 `LIBGTE`, 110
`LIBGPU`, 100 `LIBCD`, 91 `LIBAPI`, 59 `LIBSND`, 36 `LIBSPU`, two `LIBSN`, and
four startup functions attributed to `NONE2.OBJ`. No zlib or other third-party
library has been identified, so none is claimed in the TSV.

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
objects are therefore described as signatures, not FIDs. A future FID database
generated from the hash-pinned Release 2.5 objects can be another candidate
lane, but it must preserve the same version/provenance distinction.

## Cross-overlay order evidence

Psy-Q order is useful evidence, but it is not MSVC incremental-link metadata.
The project does not currently make a generic claim about PSYLINK object or
archive extraction order. Those behaviours still need a controlled PSYLINK
1.17 experiment; archive membership and address order alone are not that test.
Addresses are therefore never treated as identities by themselves.

What has been tested is narrower:

| question | test and result | evidentiary limit |
|---|---|---|
| Does the available GCC 2.6.0 PSX code-generation probe preserve C definition order within one object? | `tests/compiler_mips_smoke.py` compiles deliberately non-alphabetical exported functions at `-O0` and `-O2`; both preserve their relative source order. | This calibrates the pinned Decompals rebuild, not the still-unproven retail compiler binary/profile. An explicit inline body may be deferred to the end at `-O0`, so even this is not a universal all-function rule. |
| Can optimization remove a function body and shift later offsets? | The same probe observes an explicitly inline static body at `-O0`, its omission at `-O2`, and changed offsets for later functions. | This demonstrates why VA is not identity; it does not identify King's Field's optimization flags. |
| Did one actual linked sequence keep the same order in both overlays? | `kf lineage` verifies the retail chain described below. | This proves only the admitted chain, not a universal linker rule. |
| Does input-object order or lazy archive extraction determine retail order? | Not yet directly tested under PSYLINK 1.17. | Do not infer source-file boundaries or archive extraction history from adjacency alone. |
| Is this MSVC incremental RVA behaviour? | No corresponding incremental-link metadata or padding mechanism has been identified, and none is used by the tooling. | Treat the MSVC analogy as rejected, not as a matching rule. |

`config/evidence/overlay_lineage.tsv` captures an observed stronger case. A
contiguous 16-function audio-related chain appears in both `GAME.EXE` and
`OPEN.EXE` with the same sizes and a constant `0x2022c` address delta. Across
1,509 instructions, 1,509 preserve the same opcode/register shape and 1,338 are
word-identical before masking linked immediates. Run the executable-backed and
archive-backed regression check with:

```sh
kf lineage
```

The command also asserts the corrected `lui`/`lw` prefix in both executables,
the four SSCALL provider rows, and all four direct helper callsites. The helper
must have exactly two direct `jal` callers per overlay, both inside
`SsSeqCalledTbyT`.

That repeated order proves common linked code, not Sony ownership on its own.
The `SSCALL` promotion adds independent SDK evidence: the pinned Psy-Q corpus
names `SsSeqCalledTbyT` in `LIBSND.LIB/SSCALL.OBJ`, the retail dispatcher has
the same sound-event call sequence, and its leading `lui`/`lw` pair corrects a
previously missed eight-byte function prefix in both overlays. The private
0x30-byte copy helper is byte-identical across overlays, immediately precedes
the dispatcher, and is called only from that dispatcher. Those combined facts
are the basis for `manual-sdk-lineage` ownership.

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
- `psyq260-signature`: unique later-corpus wildcard match; and
- `psyq260-signature-ambiguous`: later signature shared by archive members; and
- `manual-sdk-lineage`: reviewed ownership based on a named SDK symbol/control-
  flow lineage plus reproducible cross-overlay instruction-shape evidence, used
  when version skew prevents an exact archive-object match.

`kf-retail-validate` rejects provider rows without a structural function,
changed sizes, duplicate addresses, invalid confidence, or missing provenance.
It does not promote signature candidates to exact claims.

## Iteration workflow

Run the reproducible candidate pass inside the default Nix shell:

```sh
kf-vendored-seed \
  --exe-dir /path/to/kings-field-japan-retail/disc
```

The output is written below `build/vendored-seed`; writing directly under
`config/retail` is refused. Diff the generated rows against the hand-owned TSV,
review new or missing boundaries in `functions.tsv`, then admit only the rows
whose ownership evidence survives inspection. This preserves manual edits when
the structural census or signature corpus changes.
