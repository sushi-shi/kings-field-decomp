# Remaining SDK object audit

The 54 entries previously supported only by FIDs or signatures have all been
checked against the supplied Psy-Q Release 2.5 objects. **53 entries are
resolved**: 51 acquire complete code-section evidence and two merge into
identified objects. One entry retains an explicit limit below. This is SDK
attribution work, not reconstructed game progress or an objdiff banking result.

## Verdicts

| Original entries | Count | Verdict |
| --- | ---: | --- |
| LIBAPI: A02, A03, A04, A06, A07, A08, A09, A11, A12, A13, A20, A21, A22, A23, A25, A32, A37, A50, A51, A52, A53, A54, A56, A61, A65, A69, A74, A75, A76, A80, C112, C171, C173, C19, C20, C21, C23, C24, C25, C27, C43, C44, C46, C47, C52, C63, C73 | 47 | All 16 bytes of each member's code match, including delay slots and padding; 79 image occurrences. No relocation masking. |
| LIBSND/PLAY | 1 | Entire 0x2c-byte code section matches; decoded call identifies PLAY. |
| LIBSND/SESKOFF | 1 | Entire 0x3c-byte code section matches in GAME and OPEN. |
| LIBSPU/S_GTSA | 1 | Entire 0x30-byte code section matches in GAME and OPEN. |
| LIBSND/PLAY\|SCNON | 1 | OPEN calls SeqPlay, resolving this entry to PLAY; merge with the existing PLAY identity. |
| LIBSND/VS_MONO | 1 | SsSetStereo matches VMANAGER's XDEF at 0x24; merge into the existing revision-skewed VMANAGER family. |
| LIBCD\|LIBGPU\|LIBSPU / ISO9660\|SYS\|S_SRMP | 1 | Six memcpy copies have resolved containing families; the two audio-dispatcher copies retain ambiguous ownership. |
| NONE2.OBJ/NONE2 | 1 | The pinned medium's previously unstaged `H2000/LIB2000/NONE2.OBJ` has an exact relocation-masked 0x18-byte code-section match in GAME and OPEN. |

After merging the two redundant identity entries, the inventory has **111
entries: 81 with exact code evidence, 29 with lineage evidence, and one
remaining**. The 29-entry lineage category has priority over an exact function
inside a differing containing object. It is not a count of 29 independently
proved full-object mismatches.

## Object evidence still needed

To replace every lineage judgment with exact object-level code evidence, the
repository needs stronger evidence for **29 named module identities**:

| Library | Modules | Count |
| --- | --- | ---: |
| LIBCD | BIOS, ISO9660 | 2 |
| LIBETC | INTR, PAD | 2 |
| LIBGPU | SYS, VSYNC | 2 |
| LIBGTE | CMB, MSC, MTX, SMP | 4 |
| LIBSND | CRES, DECRE, PAUSE, REPLAY, SEPINIT, SEQINIT, SEQREAD, SSCALL, SSCLOSE, SSINIT, SSOPEN, SSPLAY, SSVOL, STOP, TEMPO, VMANAGER | 16 |
| LIBSPU | SPU, S_INI, S_SRMP | 3 |
| **Total** |  | **29** |

These **29 known object identities need stronger closure evidence**. Several
modules have direct size, instruction, data or boundary evidence for revision
skew; others have only partial exact functions and no complete object-level
comparison.

The unresolved audio-dispatcher `memcpy` does not yet justify counting a 30th
object. A retail-matching `SSCALL.OBJ` may contain the helper and resolve both
the SSCALL lineage entry and the helper's ownership. If that object instead
shows the helper was separately linked, its owning object becomes a 30th
required identity. Until one of those layouts is observed, **29 is the proved
minimum number of identities needing evidence and 30 is the conditional count
for the currently known gaps**.

The preferred closure evidence is a matching retail-era Psy-Q archive, the
individual `.OBJ` files, or preserved SDK source with version and toolchain
provenance. Those artifacts can establish section boundaries, relocations,
local symbols, data, BSS and member identity rather than code bytes alone.

The overlay executable comparison exposes a particularly large `SPU.OBJ`
revision marker. Retail `_spu_ioctl` bounds its selector at `0x212`, then jumps
through a 531-entry table at GAME `0x80013320` or OPEN `0x800127ec`. The two
`0x84c`-byte tables have identical target offsets across the overlays and every
entry resolves to one of 54 blocks in the corresponding `_spu_ioctl` body. The
supplied Release 2.5 member has no equivalent table of that extent. This one
missing read-only contribution explains more than 93% of the initial section
shortfall in both saved links, while the supplied member's dispatcher code is
also substantially different. A matching `SPU.OBJ` is therefore both an
object-closure requirement and the single largest known executable-layout
input.

If matching artifacts cannot be found, reconstructing a vendored object can be
used as a forensic fallback to test source shape, calls and object boundaries.
It must remain excluded from game progress, and a source reconstruction alone
does not prove that its generated object was the historical linked input. Never
hand-match Sony functions merely to raise the game reconstruction percentage.

This work is needed only for exact SDK revision and object-ownership closure.
The 29 named modules are already excluded from game progress on strong lineage
evidence; their absence does not create 29 unidentified game translation units.

The exact additions are curated in
[complete code sections](../config/evidence/psyq_release_25_complete_objects.tsv)
and [relocation-masked code sections](../config/evidence/psyq_release_25_text_sections.tsv).
They do not establish complete data, BSS, linked placement, or every masked data
referent. Only the relevant existing provider rows were promoted; the admitted
function census is unchanged.

## Call-target ambiguity and shared data

PLAY and SCNON each have 326 fixed bits and one masked J26 relocation in the
same 44-byte code. Their archive XREFs differ: PLAY calls `SeqPlay`, whereas
SCNON calls `SpuVmNoiseOn`. Masked bytes alone cannot choose between them.

| Retail image | Call site | Decoded target | Result |
| --- | --- | --- | --- |
| GAME | 0x8004ae18 | SeqPlay, 0x800476e8 | PLAY |
| OPEN | 0x8002abec | SeqPlay, 0x800274bc | PLAY |

Both alternatives remain in the code-section evidence table because that table
measures masked bytes. The curated function inventory records the stronger
call-referent decision and removes OPEN's incorrect SsSetNoiseOn alias.

SESKOFF has 454 fixed bits and one J26 relocation. Its calls at GAME 0x8004b6c8
and OPEN 0x8002b49c both target SpuVmSeKeyOff, matching the archive XREF.
S_GTSA has 326 fixed bits and three relocations: HI16/LO16 `_spu_fd`, then J26
`_spu_ioctl`. The calls at GAME 0x80046818 and OPEN 0x80026638 target
`_spu_ioctl`; the loaded addresses are respectively 0x80069000 and 0x80046de0.
The code-section verdict masks those relocation fields, rather than claiming
complete linked data equality.

SsSetStereo's 16 bytes preserve all 96 unmasked bits of VMANAGER offset 0x24.
It follows SsSetMono at 0x10 and precedes SpuVmCheck at 0x34. Both mode setters
write the same halfword: GAME 0x80057cfc, OPEN 0x8003746c. The later VS_MONO
signature partition is absent from the supplied archive. This supports
VMANAGER lineage, whose complete retail object is already revision-skewed.

## memcpy ownership and remaining limits

All eight copies match the fully fixed 0x30-byte SDK function. Six also have
containing-object evidence from the archive's local symbol and local J26 patch
targets, corresponding retail callers, and member order:

| Family | SDK local offset | GAME copy | OPEN copy | Retail callers |
| --- | --- | --- | --- | --- |
| LIBCD/ISO9660 | 0x844 | 0x8003d050 | 0x8001ce70 | CD_newmedia, CD_cachefile |
| LIBSPU/S_SRMP | 0x0 | 0x80041040 | 0x80020e60 | SpuSetReverbModeParam |
| LIBGPU/SYS | 0x242c | 0x80052938 | 0x8003261c | _addque2 |

These six rows now use the identified family's lineage evidence. None of the
three full Release 2.5 code sections occurs under its relocation mask in either
retail overlay; the matching helper does not make its containing object exact.

GAME 0x8004a52c and OPEN 0x8002a300 each have two direct callers, both inside
SsSeqCalledTbyT. The supplied SSCALL object is 0x244 bytes and does not contain
this preceding helper; retail has a larger dispatcher. Neither the repeated
body nor adjacency proves which revised object supplied the helper. Resolving
this last memcpy entry requires an archive revision or source evidence that
contains this dispatcher/helper arrangement.

The pinned Release 2.5 medium stores `NONE2.OBJ` under `H2000/LIB2000` rather
than its general `PSXLIB/LIB` directory. The file is dated 1994-10-18 and has
SHA-256 `38662381b57cdbebb8d083842fd23914bad1d4c66303beb871f2f5248c38774c`.
`psy-k` decodes one 24-byte text section, HI16/LO16 relocations against
`.sdata`, a J26 relocation against `main`, and XDEF offsets 0 and 8 for
`__main` and `__SN_ENTRY_POINT`. All 134 non-relocation instruction bits match
exactly once in GAME at 0x8003ac54 and once in OPEN at 0x8001aa74. Runtime 2.6
and the later 4.3-era SDK bundle contain the same 244-byte object byte for byte.

## Archive search around the retail cutoff

[FromSoftware dates King's Field to 1994-12-16](https://www.fromsoftware.jp/ww/detail.html?csm=001).
The Archive.org `ps1_sdks` catalog and its related SDK uploads were searched
for every library release at or before that date:

| Medium | Artifact result | Comparison result |
| --- | --- | --- |
| [Runtime Library 2.0](https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%202.0%20%28Japan%29%20%28En%2CJa%29_DTL-S2160_redump.zip) | DTL-S2160; libraries dated 1994-05-20 through 1994-07-15; no LIBSPU or NONE2 | No complete relocation-masked match for any of the 29 lineage modules. The all-function FID pass found 307 hits, but only nine were substantial unique matches. |
| [Release 2.5 floppies](https://archive.org/download/ps1_sdks/Floppies.rar) | Collection dated 1994-12-29; already hash-pinned by the flake | All 17 `.LIB`/`.OBJ` paths are now preserved and individually hash-checked. The H2000 set is kept separate because its LIBAPI archive differs. Its `NONE2.OBJ`, internally dated 1994-10-18, closes the startup identity exactly. |
| [Runtime Library 2.6](https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%202.6%20%28Japan%29%20%28En%2CJa%29_DTL-S2170_redump.zip) | Libraries dated 1995-01-24/25, after retail | No complete match for the 29 lineage modules. Several later split sound objects match subsections of the retail VMANAGER/SPU runs, and NONE2 is unchanged; these are useful controls, not evidence that Runtime 2.6 was linked. |
| [User-suggested PlayStation SDK](https://archive.org/details/playstation-sdk_202301) | 93,064,381-byte `PlayStation 1.7z` payload | Despite its 1994 catalog date, its contents are SDK 4.4 and later Net Yaroze material, so it is not a retail-era library source. |

Downloaded-payload SHA-256 values are
`7bcffd476485f9a19bdadb9b33020127e0c138caf0a32f5580c9eaed2fc57c81`
for Runtime 2.0,
`7913a008fc2c3e3059b8a389f4dfb737377a35b72b43cb08b33f0f5524785bd3`
for Runtime 2.6, and
`d980197f1b7d183e427a5f3a1a348f86154a1c729a5477e09799f79847e124d4`
for the user-suggested upload.

The broader Archive.org catalog also exposes Runtime 3.0 and later discs,
Runtime 4.3-era SDK bundles, and 1998 SDevTC host-tool discs. No Runtime
2.1-2.5 distribution dated on or before 1994-12-16 was found. Runtime 2.0 is a
real lower-bound control, but its whole objects are too early. The next useful
artifact is therefore an intermediate 1994 runtime archive, preserved source,
or individual object set; later retail executables can corroborate code but do
not recover the original relocation and section records by themselves.

## Reproduction and provenance

Enter `nix develop` and initialize the hash-identical retail files with `kf init`.
The existing seeder independently re-extracts the admitted archive members,
checks every byte/mask, XDEF offset, size and image occurrence count:

```sh
kf-vendored-seed --exe-dir /path/to/retail/disc --no-fids --no-signatures \
  --output build/sdk-object-audit/verified-seed.tsv
```

The output is a proposal; it must not overwrite the curated inventory. In
particular, its masked PLAY/SCNON result does not include the call-target
review above. Inspect individual members with `psyk extract "$PSYQ_LIB/LIBSND.LIB"`
in a scratch directory, then `psyk list --code PLAY.OBJ`. Use
`kf sema --image game xref Snd_play --callees` and the separate `--image open`
query to check the decoded calls.

The general and H2000 directories are separate matcher inputs because both
contain a file named `LIBAPI.LIB` with different bytes. Run the second corpus
directly with the exposed path:

```sh
kf-fid-census --sdk-lib-dir "$PSYQ_H2000_LIB" \
  --exe-dir /path/to/retail/disc \
  --corpus-output build/sdk-object-audit/h2000-corpus.tsv \
  --output build/sdk-object-audit/h2000-matches.tsv
```

That pass produces 179 signatures and ten high-confidence retail hits. All ten
come from H2000 LIBAPI members whose object bytes also occur in the general
archive: six PSX stubs and Enter/ExitCriticalSection in the overlays. Neither
2MBYTE nor 8MBYTE produces a retail function hit, and a separate scan of each
complete 0xc4-byte text section under all 18 native relocations finds no retail
occurrence. The demo CARD object is not run twice because it is byte-identical
to `$PSYQ_LIB/CARD.OBJ`.

For another extracted SDK release, run the same function census without
changing the pinned environment:

```sh
kf-fid-census --sdk-lib-dir /path/to/release/LIB \
  --exe-dir /path/to/retail/disc \
  --corpus-output build/sdk-object-audit/release-corpus.tsv \
  --output build/sdk-object-audit/release-matches.tsv
```

Pinned member SHA-256 values for the non-stub decisions:

| Member | SHA-256 |
| --- | --- |
| PLAY | d23ec6340639ffd0d1745b2ac6b83f25eaf2c783383501d5de23d42007d2ed91 |
| SCNON | e3bd8da06d682dae938b95dcda67722519f6bf14657bc59c86522dd071b5e1d2 |
| SESKOFF | 320deb082fb8a97fb9392c53376cc6901adaf469cd5ff006fe80afb2a2e901c2 |
| S_GTSA | 62724247f04f0ff014735b7081ea3fae2eade8e3c9a357f45ef5cae3bc6505df |
| VMANAGER | 96242c8cb04ee3038d1f91133e4155a9a3356870301523b21fe3778f0ef4c570 |
| ISO9660 | 444d4d03b1fee66f8e90726c6bcbbae6f10db8cfef8131054f5cd7a6917901af |
| SYS | e7ef7c9a0b7480447ddfffac8a8e5bcc4057c6ff64f7b7cf49ce272402d494a1 |
| S_SRMP | 809188e34e6447ec50aeb16a0dde8cf2d3bcf998d29e481fb129e49ce3bdf9a5 |
| NONE2 | 38662381b57cdbebb8d083842fd23914bad1d4c66303beb871f2f5248c38774c |

## Validation

The existing exact-object seeder passed after re-extraction with 466 candidate
rows, including both NONE2 placements. Repository lint, the full test suite,
`nix flake check -L`, and `git diff --check` passed. The full build
recompiled all 99 units; all 13 vendored source units remain exact. Its final
checks still fail on the existing data-ownership, BSS-placement and
reference-closure gaps. The retired composed `kf link` experiment also
consumed the staged NONE2 object in GAME and OPEN; both archived reports retain
its pinned SHA-256. No game function was added, removed or reclassified.
