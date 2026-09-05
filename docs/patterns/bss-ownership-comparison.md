# BSS equality requires allocation identity

Uninitialized storage has no payload bytes, but equal totals are not enough for
linking. The section, extent, named object's offset/size and linkage determine
which storage an external reference or a private access actually reaches.

## Reproduced false matches

Before this correction, `scripts/kf/data_match.py` summed `.bss` and `.sbss`
sizes and compared only the sum. Objdiff's already-patched whole-section check
compared BSS extents and anonymous `(offset, size)` rows, but omitted names and
linkage. All of these controls preserve the same total allocation:

| Control | Previous `kf verify data` | Previous native objdiff |
| --- | --- | --- |
| Swap two equal-sized named objects | accepted | 100% |
| Change private storage to exported | accepted | 100% |
| Rename a defined object | accepted | 100% |
| Move `.bss` storage into `.sbss` | accepted | rejected |

The first two and storage-class controls were executed against the previous
Python gate and failed their rejection assertions. The renamed/swapped/linkage
controls were executed against the previous native CLI in both directions;
all six comparisons incorrectly returned 100%. These are synthetic ELF
controls, independent of a guessed retail data owner or function score.

## Corrected checks

The command-line gate now compares BSS per section, including allocated custom
NOBITS sections. It checks NOBITS type/flags, complete extent, and a sorted
census of named definitions: spelling, offset, explicit size, binding and ELF
visibility. ELF row order is incidental, but an omitted or renamed definition
is not interchangeable with anonymous storage. Section/file symbols are not
named allocations. No initialized bytes or relocation fields are masked.

`patches/objdiff-bss-ownership.patch`, applied after the whole-data patch,
compares the named BSS census in the shared CLI/GUI core. It includes allocation
offsets/sizes, binding flags and the explicit-versus-inferred size distinction;
an inferred size cannot silently stand in for a proved sized definition. The
existing complete-section extent check remains. Function instruction scoring
is untouched. The native controls exercise both comparison directions, JSON
section output and native reports, and retain an exact code-symbol control
even when data is deliberately wrong.

The Python suite also exercises the default gate with a real private/global
mismatch, and uses GNU objcopy to produce `.sbss` and custom NOBITS objects.
Both the missing old section and extra new section must be reported. Positive
controls retain matching private and anonymous storage and ignore symbol-table
row order.

## Full-corpus verification

All 117 source units were recompiled and all three target images redelinked.
Every source and target object remains byte-identical; all 484 function score
rows remain identical, preserving 354 exact game functions. No claim, C body,
compiler profile, allocation extent or target relocation changes.

All 482 local tests pass, as do Ruff, the 33 native data controls and the full
flake checks (53 local-retail/oracle skips only inside the flake sandbox).
The full retail build still fails its strict data, placement and reachability
gates: source data 15/63, target relink 109/117, and 665 config-only reached
ranges. This is a stronger comparison contract, not closure of those ranges
or proof of whole-program equality.

The target/source allocation model still needs original small-BSS versus
external-BSS/COMMON ownership and extent evidence. Equal current BSS claims
only establish equality of those compared claims; they do not account for an
unmodeled allocation or infer the size of a surrounding object.
