# Three decomps in one repository

This project is one reverse-engineering effort, but it has three independent
decompilation targets:

| Target | Retail role | Load window | Current function census |
| --- | --- | --- | ---: |
| `PSX.EXE` | bootstrap and overlay loader | `0x80010000..0x80010800` | 9 |
| `GAME.EXE` | main game program | `0x80012000..0x80058000` | 937 |
| `OPEN.EXE` | opening/title program | `0x80012000..0x80037800` | 667 |

`GAME.EXE` and `OPEN.EXE` occupy the same RAM window at different times. They
are not sections of one executable. The same virtual address can name unrelated
functions or data depending on which program is loaded. `PSX.EXE` is a third
linked program, not a header attached to either overlay.

Every authoritative address is keyed by `(image, va)`. One objdiff project
groups the independent object pairs under `psx/`, `game/`, and `open/`;
equal overlay addresses do not collide between object pairs. A symbol,
relocation, or function must never be joined across images by address alone.

The repository still shares everything that should be shared:

- the single pinned Psy-Q Release 2.5 SDK and separate GNU/maspsx analysis route;
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
  lib/       reusable helpers, including explicit GAME/OPEN variants
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

Source identities live in `function_identities.tsv` and `data_identities.tsv`;
curated source claims bind them to units in `config/units.toml`. A unit owns
its contiguous function run and its claimed data/RODATA contributions.
These boundaries are working models, not recovered original file names.
See [unit claims](build-system.md#unit-manifest-and-address-claims).

Provider-identified Sony/Psy-Q functions remain library reference objects and
relocation referents; they are excluded from game reconstruction progress.
Functions with multiple fragments remain withheld until their ranges are
explicitly modelled. Current attribution lives in
`config/retail/functions_vendored.tsv`.

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
