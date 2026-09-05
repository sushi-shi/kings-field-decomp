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

## PAD storage and SDK linkage correction

Reviewing the two PAD placement failures against the supplied Sony archive and
header exposes the underlying source assumptions. `LIBETC.H` declares exported
`int PadIdentifier`; the source's private `s32` declaration contradicted that
contract. PAD.OBJ places its local pad words in `.sbss` and exports the
identifier through XBSS in `.bss`. These are distinct allocation classes, not
evidence that the PAD functions belong to different translation units.

Both programs now use the authentic exported type/declaration. OPEN's private
pad words also change from explicit loaded zeros to tentative BSS definitions:
they lie after CPE residue at `0x800375d8`, and the archive plus the retail
write-before-use sequence supports uninitialized storage. Their two census rows
remain present at identical addresses and four-byte extents, classified as BSS.
No load-page bytes are removed from the inventory union. The
[PAD evidence and per-function verdicts](../config/evidence/pad_storage_and_linkage.md)
retain raw record details, hashes, instruction witnesses and the version-skew
boundary.

Both six-function vendor units remain objdiff 100%; all 484 function scores
remain unchanged, preserving the 354 exact game functions. Only the two PAD
source objects and their two module targets change; the other 115 source
objects and 1,720 target objects remain identical. Neither PAD unit is yet a
faithful allocation match: explicit `.sbss`/external-BSS support remains needed
in both the target and probe paths. The default gates retain the two placement
failures, and the overall target result remains 105/117 with strict source data
16/63. Config-only reachability and complete linked-image equality remain open.

All 459 local tests, Ruff and flake checks pass. The flake suite now receives
the pinned SDK headers/archive tools and checks the real PAD section records;
49 tests needing local retail/oracle artifacts are skipped only in that
sandbox. The full build correctly fails the unresolved data, placement and
reachability gates. No game function or score is banked.

## Data section alignment correction

The target writer now preserves the alignment constraints already used to pack
each section's member claims. A fixed four-byte ELF default had contradicted
the byte/halfword layouts in GAME notifications and OPEN's formatter scratch.
No C source, address, extent, packed symbol offset or relocation changes; the
formatter's complete original allocation remains explicitly unresolved.

Both current target models now relink without linker overrides, improving
verified targets from 105/117 to **107/117**. The ten conflicting section-base
failures remain. Six target objects change alignment metadata, while all 117
source objects and 484 function score rows remain identical, preserving 354
exact game functions. Strict source-data matching remains **16/63**. Details
and raw reference witnesses are in the
[target layout evidence](../config/evidence/target_roundtrip_layout.md#carry-packing-constraints-into-elf-section-alignment).

All 465 local tests, Ruff and flake checks pass. The full retail build continues
to reject the unresolved data, placement and reachability gaps. This is target
fidelity progress, not reconstructed linked-executable equality or byte closure.

## CD and opening-resource BSS recovery

The retail CD consumers overwrite minute/second/sector before CdlSetloc; the
authentic SDK type retains its fourth track byte, which this command does not
read. Both location objects are now BSS. GAME retains its shared declaration
in the GAME CD header for the error-screen consumer; OPEN gains a private
source definition. OPEN's scene-one and ending arena cursors also become
private tentative storage, with reviewed save-before-restore paths. All four
census ranges retain their addresses and extents; CPE residue/page zeros are
not modeled as source initializers. The
[campaign evidence](../config/evidence/cd_resource_bss.md) records each consumer,
raw instruction/table witnesses, provider boundaries and final verdicts.

Target relinking improves to **109/117**, while OPEN's config-only data backlog
falls by one (279, or 665 across both overlays). All 484 function scores remain
unchanged, preserving 354 exact game functions. Only two source objects and
two module targets change. Strict source data drops to **15/63** because the
corrected GAME storage exposes allocation/section extents that previously
appeared exact under the false initialized model. The stronger gate retains
those mismatches; it does not round targets to fit the source.

All 471 local tests, Ruff and flake checks pass. The full build remains red
on the data mismatches, eight remaining target placement conflicts and the
unresolved reachable-data ownership paths. No whole-image equality is claimed.

## Preserve private tentative-data linkage

The pinned assembler adapter previously exported every `.bss` object, losing
the compiler's `.lcomm` private linkage. A shared adapter patch now preserves
private/public identity in both BSS allocation classes and its opt-in COMMON
paths. No game source or target allocation is adjusted to fit the adapter.
The [linkage evidence](patterns/maspsx-private-bss.md) includes a reproduced
failure, both compiler probes and three-TU GNU linker controls.

All 117 source units rebuilt. Six objects change eleven private bindings and
their equivalent section-relative relocation forms; allocated section layouts
and named symbol offsets/sizes remain unchanged. The other 111 objects and all
484 function score rows remain identical, preserving 354 exact game functions.
Strict source data remains 15/63; target relinking remains 109/117. All 471 local
tests, Ruff and flake checks pass. The full build remains red on the existing
data, placement and reachability gaps, including 665 config-only data ranges.

## Reject equal-size but different BSS ownership

The data gate no longer sums `.bss` and `.sbss` into one anonymous extent.
It compares every BSS section, including allocated custom NOBITS sections,
and each named object's offset, size and linkage. The shared objdiff CLI/GUI
core also retains names and binding when comparing BSS layouts. Previously,
swapping two equal-sized globals or exporting private storage could still
report 100%; the [ownership controls](patterns/bss-ownership-comparison.md)
reproduce those false positives and verify their rejection.

All source units were recompiled and all targets redelinked. Every object and
all 484 function score rows remain identical, preserving 354 exact game
functions. All 482 local tests, 33 native data controls, Ruff and flake checks
pass. The full build remains red with strict source data 15/63, target relink
109/117 and 665 config-only reached ranges. This correction strengthens the
comparison contract without changing source, claims or delinked bytes.

## Consolidate the OPEN scene ownership pilot

Six granular TUs now form one seven-function opening-scene module, with one
shared scene header replacing four narrow headers. Shared dispatcher/helper
calls and a gap-free 896-byte camera-path run support the consolidation;
the original filename and exact TU boundary remain WIP. All bodies, data
initializers, types and bindings are preserved. The
[campaign plan and verdicts](../config/evidence/open_opening_scenes.md) record
each function, the two real data runs and the remaining ownership limits.

All 484 function scores remain unchanged, including 354 exact game functions.
Raw pre/post instruction and ordered referent checks pass for every merged
function. The dispatcher's only object change is a debug-line number; all
other 110 compiled objects and 111 target objects remain byte-identical.
All seventeen data objects individually match retail, but their two distant
runs still contradict the current single-section placement model. The strict
gate rejects that contradiction and the compiled section's extra eight bytes.

TUs fall from 117 to **112**. Regrouping changes strict source data to **13/60**
and target relinking to **106/112**, with six remaining placement conflicts.
These are changed unit denominators, not newly closed bytes. Reachability
remains at 665 config-only ranges. The audit also exposes a biased sine-table
base mislabeled as camera data and a startup GP base mislabeled as UV data;
neither is silently reclassified in this source-only campaign.

All 485 local tests, Ruff and flake checks pass. The full default build remains
red on the explicit data, placement and reachability failures. No new function
is banked, and no linked-executable equality is claimed.

## Recover SDK sine-table owners and signed referents

The pinned `LIBGTE.LIB/GEO.OBJ` proves both complete 2048-byte `rsin_tbl`
allocations and the negative relocation addends that previously pointed the
semantic graph into camera data, a path string and even `sprintf`. Six SDK
bodies and both table payloads equal retail after applying the original SDK
patches. The [campaign evidence](../config/evidence/psyq_rsin_table.md) records
all sixteen corrected pairs and each vendor function's verdict.

The shared referent model now preserves decoded `S+A` separately from its
reviewed allocation `S`. Delinking, xrefs and reachability agree on that owner,
including biased and one-past address expressions; real load/store accesses
still retain bounds diagnostics. Ambiguous/candidate/rejected identities are
not silently promoted. Unit-owned RODATA retains its actual section identity.

All 112 compiled objects, all 112 module targets and all 484 function score
rows remain unchanged, preserving 354 exact game functions. Only four
individual vendor target objects acquire the corrected table references.
Strict source data remains 13/60 and target relinking 106/112. The improved
graph also reaches the existing `floor_entry_cells` owner through its reviewed
negative offset, raising config-only ranges to **666** (GAME 387, OPEN 279).

All 502 local tests, Ruff and flake checks pass. The full default build remains
red on the explicit data, placement and reachability failures. Complete SDK
owners are not yet default-compared: a config-data delink/comparison lane is
still required, and no whole-image equality is claimed.

## Compare complete config-owned SDK contributions by default

The two sine-table owners now have real data-only target/base objects in the
normal build and objdiff projects. Bases come from the pinned SDK's complete
section; targets come independently from retail. Current native reports,
whole-object identity/storage checks and two-sided GNU relinking are required
before reachability calls either range compared. The
[campaign evidence](../config/evidence/config_data_contributions.md) records the
provider hash, full ownership boundaries, rejection controls and one explicit
false `jal` candidate made from two sine samples.

This adds **4096 default-compared bytes**, with SDK config data 2/2 separately
from source data 13/60. Target relinking is **108/114**, adding two passing
data-only targets to the unchanged source-unit results; the same six placement
conflicts remain. There are still 666 reached config-owned ranges, but two now
carry complete comparison proofs, leaving **664 unpaired** (GAME 386, OPEN 278).

All 112 compiled source objects, all 1717 pre-existing target objects and all
484 function score rows remain identical, preserving 354 exact game functions.
All 530 local tests, Ruff and flake checks pass. Small byte/halfword/alignment
controls ensure the SDK converter preserves actual section sizes without
inheriting GAS's ordinary-data padding or trimming bytes. The full default
build remains red on source-data, placement and reachable-ownership failures.
Unsupported SDK data forms and unresolved game/census owners remain explicit
work; no whole-executable equality is claimed.

## GAME player DATA ownership checkpoint

The player weapon path, floor-entry cell table and two item-interaction image
paths now have complete initialized DATA definitions in their consuming
modules. All three mutable path extents now include their retail NUL; their
old catch-all extern declarations omitted it. The floor table retains the
proven x-then-z byte fields and the exact -2 relocation addend. Its former
extern const storage assumption is replaced by the initialized-data owner.
See [per-function and data evidence](../config/evidence/game_player_data.md).

Four reached ranges move from config-only to source ownership. All **55
initializer bytes** agree with retail in both compiled and delinked objects;
this is not a claim of whole-section equality. The strict gate now exposes
core DATA 26 versus 32 bytes and item-use DATA 31 versus 32. Source-data
matches therefore drop from 13/60 to **11/60**; SDK config comparisons remain
2/2. The extra core tail overlaps the following image path in retail, so it
is not silently included as padding. No compiler output is cropped.

Only two bases and two module targets change. All function text, ordered text
relocations, RODATA and 484 function score rows remain unchanged, preserving
all 354 exact game functions. Both updated targets relink through GNU ld;
all-image target relinking remains 108/114 with the same six conflicts.
Unpaired config ranges fall from 664 to **660**. Reachable-byte closure and
linked-executable equality remain unproven.

All 535 local tests, Ruff, diff checks and clean flake checks pass (59
local-retail/oracle skips in the clean sandbox). The full comparison graph
completes; the default build remains red on explicit data, placement and
reachability failures. No exact function is banked in this ownership campaign.

## GAME collision attribute DATA and referent checkpoint

The old 284-halfword height allocation is now 255 consumed scalar entries plus
seven typed four-halfword collision rectangles, owned by the effect collision
module. The intervening zero halfword stays explicitly unclassified: unused
array element versus alignment is not proved. The shared collision header owns
the record type; no artificial extent or padding is added. Three relocation
owners are corrected without changing their retail S+A: the rectangle table,
collision-grid minus one row, and the magic height lookup at table minus two.
See [the Function Match Plan and final verdict](../config/evidence/game_collision_attribute_data.md).

Following those records also exposed a real source orientation-dispatch error.
A separate switch correction agrees with retail in 2,355 bounded scenarios
(4,710 isolated function calls), including all seven records, inclusive bounds,
unsupported orientations and forwarded query flags. The effect function rises
from 71.711580% to **74.821050%**, still non-exact. All other 483 function report
rows stay unchanged, including every one of the **354 exact game functions**.

The three updated targets independently relink every initialized byte to retail;
all-image target relink stays 108/114 with the same six placement conflicts.
Strict source DATA stays **11/60**, config SDK data 2/2, and unpaired reached
config ranges 660. The scalar section is 510 target versus 512 compiled bytes;
the record section is 56 versus 64, with its excess overlapping the next path.
The effect jump-table addends still differ. None of this is masked or cropped.

All 540 local tests, Ruff, diff checks and flake checks pass (62 expected clean
environment skips). The full compare graph completes; the default build remains
red on strict data, reachability and placement. No bank or compiler-profile
change, no original-TU claim, and no linked-executable equality claim.

## GAME player-effect objects and fade control flow

Three reached objects now have initialized typed owners: the shared nine-byte
`SoundRef` table in the player death/stats module, and a private SDK `MATRIX`
plus eight `SVECTOR` damage-camera offsets in the update module. All **105
bytes** equal retail in source and target objects. Two unresolved DAT names
and their catch-all declarations are replaced by supported private identities;
all ten HI/LO referents retain their raw targets and element/lane addends.
See [the Function Match Plan and final verdict](../config/evidence/game_player_effect_data.md).

The matrix consumer revealed an actual fade-branch error. Correcting it in a
separate focused build passed 2,516 paired bounded scenarios (5,032 isolated
calls), covering both fade ends, the middle hold, cancellation, expiry, inactive
fog restoration and every damage-state byte. `player_update` rises from
96.515860% to **96.945540%**, still non-exact. All other 483 reported function
rows and all **354 exact game functions** stay unchanged.

The final ownership exposes an additional real placement conflict: the early
matrix/table and distant previous-input word cannot occupy one flat `.data`
section. Current candidate execution and independent target relinking reject
that contradiction; neither uses a scatter/override workaround. Comparing the
CFG-only and final objects proves ownership changes only the relevant data
relocations/addends, not instructions or call topology. Per-datum byte matches
are not whole-section or linked-image closure.

Strict DATA remains **11/60**, SDK data 2/2, and target relink is **107/114**
with seven placement conflicts. Unpaired reached config ranges fall **660 to
657**. The 546 local tests, Ruff, diff checks and flake checks pass (66 expected
clean-environment skips); the full default build remains red on data,
reachability and placement. Work remains isolated on
`codex/data-matching-20260905`; no new function is banked.

## Master integration checkpoint

The user requested integration of the complete data-matching branch after
`d2f7a20`. The merge was resolved and verified in its isolated worktree against
`master` at `57f0a13`, preserving all sixteen branch commits and twenty-two
concurrent master commits. Master's publication/license changes and OPEN
graphics-runtime owner remain intact. Its updates to three now-consolidated
opening files are carried into `opening_scenes.c`; the corresponding linked-
instruction controls now select the same functions in that module. Inventory
count expectations are derived from the combined inventories, not either
parent's totals. No function is dropped or new matching technique introduced.

The merged result preserves **360/471 exact game functions** (PSX 1, GAME 262,
OPEN 97), including all six newer OPEN exact results from master. All 114 OPEN
function-report rows, including six vendor controls, equal master's current
report; GAME and PSX rows equal the verified data-branch checkpoint. Strict
DATA remains enabled by default and reports **11/59** source owners; the two
complete SDK data contributions still match. Master's aggregate removes the
separate active-window BSS placement conflict: target relink is **108/114**,
with six unresolved placement conflicts. Unpaired reached config ranges are
now **646** (GAME 379, OPEN 267). These are accounting improvements, not proof
of exhaustive reachable-byte coverage or linked-executable equality.

All **551 local tests** pass without skips, as do Ruff and diff checks. The
full compare graph rebuilds all affected units successfully; the default full
build still rejects explicit data, reachability and placement failures. The
master update is a fast-forward to the verified merge, not a reset or rewrite
of either parent's history.

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
