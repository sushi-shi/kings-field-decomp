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

## Known-reference ownership gate

The data campaign continues on `codex/data-matching-20260905` in its own worktree;
the earlier OPEN checkpoint's master-only workflow does not apply to this work.

`kf check` and the default build now run the known-reference ownership audit as
well as the strict object-data comparison. The full evidence report is available
without changing curated inventories:

```sh
kf verify reachability --output build/data-reachability/all.json
kf verify reachability --image game --output build/data-reachability/game.json
```

Every admitted non-vendor function is a root, regardless of source manifestation
or match score. The worklist follows referenced functions (including vendors),
source-owned data and configuration extents. It scans whole admitted ranges
conservatively, includes interior pointer-table sites, terminates cycles and
retains an incoming-reference witness for each reached range. Image namespaces
never share owners. Candidate edges and ambiguous owners weaken a path; later
validated edges do not promote that path. An independent stronger path can update
the witness, but the weaker reference remains visible in the report.

The report distinguishes source-owned data from config-only extents. The latter
are inventory accounting, not yet a reconstruction comparison: they may still
be anonymous gaps or referenced prefixes. A config-only range therefore remains
a default-build failure until its storage, full extent, owner and comparison
path are resolved. This does not require every vendor datum to become C source;
an explicit config-backed delinking/comparison path is still required for those
bytes. Source claims take precedence over interior census rows, without creating
duplicate private copies in consumers. Genuine overlapping owner models remain
ambiguous instead of being resolved by a naming guess.

Other diagnostics include invalid live relocation rows, missing targets,
cross-owner access/relocation spans, fragmented functions and unresolved indirect
control flow. Manually rejected candidates are retained but not followed. The
shared semantic navigator now validates reviewed data pointer rows with the same
`validate_relocation` rule the delinker uses; a matching raw word alone does not
promote an unreviewed candidate. Nested semantic scripts are build dependencies
so changes to that validation cannot leave a stale successful build stamp.

The initial audit from all 471 game roots reaches 603 vendor functions and finds
123 source-owned data ranges and 673 config-only ranges (GAME 392, OPEN 281).
These are range counts, not exact bytes or evidence of complete allocation
extents. Concrete diagnostics include GAME's prefix-only `tmd_morph_scratch`,
two reviewed grid-reference rows whose HI sites are not LUI instructions, and
data-census strings overlapping admitted instruction bytes. Those curated inputs
are not automatically rewritten by the audit.

The audit is explicitly scoped to the known reference census. Its JSON always
records `complete_reachable_bytes_proven: false`: computed/GP-relative addresses,
missing reference candidates, allocation capacities and unresolved indirect
targets still require evidence work. A clean known-reference report alone will
not establish full goal closure or linked-executable equality.

Verification of this tooling checkpoint: all 117 source objects were rebuilt and
remained byte-identical, all 484 reported function scores were unchanged, and all
354 banked exact functions remained exact. All 419 local repository tests, Ruff,
`git diff --check` and `nix flake check -L` passed. The full build correctly fails
the new known-reference gate on all three images, in addition to the existing
14 GAME/OPEN strict data-addend failures. No source function, curated inventory
or banked score was changed by this checkpoint.

## Collision-grid reference correction

The first inventory correction resolves both invalid live grid-reference rows
in GAME.EXE `collision_query_world`. Their recorded HI/LO sites were four bytes
late, naming ADDIU/ADDU instead of LUI/ADDIU. Retail bytes prove the corrected
pairs at 0x8001a67c/0x8001a680 and 0x8001a6dc/0x8001a6e0; the grid identities,
extents and C source remain unchanged. The evidence snapshot and raw-word
round-trip control are documented in
`config/evidence/game_collision_grid_relocations.md`.

Both pairs now validate and delink to their existing external grid symbols.
The function remains non-exact at 93.202490% (previously 93.190030%); all 117
source objects and the other 483 scores are unchanged, including all 354
historically exact game functions. All 420 local tests, Ruff and flake checks
pass. The full build still rejects the remaining ownership gaps and the same
14 data-addend mismatches; strict data remains 49/63. Nothing was banked.

## Instruction/string-census ownership review

The first code/data overlap review removes 31 byte-pattern string claims
(147 bytes) inside four reviewed GAME bodies and the LIBSND `note2pitch` /
`SpuVmSelectToneAndVag` pair in both overlays. These were slices of instructions,
including return delay slots and called entries, not separate data allocations.
Original rows, instruction windows and per-function evidence remain in
`config/evidence/data_census_instruction_overlaps.{md,tsv}`. No executable
bytes, source definitions or real string literals are removed.

The known-reference audit now also checks the entire reached function body for
overlapping data claims, even when no reference targets the overlap. Each
`code-data-owner-overlap` issue retains all conflicting metadata and clipped
intersections. It does not automatically reject a census hypothesis, infer
fragment-hole ownership or promote candidate paths. The new check exposes
1,441 distinct claims across 418 reached functions (GAME 773/240, OPEN 668/178).
There are 1,860 remaining scan/code overlaps across all admitted functions;
the difference is not a proof of dynamic unreachability.

The correction resolves the previously reported 15 ambiguous target occurrences
and one cross-owner relocation-site occurrence. Reached config-only ranges are
now 666 (GAME 386, OPEN 280), down by seven phantom code-as-data nodes; this is
not seven newly reconstructed allocations. All three complete loaded payloads
remain accounted for by the code/data inventory union.

All 117 reconstruction units were rebuilt. All 1,722 target object hashes,
117 source object hashes and 484 function scores remain unchanged, including
all 354 historically exact game functions. All 428 local tests, lint and flake
checks pass. The full build still fails the explicit ownership/reference gaps
and the same 14 data-addend mismatches; strict data remains 49/63. The larger
reachable-data and linked-image objective is still incomplete.

## Native objdiff section-scoring campaign

The campaign plan is tooling-only: reproduce complete-section and MIPS REL
negative controls, patch the shared comparator, build both front ends from the
same pinned source, then recompare all retail/source objects without editing C
or curating new relocation rows. The pinned upstream is objdiff 3.7.3, commit
`6bcac60df8bb0b4de5b1cb98b033bdedb9ac6aa4`; the patch is
`patches/objdiff-strict-data.patch`.

The upstream comparator stopped at the last visible symbol and could take the
higher of byte and symbol scores. An anonymous `.rodata` section therefore
scored 100% despite changed bytes or relocations. A named four-byte prefix also
hid a changed unlabelled tail. Symbol-less BSS extents could differ and still
score 100%. Relocations at a shared offset reused the first opposing row,
losing multiplicity and order. These are comparator defects, not game codegen
residues. The MIPS reader already decodes implicit `R_MIPS_32` addends; importing
an x86-only addend hook would not fix these paths.

The patch compares every initialized byte, including padding and REL addends,
and consumes relocation rows one-to-one in stable offset order. Named data
referents must agree; coincident object-relative addresses do not prove identity.
It checks BSS allocation extents and symbol layout without a higher-score escape.
Both compared sides receive the same section percentage. An inexact initialized
section cannot round up to 100%. Function instruction scoring is unchanged.

MIPS data relocations retain the input section referent and implicit addend.
Upstream's best-symbol display resolution otherwise rewrote the same `.text+N`
into a game function on the target side and an incidental `LM1` debug label on
the source side. GAME `player_core` demonstrated this with byte-identical
24-byte jump tables. Equal raw section references remain equal even when only
one object has debug labels; changing their offset or an actual named referent
still fails. Instruction-relocation display resolution is untouched.

Native report data totals use the larger paired section extent and include
base-only allocations as unmatched. Thus extra storage cannot disappear from
the denominator. These are compared-object storage totals, **not** a count of
reachable retail bytes or proof that the inventory is complete. The independent
strict data and known-reference ownership gates remain mandatory.

Retail verification also exposed a stale-report dependency: changing objdiff
recompiled the C objects, but their unchanged hashes triggered Ninja `restat`
and left old reports in place. Reports and verification stamps now depend
directly on the resolved toolchain identity. Re-entering the pinned environment
therefore cannot retain the old data score merely because source bytes agree.

`tests/objdiff_data_smoke.py` exercises actual CLI `diff` and `report generate`
outputs in both target/base directions, including anonymous bytes, a named
prefix with an unlabelled tail, zero padding, missing sections, external and
`.text`-relative pointer addends, missing/extra/duplicate/ordered/retargeted
relocations, and BSS extents. Positive controls retain identical bytes and
ordered duplicate-site rows. Every case also checks its synthetic function is
still exact. The flake's `objdiff-mips` check runs these controls alongside the
existing MIPS instruction/compiler controls.

### Verified native checkpoint

The rebuilt native reports now expose exactly the existing 14 strict data
failures (13 GAME and one OPEN). Their compared-object data totals are:

| Image | Matched section bytes / compared bytes | Native matched-data percentage |
| --- | ---: | ---: |
| PSX.EXE | 56 / 56 | 100% |
| GAME.EXE | 12,548 / 14,748 | 85.08272% |
| OPEN.EXE | 16,988 / 17,108 | 99.29857% |

Previously all three native reports said 100%. These totals count an entire
section as matched only at 100%; they are not fuzzy matched-byte counts or
reachable-image coverage. The independent data-owning-unit result stays 49/63.
Both front ends build from one patched core; its upstream tests, 25 native
MIPS data controls, all 429 local repository tests, Ruff, whitespace checks and
`nix flake check -L` pass. GUI binary construction and its version entry point
were checked; no interactive display session is claimed.

All 117 source units were rebuilt, and all three reports were regenerated after
the comparator identity changed. All 1,722 target objects, 117 source objects
and 484 function scores remain identical, preserving all 354 banked exact game
functions. No C source, curated input or banked score changes in this campaign.
The full build still rejects the 14 real addend mismatches and the unresolved
ownership/reference work, including 666 config-only ranges. Whole reachable-byte
coverage and linked-executable comparison remain incomplete.

## Independent target relink and placement gate

The next tooling campaign uses the pinned GNU MIPS linker to check actual
target-object placement and REL fixups against retail. `kf check` and the
default build now include it. The standalone report is:

```sh
kf verify roundtrip --output build/roundtrip/all.json
kf verify roundtrip --image game --unit game.cd_file
```

Every symbol in a packed section must imply the same base (`retail VA` minus
actual ELF symbol offset). Sections must satisfy their alignment and may not
overlap. Undefined identities are bound from image-local curated inputs, never
from the delinker's relocation-output log. GNU ld applies the fixups; every
initialized output byte is compared with the verified retail image. BSS stays
uninitialized. Unknown placements, externals, section types or unsupported
relocations fail. GNU ld is an independent relocation oracle, not historical
toolchain attribution or proof of reconstructed executable equality.

This found a target-model error: `_module_object` appended zero tails to match
the probe assembler's 16-byte writable-section and eight-byte switch-table
extents. Twenty-four initialized sections then contained bytes different from
retail, sometimes replacing the next module's string or jump table. The tails
are removed, not copied into source or masked in comparison. The
[layout evidence](../config/evidence/target_roundtrip_layout.md) records all
observed byte contradictions and the remaining unplaceable symbol pairs.

After rebuilding, target relink improves from 76/117 to **105/117** verified
units (PSX 1/1, GAME 72/75, OPEN 32/41), with no remaining byte-mismatch or
cross-unit section-overlap diagnostics. Ten packed DATA/BSS sections still
imply conflicting bases, and two BSS sections have alignment conflicts. These
are unresolved ownership/section-model facts, not linker options to suppress.
Config-only and standalone vendor objects are outside this gate's present
scope; exhaustive reachable-byte closure remains explicitly false.

The stricter source-data result drops from 49/63 to **16/63** data-owning units
(PSX 0/1, GAME 10/39, OPEN 6/23). Compiler-emitted extra bytes now diverge from
the actual retail claims. The earlier 14 jump-table addend differences remain;
many now encounter an extent failure first. No source bytes or function scores
were changed to recover the old data percentage. Native compared-object totals
now report PSX 40/56 (71.42857%), GAME 10,932/14,748 (74.125305%) and OPEN
10,644/17,108 (62.216507%) matched section bytes, not reachable-image coverage.

All 117 source units were rebuilt and remain byte-identical, as do all 484
reported function scores and all 354 historically exact game functions.
Forty-four of 1,722 target objects change, all module objects with removed
alignment tails; per-function targets remain identical. All 454 local tests
and Ruff pass, including 24 new placement/linker controls and the default-gate
control. `nix flake check -L` also passes: the sandbox suite retains its 48
expected local-retail/oracle skips, while the new GNU-linker controls run there.
The full build correctly rejects the 47 source-data failures, twelve
target-placement failures and outstanding ownership/reference gaps. The 666
config-only ranges remain unresolved. No function is banked.

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
