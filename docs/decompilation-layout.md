# Three decomps in one repository

This project is one reverse-engineering effort, but it has three independent
decompilation targets:

| Target | Retail role | Load window | Current function census |
| --- | --- | --- | ---: |
| `PSX.EXE` | bootstrap and overlay loader | `0x80010000..0x80010800` | 9 |
| `GAME.EXE` | main game program | `0x80012000..0x80058000` | 939 |
| `OPEN.EXE` | opening/title program | `0x80012000..0x80037800` | 669 |

`GAME.EXE` and `OPEN.EXE` occupy the same RAM window at different times. They
are not sections of one executable. The same virtual address can name unrelated
functions or data depending on which program is loaded. `PSX.EXE` is a third
linked program, not a header attached to either overlay.

Consequently, every authoritative address is keyed by `(image, va)`, and the
matching pipeline produces three objdiff projects. A symbol, relocation, or
function must never be joined across images by address alone.

The repository still shares everything that should be shared:

- the pinned Psy-Q candidate toolchain and GNU/maspsx assembly route;
- platform headers, Sony library declarations, and reconstructed types;
- analysis scripts and Ghidra configuration;
- evidence and naming conventions; and
- source files proven byte-identical between programs.

The source tree is partitioned accordingly:

```text
src/
  psx/       PSX.EXE-only reconstruction
  game/      GAME.EXE-only reconstruction
  open/      OPEN.EXE-only reconstruction
  shared/    code promoted here only after cross-image identity is proven
```

This is therefore “three decomps in one”: three link graphs and three match
scores, with one shared investigation and build environment.

## Address and naming rules

The structural inventory in `config/retail/functions.tsv` remains the source
of function boundaries. `functions_vendored.tsv` overlays Sony/Psy-Q provider
names without replacing that structure. The delinker selects names in this
order:

1. a structural function name;
2. a vendored-function name at the same `(image, va)`; or
3. a deterministic `func_<va>` placeholder.

Duplicate preferred names within one executable receive an address suffix.
They do not need suffixes merely because another executable uses the same name.

Data and relocation ownership will eventually be assigned to translation
units. That ownership is not inferable from the linked image alone, so the
initial pipeline deliberately carves one object per contiguous function. It is
a calibration topology, not a claim that the original source had 1,610
one-function files. The 750 provider-identified Sony/Psy-Q functions never
become reconstruction or objdiff units. Of the seven functions with multiple
fragments and no fragment-range manifest, four are now provider-identified and
three remain non-vendored; all seven are withheld rather than guessed. The 746
contiguous provider functions remain reference objects and relocation
referents.

## Matching tiers

There are two different goals:

1. **Object match.** Compile reconstructed C or assembly, compare its MIPS ELF
   object to the carved target object, and iterate quickly with objdiff.
2. **Retail link match.** Reconstruct original Psy-Q object boundaries, library
   extraction, link order, linker script/options, overlay loading, and PS-X EXE
   packaging with the native tools.

The first tier can progress while the exact compiler and link topology remain
under investigation. A 100% function score proves the chosen object model and
code bytes for that comparison; it does not by itself prove the final Psy-Q
link or the historical translation-unit boundary.

See [delinking-and-matching.md](delinking-and-matching.md) for the implemented
first-tier workflow.
