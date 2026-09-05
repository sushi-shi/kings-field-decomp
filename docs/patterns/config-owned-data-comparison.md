# Comparing config-owned data without circular evidence

A config inventory is an ownership model, not a reconstructed object. Giving
a range a name or carving its bytes from retail does not make it matched.
The two sides must have independent origins, and both must honor the complete
allocation and section topology.

The current implementation uses the following chain:

1. `data_contributions.tsv` selects a supported image-local identity and an
   independently supplied whole SDK section. VA, extent and linkage remain in
   `data_identities.tsv`; the provider member is pinned by hash.
2. The delinker reads the entire retail allocation. The SDK importer reads the
   entire original section and its metadata, then converts it with GAS/objcopy.
   Neither path reads the other's payload or uses an arbitrary matching slice.
   A byte-aligned temporary PROGBITS section avoids GAS's ordinary `.data`
   tail rounding; objcopy restores the original SDK name/alignment, not its
   payload or extent. Byte/halfword and non-16-byte relink controls cover this.
3. The shared native objdiff project/report requires the full data section at
   100%, with the expected byte totals and no invented functions or code.
4. The strict gate checks every runtime section and the named allocation's
   extent, offset, type, flags, alignment, binding and visibility. It also reads
   the independent provider payload again; equal corrupt target/base objects
   are not sufficient.
5. GNU ld places each whole target and provider section independently. Every
   initialized byte must reproduce retail at the owned section base. No tail
   trimming, per-symbol scattering or definition override is allowed.
6. Reachability accepts only a current full comparison for the exact image,
   owner address, extent and identity. Missing/stale native reports or objects
   restore the unpaired-config diagnostic; they are not omissions from scoring.

The SDK importer currently supports non-relocating `.data` contributions with
one complete exported object. Unsupported patch/reservation/local-symbol or
allocation records fail. This is an explicit unsupported case, not permission
to erase relocations. Alignment comes from the original SDK section: objcopy
may set that ELF constraint, but may not change section bytes, size or symbols.
Do not use this mechanism to force a compiler-probe game object to fit retail.

The [initial SDK campaign](../../config/evidence/config_data_contributions.md)
provides two complete 2048-byte controls and a rejected fake `jal` made from
sine samples. `tests/test_config_data.py` covers the independent byte origins,
native report integration, real GNU relinks and the rejection boundaries.
Source/data progress and vendor verification remain separate. Successful
individual contributions do not prove whole-image linked equality.
