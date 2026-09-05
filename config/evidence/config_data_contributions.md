# Config-owned SDK data enters the default comparison path

## Function Match Plan and scope

Baseline: `dfa8baa`. Add a data-only build/comparison lane for independently
supported config owners. Start with the two complete `rsin_tbl` contributions
already established from `LIBGTE.LIB/GEO.OBJ`; do not substitute retail bytes
for the provider side or turn an arbitrary census prefix into an owner.

The six `rsin`/`sin_1`/`rcos` bodies retain the per-function snapshots and
unmasked SDK/retail verdicts in [psyq_rsin_table.md](psyq_rsin_table.md). No
function body, claim, parameter, type, call, instruction, delay slot, compiler
profile or provider classification changes. This is data comparison and build
infrastructure, not new game-function progress.

Verification plan: create real data-only targets from retail and separate
provider bases from the original SDK. Compare whole sections and named
allocations, require native objdiff data-only reports, and independently link
both sides at their image-local retail addresses. Recompute these proofs for
reachability; a config row or an old report is not sufficient. Test invalid
extents, bindings, sections, candidate relocations, provenance and stale/missing
artifacts. Rebuild all source/target objects and run the full default gate.

## Owners and independent source

`data_contributions.tsv` explicitly maps an image-local identity to its SDK
provider, archive/member hash, whole section, alignment and evidence. Address,
size and linkage come from the existing `data_identities.tsv` owner. They are
not repeated in a second claim or added to the function manifest. Inventory
validation checks the new schema, identity, load extent, alignment, namespace,
ordering and overlap; build-time validation also rejects source ownership
overlap.

| Contribution | Retail owner | Complete bytes | SDK source |
| --- | --- | --- | --- |
| `game.sdk.libgte_geo.data` | GAME `rsin_tbl` at `80057070` | 2048 | `LIBGTE.LIB/GEO.OBJ:.data` |
| `open.sdk.libgte_geo.data` | OPEN `rsin_tbl` at `800367e8` | 2048 | The same complete SDK section, independently placed in OPEN |

The pinned member hash is
`7e62d2e92b7a30001549db9b60abaed0811db7db2ed213e3eca2e62d8a7db36e`.
The SDK section has raw LNK alignment tag 8, exports global `rsin_tbl` at offset zero,
contains exactly 2048 bytes and has no data relocations. The importer checks
the original object hash, the entire section, the export list and its offset.
The raw tag is not a byte count: later
[PSYLINK calibration](../../docs/patterns/psyq-section-alignment.md) establishes
four-byte alignment and corrects both converted ELF objects accordingly.
It refuses partial extents, extra exports, unknown allocation records, patches
and reservations instead of discarding unsupported content.

The base is a mechanical SDK-object conversion, not a C/assembly reconstruction
of a game routine: GAS includes the extracted complete SDK payload and emits
its exported object identity. The payload uses a byte-aligned temporary
PROGBITS section; objcopy restores the original SDK section name/alignment
without changing any payload, extent or symbol. GAS's empty ordinary `.data`
is checked before removal. A 24-byte control demonstrates why this matters:
ordinary MIPS GAS `.data` rounds it to 32 bytes at alignment 16. Byte,
halfword and larger non-rounded controls verify the conversion and both relinks,
including a three-byte section with alignment eight. The resulting ELF
must preserve the complete section and sole named allocation. The target is
created independently by the delinker from the complete retail range. Neither
side synthesizes a function symbol, code body or alignment tail.

## One false instruction candidate rejected

GAME `800574c0` is offset `450` within the SDK table. Its halfwords are 3068
and 3073, forming the word `0c010bfc`. Interpreted as an instruction, this looks
like `jal 80042ff0`; the old range-filtered `instruction-word` row was therefore
candidate code. The full SDK contribution and its absent data patches prove
these are sine samples, not a call or pointer. The row is explicitly retained
as `rejected` with `reviewed-non-pointer` confidence and SDK evidence provenance.
Its raw target, site and opcode proposal remain visible for audit.

No other candidate is silently discarded. A live relocation whose primary or
paired word intersects a supported config contribution blocks delinking until
the relocation or its rejection is independently established.

## Default integration and comparison contract

`kf build` creates targets under `build/delink/<image>/data/` and provider bases
under `build/objdiff/<image>/data/`. Archive paths, provider-tool identity,
contribution/identity TSVs and importer scripts are incremental dependencies.
The normal objdiff projects contain these two data-only units, alongside the
112 source TUs. Pairing rows identify them as `config-data-load`; SDK function
exclusions and game function denominators remain unchanged.

Both native data-only units report `total_data=matched_data=2048` and a 100%
whole `.data` section, with no functions or nonzero code extent. Missing,
duplicated, incomplete or stale native reports fail. The independent gate
also validates all runtime sections, ELF storage flags/alignment, object
name/size/offset/type/linkage/visibility, exact provider payload and target/base
section bytes. Two identical-but-wrong objects cannot pass by matching each
other: the original SDK section is read separately during verification.

GNU ld then places each complete target and provider section using the owned
symbol's actual offset. Each side must reproduce all 2048 retail bytes, at
one section base, without scattering symbols, overriding definitions or
cropping sections. Config targets also join `kf verify roundtrip` and its
cross-unit overlap checks. The config contribution is counted as compared in
reachability only after current provider/object/native-report/relink checks
pass for the exact image, address, extent and identity. Reachability JSON
includes the SDK, object and native-report hashes and both complete relink
witnesses, not just a cached pass label.

Source-owned data and config-owned data retain distinct reporting categories.
A successful SDK comparison is not a new game source TU, a reconstructed game
function, or proof of exhaustive reachable-byte coverage.

## Reproduction and remaining boundary

Inside `nix develop`, after hash-validated retail initialization:

```sh
python -m unittest tests.test_config_data tests.test_psyq_rsin_table
kf build compare
kf verify data
kf verify roundtrip --unit game.sdk.libgte_geo.data
kf verify roundtrip --unit open.sdk.libgte_geo.data
kf verify reachability --output build/config-data-reachability.json
kf build
```

The first provider supports complete, non-relocating SDK `.data` contributions.
It does not silently generalize that proof to SDK strings with unknown owners,
pointer tables, BSS/overlay RAM, partially known game objects or census gaps.
Those still need ownership/provider/source and relocation work. The overall
goal remains incomplete; no linked-executable equality is claimed.

## Final verification

All 112 source TUs were recompiled and all targets redelinked. Every existing
compiled object, all 1717 pre-existing target objects and all 484 function
score rows remain byte-identical to the baseline. All 354 exact game functions
remain exact. Two SDK data-only bases and two corresponding targets are added;
neither changes the game-function denominator or banks a function.

Both complete SDK data contributions pass the native report and independent
provider/target relinking: **4096 bytes newly default-compared**. Source data
remains 13/60; SDK config data is separately 2/2. Target relinking becomes
108/114, adding the two valid data-only targets to the unchanged 106/112 source
targets. The same six source-section placement conflicts remain.

Reachability still finds 666 config-owned ranges: GAME 387 and OPEN 279. Two
now have current complete comparison proofs, reducing unpaired config ranges
to **664** (GAME 386, OPEN 278). Rejecting the table's false instruction word
removes one GAME candidate-reference path; other uncertainty/overlap/indirect
diagnostics remain. Full reachable-byte coverage is still unproven.

All 530 local repository tests, Ruff and `git diff --check` pass. The flake
checks pass, including the native data-only objdiff control; 57 local-retail
and oracle controls are skipped only in the clean sandbox. The full default
build remains red on the explicit source-data, placement and remaining
reachability failures. No linked-image equality or broader config closure is
claimed.
