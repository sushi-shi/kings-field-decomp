# Vendored function inventory

`config/retail/functions_vendored.tsv` is a manually owned provider layer over
the structural function census. It answers “which admitted functions came from
an external runtime/library?” without changing function boundaries or treating
an analyzer label as ownership proof.

Only the original Japanese King's Field (`SLPS-00017`) is in scope. No KFII or
KFIII executable contributes to this list.

## Current result

| image | Release 2.5 exact evidence | Psy-Q 2.60 signature candidates | total |
|---|---:|---:|---:|
| `PSX.EXE` | 8 | 0 | 8 |
| `GAME.EXE` | 184 | 77 | 261 |
| `OPEN.EXE` | 183 | 62 | 245 |
| **total** | **375** | **139** | **514** |

The 514 rows comprise 502 named functions and 12 anonymous internal functions
whose containing Sony object is known. Provider counts are: 116 `LIBGTE`, 110
`LIBGPU`, 100 `LIBCD`, 91 `LIBAPI`, 55 `LIBSND`, 36 `LIBSPU`, two `LIBSN`, and
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
- `psyq260-signature-ambiguous`: later signature shared by archive members.

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
