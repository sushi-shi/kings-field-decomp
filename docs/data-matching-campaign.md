# Data matching and linked-image preparation

## Objective and completion boundary

Account for every byte reachable from game functions in each independently
linked image, through source `DATA`/`RODATA` ownership or reviewed configuration
TSVs. Delink those ranges faithfully and compare the reconstructed data, with
enough placement and relocation evidence to support linked-executable comparison.
Data matching is enabled by default; newly exposed mismatches are failures to
resolve, not scores to preserve by masking bytes.

Completion requires all of the following, not merely green claimed-data objects:

- Image-qualified roots cover every admitted game function, not just exact or
  already-reconstructed functions. Follow code/data references transitively,
  including referenced SDK/runtime code and its data, without counting library
  bodies as game reconstruction progress.
- Every reached object has a reviewed storage class, complete extent, ownership
  and ordered referents. A referenced first word, unknown-size array, anonymous
  census gap, or unresolved indirect reference is not a completeness proof.
- Source-owned globals have one definition and proper shared declarations;
  interior references use the owner plus a field/element offset. Keep unresolved
  names where evidence does not justify a semantic name.
- Configuration-owned bytes retain explicit image/address/extent, classification
  and evidence. They must not silently disappear from delinking or comparison,
  or become duplicate private copies in each consuming object.
- Initialized bytes, implicit addends, relocation multiplicity/ordering and
  referents are checked. BSS, runtime allocations, overlay memory, SDK storage
  and MMIO need explicit accounting appropriate to their storage, not invented
  retail initializers.
- Retail-to-object-to-retail relocation/placement controls establish that the
  delinker preserves linked bytes. Synthetic object alignment is not proof of
  retail ownership. Unknown placement remains an explicit failure.
- Preserve every banked exact function while validating the stronger data
  contract. Whole-image comparison remains unproven until all preceding
  requirements and remaining code/layout differences are resolved.

## First campaign: make existing checks trustworthy

This campaign changes verification tooling only, not reconstructed C functions
or manually curated identities. The pre-edit audit found:

- Objdiff pairs complete module objects; `kf check` and the default `kf build`
  graph already invoke the additional data gate.
- That gate blanked every relocated word. In MIPS ELF REL, these words are
  implicit addends, not disposable linked addresses. It accepted different
  `.text` destinations in jump tables as equal.
- The coverage helper counted packed object sizes instead of claimed retail
  intervals, and its purported unowned list never subtracted claimed ranges.
- At the starting checkpoint, all 60 data-owning units passed the masked gate,
  but 13 GAME units had different jump-table addends in their emitted objects.

The focused change compares all initialized section bytes after checking ordered
relocation rows, reports `R_MIPS_32` addend differences explicitly, and computes
coverage by interval union/intersection/subtraction. Tests cover pointer offsets,
switch destinations, relocated instruction bits, duplicate/ordered relocation
rows, partial claims, overlapping census intervals, image isolation and BSS.

`kf verify data --coverage` describes source claims within the loaded data census.
It is deliberately not labeled reachable-data coverage: the denominator also
contains SDK data, unresolved gaps and loaded BSS footprints. A complete TSV
partition of the load image is necessary accounting, but is not by itself a
semantic or matching proof.

### Verified checkpoint

The strict result is 47/60 data-owning units: PSX 1/1, GAME 26/39 and OPEN 20/20.
The 13 GAME failures are `render`, `render_enqueue`, `menu`, `menu_panels`,
`menu_select`, `actor_behavior`, `map_object`, `map_interaction`, `player_warp`,
`effect_pool`, `effect_map_collision`, `effect_update` and `magic`. Their first
divergences are `.text`-relative jump-table addends, exposed without changing
source objects or curated retail inputs. Objdiff's own aggregate data percentage
is not a substitute for this stricter gate.

The full `kf build` rejects these mismatches by default. All 113 reconstruction
objects and all 477 reported function scores remain unchanged; the 350 banked
exact functions remain exact. Focused controls, all 372 repository tests, lint,
and `nix flake check -L` pass. The build is intentionally not reported as green.

### OPEN integration checkpoint

Integrating the completed OPEN campaign with `daf79f1` retains 354 exact
functions (PSX 1, GAME 262, OPEN 91) out of 471 eligible. All 117 units were
rebuilt and all 396 local tests pass, as does Ruff. The stronger data result
is **49/63**: the same 13 GAME failures plus `open.render_tmd`. Its first
difference is the `.rodata` relocation at +4, referring to `.text + 0xc9c`
in retail versus `.text + 0xca0` in the compiled object. Both sections are
120 bytes, so this is an addend difference, not missing storage. The OPEN
renderer was already non-exact; its strict function score remains 98.802410%.

The full build still fails the strict data gate; no mask or baseline exception
is added. Earlier OPEN campaign notes reporting all data owners exact describe
the previous masked check, not this stronger result. Continued reconstruction
after integration is OPEN-only, directly on master; GAME failures remain an
explicit backlog rather than being silently changed during that work.

## Remaining campaign work

1. Build a confidence-preserving reachability/ownership audit over the shared
   retail reference evidence. Report unowned, partial, ambiguous and unresolved
   references; retain candidate evidence rather than silently filtering it out.
   Audit references missing from the relocation census as well.
2. Recover related data families and complete extents, starting from actual
   consuming functions. Publish the matcher evidence snapshot before each
   source change. Separate compiler literals/tables, game globals and vendor
   allocations using real ownership evidence.
3. Verify object symbol placement and data relocation topology, including
   defined/undefined ownership and local-section addends. Validate the delinker's
   data-pointer and jump-table handling with linked-byte round-trip controls.
   Calibrate full-section data scoring in objdiff itself, with MIPS controls for
   both the CLI report and GUI rather than assuming x86 addend patches apply.
4. Resolve the exposed section/addend differences and make closure checking a
   default hard gate. A green object comparison must not hide uncovered data.
5. Extend comparison to linked placements and executable bytes once the complete
   ownership, relocation and layout model is established.

## Sibling evidence consulted

The local HoMM2 project's `docs/coff-data-relocations.md` and
`docs/matching/reviewed-external-data-aliases/relocation-closure.md` distinguish
whole-object topology from selected allocations, preserve owner-relative addends,
and reject duplicate private materialization of externally owned data. Its
`docs/matching/automatic-compiler-strings/relocation-closure.md` requires a complete
payload and unambiguous relocation/addend attribution for compiler strings.
Its `patches/objdiff-complete-data-sections.patch` replaces symbol-bounded section
scores with complete data-section comparison; `objdiff-score-reloc-addend.patch`
opts x86 absolute relocations into addend checking and does not opt MIPS in.
Those ownership principles apply here; PE/PDB/COFF relocation mechanics do not
replace King's Field's MIPS/PS-X evidence rules.
