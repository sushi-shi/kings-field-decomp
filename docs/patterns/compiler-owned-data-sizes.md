# Source symbol sizes must be independent of retail claims

## Evidence and verification plan

Baseline: `c17c8a2`. While investigating strict data placement, review of
`compile_source` exposed a circular input: because GCC 2.5.7 omits data `.size`
directives, the wrapper supplied each DATA claim's size as the source symbol's
size. The same claim supplies the target symbol. Section extent checks do not
make these independent measurements.

This is a compiler-output contract correction, not a function reconstruction
or historical compiler/assembler attribution. Reproduce a complete false
match, derive source object sizes using the same pinned compiler and source,
then rebuild the whole corpus. Preserve all C, claims, profiles, section
payloads/extents/alignments and ordered relocations. Keep newly exposed
ownership mismatches visible; do not change an expected size to preserve a score.

The existing GNU-as sixteen-byte section placement failures remain open. Neither
the earlier `-no-pad-sections` evidence nor this symbol-size correction proves a
smaller historical section alignment or a different original TU boundary.

## Reproduced false match

The native GCC 2.5.7 control declares `int narrow;` followed by `unsigned char
tail[8];`, but deliberately claims a **one-byte** first object. Their claimed
addresses are `80060000` and `80060008`. The compiler emits two eight-byte
COMMON reservations, making a sixteen-byte BSS section aligned sixteen.

The synthetic target owns one byte at offset zero, the independently specified
seven-byte gap, and eight bytes at offset eight. It also has a sixteen-byte
section aligned sixteen. Before the fix, the wrapper annotates `narrow` as one
byte, so complete `diff_unit` reports `.bss: match`, including named allocations
and both placement checks. No byte masking, alignment override or missing
artifact is involved. The C object is still an `int`, not a byte.

After the correction, `narrow` has its compiler-measured four-byte symbol size.
The unchanged gate rejects the named-allocation and owned-symbol mismatch.
Both total BSS extents remain sixteen. No COMMON allocation or gap is removed.
The executable regression control failed against the old pipeline and passes
against the corrected pipeline.

## Independent compiler query

For a C unit with DATA claims, the wrapper compiles an auxiliary copy of its
already-preprocessed source, appending a uniquely named constant table of
`sizeof(object)` expressions. The helper receives only the **names**, not their
claimed sizes or addresses. It uses the identical pinned compiler flags and
assembler pipeline. The compiled table contains a magic value and the target
word width; the reader rejects undefined/ambiguous, truncated, non-initialized,
relocating or non-32-bit output.

These measured values supply the real assembly's `.size` annotations. The
real compilation still uses the original, unaugmented source: no probe table,
extra function, assertion, padding, section rewrite or instruction enters the
reconstruction object. This is a build-time query, not a source size assertion
or a means to force game code generation. The auxiliary object is never a data
counterpart and never contributes a match score.

Object metadata records `data_symbol_sizes.method = pinned-compiler-sizeof-probe`
and the measured mapping separately from the expected DATA claims. Zero-claim
units do not run the query. Assembly-source objects remain assembler-owned;
their symbol sizes are not synthesized from DATA claims.

The controls cover both pinned GCC 2.5.7 and 2.6.0, inferred-length arrays,
internal structure alignment, private/global linkage, target pointer widths,
double arrays, COMMON rounding, preprocessor-selected types and a collision
with the query's preferred name. Changing only a DATA size leaves the entire
source object byte-identical. Removing annotations leaves every allocated
section, alignment and ordered relocation unchanged, proving the query did
not leak into runtime output. The flake compiler check runs these native
controls explicitly, with pyelftools available, not merely as optional skips.

## Full-corpus result

All 112 source units are rebuilt. The query measures **100 DATA owners**;
99 agree with the curated sizes. OPEN `opening_scene0_sound` at `80035874`
is a three-byte `SoundRef`, while its existing claim says four. The source
symbol now correctly reports three. Its following zero byte remains present,
and every emitted byte and later object offset is unchanged. This campaign
does not discard that fourth retail byte, invent a fourth structure member,
or resolve its original allocation ownership without evidence.

Only that source object's symbol-size metadata changes; the other 111 source
objects and all 112 module targets are byte-identical. Every allocated section's
bytes, size, alignment and ordered relocations are preserved in all units.
All 484 function-score rows remain unchanged: **360/471 exact game functions**
and thirteen exact vendor controls. No function is newly banked.

The mandatory full build reports the new `owned-symbol-layout` discrepancy,
expected size four versus actual three, alongside the existing OPEN scene-unit
section-base conflict. Strict source data remains **8/60**, SDK data **4/4**
and target relinking **110/116**. The 623 reached config ranges without matching
counterparts and source/header DAT_ count 167 are unchanged. Full reachable-byte
coverage and linked-executable equality are still unproved.

Reproduce inside `nix develop`:

```sh
python -m unittest tests.test_compile_data_sizes -v
python tests/compiler_mips_smoke.py
kf build compare
python -m unittest discover -s tests
kf build
```

All **591 local tests** pass without skips, and both native compiler smoke
probes, Ruff and diff checks pass. The full build remains red on strict data,
reference closure and placement failures; no comparison gate or baseline is
relaxed.

`nix flake check -L` also passes. The ordinary sandbox suite runs 591 tests
with 100 expected local-tool/artifact skips; the separate compiler check runs
all four new native size tests without skips, alongside both existing compiler
and objdiff smoke controls.

Follow-up: [the OPEN sound-extent campaign](../../config/evidence/open_sound_data_extent.md)
corrects the three-byte claim while separately retaining its following byte.
